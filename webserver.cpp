
#include "webserver.hpp"
#include "webapp404.hpp"


static BlockPool blockPool;  // big lump of static memory to allocate from.

static Webapp404 webapp404;  // default to generate 404 not found.


////////////////////////////////////////////////////////////////////
// Parameter

/// @brief Convenience method to return value as an int.
/// @return value as an int.
int Parameter::asInt(){
    return atoi(_value);
}

/// @brief Convenience method to get a RGB colour.
/// @return RGB colour as uint32_t.
uint32_t Parameter::asRgb(){
    //long int strtol (const char* str, char** endptr, int base);
    const char* pos = _value;
    if(*pos == '#') ++pos;
    return strtol(pos, 0, 16);
}

/// @brief Convenience method to get value as a float.
/// @return value as a float.
float Parameter::asFloat(){
    return atof(_value);
}

////////////////////////////////////////////////////////////////////
// Header

// Build up a string ready for transmission.
const char* Header::toSend(Block* block){
    int size = strlen(_name) + strlen(_value) + 5; // +5 for: CR LF : <sp> 0
    char* buffer = (char*)block->allocate(size);
    char* there = buffer;
    const char* here = _name;
    while(*here){
        *there++ = *here++;
    }
    *there++ = ':';
    *there++ = ' ';
    here = _value;
    while(*here){
        *there++ = *here++;
    }
    *there++ = '\r';
    *there++ = '\n';
    *there = 0;
    printf("Header toSend:%s\n",buffer);
    return buffer;
}

////////////////////////////////////////////////////////////////////

// Convert 2 hex digits into a character for URL decode
static char decode(char msHex, char lsHex){
    const char* digits = "0123456789ABCDEF";
    int val='?';
    if(msHex >= 'a' && msHex <= 'f') val = (msHex - 'a' + 10) * 16;
    else if(msHex >= 'A' && msHex <= 'F') val = (msHex - 'A' + 10) * 16;
    else if(msHex >= '0' && msHex <= '9') val = (msHex - '0') * 16;
    
    if(lsHex >= 'a' && lsHex <= 'f') val += (lsHex - 'a' + 10);
    else if(lsHex >= 'A' && lsHex <= 'F') val += (lsHex - 'A' + 10);
    else if(lsHex >= '0' && lsHex <= '9') val += (lsHex - '0');

    return (char)val;
}

HttpRequest::HttpRequest(Block* block)
: block(block)
, _verb(0) 
, _path(0)
, _protocol(0)
, _body(0)
, parseMessage(0)
{

}    

HttpRequest::~HttpRequest(){
}

 
int HttpRequest::parseInitialLine(char*& here, char*& there, int length) {
 
   // First, the HTTP verb - GET, POST etc.    
    this->_verb = there;
    while(length > 0 && !isspace(*here)){
        *there++ = *here++;
        --length;
    }
    *there++ = 0;

    // Skip spaces
    while(length > 0 && isspace(*here)){
        ++here; --length;
    }

    // Get path expression
    this->_path = there;
    char* query = 0;
    while(!isspace(*here) && length){
        if(*here == '%' && length > 2) { // URL encoded
            ++here; // skip %
            char msHex = *here++;
            char lsHex = *here++;
            length -= 3;
            *there++ = decode(msHex, lsHex);
        } else if (*here == '?'){ // start of query string.
            ++here;         // skip '?'
            --length;
            query = here;
            break;          // go into next loop to parse query string
        } else { // Just normal char in 
            *there++ = *here++;
            --length;
        }
    }
    *there++ = 0;  // terminate path;
 
    if(query){
        char* key = there;
        char* value = 0;
        while(!isspace(*here) && length){
            if(*here == '%' && length > 2) { // URL encoded
                ++here; // skip %
                char msHex = *here++;
                char lsHex = *here++;
                length -= 3;
                *there++ = decode(msHex, lsHex);
            } else if (*here == '=' && length){  // start of value
                ++here;
                *there++ = 0;
                --length;
                value = there;
            } else if (*here == '&' && length){  // start of another param.
                ++here; // skip &
                *there++ = 0;
                --length;
                Parameter* p = new(block) Parameter(key, value);
                this->_Parameters.add(block, p);
                key = there;    // start of new key
                value = 0;
            } else {
                *there++ = *here++;
                --length;
            }
        }
        *there++ = 0; // terminate last value
        // Process last key/value pair
        Parameter* p = new(block) Parameter(key, value);
        this->_Parameters.add(block, p);
    }

    // skip spaces
    if(length > 0 && isspace(*here)){
        ++here; --length;
    }

     // here should now point to HTTP/1.1
    // Can optionally validate, get version etc.
    // Copy until CRLF skipping any spaces.
    this->_protocol = there;   
    while(length){
        if(*here == '\r') {
            ++here; --length;
        } else if(*here == '\n') {
            ++here; --length;
            break;
        } else if(isspace(*here)){
            ++here; --length;
        } else {
            *there++ = *here++;
            --length;
        }
    }
    *there++ = 0;
    // Ok, so the first line should be parsed!

    printf("verb: %s\n", _verb);
    printf("path: %s\n", _path);
    printf("protocol: %s\n", _protocol);

    return length;
}


/// @brief Parses a single header line of an HTTP request.
/// e.g.:
/// Accept-Encoding: gzip, deflate
/// Accept-Language: en-GB,en-US;q=0.9,en;q=0.8
/// @param here 
/// @param there 
/// @param length 
/// @return 
int HttpRequest::parseHeaderLine(char*& here, char*& there, int length){

    if(length == 0) return 0;
    char*key = there;
    char*val = 0;
    while(length){
        if(*here == ':'){
            ++here; --length;    // skip :
             break;
        } else {
            *there++ = *here++;
            --length;
        }
    }
    *there++ = 0;          // terminate key
    
    // Skip space(s)
    while(length && isspace(*here)) {
        ++here; --length;
    }

    // Copy value until CRLF
    if(length){
        val = there;
        while(length && *here!='\r' && *here!='\n'){
            *there++ = *here++;
            --length;
        }
        *there++ = 0;
    }

    if(key && val) {
        Header* header = new(block) Header(key, val);
        this->_headers.add(block, header);
    }
    // Skip over trailing CRLF
    while(length && (*here == '\r' || *here == '\n')){
        ++here; --length;
    }

    return length;
}

/// @brief Parses the body of the HTTP request.
/// @param here 
/// @param there 
/// @param length 
/// @return 
int HttpRequest::parseBody(char*& here, char*& there, int length){
    if(length){
        _body = there;
        while(length) {
            *there++ = *here++;
            --length;
        }
        *there = 0;
    }
    return length;
}

// GET /set?brt=43&rgb=%23ff38d4 HTTP/1.1
// GET /wonky%20donkey HTTP/1.1
void HttpRequest::parse(void* data, uint16_t length){

    // Total size of strings from request (including nulls)
    // will always be less or equal to the size of the request.
    // Each string has a delimiter in the request and some may
    // shrink due to URL encoding.
    char* there = (char*)block->allocate(length);
    if(there == 0){
        parseMessage = "HTTP/1.1 500 Internal Server Error\r\n";
        return;
    }

    // Now copy data from here to there, adding delimiters as
    // necessary and noting where the fields are.
    char* here = (char*)data;

    length = parseInitialLine(here, there, length);

    // Look for headers - terminate on CRLF.
    while(length > 0 && *here != '\r' && *here != '\n'){
        length = parseHeaderLine(here, there, length);
    }

    // Skip over any CRLF.
    while((length > 0) && (*here == '\r' || *here =='\n')){
        ++here; --length;
    }

    length = parseBody(here, there, length);


}

///////////////////////////////////////////////////////////////////////////////////////////////////
// HttpResponse

HttpResponse::HttpResponse(Block* block)
: block(block)
, statusCode(200)
, statusMsg("OK")
, body(0)
{}

void HttpResponse::setStatus(int code, const char* msg){
    statusCode = code;
    statusMsg = msg;
}

void HttpResponse::addHeader(const char* key, const char* value){
    Header* h = new(block) Header(key, value);
    _headers.add(block, h);
}
    
void HttpResponse::setBody(const char* payload){
    this->body = payload;
}

const char* HttpResponse::protocolLine(){
    const char* p = "HTTP/1.1 ";
    size_t len = strlen(p) + 3 + 1 + strlen(statusMsg) + 1;
    char* buff = (char*) block->allocate(len);

    if(buff) {
        char* there = buff;
        const char* here = p;
        while(*here){
            *there++ = *here++;
        }
        if(statusCode > 999) statusCode = 999; // 3 digits 
        itoa(statusCode, there, 10 );
        while(*there) ++there; //Find trailing zero
        *there++ = ' ';
        here = statusMsg;
        while(*here){
            *there++ = *here++;
        }
        *there++ = '\r';
        *there++ = '\n';
        *there++ = 0;
    }
   
    printf("Protocol line:%s\n", buff);
    return buff;
}
 

///////////////////////////////////////////////////////////////////////////////////////////////////
// HttpTransaction


HttpTransaction::HttpTransaction(Block* block)
: _request(block)
, _response(block){

}


HttpTransaction::~HttpTransaction(){

}

void* HttpTransaction::operator new(size_t size, Block* block){
    return block->allocate(size);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
Webserver::Webserver()
:appCount(0)
{
    for(int i=0; i<MAX_APPS; ++i){
        apps[i] = 0;
    }
}


void Webserver::connected(Connection* connection){
    printf("connected\n");

}

void Webserver::closed(Connection* connection){
   printf("closed\n");

}

// GET /hello.txt HTTP/1.1
// 
// HTTP/1.1 200 OK
// Date: Mon, 27 Jul 2009 12:28:53 GMT
// Server: Apache
// Content-Length: 51
//
// Some content here
err_t Webserver::receive(Connection* connection, void* data, uint16_t length){
    printf("Received %d ======================\n",length);
    char* text = (char*)data;
    for(int i=0; i<length; ++i){
        putchar(text[i]);
    }
    printf("==================================\n");
    Block* block = blockPool.allocate();
    if(block == 0){
        printf("No memory to handle request");
        const char* header = "HTTP/1.1 500 No memory\r\n";
        connection->send((uint8_t*)header, strlen(header));
        return ERR_OK;
    }

    printf("Create transaction\n");
    HttpTransaction* tx = new(block) HttpTransaction(block);
    printf("Parsing request\n");
    tx->request().parse(data, length);
    
    const char* fail = tx->request().failureMessage();
    if(fail){
        connection->send((uint8_t*)fail, strlen(fail));
        return ERR_OK;
    }

    // TODO sort out logic - if existing http request then add to it
    // Connection needs to store a pointer to tx to manage this - HttpTransaction
    // will store the state of the request (may arrive in multiple parts) and the
    // response (may need to be built up and sent piecemeal)

    printf("Looking for webapp for %s : %s\n",tx->request().verb(), tx->request().path());
    WebApp* app = &webapp404;
    for(int i=0; i<appCount; ++i){
        printf("App %d matches?\n",i);
        if(apps[i]->matches(tx->request().verb(), tx->request().path())) {
            app = apps[i];
            printf("Found webapp %d\n",i);
            break;
        }
    }
    app->process(tx->request(), tx->response());
 
    const char* value = tx->response().protocolLine();
    connection->send((uint8_t*)value, strlen(value));

    BlockListIter<Header> iter = tx->response().headers().iter();
    Header* h;
    while( (h = iter.next()) != 0){
        value = h->toSend(block);
        connection->send((uint8_t*)value, strlen(value));
    }
    
    // Blank line after any headers.
    value = "\r\n";
    connection->send((uint8_t*)value, strlen(value));

    // Followed by (optional) body.
    value = tx->response().getBody();
    if(value){
        connection->send((uint8_t*)value, strlen(value));
    }

    // TODO should really only do this when all the data is sent.
    block->free(); // Assumes whole process in one hit.

    return ERR_OK;

}

err_t Webserver::poll(Connection* connection){
    return ERR_OK;

}

void Webserver::error(Connection* connection, err_t err){
    printf("Error %d\n", err);

}

err_t Webserver::sent(Connection* connection, u16_t bytesSent){
    printf("sent %d\n", bytesSent);
    connection->close();
    return ERR_OK;
}

bool Webserver::addAppliction(WebApp* app){
    assert(app);
    if(appCount == MAX_APPS) return false;
    apps[appCount++] = app;
    return true;
}