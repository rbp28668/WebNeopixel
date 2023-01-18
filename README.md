# WebNeopixel
Use a PicoW to control a neopixel (ws2812) grid using HTTP from a browser.

Designed to control an Adafruit 8x8 RGBW grid. 
This uses the PicoW with wifi/tcp in raw polling mode.  The webserver runs on core 0 and the Neopixel control on core 1.  
Commands are passed by a simple queue.

The webserver software is layered:
Many WebApps can be regiistered with the WebServer.
The WebServer is a specialisation of SerrverApplication
ServerApplication hides a lot of the underlying detail of the LwIP stack.

WebApps are simple classes - if they match a URL they get to process it.


Note that WIFI_SSID and WIFI_PASSWORD need to be defined for this to build properly.
