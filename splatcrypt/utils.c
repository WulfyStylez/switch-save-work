#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include "utils.h"

#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#endif

u32 crc32(u8 *message, u32 len)
{
    int i, j;
    u32 byte, crc, mask;

    i = 0;
    crc = 0xFFFFFFFF;
    while(len--)
    {
        byte = message[i];
        crc = crc ^ byte;
        for (j = 7; j >= 0; j--)
        {
            mask = -(crc & 1);
            crc = (crc >> 1) ^ (0xEDB88320 & mask);
        }
        i = i + 1;
    }
    return ~crc;
}

void error(char* format, ...)
{
    char buf[0x10000];
    va_list args;
    va_start(args, format);

    vsnprintf(buf, sizeof(buf), format, args);
    va_end(args);

    fprintf(stderr, "\n\n***ERROR!!***\n\n%s\n\n*!!!ERROR!!***\n", buf);
    
    fflush(stderr);
}
char *strtrimr (char *s) {
    int i;
    for (i = strlen (s) - 1; i >= 0 && isspace (s[i]); i--)
        s[i] = '\0';

    return (s);
}
void my_strdup(char*dest, char*src)
{
	if (!(dest = strdup (src))) {
		fprintf (stderr, "strdup() failed\n");
		exit (101);
	}
}
    

u32 align(u32 offset, u32 alignment)
{
	u32 mask = ~(alignment-1);

	return (offset + (alignment-1)) & mask;
}

u64 align64(u64 offset, u32 alignment)
{
	u64 mask = ~(alignment-1);

	return (offset + (alignment-1)) & mask;
}

u64 getle64(const void* p)
{
    u8* p8 = (u8*)p;
	u64 n = p8[0];

	n |= (u64)p8[1]<<8;
	n |= (u64)p8[2]<<16;
	n |= (u64)p8[3]<<24;
	n |= (u64)p8[4]<<32;
	n |= (u64)p8[5]<<40;
	n |= (u64)p8[6]<<48;
	n |= (u64)p8[7]<<56;
	return n;
}

u64 getbe64(const void* p)
{
	u64 n = 0;
    u8* p8 = (u8*)p;

	n |= (u64)p8[0]<<56;
	n |= (u64)p8[1]<<48;
	n |= (u64)p8[2]<<40;
	n |= (u64)p8[3]<<32;
	n |= (u64)p8[4]<<24;
	n |= (u64)p8[5]<<16;
	n |= (u64)p8[6]<<8;
	n |= (u64)p8[7]<<0;
	return n;
}

u32 getle32(const void* p)
{
    u8* p8 = (u8*)p;
	return (p8[0]<<0) | (p8[1]<<8) | (p8[2]<<16) | (p8[3]<<24);
}

u32 getbe32(const void* p)
{
    u8* p8 = (u8*)p;
	return (p8[0]<<24) | (p8[1]<<16) | (p8[2]<<8) | (p8[3]<<0);
}

u32 getle16(const void* p)
{
    u8* p8 = (u8*)p;
	return (p8[0]<<0) | (p8[1]<<8);
}

u32 getbe16(const void* p)
{
    u8* p8 = (u8*)p;
	return (p8[0]<<8) | (p8[1]<<0);
}

#define ENDIAN_BIG 1
#define ENDIAN_LITTLE 2
u32 get_platform_endian()
{
    u32 test = 1;
    if(((char*)&test)[0] == 1)
        return ENDIAN_LITTLE;
    else
        return ENDIAN_BIG;
}

void reverse_copy(u8* dst, u8* src, u32 len)
{
    for(u32 i = 0; i < len; i++)
        dst[i] = src[len-1 - i];
}

// copy a primitive of len length, flipping endian if needed
void be_copy(void* out,  void* in, u32 len)
{
    u8* p8 = (u8*)out;
    u8* n8 = (u8*)in;
    if(get_platform_endian() == ENDIAN_BIG)
        memcpy(p8, n8, len);
    else
        reverse_copy(p8, n8, len);
}

// see be_copy
void le_copy(void* out, void* in, u32 len)
{
    u8* p8 = (u8*)out;
    u8* n8 = (u8*)in;
    if(get_platform_endian() == ENDIAN_LITTLE)
        memcpy(p8, n8, len);
    else
        reverse_copy(p8, n8, len);
}

void putbe16(void* p, u16 n)
{
    be_copy(p, &n, 2);
}

void putbe32(void* p, u32 n)
{
    be_copy(p, &n, 4);
}

void putbe64(void* p, u64 n)
{
    be_copy(p, &n, 8);
}

void putle16(void* p, u16 n)
{
    le_copy(p, &n, 2);
}

void putle32(void* p, u32 n)
{
    le_copy(p, &n, 4);
}

void putle64(void* p, u64 n)
{
    le_copy(p, &n, 8);
}

void hexdump(void *ptr, int buflen)
{
	u8 *buf = (u8*)ptr;
	int i, j;

	for (i=0; i<buflen; i+=16)
	{
		printf("%06x: ", i);
		for (j=0; j<16; j++)
		{ 
			if (i+j < buflen)
			{
				printf("%02x ", buf[i+j]);
			}
			else
			{
				printf("   ");
			}
		}

		printf(" ");

		for (j=0; j<16; j++) 
		{
			if (i+j < buflen)
			{
				printf("%c", (buf[i+j] >= 0x20 && buf[i+j] <= 0x7e) ? buf[i+j] : '.');
			}
		}
		printf("\n");
	}
}

static int ishex(char c)
{
	if (c >= '0' && c <= '9')
		return 1;
	if (c >= 'A' && c <= 'F')
		return 1;
	if (c >= 'a' && c <= 'f')
		return 1;
	return 0;

}

static unsigned char hextobin(char c)
{
	if (c >= '0' && c <= '9')
		return c-'0';
	if (c >= 'A' && c <= 'F')
		return c-'A'+0xA;
	if (c >= 'a' && c <= 'f')
		return c-'a'+0xA;
	return 0;
}

int hex2bytes(const char* text, unsigned int textlen, unsigned char* bytes, unsigned int size)
{
	unsigned int i, j;
	unsigned int hexcount = 0;

	for(i=0; i<textlen; i++)
	{
		if (ishex(text[i]))
			hexcount++;
	}

	if (hexcount != size*2)
	{
		fprintf(stdout, "Error, expected %d hex characters when parsing text \"", size*2);
		for(i=0; i<textlen; i++)
			fprintf(stdout, "%c", text[i]);
		fprintf(stdout, "\"\n");
		
		return -1;
	}

	for(i=0, j=0; i<textlen; i++)
	{
		if (ishex(text[i]))
		{
			if ( (j&1) == 0 )
				bytes[j/2] = hextobin(text[i])<<4;
			else
				bytes[j/2] |= hextobin(text[i]);
			j++;
		}
	}
	
	return 0;
}

int makedir(const char* dir)
{
#ifdef _WIN32
	return _mkdir(dir);
#else
	return mkdir(dir, 0777);
#endif
}

int deldir(const char* dir)
{
    char* cmd;
#ifdef _WIN32
    if(asprintf(&cmd, "del /s /q %s", dir) < 0)
        return -1;
#else
    if(asprintf(&cmd, "rm -rf %s", dir) < 0)
        return -1;
#endif
    return(system(cmd));
}

size_t get_fsize(FILE* f)
{
    fseeko(f, 0, SEEK_END);
    size_t fsize = ftello(f);
    rewind(f);
    return fsize;
}

int get_zeroes(u8* data, int max_len)
{
    int i = 0;
    for(i = 0; i < max_len; i++)
    {
        if(data[i] != 0)
            break;
    }
    return i; // will be max_len if the loop naturally terminates
}
