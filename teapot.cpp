#include <string.h>
#include "teapot.hpp"

static const char*teapot = 
"<html><head></head><body>"
"<div style=\"font-family: monospace; white-space: pre;\">"
"            _<br>"
"         _,(_)._<br>"
"    ___,(_______).<br>"
"  ,'__.           \\    /\\_<br>"
" /,' /             \\  /  /<br>"
"| | |              |,'  /<br>"
" \\`.|                  /<br>"
"  `. :           :    /<br>"
"    `.            :.,'<br>"
"      `-.________,-'<br>"
"</div>"
"</body></html>\r\n"
;



bool Teapot::matches(const char* verb, const char* path){
    bool accept = (strcmp(verb,"GET") == 0) && (strncmp(path, "/coffee",7) == 0);
    if(accept) printf("TEAPOT!\n"); else  printf("Not a teapot\n");
    return accept;
}

void Teapot::process( HttpRequest& request, HttpResponse& response){
    response.setStatus(418,"I'm a teapot");
    response.addHeader("Server", "PicoW");
    response.addHeader("Content-Type", "text/html");
    response.setBody(teapot);
}
