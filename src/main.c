#include "xyra.h"
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

	/* lex the source */
	Lexer lx;
	lexer_init(&lx, src);

	/* dump tokens for now */
	while (lx.cur.kind != TOK_EOF) {
		Token *t = &lx.cur;
		if (t->kind == TOK_NUM) {
			printf("num(%ld)\n", t->num_val);
		} else if (t->kind == TOK_ID) {
			printf("id(%.*s)\n", t->len, t->start);
		} else {
			printf("%s\n", token_name(t->kind));
		}
		lexer_next(&lx);
	}

	return 0;
}
