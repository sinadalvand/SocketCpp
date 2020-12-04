// Standard C++ headers
#include <iostream>
#include <cstring>
#include <cstdlib>

// Socket headers
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>

// UNIX system headers
#include <errno.h>

using namespace std;

const int SERVER_PORT = 5432;
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
    socklen_t len;
    sockaddr_in sin;
    sockaddr *sin_p = (sockaddr *)&sin;
    sin.sin_family = AF_INET; // IP version 4
    sin.sin_port = htons(SERVER_PORT);
    sin.sin_addr.s_addr = htonl(INADDR_ANY); // Listen on all IP addresses

    // Create socket
    int s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == -1)
        handle_error(errno, "simplex_server - socket");

    // Bind socket to local address
    int rstat = bind(s, sin_p, sizeof(sin));
    if (rstat == -1)
        handle_error(errno, "simplex_server - bind");

    rstat = listen(s, MAX_PENDING);
    if (rstat == -1)
        handle_error(errno, "simplex_server - listen");

    // Allocate a memory buffer for received messages
    char *buf = new char[MAX_LINE];

    // Wait for connections
    while (true)
    {
        // Upon return the sin structure will contain the address of the
        // connecting socket.  Note that a new socket is returned.
        len = sizeof(sin);
        int new_s = accept(s, sin_p, &len);
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

            string duf = buf;
            if (duf == "1\n")
            {
                FILE *in = fopen("send_bg.jpg", "r");
                char Buffer[2] = "";
                int len;
                string head = "Image Head";
                cout << head << endl;
                send(new_s, head.c_str(), head.length() + 1, 0);
                while ((len = fread(Buffer, sizeof(Buffer), 1, in)) > 0)
                {
                    send(s, Buffer, sizeof(Buffer), 0);
                }
                string end = "Hi";
                cout << end << endl;
                send(new_s, end.c_str(), end.length() + 1, 0);
            }

            cout << buf << flush;
        }
        closesocket(new_s);
    }
    closesocket(s);
    delete[] buf;

    return 0;
}
