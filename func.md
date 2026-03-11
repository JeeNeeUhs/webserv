# Webserv Project Allowed Functions (created using AI)

These are brief summaries. For full details, use the manual pages in your terminal by typing man <function_name> (e.g., man execve)

## Process and System Functions

- `int execve(const char *pathname, char *const argv[], char *const envp[]);` — Replaces the current process image with a new process image specified by the given executable file.
- `pid_t fork(void);` — Creates a new child process by duplicating the calling process.
- `pid_t waitpid(pid_t pid, int *wstatus, int options);` — Suspends execution of the calling process until a child process specified by the `pid` argument changes state.
- `int kill(pid_t pid, int sig);` — Sends the specified signal to a process or a group of processes.
- `void (*signal(int signum, void (*handler)(int)))(int);` — Sets the behavior (handler) for a specific signal delivered to the process.
- `int chdir(const char *path);` — Changes the current working directory of the calling process to the directory specified by `path`.

## Error Handling

- `char *strerror(int errnum);` — Returns a pointer to a string that human-readably describes the system error code passed in the argument.
- `const char *gai_strerror(int errcode);` — Translates error codes specifically returned by `getaddrinfo` into a human-readable string.
- `extern int errno;` — A special system variable (macro) set by system calls and library functions to indicate what error occurred.

## File and I/O Operations

- `int open(const char *pathname, int flags, mode_t mode);` — Opens or creates a file specified by `pathname` and returns a file descriptor.
- `ssize_t read(int fd, void *buf, size_t count);` — Attempts to read up to `count` bytes from the file descriptor `fd` into the buffer.
- `ssize_t write(int fd, const void *buf, size_t count);` — Writes up to `count` bytes from the buffer to the file referred to by the file descriptor `fd`.
- `int close(int fd);` — Closes a file descriptor so that it no longer refers to any file and may be reused.
- `int dup(int oldfd);` — Creates a copy of the file descriptor `oldfd`, automatically using the lowest-numbered unused descriptor.
- `int dup2(int oldfd, int newfd);` — Creates a copy of `oldfd` using the exactly specified file descriptor number `newfd`.
- `int pipe(int pipefd[2]);` — Creates a unidirectional data channel (pipe) that can be used for interprocess communication.
- `int fcntl(int fd, int cmd, ... /* arg */ );` — Performs various control operations on an open file descriptor, such as setting it to non-blocking mode.
- `int access(const char *pathname, int mode);` — Checks whether the calling process has the required permissions to access the file `pathname`.
- `int stat(const char *pathname, struct stat *statbuf);` — Retrieves detailed file attributes and information about the file pointed to by `pathname`.

## Directory Operations

- `DIR *opendir(const char *name);` — Opens a directory stream corresponding to the directory `name` and returns a pointer to it.
- `struct dirent *readdir(DIR *dirp);` — Returns a pointer to a structure representing the next directory entry in the open directory stream.
- `int closedir(DIR *dirp);` — Closes the directory stream associated with the given pointer.

## Networking and Sockets

- `int socket(int domain, int type, int protocol);` — Creates an endpoint for communication and returns a file descriptor referring to that endpoint.
- `int socketpair(int domain, int type, int protocol, int sv[2]);` — Creates an interconnected pair of unnamed sockets in the specified domain.
- `int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);` — Assigns a specific local address and port to the socket referred to by the file descriptor `sockfd`.
- `int listen(int sockfd, int backlog);` — Marks a socket as a passive socket that will be used to accept incoming connection requests.
- `int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);` — Accepts an incoming TCP connection attempt and creates a new connected socket for it.
- `int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);` — Initiates a connection on a socket to the destination address specified by `addr`.
- `ssize_t send(int sockfd, const void *buf, size_t len, int flags);` — Transmits a message to another connected socket.
- `ssize_t recv(int sockfd, void *buf, size_t len, int flags);` — Receives a message from a connected socket into a buffer.
- `int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen);` — Manipulates options and configurations for the socket referred to by the file descriptor `sockfd`.
- `int getsockname(int sockfd, struct sockaddr *addr, socklen_t *addrlen);` — Retrieves the current local address to which the socket `sockfd` is bound.
- `int getaddrinfo(const char *node, const char *service, const struct addrinfo *hints, struct addrinfo **res);` — Resolves hostnames and service names into dynamically allocated linked lists of socket address structures.
- `void freeaddrinfo(struct addrinfo *res);` — Frees the dynamically allocated memory created by a successful call to `getaddrinfo`.
- `struct protoent *getprotobyname(const char *name);` — Retrieves network protocol information from the system database matching the protocol `name`.

## Byte Order Conversions

- `uint16_t htons(uint16_t hostshort);` — Converts an unsigned short integer from host byte order to network byte order.
- `uint32_t htonl(uint32_t hostlong);` — Converts an unsigned integer from host byte order to network byte order.
- `uint16_t ntohs(uint16_t netshort);` — Converts an unsigned short integer from network byte order to host byte order.
- `uint32_t ntohl(uint32_t netlong);` — Converts an unsigned integer from network byte order to host byte order.

## Multiplexing (I/O Polling)

- `int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);` — Monitors multiple file descriptors simultaneously to see if any are ready for reading, writing, or have exceptional conditions.
- `int poll(struct pollfd *fds, nfds_t nfds, int timeout);` — Waits for one of a set of file descriptors to become ready to perform I/O, providing a more modern array-based interface compared to select.

### epoll (Linux specific)

- `int epoll_create(int size);` — Initializes an epoll instance for scalable I/O event notification and returns a file descriptor for it.
- `int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);` — Modifies the list of file descriptors monitored by the epoll instance.
- `int epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout);` — Waits for and retrieves I/O events from the epoll instance.

### kqueue (macOS / BSD specific)

- `int kqueue(void);` — Creates a new kernel event queue and returns a descriptor.
- `int kevent(int kq, const struct kevent *changelist, int nchanges, struct kevent *eventlist, int nevents, const struct timespec *timeout);` — Registers new events with the kqueue and retrieves any currently pending events.
