#ifndef XYRA_H
#define XYRA_H

/* token kinds */
typedef enum {
	TOK_EOF,
	TOK_ID,
	TOK_NUM,
	TOK_STR,
	TOK_FN,
	TOK_END,
	TOK_RETURN,
	TOK_IF,
	TOK_ELSE,
	TOK_FOR,
	TOK_WHILE,
	TOK_VAR,
	TOK_CONST,
	TOK_INT,
	TOK_FLOAT,
	TOK_STR_KW,
	TOK_BOOL,
	TOK_VOID,
	TOK_TRUE,
	TOK_FALSE,
	TOK_LPAREN,
	TOK_RPAREN,
	TOK_LBRACE,
	TOK_RBRACE,
	TOK_LBRACKET,
	TOK_RBRACKET,
	TOK_COMMA,
	TOK_DOT,
	TOK_SEMI,
	TOK_COLON,
	TOK_ASSIGN,
	TOK_PLUS,
	TOK_MINUS,
	TOK_STAR,
	TOK_SLASH,
	TOK_PERCENT,
	TOK_LT,
	TOK_GT,
	TOK_LE,
	TOK_GE,
	TOK_EQ,
	TOK_NE,
	TOK_AND,
	TOK_OR,
	TOK_NOT,
	TOK_ARROW,
}

typedef struct {
    TokenKind kind;
    const char *start;
    int len;
    int line;
    /* values for numbers and strings */
    union {
        long num_val;
        const char *str_val;
    };
} Token;

typedef struct {
    const char *src;
    int pos;
    int line;
    Token cur;
    Token peek;
} Lexer;

void lexer_init(Lexer *lx, const char *src);
void lexer_next(Lexer *lx);
const char *token_name(TokenKind kind);

#endif