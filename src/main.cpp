#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctime>
#include <unordered_map>
#include <stdexcept>

#include "http_request.h"

int QUEUE_LENGTH = 10;

class raii_connection {
public:
    raii_connection(int connectionfd) : connectionfd(connectionfd) {};

    ~raii_connection() { close(connectionfd); }

private:
    int connectionfd;
};

int get_port_number(int sockfd) {
    struct sockaddr_in addr;
    socklen_t length = sizeof(addr);
    if (getsockname(sockfd, (sockaddr *) &addr, &length) == -1) {
        throw std::runtime_error("Error getting port of socket");
    }
    // Use ntohs to convert from network byte order to host byte order.
    return ntohs(addr.sin_port);
}

int create_socket(int port) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
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

    port = get_port_number(sockfd);
    listen(sockfd, QUEUE_LENGTH);
    std::cout << "\n@@@ port " << port << std::endl;
    return sockfd;
}

void log_client_connection(const struct sockaddr_in &client_addr) {
    std::cout << "Client connected from "
              << inet_ntoa(client_addr.sin_addr)
              << " on port "
              << ntohs(client_addr.sin_port)
              << std::endl;
}



void handle_request(int connection_fd) {
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

int main() {
    int desired_port = 80;
    int sock_fd;
    try {
        sock_fd = create_socket(desired_port);
    } catch (std::runtime_error &e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    while (true) {
        int client_fd = accept(sock_fd, (struct sockaddr *) &client_addr, &client_addr_len);
        if (client_fd < 0) {
            perror("Could not accept request from client");
            continue;
        }
        log_client_connection(client_addr);
        handle_request(client_fd);
    }
}
