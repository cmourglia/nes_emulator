#include "cartridge.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct F6 {
    u8 lower_mapper : 4;
    u8 four_screen : 1;
    u8 trainer : 1;
    u8 has_battery_ram : 1;
    u8 mirroring : 1;
};

struct F7 {
    u8 upper_mapper : 4;
    u8 format : 2;
    u8 playchoice10 : 1;
    u8 vs_unisystem : 1;
};

struct F9 {
    u8 reserved : 7;
    u8 tv_system : 1;
};

struct F10 {
    u8 reserved1 : 2;
    u8 bus_conflict : 1;
    u8 prg_ram : 1;
    u8 reserved2 : 2;
    u8 tv_system : 2;
};

struct Header {
    char name[4];
    u8 len_prg_rom;
    u8 len_chr_rom;
    F6 f6;
    F7 f7;
    u8 len_prg_ram;
    F9 f9;
    F10 f10;
    u8 reserved[5];
};

static_assert(sizeof(F6) == 1);
static_assert(sizeof(F7) == 1);
static_assert(sizeof(F9) == 1);
static_assert(sizeof(F10) == 1);
static_assert(sizeof(Header) == 16);

constexpr u8 NES_MAGIC[] = {0x4E, 0x45, 0x53, 0x1A};
constexpr usize PRG_ROM_PAGE_SIZE = 16384;  // 0x4000
constexpr usize CHR_ROM_PAGE_SIZE = 8192;   // 0x8000

Cartridge load_cartridge(const char *filename) {
    FILE *file = fopen(filename, "rb");

    if (file == nullptr) {
        fprintf(stderr, "Error while trying to load file %s\n", filename);
        return Cartridge{};
    }

    Header header = {};
    fread(&header, sizeof(Header), 1, file);

    if (memcmp(header.name, NES_MAGIC, sizeof(NES_MAGIC)) != 0) {
        fprintf(stderr, "Cartridge ROM %s is not using iNES format\n",
                filename);
    }

    bool has_trainer = header.f6.trainer;  // Ignore 512 bytes of trainer data

    usize prg_rom_size = header.len_prg_rom * PRG_ROM_PAGE_SIZE;
    usize start_prg_rom = sizeof(Header) + (has_trainer ? 512 : 0);

    usize chr_rom_size = header.len_chr_rom * CHR_ROM_PAGE_SIZE;
    usize start_chr_rom = start_prg_rom + prg_rom_size;

    u8 mapper = (header.f6.lower_mapper) | (header.f7.upper_mapper << 4);

    Mirroring mirroring = header.f6.four_screen ? Mirroring_FourScreen
                          : header.f6.mirroring ? Mirroring_Vertical
                                                : Mirroring_Horizontal;

    Cartridge cartridge = {
        .prg_rom_size = prg_rom_size,
        .prg_rom = (u8 *)malloc(prg_rom_size),

        .chr_rom_size = chr_rom_size,
        .chr_rom = (u8 *)malloc(chr_rom_size),

        .mirroring = mirroring,
        .mapper = mapper,
    };

    fseek(file, (long)start_prg_rom, SEEK_SET);
    fread(cartridge.prg_rom, prg_rom_size, 1, file);

    fseek(file, (long)start_chr_rom, SEEK_SET);
    fread(cartridge.chr_rom, chr_rom_size, 1, file);

    return cartridge;
}

void release_cartridge(Cartridge *cartridge) {
    free(cartridge->prg_rom);
    free(cartridge->chr_rom);
}
