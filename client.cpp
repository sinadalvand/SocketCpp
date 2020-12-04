// Standard C++ headers
#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <fstream>

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

int sizeManager(int remain, int desiger)
{
    if (remain > desiger)
    {
        cout << " desier" << endl;
        return desiger;
    }
    else
    {
        cout << " remain" << endl;
        return remain;
    }
}
char *buffermanager(char *buffer, int remain)
{
    if (remain > BUFSIZ)
    {
        cout << " desier" << endl;
        return buffer;
    }
    else
    {
        cout << " remain" << endl;
        return new char[remain];
    }
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
    int BUFSIZE = 1024;
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

            char *buff = new char[BUFSIZ];
            //Read Picture Size
            printf("Reading Picture Size\n");
            recv(sock, buff, BUFSIZ, 0);
            int file_size = atoi(buff);
            cout << "Picture size:";
            cout << file_size << endl;

            std::ofstream myOutpue;
            myOutpue.open("c2.png", std::ios::binary);

            // //Read Picture Byte Array and Copy in file
            printf("Reading Picture Byte Array\n");
            // FILE image = fopen("c1.png", "w");
            ssize_t len;
            int remain_data = 0;
            remain_data = file_size;
            cout << "Recive Started:" << endl;
            int reciveddata = 0;
            while ((remain_data > 0) && ((len = recv(sock, buff, BUFSIZ, 0)) > 0))
            {
                myOutpue.write(buff, len);
                // fwrite(buff, 1, len, image);
                reciveddata += len;
                remain_data -= len;
                fprintf(stdout, "Receive %d bytes and we hope :- %d bytes\n", len, remain_data);
            }
            myOutpue.close();
            // fclose(image);
            cout << "Recive Ended.";
            cout << reciveddata << endl;
            // fclose(image);
        }
    }
    closesocket(sock);
    return 0;
}
