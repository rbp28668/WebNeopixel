
#ifndef WIFI_H
#define WIFI_H

#include "pico/stdlib.h"

class Wifi {

    protected:
    bool _failed;
    Wifi(); //

    public: 
    ~Wifi();

    bool connect(const char* ssid, const char* pass, uint32_t timeout = 30000);
 };

 class WifiStation : public Wifi {
    public:
    WifiStation();
    void restart();
 };

 class WifiAccessPoint : public Wifi {
    public:
    WifiAccessPoint(const char * ssid, const char *	password, uint32_t 	auth );
 };

 #endif