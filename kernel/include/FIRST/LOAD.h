#include <stddef.h>
#define VIDEO_MEMORY ((uint16_t*) 0xB8000)
#define VGA_WIDTH 80
#define CHAR_PIXEL ' '  
#define VGA_HEIGHT 25
volatile uint16_t* vga_buffer = (volatile uint16_t*) 0xB8000;
volatile char* video = (volatile char*)0xB8000;


void k_clear_screen(int bg) {
    for (int i = 0; i < 80 * 25 * 2; i += 2) {
        video[i] = ' ';
        video[i + 1] = bg;
    }
}

size_t strlen(const char* str) {
    size_t len = 0;
    while (str[len] != '\0') {
        len++;
    }
    return len;
}

uint8_t read_cmos(uint8_t reg) {
    asm volatile ("outb %0, %1" : : "a"(reg), "Nd"(0x70));
    uint8_t value;
    asm volatile ("inb %1, %0" : "=a"(value) : "Nd"(0x71));
    return value;
}

uint8_t bcd_to_bin(uint8_t bcd) {
    return ((bcd / 16) * 10) + (bcd & 0x0F);
}

uint8_t get_current_second() {
    uint8_t second = read_cmos(0x00);
    return bcd_to_bin(second);
}

void wait_for_next_second(uint8_t last_second) {
    uint8_t current;
    do {
        current = get_current_second();
    } while (current == last_second);
}

void putchar(int x, int y, char c, uint8_t color) {
    VIDEO_MEMORY[y * VGA_WIDTH + x] = ((uint16_t)color << 8) | c;
}

// Font 5x5 per lettere maiuscole e spazio
uint8_t SPACE[5] = {0, 0, 0, 0, 0};
uint8_t font_A[5] = {0b01110, 0b10001, 0b11111, 0b10001, 0b10001};
uint8_t font_B[5] = {0b11110, 0b10001, 0b11110, 0b10001, 0b11110};
uint8_t font_C[5] = {0b01110, 0b10000, 0b10000, 0b10000, 0b01110};
uint8_t font_D[5] = {0b11100, 0b10010, 0b10001, 0b10010, 0b11100};
uint8_t font_E[5] = {0b11111, 0b10000, 0b11110, 0b10000, 0b11111};
uint8_t font_F[5] = {0b11111, 0b10000, 0b11110, 0b10000, 0b10000};
uint8_t font_G[5] = {0b01111, 0b10000, 0b10011, 0b10001, 0b01110};
uint8_t font_H[5] = {0b10001, 0b10001, 0b11111, 0b10001, 0b10001};
uint8_t font_I[5] = {0b01110, 0b00100, 0b00100, 0b00100, 0b01110};
uint8_t font_J[5] = {0b00111, 0b00010, 0b00010, 0b10010, 0b01100};
uint8_t font_L[5] = {0b10000, 0b10000, 0b10000, 0b10000, 0b11111};
uint8_t font_M[5] = {0b10001, 0b11011, 0b10101, 0b10001, 0b10001};
uint8_t font_N[5] = {0b10001, 0b11001, 0b10101, 0b10011, 0b10001};
uint8_t font_O[5] = {0b01110, 0b10001, 0b10001, 0b10001, 0b01110};
uint8_t font_P[5] = {0b11110, 0b10001, 0b11110, 0b10000, 0b10000};
uint8_t font_R[5] = {0b11110, 0b10001, 0b11110, 0b10010, 0b10001};
uint8_t font_S[5] = {0b01111, 0b10000, 0b01110, 0b00001, 0b11110};
uint8_t font_T[5] = {0b11111, 0b00100, 0b00100, 0b00100, 0b00100};
uint8_t font_U[5] = {0b10001, 0b10001, 0b10001, 0b10001, 0b01110};
uint8_t font_V[5] = {0b10001, 0b10001, 0b10001, 0b01010, 0b00100};
uint8_t font_W[5] = {0b10001, 0b10001, 0b10101, 0b11011, 0b10001};
uint8_t font_X[5] = {0b10001, 0b01010, 0b00100, 0b01010, 0b10001};
uint8_t font_Y[5] = {0b10001, 0b01010, 0b00100, 0b00100, 0b00100};
uint8_t font_Z[5] = {0b11111, 0b00010, 0b00100, 0b01000, 0b11111};

uint8_t* get_font(char c) {
    switch (c) {
        case 'A': return font_A; case 'B': return font_B;
        case 'C': return font_C; case 'D': return font_D;
        case 'E': return font_E; case 'F': return font_F;
        case 'G': return font_G; case 'H': return font_H;
        case 'I': return font_I; case 'J': return font_J;
        case 'L': return font_L; case 'M': return font_M;
        case 'N': return font_N; case 'O': return font_O;
        case 'P': return font_P; case 'R': return font_R;
        case 'S': return font_S; case 'T': return font_T;
        case 'U': return font_U; case 'V': return font_V;
        case 'W': return font_W; case 'X': return font_X;
        case 'Y': return font_Y; case 'Z': return font_Z;
        case ' ': return SPACE;
        default: return SPACE;
    }
}

void draw_char(int x, int y, uint8_t* bitmap, uint8_t bg_color) {
    for (int row = 0; row < 5; row++) {
        for (int col = 0; col < 5; col++) {
            int bit_on = bitmap[row] & (1 << (4 - col));
            uint8_t color = bit_on ? (bg_color << 4) | 0x0 : 0x00;
            putchar(x + col, y + row, CHAR_PIXEL, color);
        }
    }
}
void scroll_up(int pixels) {
   
    for (int y = pixels; y < VGA_HEIGHT; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            uint8_t pixel = vga_buffer[y * VGA_WIDTH + x];
            vga_buffer[(y - pixels) * VGA_WIDTH + x] = pixel;
        }
    }

 
    for (int y = VGA_HEIGHT - pixels; y < VGA_HEIGHT; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            vga_buffer[y * VGA_WIDTH + x] = 0x07; 
        }
    }
}

void LOAD(void) {
    k_clear_screen(0x07);

    const char* message =
        "THIS IS THE RESULT OF FORTY-NINE EXE A GDI MALWARE MADE BY MALWARELAB. "
        "IT WAS A VERY SIMPLE PROJECT, BUT I HOPE YOU ENJOYED IT.";

    int x = 0;
    int y = 2;
    int base_line = 2; 
    int line_count = 0;

    uint8_t last_second = get_current_second();

    // https://en.wikipedia.org/wiki/BIOS_color_attributes
    uint8_t rainbow[] = {0x4, 0x6, 0xE, 0x2, 0x3, 0x1, 0x5};
    int color_index = 0;

    const int char_width = 6;
    const int line_height = 6;

    for (int i = 0; message[i] != '\0'; i++) {
        char c = message[i];

        if (c == ' ') {
            
            int word_len = 0;
            while (message[i + 1 + word_len] != '\0' && message[i + 1 + word_len] != ' ') {
                word_len++;
            }

            
            if (x + char_width * (word_len + 1) >= VGA_WIDTH) {
                x = 0;
                y += line_height;
                line_count++;

               
                if (line_count >= 4) {
                    scroll_up(line_height); 
                    y -= line_height;
                    line_count = 3;
                }
            }

            x += char_width;
            continue;
        }

        if (x + char_width > VGA_WIDTH) {
            x = 0;
            y += line_height;
            line_count++;

            if (line_count >= 4) {
                scroll_up(line_height);
                y -= line_height;
                line_count = 3;
            }
        }

        uint8_t bg = rainbow[color_index % 7];
        color_index++;

        draw_char(x, y, get_font(c), bg);
        x += char_width;

        wait_for_next_second(last_second);
        last_second = get_current_second();
    }
}
