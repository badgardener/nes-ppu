# NES-PPU

A small NES Picture Processing Unit (PPU) emulator written in C for experimenting with NES-style memory mapping, timing, and rendering state.

Features:

- Minimal PPU core
- CPU-visible register emulation
- Bus-based memory access
- Nametable VRAM and palette RAM support
- Nametable mirroring modes
- Background and sprite pipeline state
- NTSC, PAL, and Dendy timing support
- NMI / vertical blank handling hooks

Logging **(Planned)**:

- PPU register read/write tracing
- VRAM and palette memory access debugging
- Frame, scanline, and dot timing inspection
- Rendering state inspection for debugging

This project is intended for learning and debugging the NES PPU timing model and rendering pipeline.
It is not a complete NES emulator or a production-ready game console implementation.

## Typical usage

The library is designed to be embedded into a larger emulator or debugger. The public interface exposes constructors, resets, register read/write helpers, and a clock function to advance the PPU state:

- `build_nes_ppu(...)`
- `reset_nes_ppu(...)`
- `write_ppu_for_cpu(...)`
- `read_ppu_for_cpu(...)`
- `clock_nes_ppu(...)`

These are intended to be used together with a system bus or cartridge abstraction that provides access to expansion ROM/RAM or other mapped memory.
