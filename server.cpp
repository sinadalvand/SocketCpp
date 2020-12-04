
#define WINVER 0x0A00
#define _WIN32_WINNT 0x0A00

// Standard C++ headers
#include <iostream>
#include <cstring>
#include <cstdlib>

// Socket headers
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

// UNIX system headers
#include <errno.h>

using namespace std;

char const *ip = "127.0.0.1";
char const *port = "55555";
const int MAX_PENDING = 5;
const int MAX_LINE = 256;

void handle_error(int eno, char const *msg)
{
    if (eno == 0)
        cerr << msg << endl;
    else
        cerr << msg << ": " << strerror(eno) << endl;
    exit(errno);
}

bool request_dll_and_permission()
{
#if defined(WIN32)
    WSADATA wsaData;
    WORD wVersionRequested = MAKEWORD(2, 0);
    if (WSAStartup(wVersionRequested, &wsaData) != 0)
    {
        return false;
    }
    return true;
#endif
}

int main(int argc, char *argv[])
{

    bool granted = request_dll_and_permission();
    if (!granted)
        handle_error(errno, "simplex_server - socket not permited!");

    int nread;
    struct addrinfo *result = NULL;
    struct addrinfo *p = NULL;
    struct addrinfo hints;

    memset(&hints, 0, sizeof(hints));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_INET; // IP version 4

    int rv = getaddrinfo(ip, port, &hints, &result);

    int s;
    int rstat;
    for (p = result; p != NULL; p = p->ai_next)
    {
        if ((s = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
        {
            continue;
        }

        if ((rstat = bind(s, p->ai_addr, p->ai_addrlen) == -1))
        {
            closesocket(s);
            continue;
        }

        break;
    }

    if (p == NULL)
    {
        fprintf(stderr, "failed to bind socket\n");
        exit(2);
    }

    if ((rstat = listen(s, MAX_PENDING)) < 0)
    {
        fprintf(stderr, "Error in syscall listen.\n");
    }

    int pl_one;
    socklen_t pl_one_len;
    struct sockaddr_in pl_one_addr;

    // Allocate a memory buffer for received messages
    char *buf = new char[MAX_LINE];

    // Wait for connections
    while (true)
    {
        // Upon return the sin structure will contain the address of the
        // connecting socket.  Note that a new socket is returned.

        // Get the two clients connections.
        pl_one_len = sizeof(pl_one_addr);
        int new_s = accept(s, (struct sockaddr *)&pl_one_addr, &pl_one_len);
        if (new_s == -1)
            handle_error(errno, "simplex_server - accept");

        // Receive and print messages as long as client is connected.
        while (true)
        {
            nread = recv(new_s, buf, MAX_LINE, 0);
            if (nread == -1)
                handle_error(0, "simplex_server - recv");
            if (nread == 0)
                break; // client has disconnected

            cout << buf << flush;
        }
        closesocket(new_s);
    }
    closesocket(s);
    delete[] buf;

    return 0;
}
