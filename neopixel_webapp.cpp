#include "neopixel_webapp.hpp"
#include "neopixel.hpp"

static const char* form=
"<html>"
"<head></head>"
"<body>"
"<form action=\"/set\" method=\"get\">"
"<label for=\"white\">White Brightness</label>"
"<input type=\"number\" id=\"white\" value=\"0\" min=\"0\" max=\"255\" name=\"white\"><br>"
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
        accept = 
            strncmp(path,"/set",4) == 0 ||
            strncmp(path,"/colour",7) == 0
            ;
        if(accept) printf("Neopixel Webapp Accepting path %s\n", path);
    }
    return accept;
}

void NeopixelWebapp::process( HttpRequest& request, HttpResponse& response){

    printf("Neopixel Webapp Processing request for %s\n",request.path());

    uint32_t rgb = 0;
    int w = 0;
    float increment = 0.1f;
    float value = 0.0f;

    BlockListIter<Parameter> iter = request.Parameters().iter();
    Parameter* p;
    while(p = iter.next()){
        printf("Parameter %s -> %s\n", p->name(), p->value());
        if(strcmp(p->name(), "white") == 0) w = p->asInt();
        if(strcmp(p->name(), "rgb") == 0) rgb = p->asRgb();
        if(strcmp(p->name(), "inc") == 0) increment = p->asFloat();
        if(strcmp(p->name(), "value") == 0) value = p->asFloat();
    }

    if(strncmp(request.path(),"/set",4) == 0) {
        printf("Set colour %06X, white %d\n", rgb,w);
        grid.setAsync(rgb, w);
    
    } else if (strncmp(request.path(),"/colour",7) == 0){
        printf("Set colour change %06X, white %d\n", rgb,w);
        grid.colourChangeAsync(value, increment, w);
    
    } else {
        // TODO 500
    }
  

    response.setStatus(200,"OK");
    response.addHeader("Server", "PicoW");
    response.addHeader("Content-Type", "text/html");
    response.setBody(form);
}
