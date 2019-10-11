/* (C) 2019 rofl0r */
/* released into the public domain, or at your choice 0BSD or WTFPL */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static int usage() {
	printf( "form2hdr - converts a form file into a C string\n"
		"the output is written to stdout.\n"
		"usage: form2hdr filename\n");
	return 1;
}

static int isprintable(int ch) {
#define PRINTABLE " ;,:.^-+=*/%|&[](){}<>#_"
	return isdigit(ch) || isalpha(ch) || strchr(PRINTABLE, ch);
}

int main(int argc, char** argv) {
	if(argc != 2) return usage();
	int f_arg = 1;
	unsigned cpl = 77;
	FILE *f = fopen(argv[f_arg], "r");
	if(!f) { perror("fopen"); return 1; }
	unsigned char buf[1024];
	int found = 0;
	while(fgets(buf, sizeof buf, f)) {
		if(strstr(buf, "START_INCLUDE")) {
			found = 1;
			break;
		}
	}
	if(!found) {
		fprintf(stderr, "error: form start marker %s not found!\n", "START_INCLUDE");
		return 1;
	}
	{
		char *p = strrchr(argv[f_arg], '/');
		if(!p) p = argv[f_arg];
		else p++;
		printf("const char %s[] =\n", p);
	}

	int ch;
	unsigned cnt = 0;
	while((ch = fgetc(f)) != EOF) {
		if(cnt == 0) printf("\"");
		if(ch == '\n') { printf("\\n"); cnt++; }
		else if (ch == '\t') { printf("\\t"); cnt++; }
		else if (ch == '\"') { printf("\\\""); cnt++; }
		else if (ch == '\'') { printf("\\'"); cnt++; }
		else if(isprintable(ch)) printf("%c", ch);
		else {
			printf("\\x%02X\"\"", ch);
			cnt += 5;
		}
		cnt++;
		if(cnt >= cpl) {
			printf("\"\n");
			cnt = 0;
		}
	}
	if(cnt) printf("\"\n");
	printf(";\n");
	fclose(f);
	return 0;
}

