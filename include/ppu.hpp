#pragma once
#include <cstdint>
#include <array>
#include <SDL2/SDL.h>

#include "mmu.hpp"

static constexpr uint16_t SCREEN_WIDTH = 144;
static constexpr uint16_t SCREEN_HEIGHT = 160;


class Display {
private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;

public:
    Display();
    ~Display();

    void redraw(const std::array<uint8_t, SCREEN_WIDTH * SCREEN_HEIGHT * 4>& buffer);
};

enum class PPU_MODE {
    HBLANK,
    VBLANK,
    OAM_SCAN,
    PIXEL_TRANSFER
};

class PPU {
private:
    PPU_MODE currentMode{PPU_MODE::OAM_SCAN};
    static constexpr int DOTS_PER_SCANLINE = 456;
    static constexpr int SCANLINES_PER_FRAME = 154;

    static constexpr uint16_t LCDC_ADDRESS = 0xFF40;
    static constexpr uint16_t STAT_ADDRESS = 0xFF41;
    static constexpr uint16_t SCY_ADDRESS = 0xFF42;
    static constexpr uint16_t SCX_ADDRESS = 0xFF43;
    static constexpr uint16_t LY_ADDRESS = 0xFF44;
    static constexpr uint16_t LYC_ADDRESS = 0xFF45;
    static constexpr uint16_t BGP_ADDRESS = 0xFF47;
    static constexpr uint16_t OBP0_ADDRESS = 0xFF48;
    static constexpr uint16_t OBP1_ADDRESS = 0xFF49;
    static constexpr uint16_t WX_ADDRESS = 0xFF4A;
    static constexpr uint16_t WY_ADDRESS = 0xFF4B;

    static constexpr uint16_t SCREEN_SIZE = SCREEN_WIDTH * SCREEN_HEIGHT;

    static constexpr uint16_t TILE_BLOCK_0_START = 0x8000;
    static constexpr uint16_t TILE_BLOCK_0_END = 0x87FF;
    static constexpr uint16_t TILE_BLOCK_1_START = 0x8800;
    static constexpr uint16_t TILE_BLOCK_1_END = 0x8FFF;
    static constexpr uint16_t TILE_BLOCK_2_START = 0x9000;
    static constexpr uint16_t TILE_BLOCK_2_END = 0x97FF;

    static constexpr uint16_t TILE_MAP_0_START = 0x9800;
    static constexpr uint16_t TILE_MAP_0_END = 0x9BFF;
    static constexpr uint16_t TILE_MAP_1_START = 0x9C00;
    static constexpr uint16_t TILE_MAP_1_END = 0x9FFF;

    static constexpr uint8_t TILE_BYTE_SIZE = 16;
    static constexpr uint8_t TILE_PIXEL_SIZE = 8;
    static constexpr uint8_t NUM_TILES_PER_COLUMN = SCREEN_WIDTH / TILE_PIXEL_SIZE;
    static constexpr uint8_t NUM_TILES_PER_ROW = SCREEN_HEIGHT / TILE_PIXEL_SIZE;

    std::array<uint8_t, SCREEN_SIZE * 4> frameBuffer{};
    MMU& bus;

    Display display;
    int m_dots{0};

    uint16_t getTileAddress(uint8_t tileNumber);
    void setPixel(int x, int y, uint8_t value);

public:
    explicit PPU(MMU& bus);

    void tick(int cycles);
    void drawScanline();
    void drawSprites();
    void drawWindow();
};