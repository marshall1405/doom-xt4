#pragma once
#include "Player.hpp"
#include "Entity.hpp"
#include "render/Camera.hpp"
#include "render/screens.h"

enum class GameState { TITLE, PLAYING, DEAD, WIN };
static GameState gameState        = GameState::PLAYING;
static uint32_t  deadTimer        = 0;
static uint32_t  winTimer         = 0;
static bool      _wantsFullRefresh = true; // full refresh on first frame

static void resetGame() {
    player = Player(8.0f, 8.0f, 0.25f);
    player.health = 100;
    player.ammo   = 50;
    enemies[0] = Enemy(3.0f,  3.0f);
    enemies[1] = Enemy(12.0f, 3.0f);
    enemies[2] = Enemy(3.0f,  12.0f);
    enemies[3] = Enemy(12.0f, 12.0f);
    camera = Camera();
}

static void drawTitleScreen() {
    Screen::clear(240); // white bg

    // DOOM logo centred horizontally, near top
    int lx = (Screen::SCREEN_WIDTH  - 160) / 2;
    int ly = 10;
    drawBitmap1bit(scr_title_logo, 160, 40, lx, ly);

    // Divider line
    for (int x = 10; x < Screen::SCREEN_WIDTH - 10; x++)
        Screen::drawPixel(x, 57, 10);

    // Subtitle & prompt
    const char* sub  = "DOOM FOR XTEINK";
    const char* hint = "PRESS ANY BUTTON";
    int sx = (Screen::SCREEN_WIDTH - Screen::textWidth(sub))  / 2;
    int hx = (Screen::SCREEN_WIDTH - Screen::textWidth(hint)) / 2;
    Screen::drawText(sx, 63, sub,  10);
    Screen::drawText(hx, 80, hint, 10);

    // Flashing cursor dots
    static uint8_t blink = 0;
    if ((++blink / 4) % 2 == 0) {
        for (int i = 0; i < 3; i++)
            Screen::drawPixel(hx - 6 + i * 3, 83, 10);
    }
}

static void drawWinScreen() {
    Screen::clear(10);

    const char* msg  = "YOU WIN!";
    const char* hint = "PRESS ANY BUTTON";
    int mx = (Screen::SCREEN_WIDTH - Screen::textWidth(msg))  / 2;
    int hx = (Screen::SCREEN_WIDTH - Screen::textWidth(hint)) / 2;
    int cy = Screen::SCREEN_HEIGHT / 2;
    Screen::drawText(mx, cy - 10, msg,  240);
    Screen::drawText(hx, cy + 5,  hint, 240);
}

static void drawDeadScreen() {
    Screen::clear(10); // black bg

    // Dead face centred
    int fx = (Screen::SCREEN_WIDTH  - 48) / 2;
    int fy = (Screen::SCREEN_HEIGHT - 48) / 2 - 8;
    drawBitmap1bit(scr_dead_face, 48, 48, fx, fy);

    const char* msg  = "YOU DIED";
    const char* hint = "PRESS ANY BUTTON";
    int mx = (Screen::SCREEN_WIDTH - Screen::textWidth(msg))  / 2;
    int px = (Screen::SCREEN_WIDTH - Screen::textWidth(hint)) / 2;
    Screen::drawText(mx, fy - 10, msg,  240);
    Screen::drawText(px, fy + 53, hint, 240);
}

namespace Game {
    void setup() { resetGame(); }

    // Returns true (once) when a state transition needs a full e-ink refresh
    bool consumeFullRefresh() { bool v = _wantsFullRefresh; _wantsFullRefresh = false; return v; }
    bool isPlaying()          { return gameState == GameState::PLAYING; }

    // Draws the title/logo into Screen::buffer to be shown on power-off
    void prepareOffScreen() {
        Screen::clear(240);
        int lx = (Screen::SCREEN_WIDTH  - 160) / 2;
        drawBitmap1bit(scr_title_logo, 160, 40, lx, 10);
        for (int x = 10; x < Screen::SCREEN_WIDTH - 10; x++)
            Screen::drawPixel(x, 57, 10);
        const char* sub = "DOOM FOR XTEINK";
        Screen::drawText((Screen::SCREEN_WIDTH - Screen::textWidth(sub)) / 2, 63, sub, 10);
    }

    void loop(float delta, InputData input) {

        if (gameState == GameState::DEAD) {
            drawDeadScreen();
            uint32_t now = millis();
            if (input.anyPress && now - deadTimer > 1500) {
                resetGame();
                gameState = GameState::PLAYING;
                _wantsFullRefresh = true;
            }
            return;
        }

        if (gameState == GameState::WIN) {
            drawWinScreen();
            uint32_t now = millis();
            if (input.anyPress && now - winTimer > 1500) {
                resetGame();
                gameState = GameState::PLAYING;
                _wantsFullRefresh = true;
            }
            return;
        }

        // --- PLAYING ---
        Screen::clear(240);
        player.update(input, delta);

        for (int i = 0; i < NUM_ENEMIES; i++) {
            enemies[i].update(player.pos, delta);
            if (enemies[i].alive) {
                float dx = enemies[i].pos.x - player.pos.x;
                float dy = enemies[i].pos.y - player.pos.y;
                if (dx*dx + dy*dy < 0.4f) {
                    player.health -= (int)(delta * 20);
                    if (player.health < 0) player.health = 0;
                }
            }
        }

        if (input.shoot && player.ammo > 0) {
            player.ammo--;
            camera.triggerGunFlash();
            for (int i = 0; i < NUM_ENEMIES; i++) {
                Enemy& e = enemies[i];
                if (!e.alive) continue;
                float dx = e.pos.x - player.pos.x;
                float dy = e.pos.y - player.pos.y;
                float t  = dx * player.dir.x + dy * player.dir.y;
                if (t <= 0) continue;
                float px   = dx - t * player.dir.x;
                float py   = dy - t * player.dir.y;
                float perp = sqrtf(px*px + py*py);
                if (perp < 0.45f && t < camera.Zbuffer[Screen::SCREEN_WIDTH / 2]) {
                    e.alive = false;
                    bool allDead = true;
                    for (int j = 0; j < NUM_ENEMIES; j++) if (enemies[j].alive) { allDead = false; break; }
                    if (allDead) { gameState = GameState::WIN; winTimer = millis(); _wantsFullRefresh = true; }
                }
            }
        }

        if (player.health <= 0) {
            gameState = GameState::DEAD;
            deadTimer = millis();
            _wantsFullRefresh = true;
            return;
        }

        camera.update(player);
        camera.draw(player.health, player.ammo);
    }
}
