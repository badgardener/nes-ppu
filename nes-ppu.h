#ifndef NES_PPU_H
#define NES_PPU_H

/**
 * Cycle-level emulation interface for the NES Picture Processing Unit.
 *
 * This header defines the PPU state, memory organization, rendering
 * pipeline, register interface, timing state, interrupt state, and
 * public interface used by the NES PPU emulator.
 *
 * The PPU is represented as an explicit machine state. Its registers,
 * scrolling state, background pipeline, sprite evaluation state,
 * memory bus, timing counters, DMA state, I/O latches, rendering state,
 * interrupt state, and temporary values are exposed through the
 * NES_PPU structure so that the emulator can operate at cycle granularity.
 *
 * The implementation is provided by nes-ppu.c.
 */

#ifndef NES_BIN_DATATYPES
#define NES_BIN_DATATYPES

#include <stdbool.h>
#include <stdint.h>

/**
 * Signed 8-bit integer.
 */
typedef int8_t byte;

/**
 * Signed 16-bit integer.
 */
typedef int16_t word;

/**
 * Unsigned 8-bit integer.
 */
typedef uint8_t ubyte;

/**
 * Unsigned 16-bit integer.
 */
typedef uint16_t uword;

/**
 * Unsigned counter used for cycle and frame counts.
 */
typedef uint64_t counter;

#endif // NES_BIN_DATATYPES

/**
 * Forward declaration of the PPU memory bus.
 *
 * The complete memory bus implementation is provided by the system bus.
 */
typedef struct PPU_MemoryBus PPU_MemoryBus;

/**
 * NES hardware timing regions.
 */
#ifndef NES_REGIONS
#define NES_REGIONS

typedef enum NES_Regions {
  /**
   * NTSC timing region.
   */
  REGION_NTSC,

  /**
   * PAL timing region.
   */
  REGION_PAL,

  /**
   * Dendy timing region.
   */
  REGION_DENDY,
} NES_Regions;

#endif // NES_REGIONS

/**
 * Nametable mirroring configuration.
 *
 * Determines how the logical nametable address space maps to physical
 * nametable memory.
 */
typedef enum PPU_Mirroring {
  /**
   * Four-screen nametable arrangement.
   */
  MIRROR_FOURSCREEN,

  /**
   * Horizontal nametable mirroring.
   */
  MIRROR_HORIZONTAL,

  /**
   * Vertical nametable mirroring.
   */
  MIRROR_VERTICAL,

  /**
   * Single-screen mirroring mapped to the first nametable.
   */
  MIRROR_SINGLE0,

  /**
   * Single-screen mirroring mapped to the second nametable.
   */
  MIRROR_SINGLE1,
} PPU_Mirroring;

/**
 * Complete state of the emulated NES PPU.
 *
 * NES_PPU contains the state required to advance the PPU one dot at a
 * time. Architectural registers, rendering pipeline state, memory
 * interfaces, timing counters, interrupt signals, and internal latches
 * are grouped together so that the PPU can be inspected and debugged
 * at cycle granularity.
 */
typedef struct NES_PPU {
  /**
   * CPU-visible PPU registers.
   */
  struct reg {
    /**
     * PPU control register ($2000).
     *
     * Controls nametable selection, VRAM address increment, sprite
     * pattern table selection, background pattern table selection,
     * sprite size, and NMI generation.
     */
    ubyte PPUCTRL;

    /**
     * PPU mask register ($2001).
     *
     * Controls grayscale output, color emphasis, background and sprite
     * rendering, and rendering within the leftmost eight pixels.
     */
    ubyte PPUMASK;

    /**
     * PPU status register ($2002).
     *
     * Exposes vertical blank, sprite-zero hit, and sprite overflow flags.
     */
    ubyte PPUSTATUS;

    /**
     * OAM address register ($2003).
     *
     * Selects the primary OAM byte accessed by OAMDATA.
     */
    ubyte OAMADDR;

    /**
     * OAM data register ($2004).
     *
     * Provides CPU access to primary OAM.
     */
    ubyte OAMDATA;

    /**
     * Scroll register ($2005).
     *
     * Accepts horizontal and vertical scroll components through the
     * shared two-write register interface.
     */
    ubyte PPUSCROLL;

    /**
     * VRAM address register ($2006).
     *
     * Accepts the high and low portions of the CPU-specified PPU address.
     */
    ubyte PPUADDR;

    /**
     * VRAM data register ($2007).
     *
     * Provides CPU access to PPU memory through the internal read buffer
     * and address increment mechanism.
     */
    ubyte PPUDATA;
  } reg;

  /**
   * Internal scrolling and VRAM address state.
   *
   * Implements the temporary and current address registers and the
   * fine horizontal scroll state used by the rendering pipeline.
   */
  struct loopy {
    /**
     * Current VRAM address register.
     *
     * Contains the active coarse X, coarse Y, nametable, and fine Y
     * address components.
     */
    uword v;

    /**
     * Temporary VRAM address register.
     *
     * Receives address and scroll information through CPU register writes.
     */
    uword t;

    /**
     * Fine horizontal scroll position.
     *
     * Selects the starting bit within the background pattern shifters.
     */
    ubyte fine_x;

    /**
     * First-or-second write toggle.
     *
     * False indicates that the next write is the first write; true
     * indicates that the next write is the second write.
     */
    bool w;
  } loopy;

  /**
   * Background rendering pipeline state.
   */
  struct background {
    /**
     * Latched nametable tile identifier.
     */
    ubyte nametable_latch;

    /**
     * Latched attribute table value.
     */
    ubyte attribute_latch;

    /**
     * Latched low background pattern byte.
     */
    ubyte pattern_low_latch;

    /**
     * Latched high background pattern byte.
     */
    ubyte pattern_high_latch;

    /**
     * Low-plane background pattern shift register.
     */
    uword pattern_shift_low;

    /**
     * High-plane background pattern shift register.
     */
    uword pattern_shift_high;

    /**
     * Low-plane background attribute shift register.
     */
    uword attribute_shift_low;

    /**
     * High-plane background attribute shift register.
     */
    uword attribute_shift_high;

    /**
     * Tile identifier fetched for the next background tile.
     */
    ubyte next_tile_id;

    /**
     * Palette attribute fetched for the next background tile.
     */
    ubyte next_tile_attribute;

    /**
     * Low pattern-plane byte fetched for the next background tile.
     */
    ubyte next_tile_pattern_low;

    /**
     * High pattern-plane byte fetched for the next background tile.
     */
    ubyte next_tile_pattern_high;
  } bg;

  /**
   * Sprite memory, evaluation, and rendering pipeline state.
   */
  struct sprite {
    /**
     * Primary Object Attribute Memory.
     *
     * Stores 64 sprites, each represented by four bytes.
     */
    ubyte oam[256];

    /**
     * Secondary OAM used during sprite evaluation.
     *
     * Holds up to eight selected sprites for the next scanline.
     */
    ubyte secondary_oam[32];

    /**
     * Internal primary OAM address.
     */
    ubyte oam_addr;

    /**
     * Current secondary OAM byte address.
     */
    ubyte secondary_oam_addr;

    /**
     * Primary OAM sprite index used during evaluation.
     */
    ubyte eval_n;

    /**
     * Byte index within the sprite currently being evaluated.
     */
    ubyte eval_m;

    /**
     * Temporary byte read during sprite evaluation.
     */
    ubyte eval_latch;

    /**
     * Number of sprites selected during the current evaluation.
     */
    ubyte eval_count;

    /**
     * Current secondary OAM write position.
     */
    ubyte eval_sec_addr;

    /**
     * Indicates that sprite overflow has been detected during evaluation.
     */
    bool eval_overflow;

    /**
     * Indicates that sprite evaluation has completed.
     */
    bool eval_done;

    /**
     * Number of sprites available to the current rendering pipeline.
     */
    ubyte sprite_count;

    /**
     * Number of sprites selected for the next scanline.
     */
    ubyte next_sprite_count;

    /**
     * Horizontal positions of the current sprites.
     */
    ubyte x[8];

    /**
     * Attribute bytes of the current sprites.
     */
    ubyte attributes[8];

    /**
     * Low pattern-plane bytes of the current sprites.
     */
    ubyte pattern_low[8];

    /**
     * High pattern-plane bytes of the current sprites.
     */
    ubyte pattern_high[8];

    /**
     * Horizontal positions of the next scanline's sprites.
     */
    ubyte next_x[8];

    /**
     * Attribute bytes of the next scanline's sprites.
     */
    ubyte next_attributes[8];

    /**
     * Low pattern-plane bytes of the next scanline's sprites.
     */
    ubyte next_pattern_low[8];

    /**
     * High pattern-plane bytes of the next scanline's sprites.
     */
    ubyte next_pattern_high[8];

    /**
     * Temporary low sprite pattern byte.
     */
    ubyte pattern_low_latch;

    /**
     * Temporary high sprite pattern byte.
     */
    ubyte pattern_high_latch;

    /**
     * Temporary OAM data byte.
     */
    ubyte oam_data_latch;

    /**
     * Indicates that sprite zero may participate in the current scanline.
     */
    ubyte sprite_zero_possible;

    /**
     * Indicates that sprite zero is currently eligible for pixel rendering.
     */
    bool sprite_zero_rendering;

    /**
     * Indicates that a sprite-zero hit has occurred.
     */
    bool sprite_zero_hit;

    /**
     * Indicates that sprite evaluation detected overflow.
     */
    bool sprite_overflow;
  } sprite;

  /**
   * PPU memory bus and internal memory.
   */
  struct memory {
    /**
     * Internal nametable VRAM.
     *
     * Provides 4 KiB of storage for nametable memory and mirroring.
     */
    ubyte vram[0x1000];

    /**
     * Internal palette RAM.
     *
     * Stores background and sprite palette entries.
     */
    ubyte palette_ram[0x20];

    /**
     * External memory bus used to access cartridge expansion memory.
     */
    PPU_MemoryBus *expansion;

    /**
     * Internal PPUDATA read buffer.
     *
     * Holds buffered reads from PPU memory.
     */
    ubyte read_buffer;

    /**
     * Internal PPU memory I/O latch.
     */
    ubyte io_latch;

    /**
     * Palette-access I/O latch.
     */
    ubyte palette_io_latch;
  } bus;

  /**
   * PPU timing and frame state.
   */
  struct timing {
    /**
     * Active NES hardware timing region.
     */
    NES_Regions region;

    /**
     * Active nametable mirroring configuration.
     */
    PPU_Mirroring mirroring;

    /**
     * Total PPU dots executed.
     */
    counter cycles;

    /**
     * Number of frames processed.
     */
    counter frames;

    /**
     * Current scanline.
     */
    uword scanline;

    /**
     * Current dot within the scanline.
     */
    uword dot;

    /**
     * Number of scanlines in the active timing region.
     */
    uword total_scanlines;

    /**
     * Indicates whether the current frame is an odd frame.
     */
    bool odd_frame;

    /**
     * Indicates that a complete frame is ready for consumption.
     */
    bool frame_complete;

    /**
     * Indicates that rendering is currently active.
     */
    bool rendering;

    /**
     * Indicates that vertical blank has occurred.
     */
    bool nmi_occurred;

    /**
     * Current NMI output enable state from PPUCTRL.
     */
    bool nmi_output;

    /**
     * Current NMI signal line state.
     */
    bool nmi_line;

    /**
     * Indicates that an NMI request is pending.
     */
    bool nmi_pending;
  } timing;

  /**
   * OAM DMA transfer state.
   *
   * Represents the transfer of a CPU memory page into primary OAM.
   */
  struct dma {
    /**
     * Indicates that OAM DMA is active.
     */
    bool active;

    /**
     * Indicates that the DMA alignment or dummy phase is active.
     */
    bool dummy;

    /**
     * Indicates whether the current DMA phase reads source memory.
     */
    bool read_phase;

    /**
     * Source page selected for the DMA transfer.
     */
    ubyte page;

    /**
     * Current byte offset within the source page.
     */
    ubyte offset;

    /**
     * Byte temporarily held between a source read and OAM write.
     */
    ubyte data;

    /**
     * Number of DMA cycles elapsed.
     */
    counter cycles;
  } dma;

  /**
   * PPU I/O state and register access latches.
   */
  struct io {
    /**
     * Internal PPU I/O open-bus value.
     */
    ubyte open_bus;

    /**
     * Decay state for the eight open-bus data bits.
     */
    ubyte io_bus_decay[8];

    /**
     * Per-bit open-bus decay counters.
     */
    ubyte io_bus_decay_counter[8];

    /**
     * Fine horizontal scroll latch.
     */
    ubyte fine_x;

    /**
     * Shared first-or-second write toggle for PPUSCROLL and PPUADDR.
     */
    ubyte write_toggle;

    /**
     * Buffered value used by CPU reads from PPUDATA.
     */
    ubyte read_buffer;

    /**
     * Most recently accessed CPU-visible PPU register.
     */
    ubyte last_register;
  } io;

  /**
   * Pixel output and rendering configuration state.
   */
  struct rendering {
    /**
     * RGBA framebuffer for the 256 by 240 NES display.
     */
    ubyte framebuffer[256 * 240 * 4];

    /**
     * Current horizontal pixel coordinate.
     */
    uword pixel_x;

    /**
     * Current vertical pixel coordinate.
     */
    uword pixel_y;

    /**
     * Palette RAM index selected for the current pixel.
     */
    ubyte palette_index;

    /**
     * Encoded color value selected for the current pixel.
     */
    ubyte color;

    /**
     * Color emphasis bits applied to the output.
     */
    ubyte emphasis;

    /**
     * Indicates whether grayscale output is enabled.
     */
    bool grayscale;

    /**
     * Indicates whether background rendering is enabled in the leftmost
     * eight pixels.
     */
    bool left_background_enabled;

    /**
     * Indicates whether sprite rendering is enabled in the leftmost
     * eight pixels.
     */
    bool left_sprites_enabled;

    /**
     * Indicates whether background rendering is enabled.
     */
    bool background_enabled;

    /**
     * Indicates whether sprite rendering is enabled.
     */
    bool sprites_enabled;

    /**
     * Indicates whether background pixels should be rendered.
     */
    bool show_background;

    /**
     * Indicates whether sprite pixels should be rendered.
     */
    bool show_sprites;
  } render;

  /**
   * PPU non-maskable interrupt signal state.
   */
  struct interrupt {
    /**
     * Current NMI output line state.
     */
    bool nmi_line;

    /**
     * Indicates that an NMI edge is pending processing.
     */
    bool nmi_pending;

    /**
     * NMI line state observed during the previous timing step.
     */
    bool nmi_previous;
  } interrupt;

  /**
   * PPU initialization, reset, and vertical blank state.
   */
  struct state {
    /**
     * Indicates that the PPU has been initialized.
     */
    bool initialized;

    /**
     * Indicates that a PPU reset is pending.
     */
    bool reset_pending;

    /**
     * Indicates that rendering is enabled by the current configuration.
     */
    bool rendering_enabled;

    /**
     * Indicates that the vertical blank interval has started.
     */
    bool vblank_started;

    /**
     * Indicates that vertical blank has been suppressed.
     */
    bool vblank_suppressed;

    /**
     * Indicates that PPUSTATUS was read during the vertical blank interval.
     */
    bool status_read_during_vblank;

    /**
     * Indicates that vertical blank generation should be suppressed.
     */
    bool suppress_vblank;
  } state;

  /**
   * Temporary values used by PPU operations.
   */
  struct temp {
    /**
     * General-purpose temporary data byte.
     */
    ubyte data;

    /**
     * General-purpose temporary value.
     */
    ubyte value;

    /**
     * General-purpose temporary latch.
     */
    ubyte latch;

    /**
     * High byte of a temporary address.
     */
    ubyte address_high;

    /**
     * Low byte of a temporary address.
     */
    ubyte address_low;

    /**
     * General-purpose temporary address.
     */
    uword address;

    /**
     * Temporary VRAM or memory address.
     */
    uword temporary_address;

    /**
     * Address currently presented to the PPU memory bus.
     */
    uword bus_address;

    /**
     * Data currently presented to the PPU memory bus.
     */
    ubyte bus_data;

    /**
     * Indicates whether the current PPU memory bus operation is a write.
     */
    bool bus_write;
  } temp;
} NES_PPU;

/**
 * Write to a CPU-visible PPU register.
 *
 * Processes a write to the specified PPU register and updates the
 * corresponding register state, address latches, scrolling state,
 * memory interface, or control signals.
 *
 * The register argument identifies the CPU-visible PPU register, and
 * val contains the byte written by the CPU.
 */
void write_ppu_for_cpu(NES_PPU *p, ubyte reg, ubyte val);

/**
 * Read from a CPU-visible PPU register.
 *
 * Returns the value observed by the CPU when reading the specified
 * PPU register, including register-specific side effects, open-bus
 * behavior, and buffered memory access where applicable.
 *
 * A negative return value may be used to indicate an invalid access.
 */
word read_ppu_for_cpu(NES_PPU *p, ubyte reg);

/**
 * Advance the PPU by one clock dot.
 *
 * Advances PPU timing, rendering, memory accesses, sprite evaluation,
 * background fetches, pixel generation, and interrupt signal processing
 * according to the current machine state.
 *
 * The surrounding system is responsible for coordinating PPU memory
 * accesses and CPU execution.
 */
void clock_nes_ppu(NES_PPU *p);

/**
 * Read from cartridge expansion memory through the PPU memory bus.
 *
 * Should be implemented in BUS. Return -1 by default.
 */
word read_expansion_for_ppu(PPU_MemoryBus *membus, uword addr);

#endif // NES_PPU_H
