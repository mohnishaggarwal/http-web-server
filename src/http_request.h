//
// Created by Aggarwal, Mohnish on 4/17/24.
//

#ifndef WEBSERVER_HTTP_REQUEST_H
#define WEBSERVER_HTTP_REQUEST_H

#include <string>
#include <unordered_map>
#include <sstream>

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

#endif
