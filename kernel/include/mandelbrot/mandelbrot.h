#define MAX_ITER     1000
#define KEYBOARD_STATUS_PORT 0x64
#define BG_COLOR 0x0B

uint8_t read_key_scancode() {
    while ((inb(KEYBOARD_STATUS_PORT) & 1) == 0);
    return inb(KEYBOARD_DATA_PORT);
}

uint8_t mandelbrot_color(int iter) {
    static const uint8_t palette[] = {
        0x1,
        0x2,
        0x3,
        0x4,
        0x5,
        0x6,
        0xE,
        0xF
    };

    int palette_size = sizeof(palette) / sizeof(palette[0]);
    return palette[iter % palette_size];
}

void print_instructions() {
    volatile unsigned short* video = (unsigned short*) VIDEO_MEMORY;

    const char *line1 = "Use WASD to move, J/O to zoom, L to rotate, ESC to exit.";
    const char *line2 = "If you want shutdown your VM/PC press F.";

    for (int i = 0; line1[i] && i < SCREEN_WIDTH; i++) {
        video[i] = (0x0F << 8) | line1[i];
    }

    for (int i = 0; line2[i] && i < SCREEN_WIDTH; i++) {
        video[SCREEN_WIDTH + i] = (0x0F << 8) | line2[i];
    }
}

void keyboard_game(int *mandelbrot_x, int *mandelbrot_y, double *scaleX, double *scaleY, bool *exit_flag, bool *vertical_mode) {
    uint8_t scancode = read_key_scancode();

    switch(scancode) {
        case 0x11: (*mandelbrot_y)--; break;        // W
        case 0x1F: (*mandelbrot_y)++; break;        // S
        case 0x1E: (*mandelbrot_x)--; break;        // A
        case 0x20: (*mandelbrot_x)++; break;        // D
        case 0x24: *scaleX /= 1.1; *scaleY /= 1.1; break; // J (zoom in)
        case 0x18: *scaleX *= 1.1; *scaleY *= 1.1; break; // O (zoom out)
        case 0x26: *vertical_mode = !(*vertical_mode); break; // L (rotate)
        case 0x21: acpi_shutdown(); break;          // F (shutdown)
        case 0x01: *exit_flag = true; break;        // ESC (exit)
        default: break;
    }
}

void drawMandelbrot(int mandelbrot_x, int mandelbrot_y, double scaleX, double scaleY, bool vertical_mode) {
    volatile unsigned short* video = (unsigned short*) VIDEO_MEMORY;

    double centerX = -0.5 + mandelbrot_x * scaleX;
    double centerY = 0.0 + mandelbrot_y * scaleY;

    for (int y = 2; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            double cr, ci;

            if (!vertical_mode) {
                cr = (x - SCREEN_WIDTH / 2) * scaleX + centerX;
                ci = (y - SCREEN_HEIGHT / 2) * scaleY + centerY;
            } else {
                cr = (y - SCREEN_HEIGHT / 2) * scaleX + centerX;
                ci = (x - SCREEN_WIDTH / 2) * scaleY + centerY;
            }

            double zr = 0.0, zi = 0.0;
            int iter = 0;

            while (zr*zr + zi*zi <= 4.0 && iter < MAX_ITER) {
                double temp = zr*zr - zi*zi + cr;
                zi = 2.0*zr*zi + ci;
                zr = temp;
                iter++;
            }

            if (iter == MAX_ITER) {
                video[y * SCREEN_WIDTH + x] = ((BG_COLOR << 4) | BG_COLOR) << 8 | ' ';
            } else {
                uint8_t fg = mandelbrot_color(iter);
                video[y * SCREEN_WIDTH + x] = ((BG_COLOR << 4) | fg) << 8 | 219;
            }
        }
    }
}

void MANDELBROT_THING() {
    k_clear_screen(0x00);
    int mandelbrot_x = 0, mandelbrot_y = 0;
    double scaleX = 3.5 / SCREEN_WIDTH;
    double scaleY = 2.0 / SCREEN_HEIGHT;
    bool exit_flag = false;
    bool vertical_mode = false;

    while (!exit_flag) {
        print_instructions();
        drawMandelbrot(mandelbrot_x, mandelbrot_y, scaleX, scaleY, vertical_mode);

        if (inb(KEYBOARD_STATUS_PORT) & 1) {
            keyboard_game(&mandelbrot_x, &mandelbrot_y, &scaleX, &scaleY, &exit_flag, &vertical_mode);
        }
    }

    return 0;
}
