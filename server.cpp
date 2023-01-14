// See https://www.nongnu.org/lwip/2_1_x/index.html
// pico-examples/picow_tcp_server.c at master · raspberrypi/pico-examples · GitHub
// https://github.com/raspberrypi/pico-examples/blob/master/pico_w/tcp_server/picow_tcp_server.c



/**
 * Copyright (c) 2022 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>
#include <stdlib.h>

#include "server.hpp"

#define TCP_PORT 4242
#define DEBUG_printf printf
#define POLL_TIME_S 5


/////////////////////////////////////////////////////////////
// Client



/// @brief Opens a connection.
/// @param pServer is the server the connection is to.
/// @param client_pcb has the connection details.
void TcpServer::ServerConnection::open(TcpServer* pServer, struct tcp_pcb *client_pcb){
    this->client_pcb = client_pcb;
    this->server = pServer;
    tcp_arg(client_pcb, this);           
    tcp_sent(client_pcb, TcpServer::ServerConnection::sent);
    tcp_recv(client_pcb, TcpServer::ServerConnection::received);
    tcp_poll(client_pcb, TcpServer::ServerConnection::poll, POLL_TIME_S * 2);
    tcp_err(client_pcb, TcpServer::ServerConnection::error);            
}

/// @brief closes the connection.
/// @return ERR_OK or ERR_ABRT if closing errored and the connection was subsequently aborted.
err_t TcpServer::ServerConnection::close(){
    err_t err = ERR_OK;
    if(client_pcb) {
        tcp_arg(client_pcb, NULL);
        tcp_poll(client_pcb, NULL, 0);
        tcp_sent(client_pcb, NULL);
        tcp_recv(client_pcb, NULL);
        tcp_err(client_pcb, NULL);
        err = tcp_close(client_pcb);
        if (err != ERR_OK) {
            DEBUG_printf("close failed %d, calling abort\n", err);
            tcp_abort(client_pcb);
            err = ERR_ABRT;
        }
    }
    markAsClosed();
    return err;
}

// Callback for when data is sent.
// tpcb	The connection pcb for which data has been acknowledged
// len	The amount of bytes acknowledged
// ERR_OK: try to send some data by calling tcp_output Only return ERR_ABRT if you have called tcp_abort from within the callback function!
err_t TcpServer::ServerConnection::sent(void *arg, struct tcp_pcb *tpcb, u16_t len) {
    TcpServer::ServerConnection* connection = static_cast<TcpServer::ServerConnection*>(arg);
    DEBUG_printf("tcp_server_sent %u\n", len);
    return connection->app()->sent(connection, len);
}

// Callback for data received.
// With the raw API, tcp_recv() sets up to receive data via a callback function. 
// Your callback is delivered chains of pbufs as they become available. 
// You have to manage extracting data from the pbuf chain, and don't forget to watch out for multiple
//  pbufs in a single callback: the 'tot_len' field indicates the total length of data in the pbuf chain. 
// You must call tcp_recved() to tell LWIP when you have processed the received data. 
// As with the netconn API, you may receive more or less data than you want, and will have to either wait 
// for further callbacks, or hold onto excess data for later processing.
// tpcb	The connection pcb which received data
// p	The received data (or NULL when the connection has been closed!)
// err	An error code if there has been an error receiving Only return ERR_ABRT if you have called tcp_abort from within the callback function!
err_t TcpServer::ServerConnection::received(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {
    TcpServer::ServerConnection* connection = static_cast<TcpServer::ServerConnection*>(arg);

    // Connection closed?    
    if (!p) {
        connection->app()->closed(connection);
        return connection->close();
    }

    // this method is callback from lwIP, so cyw43_arch_lwip_begin is not required, however you
    // can use this method to cause an assertion in debug mode, if this method is called when
    // cyw43_arch_lwip_begin IS needed
    cyw43_arch_lwip_check();

    // Process data...
    if (p->tot_len > 0) {
        DEBUG_printf("tcp_server_recv %d, err %d\n", p->tot_len, err);

        // Receive the buffer and shovel the data to the app.
        struct pbuf *here = p;
        while(here){
            err = connection->app()->receive(connection, here->payload, here->len);
            if(err != ERR_OK) return err;
            here = here->next;
        }
         tcp_recved(tpcb, p->tot_len);
    }
    pbuf_free(p);

    return ERR_OK;
}

// Callback for TCP poll
// When a connection is idle (i.e., no data is either transmitted or received), lwIP will 
// repeatedly poll the application by calling a specified callback function. 
// This can be used either as a watchdog timer for killing connections that have stayed idle for too long, 
// or as a method of waiting for memory to become available. 
// returns ERR_OK: try to send some data by calling tcp_output.
// Only return ERR_ABRT if you have called tcp_abort from within the callback function!
err_t TcpServer::ServerConnection::poll(void *arg, struct tcp_pcb *tpcb) {
    TcpServer::ServerConnection* connection = static_cast<TcpServer::ServerConnection*>(arg);
    // DEBUG_printf("tcp_server_poll_fn\n");
    return connection->app()->poll(connection);
}

// Callback for TCP error
// Called when the pcb receives a RST or is unexpectedly closed for any other reason.
// err	Error code to indicate why the pcb has been closed 
// ERR_ABRT: aborted through tcp_abort or by a TCP timer 
// ERR_RST: the connection was reset by the remote host
void TcpServer::ServerConnection::error(void *arg, err_t err) {
    TcpServer::ServerConnection* connection = static_cast<TcpServer::ServerConnection*>(arg);
    connection->app()->closed(connection);
    connection->markAsClosed();
}

/// @brief Sends data from the server back to client.
/// @param data is the data to send.
/// @param len is the number of bytes to send
/// @param moreToCome if true signals there's more data to come.
/// @return error status, hopefully ERR_OK.
err_t TcpServer::ServerConnection::send(uint8_t* data, size_t len, bool moreToCome)
{
    DEBUG_printf("Writing %ld bytes to client\n", len);
    // this method is callback from lwIP, so cyw43_arch_lwip_begin is not required, however you
    // can use this method to cause an assertion in debug mode, if this method is called when
    // cyw43_arch_lwip_begin IS needed
    cyw43_arch_lwip_check();
    u8_t apiflags = TCP_WRITE_FLAG_COPY;
    if(moreToCome) apiflags |= TCP_WRITE_FLAG_MORE;
    err_t err = tcp_write(client_pcb, data, len, apiflags);
    if (err != ERR_OK) {
        DEBUG_printf("Failed to write data %d\n", err);
        return err;
    }
    return ERR_OK;
}

/// @brief aborts the connection.
/// @return ERR_ABRT
err_t TcpServer::ServerConnection::abort(){
    tcp_abort(client_pcb);
    markAsClosed();
    return ERR_ABRT;
}
    

/////////////////////////////////////////////////////////////
//  TcpServer

TcpServer::TcpServer(ServerApplication* app)
: app(app)
, server_pcb(0)  
, complete(false)
, errorStatus(ERR_OK)
{}



// Find an unallocated client descriptor (client_pcb is null).  If found
// then its client_pcb is set and a pointer is returned.
TcpServer::ServerConnection* TcpServer::allocateClient(TcpServer* pServer, struct tcp_pcb *client_pcb){
    for(int i=0; i<MAX_CLIENTS;++i){
        if(!clients[i].isOpen()){
            clients[i].open(pServer, client_pcb);
            return clients+i;
        }
    }
    return 0; // no spare 
}


// Callback when client connects
err_t TcpServer::accept(void *arg, struct tcp_pcb *client_pcb, err_t err) {
    TcpServer* server = static_cast<TcpServer*>(arg);

    if (err != ERR_OK || client_pcb == NULL) {
        DEBUG_printf("Failure in accept\n");
        return ERR_VAL;
    }
    DEBUG_printf("Client connected\n");

    TcpServer::ServerConnection* connection = server->allocateClient(server, client_pcb);
    if(connection) {
        server->app->connected(connection);
        return ERR_OK;
    }

    return ERR_MEM;
}



/// @brief Opens a server connection to listen on the given port.
/// @param port is the port number to listen on.
/// @return true if successful, falow otherwise.
bool TcpServer::open(uint16_t port) {
   DEBUG_printf("Starting server at %s on port %u\n", ip4addr_ntoa(netif_ip4_addr(netif_list)), port);

    struct tcp_pcb *pcb = tcp_new_ip_type(IPADDR_TYPE_ANY);
    if (!pcb) {
        DEBUG_printf("failed to create pcb\n");
        return false;
    }

    err_t err = tcp_bind(pcb, NULL, port);
    if (err) {
        DEBUG_printf("failed to bind to port %d\n",port);
        return false;
    }

    server_pcb = tcp_listen_with_backlog(pcb, 1);
    if (!server_pcb) {
        DEBUG_printf("failed to listen\n");
        if (pcb) {
            tcp_close(pcb);
        }
        return false;
    }

    tcp_arg(server_pcb, this);    // pass this object in callbacks
    tcp_accept(server_pcb, TcpServer::accept); // call tcp_server_accept to accept connections

    return true;
}


/// @brief Close the server.
// Note that this closes the whole server, not just an individual client connection.
// Also sets the complete flag to terminate the run loop.
/// @return ERR_OK
err_t TcpServer::close() {
    err_t err = ERR_OK;
    for(int i=0; i<MAX_CLIENTS; ++i){
        if(clients[i].isOpen()){
            clients[i].close();
        }
    }

    if (server_pcb) {
        tcp_arg(server_pcb, NULL);
        tcp_close(server_pcb);
        server_pcb = NULL;
    }

    complete = true;

    return err;
}

/// @brief Runs the main polling loop of the application.
void TcpServer::run(){
     while(!complete) {
        cyw43_arch_poll();
    }
}

