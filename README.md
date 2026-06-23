*This project has been created as part of the 42 curriculum by ahekinci, ebabaogl.*

## Description
webserv is a non-blocking HTTP server written from scratch in **C++98**.
The server is driven by a single I/O multiplexing call `poll()` that monitors
reading and writing simultaneously for every socket.

### Scope and limitations
- The implementation targets **HTTP/1.0** as its reference point. HTTP/1.1 is not
  fully supported, only the subset needed to interoperate with modern browsers and
  the required features is implemented.
- Virtual hosts are not implemented. As stated in the subject, this feature is
  considered out of scope.

### Features
- Single `poll()`-equivalent event loop for all client and listening sockets.
- Listens on multiple `interface:port` pairs, each serving its own content.
- `GET`, `POST` and `DELETE` methods.
- Static file serving.
- File uploads from clients to a configurable storage location.
- Per-route configuration: accepted methods, HTTP redirections, root directory,
  directory listing (autoindex) on/off, default index file, and upload location.
- CGI execution based on file extension, including correct working directory and
  environment variables; chunked request bodies are un-chunked before being handed
  to the CGI.
- Maximum client request body size limit.
- Custom error pages, with built-in default error pages when none are provided.

### Bonus
The bonus part is included:

- Cookies and session management: see the provided example configuration and pages.
- Multiple CGI types: the server can be configured to handle more than one CGI
  interpreter.

## Instructions
### Requirements
- A C++ compiler supporting `-std=c++98`
- `make`
- A CGI interpreter if you want to test the CGI features

### Build
```sh
make        # builds the webserv binary
make clean  # removes object files
make fclean # removes object files and the binary
make re     # fclean + all
```
 
### Run
```sh
./webserv [configuration file]
```

If no configuration file is given as an argument, a default path is used.

```sh
./webserv config/default.conf
```

Then open the configured address in a browser, for example:
```
http://localhost:8080/
```

### Configuration
The configuration file lets you define, among other things:
 
- All `interface:port` pairs the server listens on.
- Default error pages.
- The maximum allowed client request body size.
- Per-route rules (no regex):
  - the list of accepted HTTP methods,
  - HTTP redirections,
  - the root directory the requested file is resolved against
    (e.g. `/kapouet` rooted to `/tmp/www` resolves `/kapouet/pouic/toto/pouet`
    to `/tmp/www/pouic/toto/pouet`),
  - directory listing enabled/disabled,
  - the default file to serve when the request targets a directory,
  - the upload storage location,
  - CGI execution based on file extension.

## Resources
References used for this project:
 
- https://www.alimnaqvi.com/blog/webserv
- https://datatracker.ietf.org/doc/html/rfc3875
- https://datatracker.ietf.org/doc/html/rfc1945
- https://datatracker.ietf.org/doc/html/rfc2616
- https://en.wikipedia.org/wiki/Uniform_Resource_Identifier
- https://pypi.org/project/legacy-cgi/
- https://pkg.go.dev/net/http/cgi
- https://datatracker.ietf.org/doc/html/rfc3986#section-5.2.2
- https://github.com/nginx/nginx/blob/d44205284fa41662da803b796d6056fc1e59b1f3/src/http/modules/ngx_http_static_module.c#L148
- https://datatracker.ietf.org/doc/html/rfc2046
- https://datatracker.ietf.org/doc/html/rfc9110
- https://datatracker.ietf.org/doc/html/rfc6265

### Use of AI
AI tools were used to:

- clarify points of the HTTP and CGI specifications and the meaning of certain RFC
  sections;
- brainstorm and review the design of specific components (e.g. the configuration
  parser, the request parser);

All AI-generated suggestions were reviewed, tested, and discussed with authors before being
integrated.
