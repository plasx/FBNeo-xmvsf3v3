// Stateless portrait2 palette decoder (portrait1 + six colours + wide HUD
// geometry). Input ROM bytes retain the normal FBNeo CPS2 chip order (each
// 68000 word has its low byte first). No native palette, emulated RAM, raster
// state or object memory is modified.
#ifndef XMV_PORTRAIT_PALETTE_H
#define XMV_PORTRAIT_PALETTE_H
#include <stdint.h>
#include <stddef.h>

static inline uint16_t XmvPortraitWord(const uint8_t* rom, size_t at)
{
    return (uint16_t)((unsigned)rom[at] | (unsigned)rom[at + 1] << 8);
}

// Portrait markers carried in the buffered OBJ palette field:
//   $1C/$1D  portrait1 stock costume 0/1 (older ROMs, capability without bit2)
//   $12..$17 portrait2 colour 0..5 (ROM saw capability bit2)
// Geometry: normal view left raw X $E040 / right $E1A0; the expanded 512-wide
// view (capability bit3) left $E000 / right $E1E0. Raw Y is one of the three
// compact HUD rows. Left objects are unflipped 2x1 ($0100), right ones are
// X-flipped 2x1 ($0120). Anything else is not a portrait.
static inline bool XmvPortraitObject2(unsigned tile, unsigned rawX, unsigned rawY,
    unsigned attributes, bool wide, unsigned* color)
{
    if (tile < 0x1ff00 || tile > 0x1ff22 || (tile & 1)) return false;
    if (rawY != 0x2020 && rawY != 0x202e && rawY != 0x203c) return false;
    unsigned flags = attributes & ~0x1fu, marker = attributes & 0x1fu;
    if (rawX == (wide ? 0xe000u : 0xe040u)) {
        if (flags != 0x0100) return false;
    } else if (rawX == (wide ? 0xe1e0u : 0xe1a0u)) {
        if (flags != 0x0120) return false;
    } else {
        return false;
    }
    if (marker == 0x1c || marker == 0x1d) { *color = marker - 0x1c; return true; }
    if (marker >= 0x12 && marker <= 0x17) { *color = marker - 0x12; return true; }
    return false;
}

// Returns 16 RGB444 colours in the same pen order as CpsPalUpdate (pen^15).
// A false result never touches output. Colours 0/1 read the pinned Euro DATA
// directory $1145A8 (+$60 for costume 1); colours 2..5 read the ROM's X3C6
// variant table at $335000 (header: 'X3C6', version 1, 19 identities,
// 4 variants, 96-byte stride, data at +$10), row 0 of the selected block.
static inline bool XmvPortraitColors2(bool diagnostic, const uint8_t* rom,
    size_t romBytes, unsigned tile, unsigned color, uint16_t output[16])
{
    static const unsigned characters[18] = {
        0x00,0x02,0x04,0x06,0x08,0x0a,0x0c,0x0e,
        0x12,0x14,0x16,0x18,0x1a,0x1c,0x1e,0x20,0x22,0x24
    };
    static const unsigned pointers[19] = {
        0x114d7c,0x114e3c,0x114fbc,0x11531c,0x1153dc,
        0x11549c,0x11559c,0x1156bc,0x11577c,0x11589c,
        0x115a1c,0x115b9c,0x115c5c,0x115d5c,0x115fdc,
        0x11609c,0x11621c,0x11639c,0x1164bc
    };
    if (!diagnostic || !rom || !output || romBytes < 0x338000 || color > 5) return false;
    if (tile < 0x1ff00 || tile > 0x1ff22 || (tile & 1)) return false;
    unsigned character = characters[(tile - 0x1ff00) / 2];
    unsigned pointer;
    if (color <= 1) {
        unsigned directory = 0x1145a8 + character * 2;
        pointer = (unsigned)XmvPortraitWord(rom, directory) << 16 |
            XmvPortraitWord(rom, directory + 2);
        // This profile is for the pinned Euro DATA directory only.
        if (pointer != pointers[character / 2]) return false;
        pointer += color * 0x60;
    } else {
        const unsigned table = 0x335000;
        if (XmvPortraitWord(rom, table) != 0x5833 || XmvPortraitWord(rom, table + 2) != 0x4336 ||
            XmvPortraitWord(rom, table + 4) != 1 || XmvPortraitWord(rom, table + 6) != 19 ||
            XmvPortraitWord(rom, table + 8) != 4 || XmvPortraitWord(rom, table + 10) != 96 ||
            XmvPortraitWord(rom, table + 12) != 0x10) return false;
        pointer = table + 0x10 + ((character / 2) * 4 + (color - 2)) * 96;
    }
    if (pointer > romBytes - 32) return false;
    for (unsigned pen = 0; pen < 16; ++pen)
        output[pen ^ 15] = XmvPortraitWord(rom, pointer + pen * 2) & 0x0fff;
    return true;
}
#endif
