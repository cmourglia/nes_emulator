#pragma once

#include "common.h"

#include "cpu.h"

struct Cartridge;

struct Bus {
    CPU cpu;
    // PPU ppu;
    Cartridge *cartridge;

    u8 cpu_ram[0x800];
};

void bus_load_cartridge(Bus *bus, Cartridge *cartridge);
void bus_reset(Bus *bus);

u8 read_mem(Bus *bus, u16 addr);
void write_mem(Bus *bus, u16 addr, u8 value);
