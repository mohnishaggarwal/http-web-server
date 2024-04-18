# Simple C++ Web Server

This is a simple, lightweight web server written in C++, capable of handling HTTP GET requests. The server listens on port 80 and serves static content. It includes  basic error handling and is designed robustly for adding in more complex error handling.

## Features

- **HTTP GET Requests**: Handles incoming HTTP GET requests on port 80.
- **Static Content**: Serves static files from the server.
- **Basic Error Handling**: Returns a 404 Not Found error for non-existent files or a 500 for non GET requests.

## Getting Started

These instructions will get your copy of the project up and running on your local machine for development and testing purposes.

### Prerequisites

To run this project, you will need Docker installed on your machine. Visit [Docker's website](https://www.docker.com/get-started) for installation instructions.

### Installing

A step-by-step series of examples that tell you how to get a development environment running:

1. **Clone the repository**

   ```bash
   git clone https://github.com/mohnishaggarwal/http-web-server.git
   cd http-web-server
    ```
2. **Build the Docker image**

    ```bash
    docker build -t cpp-web-server .
    ```

3. **Run the server**
    ```bash
   docker run -d -p 80:80 cpp-web-server
   ```
   This command will start the server in a detached mode, running in the background.

### Usage

Once the server is running, you can access it via any web browser or tool like curl using your machine's IP address or localhost if you are running the browser or tool on the same machine.

Try this out in your browser by going to http://localhost/helloworld.html
