//
// Created by Aggarwal, Mohnish on 4/17/24.
//

#ifndef WEBSERVER_SERVER_H
#define WEBSERVER_SERVER_H

#include <iostream>
#include <stdexcept>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "usings.h"
#include "http_request.h"
#include "http_response.h"

class server {
public:
    server();

private:
    class raii_connection {
    public:
        raii_connection(socket_file_descriptor connectionfd);
        ~raii_connection();
    private:
        socket_file_descriptor connectionfd;
    };

    void start_server();
    port_number get_port(int sockfd);
    socket_file_descriptor create_socket(port_number port);
    void log_client_connection(const struct sockaddr_in &client_addr);
    void handle_request(socket_file_descriptor connection_fd);
};

#endif
