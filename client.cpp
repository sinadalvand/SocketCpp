// Standard C++ headers
#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>

// Socket headers
#include <winsock2.h>
#include <windows.h>

// UNIX system headers
#include <errno.h>

using namespace std;

const int SERVER_PORT = 5432;
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

void log(char *msg)
{
    cout << msg << endl;
}

int main(int argc, char *argv[])
{

    // if (argc != 2)
    // handle_error(0, "usage: simplex_client host");

    // char *host = argv[1];

    bool granted = request_dll_and_permission();
    if (!granted)
        handle_error(errno, "simplex_server - socket not permited!");

    // Convert hostname to IP address
    hostent *hp = gethostbyname("localhost");

    if (hp == NULL)
        handle_error(0, "simplex_client - gethostbyname (lookup error)");
    if (hp->h_addrtype != AF_INET)
        handle_error(0, "simplex_client - gethostname (not IPv4)");

    // Translate hostname and port number to a remote socket address
    sockaddr_in sin;
    sockaddr *sin_p = (sockaddr *)&sin;
    sin.sin_family = AF_INET; // IP version 4
    sin.sin_port = htons(SERVER_PORT);
    // h_addr contains the memory address of the IP address (?!?)
    sin.sin_addr.s_addr = *(uint32_t *)hp->h_addr;
    const char *rmem = inet_ntoa(sin.sin_addr);
    if (rmem == NULL)
        handle_error(errno, "simplex_client - inet_ntop");
    cout << "server IP address: " << rmem << endl;

    // Create a socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1)
        handle_error(errno, "simplex_client - socket");

    // Connect to the server
    int rstat = connect(sock, sin_p, sizeof(sin));
    if (rstat == -1)
        handle_error(errno, "simplex_client - connect");

    string buf;
    while (getline(cin, buf))
    { // type ^D to quit
        // getline strips the newline, we want to pass it to the server
        buf.push_back('\n');
        // send takes a C string as an argument, NOT a C++ string
        // The string and the terminating 0 is sent, hence length()+1,
        // not length()
        send(sock, buf.c_str(), buf.length() + 1, 0);

        if (buf == "1\n")
        {
            cout << "Start get Image" << endl;

            FILE *fp = fopen("recv-bg.jpg", "w");
            while (1)

            {
                cout << "Buffering ..." << endl;
                char Buffer[2] = "";
                if (recv(sock, Buffer, sizeof(Buffer), 0))
                {
                    string duffer = Buffer;
                    if (duffer == "Hi")
                    {
                        break;
                    }
                    else
                    {
                        cout << "Writed ..." << endl;
                        fwrite(Buffer, sizeof(Buffer), 1, fp);
                    }
                }
            }
            fclose(fp);
            cout << "Done" << endl;
        }
    }
    closesocket(sock);
    return 0;
}
