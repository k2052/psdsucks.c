#ifndef _PSDSUCKS_H_
#define _PSDSUCKS_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <fcntl.h>

#include <stdint.h>
#include <sys/types.h>
#include <wchar.h>
#include <ctype.h>
#include <errno.h>
#include <sys/stat.h>

#include <getopt.h>

#ifdef HAVE_UNISTD_H
	#include <unistd.h>
#endif

#ifdef HAVE_ICONV_H
	#include <iconv.h>

	extern iconv_t ic;
#endif

#define MKDIR mkdir

#define PSDBSIZE(version) ((version) << 2)

/* Photoshop Mode Constants */
#define SCAVENGE_MODE -1
#define ModeBitmap		 0
#define ModeGrayScale		 1
#define ModeIndexedColor 2
#define ModeRGBColor		 3
#define ModeCMYKColor		 4
#define ModeHSLColor		 5
#define ModeHSBColor		 6
#define ModeMultichannel 7
#define ModeDuotone		 8
#define ModeLabColor		 9
#define ModeGray16		10
#define ModeRGB48			11
#define ModeLab48			12
#define ModeCMYK64		13
#define ModeDeepMultichannel 14
#define ModeDuotone16		15

#define PAD2(x) (((x)+1) & -2) // same or next even
#define PAD4(x) (((x)+3) & -4) // same or next multiple of 4
#define PAD_BYTE 0

#define VERBOSE if(verbose) printf
#define UNQUIET if(!quiet) printf

#define FIXEDPT(x) ((x)/65536.)

#define KEYMATCH(p, str) (!memcmp(p, str, strlen(str)))

#define TRANS_CHAN_ID (-1)
#define LMASK_CHAN_ID (-2)
#define UMASK_CHAN_ID (-3)

typedef FILE *psd_file_t;
typedef uint32_t psd_bytes_t;
typedef uint32_t psd_pixels_t;

struct psd_header
{
	char sig[4];
	unsigned short version;
	char reserved[6];
	unsigned short channels;
	uint32_t rows;
	uint32_t cols;
	unsigned short depth;
	short mode;

	psd_bytes_t colormodepos; 
	psd_bytes_t resourcepos;  
	int nlayers, mergedalpha; 
	struct layer_info *linfo;     
	psd_bytes_t lmistart, lmilen; 
	psd_bytes_t layerdatapos; 
	psd_bytes_t global_lmi_pos, global_lmi_len;
	struct channel_info *merged_chans;
};

struct layer_mask_info
{
	psd_bytes_t size; 

	// if size == 20
	int32_t top;
	int32_t left;
	int32_t bottom;
	int32_t right;
	char default_colour;
	char flags;

	// if size == 36:
	char real_flags;
	char real_default_colour;
	int32_t real_top;
	int32_t real_left;
	int32_t real_bottom;
	int32_t real_right;
};

struct blend_mode_info
{
	char sig[4];
	char key[4];
	unsigned char opacity;
	unsigned char clipping;
	unsigned char flags;
};

struct channel_info
{
	int id;                  
	int comptype;            
	psd_pixels_t rows, cols, rowbytes; 
	psd_bytes_t length;     

	psd_bytes_t rawpos;       
	psd_bytes_t *rowpos;      
};

struct layer_info
{
	int32_t top;
	int32_t left;
	int32_t bottom;
	int32_t right;
	short channels;

	struct channel_info *chan;
	int *chindex;    
	struct blend_mode_info blend;
	struct layer_mask_info mask;
	char *name;
	char *unicode_name;
	char *nameno; 

	psd_bytes_t additionalpos;
	psd_bytes_t additionallen;
};

struct dictentry
{
	int id;
	char *key, *tag, *desc;
	void (*func)(psd_file_t f, int level, int len, struct dictentry *dict);
};

struct colour_space
{
	const unsigned short id; 
	const char *name, *components;  
};

/**
 * Constants.c:
 */

extern const char *channelsuffixes[], *mode_names[];
extern struct colour_space colour_spaces[];
extern const int mode_channel_count[];
extern const int mode_colour_space[];

extern char dirsep[], *pngdir;
extern int verbose, quiet, rsrc, print_rsrc, resdump, extra, makedirs,
		   numbered, help, split, nwarns, writelist,
		   writexml, unicode_filenames,
		   rebuild, rebuild_v1, merged_only;

extern FILE *listfile;

/**
 * Error handling functions
 */

void fatal(char *s);
void warn_msg(char *fmt, ...);
void alwayswarn(char *fmt, ...);

void *ckmalloc(size_t n, char *file, int line);

/**
 * Get bytes & turn into data.
 */

char    *getpstr(psd_file_t f);
char    *getpstr2(psd_file_t f);
char    *getkey(psd_file_t f);
double   getdoubleB(psd_file_t f);
int32_t  get4B(psd_file_t f);
int64_t  get8B(psd_file_t f);
int      get2B(psd_file_t f);
unsigned get2Bu(psd_file_t f);

/**
 * Put bytes
 */

unsigned put4B(psd_file_t f, int32_t);
unsigned put8B(psd_file_t f, int64_t);
unsigned putpsdbytes(psd_file_t f, int version, uint64_t value);
unsigned put2B(psd_file_t f, int);

/**
 * Peek at bytes. Everyones a pervert.
 */

int32_t peek4B(unsigned char *p);
int64_t peek8B(unsigned char *p);
int     peek2B(unsigned char *p);

void openfiles(char *psdpath, struct psd_header *h);

/**
 * Do Stuff
 */
int dopsd(psd_file_t f, char *fname, struct psd_header *h);
