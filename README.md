# Gameboy Emulator

This is a Gameboy emulator written in C++.

## Building

To build the emulator, you need CMake and SDL2.

```bash
mkdir build
cd build
cmake ..
make
```

## Running

To run the emulator, you need to provide a Gameboy ROM file.

```bash
./gameboy <rom_file> [--test]
```

The `--test` flag will print cartridge information to the console.
