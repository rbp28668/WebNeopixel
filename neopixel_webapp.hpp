#ifndef NEOPIXEL_WEBAPP_HPP
#define NEOPIXEL_WEBAPP_HPP

#include "webserver.hpp"

class NeopixelWebapp: public WebApp{
   public:
    virtual bool matches(const char* verb, const char* path);
    virtual void process(HttpRequest& request, HttpResponse& response);

};
#endif