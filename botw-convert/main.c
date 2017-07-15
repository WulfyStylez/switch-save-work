#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <dirent.h>
#include <ftw.h>

#include "utils.h"



int jpg_extension_check(char* str)
{
    size_t in_len = strlen(str);
    return strncasecmp(str + in_len - 4, ".jpg", 4);
}

int endian_flip_file(char* fname)
{
    int i, j;
    
    printf("Processing file %s\n", fname);
    // skip jpgs!
    if(jpg_extension_check(fname) == 0)
    {
        //printf("Skipping .jpg %s\n\n", fname);
        return 0;
    }
    
    FILE* f_in = fopen(fname, "rb+");
    if(f_in == NULL)
    {
        fprintf(stderr, "Failed to open %s\n", fname);
        return -1;
    }
    
    u32 fsize = get_fsize(f_in);
    u32* f_data = malloc(fsize);
    if(!f_data)
    {
        fprintf(stderr, "Failed to allocate %d bytes for save\n", (u32)fsize);
        return -1;
    }
    if(fread(f_data, 1, fsize, f_in) != fsize)
    {
        fprintf(stderr, "Failed to read file!\n");
        return -1;
    }
    rewind(f_in);
    
    // should support up to 1.3.0
    u32 string_data_hashes[] = {0x81068a31, 0x0bee9e46, 0xd913b769,0xddb52297,0xaab21201,0x33bb43bb,0x1c10782f,0xf43eeba6,0x3863b57d,0x750e9d0e,0xccab71fd,0xbbac416b,0x22a510d1,0x55a22047,0xcbc6b5e4,0x71fe692e,0x39d08876,0x43c96bc7,0xd2e91906,0xf6332484,0xa4d177ef,0xfacfd421,0x0f9674ff,0x708311bc,0xc247b696,0x00ac6bc9,0x9c6cfd3f,0x6150c6be,0x333aa6e5,0x17cb2b11,0x7b74e117,0x5f283289,0x9b280a3b,0x2f95768f,0x530dd198,0xe7b0ad2c, 0x46D266B6, 0xF26F1A02};
    // ^ last two are for caption.sav strings
    
    // TODO: Switch->WiiU conversion needs opposite endian funcs
    bool uses_keydata = false;
    if(getbe32(&f_data[1]) == 0xFFFFFFFF && getbe32(&f_data[2]) == 1)
        uses_keydata = true;
    if(uses_keydata)
    {
        // flip first 3 u32s, then flip additional ones as needed
        for(i = 0; i < 3; i++)
            putle32(&f_data[i], getbe32(&f_data[i]));
        
        for(i = 3; i < fsize / 4; i += 2)
        {
            u32 key = getbe32(&f_data[i]);
            u32 val = getbe32(&f_data[i+1]);
            
            if(key == 0xFFFFFFFF)
                continue;
            //printf("%08X %08X\n", key, val);
            bool flip_data = true;
            for(j = 0; j < sizeof(string_data_hashes)/4; j++)
            {
                if(key == string_data_hashes[j])
                {
                    //printf("Matched string %08X\n", string_data_hashes[j]);
                    flip_data = false;
                }
            }
            putle32(&f_data[i], key);
            if(flip_data)
                putle32(&f_data[i+1], val);
        }
    }
    else
    {
        for(i = 0; i < fsize / 4; i++)
            putle32(&f_data[i], getbe32(&f_data[i]));
    }
    
    if(fwrite(f_data, 1, fsize, f_in) != fsize)
    {
        fprintf(stderr, "Failed to write file!\n");
        return -1;
    }
    
    fclose(f_in);
    return 0;
}

// returns 1 if this is a directory
int entry_check(const char *parent, char *name) {
    struct stat st_buf;
    if (!strcmp(".", name) || !strcmp("..", name))
        return -1;
    char *path = malloc(strlen(name) + strlen(parent) + 2);
    sprintf(path, "%s/%s", parent, name);
    stat(path, &st_buf);
    free(path);
    return S_ISDIR(st_buf.st_mode);
}

int flip_dir(const char *name) {
    DIR *dir = opendir(name);
    struct dirent *ent;
    while (ent = readdir(dir)) {
        char *entry_name = ent->d_name;
        int check = entry_check(name, entry_name);
        if(check < 0) continue;

        char *path = malloc(strlen(entry_name) + strlen(name) + 2);
        sprintf(path, "%s/%s", name, entry_name);

        if (check == 1) // parsable dir
            flip_dir(path);
        else if(check == 0) // file, let's try to endian-flip it
            endian_flip_file(path);
          
        free(path);
    }
    closedir(dir);

    return 0;
}

int main(int argc, char* argv[])
{
    // disable printf buffering for now
    setbuf(stdout, NULL);
    //setbuf(stderr, NULL);
    
    printf("Breath of the Wild WiiU->Switch save converter v1.0\n"
        "By WulfyStylez/SALT 2k17\n\n");
    
    if(argc != 2)
    {
        printf("Usage: %s [game_data.sav]\n", argv[0]);
        return -1;
    }
    
    flip_dir(argv[1]);
}
