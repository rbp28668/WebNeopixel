#ifndef WEBAPP_404_HPP
#define WEBAPP_404_HPP

#include "webserver.hpp"

class Webapp404: public WebApp{
   public:
    virtual bool matches(const char* verb, const char* path);
    virtual void process(HttpRequest& request, HttpResponse& response);

};
#endif