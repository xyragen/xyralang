#ifndef XYRA_AST_H
#define XYRA_AST_H

#include "xyra.h"

/* node types in the ast */
typedef enum {
	AST_INT,
	AST_FLOAT,
	AST_STR,
	AST_BOOL,
	AST_VAR,
	AST_BINOP,
	AST_UNARY,
	AST_CALL,
	AST_BLOCK,
	AST_FN,
	AST_FN_CALL,
	AST_IF,
	AST_FOR,
	AST_WHILE,
	AST_RETURN,
	AST_ASSIGN,
	AST_MEMBER,
} ASTKind;

/* type info for type checking */
typedef enum {
	TYPE_NONE,
	TYPE_INT,
	TYPE_FLOAT,
	TYPE_STR,
	TYPE_BOOL,
	TYPE_VOID,
	TYPE_FN,
} TypeKind;

/* forward decl */
typedef struct ASTNode ASTNode;

struct ASTNode {
	ASTKind kind;
	TypeKind type;
	Token tok;
	/* union of all node data */
	union {
		long ival;
		double fval;
		const char *sval;
		int bval;
		struct { ASTNode *left; ASTNode *right; TokenKind op; } binop;
		struct { TokenKind op; ASTNode *operand; } unary;
		struct { const char *name; ASTNode **args; int nargs; } call;
		struct { ASTNode **stmts; int nstmts; } block;
		struct { const char *name; const char **params; int nparams; TypeKind ret_type; ASTNode *body; } fn;
		struct { ASTNode *cond; ASTNode *then_block; ASTNode *else_block; } ifstmt;
		struct { ASTNode *init; ASTNode *cond; ASTNode *step; ASTNode *body; } forstmt;
		struct { ASTNode *cond; ASTNode *body; } whilestmt;
		struct { ASTNode *expr; } ret;
		struct { const char *name; ASTNode *val; } assign;
	};
	/* child nodes for any traversal */
	ASTNode *next;
};

/* create a new ast node */
ASTNode *ast_new(ASTKind kind, Token tok);

/* a program is just a list of top level statements */
typedef struct {
	ASTNode **items;
	int count;
	int cap;
} AST;

void ast_init(AST *tree);
void ast_push(AST *tree, ASTNode *node);

#endif