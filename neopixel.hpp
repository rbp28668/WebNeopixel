#ifndef NEOPIXEL_HPP
#define NEOPIXEL_HPP


#include "pico/stdlib.h"
#include "pico/util/queue.h"
#include "hardware/pio.h"
#include "dma.hpp"

#define GRID_WIDTH (8)
#define GRID_HEIGHT (8)
#define PIXEL_COUNT (GRID_WIDTH * GRID_HEIGHT)
#define SCALE (10000.0f)  // for sending floats via ints


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
    int32_t params[8];
};

/// @brief interface for animated patterns.
class Action{
    public:
    virtual void tick() = 0;
    virtual void start(NeopixelGrid* grid, Command* cmd) = 0;
};

/// @brief Holds normalised coordinates of a pixel
class Coordinate {
    public:
    float x;    // normalised to range -1 .. 1
    float y;    // normalised to range -1 .. 1
    float r;    // normalised to range 0 .. 1.414  (in the corners, 1 at edge)
    float theta; // -PI to +PI
};

class NeopixelGrid {

    uint32_t colour;
    uint32_t white;

    uint32_t buffer[2*PIXEL_COUNT]; // Allow for double buffering.
    uint32_t* pixels;

    PIO pio; // which PIO is in use to drive the pixels.
    uint sm; // and which statemachine is in use.

    Dma dma; // to shovel pixels to the PIO

    const uint32_t RED = 0x00FF0000;
    const uint32_t GREEN = 0xFF000000;
    const uint32_t BLUE = 0x0000FF00;
    const uint32_t WHITE = 0x000000FF;


    // inline void put_pixel(uint32_t pixel) {
    //     pio_sm_put_blocking(pio, sm, pixel);
    // }

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
    Coordinate coordinates[PIXEL_COUNT];

    void initialiseCoordinates();

    void sendRippleCmd(uint16_t code, float hue, float hue2, float value, float increment, float count, uint8_t white );

    public:
    NeopixelGrid();
    void send();
    void set(uint32_t rgb, uint8_t white);  // whole grid
    void setPixel(int idx, uint32_t rgb, uint8_t white);  // single pixel
    void setPixelRaw(int idx, uint32_t rgbw);  // single pixel in Neopixel format
    
    void tick(); // to run commands, animate etc.

    static uint32_t hsvToRgb(float h, float s, float v); 
    static uint32_t hvToRgb(float hue, float value); // assumes s = 1
   
    const Coordinate& coordinate(int idx) { return coordinates[idx];}
    bool run(Command* cmd); // true if accepted to run.

    void setAsync(uint32_t rgb, uint8_t white);
    void rateAsync(unsigned int rate);
    void colourChangeAsync(float value, float increment, uint8_t white);
    void rippleAsync(float hue, float hue2, float value, float increment, float count, uint8_t white);
    void spokesAsync(float hue, float hue2, float value, float increment, float count, uint8_t white);
    void horizontalAsync(float hue, float hue2, float value, float increment, float count, uint8_t white);
    void verticalAsync(float hue, float hue2, float value, float increment, float count, uint8_t white);
    void sparkleAsync();
};


#endif