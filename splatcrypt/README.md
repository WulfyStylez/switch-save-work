# splatcrypt: Splatoon 2 save crypto tool.
This tool encrypts and decrypts saves from Splatoon 2 version 1.1.1 and on. It's provided uncompiled as it requires keydata I'm not comfortable with distributing. Tested and working on \*nix, needs libssl-dev.


Usage: splatcrypt [-e|-d] save.dat save_out.dat

-d will take a 1.1.1-format save, decrypt it and change version to 0. This effectively turns it into a 1.0-format save.

-e will take a 1.0-format save and encrypt+authenticate it for use on 1.1.1+. The save's CRC32 will also be corrected.


Keydata (lookup_table[]) can be acquired from an updated Splatoon 2 executable, which you can almost definitely acquire if you have saves in the first place. It's a 0x100-byte buffer referenced by the unlabelled function in e.g. Cmn::SaveDataFactory::encode and decode (this func does RNG-based key generation btw). 

The SHA256 of this data (raw, little-endian) is 9F9796262CA09932627C70605B2D62423B556816366CC3F0A9D330153A1C1228. Provide it as a series of u32s. Don't forget to endian-flip each u32 if converting from raw data!
