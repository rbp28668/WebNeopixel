/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <stdlib.h>


#include "pico/stdlib.h"
#include "hardware/pio.h"
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

NullAction nullAction;
SetAction setAction;

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

}

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
                break;

                case 2:  // set cycle time for animation.
                cycle_time = cmd.params[1];
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

