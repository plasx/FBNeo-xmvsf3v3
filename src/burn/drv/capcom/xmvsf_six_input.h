// Pure deterministic input transport. No emulator callbacks, host polling or gameplay routing.
#ifndef XMV_SIX_INPUT_H
#define XMV_SIX_INPUT_H
#include <stdint.h>
#include <string.h>
#include "xmvsf_six_abi.h"

struct XmvSixRecord {
    uint32_t sequence;
    uint16_t held[XMV6_PLAYERS], pressed[XMV6_PLAYERS], released[XMV6_PLAYERS];
};

struct XmvSixInputState {
    uint32_t abi;
    uint32_t sequence;
    uint16_t count;
    uint16_t held[XMV6_PLAYERS], previous[XMV6_PLAYERS];
    uint16_t pressed[XMV6_PLAYERS], released[XMV6_PLAYERS];
    XmvSixRecord history[XMV6_DEPTH];
};

static inline void XmvSixReset(XmvSixInputState* s)
{
    memset(s, 0, sizeof(*s));
    s->abi = XMV6_VERSION;
}

static inline uint16_t XmvSixClean(uint16_t value)
{
    value &= XMV6_VALID_MASK;
    if ((value & 3) == 3) value &= ~3u;
    if ((value & 12) == 12) value &= ~12u;
    return value;
}

static inline void XmvSixSample(XmvSixInputState* s, const uint16_t raw[XMV6_PLAYERS])
{
    ++s->sequence; // Unsigned wrap is part of ABI1.
    if (s->count < XMV6_DEPTH) ++s->count;
    XmvSixRecord* r = &s->history[s->sequence % XMV6_DEPTH];
    r->sequence = s->sequence;
    for (unsigned p = 0; p < XMV6_PLAYERS; ++p) {
        s->previous[p] = s->held[p];
        s->held[p] = XmvSixClean(raw[p]);
        s->pressed[p] = s->held[p] & ~s->previous[p];
        s->released[p] = s->previous[p] & ~s->held[p];
        r->held[p] = s->held[p];
        r->pressed[p] = s->pressed[p];
        r->released[p] = s->released[p];
    }
}

// A read-only helper for tests/consumers: 1=next ordered sample, 0=no sample,
// -1=overrun/reset mismatch. On -1 caller clears pending actions, adopts held,
// and sets its cursor to current sequence. It must not consume stale history.
static inline int XmvSixNext(const XmvSixInputState* s, uint32_t cursor, XmvSixRecord* out)
{
    uint32_t gap = s->sequence - cursor;
    if (!gap) return 0;
    if (gap > s->count || gap > XMV6_DEPTH) return -1;
    const XmvSixRecord* r = &s->history[(cursor + 1u) % XMV6_DEPTH];
    if (r->sequence != cursor + 1u) return -1;
    *out = *r;
    return 1;
}

static inline uint16_t XmvSixReadWord(const XmvSixInputState* s, unsigned offset)
{
    if ((offset & 1u) || offset >= XMV6_WINDOW_BYTES) return 0;
    switch (offset) {
        case XMV6_REG_MAGIC_XM: return 0x584d;
        case XMV6_REG_MAGIC_6P: return 0x3650;
        case XMV6_REG_VERSION: return XMV6_VERSION;
        case XMV6_REG_PLAYERS: return XMV6_PLAYERS;
        case XMV6_REG_SEQUENCE: return (uint16_t)(s->sequence >> 16);
        case XMV6_REG_SEQUENCE + 2: return (uint16_t)s->sequence;
        case XMV6_REG_VALID_MASK: return XMV6_VALID_MASK;
        case XMV6_REG_DEPTH: return XMV6_DEPTH;
        case XMV6_REG_COUNT: return s->count;
        case XMV6_REG_CAPABILITIES: return 3; // bit0=work RAM; bit1=portrait1 stateless palette.
    }
    const unsigned starts[] = {XMV6_REG_HELD, XMV6_REG_PRESSED, XMV6_REG_RELEASED, XMV6_REG_PREVIOUS};
    const uint16_t* arrays[] = {s->held, s->pressed, s->released, s->previous};
    for (unsigned i = 0; i < 4; ++i)
        if (offset >= starts[i] && offset < starts[i] + XMV6_PLAYERS * 2)
            return arrays[i][(offset - starts[i]) / 2];
    if (offset >= XMV6_REG_HISTORY && offset < XMV6_REG_HISTORY + XMV6_DEPTH * XMV6_RECORD_BYTES) {
        unsigned index = (offset - XMV6_REG_HISTORY) / XMV6_RECORD_BYTES;
        unsigned at = (offset - XMV6_REG_HISTORY) % XMV6_RECORD_BYTES;
        const XmvSixRecord* r = &s->history[index];
        if (at == 0) return (uint16_t)(r->sequence >> 16);
        if (at == 2) return (uint16_t)r->sequence;
        if (at < XMV6_REC_PRESSED) return r->held[(at - XMV6_REC_HELD) / 2];
        if (at < XMV6_REC_RELEASED) return r->pressed[(at - XMV6_REC_PRESSED) / 2];
        return r->released[(at - XMV6_REC_RELEASED) / 2];
    }
    return 0;
}

static inline uint8_t XmvSixReadByte(const XmvSixInputState* s, unsigned offset)
{
    if (offset >= XMV6_WINDOW_BYTES) return 0;
    uint16_t word = XmvSixReadWord(s, offset & ~1u);
    return (uint8_t)((offset & 1u) ? word : word >> 8);
}

static inline uint16_t XmvSixRamReadWord(const uint8_t* ram, unsigned offset)
{
    if ((offset & 1u) || offset >= XMV6_WORK_RAM_BYTES - 1) return 0;
    return (uint16_t)((unsigned)ram[offset] << 8 | ram[offset + 1]);
}

static inline void XmvSixRamWriteWord(uint8_t* ram, unsigned offset, uint16_t value)
{
    if ((offset & 1u) || offset >= XMV6_WORK_RAM_BYTES - 1) return;
    ram[offset] = (uint8_t)(value >> 8);
    ram[offset + 1] = (uint8_t)value;
}
#endif
