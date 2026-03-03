#include <stdint.h>

/* --- 1. MULTIBOOT HEADER (High Resolution: 1920x1080) --- */
#define MULTIBOOT_MAGIC 0x1BADB002
#define MULTIBOOT_FLAGS 0x00000007 

struct multiboot_header {
    uint32_t magic; uint32_t flags; uint32_t checksum;
    uint32_t unused[5]; uint32_t mode; uint32_t width; uint32_t height; uint32_t depth;
} __attribute__((packed));

__attribute__((section(".multiboot")))
const struct multiboot_header header = {
    MULTIBOOT_MAGIC, MULTIBOOT_FLAGS, -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS),
    {0,0,0,0,0}, 0, 1920, 1080, 32
};

/* --- 2. HAL (Hardware Abstraction Layer) --- */
// "The bridge between code and actual CPU instructions"
static inline void outb(uint16_t port, uint8_t val) { asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port)); }
static inline uint8_t inb(uint16_t port) { uint8_t ret; asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port)); return ret; }

void mouse_wait(uint8_t type) {
    uint32_t timeout = 100000;
    if (type == 0) while (timeout-- && !(inb(0x64) & 1));
    else while (timeout-- && (inb(0x64) & 2));
}

void hal_init_mouse() {
    uint8_t status;
    mouse_wait(1); outb(0x64, 0xA8); // Enable Mouse Port
    mouse_wait(1); outb(0x64, 0x20); // Get Command Byte
    mouse_wait(0); status = (inb(0x60) | 2); // Enable IRQ12
    mouse_wait(1); outb(0x64, 0x60); // Set Command Byte
    mouse_wait(1); outb(0x60, status);
    mouse_wait(1); outb(0x64, 0xD4); 
    mouse_wait(1); outb(0x60, 0xF4); // Enable Streaming
    mouse_wait(0); inb(0x60);        // ACK
}

/* --- 3. GDI (Graphics Device Interface) --- */
// "The engine that draws windows and text"
uint32_t* front_buffer;
uint32_t* back_buffer;
uint32_t fb_pitch_pixels;

void gdi_put_pixel(int x, int y, uint32_t color) {
    if (x < 0 || y < 0 || x >= 1920 || y >= 1080) return;
    back_buffer[y * fb_pitch_pixels + x] = color;
}

void gdi_draw_rect(int x, int y, int w, int h, uint32_t color) {
    for (int j = 0; j < h; j++) {
        for (int i = 0; i < w; i++) gdi_put_pixel(x + i, y + j, color);
    }
}

/* --- 4. EXECUTIVE SERVICES & SHELL --- */
// Window Manager: "Handles overlapping and dragging"
int mouse_x = 960, mouse_y = 540;
int win_x = 100, win_y = 100, win_w = 500, win_h = 350;

void render_desktop() {
    // 1. Wallpaper (Desktop Manager)
    for (uint32_t i = 0; i < 1080 * fb_pitch_pixels; i++) back_buffer[i] = 0xFF1A1C1E;

    // 2. Taskbar (Skeleton UI)
    gdi_draw_rect(0, 1030, 1920, 50, 0xAAFFFFFF);

    // 3. Main Window (Window Manager)
    gdi_draw_rect(win_x, win_y, win_w, 30, 0xFF0078D4); // Blue Header
    gdi_draw_rect(win_x, win_y + 30, win_w, win_h - 30, 0xFFF3F3F3); // Body

    // 4. Mouse Cursor (HAL driven)
    gdi_draw_rect(mouse_x, mouse_y, 8, 8, 0xFFFFFFFF);

    // 5. I/O Manager: Swap buffers to screen
    for (int y = 0; y < 1080; y++) {
        for (int x = 0; x < 1920; x++) {
            front_buffer[y * fb_pitch_pixels + x] = back_buffer[y * fb_pitch_pixels + x];
        }
    }
}

/* --- 5. EXECUTIVE KMAIN --- */
void kmain(void* mbi_ptr) {
    uint32_t* mbi = (uint32_t*)mbi_ptr;
    front_buffer = (uint32_t*)(uintptr_t)mbi[22]; 
    fb_pitch_pixels = mbi[24] / 4; 
    back_buffer = (uint32_t*)0x1400000; 

    hal_init_mouse(); // Re-adds the mouse driver

    while(1) {
        // Handle Mouse Input
        if (inb(0x64) & 1) {
            uint8_t status = inb(0x64);
            if (status & 0x20) {
                int8_t b1 = inb(0x60);
                int8_t rel_x = inb(0x60);
                int8_t rel_y = inb(0x60);
                mouse_x += rel_x; 
                mouse_y -= rel_y;
            }
        }
        render_desktop();
        asm volatile("hlt");
    }
}