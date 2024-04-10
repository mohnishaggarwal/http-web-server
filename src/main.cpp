#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sstream>
#include <fstream>
#include <ctime>
#include <unordered_map>
#include <stdexcept>

using status_code = std::uint32_t;
namespace fs = std::filesystem;

int QUEUE_LENGTH = 10;

class raii_connection {
public:
    raii_connection(int connectionfd) : connectionfd(connectionfd) {};

    ~raii_connection() { close(connectionfd); }

private:
    int connectionfd;
};

class http_request {
public:
    http_request(const std::string &raw_request) {
        parse_request(raw_request);
    }

    const std::unordered_map<std::string, std::string>& get_headers() const {
        return headers;
    }
    std::string get_method() const {
        return method;
    }
    std::string get_url() const {
        return url;
    }
    std::string get_http_version() const {
        return protocol_version;
    }

private:
    std::unordered_map<std::string, std::string> headers;
    std::string method;
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
        line_stream >> method;
        line_stream >> url;
        line_stream >> protocol_version;
    }
};

class http_response {
public:
    http_response(const http_request &request) {
        try {
            init(request);
        } catch (std::runtime_error &e) {
            build_response(500, "Unknown server error");
        }
    }

    std::string to_string() const {
        std::ostringstream resp_stream;
        resp_stream << response_status_line->to_string() << "\r\n";
        for (const auto& header : headers) {
            resp_stream << header.first << ": " << header.second << "\r\n";
        }
        resp_stream << "\r\n" << body;
        return  resp_stream.str();
    }

private:
    class status_line {
    public:
        status_line(const std::string &protocol_version, status_code code, const std::string &status_message)
        : protocol_version(protocol_version), code(code), status_message(status_message) {};

        std::string to_string() const {
            std::ostringstream resp_stream;
            resp_stream << get_protocol_version() << " "
                        << get_status_code() << " "
                        << get_status_message();
            return resp_stream.str();
        }

        std::string get_protocol_version() const {
            return protocol_version;
        }
        status_code get_status_code() const {
            return code;
        }
        std::string get_status_message() const {
            return status_message;
        }
    private:
        std::string protocol_version;
        status_code code;
        std::string status_message;
    };

    status_line *response_status_line;
    std::unordered_map<std::string, std::string> headers;
    std::string body;

    void init(const http_request &request) {
        if (request.get_method() == "GET") {
            std::string file_path = "files" + request.get_url();
            std::string data;
            try {
                data = read_file(file_path);
            } catch (std::runtime_error &e) {
                build_response(404, request.get_url() + " Not Found");
                return;
            }
            build_response(200, data);
            return;
        }
        build_response(400, "Request " + request.get_method() + " not supported");
    }

   std::string read_file(const std::string& path) {
        std::ostringstream buffer;
        std::ifstream input_file(path);

        if (!input_file.is_open()) {
            std::cerr << "Could not open the file - '" << path << "'" << std::endl;
            throw std::runtime_error("Could not open the file - '" + path + "'");
        }

        buffer << input_file.rdbuf();
        return buffer.str();
    }

    std::string get_status_message(status_code code) {
        // TODO - implement more messages dependent on status code
        if (code == 200) {
            return "OK";
        } else if (code == 400) {
            return "Bad Request";
        } else if (code == 404) {
            return "Not Found";
        } else if (code == 500) {
            return "Internal Server Error";
        }
        throw std::runtime_error("Status code not implemented");
    }

    void build_response(status_code code, const std::string &data) {
        response_status_line = new status_line("HTTP/1.1", code, get_status_message(code));
        body = data;
        build_header_lines();
    }

    // This function assumes caller has set the body beforehand and the function body is a null
    void build_header_lines() {
        // TODO - include a last modified header line.
        headers["Connection"] = "close";
        headers["Content-Type"] = "text/html";
        headers["Server"] = "TheNewApache/3.14"; // Arbitrary server name
        headers["Content-Length"] = std::to_string(body.length());

        // Store date as RFC 1123 format
        char date_buffer[40];
        time_t now = time(nullptr);
        struct tm tm = *gmtime(&now);
        strftime(date_buffer, sizeof(date_buffer), "%a, %d %b %Y %H:%M:%S GMT", &tm);
        headers["Date"] = std::string(date_buffer);
    }
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
