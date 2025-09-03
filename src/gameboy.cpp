#include "gameboy.hpp"

Gameboy::Gameboy(Cartridge& cartridge) : cartridge(cartridge), mmu(cartridge), cpu(mmu), ppu(mmu) {}

void Gameboy::run() {
    // Main emulation loop
    bool quit = false;
    SDL_Event event;

    while (!quit) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
            } else if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_RIGHT: mmu.handleKeyDown(0); break;
                    case SDLK_LEFT: mmu.handleKeyDown(1); break;
                    case SDLK_UP: mmu.handleKeyDown(2); break;
                    case SDLK_DOWN: mmu.handleKeyDown(3); break;
                    case SDLK_z: mmu.handleKeyDown(4); break; // A button
                    case SDLK_x: mmu.handleKeyDown(5); break; // B button
                    case SDLK_SPACE: mmu.handleKeyDown(6); break; // Select button
                    case SDLK_RETURN: mmu.handleKeyDown(7); break; // Start button
                    default: break;
                }
            } else if (event.type == SDL_KEYUP) {
                switch (event.key.keysym.sym) {
                    case SDLK_RIGHT: mmu.handleKeyUp(0); break;
                    case SDLK_LEFT: mmu.handleKeyUp(1); break;
                    case SDLK_UP: mmu.handleKeyUp(2); break;
                    case SDLK_DOWN: mmu.handleKeyUp(3); break;
                    case SDLK_z: mmu.handleKeyUp(4); break;
                    case SDLK_x: mmu.handleKeyUp(5); break;
                    case SDLK_SPACE: mmu.handleKeyUp(6); break;
                    case SDLK_RETURN: mmu.handleKeyUp(7); break;
                    default: break;
                }
            }
        }
        int cycles = cpu.cycle();
        ppu.tick(cycles);
        mmu.tick(cycles);
    }
}