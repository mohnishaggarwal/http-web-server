#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sstream>
#include <unordered_map>
#include <stdexcept>

int QUEUE_LENGTH = 10;

class raii_connection {
public:
    raii_connection(int connectionfd) : connectionfd(connectionfd) {};

    ~raii_connection() { close(connectionfd); }

private:
    int connectionfd;
};

enum class http_method {
    GET // only insert GET for now, integrate support for other methods later
};

class http_request {
public:
    http_request(const std::string &raw_request) {
        parse_request(raw_request);
    }

    const std::unordered_map<std::string, std::string>& get_headers() const {
        return headers;
    }
    const http_method get_method() const {
        return method;
    }
    const std::string& get_url() const {
        return url;
    }
    const std::string& get_http_version() const {
        return protocol_version;
    }

private:
    std::unordered_map<std::string, std::string> headers;
    http_method method;
    std::string url;
    std::string protocol_version;

    void parse_request(const std::string &raw_request) {
        std::istringstream request_stream(raw_request);
        std::string request_line;
        std::getline(request_stream, request_line, '\r');
        parse_request_line(request_line);

        std::string header_line;
        while (std::getline(request_stream, header_line) && header_line != "\r") {
            parse_header_line(header_line);
        }
    }

    void parse_header_line(const std::string& header_line) {
        auto colon_pos = header_line.find(':');
        if (colon_pos != std::string::npos) {
            std::string header_name = header_line.substr(0, colon_pos);
            std::string header_value = header_line.substr(colon_pos + 2, header_line.length() - colon_pos - 3);
            headers[header_name] = header_value;
        }
    }

    void parse_request_line(const std::string& request_line) {
        std::istringstream line_stream(request_line);
        std::string method_str;
        line_stream >> method_str;
        method = parse_method(method_str);
        line_stream >> url;
        line_stream >> protocol_version;
    }

    http_method parse_method(const std::string& method_str) {
        if (method_str == "GET") return http_method::GET;
        throw std::runtime_error("Unsupported method type");
    }
};

class http_response {
public:

private:
    class status_line {
    public:
        status_line(const std::string protocol_version, const std::string status_code, const std::string status_message)
        : protocol_version(protocol_version), status_code(status_code), status_message(status_message) {};

        const std::string& get_protocol_version() const {
            return protocol_version;
        }
        const std::string& get_status_code() const {
            return status_code;
        }
        const std::string& get_status_message() const {
            return status_message;
        }
    private:
        std::string protocol_version;
        std::string status_code;
        std::string status_message;
    };

    status_line response_status_line;
    std::unordered_map<std::string, std::string> headers;
    char* body;
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
    std::cout << request_str << std::endl;
}

int main() {
    int desired_port = 80;
    int sock_fd;
    try {
        sock_fd = create_socket(desired_port);
    } catch (std::runtime_error &e) {
        std::cout << e.what() << std::endl;
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
