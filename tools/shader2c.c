#include <stdio.h>
#include <stdlib.h>

void convert_single_line(FILE *in, FILE *out) {
	fputs("\"", out);

	for(;;) {
		int next = fgetc(in);
		if(next == EOF) break;
		else if(next == '\n') break;
		else if(next == '\r') continue;
		else if(next == '"') fputs("\\\"", out);
		else if(next == '\\') fputs("\\\\", out);
		else {
			fputc(next, out);
		}
	}

	fputs("\\n\"\n", out);
}

int main(int argc, char **argv) {
	if(argc != 3) {
		printf("usage: %s <input-file> <output-file>\n", argv[0]);
		return -1;
	}

	FILE *in = fopen(argv[1], "r");
	if(!in) {
		printf("failed to open input %s\n", argv[1]);
		return -2;
	}

	FILE *out = fopen(argv[2], "w");
	if(!out) {
		printf("failed to open output %s\n", argv[2]);
		return -3;
	}

	
	char *rewrite = argv[1]; // Rewrite the input file name for the shader variable name
	while(*rewrite) {
		if(*rewrite == '/') *rewrite = '_';
		if(*rewrite == '.') *rewrite = '_';

		++rewrite;
	}

	fprintf(out, "const char *%s = ", argv[1]);

	while(!feof(in)) {
		convert_single_line(in, out);
	}

	fputs(";", out);
	
	fclose(out);
	fclose(in);
}