//
// Created by Aggarwal, Mohnish on 4/17/24.
//

#ifndef WEBSERVER_HTTP_RESPONSE_H
#define WEBSERVER_HTTP_RESPONSE_H

#include <iostream>
#include <fstream>

#include "http_request.h"
#include "usings.h"

class http_response {
public:
    http_response(const http_request &request);
    std::string to_string() const;

private:
    class status_line {
    public:
        status_line(const std::string &protocol_version, status_code code, const std::string &status_message);
        status_line();
        std::string to_string() const;
        std::string get_protocol_version() const;
        status_code get_status_code() const;
        std::string get_status_message() const;

    private:
        std::string protocol_version;
        status_code code;
        std::string status_message;
    };

    status_line response_status_line;
    std::unordered_map<std::string, std::string> headers;
    std::string body;

    void init(const http_request &request);
    std::string read_file(const std::string& path);
    std::string get_status_message(status_code code);
    void build_response(status_code code, const std::string &data);
    void build_header_lines();
};

#endif
