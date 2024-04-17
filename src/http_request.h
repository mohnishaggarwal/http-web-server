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
    http_request(const std::string &raw_request);

    std::string get_method() const;
    std::string get_url() const;

private:
    std::unordered_map<std::string, std::string> headers;
    std::string method;
    std::string url;
    std::string protocol_version;

    void parse_request(const std::string &raw_request);
    void parse_header_line(const std::string& header_line);
    void parse_request_line(const std::string& request_line);
};

#endif
