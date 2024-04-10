# Basic C++ HTTP Web Server

This project is a simple implementation of an HTTP web server in C++. The server is currently in a preliminary stage, supporting only the GET method. Its primary function is to serve static content, and it has been designed with simplicity in mind to demonstrate the foundational concepts of web server operation in C++.

## Features

- **GET Method Support**: The server currently supports the GET request method, allowing clients to request files and resources.
- **Static File Serving**: The server can serve static files. As of now, there's a single test file available (`helloworld.html`) to demonstrate this functionality.
- **Basic Error Handling**: Basic error handling is implemented, with the server capable of returning 404 (Not Found) and 500 (Internal Server Error) status codes.

## Requirements

- C++17 compiler (The server utilizes C++17 features and the standard filesystem library for file operations)
- Linux environment (The server uses POSIX-specific features for networking)