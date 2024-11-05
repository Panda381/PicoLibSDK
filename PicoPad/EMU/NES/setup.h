// === Emulator setup
// - need to update main.cpp, InfoNES.cpp and K6502_rw.cpp

// system
#define NES_CLKSYS	200000	// system clock in kHz (determines mainly frequency of display updates)
#define NES_FREQ	1789773	// emulated frequency in Hz (determines emulation speed of the program)
#define NES_PAL		3	// Use NES palettes: 1=GBAR4444-main, 2=RGB555-Win,
				//    3=RGB888-Matthew, 4=RGB888-Piotr, 5=RGB888-NesDev,
				//    6=RGB888-Wiki, 7=RGB888-Smooth

// fine-tuning timing
#define NES_NMI_SHIFT	0	// shift start of V-blank (try to change if some lines flicker)
#define	NES_HIT_SHIFT	30	// shift sprite#0 hit (recommended 30; try to change if some lines flicker)

// clip image
#define NES_CLIP_UP	0	// clip image up
#define NES_CLIP_DOWN	0	// clip image down
#define NES_CLIP_LEFT	0	// clip image left
#define NES_CLIP_RIGHT	0	// clip image right
