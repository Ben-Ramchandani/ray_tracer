#include<stdio.h>
#include<stdlib.h>

#define WIDTH 2
#define HEIGHT 2

void write_pnm(char* data, int width, int height, FILE *fp) {
	int ret1, ret2;
	ret1 = fprintf(fp, "P6\n%d %d\n255\n", width, height);
	ret2 = fwrite(data, 1, width*height*3, fp);
	if(!(ret1 > 0 && ret2 == width*height*3)) {
		fprintf(stderr, "Fatal: write_pnm: Writing to file failed.\n");
		exit(1);
	}
}
/*
int main() {
	char data[WIDTH][HEIGHT][3] = {{{255, 0, 0}, {255, 0, 0}}, {{0, 255, 0}, {0, 0, 255}}};
	write_pnm((char *) data, WIDTH, HEIGHT, stdout);
	return 0;
}
*/

