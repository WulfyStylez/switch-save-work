#ifndef _UTILS_H_
#define _UTILS_H_

#include <stdio.h>
#include "types.h"

#ifdef _WIN32
#define PATH_SEPERATOR '\\'
#else
#define PATH_SEPERATOR '/'
#endif

#ifndef MAX_PATH
	#define MAX_PATH 255
#endif

#ifdef __cplusplus
extern "C" {
#endif

void error(char* format, ...);
char *strtrimr (char *s);
void my_strdup(char*dest, char*src);

u32 crc32(u8 *message, u32 len);

u32 align(u32 offset, u32 alignment);
u64 align64(u64 offset, u32 alignment);
u64 getle64(const void* p);
u32 getle32(const void* p);
u32 getle16(const void* p);
u64 getbe64(const void* p);
u32 getbe32(const void* p);
u32 getbe16(const void* p);
void putle16(void* p, u16 n);
void putle32(void* p, u32 n);
void putle64(void* p, u64 n);
void putbe16(void* p, u16 n);
void putbe32(void* p, u32 n);
void putbe64(void* p, u64 n);

void hexdump(void *ptr, int buflen);
int key_load(char *name, u8 *out_buf);
int hex2bytes(const char* text, unsigned int textlen, unsigned char* bytes, unsigned int size);

int makedir(const char* dir);
int deldir(const char* dir);

// rewinds to 0 when done
size_t get_fsize(FILE* f);
// returns number of consecutive zero bytes from offset 0, up to max_len
int get_zeroes(u8* data, int max_len);

#ifdef __cplusplus
}
#endif

#endif // _UTILS_H_
