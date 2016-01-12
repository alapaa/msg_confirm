#include <iostream>
#include <string>
#include <sstream>
#include <stdexcept>
#include <system_error>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <strings.h> // bzero

#include "socket_wrapper.h"


#define BUFLEN 512

using std::string;
using std::cin;
using std::cout;
using std::cerr;
using std::stringstream;
using std::getline;

using namespace Netrounds::Util;

enum Progtype
{
    CLIENT = 1,
    SERVER,
    UNDEF
};

void run_client(string server_ip, unsigned short server_port)
{
    struct sockaddr_in si_serv;
    int retval;

    string msg;

    SocketWrapper swr(socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP));
    if (swr.get() == -1)
    {
        throw std::system_error(errno, std::system_category());
    }

    bzero(&si_serv, sizeof(si_serv));
    si_serv.sin_family = AF_INET;
    si_serv.sin_port = htons(server_port);
    retval = inet_pton(AF_INET, server_ip.c_str(), &si_serv.sin_addr);
    if (retval != 1) {
        throw std::runtime_error("inet_pton() failed!");
    }

    cout << "--- Enter strings to send, terminate with ctrl-D (EOF)\n";
    while (getline(cin, msg))
    {
        cout << "--- Sending msg\'" << msg << "\'...\n";
        retval = sendto(swr.get(), msg.c_str(), msg.length()+1, 0,
                        (struct sockaddr*)&si_serv, sizeof(si_serv));
        if (retval == -1)
        {
            throw std::system_error(errno, std::system_category());
        }
        cout << "--- msg sent.";
    }

    cout << "Done sending, terminating!\n";
}

void run_server(unsigned short server_port)
{
    struct sockaddr_in si_serv;
    struct sockaddr_in si_cli;
    socklen_t slen = sizeof(si_cli);
    char buf[BUFLEN];
    int retval;

    SocketWrapper swr(socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP));
    if (swr.get() == -1)
    {
        throw std::system_error(errno, std::system_category());
    }

    bzero(&si_serv, sizeof(si_serv));
    si_serv.sin_family = AF_INET;
    si_serv.sin_port = htons(server_port);
    si_serv.sin_addr.s_addr = htonl(INADDR_ANY);

    retval = bind(swr.get(), (sockaddr*)(&si_serv), sizeof(si_serv));
    if (retval == -1)
    {
        throw std::system_error(errno, std::system_category());
    }

    for (;;)
    {
        retval = recvfrom(swr.get(), buf, BUFLEN, 0, (sockaddr*)&si_cli, &slen);
        if (retval == -1)
        {
            throw std::system_error(errno, std::system_category());
        }

        cout << "Received packet from " << inet_ntoa(si_cli.sin_addr)
             << ":" << ntohs(si_cli.sin_port) << ". Data: \'" << buf << "\'\n";
    }
}

int main(int argc, char* argv[])
{
    Progtype progtype = UNDEF;

    unsigned short server_port = -1;
    string server_ip;
    if (argc == 2)
    {
        // server
        progtype = SERVER;
        stringstream ss(argv[1]);
        ss >> server_port;
        if (ss.fail()) {
            throw std::runtime_error("Error converting server port!");
        }

    }
    else if (argc == 3)
    {
        // client
        progtype = CLIENT;
        server_ip = argv[1];
        stringstream ss(argv[2]);
        ss >> server_port;
        if (ss.fail()) {
            throw std::runtime_error("Error converting server port!");
        }
    }
    else if (argc < 2 || argc > 3)
    {
        cerr << "Usage:\n"
            "sendrecv -server <recv port> OR\n"
            "sendrecv -client <server IPv4> <server port>\n";
    }

    if (progtype == SERVER)
    {
        run_server(server_port);
    }
    else if (progtype == CLIENT)
    {
        run_client(server_ip, server_port);
    }

    return 0;
}


// #include <arpa/inet.h>
//      2  #include <netinet/in.h>
//           3  #include <stdio.h>
//           4  #include <sys/types.h>
//           5  #include <sys/socket.h>
//           6  #include <unistd.h>
//           7
//           8  #define BUFLEN 512
//           9  #define NPACK 10
//          10  #define PORT 9930
//          11
//      12  void diep(char *s)
//      13  {
//          14    perror(s);
//          15    exit(1);
//          16  }
//     17
//     18  int main(void)
//     19  {
//         20
//         21    int s, i, slen=sizeof(si_other);
//         22    char buf[BUFLEN];
//             23
//                 24    if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
//                 25      diep("socket");
//                 26
//                     27    memset((char *) &si_me, 0, sizeof(si_me));
//                 28    si_me.sin_family = AF_INET;
//                 29    si_me.sin_port = htons(PORT);
//                 30    si_me.sin_addr.s_addr = htonl(INADDR_ANY);
//                 31    if (bind(s, &si_me, sizeof(si_me))==-1)
//                     32        diep("bind");
//                     33
//                         34    for (i=0; i<NPACK; i++) {
//                         35      if (recvfrom(s, buf, BUFLEN, 0, &si_other,
//      &slen)==-1)
//                             36        diep("recvfrom()");
//                         37      printf("Received packet from %s:%d\nData:
//      %s\n\n",
//                                        38
//                                        inet_ntoa(si_other.sin_addr),
//                                        ntohs(si_other.sin_port), buf);
//                         39    }
//                         40
//                             41    close(s);
//                         42    return 0;
//                         43 }
