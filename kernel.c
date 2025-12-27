#include <stdint.h>

/* --- Hardware Communication --- */
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outb(uint16_t port, uint8_t val) {
    asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

#define WIDTH 80
#define HEIGHT 25
static uint16_t* const VGA_MEMORY = (uint16_t*) 0xB8000;

/* --- OS States & Browser Data --- */
typedef enum { DESKTOP, BROWSER } MODE;
MODE current_mode = DESKTOP;
char url_buffer[50] = "";
int url_idx = 0;
char page_content[100] = "Ready to browse MEWO-NET...";

/* --- Full Keyboard Map --- */
const char scancode_map[] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0,
    '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' '
};

/* --- Graphics --- */
void kprint(const char* str, int x, int y, uint8_t col) {
    for (int i = 0; str[i] != '\0'; i++) {
        VGA_MEMORY[y * WIDTH + x + i] = (uint16_t)str[i] | (uint16_t)col << 8;
    }
}

void draw_rect(int x, int y, int w, int h, uint8_t col) {
    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            VGA_MEMORY[(y + i) * WIDTH + (x + j)] = (uint16_t)' ' | (uint16_t)col << 8;
        }
    }
}

/* --- App Rendering --- */
void render() {
    if (current_mode == DESKTOP) {
        draw_rect(0, 0, WIDTH, HEIGHT, 0x1F); // Blue Desktop
        kprint("=== MEWO-OS DESKTOP ===", 28, 10, 0x1F);
        kprint("F1: Desktop  |  F2: Web Browser", 25, 12, 0x1B);
    } else {
        draw_rect(0, 0, WIDTH, HEIGHT, 0x70); // Gray Browser
        draw_rect(0, 1, WIDTH, 1, 0x0F);      // White Address Bar
        kprint("ADDRESS: ", 1, 1, 0x0F);
        kprint(url_buffer, 10, 1, 0x0F);
        kprint(page_content, 5, 5, 0x71);
    }
    // Bottom Taskbar
    draw_rect(0, 24, WIDTH, 1, 0x07);
    kprint("F1: HOME | F2: BROWSER | F12: REBOOT", 1, 24, 0x07);
}

/* --- Interaction Logic --- */
void handle_keyboard() {
    if (inb(0x64) & 1) {
        uint8_t sc = inb(0x60);
        if (sc & 0x80) return; // Ignore key release

        // Global Keybinds
        if (sc == 0x3B) { current_mode = DESKTOP; render(); } // F1
        if (sc == 0x3C) { current_mode = BROWSER; render(); } // F2
        if (sc == 0x58) { outb(0x64, 0xFE); }                // F12 Reboot Laptop

        if (current_mode == BROWSER) {
            if (sc == 0x1C) { // ENTER
                if (url_buffer[0] == 'g') { 
                    const char* msg = "GITHUB: MEWO Official Repo - Status: Online";
                    for(int i=0; i<45; i++) page_content[i] = msg[i];
                } else if (url_buffer[0] == 'y') {
                    const char* msg = "YOUTUBE: Playing 'OS Dev Tutorial'...";
                    for(int i=0; i<38; i++) page_content[i] = msg[i];
                } else {
                    const char* msg = "Error: Site not found in MEWO DNS.";
                    for(int i=0; i<35; i++) page_content[i] = msg[i];
                }
                render();
            } else if (sc == 0x0E && url_idx > 0) { // BACKSPACE
                url_buffer[--url_idx] = '\0';
                render();
            } else if (sc < sizeof(scancode_map) && scancode_map[sc]) {
                if (url_idx < 45) {
                    url_buffer[url_idx++] = scancode_map[sc];
                    url_buffer[url_idx] = '\0';
                    render();
                }
            }
        }
    }
}

void kernel_main() {
    render();
    while (1) {
        handle_keyboard();
    }
}