#include "xyra.h"
#include "ast.h"
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>

static char *read_file(const char *path) {
	FILE *f = fopen(path, "rb");
	if (!f) return NULL;
	fseek(f, 0, SEEK_END);
	long len = ftell(f);
	fseek(f, 0, SEEK_SET);
	char *buf = malloc(len + 1);
	if (!buf) { fclose(f); return NULL; }
	fread(buf, 1, len, f);
	buf[len] = '\0';
	fclose(f);
	return buf;
}

int main(int argc, char **argv) {
	if (argc < 2) {
		fprintf(stderr, "usage: xyra <file.xyra> [-o out]\n");
		return 1;
	}

	char *src = read_file(argv[1]);
	if (!src) {
		fprintf(stderr, "xyra: cannot read %s\n", argv[1]);
		return 1;
	}

	/* parse the source */
	Parser p;
	AST *tree = parse(&p, src);

	if (p.errors > 0) {
		fprintf(stderr, "xyra: %d parse errors\n", p.errors);
		return 1;
	}

	/* print top level functions */
	for (int i = 0; i < tree->count; i++) {
		ASTNode *fn = tree->items[i];
		if (fn->kind == AST_FN) {
			printf("fn %s(%d params) -> body\n", fn->fn.name, fn->fn.nparams);
		}
	}

	return 0;
}
