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

#endif
