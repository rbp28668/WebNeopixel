#ifndef INDEX_PAGE_HPP
#define INDEX_PAGE_HPP

#include "webserver.hpp"

class IndexPage: public WebApp{
   public:
    virtual bool matches(const char* verb, const char* path);
    virtual void process(HttpRequest& request, HttpResponse& response);

};
#endif