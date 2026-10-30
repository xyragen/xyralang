#ifndef XYRA_PARSER_H
#define XYRA_PARSER_H

#include "xyra.h"
#include "ast.h"

typedef struct {
	Lexer *lx;
	Token cur;
	int errors;
} Parser;

/* parse a source string into an AST */
AST *parse(Parser *p, const char *src);

/* free the AST when done */
void ast_free(AST *tree);

#endif