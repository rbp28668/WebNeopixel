#include "webapp404.hpp"

static const char* body =
"<html>"
"<head></head>"
"<body>"
"<h1>404-This is not the page you're looking for</h1>"
"</body>"
"</html>"
"\r\n";



bool Webapp404::matches(const char* verb, const char* path){
    return true;
}

void Webapp404::process( HttpRequest& request, HttpResponse& response){
    response.setStatus(404,"Not Found");
    response.addHeader("Server", "PicoW");
    response.addHeader("Content-Type", "text/html");
    response.setBody(body);
}
