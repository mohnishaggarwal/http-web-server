//
// Created by Aggarwal, Mohnish on 4/17/24.
//

#include "http_request.h"

http_request::http_request(const std::string &raw_request) {
    parse_request(raw_request);
}

std::string http_request::get_method() const {
    return method;
}

std::string http_request::get_url() const {
    return url;
}

void http_request::parse_request(const std::string &raw_request) {
    std::istringstream request_stream(raw_request);
    std::string request_line;
    std::getline(request_stream, request_line, '\r');
    parse_request_line(request_line);

    std::string header_line;
    while (std::getline(request_stream, header_line) && header_line != "\r") {
        parse_header_line(header_line);
    }
}

void http_request::parse_header_line(const std::string& header_line) {
    auto colon_pos = header_line.find(':');
    if (colon_pos != std::string::npos) {
        std::string header_name = header_line.substr(0, colon_pos);
        std::string header_value = header_line.substr(colon_pos + 2, header_line.length() - colon_pos - 3);
        headers[header_name] = header_value;
    }
}

void http_request::parse_request_line(const std::string& request_line) {
    std::istringstream line_stream(request_line);
    std::string method_str;
    line_stream >> method;
    line_stream >> url;
    line_stream >> protocol_version;
}