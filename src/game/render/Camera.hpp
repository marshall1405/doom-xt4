#pragma once
#include "../Player.hpp"
#include "../Entity.hpp"
#include "Screen.hpp"
#include "sprites.h"
#include "../map.h"
#include <math.h>

class Camera {
    static constexpr int H   = Screen::SCREEN_HEIGHT;
    static constexpr int W   = Screen::SCREEN_WIDTH;
    static constexpr int H_2 = H / 2;
    static constexpr int W_2 = W / 2;

    Vector2 pos, dir, plane;
    uint32_t _gunFlashMs = 0;
    static constexpr uint32_t GUN_FLASH_DURATION = 300;
public:
    float Zbuffer[W];

    void update(const Player& p) {
        pos   = p.pos;
        dir   = p.dir;
        plane = Vector2(dir.y * 0.66f, -dir.x * 0.66f);
    }

    void triggerGunFlash() { _gunFlashMs = millis(); }

    void draw(int health, int ammo) {
        drawWalls();
        drawSprites();
        drawGun();
        drawHUD(health, ammo);
    }

private:
    void drawWalls();
    void drawSprites();
    void drawGun();
    void drawHUD(int health, int ammo);
};

Camera camera;

void Camera::drawWalls() {
    for (int x = 0; x < W; x++) {
        float camX    = 2.0f * x / float(W) - 1.0f;
        float rayDirX = dir.x + plane.x * camX;
        float rayDirY = dir.y + plane.y * camX;

        int mapX = (int)pos.x;
        int mapY = (int)pos.y;

        float deltaDistX = fabsf(rayDirX) < 1e-6f ? 1e30f : fabsf(1.0f / rayDirX);
        float deltaDistY = fabsf(rayDirY) < 1e-6f ? 1e30f : fabsf(1.0f / rayDirY);

        float sideDistX, sideDistY;
        int stepX, stepY;

        if (rayDirX < 0) { stepX = -1; sideDistX = (pos.x - mapX) * deltaDistX; }
        else              { stepX =  1; sideDistX = (mapX + 1.0f - pos.x) * deltaDistX; }
        if (rayDirY < 0) { stepY = -1; sideDistY = (pos.y - mapY) * deltaDistY; }
        else              { stepY =  1; sideDistY = (mapY + 1.0f - pos.y) * deltaDistY; }

        int side = 0;
        for (int steps = 0; steps < MAP_W + MAP_H; steps++) {
            if (sideDistX < sideDistY) { sideDistX += deltaDistX; mapX += stepX; side = 0; }
            else                        { sideDistY += deltaDistY; mapY += stepY; side = 1; }
            if (mapX < 0 || mapX >= MAP_W || mapY < 0 || mapY >= MAP_H) break;
            if (gameMap[mapY][mapX] > 0) break;
        }

        float perpDist = (side == 0) ? (sideDistX - deltaDistX) : (sideDistY - deltaDistY);
        if (perpDist < 0.05f) perpDist = 0.05f;
        Zbuffer[x] = perpDist;

        int lineH     = (int)(H / perpDist);
        int drawStart = H_2 - lineH / 2; if (drawStart < 0)  drawStart = 0;
        int drawEnd   = H_2 + lineH / 2; if (drawEnd >= H)   drawEnd   = H - 1;

        uint8_t wallColor = (side == 0) ? 20 : 60;

        for (int y = 0;           y < drawStart; y++) Screen::drawPixel(x, y, 240);
        for (int y = drawStart;   y <= drawEnd;  y++) Screen::drawPixel(x, y, wallColor);
        for (int y = drawEnd + 1; y < H;        y++) Screen::drawPixel(x, y, 150);
    }
}

void Camera::drawSprites() {
    static uint32_t lastMs = 0;
    static int animFrame = 0;
    uint32_t now = millis();
    if (now - lastMs > 200) { animFrame = (animFrame + 1) % 4; lastMs = now; }

    struct SD { float distSq; int idx; };
    SD sorted[NUM_ENEMIES];
    for (int i = 0; i < NUM_ENEMIES; i++) {
        float dx = enemies[i].pos.x - pos.x;
        float dy = enemies[i].pos.y - pos.y;
        sorted[i] = {dx*dx + dy*dy, i};
    }
    for (int i = 0; i < NUM_ENEMIES - 1; i++)
        for (int j = 0; j < NUM_ENEMIES - 1 - i; j++)
            if (sorted[j].distSq < sorted[j+1].distSq) {
                SD tmp = sorted[j]; sorted[j] = sorted[j+1]; sorted[j+1] = tmp;
            }

    float invDet = 1.0f / (plane.x * dir.y - dir.x * plane.y);

    for (int i = 0; i < NUM_ENEMIES; i++) {
        Enemy& e = enemies[sorted[i].idx];
        if (!e.alive) continue;

        float sx = e.pos.x - pos.x;
        float sy = e.pos.y - pos.y;
        float transformX = invDet * (dir.y * sx - dir.x * sy);
        float transformY = invDet * (-plane.y * sx + plane.x * sy);
        if (transformY <= 0.1f) continue;

        int spriteH = abs((int)(H / transformY));
        int screenX = (int)(W_2 * (1.0f + transformX / transformY));
        int startY  = H_2 - spriteH / 2; if (startY < 0)  startY = 0;
        int endY    = H_2 + spriteH / 2; if (endY >= H)   endY   = H - 1;
        int startX  = screenX - spriteH / 2;
        int endX    = screenX + spriteH / 2;
        int spanW   = endX - startX + 1;

        for (int x = startX; x <= endX; x++) {
            if (x < 0 || x >= W || transformY >= Zbuffer[x]) continue;
            int tx = (x - startX) * 32 / spanW;
            for (int y = startY; y <= endY; y++) {
                int ty = (y - startY) * 32 / spriteH;
                if (sprBit(bmp_poss_mask, animFrame, tx, ty))
                    Screen::drawPixel(x, y, sprBit(bmp_poss_bits, animFrame, tx, ty) ? 10 : 240);
            }
        }
    }
}

void Camera::drawGun() {
    bool flashing = (_gunFlashMs != 0 && millis() - _gunFlashMs < GUN_FLASH_DURATION);

    constexpr int GW = 48;
    constexpr int GH = 48;
    int originX = W / 2 - GW / 2 + 8;
    // Recoil: gun drops 3px on fire
    int originY = H - GH + 10 + (flashing ? 3 : 0);

    for (int dy = 0; dy < GH; dy++) {
        int sy = dy * 32 / GH;
        for (int dx = 0; dx < GW; dx++) {
            int sx = dx * 32 / GW;
            if (!sprBit(bmp_pist_mask, 0, sx, sy)) continue;
            uint8_t color = sprBit(bmp_pist_bits, 0, sx, sy) ? 10 : 240;
            Screen::drawPixel(originX + dx, originY + dy, color);
        }
    }

    if (flashing) {
        // Barrel tip is at sprite row 0, source-x ~14-15, which maps to screen ~+21
        // Draw a small muzzle flash starburst above it
        int fx = originX + 21;
        int fy = originY - 1; // just above the barrel tip
        // Vertical stem
        Screen::drawPixel(fx,   fy,   10);
        Screen::drawPixel(fx,   fy-1, 10);
        Screen::drawPixel(fx,   fy-2, 10);
        Screen::drawPixel(fx,   fy-3, 10);
        Screen::drawPixel(fx,   fy-4, 10);
        // Horizontal spread at midpoint
        Screen::drawPixel(fx-2, fy-2, 10);
        Screen::drawPixel(fx-1, fy-2, 10);
        Screen::drawPixel(fx+1, fy-2, 10);
        Screen::drawPixel(fx+2, fy-2, 10);
        // Diagonal sparks
        Screen::drawPixel(fx-2, fy-4, 10);
        Screen::drawPixel(fx+2, fy-4, 10);
        Screen::drawPixel(fx-1, fy-1, 240);
        Screen::drawPixel(fx+1, fy-1, 240);
    }
}

// 3x5 pixel font for digits 0-9
// Each row: bit2=left, bit1=mid, bit0=right
static const uint8_t DIGITS[10][5] = {
    {7,5,5,5,7}, // 0
    {2,6,2,2,7}, // 1
    {7,1,7,4,7}, // 2
    {7,1,3,1,7}, // 3
    {5,5,7,1,1}, // 4
    {7,4,7,1,7}, // 5
    {7,4,7,5,7}, // 6
    {7,1,1,2,2}, // 7
    {7,5,7,5,7}, // 8
    {7,5,7,1,7}, // 9
};

static void drawDigit(int x, int y, int d, uint8_t color) {
    for (int row = 0; row < 5; row++) {
        uint8_t bits = DIGITS[d][row];
        if (bits & 4) Screen::drawPixel(x,   y+row, color);
        if (bits & 2) Screen::drawPixel(x+1, y+row, color);
        if (bits & 1) Screen::drawPixel(x+2, y+row, color);
    }
}

static void drawNumber(int x, int y, int val, uint8_t color) {
    if (val > 999) val = 999;
    if (val < 0)   val = 0;
    int hundreds = val / 100;
    int tens     = (val % 100) / 10;
    int ones     = val % 10;
    int cx = x;
    if (val >= 100) { drawDigit(cx, y, hundreds, color); cx += 4; }
    if (val >= 10)  { drawDigit(cx, y, tens,     color); cx += 4; }
    drawDigit(cx, y, ones, color);
}

void Camera::drawHUD(int health, int ammo) {
    // Background bar
    for (int y = H-9; y < H; y++)
        for (int x = 0; x < W; x++)
            Screen::drawPixel(x, y, 20);

    // HP: label + number
    // Draw "HP" as two tiny 3x5 letters
    // H
    Screen::drawPixel(5,H-8,240); Screen::drawPixel(7,H-8,240);
    Screen::drawPixel(5,H-7,240); Screen::drawPixel(7,H-7,240);
    Screen::drawPixel(5,H-6,240); Screen::drawPixel(6,H-6,240); Screen::drawPixel(7,H-6,240);
    Screen::drawPixel(5,H-5,240); Screen::drawPixel(7,H-5,240);
    Screen::drawPixel(5,H-4,240); Screen::drawPixel(7,H-4,240);
    // P
    Screen::drawPixel(9,H-8,240); Screen::drawPixel(10,H-8,240);
    Screen::drawPixel(9,H-7,240); Screen::drawPixel(11,H-7,240);
    Screen::drawPixel(9,H-6,240); Screen::drawPixel(10,H-6,240);
    Screen::drawPixel(9,H-5,240);
    Screen::drawPixel(9,H-4,240);

    drawNumber(14, H-8, health, 240);

    // AMMO: label + number on right side
    // A
    int ax = W - 38;
    Screen::drawPixel(ax+1,H-8,240);
    Screen::drawPixel(ax,H-7,240); Screen::drawPixel(ax+2,H-7,240);
    Screen::drawPixel(ax,H-6,240); Screen::drawPixel(ax+1,H-6,240); Screen::drawPixel(ax+2,H-6,240);
    Screen::drawPixel(ax,H-5,240); Screen::drawPixel(ax+2,H-5,240);
    Screen::drawPixel(ax,H-4,240); Screen::drawPixel(ax+2,H-4,240);

    drawNumber(W - 34, H-8, ammo, 240);
}
