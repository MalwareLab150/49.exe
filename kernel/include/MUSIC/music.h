#define SLOWDOWN_FACTOR 80 // Aumenta questo valore per rallentare ulteriormente
#define PIT_CHANNEL2 0x42
#define PIT_COMMAND  0x43
#define PIT_FREQUENCY 1193180

typedef struct {
    uint32_t freq;   
    int duration;  
} Note;

static inline uint8_t inb(uint16_t port);

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" :: "a"(val), "Nd"(port));
}

void speaker_on() {
    uint8_t tmp = inb(0x61);
    if ((tmp & 3) != 3) {
        outb(0x61, tmp | 3);
    }
}

void speaker_off() {
    uint8_t tmp = inb(0x61) & 0xFC;
    outb(0x61, tmp);
}


void play_frequency(uint32_t freq) {
    if (freq == 0) {
        speaker_off();
        return;
    }
    uint16_t div = (uint16_t)(PIT_FREQUENCY / freq);
    outb(PIT_COMMAND, 0xB6);
    outb(PIT_CHANNEL2, (uint8_t)(div & 0xFF));
    outb(PIT_CHANNEL2, (uint8_t)((div >> 8) & 0xFF));
    speaker_on();
}



Note melody[] = {

    {440, 6}, {494, 6}, {523, 6}, {587, 6}, {659, 8}, {0, 2},
    {587, 6}, {523, 6}, {494, 6}, {440, 6}, {392, 8}, {0, 2},

    {330, 4}, {349, 4}, {392, 4}, {440, 4}, {494, 6}, {0, 2},
    {440, 4}, {392, 4}, {349, 4}, {330, 4}, {294, 6}, {0, 2},

    {262, 4}, {294, 4}, {330, 4}, {349, 4}, {392, 4}, {440, 6},
    {494, 6}, {523, 8}, {587, 10}, {0, 4},
    {659, 4}, {698, 4}, {784, 6}, {0, 2},
    {784, 4}, {698, 4}, {659, 6}, {587, 6}, {523, 6}, {440, 8},

    {330, 4}, {392, 4}, {440, 6}, {494, 6}, {523, 8}, {0, 2},
    {659, 8}, {587, 8}, {523, 10}, {440, 12}, {0, 6},
    {523, 28},
    {587, 32},
    {659, 36},
    {698, 32},
    {784, 40},
    {349, 44},
    {262, 48},
    {0, 24},
    {330, 12},
    {0, 4},
    {349, 12},
    {0, 4},
    {370, 12},
    {0, 4},
    {392, 16},
    {220, 20},
    {196, 40},
    {262, 28},
    {330, 32},
    {392, 36},
    {494, 40},
    {392, 36},
    {330, 32},
    {262, 28},
    {220, 36},
    {0, 16},
    {247, 36},
    {0, 12},
    {311, 44},
    {0, 12},
    {415, 48},
    {0, 20},
    {196, 40},
    {220, 36},
    {247, 32},
    {175, 44},
    {262, 36},
    {0, 12},
    {330, 28},
    {392, 32},
    {523, 36},
    {587, 40},
    {659, 44},
    {440, 36},
    {0, 16},
    {220, 32},
    {0, 8},
    {220, 32},
    {311, 28},
    {175, 40},
    {0, 12},
    {440, 24},
    {0, 8},
    {262, 28},
    {330, 32},
    {415, 36},
    {494, 40},
    {370, 28},
    {554, 36},
    {0, 20},
    {175, 48},
    {220, 44},
    {262, 40},
    {349, 36},
    {523, 28},
    {349, 36},
    {262, 40},
    {0, 24},
    {587, 36},
    {523, 32},
    {494, 36},
    {440, 40},
    {392, 44},
    {349, 48},
    {262, 52},
    {0, 20},
    {311, 32},
    {370, 28},
    {415, 36},
    {466, 32},
    {311, 40},
    {220, 44},
    {0, 16},
    {196, 32},
    {220, 36},
    {247, 40},
    {262, 44},
    {294, 48},
    {330, 52},
    {349, 56},
    {0, 20},
    20,
    294,
    247,
    392,
    262,
    330,
    440,
    311,
    523,
    175,
    311,
    523,
    175,
    330,
    440,
    311,
    523,
    175,

};

int melody_length = sizeof(melody) / sizeof(melody[0]);


static int current_note = 0;
static int note_timer = 0;
void music_tick() {
    if (note_timer <= 0) {
        // passa alla prossima nota
        play_frequency(melody[current_note].freq);
        note_timer = melody[current_note].duration * SLOWDOWN_FACTOR;  // Aggiungi il fattore di rallentamento
        current_note++;
        if (current_note >= melody_length) {
            current_note = 0; // ricomincia il loop
        }
    } else {
        note_timer--;
    }
}