#include "client.h"

#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/event.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#define BUFSIZE 1024

int tcpOpen(const char *host, int port, char *error, size_t errorLength);
int sendBufferToSocket(int sckfd, const char *buf, size_t len);

int connectAndRead(char *error, size_t errorLength, void (^onRead)(char *)) {
    struct kevent chlist[1];   /* events we want to monitor */
    struct kevent evlist[1];   /* events that were triggered */
    char buffer[BUFSIZE];
    char *request = "Hello world\n";
    int sckfd, kq, events_count, i;

    if ((sckfd = tcpOpen("fzed.io", 1337, error, errorLength)) < 0) {
        return EXIT_FAILURE;
    }

    if ((kq = kqueue()) == -1) {
        strncpy(error, "kqueue() failed", errorLength);
        return EXIT_FAILURE;
    }

    EV_SET(&chlist[0], sckfd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, 0);

    /* Send initial mock request */
    if (sendBufferToSocket(sckfd, request, strlen(request)) < 0) {
        strncpy(error, "sendBufferToSocket() failed", errorLength);
        return EXIT_FAILURE;
    }

    for (;;) {
        if ((events_count = kevent(kq, chlist, 1, evlist, 1, NULL)) < 0) {
            strncpy(error, "kevent() failed", errorLength);
            return EXIT_FAILURE;
        }

        if (events_count == 0) {
            continue;
        }

        for (i = 0; i < events_count; i++) {
            if (evlist[i].flags & EV_ERROR) {
                snprintf(error, errorLength, "EV_ERROR: %s\n", strerror((int)evlist[i].data));
                exit(EXIT_FAILURE);
            }

            if (evlist[i].ident == sckfd) {
                /* We have data from the host */
                memset(buffer, 0, BUFSIZE);

                if (read(sckfd, buffer, BUFSIZE) < 0) {
                    strncpy(error, "read() failed", errorLength);
                    return EXIT_FAILURE;
                }

                onRead(buffer);
//                fputs(buffer, stdout);
            }
        }

        if (evlist[0].flags & EV_EOF) {
            break;
        }
    }

    if (close(kq) == -1) {
        strncpy(error, "close() failed", errorLength);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int tcpOpen(const char *host, int port, char *error, size_t errorLength) {
    struct sockaddr_in server;
    struct hostent *hp;
    int sckfd;

    if ((hp = gethostbyname(host)) == NULL) {
        strncpy(error, "gethostbyname() failed", errorLength);
        return -1;
    }

    if ((sckfd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        strncpy(error, "socket() failed", errorLength);
        return -1;
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr = *((struct in_addr *)hp->h_addr);
    memset(&(server.sin_zero), 0, 8);

    if (connect(sckfd, (struct sockaddr *)&server, sizeof(struct sockaddr)) < 0) {
        strncpy(error, "connect() failed", errorLength);
        return -1;
    }

    return sckfd;
}

int sendBufferToSocket(int sckfd, const char *buf, size_t len) {
    ssize_t bytessent;
    int pos;

    pos = 0;
    do {
        if ((bytessent = send(sckfd, buf + pos, len - pos, 0)) < 0) {
            return -1;
        }

        pos += bytessent;
    } while (bytessent > 0);

    return 0;
}
