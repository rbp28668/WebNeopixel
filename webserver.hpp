
#ifndef WEBSERVER_HPP
#define WEBSERVER_HPP

#include "server.hpp"
#include "block_malloc.hpp"
#include "block_list.hpp"

// Maximum number of applications that can be registered with this sever
#define MAX_APPS (32)

class Parameter {
    const char* _name;
    const char* _value;

    public:
    Parameter(const char* name, const char* value) : _name(name), _value(value) {}
    void* operator new(size_t size, Block* block) { return block->allocate(size);}
    void operator delete  ( void* ptr ) noexcept {assert(false);} 
    const char* name() {return _name;}
    const char* value() {return _value;}
    void setName(const char* name) { _name = name;}
    void setValue(const char* value) { _value = value;}
    int asInt();
    uint32_t asRgb();
    float asFloat();
};

class Header {
    const char* _name;
    const char* _value;

    public:
    Header(const char* name, const char* value): _name(name), _value(value) {}
    void* operator new(size_t size, Block* block) { return block->allocate(size);}
    void operator delete  ( void* ptr ) noexcept {assert(false);} 
    const char* name() {return _name;}
    const char* value() {return _value;}
    
    const char* toSend(Block* block);
};

class HttpRequest{

    Block* block;   // memory block(s) for this request
    char* _verb;     // GET etc.
    char* _path;     //  /wombles
    char* _protocol; // HTTP/1.1
    BlockList<Parameter> _Parameters;
    BlockList<Header> _headers;
    char* _body;
    bool _complete;

    const char* parseMessage;

    int parseInitialLine(char*& here, char*& there, int length);
    int parseHeaderLine(char*& here, char*& there, int length);
    int parseBody(char*& here, char*& there, int length);

    public:
    HttpRequest(Block* block);
    ~HttpRequest();
 
    bool isComplete() const { return _complete;}

    const char* verb() { return _verb;}
    const char* path() { return _path;}
    const char* protocol() { return _protocol;}
    const char* failureMessage() {return parseMessage;}
    BlockList<Parameter>& Parameters() {return _Parameters;}
    BlockList<Header>& headers() {return _headers;}
  
    void parse(void* data, uint16_t length);
};

class HttpResponse{
    Block* block;
    BlockList<Header> _headers;
    int statusCode;
    const char* statusMsg;
    const char* body;
 
    public:
    HttpResponse(Block* block);

    void setStatus(int code, const char* msg);
    void addHeader(const char* key, const char* body);
    void setBody(const char* payload);

    const char* protocolLine();
    BlockList<Header>& headers() {return _headers;}
    const char* getBody() { return body;}

 };


class HttpTransaction {
    Block* _block;  // memory for this and children
    HttpRequest _request;
    HttpResponse _response;
    size_t bytesToSend; // track outstanding response bytes.
 
    public:
    HttpTransaction(Block* block);
    ~HttpTransaction();
    static void* operator new(size_t size, Block* block);
    static void operator delete(void* mem) {}
    HttpRequest& request() { return _request;}
    HttpResponse& response() { return _response;}
    Block* getBlock() { return _block;}

    void setSendSize(size_t byteCount) {bytesToSend = byteCount;}
    void sent(size_t bytes) { bytesToSend -= bytes;}
    bool sendComplete() { return bytesToSend == 0;}
};

class WebApp {
    public:
    virtual bool matches(const char* verb, const char* path) = 0;
    virtual void process(HttpRequest& request, HttpResponse& response) = 0;
};


class Webserver: public ServerApplication {
    
    WebApp* apps[MAX_APPS];
    uint appCount;

    size_t sendResponse(HttpTransaction* tx, Connection* connection);

    public:
    Webserver();
    
    virtual void connected(Connection* connection);
    virtual void closed(Connection* connection);
    virtual err_t receive(Connection* connection, void* data, uint16_t length);
    virtual err_t poll(Connection* connection);
    virtual void error(Connection* connection, err_t err);
    virtual err_t sent(Connection* connection, u16_t bytesSent);

    bool addAppliction(WebApp* app);
};

#endif