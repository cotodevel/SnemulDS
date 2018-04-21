#ifndef __xenofunzip_h__
#define __xenofunzip_h__

#include "typedefsTGDS.h"
#include "dsregs.h"

#include <sys/reent.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <_ansi.h>
#include <reent.h>
#include <sys/lock.h>
#include <fcntl.h>
#include <stdlib.h>

#include "fsfatlayerTGDSLegacy.h"

#define min(a,b) ((a)<(b)?(a):(b))
#define err(e) {clrscr();printf("%s",(char*)e);while(1);}

//128K Buffer: speedup .zip operation
#define BUFFER_SIZE (sint32)(1024 * 128)

/* PKZIP header definitions */
#define ZIPMAG 0x4b50           /* two-byte zip lead-in */
#define LOCREM 0x0403           /* remaining two bytes in zip signature */
#define LOCSIG 0x04034b50L      /* full signature */
#define LOCFLG 4                /* offset of bit flag */
#define  CRPFLG 1               /*  bit for encrypted entry */
#define  EXTFLG 8               /*  bit for extended local header */
#define LOCHOW 6                /* offset of compression method */
#define LOCTIM 8                /* file mod time (for decryption) */
#define LOCCRC 12               /* offset of crc */
#define LOCSIZ 16               /* offset of compressed size */
#define LOCLEN 20               /* offset of uncompressed length */
#define LOCFIL 24               /* offset of file name field length */
#define LOCEXT 26               /* offset of extra field length */
#define LOCHDR 28               /* size of local header, including LOCREM */
#define EXTHDR 16               /* size of extended local header, inc sig */

/* GZIP header definitions */
#define GZPMAG 0x8b1f           /* two-byte gzip lead-in */
#define GZPHOW 0                /* offset of method number */
#define GZPFLG 1                /* offset of gzip flags */
#define  GZPMUL 2               /* bit for multiple-part gzip file */
#define  GZPISX 4               /* bit for extra field present */
#define  GZPISF 8               /* bit for filename present */
#define  GZPISC 16              /* bit for comment present */
#define  GZPISE 32              /* bit for encryption */
#define GZPTIM 2                /* offset of Unix file modification time */
#define GZPEXF 6                /* offset of extra flags */
#define GZPCOS 7                /* offset of operating system compressed on */
#define GZPHDR 8                /* length of minimal gzip header */

#define STORED            0    /* compression methods */
#define SHRUNK            1
#define REDUCED1          2
#define REDUCED2          3
#define REDUCED3          4
#define REDUCED4          5
#define IMPLODED          6
#define TOKENIZED         7
#define DEFLATED          8
#define ENHDEFLATED       9
#define DCLIMPLODED      10
#define BZIPPED          12
#define LZMAED           14
#define IBMTERSED        18
#define IBMLZ77ED        19
#define WAVPACKED        97
#define PPMDED           98
#define NUM_METHODS      17     /* number of known method IDs */

/////

typedef unsigned int   ulg;
typedef unsigned short ush;
typedef unsigned char  uch;

#define SH(p) ((ush)(uch)((p)[0]) | ((ush)(uch)((p)[1]) << 8))
#define LG(p) ((ulg)(SH(p)) | ((ulg)(SH((p)+2)) << 16))


#endif


#ifdef __cplusplus
extern "C"{
#endif

//wrappers for memory blocks required by xenofunzip library
extern void xfree(void *opaque, void *address);
extern void* xcalloc(void *opaque, unsigned items, unsigned size);
extern int funzipstdio(FILE *in, FILE *out);
extern int do_decompression(char *inname, char *outname);
extern int do_decompression_ewramstack(char *inname, char *outname);	//coto: use this to use EWRAM as giant stack by default for decompressing giant .zip / .gz files 

#ifdef __cplusplus
}
#endif

