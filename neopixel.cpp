/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>


#include "pico/stdlib.h"
#include "hardware/clocks.h"


#include "ws2812.pio.h"
#include "neopixel.hpp"

#define IS_RGBW true
#define WS2812_PIN 7

NeopixelGrid grid;


class NullAction: public Action {
    public:
    virtual void tick(){}
    virtual void start(NeopixelGrid* grid, Command* cmd) {}
};

class SetAction: public Action {
    public:
    virtual void tick(){}
    virtual void start(NeopixelGrid* grid, Command* cmd) {grid->set(cmd->params[0], cmd->params[1]);}
};


////////////////////////////////////////////////////////////////////////////////////////////
class ColourChangeAction: public Action {
    float hue;         // current hue
    float value;       // how bright the RGB is
    float increment;   // for hue per tick
    uint8_t white;       // white LED value.
    NeopixelGrid* grid;

    public:
    virtual void tick();
    virtual void start(NeopixelGrid* grid, Command* cmd);
};

void ColourChangeAction::tick(){
    hue = hue + increment;
    if(hue > 1.0) hue = 0;
    uint32_t rgb = grid->hsvToRgb(hue, 1.0, value);
    grid->set(rgb, white);
}

void ColourChangeAction::start(NeopixelGrid* grid, Command* cmd){
    this->grid = grid;
    this->hue = 0;
    this->value = (1.0f / SCALE) * (float)cmd->params[0];
    this->increment = (1.0f/ SCALE) * (float)cmd->params[1];
    this->white = cmd->params[2];
}

////////////////////////////////////////////////////////////////////////////////////////////

NullAction nullAction;
SetAction setAction;
ColourChangeAction colourChangeAction;


////////////////////////////////////////////////////////////////////////////////////////////

NeopixelGrid::NeopixelGrid()
: currentAction(0)
, cycle_time(100)
, cycle(100)
{
   // todo get free sm
    PIO pio = pio0;
    int sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);

    ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, IS_RGBW);

    for(int i=0; i<PIXEL_COUNT; ++i){
        pixels[i] = 0;
    }

    queue_init(	&commandQueue, sizeof(Command), 16);
    initialiseCoordinates();

}


// Precalculate coordinates of each pixel on a -1..1 grid in both cartesian and polar
// values.  These are for the animations.
//   Pixels at points - 8 pixels but only 7 units between them so logically we have
//   the following for uniform spacing.  
//            0 -> -3.5 
//            1 -> -2.5
//            2 -> -1.5
//            3 -> -0.5
//            4 -> +0.5
//            5 -> 1.5
//            6 -> 2.5
//            7 -> 3.5
//  We then normalise the range to lie in -1 to +1
void NeopixelGrid::initialiseCoordinates(){
    int idx = 0;
    for(int iy=0; iy<GRID_HEIGHT; ++iy){
        float y = (-3.5 + iy) * (1.0 / 3.5);
        for(int ix=0; ix<GRID_WIDTH; ++ix){
            float x = (-3.5 + ix) * (1.0 / 3.5);
            float radius = std::sqrt(x*x + y*y);
            float theta = std::atan2(y,x);
            coordinates[idx].x = x;
            coordinates[idx].y = y;
            coordinates[idx].r = radius;
            coordinates[idx].theta = theta;
            ++idx;
        }
    }
   
}

uint32_t NeopixelGrid::hsvToRgb(float hue, float saturation, float value){
    float r, g, b;

    if(hue > 1.0) hue = 1.0;
    if(saturation > 1.0) saturation = 1.0;
    if(value > 1.0) value = 1.0;

    int h = (int)(hue * 6);
    float f = hue * 6 - h;
    float p = value * (1 - saturation);
    float q = value * (1 - f * saturation);
    float t = value * (1 - (1 - f) * saturation);

    if (h == 0) {
        r = value;
        g = t;
        b = p;
    } else if (h == 1) {
        r = q;
        g = value;
        b = p;
    } else if (h == 2) {
        r = p;
        g = value;
        b = t;
    } else if (h == 3) {
        r = p;
        g = q;
        b = value;
    } else if (h == 4) {
        r = t;
        g = p;
        b = value;
    } else if (h <= 6) {
        r = value;
        g = p;
        b = q;
    } else {
        assert(false);
    }

    uint32_t rgb = 
        ((int)(r * 255) << 16) +
        ((int)(g * 255) << 8) +
        (int (b * 255));
    return rgb;
}

/// @brief Sends the array of pixels to the display.
/// TODO - use DMA
void NeopixelGrid::send(){
    for(int i=0; i<PIXEL_COUNT; ++i){
        pio_sm_put_blocking(pio0, 0, pixels[i]);
    }
}

void NeopixelGrid::set(uint32_t rgb, uint8_t white){
    this->colour = rgb;
    this->white = white;

    uint8_t r = (rgb & 0xFF0000)>>16;
    uint8_t g = (rgb & 0x00FF00)>>8;
    uint8_t b = (rgb & 0x0000FF);

    uint32_t pixel = rgbw(r,g,b,white);
    for(int i=0; i<PIXEL_COUNT; ++i){
        pixels[i] = pixel;
    }
    send();

}

void NeopixelGrid::tick() {
    if(!queue_is_empty(&commandQueue)){
        Command cmd;
        if(queue_try_remove(&commandQueue,&cmd)){
            switch(cmd.code){
                case 0:  // Turn off any animation
                currentAction = 0;
                break;

                case 1:  // set colours of whole grid
                set(cmd.params[0], cmd.params[1]);
                currentAction = 0;
                break;

                case 2:  // set cycle time for animation.
                cycle_time = cmd.params[1];
                break;

                case 3:  // Colour cycle.
                currentAction = &colourChangeAction;
                colourChangeAction.start(this, &cmd); 
                break;
            }
        }
    }
    --cycle;
    if(cycle == 0){
        if(currentAction){
            currentAction->tick();
        }
        cycle = cycle_time;
    }
}
    
bool NeopixelGrid::run(Command* cmd){
    bool success = queue_try_add(&commandQueue, cmd);
    return success;
}


void run_neopixel() {
    grid.send();
    while(true) {
        grid.tick();
        sleep_ms(1);
    }
}

void NeopixelGrid::setAsync(uint32_t rgb, uint8_t white){
    Command cmd;
    cmd.code = 1; // set colour
    cmd.params[0] = rgb;
    cmd.params[1] = white;
    run(&cmd);
}

void NeopixelGrid::colourChangeAsync(float value, float increment, uint8_t white){
    Command cmd;
    cmd.code = 3;  // colour change
    cmd.params[0] = (int32_t)(value * SCALE);
    cmd.params[1] = (int32_t)(increment * SCALE); 
    cmd.params[2] = white;
    run(&cmd);
}
