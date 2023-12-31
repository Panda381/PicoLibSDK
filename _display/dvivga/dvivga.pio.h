// -------------------------------------------------- //
// This file is autogenerated by pioasm; do not edit! //
// -------------------------------------------------- //

#if !PICO_NO_HARDWARE
#include "hardware/pio.h"
#endif

// --- //
// dvi //
// --- //

#define dvi_wrap_target 0
#define dvi_wrap 1

static const uint16_t dvi_program_instructions[] = {
            //     .wrap_target
    0x70a1, //  0: out    pc, 1           side 2     
    0x68a1, //  1: out    pc, 1           side 1     
            //     .wrap
};

#if !PICO_NO_HARDWARE
static const struct pio_program dvi_program = {
    .instructions = dvi_program_instructions,
    .length = 2,
    .origin = 0,
};

static inline pio_sm_config dvi_program_get_default_config(uint offset) {
    pio_sm_config c = pio_get_default_sm_config();
    sm_config_set_wrap(&c, offset + dvi_wrap_target, offset + dvi_wrap);
    sm_config_set_sideset(&c, 2, false, false);
    return c;
}
#endif

// ------- //
// dvi_inv //
// ------- //

#define dvi_inv_wrap_target 0
#define dvi_inv_wrap 1

static const uint16_t dvi_inv_program_instructions[] = {
            //     .wrap_target
    0x68a1, //  0: out    pc, 1           side 1     
    0x70a1, //  1: out    pc, 1           side 2     
            //     .wrap
};

#if !PICO_NO_HARDWARE
static const struct pio_program dvi_inv_program = {
    .instructions = dvi_inv_program_instructions,
    .length = 2,
    .origin = 0,
};

static inline pio_sm_config dvi_inv_program_get_default_config(uint offset) {
    pio_sm_config c = pio_get_default_sm_config();
    sm_config_set_wrap(&c, offset + dvi_inv_wrap_target, offset + dvi_inv_wrap);
    sm_config_set_sideset(&c, 2, false, false);
    return c;
}
#endif

// ------- //
// minivga //
// ------- //

#define minivga_wrap_target 11
#define minivga_wrap 15

#define minivga_offset_hsync 0u
#define minivga_offset_entry 1u
#define minivga_offset_vsync 3u
#define minivga_offset_vhsync 6u
#define minivga_offset_dark 9u
#define minivga_offset_output 13u
#define minivga_offset_extra 13u
#define minivga_offset_voutput 16u
#define minivga_offset_vextra 16u

static const uint16_t minivga_program_instructions[] = {
    0x0840, //  0: jmp    x--, 0          side 1     
    0x683b, //  1: out    x, 27           side 1     
    0x68a5, //  2: out    pc, 5           side 1     
    0x1043, //  3: jmp    x--, 3          side 2     
    0x703b, //  4: out    x, 27           side 2     
    0x70a5, //  5: out    pc, 5           side 2     
    0x1846, //  6: jmp    x--, 6          side 3     
    0x783b, //  7: out    x, 27           side 3     
    0x78a5, //  8: out    pc, 5           side 3     
    0xa003, //  9: mov    pins, null      side 0     
    0x004a, // 10: jmp    x--, 10         side 0     
            //     .wrap_target
    0x603b, // 11: out    x, 27           side 0     
    0x60a5, // 12: out    pc, 5           side 0     
    0x6310, // 13: out    pins, 16        side 0 [3] 
    0x004d, // 14: jmp    x--, 13         side 0     
    0xa003, // 15: mov    pins, null      side 0     
            //     .wrap
    0x7310, // 16: out    pins, 16        side 2 [3] 
    0x1050, // 17: jmp    x--, 16         side 2     
    0xb003, // 18: mov    pins, null      side 2     
    0x703b, // 19: out    x, 27           side 2     
    0x70a5, // 20: out    pc, 5           side 2     
};

#if !PICO_NO_HARDWARE
static const struct pio_program minivga_program = {
    .instructions = minivga_program_instructions,
    .length = 21,
    .origin = 2,
};

static inline pio_sm_config minivga_program_get_default_config(uint offset) {
    pio_sm_config c = pio_get_default_sm_config();
    sm_config_set_wrap(&c, offset + minivga_wrap_target, offset + minivga_wrap);
    sm_config_set_sideset(&c, 2, false, false);
    return c;
}
#endif

// ------------- //
// minivga_csync //
// ------------- //

#define minivga_csync_wrap_target 11
#define minivga_csync_wrap 15

#define minivga_csync_offset_hsync 0u
#define minivga_csync_offset_entry 1u
#define minivga_csync_offset_vsync 3u
#define minivga_csync_offset_vhsync 6u
#define minivga_csync_offset_dark 9u
#define minivga_csync_offset_output 13u
#define minivga_csync_offset_extra 13u
#define minivga_csync_offset_voutput 16u
#define minivga_csync_offset_vextra 16u

static const uint16_t minivga_csync_program_instructions[] = {
    0x1040, //  0: jmp    x--, 0          side 1     
    0x703b, //  1: out    x, 27           side 1     
    0x70a5, //  2: out    pc, 5           side 1     
    0x1043, //  3: jmp    x--, 3          side 1     
    0x703b, //  4: out    x, 27           side 1     
    0x70a5, //  5: out    pc, 5           side 1     
    0x0046, //  6: jmp    x--, 6          side 0     
    0x603b, //  7: out    x, 27           side 0     
    0x60a5, //  8: out    pc, 5           side 0     
    0xa003, //  9: mov    pins, null      side 0     
    0x004a, // 10: jmp    x--, 10         side 0     
            //     .wrap_target
    0x603b, // 11: out    x, 27           side 0     
    0x60a5, // 12: out    pc, 5           side 0     
    0x6310, // 13: out    pins, 16        side 0 [3] 
    0x004d, // 14: jmp    x--, 13         side 0     
    0xa003, // 15: mov    pins, null      side 0     
            //     .wrap
    0x7310, // 16: out    pins, 16        side 1 [3] 
    0x1050, // 17: jmp    x--, 16         side 1     
    0xb003, // 18: mov    pins, null      side 1     
    0x703b, // 19: out    x, 27           side 1     
    0x70a5, // 20: out    pc, 5           side 1     
};

#if !PICO_NO_HARDWARE
static const struct pio_program minivga_csync_program = {
    .instructions = minivga_csync_program_instructions,
    .length = 21,
    .origin = 2,
};

static inline pio_sm_config minivga_csync_program_get_default_config(uint offset) {
    pio_sm_config c = pio_get_default_sm_config();
    sm_config_set_wrap(&c, offset + minivga_csync_wrap_target, offset + minivga_csync_wrap);
    sm_config_set_sideset(&c, 1, false, false);
    return c;
}
#endif

