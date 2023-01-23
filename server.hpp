#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#include "lwip/pbuf.h"
#include "lwip/tcp.h"

#define MAX_CLIENTS 8

// Interface to describe a TCP connection from the application's point of view.
class Connection{
    void* _appState;
    public:
    void setAppState(void* state) {_appState = state;}
    void* getAppState() const { return _appState;}
    
    virtual err_t send(uint8_t* data, size_t len, bool moreToCome=false) = 0;
    virtual err_t close() = 0;
    virtual bool isOpen() = 0;
    virtual err_t abort() = 0;
};

// Interface to define a server application i.e. one that handles new connections and
// manages a request/response protocol.
class ServerApplication {
    public:
    virtual void connected(Connection* connection) = 0;
    virtual void closed(Connection* connection) = 0;
    virtual err_t receive(Connection* connection, void* data, uint16_t length) = 0;
    virtual err_t poll(Connection* connection) = 0;
    virtual void error(Connection* connection, err_t err) = 0;
    virtual err_t sent(Connection* connection, u16_t bytesSent) = 0;
};

class TcpServer {

    // Internal class to track connections to the server.
    class ServerConnection: 
    public Connection {
        TcpServer* server;
        struct tcp_pcb *client_pcb;
   
        // Callback functions for managing client connection
        static err_t poll(void *arg, struct tcp_pcb *tpcb);
        static void error(void *arg, err_t err);
        static err_t sent(void *arg, struct tcp_pcb *tpcb, u16_t len);
        static err_t received(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);

        public:

        ServerConnection() 
            : server(0),
            client_pcb(0)
            {}

        //TcpServer* getServer() { return server;}
        void markAsClosed() {client_pcb = 0;}
        void open(TcpServer* server, struct tcp_pcb *client_pcb);

        inline ServerApplication* app() {return server->getApp();}

        virtual err_t close();
        virtual bool isOpen() { return client_pcb != 0;}
        virtual err_t send(uint8_t* data, size_t len, bool moreToCome=false);
        virtual err_t abort();
     };   

    ServerApplication* app;         // Server application using this TCP server.
    struct tcp_pcb *server_pcb;     // For listening for incoming connections.
    bool complete;                  // Set true to terminate loop.
    err_t errorStatus;
    ServerConnection clients[MAX_CLIENTS];  // Max number of connections

    ServerConnection* allocateClient(TcpServer* pServer, struct tcp_pcb *client_pcb);

    // Callback functions for LWIP
    static err_t accept(void *arg, struct tcp_pcb *client_pcb, err_t err);

    public:

    TcpServer(ServerApplication* app);
    ServerApplication* getApp() {return app;}
    bool open(uint16_t port);
    err_t close();
    void run();
};

#endif