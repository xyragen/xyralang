#include "ast.h"
#include <stdlib.h>

ASTNode *ast_new(ASTKind kind, Token tok) {
	ASTNode *n = malloc(sizeof(ASTNode));
	if (!n) return NULL;
	n->kind = kind;
	n->type = TYPE_NONE;
	n->tok = tok;
	n->next = NULL;
	return n;
}

void ast_init(AST *tree) {
	tree->items = NULL;
	tree->count = 0;
	tree->cap = 0;
}

void ast_push(AST *tree, ASTNode *node) {
	if (tree->count >= tree->cap) {
		tree->cap = tree->cap ? tree->cap * 2 : 8;
		tree->items = realloc(tree->items, tree->cap * sizeof(ASTNode*));
	}
	tree->items[tree->count++] = node;
}