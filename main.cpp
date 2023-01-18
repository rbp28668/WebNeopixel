
extern "C" {
#include <cyw43.h>
}
#include "pico/multicore.h"
#include "wifi.hpp"
#include "server.hpp"
#include "webserver.hpp"
#include "neopixel_webapp.hpp"

WifiStation station;
Webserver webserver;
NeopixelWebapp webapp;

// TODO GET /favicon.ico HTTP/1.1
// TODO - better 404 support

extern void run_neopixel();

extern unsigned int checkCode();

int main() {

    stdio_init_all();
    printf("\n\nNeoPixel Startup\n");

    printf("Startup checksum %08x\n",checkCode());
    // start neopixels on cpu 1
    multicore_launch_core1(run_neopixel);

    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);

    uint8_t mac[6];
    if(cyw43_wifi_get_mac(&cyw43_state, CYW43_ITF_STA, mac) == 0){
        printf("MAC ADDR: " );
        for(int i=0;i<5;++i) printf("%02.2x:",mac[i]);
        printf("%2.2x\n",mac[5]);
    }
  
    const char* ssid = PICO_WIFI_SSID;
    const char* password = PICO_WIFI_PASSWORD;
    while(true) {
        if(station.connect(ssid, password, 30000)) {

            webserver.addAppliction(&webapp);

            TcpServer server(&webserver);
            if(server.open(8080)){
                printf("Run server checksum %08x\n",checkCode());

                server.run();
            }
            server.close();
        }
        printf("NeoPixel Restart\n");
        sleep_ms(1000);
        station.restart();
    }
}