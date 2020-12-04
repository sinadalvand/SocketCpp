#define WINVER 0x0A00
#define _WIN32_WINNT 0x0A00

#include <stdio.h>
#include <string>
#include <iostream>
#include <exception>

// Socket headers
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

using namespace std;

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

    if (argc != 2)
        handle_error(0, "usage: showip host");

    char *host = argv[1];

    struct addrinfo *res = NULL;
    struct addrinfo *p = NULL;
    struct addrinfo hints;

    memset(&hints, 0, sizeof(hints));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_UNSPEC; // IP version 4

    int stauts;
    if ((stauts = getaddrinfo(host, NULL, &hints, &res)) != 0)
    {
        fprintf(stderr, "getaddinfo: %s\n", gai_strerror(stauts));
        return 2;
    }

    printf("IP addresses for %s\n\n", host);

    int s;
    int rstat;
    void *addr;
    char *ipver;
    for (p = res; p != NULL; p = p->ai_next)
    {
        if (p->ai_family == AF_INET)
        {
            struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
            addr = &(ipv4->sin_addr);
            ipver = "IPV4";
        }
        else
        {
            struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
            addr = &(ipv6->sin6_addr);
            ipver = "IPV6";
        }

        const char *ipstr = inet_ntoa(((struct sockaddr_in *)p->ai_addr)->sin_addr);
        printf("%s:%s\n", ipver, ipstr);

        break;
    }

    freeaddrinfo(res);
    return 0;
}
