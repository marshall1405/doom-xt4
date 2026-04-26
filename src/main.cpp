#include <Arduino.h>
#include <EInkDisplay.h>
#include <InputManager.h>
#include <esp_sleep.h>
#include "game/game.hpp"

#define EPD_SCLK 8
#define EPD_MOSI 10
#define EPD_CS   21
#define EPD_DC   4
#define EPD_RST  5
#define EPD_BUSY 6

constexpr float TURN_STEP = 30.0f * 3.14159f / 180.0f;

EInkDisplay display(EPD_SCLK, EPD_MOSI, EPD_CS, EPD_DC, EPD_RST, EPD_BUSY);
InputManager inputMgr;

// Input accumulators — written by background task, consumed by loop()
static volatile int  pendingTurns = 0;
static volatile bool pendingShoot = false;
static volatile bool pendingPower = false;
static volatile bool pendingAny   = false;

static void pollInput() {
    inputMgr.update();
    if (inputMgr.wasPressed(InputManager::BTN_UP))      { pendingTurns--; pendingAny = true; }
    if (inputMgr.wasPressed(InputManager::BTN_DOWN))    { pendingTurns++; pendingAny = true; }
    if (inputMgr.wasPressed(InputManager::BTN_LEFT))    { pendingTurns++; pendingAny = true; }
    if (inputMgr.wasPressed(InputManager::BTN_RIGHT) ||
        inputMgr.wasPressed(InputManager::BTN_CONFIRM)) { pendingShoot = true; pendingAny = true; }
    if (inputMgr.wasPressed(InputManager::BTN_POWER))   pendingPower = true;
}

// Runs at higher priority than loop() — gets CPU during the display waveform's delay(1) yields
static void inputTaskFn(void*) {
    for (;;) {
        pollInput();
        vTaskDelay(pdMS_TO_TICKS(8)); // poll every 8ms, catches taps as short as 8ms
    }
}

static void renderToEInk();
static void powerOff();

void setup() {
    Serial.begin(115200);
    display.begin();
    display.clearScreen(0xFF);
    inputMgr.begin();
    Game::setup();

    // Start input polling task at priority 2 (loop() runs at 1)
    xTaskCreate(inputTaskFn, "input", 2048, nullptr, 2, nullptr);
}

void loop() {
    if (pendingPower) powerOff();

    static uint64_t lastTime = micros();
    uint64_t now = micros();
    float dt = (float)(now - lastTime) / 1000000.0f;
    if (dt > 1.0f) dt = 1.0f;
    lastTime = now;

    InputData input;
    input.fwd       = true;
    input.turnAngle = (int)pendingTurns * TURN_STEP;
    input.shoot     = pendingShoot;
    input.anyPress  = pendingAny;
    pendingTurns = 0;
    pendingShoot = false;
    pendingAny   = false;

    Game::loop(dt, input);
    renderToEInk(); // blocks ~200ms — input task runs during this
}

static void renderToEInk() {
    static int frameCount = 0;

    uint8_t* fb = display.getFrameBuffer();
    memset(fb, 0xFF, EInkDisplay::MAX_BUFFER_SIZE);

    for (int sy = 0; sy < Screen::SCREEN_HEIGHT; sy++) {
        for (int sx = 0; sx < Screen::SCREEN_WIDTH; sx += 2) {
            // Process 2 screen pixels → 1 e-ink byte (4 bits each, never straddles bytes)
            uint8_t hi = (Screen::buffer[sy * Screen::SCREEN_WIDTH + sx]     < 128) ? 0x00 : 0xF0;
            uint8_t lo = (Screen::buffer[sy * Screen::SCREEN_WIDTH + sx + 1] < 128) ? 0x00 : 0x0F;
            uint8_t einkByte = hi | lo;
            if (einkByte != 0xFF) {
                int px = sx * 4;
                for (int dy = 0; dy < 4; dy++) {
                    fb[(sy * 4 + dy) * EInkDisplay::DISPLAY_WIDTH_BYTES + px / 8] = einkByte;
                }
            }
        }
    }

    EInkDisplay::RefreshMode mode = (++frameCount % 40 == 0)
        ? EInkDisplay::FULL_REFRESH
        : EInkDisplay::FAST_REFRESH;

    display.displayBuffer(mode);
}

static void powerOff() {
    display.clearScreen(0xFF);
    display.displayBuffer(EInkDisplay::FULL_REFRESH);
    display.deepSleep();
    esp_deep_sleep_enable_gpio_wakeup(1ULL << InputManager::POWER_BUTTON_PIN,
                                      ESP_GPIO_WAKEUP_GPIO_LOW);
    esp_deep_sleep_start();
}
