# Zelda: Breath Of The Wild save converter
This tool takes a folder from a Wii U BotW save and does careful endian flipping to fully convert it to work with the Switch version instead.
Switch -> Wii U support is planned but, you know, not really high priority.

usage: botwconvert [savedir]

TECHNICAL:
This tool works by endian-flipping every u32 in every file except for .jpg's. Additionally, in the case of files using key-and-data pairs to store data (game_data.sav, caption.sav) it will preserve the endian of strings, as flipping them will break the save. Thanks to MrCheeze for acquiring and making all this data available ( https://github.com/MrCheeze/botw-tools )