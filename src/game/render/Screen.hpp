#pragma once
#include <stdint.h>
#include <string.h>
#include <stddef.h>

namespace Screen {
    constexpr int SCREEN_WIDTH  = 200;
    constexpr int SCREEN_HEIGHT = 120;

    // 24 KB grayscale framebuffer — 0=black, 255=white
    uint8_t buffer[SCREEN_WIDTH * SCREEN_HEIGHT];

    inline void clear(uint8_t color = 240) {
        memset(buffer, color, sizeof(buffer));
    }

    inline void drawPixel(int x, int y, uint8_t color) {
        if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT)
            buffer[y * SCREEN_WIDTH + x] = color;
    }

    // 3x5 font — bit2=left, bit1=mid, bit0=right per row
    static constexpr uint8_t FONT[27][5] = {
        {2,5,7,5,5},{6,5,6,5,6},{3,4,4,4,3},{6,5,5,5,6},{7,4,6,4,7}, // A B C D E
        {7,4,6,4,4},{3,4,5,5,3},{5,5,7,5,5},{7,2,2,2,7},{3,1,1,5,2}, // F G H I J
        {5,6,4,6,5},{4,4,4,4,7},{5,7,5,5,5},{7,5,5,5,5},{2,5,5,5,2}, // K L M N O
        {6,5,6,4,4},{2,5,5,7,1},{6,5,6,5,5},{3,4,2,1,6},{7,2,2,2,2}, // P Q R S T
        {5,5,5,5,3},{5,5,5,2,2},{5,5,5,7,5},{5,5,2,5,5},{5,5,2,2,2}, // U V W X Y
        {7,1,2,4,7},{0,0,0,0,0},                                       // Z SPACE
    };

    inline void drawChar(int x, int y, char c, uint8_t color = 10) {
        int idx = (c >= 'A' && c <= 'Z') ? (c - 'A') : 26; // default space
        for (int row = 0; row < 5; row++) {
            uint8_t bits = FONT[idx][row];
            if (bits & 4) drawPixel(x,   y+row, color);
            if (bits & 2) drawPixel(x+1, y+row, color);
            if (bits & 1) drawPixel(x+2, y+row, color);
        }
    }

    // Draws uppercase string, 4px per char (3 wide + 1 gap)
    inline void drawText(int x, int y, const char* str, uint8_t color = 10) {
        for (int i = 0; str[i]; i++)
            drawChar(x + i * 4, y, str[i], color);
    }

    // Returns pixel width of string
    inline int textWidth(const char* str) {
        int len = 0;
        while (str[len]) len++;
        return len * 4 - 1;
    }
}
