#include<stdio.h>
#include<stdlib.h>

void write_pnm(char* data, int width, int height, FILE *fp) {
	int ret1, ret2;
	ret1 = fprintf(fp, "P6\n%d %d\n255\n", width, height);
	ret2 = fwrite(data, 1, width*height*3, fp);
	if(!(ret1 > 0 && ret2 == width*height*3)) {
		fprintf(stderr, "Fatal: write_pnm: Writing to file failed.\n");
		exit(1);
	}
}

