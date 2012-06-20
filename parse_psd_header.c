#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <fcntl.h>

struct psd_header {
	char sig[4];
	unsigned short version;
	unsigned short depth;
};

typedef FILE *psd_file_t;

unsigned get2Bu(psd_file_t f){
	unsigned n = fgetc(f)<<8;
	return n |= fgetc(f);
}

int dopsd(psd_file_t f, struct psd_header *h)
{
	int result = 0;
	fread(h->sig, 1, 4, f);
	h->version = get2Bu(f);

  printf("sig %s, version %u \n\n", h->sig, h->version);

	return result;
}

int main()
{
  FILE *file;
	struct psd_header h;

	file = fopen("bob.psd", "rb");
	int result;
	result = dopsd(file, &h);
	return 0;
}