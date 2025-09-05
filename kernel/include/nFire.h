#include "FOTO/nCRACKXX.h"
#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25
void nCracxx() {
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
        uint8_t c = Nfire[i];
        uint8_t color = ncolore[i];
        uint16_t v = (color << 8) | c;
        VIDEO_MEMORY[i] = v;
    }
}
static inline void scrivi(int x, int y, char c, uint8_t color) {
    if (x < 0 || x >= SCREEN_WIDTH || y < 0 || y >= SCREEN_HEIGHT) return;
    VIDEO_MEMORY[y * SCREEN_WIDTH + x] = ((uint16_t)color << 8) | (uint8_t)c;
}

void DISEGNA_NCRACK() {
    k_clear_screen(0x00);
    nCracxx();
    const char* msg = "DONA 2 FOTTUTI EURO. MalwareLab150 vi vuole bene :)";
    int len = 0;
    while (msg[len]) len++;

    
    int start_x = (SCREEN_WIDTH - len) / 2;
    int y = SCREEN_HEIGHT / 2;

    for (int i = 0; i < len; i++) {
        scrivi(start_x + i, y, msg[i], 0x04); 
    }
}
