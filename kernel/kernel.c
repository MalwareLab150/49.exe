#include "include/FOTO/image_data.h"
#include "include/FOTO/SFONDO.h"
#include "include/FIRST/LOAD.h"
#include "include/nFire.h"
#include "include/BEGIN/Salvia_divinorum_starter.h"
#include "include/MUSIC/music.h"
#define VIDEO_MEM ((volatile uint16_t*)0xB8000)
#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25
#define MAP_WIDTH 16
#define MAP_HEIGHT 16
#define COLOR_BLACK 0x0
#define COLOR_DARK_GREY 0x8
#define COLOR_LIGHT_GREY 0x7
#define PORT_KEYBOARD_STATUS 0x64
#define PORT_KEYBOARD_DATA 0x60
#define SCANCODE_W 0x11
#define SCANCODE_S 0x1F
#define SCANCODE_A 0x1E
#define SCANCODE_D 0x20
//tempo
int start_minute = -1;
int start_second = -1;

static inline uint8_t cmos_read(uint8_t reg) {
    __asm__ volatile ("outb %0, $0x70" :: "a"(reg));
    uint8_t val;
    __asm__ volatile ("inb $0x71, %0" : "=a"(val));
    return val;
}

static inline float fabsf(float x) {
    return (x < 0) ? -x : x;
}

float sinf(float x) {
    const float PI = 3.14159265f;
    while (x > PI) x -= 2 * PI;
    while (x < -PI) x += 2 * PI;
    float x2 = x * x;
    float x3 = x2 * x;
    float x5 = x3 * x2;
    return x - x3 / 6.0f + x5 / 120.0f;
}

float cosf(float x) {
    const float PI_2 = 1.57079632f;
    return sinf(x + PI_2);
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void put_char(int x, int y, char c, uint8_t color) {
    if (x < 0 || x >= SCREEN_WIDTH || y < 0 || y >= SCREEN_HEIGHT) return;
    VIDEO_MEM[y * SCREEN_WIDTH + x] = ((uint16_t)color << 8) | (uint8_t)c;
}

const int map[MAP_WIDTH * MAP_HEIGHT] = {
    [0 ... (MAP_WIDTH * MAP_HEIGHT - 1)] = 0,
    [7 * MAP_WIDTH + 7] = 1
};

typedef struct {
    float x, y;
    float dirX, dirY;
    float planeX, planeY;
} Player;

Player player = {
    8.0f, 6.0f,
    0.0f, 1.0f,
    0.66f, 0.0f
};

void draw_progress_bar() {
    int h, m, s;
    get_time(&h, &m, &s);

    if (start_minute == -1) {
        start_minute = m;
        start_second = s;
    }


    int elapsed = (m * 60 + s) - (start_minute * 60 + start_second);
    if (elapsed < 0) elapsed += 3600; 
    if (elapsed > 60) elapsed = 60;

    int remaining = 60 - elapsed;

    int barLength = (SCREEN_WIDTH * elapsed) / 60;

    uint8_t color = 0x0A; // verde

    static int frame = 0;
    frame++;

    if (remaining <= 5) {
        if ((frame / 10) % 2 == 0) color = 0x0C; // rosso lampeggiante
        else color = 0x00;
    } else if (remaining <= 20) {
        if ((frame / 20) % 2 == 0) color = 0x0E; // giallo lampeggiante
        else color = 0x00;
    }

  
    for (int i = 0; i < SCREEN_WIDTH; i++) {
        if (i < barLength)
            put_char(i, 0, 0xDB, color);
        else
            put_char(i, 0, ' ', 0x00);
    }


    char buf[16];
    buf[0] = 'T'; buf[1] = 'i'; buf[2] = 'm'; buf[3] = 'e'; buf[4] = ':';
    buf[5] = ' ';
    buf[6] = '0' + (remaining / 10);
    buf[7] = '0' + (remaining % 10);
    buf[8] = 's';
    buf[9] = '\0';

    int textStart = (SCREEN_WIDTH - 9) / 2; 
    for (int i = 0; buf[i]; i++) {
        put_char(textStart + i, 0, buf[i], 0x0F);
    }

static int triggered = 0;
if (remaining <= 0 && !triggered) {
    DISEGNA_NCRACK();  
    triggered = 1;

   
    while (1) {
        __asm__ volatile ("hlt"); 
    }
  }
}


void get_time(int *h, int *m, int *s) {
    uint8_t sec = cmos_read(0x00);
    uint8_t min = cmos_read(0x02);
    uint8_t hr  = cmos_read(0x04);

    *s = bcd_to_bin(sec);
    *m = bcd_to_bin(min);
    *h = bcd_to_bin(hr);
}


void render_frame() {
    for (int x = 0; x < SCREEN_WIDTH; x++) {
        float cameraX = 2.0f * x / (float)SCREEN_WIDTH - 1.0f;
        float rayDirX = player.dirX + player.planeX * cameraX;
        float rayDirY = player.dirY + player.planeY * cameraX;

        int mapX = (int)(player.x);
        int mapY = (int)(player.y);

        float sideDistX;
        float sideDistY;

        float deltaDistX = (rayDirX == 0) ? 1e30f : fabsf(1 / rayDirX);
        float deltaDistY = (rayDirY == 0) ? 1e30f : fabsf(1 / rayDirY);
        float perpWallDist;

        int stepX, stepY;
        int hit = 0;
        int side;

        if (rayDirX < 0) {
            stepX = -1;
            sideDistX = (player.x - mapX) * deltaDistX;
        } else {
            stepX = 1;
            sideDistX = (mapX + 1.0f - player.x) * deltaDistX;
        }

        if (rayDirY < 0) {
            stepY = -1;
            sideDistY = (player.y - mapY) * deltaDistY;
        } else {
            stepY = 1;
            sideDistY = (mapY + 1.0f - player.y) * deltaDistY;
        }

        while (!hit) {
            if (sideDistX < sideDistY) {
                sideDistX += deltaDistX;
                mapX += stepX;
                side = 0;
            } else {
                sideDistY += deltaDistY;
                mapY += stepY;
                side = 1;
            }

            if (mapX >= 0 && mapX < MAP_WIDTH && mapY >= 0 && mapY < MAP_HEIGHT) {
                if (map[mapY * MAP_WIDTH + mapX] > 0) hit = 1;
            } else {
                hit = 1;
            }
        }

        if (side == 0)
            perpWallDist = (mapX - player.x + (1 - stepX) / 2) / rayDirX;
        else
            perpWallDist = (mapY - player.y + (1 - stepY) / 2) / rayDirY;

        int lineHeight = (int)(SCREEN_HEIGHT / perpWallDist);
        int drawStart = -lineHeight / 2 + SCREEN_HEIGHT / 2;
        if (drawStart < 1) drawStart = 1;   
        int drawEnd = lineHeight / 2 + SCREEN_HEIGHT / 2;
        if (drawEnd >= SCREEN_HEIGHT) drawEnd = SCREEN_HEIGHT - 1;

        for (int y = 1; y < SCREEN_HEIGHT; y++) {  
            if (y >= drawStart && y <= drawEnd) {
                
                if (mapX == 7 && mapY == 7) {
                   
                    int wallPixel = y - drawStart;
                    int wallHeight = drawEnd - drawStart + 1;
                    int imgY = (wallPixel * SCREEN_HEIGHT) / wallHeight;
                    int imgX = x % SCREEN_WIDTH;
                    int imgIndex = imgY * SCREEN_WIDTH + imgX;

                    char c = image_chars[imgIndex];
                    uint8_t color = image_colors[imgIndex];
                    put_char(x, y, c, color);
                } else {
                    
                    char wallChar = 0xDB;
                    uint8_t color = (side == 1) ? COLOR_DARK_GREY : COLOR_LIGHT_GREY;
                    put_char(x, y, wallChar, color);
                }
            } else {
               
                int bgIndex = y * SCREEN_WIDTH + x;
                char c = FOTO_CHARS[bgIndex];
                uint8_t color = FOTO_COLORES[bgIndex];
                put_char(x, y, c, color);
            }
        }
    }
}


void delay(int count) {
    for (volatile int i = 0; i < count * 10000; i++)
        __asm__ volatile ("nop");
}


void kmain() {
    k_clear_screen(0x00);
    START();
    k_clear_screen(0x00);
    const char* lines[] = {
"=== Welcome! ===",
"Use W, A, S, D to move",
"You have 60 seconds to explore",
"A special wall awaits you...",
"",
"Press F to start"
    };

    int num_lines = sizeof(lines) / sizeof(lines[0]);
    for (int i = 0; i < num_lines; i++) {
        const char* msg = lines[i];
        int len = 0;
        while (msg[len]) len++;
        int start_x = (SCREEN_WIDTH - len) / 2;
        for (int j = 0; j < len; j++) {
            put_char(start_x + j, 10 + i, msg[j], 0x0F);
        }
    }

    
    while (1) {
        if (inb(PORT_KEYBOARD_STATUS) & 1) {
            uint8_t sc = inb(PORT_KEYBOARD_DATA);
            if (sc == 0x21) break; 
        }
    }

    k_clear_screen(0x00); //clear screen

    while (1) {
        draw_progress_bar();
        render_frame();
        music_tick();  

        if (inb(PORT_KEYBOARD_STATUS) & 1) {
            uint8_t sc = inb(PORT_KEYBOARD_DATA);

            if (sc == SCANCODE_W) {
                float newX = player.x + player.dirX * 0.1f;
                float newY = player.y + player.dirY * 0.1f;
                if (map[(int)newY * MAP_WIDTH + (int)newX] == 0) {
                    player.x = newX;
                    player.y = newY;
                }
            } else if (sc == SCANCODE_S) {
                float newX = player.x - player.dirX * 0.1f;
                float newY = player.y - player.dirY * 0.1f;
                if (map[(int)newY * MAP_WIDTH + (int)newX] == 0) {
                    player.x = newX;
                    player.y = newY;
                }
            } else if (sc == SCANCODE_A) {
                float oldDirX = player.dirX;
                player.dirX = player.dirX * cosf(0.1f) - player.dirY * sinf(0.1f);
                player.dirY = oldDirX * sinf(0.1f) + player.dirY * cosf(0.1f);
                float oldPlaneX = player.planeX;
                player.planeX = player.planeX * cosf(0.1f) - player.planeY * sinf(0.1f);
                player.planeY = oldPlaneX * sinf(0.1f) + player.planeY * cosf(0.1f);
            } else if (sc == SCANCODE_D) {
                float oldDirX = player.dirX;
                player.dirX = player.dirX * cosf(-0.1f) - player.dirY * sinf(-0.1f);
                player.dirY = oldDirX * sinf(-0.1f) + player.dirY * cosf(-0.1f);
                float oldPlaneX = player.planeX;
                player.planeX = player.planeX * cosf(-0.1f) - player.planeY * sinf(-0.1f);
                player.planeY = oldPlaneX * sinf(-0.1f) + player.planeY * cosf(-0.1f);
            }
        }
    }
}
