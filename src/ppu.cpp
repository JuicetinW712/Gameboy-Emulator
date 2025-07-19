#include "ppu.hpp"
#include <cassert>
#include <stdexcept>
#include <cstddef>

Display::Display() {
    SDL_Init(SDL_INIT_VIDEO);
    window = SDL_CreateWindow("Gameboy", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
}

Display::~Display() {
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void Display::redraw(const std::array<uint8_t, SCREEN_WIDTH * SCREEN_HEIGHT * 4>& buffer) {
    SDL_UpdateTexture(texture, NULL, buffer.data(), SCREEN_WIDTH * 4);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}

PPU::PPU(MMU& bus) : bus(bus), display(Display()) {}

void PPU::tick(int cycles) {
    m_dots += cycles;

    uint8_t currentScanline = bus.read(LY_ADDRESS);
    uint8_t lcdc = bus.read(LCDC_ADDRESS);
    bool lcdEnabled = (lcdc >> 7) & 1;

    if (!lcdEnabled) {
        // If LCD is disabled, reset scanline and mode
        bus.write(LY_ADDRESS, 0);
        m_dots = 0;
        currentMode = PPU_MODE::HBLANK;
        return;
    }

    PPU_MODE newMode = currentMode;

    switch (currentMode) {
        case PPU_MODE::HBLANK:
            if (m_dots >= DOTS_PER_SCANLINE - 80) { // OAM Scan takes 80 dots
                newMode = PPU_MODE::OAM_SCAN;
            }
            break;
        case PPU_MODE::VBLANK:
            if (m_dots >= DOTS_PER_SCANLINE) {
                m_dots -= DOTS_PER_SCANLINE;
                currentScanline++;
                bus.write(LY_ADDRESS, currentScanline);

                if (currentScanline > 153) {
                    newMode = PPU_MODE::OAM_SCAN;
                    bus.write(LY_ADDRESS, 0);
                    display.redraw(frameBuffer);
                }
            }
            break;
        case PPU_MODE::OAM_SCAN:
            if (m_dots >= 80) {
                newMode = PPU_MODE::PIXEL_TRANSFER;
            }
            break;
        case PPU_MODE::PIXEL_TRANSFER:
            if (m_dots >= 80 + 172) { // OAM Scan + Pixel Transfer = 80 + 172 = 252 dots
                newMode = PPU_MODE::HBLANK;
                drawScanline();
            }
            break;
    }

    if (newMode != currentMode) {
        currentMode = newMode;
        uint8_t stat = bus.read(STAT_ADDRESS);
        stat &= 0xFC; // Clear mode bits
        stat |= static_cast<uint8_t>(currentMode);
        bus.write(STAT_ADDRESS, stat);

        // Request STAT interrupts
        if (currentMode == PPU_MODE::HBLANK && ((stat >> 3) & 1)) {
            bus.requestInterrupt(0x02); // LCD STAT interrupt
        } else if (currentMode == PPU_MODE::VBLANK && ((stat >> 4) & 1)) {
            bus.requestInterrupt(0x02); // LCD STAT interrupt
        } else if (currentMode == PPU_MODE::OAM_SCAN && ((stat >> 5) & 1)) {
            bus.requestInterrupt(0x02); // LCD STAT interrupt
        }
    }

    // LYC == LY interrupt
    if (bus.read(LY_ADDRESS) == bus.read(LYC_ADDRESS)) {
        uint8_t stat = bus.read(STAT_ADDRESS);
        stat |= 0x04; // Set coincidence flag
        bus.write(STAT_ADDRESS, stat);
        if (((stat >> 6) & 1)) {
            bus.requestInterrupt(0x02); // LCD STAT interrupt
        }
    } else {
        uint8_t stat = bus.read(STAT_ADDRESS);
        stat &= ~0x04; // Clear coincidence flag
        bus.write(STAT_ADDRESS, stat);
    }

    if (currentScanline == 144 && currentMode == PPU_MODE::VBLANK) {
        bus.requestInterrupt(0x01); // V-blank interrupt
    }

    if (m_dots >= DOTS_PER_SCANLINE) {
        m_dots -= DOTS_PER_SCANLINE;
        currentScanline++;
        bus.write(LY_ADDRESS, currentScanline);

        if (currentScanline == 144) {
            newMode = PPU_MODE::VBLANK;
        } else if (currentScanline > 153) {
            newMode = PPU_MODE::OAM_SCAN;
            bus.write(LY_ADDRESS, 0);
            display.redraw(frameBuffer);
        } else {
            newMode = PPU_MODE::OAM_SCAN;
        }
    }

    if (newMode != currentMode) {
        currentMode = newMode;
        uint8_t stat = bus.read(STAT_ADDRESS);
        stat &= 0xFC; // Clear mode bits
        stat |= static_cast<uint8_t>(currentMode);
        bus.write(STAT_ADDRESS, stat);

        // Request STAT interrupts
        if (currentMode == PPU_MODE::HBLANK && ((stat >> 3) & 1)) {
            bus.requestInterrupt(0x02); // LCD STAT interrupt
        } else if (currentMode == PPU_MODE::VBLANK && ((stat >> 4) & 1)) {
            bus.requestInterrupt(0x02); // LCD STAT interrupt
        } else if (currentMode == PPU_MODE::OAM_SCAN && ((stat >> 5) & 1)) {
            bus.requestInterrupt(0x02); // LCD STAT interrupt
        }
    }

    // LYC == LY interrupt
    if (bus.read(LY_ADDRESS) == bus.read(LYC_ADDRESS)) {
        uint8_t stat = bus.read(STAT_ADDRESS);
        stat |= 0x04; // Set coincidence flag
        bus.write(STAT_ADDRESS, stat);
        if (((stat >> 6) & 1)) {
            bus.requestInterrupt(0x02); // LCD STAT interrupt
        }
    } else {
        uint8_t stat = bus.read(STAT_ADDRESS);
        stat &= ~0x04; // Clear coincidence flag
        bus.write(STAT_ADDRESS, stat);
    }
}

uint16_t PPU::getTileAddress(uint8_t tileNumber) {
    uint8_t lcdControlValue = bus.read(LCDC_ADDRESS);
    bool tileAddressingMode = static_cast<bool>((lcdControlValue >> 4) & 1);

    uint16_t tileAddress = tileAddressingMode ?
        TILE_BLOCK_0_START + (TILE_BYTE_SIZE * tileNumber) :
        TILE_BLOCK_2_START + (TILE_BYTE_SIZE * static_cast<int8_t>(tileNumber));

    return tileAddress;
}

void PPU::setPixel(int x, int y, uint8_t value) {
    assert(value < 4 && "Color value should not exceed 2 bits");
    assert(x < SCREEN_WIDTH && x >= 0 && y < SCREEN_HEIGHT && y >= 0);

    uint8_t palette = bus.read(BGP_ADDRESS);
    uint8_t color = (palette >> (value * 2)) & 0x3;

    uint8_t rgbValue;
    switch (color) {
        case 0x00: rgbValue = 0; break; // black
        case 0x01: rgbValue = 96; break;
        case 0x02: rgbValue = 192; break;
        case 0x03: rgbValue = 255; break; //
        default: throw std::out_of_range("Color value exceeds 2 bits - should not even be reachable");
    }

    // Set RGBA values
    int screenOffset = (y * SCREEN_WIDTH + x) * 4;
    frameBuffer[screenOffset] = rgbValue;
    frameBuffer[screenOffset + 1] = rgbValue;
    frameBuffer[screenOffset + 2] = rgbValue;
    frameBuffer[screenOffset + 3] = 255;
}

void PPU::drawScanline() {
    uint8_t lcdControlValue = bus.read(LCDC_ADDRESS);
    bool tileMapMode = static_cast<bool>((lcdControlValue >> 3) & 1);

    // Entire tile map is 256 * 256 which is way larger than the gameboy screen
    // meaning only part of the tile map is displayed
    // scX and scY define the starting offset which wraps around if too large
    uint8_t scX = bus.read(SCX_ADDRESS);
    uint8_t scY = bus.read(SCY_ADDRESS);

    // 1 = 9C00–9FFF; 0 = 9800–9BFF
    uint16_t tileMapStart = tileMapMode ? TILE_MAP_1_START : TILE_MAP_0_START;
    uint8_t y = bus.read(LY_ADDRESS);

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
    drawWindow();
    drawSprites();
}

void PPU::drawWindow() {
    uint8_t lcdControlValue = bus.read(LCDC_ADDRESS);
    bool windowDisplayEnable = static_cast<bool>((lcdControlValue >> 5) & 1);
    if (!windowDisplayEnable) {
        return;
    }

    uint8_t windowTileMapDisplaySelect = static_cast<bool>((lcdControlValue >> 6) & 1);
    uint16_t windowTileMapStart = windowTileMapDisplaySelect ? TILE_MAP_1_START : TILE_MAP_0_START;

    uint8_t wx = bus.read(WX_ADDRESS) - 7;
    uint8_t wy = bus.read(WY_ADDRESS);

    uint8_t currentScanline = bus.read(LY_ADDRESS);

    if (currentScanline >= wy && currentScanline < (wy + SCREEN_HEIGHT)) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            if (x >= wx && x < (wx + SCREEN_WIDTH)) {
                uint8_t windowX = x - wx;
                uint8_t windowY = currentScanline - wy;

                uint8_t windowXTile = windowX / TILE_PIXEL_SIZE;
                uint8_t windowYTile = windowY / TILE_PIXEL_SIZE;

                uint16_t tileMapOffset = (NUM_TILES_PER_COLUMN * windowYTile) + windowXTile;
                uint8_t tileNumber = bus.read(windowTileMapStart + tileMapOffset);

                uint16_t tileAddress = getTileAddress(tileNumber);
                uint8_t tileX = windowX % TILE_PIXEL_SIZE;
                uint8_t tileY = windowY % TILE_PIXEL_SIZE;

                uint8_t tileByte1 = bus.read(tileAddress + (2 * tileY));
                uint8_t tileByte2 = bus.read(tileAddress + (2 * tileY + 1));

                uint8_t upperColorBit = (tileByte1 >> (7 - tileX)) & 1;
                uint8_t lowerColorBit = (tileByte2 >> (7 - tileX)) & 1;
                uint8_t colorValue = (upperColorBit << 1) | lowerColorBit;

                setPixel(x, currentScanline, colorValue);
            }
        }
    }
}

void PPU::drawSprites() {
    uint8_t lcdControlValue = bus.read(LCDC_ADDRESS);
    bool spriteDisplayEnable = static_cast<bool>((lcdControlValue >> 1) & 1);
    if (!spriteDisplayEnable) {
        return;
    }

    bool spriteSize = static_cast<bool>((lcdControlValue >> 2) & 1); // 0: 8x8, 1: 8x16
    uint8_t spriteHeight = spriteSize ? 16 : 8;

    uint8_t currentScanline = bus.read(LY_ADDRESS);

    // OAM starts at 0xFE00 and contains 40 sprites, each 4 bytes long
    for (uint16_t i = 0; i < 40; i++) {
        uint16_t spriteAddress = 0xFE00 + (i * 4);
        uint8_t yPos = bus.read(spriteAddress) - 16;
        uint8_t xPos = bus.read(spriteAddress + 1) - 8;
        uint8_t tileNumber = bus.read(spriteAddress + 2);
        uint8_t attributes = bus.read(spriteAddress + 3);

        bool bgAndWindowOverSprite = static_cast<bool>((attributes >> 7) & 1);
        bool yFlip = static_cast<bool>((attributes >> 6) & 1);
        bool xFlip = static_cast<bool>((attributes >> 5) & 1);
        uint8_t paletteNumber = static_cast<uint8_t>((attributes >> 4) & 1);

        if (currentScanline >= yPos && currentScanline < (yPos + spriteHeight)) {
            uint8_t tileY = currentScanline - yPos;
            if (yFlip) {
                tileY = spriteHeight - 1 - tileY;
            }

            uint16_t tileAddress = TILE_BLOCK_0_START + (TILE_BYTE_SIZE * tileNumber);
            uint8_t tileByte1 = bus.read(tileAddress + (2 * tileY));
            uint8_t tileByte2 = bus.read(tileAddress + (2 * tileY + 1));

            for (int x = 0; x < TILE_PIXEL_SIZE; x++) {
                uint8_t tileX = x;
                if (xFlip) {
                    tileX = TILE_PIXEL_SIZE - 1 - tileX;
                }

                uint8_t upperColorBit = (tileByte1 >> (7 - tileX)) & 1;
                uint8_t lowerColorBit = (tileByte2 >> (7 - tileX)) & 1;
                uint8_t colorValue = (upperColorBit << 1) | lowerColorBit;

                if (colorValue == 0) { // Color 0 is transparent
                    continue;
                }

                uint8_t palette = (paletteNumber == 0) ? bus.read(OBP0_ADDRESS) : bus.read(OBP1_ADDRESS);
                uint8_t color = (palette >> (colorValue * 2)) & 0x3;

                uint8_t rgbValue;
                switch (color) {
                    case 0x00: rgbValue = 0; break; // black
                    case 0x01: rgbValue = 96; break;
                    case 0x02: rgbValue = 192; break;
                    case 0x03: rgbValue = 255; break; //
                    default: throw std::out_of_range("Color value exceeds 2 bits - should not even be reachable");
                }

                int screenX = xPos + x;
                if (screenX >= 0 && screenX < SCREEN_WIDTH) {
                    int screenOffset = (currentScanline * SCREEN_WIDTH + screenX) * 4;
                    frameBuffer[screenOffset] = rgbValue;
                    frameBuffer[screenOffset + 1] = rgbValue;
                    frameBuffer[screenOffset + 2] = rgbValue;
                    frameBuffer[screenOffset + 3] = 255;
                }
            }
        }
    }
}