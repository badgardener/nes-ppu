#include "nes-ppu.h"

#include <stdlib.h>
#include <string.h>

const ubyte NTSC_COLOR_PALETTE[64 * 3] = {
    0x57, 0x57, 0x57, 0x00, 0x0C, 0x8E, 0x08, 0x00, 0xA6, 0x34, 0x00, 0x96,
    0x55, 0x00, 0x61, 0x63, 0x00, 0x15, 0x5A, 0x00, 0x00, 0x3C, 0x0E, 0x00,
    0x11, 0x28, 0x00, 0x00, 0x3B, 0x00, 0x00, 0x42, 0x00, 0x00, 0x3A, 0x05,
    0x00, 0x26, 0x52, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xA5, 0xA5, 0xA5, 0x00, 0x41, 0xD9, 0x2F, 0x1E, 0xFF, 0x67, 0x04, 0xF2,
    0x94, 0x00, 0xB4, 0xAA, 0x00, 0x57, 0xA3, 0x18, 0x00, 0x80, 0x39, 0x00,
    0x4B, 0x5B, 0x00, 0x13, 0x76, 0x00, 0x00, 0x81, 0x00, 0x00, 0x79, 0x23,
    0x00, 0x62, 0x88, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xFF, 0x4A, 0x9F, 0xFF, 0x79, 0x7E, 0xFF, 0xAF, 0x63, 0xFF,
    0xDD, 0x55, 0xFF, 0xF7, 0x57, 0xC2, 0xF7, 0x6A, 0x63, 0xDC, 0x88, 0x10,
    0xAE, 0xA9, 0x00, 0x78, 0xC4, 0x00, 0x4A, 0xD2, 0x11, 0x2F, 0xCF, 0x64,
    0x2F, 0xBD, 0xC4, 0x41, 0x41, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xFF, 0xB9, 0xDD, 0xFF, 0xCA, 0xD1, 0xFF, 0xDE, 0xC6, 0xFF,
    0xF0, 0xC0, 0xFF, 0xFC, 0xC0, 0xEE, 0xFD, 0xC6, 0xCA, 0xF5, 0xD0, 0xAA,
    0xE4, 0xDD, 0x95, 0xD0, 0xE8, 0x92, 0xBD, 0xEE, 0xA2, 0xB2, 0xEE, 0xC0,
    0xB0, 0xE8, 0xE3, 0xB3, 0xB3, 0xB3, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

const ubyte PAL_COLOR_PALETTE[64 * 3] = {
    0x62, 0x62, 0x62, 0x00, 0x23, 0x65, 0x0D, 0x10, 0x7F, 0x2B, 0x01, 0x7F,
    0x45, 0x00, 0x65, 0x54, 0x00, 0x37, 0x54, 0x05, 0x01, 0x45, 0x15, 0x00,
    0x2B, 0x28, 0x00, 0x0D, 0x37, 0x00, 0x00, 0x3F, 0x00, 0x00, 0x3E, 0x01,
    0x00, 0x33, 0x37, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xAB, 0xAB, 0xAB, 0x11, 0x51, 0xAA, 0x34, 0x38, 0xCE, 0x5C, 0x23, 0xCE,
    0x7F, 0x19, 0xAA, 0x93, 0x1A, 0x6C, 0x93, 0x28, 0x24, 0x7F, 0x3F, 0x00,
    0x5C, 0x58, 0x00, 0x34, 0x6C, 0x00, 0x11, 0x77, 0x00, 0x00, 0x75, 0x24,
    0x00, 0x67, 0x6C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xFF, 0x61, 0xA1, 0xFB, 0x84, 0x89, 0xFF, 0xAD, 0x74, 0xFF,
    0xD0, 0x69, 0xFB, 0xE4, 0x6B, 0xBC, 0xE4, 0x79, 0x74, 0xD0, 0x8F, 0x36,
    0xAD, 0xA8, 0x12, 0x84, 0xBD, 0x12, 0x61, 0xC8, 0x36, 0x4D, 0xC6, 0x74,
    0x4D, 0xB8, 0xBC, 0x4E, 0x4E, 0x4E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xFF, 0xC4, 0xDD, 0xFF, 0xD1, 0xD3, 0xFF, 0xE1, 0xCB, 0xFF,
    0xEF, 0xC7, 0xFF, 0xF7, 0xC8, 0xE7, 0xF7, 0xCD, 0xCB, 0xEF, 0xD6, 0xB3,
    0xE1, 0xDF, 0xA5, 0xD1, 0xE7, 0xA5, 0xC4, 0xEC, 0xB3, 0xBC, 0xEB, 0xCB,
    0xBC, 0xE5, 0xE7, 0xB8, 0xB8, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

const ubyte DENDY_COLOR_PALETTE[64 * 3] = {
    0x66, 0x66, 0x66, 0x00, 0x2A, 0x88, 0x14, 0x12, 0xA7, 0x3B, 0x00, 0xA4,
    0x5C, 0x00, 0x7E, 0x6E, 0x00, 0x40, 0x6C, 0x06, 0x00, 0x56, 0x1D, 0x00,
    0x33, 0x35, 0x00, 0x0B, 0x48, 0x00, 0x00, 0x52, 0x00, 0x00, 0x4F, 0x08,
    0x00, 0x40, 0x4D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xAD, 0xAD, 0xAD, 0x15, 0x5F, 0xD9, 0x42, 0x40, 0xFF, 0x75, 0x27, 0xFE,
    0xA0, 0x1A, 0xCC, 0xB7, 0x1E, 0x7B, 0xB5, 0x31, 0x20, 0x99, 0x4E, 0x00,
    0x6B, 0x6D, 0x00, 0x38, 0x87, 0x00, 0x0C, 0x93, 0x00, 0x00, 0x8F, 0x32,
    0x00, 0x7C, 0x8D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0xFE, 0xFF, 0x64, 0xB0, 0xFF, 0x92, 0x90, 0xFF, 0xC6, 0x76, 0xFF,
    0xF3, 0x6A, 0xFF, 0xFE, 0x6E, 0xCC, 0xFE, 0x81, 0x70, 0xEA, 0x9E, 0x22,
    0xBC, 0xBE, 0x00, 0x88, 0xD8, 0x00, 0x5C, 0xE4, 0x30, 0x45, 0xE0, 0x82,
    0x48, 0xCD, 0xDE, 0x4F, 0x4F, 0x4F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0xFE, 0xFF, 0xC0, 0xDF, 0xFF, 0xD3, 0xD2, 0xFF, 0xE8, 0xC8, 0xFF,
    0xFB, 0xC2, 0xFF, 0xFE, 0xC4, 0xEA, 0xFE, 0xCC, 0xC5, 0xF7, 0xD8, 0xA5,
    0xE4, 0xE5, 0x94, 0xCF, 0xEF, 0x96, 0xBD, 0xF4, 0xAB, 0xB3, 0xF3, 0xCC,
    0xB5, 0xEB, 0xF2, 0xB8, 0xB8, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

static inline uword mirror_nametable(NES_PPU *p, uword addr) {
  uword offset = addr & 0x0FFF;
  uword table = offset >> 10;
  uword index = offset & 0x03FF;

  switch (p->timing.mirroring) {
  case MIRROR_HORIZONTAL:
    table >>= 1;
    break;
  case MIRROR_VERTICAL:
    table &= 1;
    break;
  case MIRROR_SINGLE0:
    table = 0;
    break;
  case MIRROR_SINGLE1:
    table = 1;
    break;
  case MIRROR_FOURSCREEN:
    break;
  }

  return (table << 10) | index;
}

static inline ubyte mirror_palette(uword addr) {
  ubyte index = addr & 0x1F;

  if ((index & 0x13) == 0x10) {
    index -= 0x10;
  }

  return index;
}

static inline ubyte get_color_R(NES_PPU *p, ubyte i) {
  return p->PALETTE[i * 3];
}
static inline ubyte get_color_G(NES_PPU *p, ubyte i) {
  return p->PALETTE[(i * 3) + 1];
}
static inline ubyte get_color_B(NES_PPU *p, ubyte i) {
  return p->PALETTE[(i * 3) + 2];
}

static inline void set_color_R(NES_PPU *p, uword px, ubyte R) {
  p->render.framebuffer[px * 3] = R;
}
static inline void set_color_G(NES_PPU *p, uword px, ubyte G) {
  p->render.framebuffer[(px * 3) + 1] = G;
}
static inline void set_color_B(NES_PPU *p, uword px, ubyte B) {
  p->render.framebuffer[(px * 3) + 2] = B;
}

static inline void set_color_RGB(NES_PPU *p, uword px, ubyte i) {
  set_color_R(p, px, get_color_R(p, i));
  set_color_G(p, px, get_color_G(p, i));
  set_color_B(p, px, get_color_B(p, i));
}

static void write_ppu(NES_PPU *p, uword addr, ubyte val) {
  addr &= 0x3FFF;
  p->io.open_bus = val;

  if (addr <= 0x1FFF) {
    write_expansion_for_ppu(p->bus.expansion, addr, val);
    return;
  }

  if (addr >= 0x3000 && addr <= 0x3EFF) {
    addr -= 0x1000;
  }

  if (addr <= 0x2FFF) {
    p->bus.vram[mirror_nametable(p, addr - 0x2000)] = val;
    return;
  }

  p->bus.palette_ram[mirror_palette(addr)] = val & 0x3F;
}

static bool read_ppu(NES_PPU *p, uword addr, ubyte *result) {
  addr &= 0x3FFF;

  if (addr <= 0x1FFF) {
    word data = read_expansion_for_ppu(p->bus.expansion, addr);

    if (data < 0 || data > 0xFF) {
      return false;
    }

    *result = (ubyte)data;
  } else {
    if (addr >= 0x3000 && addr <= 0x3EFF) {
      addr -= 0x1000;
    }

    if (addr <= 0x2FFF) {
      *result = p->bus.vram[mirror_nametable(p, addr - 0x2000)];
    } else {
      *result = (p->bus.palette_ram[mirror_palette(addr)] & 0x3F) |
                (p->io.open_bus & 0xC0);
    }
  }

  p->io.open_bus = *result;
  return true;
}

static inline void update_nmi_line(NES_PPU *p) {
  bool line = p->timing.nmi_output && p->timing.nmi_occurred;

  if (line && !p->interrupt.nmi_previous) {
    p->interrupt.nmi_pending = true;
  }

  p->timing.nmi_line = line;
  p->interrupt.nmi_previous = line;
}

void write_ppu_for_cpu(NES_PPU *p, ubyte reg, ubyte val) {
  if (!p) {
    return;
  }

  reg &= 7;
  p->io.open_bus = val;

  switch (reg) {
  case 0: {
    p->reg.PPUCTRL = val;
    p->loopy.t = (p->loopy.t & 0xF3FF) | ((uword)(val & 3) << 10);
    p->timing.nmi_output = (val & 0x80) != 0;
    update_nmi_line(p);
    break;
  }

  case 1: {
    p->reg.PPUMASK = val;
    p->render.grayscale = (val & 1) != 0;
    p->render.left_background_enabled = (val & 2) != 0;
    p->render.left_sprites_enabled = (val & 4) != 0;
    p->render.background_enabled = (val & 8) != 0;
    p->render.sprites_enabled = (val & 0x10) != 0;
    p->render.emphasis = (val >> 5) & 7;
    p->timing.rendering = (val & 0x18) != 0;
    break;
  }

  case 3: {
    p->reg.OAMADDR = val;
    break;
  }

  case 4: {
    p->sprite.oam[p->reg.OAMADDR++] = val;
    break;
  }

  case 5: {
    if (!p->io.write_toggle) {
      p->loopy.fine_x = val & 7;
      p->loopy.t = (p->loopy.t & 0xFFE0) | (val >> 3);
      p->io.write_toggle = true;
    } else {
      p->loopy.t = (p->loopy.t & 0x8FFF) | ((uword)(val & 7) << 12);
      p->loopy.t = (p->loopy.t & 0xFC1F) | ((uword)(val & 0xF8) << 2);
      p->io.write_toggle = false;
    }

    break;
  }

  case 6: {
    if (!p->io.write_toggle) {
      p->loopy.t = (p->loopy.t & 0x80FF) | ((uword)(val & 0x3F) << 8);
      p->io.write_toggle = true;
    } else {
      p->loopy.t = (p->loopy.t & 0xFF00) | val;
      p->loopy.v = p->loopy.t;
      p->io.write_toggle = false;
    }

    break;
  }

  case 7: {
    write_ppu(p, p->loopy.v, val);
    p->loopy.v = (p->loopy.v + ((p->reg.PPUCTRL & 4) ? 32 : 1)) & 0x7FFF;
    break;
  }
  }
}

word read_ppu_for_cpu(NES_PPU *p, ubyte reg) {
  if (!p) {
    return -1;
  }

  reg &= 7;
  ubyte val = p->io.open_bus;

  switch (reg) {
  case 2: {
    val = (p->reg.PPUSTATUS & 0xE0) | (p->io.open_bus & 0x1F);
    p->reg.PPUSTATUS &= 0x7F;
    p->timing.nmi_occurred = false;
    p->state.status_read_during_vblank = p->state.vblank_started;
    p->state.vblank_suppressed |= !p->state.vblank_started;
    p->io.write_toggle = false;
    update_nmi_line(p);
    break;
  }

  case 4: {
    val = p->sprite.oam[p->reg.OAMADDR];
    break;
  }

  case 7: {
    uword addr = p->loopy.v & 0x3FFF;

    if (addr >= 0x3F00) {
      ubyte buffered;
      if (read_ppu(p, addr, &val)) {
        p->bus.read_buffer = read_ppu(p, addr - 0x1000, &buffered)
                                 ? buffered
                                 : p->bus.read_buffer;
      } else {
        val = p->io.open_bus;
      }
    } else {
      val = p->bus.read_buffer;
      ubyte buffered;

      if (read_ppu(p, addr, &buffered)) {
        p->bus.read_buffer = buffered;
      }
    }

    p->loopy.v = (p->loopy.v + ((p->reg.PPUCTRL & 4) ? 32 : 1)) & 0x7FFF;
    break;
  }
  }

  p->io.open_bus = val;
  p->io.last_register = reg;
  return val;
}

static inline void choose_palette(NES_PPU *p) {
  switch (p->timing.region) {
  case REGION_NTSC: {
    p->PALETTE = NTSC_COLOR_PALETTE;
    break;
  }

  case REGION_PAL: {
    p->PALETTE = PAL_COLOR_PALETTE;
    break;
  }

  case REGION_DENDY: {
    p->PALETTE = DENDY_COLOR_PALETTE;
    break;
  }
  }
}

NES_PPU *build_nes_ppu(NES_Regions r, PPU_MemoryBus *b, PPU_Mirroring m) {
  NES_PPU *ppu = calloc(1, sizeof(NES_PPU));

  if (ppu) {
    ppu->timing.region = r;
    ppu->timing.mirroring = m;
    ppu->bus.expansion = b;
    choose_palette(ppu);

    for (uword i = 0; i < (256 * 240); i++) {
      set_color_R(ppu, i, get_color_R(ppu, 0));
      set_color_G(ppu, i, get_color_G(ppu, 0));
      set_color_B(ppu, i, get_color_B(ppu, 0));
    }
  }

  return ppu;
}

void reset_nes_ppu(NES_PPU *p) {
  if (p) {
    NES_Regions r = p->timing.region;
    PPU_MemoryBus *b = p->bus.expansion;
    PPU_Mirroring m = p->timing.mirroring;
    counter steps = p->steps;

    memset(p, 0, sizeof(NES_PPU));
    p->timing.region = r;
    p->timing.mirroring = m;
    p->bus.expansion = b;
    p->steps = steps;

    choose_palette(p);

    for (uword i = 0; i < (256 * 240); i++) {
      set_color_R(p, i, get_color_R(p, 0));
      set_color_G(p, i, get_color_G(p, 0));
      set_color_B(p, i, get_color_B(p, 0));
    }
  }
}

static void __clock_nes_ppu_ntsc(NES_PPU *p) {
  uword scanline = p->timing.scanline;
  uword dot = p->timing.dot;
  bool rendering = p->render.background_enabled || p->render.sprites_enabled;
  bool bg_enabled = p->render.background_enabled;
  bool spr_enabled = p->render.sprites_enabled;
  bool visible = scanline < 240;
  bool prerender = scanline == 261;
  bool render_line = visible || prerender;
  bool fetch_line = render_line && rendering;

  if (dot == 0) {
    p->sprite.sprite_count = p->sprite.next_sprite_count;
    p->sprite.sprite_zero_possible = p->sprite.sprite_zero_rendering;
    p->sprite.sprite_zero_rendering = false;

    for (int i = 0; i < 8; i++) {
      p->sprite.x[i] = p->sprite.next_x[i];
      p->sprite.attributes[i] = p->sprite.next_attributes[i];
      p->sprite.pattern_low[i] = p->sprite.next_pattern_low[i];
      p->sprite.pattern_high[i] = p->sprite.next_pattern_high[i];
    }
  }

  if (render_line && dot >= 1 && dot <= 256 && visible) {
    uword px = dot - 1;
    p->render.pixel_x = px;
    p->render.pixel_y = scanline;

    ubyte bg_pixel = 0;
    ubyte bg_palette = 0;

    if (bg_enabled && (px >= 8 || p->render.left_background_enabled)) {
      unsigned int shift = 15 - p->loopy.fine_x;
      ubyte p0 = (p->bg.pattern_shift_low >> shift) & 1;
      ubyte p1 = (p->bg.pattern_shift_high >> shift) & 1;
      ubyte a0 = (p->bg.attribute_shift_low >> shift) & 1;
      ubyte a1 = (p->bg.attribute_shift_high >> shift) & 1;

      bg_pixel = (p1 << 1) | p0;
      bg_palette = (a1 << 1) | a0;
    }

    ubyte spr_pixel = 0;
    ubyte spr_palette = 0;
    bool spr_priority = false;
    bool spr_is_zero = false;

    if (spr_enabled && (px >= 8 || p->render.left_sprites_enabled)) {
      for (int i = 0; i < p->sprite.sprite_count && i < 8; i++) {
        if (p->sprite.x[i] != 0) {
          continue;
        }

        ubyte attr = p->sprite.attributes[i];
        ubyte s0 = (p->sprite.pattern_low[i] >> 7) & 1;
        ubyte s1 = (p->sprite.pattern_high[i] >> 7) & 1;
        ubyte sp = (s1 << 1) | s0;

        if (sp != 0) {
          spr_pixel = sp;
          spr_palette = 0x10 | ((attr & 3) << 2);
          spr_priority = (attr & 0x20) != 0;
          spr_is_zero = i == 0 && p->sprite.sprite_zero_possible;
          break;
        }
      }
    }

    if (spr_is_zero && bg_pixel != 0 && spr_pixel != 0 && bg_enabled &&
        spr_enabled && px != 255) {
      p->reg.PPUSTATUS |= 0x40;
    }

    ubyte final_palette_index;

    if (bg_pixel == 0 && spr_pixel == 0) {
      final_palette_index = 0;
    } else if (bg_pixel == 0) {
      final_palette_index = spr_palette | spr_pixel;
    } else if (spr_pixel == 0 || spr_priority) {
      final_palette_index = (bg_palette << 2) | bg_pixel;
    } else {
      final_palette_index = spr_palette | spr_pixel;
    }

    ubyte pal_addr = mirror_palette(0x3F00 + final_palette_index);
    ubyte color_idx = p->bus.palette_ram[pal_addr] & 0x3F;

    if (p->render.grayscale) {
      color_idx &= 0x30;
    }

    p->render.palette_index = final_palette_index;
    p->render.color = color_idx;
    set_color_RGB(p, px + scanline * 256, color_idx);
  }

  if (fetch_line && ((dot >= 1 && dot <= 256) || (dot >= 321 && dot <= 336))) {
    p->bg.pattern_shift_low <<= 1;
    p->bg.pattern_shift_high <<= 1;
    p->bg.attribute_shift_low <<= 1;
    p->bg.attribute_shift_high <<= 1;

    if (bg_enabled) {
      ubyte phase = (dot - 1) & 7;

      if (phase == 0) {
        ubyte tile = 0;
        if (read_ppu(p, 0x2000 | (p->loopy.v & 0x0FFF), &tile)) {
          p->bg.next_tile_id = tile;
        } else {
          p->bg.next_tile_id = 0;
        }
      } else if (phase == 2) {
        uword v = p->loopy.v;
        uword addr =
            0x23C0 | (v & 0x0C00) | ((v >> 4) & 0x38) | ((v >> 2) & 0x07);
        ubyte attr = 0;

        if (read_ppu(p, addr, &attr)) {
          ubyte shift = (ubyte)((((v >> 4) & 4) | (v & 2)) << 1);
          p->bg.next_tile_attribute = (attr >> shift) & 3;
        } else {
          p->bg.next_tile_attribute = 0;
        }
      } else if (phase == 4 || phase == 6) {
        uword fine_y = (p->loopy.v >> 12) & 7;
        uword table = (p->reg.PPUCTRL & 0x10) ? 0x1000 : 0;
        uword addr = table + (uword)p->bg.next_tile_id * 16 + fine_y +
                     (phase == 6 ? 8 : 0);
        ubyte pattern = 0;

        if (!read_ppu(p, addr, &pattern)) {
          pattern = 0;
        }

        if (phase == 4) {
          p->bg.next_tile_pattern_low = pattern;
        } else {
          p->bg.next_tile_pattern_high = pattern;
        }
      } else if (phase == 7) {
        p->bg.pattern_shift_low =
            (p->bg.pattern_shift_low & 0xFF00) | p->bg.next_tile_pattern_low;
        p->bg.pattern_shift_high =
            (p->bg.pattern_shift_high & 0xFF00) | p->bg.next_tile_pattern_high;

        p->bg.attribute_shift_low =
            (p->bg.attribute_shift_low & 0xFF00) |
            ((p->bg.next_tile_attribute & 1) ? 0xFF : 0);
        p->bg.attribute_shift_high =
            (p->bg.attribute_shift_high & 0xFF00) |
            ((p->bg.next_tile_attribute & 2) ? 0xFF : 0);

        if ((p->loopy.v & 0x001F) == 31) {
          p->loopy.v &= (uword)~0x001F;
          p->loopy.v ^= 0x0400;
        } else {
          p->loopy.v++;
        }
      }
    } else if ((dot & 7) == 0) {
      p->bg.pattern_shift_low <<= 1;
      p->bg.pattern_shift_high <<= 1;
      p->bg.attribute_shift_low <<= 1;
      p->bg.attribute_shift_high <<= 1;
    }
  }

  if (fetch_line && dot == 256) {
    uword v = p->loopy.v;

    if ((v & 0x7000) != 0x7000) {
      v += 0x1000;
    } else {
      v &= (uword)~0x7000;
      uword y = (v & 0x03E0) >> 5;

      if (y == 29) {
        y = 0;
        v ^= 0x0800;
      } else if (y == 31) {
        y = 0;
      } else {
        y++;
      }

      v = (v & (uword)~0x03E0) | (y << 5);
    }

    p->loopy.v = v;
  }

  if (fetch_line && dot == 257) {
    p->loopy.v = (p->loopy.v & 0xFBE0) | (p->loopy.t & 0x041F);
  }

  if (prerender && fetch_line && dot >= 280 && dot <= 304) {
    p->loopy.v = (p->loopy.v & 0x841F) | (p->loopy.t & 0x7BE0);
  }

  if (render_line && dot >= 1 && dot <= 64 && (dot & 1) == 0) {
    p->sprite.secondary_oam[(dot >> 1) - 1] = 0xFF;
  }

  if (render_line && dot == 1) {
    p->sprite.secondary_oam_addr = 0;
    p->sprite.eval_n = 0;
    p->sprite.eval_m = 0;
    p->sprite.eval_latch = 0xFF;
    p->sprite.eval_count = 0;
    p->sprite.eval_sec_addr = 0;
    p->sprite.eval_done = false;
    p->sprite.sprite_overflow = false;
    p->sprite.sprite_zero_rendering = false;
  }

  if (render_line && rendering && dot >= 65 && dot <= 256) {
    if (dot & 1) {
      if (p->sprite.eval_n < 64) {
        p->sprite.eval_latch =
            p->sprite.oam[(uword)p->sprite.eval_n * 4 + p->sprite.eval_m];
      } else {
        p->sprite.eval_latch = 0xFF;
      }
    } else if (!p->sprite.eval_done) {
      uword target = prerender ? 0 : scanline + 1;
      uword height = (p->reg.PPUCTRL & 0x20) ? 16 : 8;

      if (p->sprite.eval_n >= 64) {
        p->sprite.eval_done = true;
      } else if (p->sprite.eval_count < 8) {
        if (p->sprite.eval_m == 0) {
          ubyte y = p->sprite.eval_latch;
          ubyte row = (ubyte)(target - (uword)y - 1);

          if (row < height) {
            p->sprite.secondary_oam[p->sprite.eval_sec_addr++] = y;
            p->sprite.eval_m = 1;

            if (p->sprite.eval_n == 0) {
              p->sprite.sprite_zero_rendering = true;
            }
          } else {
            p->sprite.eval_n++;
          }
        } else {
          p->sprite.secondary_oam[p->sprite.eval_sec_addr++] =
              p->sprite.eval_latch;
          p->sprite.eval_m++;

          if (p->sprite.eval_m == 4) {
            p->sprite.eval_m = 0;
            p->sprite.eval_n++;
            p->sprite.eval_count++;
          }
        }

        if (p->sprite.eval_sec_addr >= 32) {
          p->sprite.eval_count = 8;
        }
      } else {
        ubyte row = (ubyte)(target - (uword)p->sprite.eval_latch - 1);

        if (row < height) {
          p->reg.PPUSTATUS |= 0x20;
          p->sprite.sprite_overflow = true;
          p->sprite.eval_m = (p->sprite.eval_m + 1) & 3;
        }

        p->sprite.eval_n++;
      }
    }
  }

  if (render_line && dot >= 257 && dot <= 320) {
    uword slot = (dot - 257) >> 3;
    uword phase = (dot - 257) & 7;

    if (dot == 257) {
      p->sprite.next_sprite_count = p->sprite.eval_count;
      for (int i = 0; i < 8; i++) {
        p->sprite.next_x[i] = 0xFF;
        p->sprite.next_attributes[i] = 0xFF;
        p->sprite.next_pattern_low[i] = 0;
        p->sprite.next_pattern_high[i] = 0;
      }
    }

    if (fetch_line && phase == 0) {
      ubyte ignored = 0;
      read_ppu(p, 0x2000 | (p->loopy.v & 0x0FFF), &ignored);
    }

    if (slot < 8 && slot < p->sprite.eval_count) {
      ubyte y = p->sprite.secondary_oam[slot * 4];
      ubyte tile = p->sprite.secondary_oam[slot * 4 + 1];
      ubyte attr = p->sprite.secondary_oam[slot * 4 + 2];
      ubyte x = p->sprite.secondary_oam[slot * 4 + 3];

      if (phase == 0) {
        p->sprite.next_x[slot] = x;
        p->sprite.next_attributes[slot] = attr;
      }

      if (fetch_line && (phase == 4 || phase == 6)) {
        uword target = prerender ? 0 : scanline + 1;
        uword height = (p->reg.PPUCTRL & 0x20) ? 16 : 8;
        uword row = (ubyte)(target - (uword)y - 1);

        if (attr & 0x80) {
          row = height - 1 - row;
        }

        uword addr;

        if (height == 16) {
          uword table = (tile & 1) ? 0x1000 : 0;
          uword tile_index = tile & 0xFE;

          if (row >= 8) {
            tile_index++;
            row -= 8;
          }

          addr = table + tile_index * 16 + row;
        } else {
          uword table = (p->reg.PPUCTRL & 0x08) ? 0x1000 : 0;
          addr = table + (uword)tile * 16 + row;
        }

        if (phase == 6) {
          addr += 8;
        }

        ubyte value = 0;
        if (!read_ppu(p, addr, &value)) {
          value = 0;
        }

        if (phase == 4) {
          p->sprite.next_pattern_low[slot] = value;
        } else {
          p->sprite.next_pattern_high[slot] = value;
        }
      }
    } else if (fetch_line && (phase == 4 || phase == 6)) {
      ubyte ignored = 0;
      uword addr = phase == 4 ? 0x0000 : 0x0008;
      read_ppu(p, addr, &ignored);
    }
  }

  if (visible && dot >= 1 && dot <= 256 && rendering) {
    for (int i = 0; i < p->sprite.sprite_count && i < 8; i++) {
      if (p->sprite.x[i] != 0) {
        p->sprite.x[i]--;
      } else {
        p->sprite.pattern_low[i] <<= 1;
        p->sprite.pattern_high[i] <<= 1;
      }
    }
  }

  if (scanline == 241 && dot == 1) {
    if (!p->state.suppress_vblank && !p->state.vblank_suppressed &&
        !p->state.status_read_during_vblank) {
      p->timing.nmi_occurred = true;
      p->reg.PPUSTATUS |= 0x80;
      p->state.vblank_started = true;
      update_nmi_line(p);
      p->timing.frame_complete = true;
    }

    p->state.suppress_vblank = false;
    p->state.vblank_suppressed = false;
    p->state.status_read_during_vblank = false;
  }

  if (prerender && dot == 1) {
    p->timing.nmi_occurred = false;
    p->reg.PPUSTATUS &= (ubyte)~0xE0;
    p->state.vblank_started = false;
    p->state.vblank_suppressed = false;
    p->state.status_read_during_vblank = false;
    update_nmi_line(p);
  }

  if (prerender && rendering && dot == 339 && p->timing.odd_frame) {
    p->timing.dot = 0;
    p->timing.scanline = 0;
    p->timing.frames++;
    p->timing.odd_frame = false;
    p->timing.frame_complete = false;
    return;
  }

  if (dot == 340) {
    p->timing.dot = 0;

    if (scanline == 261) {
      p->timing.scanline = 0;
      p->timing.frames++;
      p->timing.odd_frame = !p->timing.odd_frame;
      p->timing.frame_complete = false;
    } else {
      p->timing.scanline = scanline + 1;
    }
  } else {
    p->timing.dot = dot + 1;
  }
}

static void __clock_nes_ppu_pal(NES_PPU *p) {
  /** ::TODO::
   * For now simply clock NTSC
   */

  __clock_nes_ppu_ntsc(p);
}

static void __clock_nes_ppu_dendy(NES_PPU *p) {
  /** ::TODO::
   * For now simply clock NTSC
   */

  __clock_nes_ppu_ntsc(p);
}

void clock_nes_ppu(NES_PPU *p) {
  if (!p) {
    return;
  }

  p->steps++;

  switch (p->timing.region) {
  case REGION_NTSC: {
    __clock_nes_ppu_ntsc(p);
    break;
  }

  case REGION_PAL: {
    __clock_nes_ppu_pal(p);
    break;
  }

  case REGION_DENDY: {
    __clock_nes_ppu_dendy(p);
    break;
  }
  }
}
