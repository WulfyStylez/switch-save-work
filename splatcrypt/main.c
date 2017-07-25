#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <dirent.h>
#include <openssl/aes.h> 
#include <openssl/cmac.h>

#include "utils.h"

u32 rand32(u32 rand[4])
{
    u32 t = rand[0];
    t ^= (t << 11);
    t ^= (t >> 8);
    t ^= rand[3];
    t ^= (rand[3] >> 19);
    
    rand[0] = rand[1];
    rand[1] = rand[2];
    rand[2] = rand[3];
    rand[3] = t;
    return t;
}

u32 lookup_table[64] = {0};

void generate_randkey(u32 rand[4], u8 key[16])
{
    u32* key32 = (u32*)key;
    memset(key, 0, 16);
    
    // outer loop is for each u32 of the key, inner is for each u8
    for(u32 i = 0; i < 4; i++)
    {
        for(u32 j = 0; j < 4; j++)
        {
            u32 state = key32[i];
            // get a lookup value from a random index within bounds
            u32 lookup = lookup_table[rand32(rand) >> 26];
            // generate 2 random bits in bits 3 and 4
            u32 t = (rand32(rand) >> 27) & 0x18;
            
            // shift lookup by t value, use this as the bottom u8 of the state
            t = lookup >> t;
            state |= (t & 0xFF);
            
            // shift to free up the bottom u8, unless we just set it
            if(j != 3)
                state = state << 8;
            
            key32[i] = state;
        }
    }
}

// all little-endian, be sure to use getle32/etc to be safe
typedef struct {
    u32 product_version;
    u32 develop_version;
    u32 crc32;
    u32 pad;
    u8 body[0x483A0];
    u8 iv[16];
    u32 rand[4]; // random ctx used to generate keys
    u8 aes_cmac[16];
} save_v1;

void do_body_cmac(u8 save_body[0x483A0], u8 key[16], u8 cmac_out[16])
{
    size_t cmac_out_len;
    CMAC_CTX *cmac = CMAC_CTX_new();
    CMAC_Init(cmac, key, 16, EVP_aes_128_cbc(), NULL);
    CMAC_Update(cmac, save_body, 0x483A0);
    CMAC_Final(cmac, cmac_out, &cmac_out_len);
    CMAC_CTX_free(cmac);
}

void decrypt_v1_save(u8* save_buf)
{
    save_v1 *save = (save_v1*)save_buf;
    // pull in the rand ctx and generate keys
    u32 rand_ctx[4];
    for(int i = 0; i < 4; i++)
        rand_ctx[i] = getle32(&save->rand[i]);
    
    u8 enc_key[16];
    u8 auth_key[16];
    generate_randkey(rand_ctx, enc_key);
    generate_randkey(rand_ctx, auth_key);
    
    AES_KEY aes;
    AES_set_decrypt_key(enc_key, 128, &aes);
    AES_cbc_encrypt((u8*)&save->body, (u8*)&save->body, 0x483A0, &aes, (u8*)&save->iv, AES_DECRYPT);
    
    printf("Save decrypted, verifying AES-CMAC...\n");
    u8 cmac_out[16];
    do_body_cmac((u8*)&save->body, auth_key, cmac_out);
    
    if(memcmp(cmac_out, &save->aes_cmac, 16))
        printf("CMAC check failed!\n");
    else
        printf("CMAC check passed!\n");
    
    printf("Checking CRC32...\n");
    u32 crc = crc32((u8*)&save->body, 0x483A0);
    if(memcmp(&crc, &save->crc32, 4))
        printf("CRC check failed!\n");
    else
        printf("CRC check passed!\n");
    
    printf("Patching save to v0 and writing out...\n");
    putle32(&save->product_version, 0);
}

void encrypt_v0_save(u8* save_buf)
{
    save_v1 *save = (save_v1*)save_buf;
    
    // fix up the CRC32 over the decrypted body
    printf("Fixing up CRC32...\n");
    putle32(&save->crc32, crc32((u8*)&save->body, 0x483A0));
    
    // generate random IV and seed, and add them to the save
    printf("Generating security data...\n");
    srand(time(NULL));
    u32 rand_ctx[4];
    u32 iv[4];
    for(int i = 0; i < 4; i++)
    {
        rand_ctx[i] = rand();
        putle32(&save->rand[i], rand_ctx[i]);
        iv[i] = rand();
    }
    memcpy(&save->iv, iv, 16);
    
    // generate keys
    u8 enc_key[16];
    u8 auth_key[16];
    generate_randkey(rand_ctx, enc_key);
    generate_randkey(rand_ctx, auth_key);
    
    printf("Generating save body CMAC...\n");
    // calculate AES-CMAC of decrypted data and write it to the save
    do_body_cmac((u8*)&save->body, auth_key, (u8*)&save->aes_cmac);
    
    // encrypt the body of the save
    printf("Encrypting save body...\n");
    AES_KEY aes;
    AES_set_encrypt_key(enc_key, 128, &aes);
    AES_cbc_encrypt((u8*)&save->body, (u8*)&save->body, 0x483A0, &aes, (u8*)iv, AES_ENCRYPT);
    
    printf("Fixing up version to v1 and writing...\n");
    putle32(&save->product_version, 1);
}

typedef enum {encrypt, decrypt} prog_mode;

int main(int argc, char* argv[])
{
    setbuf(stdout, NULL);
    printf("Splatoon 2 save crypt tool v1.0\n"
        "By WulfyStylez/SALT 2k17\n\n");
    
    if(argc != 4)
    {
        printf("Usage: %s [-e|-d] [save.dat] [save_out.dat]\n"
               "-e: encrypt a v0 save into a v1 save\n"
               "-d: decrypt a v1 save and convert to v0\n"
               "Tested up to version 1.1.1\n", argv[0]);
        return -1;
    }
    
    // check mode
    prog_mode mode;
    if(!strcmp(argv[1], "-e"))
        mode = encrypt;
    else if(!strcmp(argv[1], "-d"))
        mode = decrypt;
    else
    {
        printf("Invalid mode %s !\n", argv[1]);
        return -1;
    }
        
    
    FILE* f_in = fopen(argv[2], "rb");
    if(f_in == NULL)
    {
        printf("Failed to open file %s for reading!\n", argv[1]);
        return -1;
    }
    
    size_t fsize = get_fsize(f_in);
    if(fsize != 0x483B0 && fsize != 0x483E0)
    {
        printf("Invalid file size for 1.0 or 1.1.1 save!\n");
        fclose(f_in);
        return -1;
    }
    
    // always malloc the larger size so we can safely append to v0 saves
    u8* save_buf = malloc(0x483E0);
    if(save_buf == NULL)
    {
        printf("Failed to allocate %d bytes for reading game save!\n", 0x483E0);
        fclose(f_in);
        return -1;
    }
    
    size_t read_size = fread(save_buf, 1, fsize, f_in);
    fclose(f_in);
    if(read_size != fsize)
    {
        printf("Failed to read save file!\n");
        return -1;
    }
    
    // 60,000,000 years of C error checking later, sanity-check save and mode
    u32 version = getle32(&save_buf[0]);
    if(version == 0 && mode == decrypt)
    {
        // TODO: encrypt these
        printf("Version 0 saves are already decrypted!\n");
        free(save_buf);
        return -1;
    }
    else if(version == 1 && mode == encrypt)
    {
        // TODO: encrypt these
        printf("Version %d saves are already encrypted!\n", version);
        free(save_buf);
        return -1;
    }
    else if(version > 1)
    {
        printf("Unrecognized save version %d!\n", version);
        free(save_buf);
        return -1;
    }
    
    if(mode == decrypt)
        decrypt_v1_save(save_buf);
    else if(mode == encrypt)
        encrypt_v0_save(save_buf);
    
    FILE* f_out = fopen(argv[3], "wb");
    if(f_out == NULL)
    {
        printf("Failed to open file %s for writing!\n", argv[3]);
        free(save_buf);
        return -1;
    }
    
    size_t size_to_write = mode == encrypt ? 0x483E0 : 0x483B0;
    size_t size_written = fwrite(save_buf, 1, size_to_write, f_out);
    fclose(f_out);
    free(save_buf);
    if(size_written != size_to_write)
    {
        printf("Failed to write to output file!\n");
        return -1;
    }
    
    printf("Done!\n");
}
