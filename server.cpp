#include <errno.h>
#include <fcntl.h>
#include <memory.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

int main() {
    int status = 0;

    struct addrinfo hints;
    struct addrinfo* servinfo;
    const char port[] = "3542";

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    status = getaddrinfo(NULL, port, &hints, &servinfo);
    if (status != 0) {
        fprintf(stderr, "gettaddinfo error: %s\n", gai_strerror(status));
        exit(1);
    }

    int sd = socket(servinfo->ai_family,
        servinfo->ai_socktype,
        servinfo->ai_protocol);

    status = bind(sd, servinfo->ai_addr, servinfo->ai_addrlen);
    if (status < 0) {
        fprintf(stderr, "bind error: %d\n", errno);
        exit(1);
    }
    freeaddrinfo(servinfo);

    int backlog = 10;
    status = listen(sd, backlog);
    if (status < 0) {
        fprintf(stderr, "listen error: %d\n", errno);
        exit(1);
    }

    while(true) {

        struct sockaddr_storage their_addr;
        socklen_t addr_size = sizeof(their_addr);
        int conn_fd = accept(sd, (struct  sockaddr*)&their_addr, &addr_size);
        if (conn_fd == -1) {
            fprintf(stderr, "accept error: %d\n", errno);
            continue;
        }

        std::vector<char> buf;

        int read_bytes = 0;
        char read_buf[1024];
        while ((read_bytes = read(conn_fd, read_buf, sizeof(read_buf))) > 0) {
            buf.insert(buf.end(), read_buf, read_buf + read_bytes);
        }
        if (read_bytes == -1) {
            fprintf(stderr, "read error: %d\n", errno);
            close(conn_fd);
            exit(1);
        }

        int write_bytes = 0;
        char* write_buf = buf.data();
        int bytes_to_write = buf.size();
        while ((write_bytes = write(conn_fd, write_buf, bytes_to_write)) > 0) {
            write_buf += write_bytes;
            bytes_to_write -= write_bytes;
        }
        if (write_bytes == -1) {
            fprintf(stderr, "write error: %d\n", errno);
            close(conn_fd);
            exit(1);
        }

        close(conn_fd);
    }

    return 0;
}