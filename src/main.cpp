#include <Arduino.h>
#include <EInkDisplay.h>

#define EPD_SCLK 8
#define EPD_MOSI 10
#define EPD_CS   21
#define EPD_DC   4
#define EPD_RST  5
#define EPD_BUSY 6

EInkDisplay display(EPD_SCLK, EPD_MOSI, EPD_CS, EPD_DC, EPD_RST, EPD_BUSY);

static void drawRect(uint8_t* fb, int x, int y, int w, int h) {
    for (int row = y; row < y + h; row++) {
        for (int col = x; col < x + w; col++) {
            fb[row * EInkDisplay::DISPLAY_WIDTH_BYTES + col / 8] &= ~(0x80 >> (col % 8));
        }
    }
}

void setup() {
    display.begin();
    display.clearScreen(0xFF);  // white background

    uint8_t* fb = display.getFrameBuffer();

    // vertical bar of the "1", centered on 800x480
    drawRect(fb, 370, 100, 60, 280);

    // top-left stroke
    drawRect(fb, 300, 100, 70, 50);

    display.displayBuffer(EInkDisplay::FULL_REFRESH);
}

void loop() {}
