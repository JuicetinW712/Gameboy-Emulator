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
    Display() {
        SDL_Init(SDL_INIT_VIDEO);
        window = SDL_CreateWindow("Gameboy", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
    }
    ~Display() {
        SDL_DestroyTexture(texture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }

    void redraw(const std::array<uint8_t, SCREEN_WIDTH * SCREEN_HEIGHT * 4>& buffer) {
        SDL_UpdateTexture(texture, NULL, buffer.data(), SCREEN_WIDTH * 4);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
    }
};

class PPU {
private:
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

    // static constexpr uint16_t SCREEN_WIDTH = 144;
    // static constexpr uint16_t SCREEN_HEIGHT = 160;
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

    uint16_t getTileAddress(uint8_t tileNumber) {
        uint8_t lcdControlValue = bus.read(LCDC_ADDRESS);
        bool tileAddressingMode = static_cast<bool>((lcdControlValue >> 4) & 1);

        uint16_t tileAddress = tileAddressingMode ?
            TILE_BLOCK_0_START + (TILE_BYTE_SIZE * tileNumber) :
            TILE_BLOCK_2_START + (TILE_BYTE_SIZE * static_cast<int8_t>(tileNumber));

        return tileAddress;
    }

    void setPixel(int x, int y, uint8_t value) {
        assert(value < 4 && "Color value should not exceed 2 bits");
        assert(x < SCREEN_WIDTH && x >= 0 && y < SCREEN_HEIGHT && y >= 0);

        // TODO check if these color values make sense?
        uint8_t rgbValue;
        switch (value & 0b11) {
            case 0x00: rgbValue = 0; break; // black
            case 0x01: rgbValue = 96; break;
            case 0x10: rgbValue = 192; break;
            case 0x11: rgbValue = 255; break; //
            default: throw std::out_of_range("Color value exceeds 2 bits - should not even be reachable");
        }

        // Set RGBA values
        int screenOffset = (y * SCREEN_WIDTH + x) * 4;
        frameBuffer[screenOffset] = rgbValue;
        frameBuffer[screenOffset + 1] = rgbValue;
        frameBuffer[screenOffset + 2] = rgbValue;
        frameBuffer[screenOffset + 3] = 255;
    }

public:
    explicit PPU(MMU& bus) : bus(bus), display(Display()) {}

    void drawScreen() {
        uint8_t lcdControlValue = bus.read(LCDC_ADDRESS);
        bool tileMapMode = static_cast<bool>((lcdControlValue >> 3) & 1);

        // Entire tile map is 256 * 256 which is way larger than the gameboy screen
        // meaning only part of the tile map is displayed
        // scX and scY define the starting offset which wraps around if too large
        uint8_t scX = bus.read(SCX_ADDRESS);
        uint8_t scY = bus.read(SCY_ADDRESS);

        // 1 = 9C00–9FFF; 0 = 9800–9BFF
        uint16_t tileMapStart = tileMapMode ? TILE_MAP_1_START : TILE_MAP_0_START;

        for (int y = 0; y < SCREEN_HEIGHT; y++) {
            for (int x = 0; x < SCREEN_WIDTH; x++) {
                uint8_t bgX = (x + scX) & 0xFF;
                uint8_t bgY = (y + scY) & 0xFF;

                // Screen is rendered 32 * 32 tiles
                uint8_t bgXTile = bgX / TILE_PIXEL_SIZE;
                uint8_t bgYTile = bgY / TILE_PIXEL_SIZE;

                // The tile map stores which tile number the current pixel corresponds to
                uint16_t tileMapOffset = (NUM_TILES_PER_COLUMN * bgYTile) + bgXTile;
                uint8_t tileNumber = bus.read(tileMapStart + tileMapOffset);

                // Get the specific pixel from that tile
                // Each tile is 2 bytes
                uint16_t tileAddress = getTileAddress(tileNumber);
                uint8_t tileX = bgX % TILE_PIXEL_SIZE;
                uint8_t tileY = bgY % TILE_PIXEL_SIZE;

                // Each row on a tile is represented by 2 bytes
                // The first byte contains the upper bits of the color
                // while the second byte contains the lower bits of the color
                uint8_t tileByte1 = bus.read(tileAddress + (2 * tileY));
                uint8_t tileByte2 = bus.read(tileAddress + (2 * tileY + 1));

                uint8_t upperColorBit = (tileByte1 >> (7 - tileX)) & 1;
                uint8_t lowerColorBit = (tileByte2 >> (7 - tileX)) & 1;
                uint8_t colorValue = (upperColorBit << 1) | lowerColorBit;

                setPixel(x, y, colorValue);
            }
        }
    }
};

