#include "xyra.h"
#include <string.h>

static int is_alpha(int c) {
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

static int is_num(int c) {
	return c >= '0' && c <= '9';
}

static int is_alnum(int c) {
	return is_alpha(c) || is_num(c);
}

static int is_ws(int c) {
	return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

void lexer_init(Lexer *lx, const char *src) {
	lx->src = src;
	lx->pos = 0;
	lx->line = 1;
	memset(&lx->cur, 0, sizeof(Token));
	memset(&lx->peek, 0, sizeof(Token));
	lexer_next(lx);
	lexer_next(lx);
}

static Token mk_tok(TokenKind kind, const char *start, int len, int line) {
	Token t;
	t.kind = kind;
	t.start = start;
	t.len = len;
	t.line = line;
	return t;
}

void lexer_next(Lexer *lx) {
	const char *p = lx->src + lx->pos;
	int start = lx->pos;

	lx->cur = lx->peek;

	/* skip whitespace */
	while (is_ws(*p)) {
		if (*p == '\n') lx->line++;
		p++;
		lx->pos++;
	}

	if (*p == '\0') {
		lx->peek = mk_tok(TOK_EOF, p, 0, lx->line);
		return;
	}

	start = lx->pos;

	/* identifiers and keywords */
	if (is_alpha(*p)) {
		while (is_alnum(*p)) {
			p++;
			lx->pos++;
		}
		const char *ks = lx->src + start;
		int len = lx->pos - start;
		if (len == 2 && memcmp(ks, "fn", 2) == 0) { lx->peek = mk_tok(TOK_FN, ks, len, lx->line); return; }
		if (len == 3) {
			if (memcmp(ks, "end", 3) == 0) { lx->peek = mk_tok(TOK_END, ks, len, lx->line); return; }
			if (memcmp(ks, "for", 3) == 0) { lx->peek = mk_tok(TOK_FOR, ks, len, lx->line); return; }
			if (memcmp(ks, "var", 3) == 0) { lx->peek = mk_tok(TOK_VAR, ks, len, lx->line); return; }
			if (memcmp(ks, "int", 3) == 0) { lx->peek = mk_tok(TOK_INT, ks, len, lx->line); return; }
		}
		if (len == 4) {
			if (memcmp(ks, "else", 4) == 0) { lx->peek = mk_tok(TOK_ELSE, ks, len, lx->line); return; }
			if (memcmp(ks, "true", 4) == 0) { lx->peek = mk_tok(TOK_TRUE, ks, len, lx->line); return; }
			if (memcmp(ks, "void", 4) == 0) { lx->peek = mk_tok(TOK_VOID, ks, len, lx->line); return; }
		}
		if (len == 5) {
			if (memcmp(ks, "float", 5) == 0) { lx->peek = mk_tok(TOK_FLOAT, ks, len, lx->line); return; }
			if (memcmp(ks, "while", 5) == 0) { lx->peek = mk_tok(TOK_WHILE, ks, len, lx->line); return; }
			if (memcmp(ks, "false", 5) == 0) { lx->peek = mk_tok(TOK_FALSE, ks, len, lx->line); return; }
		}
		if (len == 6) {
			if (memcmp(ks, "return", 6) == 0) { lx->peek = mk_tok(TOK_RETURN, ks, len, lx->line); return; }
			if (memcmp(ks, "const", 6) == 0) { lx->peek = mk_tok(TOK_CONST, ks, len, lx->line); return; }
		}
		if (len == 4 && memcmp(ks, "bool", 4) == 0) { lx->peek = mk_tok(TOK_BOOL, ks, len, lx->line); return; }
		if (len == 3 && memcmp(ks, "str", 3) == 0) { lx->peek = mk_tok(TOK_STR_KW, ks, len, lx->line); return; }
		lx->peek = mk_tok(TOK_ID, ks, len, lx->line);
		return;
	}

	/* numbers */
	if (is_num(*p)) {
		long val = 0;
		while (is_num(*p)) {
			val = val * 10 + (*p - '0');
			p++;
			lx->pos++;
		}
		lx->peek = mk_tok(TOK_NUM, lx->src + start, lx->pos - start, lx->line);
		lx->peek.num_val = val;
		return;
	}

	/* strings */
	if (*p == '"') {
		p++;
		lx->pos++;
		const char *str_start = p;
		int str_len = 0;
		while (*p && *p != '"') {
			if (*p == '\\' && p[1]) {
				p += 2;
				lx->pos += 2;
				str_len++;
			} else {
				p++;
				lx->pos++;
				str_len++;
			}
		}
		if (*p == '"') {
			lx->pos++;
		}
		lx->peek = mk_tok(TOK_STR, str_start, str_len, lx->line);
		return;
	}

	/* comments */
	if (*p == '/' && p[1] == '/') {
		while (*p && *p != '\n') {
			p++;
			lx->pos++;
		}
		lexer_next(lx);
		return;
	}

	/* multi line comments */
	if (*p == '/' && p[1] == '*') {
		lx->pos += 2;
		p += 2;
		while (*p) {
			if (*p == '*' && p[1] == '/') {
				lx->pos += 2;
				p += 2;
				break;
			}
			if (*p == '\n') lx->line++;
			p++;
			lx->pos++;
		}
		lexer_next(lx);
		return;
	}

	/* two char operators */
	if (*p == '=' && p[1] == '=') { lx->pos += 2; lx->peek = mk_tok(TOK_EQ, lx->src + start, 2, lx->line); return; }
	if (*p == '!' && p[1] == '=') { lx->pos += 2; lx->peek = mk_tok(TOK_NE, lx->src + start, 2, lx->line); return; }
	if (*p == '<' && p[1] == '=') { lx->pos += 2; lx->peek = mk_tok(TOK_LE, lx->src + start, 2, lx->line); return; }
	if (*p == '>' && p[1] == '=') { lx->pos += 2; lx->peek = mk_tok(TOK_GE, lx->src + start, 2, lx->line); return; }
	if (*p == '=' && p[1] == '>') { lx->pos += 2; lx->peek = mk_tok(TOK_ARROW, lx->src + start, 2, lx->line); return; }
	if (*p == '&' && p[1] == '&') { lx->pos += 2; lx->peek = mk_tok(TOK_AND, lx->src + start, 2, lx->line); return; }
	if (*p == '|' && p[1] == '|') { lx->pos += 2; lx->peek = mk_tok(TOK_OR, lx->src + start, 2, lx->line); return; }

	/* single char tokens */
	char c = *p;
	lx->pos++;
	Token t = mk_tok(TOK_EOF, p, 1, lx->line);
	switch (c) {
		case '(': t.kind = TOK_LPAREN; break;
		case ')': t.kind = TOK_RPAREN; break;
		case '{': t.kind = TOK_LBRACE; break;
		case '}': t.kind = TOK_RBRACE; break;
		case '[': t.kind = TOK_LBRACKET; break;
		case ']': t.kind = TOK_RBRACKET; break;
		case ',': t.kind = TOK_COMMA; break;
		case '.': t.kind = TOK_DOT; break;
		case ';': t.kind = TOK_SEMI; break;
		case ':': t.kind = TOK_COLON; break;
		case '=': t.kind = TOK_ASSIGN; break;
		case '+': t.kind = TOK_PLUS; break;
		case '-': t.kind = TOK_MINUS; break;
		case '*': t.kind = TOK_STAR; break;
		case '/': t.kind = TOK_SLASH; break;
		case '%': t.kind = TOK_PERCENT; break;
		case '<': t.kind = TOK_LT; break;
		case '>': t.kind = TOK_GT; break;
		case '!': t.kind = TOK_NOT; break;
		default: t.kind = TOK_EOF; break;
	}
	lx->peek = t;
}

const char *token_name(TokenKind kind) {
	switch (kind) {
		case TOK_EOF: return "EOF";
		case TOK_ID: return "id";
		case TOK_NUM: return "num";
		case TOK_STR: return "str";
		case TOK_FN: return "fn";
		case TOK_END: return "end";
		case TOK_RETURN: return "return";
		case TOK_IF: return "if";
		case TOK_ELSE: return "else";
		case TOK_FOR: return "for";
		case TOK_WHILE: return "while";
		case TOK_VAR: return "var";
		case TOK_CONST: return "const";
		case TOK_INT: return "int";
		case TOK_FLOAT: return "float";
		case TOK_STR_KW: return "str";
		case TOK_BOOL: return "bool";
		case TOK_VOID: return "void";
		case TOK_TRUE: return "true";
		case TOK_FALSE: return "false";
		case TOK_LPAREN: return "(";
		case TOK_RPAREN: return ")";
		case TOK_LBRACE: return "{";
		case TOK_RBRACE: return "}";
		case TOK_LBRACKET: return "[";
		case TOK_RBRACKET: return "]";
		case TOK_COMMA: return ",";
		case TOK_DOT: return ".";
		case TOK_SEMI: return ";";
		case TOK_COLON: return ":";
		case TOK_ASSIGN: return "=";
		case TOK_PLUS: return "+";
		case TOK_MINUS: return "-";
		case TOK_STAR: return "*";
		case TOK_SLASH: return "/";
		case TOK_PERCENT: return "%";
		case TOK_LT: return "<";
		case TOK_GT: return ">";
		case TOK_LE: return "<=";
		case TOK_GE: return ">=";
		case TOK_EQ: return "==";
		case TOK_NE: return "!=";
		case TOK_AND: return "&&";
		case TOK_OR: return "||";
		case TOK_NOT: return "!";
		case TOK_ARROW: return "->";
	}
	return "?";
}