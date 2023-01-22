/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>


#include "pico/stdlib.h"
#include "pico/time.h"
#include "hardware/clocks.h"


#include "ws2812.pio.h"
#include "neopixel.hpp"

#define IS_RGBW true
#define WS2812_PIN 7

NeopixelGrid grid;

/// @brief Utility function to add 2 RGB values together.
/// @param c1 
/// @param c2 
/// @return 
static uint32_t addRgb(uint32_t c1, uint32_t c2){

    uint32_t r = (c1 & 0xFF0000) + (c2 & 0xFF0000);
    uint32_t g = (c1 & 0x00FF00) + (c2 & 0x00FF00);
    uint32_t b = (c1 & 0x0000FF) + (c2 & 0x0000FF);

    if(r > 0xFF0000) r = 0xFF0000;
    if(g > 0x00FF00) g = 0x00FF00;
    if(b > 0x0000FF) b = 0x0000FF;

    return r | g | b;
}

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
// Utility base class for actions
class ActionBase: public Action {
    protected:
    const static int SINE_LENGTH = 1000;
    static float sinfLookup[SINE_LENGTH];

    float hue;          // colour to animate.
    float hue2;          // colour to animate.
    float value;       // how bright the RGB is 0..1
    float count;        // number of ripples
    uint8_t white;       // white LED value.
    NeopixelGrid* grid;
    int baseIndices[PIXEL_COUNT]; // precalculated from radius & count.
    int phaseIndex;
    int inc;

    void readParameters(NeopixelGrid* grid, Command* cmd);

    public:
    ActionBase();
    virtual void tick();
    virtual void start(NeopixelGrid* grid, Command* cmd) =0;
    static void show(); // debug
};

float ActionBase::sinfLookup[ActionBase::SINE_LENGTH];

ActionBase::ActionBase()
: hue(0)
, hue2(1)
, value(1)
, count(1)
, white(0)
, grid(0)
, phaseIndex(0)
, inc(0)
{
    for(int i=0; i<SINE_LENGTH; ++i){
        sinfLookup[i] = (1.0f + sinf(i * 2.0f * (float)M_PI / (float)SINE_LENGTH))/2;
    }
}

void ActionBase::readParameters(NeopixelGrid* grid, Command* cmd){
    this->grid = grid;
    this->hue = (1.0f / SCALE) * (float)cmd->params[0];
    this->hue2 = (1.0f / SCALE) * (float)cmd->params[1];
    this->value = (1.0f / SCALE) * (float)cmd->params[2];
    this->inc  =  cmd->params[3];
    this->count = (1.0f / SCALE) * (float)cmd->params[4];
    this->white = cmd->params[5];

    if(hue < 0) hue = 0; else if(hue > 1) hue = 1.0f;
    if(hue2 < 0) hue2 = -1.0f; else if(hue2 > 1) hue2 = 1.0f;
    if(value < 0) value = 0; else if(value > 1) value = 1.0f;
    if(inc < -500) inc = 500; else if(inc > 500) inc = 500;
    if(count < 0) count = 0; else if(count > 10) count = 10;
    // ignore white - will truncate anyway.
}

void ActionBase::tick(){
  
    phaseIndex -= inc;
    if(phaseIndex < 0) 
        phaseIndex += SINE_LENGTH;
    else if(phaseIndex >= SINE_LENGTH)
        phaseIndex -= SINE_LENGTH;

    // So we've got a lookup in baseIndices of the index for each pixel of where they are 
    // in the sine array before the ripple effect is added. Add phaseIndex (varying) to
    // get the actual position and then lookup the sine value.  This avoids a lot
    // of floating point.
    //absolute_time_t t0 = get_absolute_time();
    for(int i=0; i<PIXEL_COUNT; ++i){
        int idx = baseIndices[i] + phaseIndex;
        while(idx >= SINE_LENGTH) idx -= SINE_LENGTH;

        float v1 = sinfLookup[idx]; // range 0..1
        float v2 = 1.0f - v1;
        uint32_t rgb = grid->hvToRgb(hue, v1*value);
        if(hue2 >= 0){
            rgb = addRgb(rgb, grid->hvToRgb(hue2, v2*value));
        }
        grid->setPixel(i, rgb, white);
    }
   //absolute_time_t t1 = get_absolute_time();
      grid->send();
   //absolute_time_t t2 = get_absolute_time();
  
   //printf("T0-T1: %lu, T1-T2: %lu\n", (unsigned long)absolute_time_diff_us(t0,t1), (unsigned long)absolute_time_diff_us(t1,t2));
}

////////////////////////////////////////////////////////////////////////////////////////////
class RipplesAction: public ActionBase {
  
    public:
    RipplesAction();
    virtual void start(NeopixelGrid* grid, Command* cmd);
};

RipplesAction::RipplesAction() : ActionBase() {}

void RipplesAction::start(NeopixelGrid* grid, Command* cmd){
    readParameters(grid, cmd);

    // Set up basic ripple effect indices based on radius & count.
    // Indexing into the sinf array with the phase offset gives the value of the pixel.
    for(int i=0; i<PIXEL_COUNT; ++i){
        const Coordinate& c = grid->coordinate(i);    
        int idx = int(c.r * count * SINE_LENGTH) % SINE_LENGTH;
        baseIndices[i] = idx;
    }
}
////////////////////////////////////////////////////////////////////////////////////////////
class HorizontalAction: public ActionBase {
  
    public:
    HorizontalAction();
    virtual void start(NeopixelGrid* grid, Command* cmd);
};

HorizontalAction::HorizontalAction() : ActionBase() {}

void HorizontalAction::start(NeopixelGrid* grid, Command* cmd){
    readParameters(grid, cmd);

    // Set up basic ripple effect indices based on x coordinate.
    // X is normalised from -1 to 1 so convert into range 0..1
    // Indexing into the sinf array with the phase offset gives the value of the pixel.
    for(int i=0; i<PIXEL_COUNT; ++i){
        const Coordinate& c = grid->coordinate(i);    
        int idx = int((1.0f + c.x)/2 * count * SINE_LENGTH) % SINE_LENGTH;
        baseIndices[i] = idx;
    }
}

class VerticalAction: public ActionBase {
  
    public:
    VerticalAction();
    virtual void start(NeopixelGrid* grid, Command* cmd);
};

VerticalAction::VerticalAction() : ActionBase() {}

void VerticalAction::start(NeopixelGrid* grid, Command* cmd){
    readParameters(grid, cmd);

    // Set up basic ripple effect indices based on radius & count.
    // Indexing into the sinf array with the phase offset gives the value of the pixel.
    for(int i=0; i<PIXEL_COUNT; ++i){
        const Coordinate& c = grid->coordinate(i);    
        int idx = int((1.0f + c.y)/2 * count * SINE_LENGTH) % SINE_LENGTH;
        baseIndices[i] = idx;
    }
}


////////////////////////////////////////////////////////////////////////////////////////////
class SpokesAction: public ActionBase {
    public:
    SpokesAction();
    virtual void start(NeopixelGrid* grid, Command* cmd);
};

SpokesAction::SpokesAction() : ActionBase() {}


void SpokesAction::start(NeopixelGrid* grid, Command* cmd){
    readParameters(grid, cmd);
    // Set up basic ripple effect indices based on angle & count.
    // Indexing into the sinf array with the phase offset gives the value of the pixel.
    // angle is -PI to +PI.  Want that range to sweep through SINE_LENGTH and restart to
    // zero.  So Theta/PI sweeps from -1 to +1,    (Theta/PI + 1)/2 => 0..1
    for(int i=0; i<PIXEL_COUNT; ++i){
        const Coordinate& c = grid->coordinate(i);    
        int idx = int( (c.theta / (float)M_PI + 1.0f)/2 * count * SINE_LENGTH) % SINE_LENGTH;  // theta -pi to pi
        baseIndices[i] = idx;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////

NullAction nullAction;
SetAction setAction;
ColourChangeAction colourChangeAction;
RipplesAction ripplesAction;
SpokesAction spokesAction;
HorizontalAction horizontalAction;
VerticalAction verticalAction;


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
//  We then normalise the range to lie in -1 to +1 at the edges(approx) and roughly sqrt(2) 
// in the corners.
void NeopixelGrid::initialiseCoordinates(){
    int idx = 0;
    for(int iy=0; iy<GRID_HEIGHT; ++iy){
        float y = (-3.5f + iy) * (1.0f / 3.5f);
        for(int ix=0; ix<GRID_WIDTH; ++ix){
            float x = (-3.5f + ix) * (1.0f / 3.5f);
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

uint32_t NeopixelGrid::hvToRgb(float hue, float value){
    float r, g, b;

    if(hue > 1.0) hue = 1.0;
    if(value > 1.0) value = 1.0;

    int h = (int)(hue * 6);
    float f = hue * 6 - h;
    float p = 0;
    float q = value * (1 - f );
    float t = value * f;

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

/// @brief Sets an individual pixel.
/// @param idx 
/// @param rgb 
/// @param white 
void NeopixelGrid::setPixel(int idx, uint32_t rgb, uint8_t white){
    assert(idx >= 0 && idx < PIXEL_COUNT);

    uint8_t r = (rgb & 0xFF0000)>>16;
    uint8_t g = (rgb & 0x00FF00)>>8;
    uint8_t b = (rgb & 0x0000FF);

    pixels[idx] = rgbw(r,g,b,white);
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

                case 4:  // ripples
                currentAction = &ripplesAction;
                ripplesAction.start(this, &cmd);
                break;

                case 5:  // spokes
                currentAction = &spokesAction;
                spokesAction.start(this, &cmd);
                break;

               case 6:  // horizontal
                currentAction = &horizontalAction;
                horizontalAction.start(this, &cmd);
                break;

               case 7:  // vertical
                currentAction = &verticalAction;
                verticalAction.start(this, &cmd);
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

void NeopixelGrid::sendRippleCmd(uint16_t code, float hue, float hue2, float value, float increment, float count, uint8_t white){
    Command cmd;
    cmd.code = code;
    cmd.params[0] = (int32_t)(hue * SCALE);
    cmd.params[1] = (int32_t)(hue2 * SCALE);
    cmd.params[2] = (int32_t)(value * SCALE); 
    cmd.params[3] = increment;
    cmd.params[4] = (int32_t)(count * SCALE); 
    cmd.params[5] = white;
    run(&cmd);
}

void NeopixelGrid::rippleAsync(float hue, float hue2, float value, float increment, float count, uint8_t white){
    sendRippleCmd(4, hue, hue2, value, increment, count, white);
 }

void NeopixelGrid::spokesAsync(float hue, float hue2, float value, float increment, float count, uint8_t white){
    sendRippleCmd(5, hue, hue2, value, increment, count, white);
}

void NeopixelGrid::horizontalAsync(float hue, float hue2, float value, float increment, float count, uint8_t white){
    sendRippleCmd(6, hue, hue2, value, increment, count, white);
}

void NeopixelGrid::verticalAsync(float hue, float hue2, float value, float increment, float count, uint8_t white){
    sendRippleCmd(7, hue, hue2, value, increment, count, white);
}