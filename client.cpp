#define WINVER 0x0A00
#define _WIN32_WINNT 0x0A00

// Standard C++ headers
#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>

// Socket headers
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

// UNIX system headers
#include <errno.h>

using namespace std;

char const *port = "55555";
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

char const *host = "localhost";

int main(int argc, char *argv[])
{

    bool granted = request_dll_and_permission();
    if (!granted)
        handle_error(errno, "simplex_server - socket not permited!");

    // if (argc != 2)
    //     handle_error(0, "usage: simplex_client host");

    // char *host = argv[1];

    struct addrinfo *result = NULL;
    struct addrinfo *p = NULL;
    struct addrinfo hints;

    memset(&hints, 0, sizeof(hints));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_INET; // IP version 4

    int rv = getaddrinfo(host, port, &hints, &result);

    int s;
    int rstat;
    for (p = result; p != NULL; p = p->ai_next)
    {

        const char *rmem = inet_ntoa(((struct sockaddr_in *)p->ai_addr)->sin_addr);
        printf("The IP address is: %s\n", rmem);

        if ((s = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
        {
            continue;
        }

        if ((rstat = connect(s, p->ai_addr, p->ai_addrlen) == -1))
        {
            closesocket(s);
            continue;
        }

        break;
    }

    string buf;
    while (getline(cin, buf))
    { // type ^D to quit
        // getline strips the newline, we want to pass it to the server
        buf.push_back('\n');
        // send takes a C string as an argument, NOT a C++ string
        // The string and the terminating 0 is sent, hence length()+1,
        // not length()
        send(s, buf.c_str(), buf.length() + 1, 0);
    }
    freeaddrinfo(p);
    closesocket(s);
    return 0;
}