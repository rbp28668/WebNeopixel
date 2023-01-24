# WebNeopixel
Use a PicoW to control a neopixel (ws2812) grid using HTTP from a browser.

Designed to control an Adafruit 8x8 RGBW grid. 
This uses the PicoW with wifi/tcp in raw polling mode.  The webserver runs on core 0 and the Neopixel control on core 1.  
Commands are passed by a simple queue.

## Webserver
This is built on a generic web server (the classes )
The webserver software is layered:
Many WebApps can be regiistered with the WebServer.
The WebServer is a specialisation of SerrverApplication
ServerApplication hides a lot of the underlying detail of the LwIP stack.

For memory allocation it uses a custom block pool in block_malloc and block_list to allow complex
structures to be built up when parsing a web request.

## Webapps
WebApps are simple classes - if they match a URL they get to process it.
THere's 
 * Neopixel webapp - sends commands to the neopixel engine running on core 1.
 * Teapot - responds to /coffee with 418 I'm a teapot.
 * Index - responds to / and /index URLs with a web page.

if there is no matching webapp the web server defaults to webapp404.

## Misc
This uses DHCP to find an address and listens on port 80.

Note that WIFI_SSID and WIFI_PASSWORD need to be defined for this to build properly.

