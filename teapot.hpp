#ifndef TEAPOT_HPP
#define TEAPOT_HPP

#include "webserver.hpp"

class Teapot: public WebApp{
   public:
    virtual bool matches(const char* verb, const char* path);
    virtual void process(HttpRequest& request, HttpResponse& response);

};
#endif