 #include <stdio.h>
 
 #include "pico/stdlib.h"
 #include "pico/cyw43_arch.h"

#include "wifi.hpp"

 Wifi::Wifi() : _failed(false){
    if (cyw43_arch_init_with_country(CYW43_COUNTRY_UK)) {
        _failed = true;
    }
  }

 Wifi::~Wifi() {
    cyw43_arch_deinit();
  }
  
 bool Wifi::connect(const char* ssid, const char* pass, uint32_t timeout){
    return cyw43_arch_wifi_connect_timeout_ms(ssid, pass, CYW43_AUTH_WPA2_AES_PSK, timeout) == 0;
}



WifiStation::WifiStation(){
    cyw43_arch_enable_sta_mode();
}
 
void WifiStation::restart(){
    cyw43_arch_deinit();
    cyw43_arch_init_with_country(CYW43_COUNTRY_UK);
    cyw43_arch_enable_sta_mode();
}

 WifiAccessPoint::WifiAccessPoint(const char * ssid, const char * password, uint32_t auth ){
    cyw43_arch_enable_ap_mode(ssid, password, auth);
 }