#include "neopixel_webapp.hpp"
#include "neopixel.hpp"

extern NeopixelGrid grid;  

bool NeopixelWebapp::matches(const char* verb, const char* path){
    bool accept = false;
    if(strcmp(verb,"GET") == 0) {
        accept = 
            strncmp(path,"/set",4) == 0 ||
            strncmp(path,"/cycle",6) == 0 ||
            strncmp(path,"/colour",7) == 0 ||
            strncmp(path,"/ripples",8) == 0 ||
            strncmp(path,"/spokes",7) == 0 ||
            strncmp(path,"/horizontal",11) == 0 ||
            strncmp(path,"/vertical",9) == 0 ||
            strncmp(path,"/sparkle",8) == 0 ||
            strncmp(path,"/show",5) == 0 ||
            false;
            ;
        if(accept) printf("Neopixel Webapp Accepting path %s\n", path);
    }
    return accept;
}

void NeopixelWebapp::process( HttpRequest& request, HttpResponse& response){

    printf("Neopixel Webapp Processing request for %s\n",request.path());
    unsigned int rate = 1;
    uint32_t rgb = 0;
    int w = 0;
    float increment = 0.1f;
    float value = 0.0f;
    float hue = 0.0f;
    float hue2 = -1.0f;
    float count = 1.0f;

    BlockListIter<Parameter> iter = request.Parameters().iter();
    Parameter* p;
    while(p = iter.next()){
        printf("Parameter %s -> %s\n", p->name(), p->value());
        if(strcmp(p->name(), "rate") == 0) rate = (unsigned) p->asInt();
        if(strcmp(p->name(), "white") == 0) w = p->asInt();
        if(strcmp(p->name(), "rgb") == 0) rgb = p->asRgb();
        if(strcmp(p->name(), "inc") == 0) increment = p->asFloat();
        if(strcmp(p->name(), "value") == 0) value = p->asFloat();
        if(strcmp(p->name(), "hue") == 0) hue = p->asFloat();
        if(strcmp(p->name(), "hue2") == 0) hue2 = p->asFloat();
        if(strcmp(p->name(), "count") == 0) count = p->asFloat();

    }

    if(strncmp(request.path(),"/set",4) == 0) {
         grid.setAsync(rgb, w);
    } else if(strncmp(request.path(),"/cycle",6) == 0) {
        grid.rateAsync(rate);
    } else if (strncmp(request.path(),"/colour",7) == 0){
        grid.colourChangeAsync(value, increment, w);
    } else if (strncmp(request.path(),"/ripples",8) == 0) {
        grid.rippleAsync(hue, hue2, value, (int) increment, count, w);
    } else if (strncmp(request.path(),"/spokes",7) == 0) {
        grid.spokesAsync(hue, hue2, value, increment, count, w);
    } else if (strncmp(request.path(),"/horizontal",11) == 0) {
        grid.horizontalAsync(hue, hue2, value, increment, count, w);
    } else if (strncmp(request.path(),"/vertical",9) == 0) {
        grid.verticalAsync(hue, hue2, value, increment, count, w);
    } else if (strncmp(request.path(),"/sparkle",8) == 0) {
        grid.sparkleAsync();
    } else if (strncmp(request.path(),"/show",5) == 0)  {
        printf("Radius\n");
        int idx = 0;
        for(int iy=0; iy<GRID_HEIGHT; ++iy){
            for(int ix=0; ix<GRID_WIDTH;++ix){
                float r = grid.coordinate(idx).r;
                printf("%f\t", r);
                ++idx;
            }
            printf("\n");
        }
        printf("Theta\n");
        idx = 0;
        for(int iy=0; iy<GRID_HEIGHT; ++iy){
            for(int ix=0; ix<GRID_WIDTH;++ix){
                float t = grid.coordinate(idx).theta;
                printf("%f\t", t);
                ++idx;
            }
            printf("\n");
        }
 
    } else {

    }
  

    response.setStatus(200,"OK");
    response.addHeader("Server", "PicoW");
    response.addHeader("Access-Control-Allow-Origin","*");
    
}
