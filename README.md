# Chip 8 Emulator

This is a CHIP-8 emulator written in C using raylib.

<img src="screenshot.png" alt="Screenshot of the emulator running a program" width="720" />

## Build (macOS/Linux)

```bash
cmake -S . -B build-native
cmake --build build-native -j
```

## Run

```bash
./build-native/chip8_emulator <path-to-rom.ch8>
```
