# Webserv

Webserv is a C++98 project that allows you to create your own HTTP server inspired by NGINX. This project aims to provide a deeper understanding of HTTP and its protocols, as well as the development of a functional web server.

<p align="center">
  <img src="https://github.com/IcQuackson/webserv-42/assets/61185097/06367bae-12cc-4264-a0a7-4082f5b2ded6" alt="Image">
</p>

## Usage

To use Webserv, follow these steps:

1. Clone the repository to your local machine.
2. Compile the source files by using the ```make``` command
3. Run the executable to start the HTTP server: ```./webserv <configuration_file>```
4. Access the server using a web browser or tools like cURL.

## Functionalities

Webserv provides the following functionalities:

- I/O Multiplexing using poll()
- Non-blocking file descriptors for read/recv and write/send functions.
- Support for GET, POST and DELETE methods.
- Support for multipart requests.
- Compatibility with web browsers.
- Accurate HTTP response status codes.
- CGI execution.
- Ability to serve a fully static website.
- File upload and download functionality for clients.
- Directory Listing.
- Stress tested with 500 concurrent users during 60s with 100% availability.
- Successful memory management on execution and error.

The configuration file allows customization of the server such as:
- Server names.
- Client body size limit.
- Locations.
- Allowed methods.
- Default error pages.
- Route redirections.
- CGI execution settings.
- Listening to multiple ports.

## I would like to express my gratitude to my peer, pedperei, for his collaboration and support throughout this project. His contributions were invaluable and greatly appreciated.
