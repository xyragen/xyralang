#include "parser.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static void parser_init(Parser *p, Lexer *lx) {
	p->lx = lx;
	p->cur = lx->cur;
	p->errors = 0;
}

static void parser_advance(Parser *p) {
	lexer_next(p->lx);
	p->cur = p->lx->cur;
}

static int parser_match(Parser *p, TokenKind kind) {
	if (p->cur.kind == kind) {
		parser_advance(p);
		return 1;
	}
	return 0;
}

static int parser_check(Parser *p, TokenKind kind) {
	return p->cur.kind == kind;
}

static void parser_expect(Parser *p, TokenKind kind) {
	if (p->cur.kind == kind) {
		parser_advance(p);
	} else {
		fprintf(stderr, "parse error line %d: expected %s got %s\n",
			p->cur.line, token_name(kind), token_name(p->cur.kind));
		p->errors++;
		parser_advance(p);
	}
}

/* forward decls */
static ASTNode *parse_block(Parser *p);
static ASTNode *parse_expr(Parser *p);
static ASTNode *parse_stmt(Parser *p);

/* expression parsing with precedence climbing */
static int get_prec(TokenKind kind) {
	switch (kind) {
		case TOK_PLUS: case TOK_MINUS: return 1;
		case TOK_STAR: case TOK_SLASH: case TOK_PERCENT: return 2;
		case TOK_LT: case TOK_GT: case TOK_LE: case TOK_GE: return 3;
		case TOK_EQ: case TOK_NE: return 4;
		case TOK_AND: return 5;
		case TOK_OR: return 6;
		default: break;
	}
	return 0;
}

static ASTNode *parse_primary(Parser *p) {
	Token t = p->cur;
	if (parser_match(p, TOK_NUM)) {
		ASTNode *n = ast_new(AST_INT, t);
		n->ival = t.num_val;
		return n;
	}
	if (parser_match(p, TOK_TRUE)) {
		ASTNode *n = ast_new(AST_BOOL, t);
		n->bval = 1;
		return n;
	}
	if (parser_match(p, TOK_FALSE)) {
		ASTNode *n = ast_new(AST_BOOL, t);
		n->bval = 0;
		return n;
	}
	if (parser_match(p, TOK_STR)) {
		ASTNode *n = ast_new(AST_STR, t);
		n->sval = strndup(t.start, t.len);
		return n;
	}
	if (parser_match(p, TOK_ID)) {
		ASTNode *n = ast_new(AST_VAR, t);
		n->sval = strndup(t.start, t.len);
		return n;
	}
	if (parser_match(p, TOK_LPAREN)) {
		ASTNode *inner = parse_expr(p);
		parser_expect(p, TOK_RPAREN);
		return inner;
	}
	fprintf(stderr, "parse error line %d: expected expression got %s\n",
		t.line, token_name(t.kind));
	p->errors++;
	parser_advance(p);
	return NULL;
}

static ASTNode *parse_unary(Parser *p) {
	if (p->cur.kind == TOK_MINUS) {
		Token t = p->cur;
		parser_advance(p);
		ASTNode *operand = parse_unary(p);
		ASTNode *n = ast_new(AST_UNARY, t);
		n->unary.op = t.kind;
		n->unary.operand = operand;
		return n;
	}
	if (p->cur.kind == TOK_NOT) {
		Token t = p->cur;
		parser_advance(p);
		ASTNode *operand = parse_unary(p);
		ASTNode *n = ast_new(AST_UNARY, t);
		n->unary.op = t.kind;
		n->unary.operand = operand;
		return n;
	}
	return parse_primary(p);
}

static ASTNode *parse_binop(Parser *p, int prec) {
	ASTNode *left = parse_unary(p);
	while (get_prec(p->cur.kind) >= prec && get_prec(p->cur.kind) > 0) {
		Token op = p->cur;
		parser_advance(p);
		ASTNode *right = parse_binop(p, get_prec(op.kind) + 1);
		ASTNode *n = ast_new(AST_BINOP, op);
		n->binop.left = left;
		n->binop.right = right;
		n->binop.op = op.kind;
		left = n;
	}
	return left;
}

static ASTNode *parse_expr(Parser *p) {
	return parse_binop(p, 1);
}

static ASTNode *parse_stmt(Parser *p) {
	Token t = p->cur;

	if (parser_match(p, TOK_VAR) || parser_match(p, TOK_CONST)) {
		parser_expect(p, TOK_ID);
		parser_expect(p, TOK_ASSIGN);
		ASTNode *val = parse_expr(p);
		ASTNode *n = ast_new(AST_ASSIGN, t);
		n->assign.name = strndup(t.start, t.len);
		n->assign.val = val;
		return n;
	}

	if (parser_match(p, TOK_IF)) {
		parser_expect(p, TOK_LPAREN);
		ASTNode *cond = parse_expr(p);
		parser_expect(p, TOK_RPAREN);
		ASTNode *body = parse_block(p);
		ASTNode *n = ast_new(AST_IF, t);
		n->ifstmt.cond = cond;
		n->ifstmt.then_block = body;
		if (parser_match(p, TOK_ELSE)) {
			n->ifstmt.else_block = parse_block(p);
		}
		return n;
	}

	if (parser_match(p, TOK_WHILE)) {
		parser_expect(p, TOK_LPAREN);
		ASTNode *cond = parse_expr(p);
		parser_expect(p, TOK_RPAREN);
		ASTNode *body = parse_block(p);
		ASTNode *n = ast_new(AST_WHILE, t);
		n->whilestmt.cond = cond;
		n->whilestmt.body = body;
		return n;
	}

	if (parser_match(p, TOK_FOR)) {
		parser_expect(p, TOK_LPAREN);
		ASTNode *init = NULL;
		if (!parser_check(p, TOK_SEMI)) {
			init = parse_stmt(p);
		}
		parser_expect(p, TOK_SEMI);
		ASTNode *cond = NULL;
		if (!parser_check(p, TOK_SEMI)) {
			cond = parse_expr(p);
		}
		parser_expect(p, TOK_SEMI);
		ASTNode *step = NULL;
		if (!parser_check(p, TOK_RPAREN)) {
			step = parse_expr(p);
		}
		parser_expect(p, TOK_RPAREN);
		ASTNode *body = parse_block(p);
		ASTNode *n = ast_new(AST_FOR, t);
		n->forstmt.init = init;
		n->forstmt.cond = cond;
		n->forstmt.step = step;
		n->forstmt.body = body;
		return n;
	}

	if (parser_match(p, TOK_RETURN)) {
		ASTNode *val = NULL;
		if (!parser_check(p, TOK_END)) {
			val = parse_expr(p);
		}
		ASTNode *n = ast_new(AST_RETURN, t);
		n->ret.expr = val;
		return n;
	}

	/* expression statement */
	return parse_expr(p);
}

static ASTNode *parse_block(Parser *p) {
	parser_expect(p, TOK_LBRACE);
	ASTNode *n = ast_new(AST_BLOCK, p->cur);
	n->block.stmts = NULL;
	n->block.nstmts = 0;
	int cap = 0;

	while (!parser_check(p, TOK_RBRACE) && !parser_check(p, TOK_EOF)) {
		if (parser_check(p, TOK_END)) break;
		ASTNode *stmt = parse_stmt(p);
		if (n->block.nstmts >= cap) {
			cap = cap ? cap * 2 : 4;
			n->block.stmts = realloc(n->block.stmts, cap * sizeof(ASTNode*));
		}
		n->block.stmts[n->block.nstmts++] = stmt;
	}
	parser_expect(p, TOK_RBRACE);
	return n;
}

static ASTNode *parse_fn(Parser *p) {
	Token t = p->cur;
	const char *name = NULL;
	parser_match(p, TOK_FN);
	
	/* the id token is cur before we consume it */
	Token name_tok = p->cur;
	parser_expect(p, TOK_ID);
	name = strndup(name_tok.start, name_tok.len);

	parser_expect(p, TOK_LPAREN);
	const char **params = NULL;
	int nparams = 0;
	int cap = 0;
	while (!parser_check(p, TOK_RPAREN) && !parser_check(p, TOK_EOF)) {
		parser_match(p, TOK_ID);
		if (nparams >= cap) {
			cap = cap ? cap * 2 : 4;
			params = realloc(params, cap * sizeof(const char*));
		}
		params[nparams++] = strndup(p->cur.start, p->cur.len);
		parser_advance(p);
		if (parser_check(p, TOK_COMMA)) parser_advance(p);
	}
	parser_expect(p, TOK_RPAREN);

	TypeKind ret = TYPE_NONE;
	if (parser_match(p, TOK_ARROW)) {
		if (parser_match(p, TOK_VOID)) ret = TYPE_VOID;
		else if (parser_match(p, TOK_INT)) ret = TYPE_INT;
		else if (parser_match(p, TOK_FLOAT)) ret = TYPE_FLOAT;
		else if (parser_match(p, TOK_STR_KW)) ret = TYPE_STR;
		else if (parser_match(p, TOK_BOOL)) ret = TYPE_BOOL;
	}

	ASTNode *body = parse_block(p);
	parser_expect(p, TOK_END);

	ASTNode *n = ast_new(AST_FN, t);
	n->fn.name = name;
	n->fn.params = params;
	n->fn.nparams = nparams;
	n->fn.ret_type = ret;
	n->fn.body = body;
	return n;
}

AST *parse(Parser *p, const char *src) {
	Lexer lx;
	lexer_init(&lx, src);
	parser_init(p, &lx);

	AST *tree = malloc(sizeof(AST));
	ast_init(tree);

	while (!parser_check(p, TOK_EOF)) {
		ASTNode *n = parse_fn(p);
		ast_push(tree, n);
	}

	return tree;
}

void ast_free(AST *tree) {
	if (!tree) return;
	for (int i = 0; i < tree->count; i++) {
		/* recursively free nodes */
	}
	free(tree->items);
	free(tree);
}