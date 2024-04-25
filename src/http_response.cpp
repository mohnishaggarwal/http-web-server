//
// Created by Aggarwal, Mohnish on 4/17/24.
//

#include "http_response.h"

http_response::http_response(const http_request &request) {
    try {
        init(request);
    } catch (std::runtime_error &e) {
        build_response(500, "Unknown server error");
    }
}


std::string http_response::to_string() const {
    std::ostringstream resp_stream;
    resp_stream << response_status_line.to_string() << "\r\n";
    for (const auto& header : headers) {
        resp_stream << header.first << ": " << header.second << "\r\n";
    }
    resp_stream << "\r\n" << body;
    return  resp_stream.str();
}

void http_response::init(const http_request &request) {
    if (request.get_method() == "GET") {
        std::string file_path = "public/content" + request.get_url();
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

std::string http_response::read_file(const std::string &path) {
    std::ostringstream buffer;
    std::ifstream input_file(path);

    if (!input_file.is_open()) {
        std::cerr << "Could not open the file - '" << path << "'" << std::endl;
        throw std::runtime_error("Could not open the file - '" + path + "'");
    }

    buffer << input_file.rdbuf();
    return buffer.str();
}

std::string http_response::get_status_message(status_code code) {
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

void http_response::build_response(status_code code, const std::string &data) {
    response_status_line = status_line("HTTP/1.0", code, get_status_message(code));
    body = data;
    build_header_lines();
}

void http_response::build_header_lines() {
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

http_response::status_line::status_line(const std::string &protocol_version, status_code code,
                                        const std::string &status_message): protocol_version(protocol_version), code(code), status_message(status_message) {};

http_response::status_line::status_line() : protocol_version(""), code(0), status_message("") {};

std::string http_response::status_line::to_string() const {
    std::ostringstream resp_stream;
    resp_stream << get_protocol_version() << " "
                << get_status_code() << " "
                << get_status_message();
    return resp_stream.str();
}

std::string http_response::status_line::get_protocol_version() const {
    return protocol_version;
}

std::string http_response::status_line::get_status_message() const {
    return status_message;
}

status_code http_response::status_line::get_status_code() const {
    return code;
}
