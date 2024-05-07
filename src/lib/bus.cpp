#include "bus.h"

#include <stdio.h>
#include <stdlib.h>

#include "cartridge.h"

constexpr u16 RAM_START = 0x0000;
constexpr u16 RAM_MIRROR_END = 0x2000;  // Excluded

constexpr u16 PPU_REGISTERS_START = 0x2000;
constexpr u16 PPU_REGISTERS_END = 0x2008;         // Excluded
constexpr u16 PPU_REGISTERS_MIRROR_END = 0x4000;  // Excluded

constexpr u16 PRG_ROM_START = 0x8000;
constexpr u16 PRG_ROM_END = 0xFFFF;

bool cart_read(Bus *bus, u16 addr, u8 *value) {
    // TODO: Use mappers here
    if (addr <= PRG_ROM_START || addr >= PRG_ROM_END) {
        return false;
    }

    addr -= PRG_ROM_START;
    // Mirror if addr > 0x4000
    addr = bus->cartridge->prg_rom_size == 0x4000 ? addr % 0x4000 : addr;
    *value = bus->cartridge->prg_rom[addr];
    return true;
}

bool cart_write(Bus *bus, u16 addr, u8 value) {
    UNUSED(bus);
    UNUSED(addr);
    UNUSED(value);

    return false;
}

u8 read_mem(Bus *bus, u16 addr) {
    u8 value = 0xFF;
    if (cart_read(bus, addr, &value)) {
    } else if (addr >= RAM_START && addr <= RAM_MIRROR_END) {
        addr &= 0x07FF;  // 00000111 11111111
        value = bus->cpu_ram[addr];
    } else if (addr >= PPU_REGISTERS_START && addr < PPU_REGISTERS_MIRROR_END) {
        addr &= 0x2007;  // 00100000 00000111
        UNUSED(addr);
        // TODO
    } else {
        // TODO
    }
    return value;
}

void write_mem(Bus *bus, u16 addr, u8 value) {
    if (cart_write(bus, addr, value)) {
    } else if (addr >= RAM_START && addr < RAM_MIRROR_END) {
        addr &= 0x07FF;
        bus->cpu_ram[addr] = value;
    } else if (addr >= PPU_REGISTERS_START && addr < PPU_REGISTERS_MIRROR_END) {
        addr &= 0x2007;
        // TODO
    } else {
        // TODO
    }
}

void bus_load_cartridge(Bus *bus, Cartridge *cartridge) {
    bus->cartridge = cartridge;
    // FIXME: Anything else ?
}

void bus_reset(Bus *bus) {
    bus->cpu.bus = bus;

    cpu_reset(&bus->cpu);
    // TODO: Reset other stuff
}
