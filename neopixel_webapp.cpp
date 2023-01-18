#include "neopixel_webapp.hpp"
#include "neopixel.hpp"

static const char* form=
"<html>"
"<head></head>"
"<body>"
"<form action=\"/set\" method=\"get\">"
"<label for=\"brt\">White Brightness</label>"
"<input type=\"number\" id=\"brt\" value=\"0\" min=\"0\" max=\"255\" name=\"brt\"><br>"
"<label for=\"rgb\">RGB Colour:</label>"
"<input type=\"color\" id=\"rgb\" name=\"rgb\"><br>"
"<input type=\"submit\" value=\"Submit\">"
"</form>"
"</body>"
"</html>"
"\r\n";


extern NeopixelGrid grid;  


bool NeopixelWebapp::matches(const char* verb, const char* path){
    bool accept = false;
    if(strcmp(verb,"GET") == 0) {
        accept = strncmp(path,"/set",4) == 0;
        if(accept) printf("Neopixel Webapp Accepting path %s\n", path);
    }
    return accept;
}

void NeopixelWebapp::process( HttpRequest& request, HttpResponse& response){

    printf("Neopixel Webapp Processing request for %s\n",request.path());

    BlockListIter<Parameter> iter = request.Parameters().iter();
    uint32_t rgb = 0;
    int w = 0;
    Parameter* p;
    while(p = iter.next()){
        printf("Parameter %s -> %s\n", p->name(), p->value());
        if(strcmp(p->name(), "brt") == 0) w = p->asInt();
        if(strcmp(p->name(), "rgb") == 0) rgb = p->asRgb();
    }

    printf("Set colour %06X, white %d\n", rgb,w);
    grid.setAsync(rgb, w);
 
    response.setStatus(200,"OK");
    response.addHeader("Server", "PicoW");
    response.addHeader("Content-Type", "text/html");
    response.setBody(form);
}
