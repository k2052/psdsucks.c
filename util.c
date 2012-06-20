#include <stdarg.h>
#include <ctype.h>
#include <errno.h>

#include "psdparse.h"

/**
 * Error Handling
 */
#define WARNLIMIT 10

#ifdef HAVE_ICONV_H
	iconv_t ic = (iconv_t)-1;
#endif

void fatal(char *s)
{
	fflush(stdout);
	fputs(s, stderr);
	exit(EXIT_FAILURE);
}

int nwarns = 0;

void warn_msg(char *fmt, ...)
{
	char s[0x200];
	va_list v;

	if(nwarns == WARNLIMIT) fputs("#   (further warnings suppressed)\n", stderr);

	++nwarns;
	if(nwarns <= WARNLIMIT)
	{
		va_start(v, fmt);
		vsnprintf(s, 0x200, fmt, v);
		va_end(v);
		fflush(stdout);
		fprintf(stderr, "#   warning: %s\n", s);
	}
}

void alwayswarn(char *fmt, ...)
{
	char s[0x200];
	va_list v;

	va_start(v, fmt);
	vsnprintf(s, 0x200, fmt, v);
	va_end(v);
	fflush(stdout);
	fputs(s, stderr);
}

void *ckmalloc(size_t n, char *file, int line)
{
	void *p = malloc(n);
	if(p
		return p;
	else {
		fprintf(stderr, "can't get %ld bytes @ %s:%d\n", n, file, line);
		exit(1);
	}

	return NULL;
}

static int platform_is_LittleEndian()
{
	union{ int a; char b; } u;
	u.a = 1;
	
	return u.b;
}

int hexdigit(unsigned char c)
{
	c = toupper(c);
	return c - (c >= 'A' ? 'A'-10 : '0');
}

/*
 * Get bytes & turn to data
 */
char *getpstr(psd_file_t f)
{
	static char pstr[0x100];
	int len = fgetc(f);

	if(len != EOF) {
		fread(pstr, 1, len, f);
		pstr[len] = 0;
	}
	else
		pstr[0] = 0;

	return pstr;
}

// Padded Pascal string
char *getpstr2(psd_file_t f)
{
	static char pstr[0x100];
	int len = fgetc(f);

	if(len != EOF) 
	{
		fread(pstr, 1, len, f);
		pstr[len] = 0;
		if(!(len & 1))
			fgetc(f); // skip padding
	}
	else
		pstr[0] = 0;

	return pstr;
}

char *getkey(psd_file_t f)
{
	static char k[5];

	if(fread(k, 1, 4, f) == 4)
		k[4] = 0;
	else
		k[0] = 0;

	return k;
}

double getdoubleB(psd_file_t f)
{
	union {
		double d;
		unsigned char c[8];
	} u, urev;

	if(fread(u.c, 1, 8, f) == 8)
	{
		if(platform_is_LittleEndian())
		{
			urev.c[0] = u.c[7];
			urev.c[1] = u.c[6];
			urev.c[2] = u.c[5];
			urev.c[3] = u.c[4];
			urev.c[4] = u.c[3];
			urev.c[5] = u.c[2];
			urev.c[6] = u.c[1];
			urev.c[7] = u.c[0];

			return urev.d;
		}
		else
			return u.d;
	}

	return 0;
}

int32_t get4B(psd_file_t f)
{
	long n = fgetc(f) << 24;
	n |= fgetc(f) << 16;
	n |= fgetc(f) << 8;

	return n | fgetc(f);
}

int64_t get8B(psd_file_t f)
{
	int64_t msl = (unsigned long) get4B(f);
	return (msl << 32) | (unsigned long) get4B(f);
}

int get2B(psd_file_t f)
{
	unsigned n = fgetc(f) << 8;
	n |= fgetc(f);
	return n < 0x8000 ? n : n - 0x10000;
}

unsigned get2Bu(psd_file_t f)
{
	unsigned n = fgetc(f)<<8;
	return n |= fgetc(f);
}

/**
 * Put bytes
 */

unsigned put4B(psd_file_t f, int32_t value)
{
	return fputc(value >> 24, f) != EOF
		&& fputc(value >> 16, f) != EOF
		&& fputc(value >>  8, f) != EOF
		&& fputc(value, f) != EOF;
}

unsigned put8B(psd_file_t f, int64_t value)
{
	return put4B(f, value >> 32) && put4B(f, value);
}

unsigned putpsdbytes(psd_file_t f, int version, uint64_t value)
{
	if(version == 1 && value > UINT32_MAX)
		fatal("");

	return version == 1 ? put4B(f, value) : put8B(f, value);
}

unsigned put2B(psd_file_t f, int value)
{
	return fputc(value >> 8, f) != EOF
		&& fputc(value, f) != EOF;
}

/**
 * Peek at bytes
 */

int32_t peek4B(unsigned char *p)
{
	return (p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3];
}

int64_t peek8B(unsigned char *p)
{
	int64_t msl = (unsigned long) peek4B(p);
	return (msl << 32) | (unsigned long) peek4B(p+4);
}

int peek2B(unsigned char *p)
{
	unsigned n = (p[0] << 8) | p[1];
	return n < 0x8000 ? n : n - 0x10000;
}

unsigned peek2Bu(unsigned char *p)
{
	return (p[0] << 8) | p[1];
}