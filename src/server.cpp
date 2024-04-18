//
// Created by Aggarwal, Mohnish on 4/17/24.
//

#include "server.h"

server::server() {
    start_server();
}

void server::start_server() {
    port_number desired_port = 80;
    socket_file_descriptor sock_fd;
    try {
        sock_fd = create_socket(desired_port);
    } catch (std::runtime_error &e) {
        std::cerr << e.what() << std::endl;
        return;
    }

    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    while (true) {
        socket_file_descriptor client_fd = accept(sock_fd, (struct sockaddr *) &client_addr, &client_addr_len);
        if (client_fd < 0) {
            perror("Could not accept request from client");
            continue;
        }
        log_client_connection(client_addr);
        std::thread req_thread(handle_request, client_fd);
        req_thread.detach();
    }
}

port_number server::get_port(int sockfd) {
    struct sockaddr_in addr;
    socklen_t length = sizeof(addr);
    if (getsockname(sockfd, (sockaddr *) &addr, &length) == -1) {
        throw std::runtime_error("Error getting port of socket");
    }
    // Use ntohs to convert from network byte order to host byte order.
    return ntohs(addr.sin_port);
}

socket_file_descriptor server::create_socket(port_number port) {
    socket_file_descriptor sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        throw std::runtime_error("Error opening stream socket");
    }

    // Allow the socket address to be reused in case server is restarted
    int yesval = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yesval, sizeof(yesval)) == -1) {
        throw std::runtime_error("Error setting socket option SO_REUSEADDR");
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);

    if (bind(sockfd, (sockaddr *) &addr, sizeof(addr)) == -1) {
        throw std::runtime_error("Error binding socket to an addr");
    }

    port = get_port(sockfd);
    uint8_t queue_length = 10;
    listen(sockfd, queue_length);
    std::cout << "\n@@@ port " << port << std::endl;
    return sockfd;
}

void server::log_client_connection(const struct sockaddr_in &client_addr) {
    std::cout << "Client connected from "
              << inet_ntoa(client_addr.sin_addr)
              << " on port "
              << ntohs(client_addr.sin_port)
              << std::endl;
}

void server::handle_request(socket_file_descriptor connection_fd) {
    raii_connection conn = raii_connection(connection_fd);

    char buffer[4096];
    ssize_t bytes_received = recv(connection_fd, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received < 0) {
        std::cout << "Failed to receive bytes" << std::endl;
        return;
    }
    buffer[bytes_received] = '\0';

    std::string request_str(buffer);
    http_request request(request_str);
    http_response response(request);

    std::string resp_str = response.to_string();
    uint64_t resp_size = resp_str.size();
    const char *resp = resp_str.c_str();
    ssize_t bytes_sent = 0;
    do {
        bytes_sent += send(connection_fd, resp + bytes_sent, resp_size - bytes_sent,
                           MSG_NOSIGNAL);
    } while (bytes_sent < resp_size);
}

server::raii_connection::raii_connection(socket_file_descriptor connectionfd) : connectionfd(connectionfd) {};
server::raii_connection::~raii_connection() { close(connectionfd); }

