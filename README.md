# 🎮 Gameboy Emulator

A Gameboy emulator I wrote in C++. Supports MBC1 and MBC2 games

## 🚀 Getting Started

### Building from Source

To build the emulator, you will need the following prerequisites:

*   **CMake:** Version 3.10 or higher.
*   **SDL2:** Simple DirectMedia Layer development libraries.

Follow these steps to compile the emulator:

```bash
# Create a build directory
mkdir build
cd build

# Configure the project with CMake
cmake ..

# Compile the project
make
```

Upon successful compilation, an executable named `gameboy` will be created in the `build/` directory.

### Running the Emulator

To run a Gameboy ROM, execute the compiled `gameboy` executable followed by the path to your `.gb` ROM file.

```bash
./build/gameboy path/to/your/game.gb
```

#### Optional: Test Mode

You can enable a test mode to display detailed cartridge information in the console by using the `--test` flag:

```bash
./build/gameboy path/to/your/game.gb --test
```

## 🕹️ Key Bindings

The emulator maps standard keyboard keys to Gameboy controls:

| Gameboy Control | Keyboard Key |
| :-------------- | :----------- |
| **Right**       | `Right Arrow`|
| **Left**        | `Left Arrow` |
| **Up**          | `Up Arrow`   |
| **Down**        | `Down Arrow` |
| **A Button**    | `Z`          |
| **B Button**    | `X`          |
| **Select**      | `Spacebar`   |
| **Start**       | `Enter`      |

## 📸 Screenshots (TODO)
