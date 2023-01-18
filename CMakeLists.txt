

cmake_minimum_required(VERSION 3.16)

set(PICO_BOARD pico_w)

# Pull in PICO SDK (must be before project)
include(pico_sdk_import.cmake)


project(neopixel C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

add_compile_definitions( 
        PICO_WIFI_SSID="$ENV{WIFI_SSID}" 
        PICO_WIFI_PASSWORD="$ENV{WIFI_PASSWORD}" 
)

# Initialize the SDK
pico_sdk_init()


add_executable(neopixel)
       
pico_generate_pio_header(neopixel ${CMAKE_CURRENT_LIST_DIR}/ws2812.pio)
#pico_generate_pio_header(neopixel ${CMAKE_CURRENT_LIST_DIR}/ws2812.pio OUTPUT_DIR ${CMAKE_CURRENT_LIST_DIR}/generated)

target_sources(neopixel PRIVATE
main.cpp
neopixel.cpp
dma.cpp
wifi.cpp
server.cpp
webserver.cpp
block_malloc.cpp
block_list.cpp
webapp404.cpp
neopixel_webapp.cpp
crc32.cpp
)

target_include_directories(neopixel PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Pull in our pico_stdlib which pulls in commonly used features
target_link_libraries(neopixel PRIVATE
        pico_stdlib
        hardware_pio 
        hardware_dma
        pico_cyw43_arch_lwip_poll
        pico_multicore
        #pico_cyw43_arch
        #cyw43_driver
       )

# enable usb output, disable uart output
pico_enable_stdio_usb(neopixel 0)
pico_enable_stdio_uart(neopixel 1)

# create map/bin/hex file etc.
pico_add_extra_outputs(neopixel)

