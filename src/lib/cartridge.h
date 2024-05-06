#pragma once

#include "common.h"

enum Mirroring {
    Mirroring_Vertical,
    Mirroring_Horizontal,
    Mirroring_FourScreen,
};

struct Cartridge {
    usize prg_rom_size;
    u8 *prg_rom;

    usize chr_rom_size;
    u8 *chr_rom;

    Mirroring mirroring;
    u8 mapper;
};

Cartridge load_cartridge(const char* filename);

void release_cartridge(Cartridge* cartridge);
