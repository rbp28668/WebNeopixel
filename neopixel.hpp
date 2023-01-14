#ifndef NEOPIXEL_HPP
#define NEOPIXEL_HPP


#include "pico/stdlib.h"
#include "pico/util/queue.h"
#include "hardware/pio.h"

#define PIXEL_COUNT (64)

   // Pixel format is:  GGRRBBWW
    // With input in top left and words on silk screen in normal orientation
    // layout of pixels is:
    // 0  1  2  3  4  5  6  7
    // 8  9  10 11 12 13 14 15
    // 16 17 18 19 20 21 22 23
    // ...
    // 56 57 58 59 60 61 62 63

class NeopixelGrid;

struct Command{
    uint16_t code;
    uint32_t params[8];
};

class Action{
    public:
    virtual void tick() = 0;
    virtual void start(NeopixelGrid* grid, Command* cmd) = 0;
};



class NeopixelGrid {

    uint32_t colour;
    uint32_t white;

    uint32_t pixels[PIXEL_COUNT];

    const uint32_t RED = 0x00FF0000;
    const uint32_t GREEN = 0xFF000000;
    const uint32_t BLUE = 0x0000FF00;
    const uint32_t WHITE = 0x000000FF;


    static inline void put_pixel(uint32_t pixel) {
        pio_sm_put_blocking(pio0, 0, pixel);
    }

    static inline uint32_t rgb(uint8_t r, uint8_t g, uint8_t b) {
        return
                ((uint32_t) (r) << 16) |
                ((uint32_t) (g) << 24) |
                ((uint32_t) (b) << 8);
    }

    static inline uint32_t rgbw(uint8_t r, uint8_t g, uint8_t b, uint8_t w) {
        return
                ((uint32_t) (r) << 16) |
                ((uint32_t) (g) << 24) |
                ((uint32_t) (b) << 8) |
                (uint32_t) w;
    }

    uint cycle_time; // for animation etc.
    uint cycle;
    Action* currentAction;
    queue_t commandQueue;

    public:
    NeopixelGrid();
    void send();
    void set(uint32_t rgb, uint8_t white);
    void tick(); // to run commands, animate etc.
    
    bool run(Command* cmd); // true if accepted to run.
};


#endif