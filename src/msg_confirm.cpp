#include <iostream>
#include <string>
#include <sstream>
#include <stdexcept>
#include <system_error>
#include <chrono>
#include <thread>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <strings.h> // bzero

#include "socket_wrapper.h"


#define BUFLEN 2048

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

void run_client(string server_ip, string server2_ip, unsigned short server_port)
{
    struct sockaddr_in si_serv;
    struct sockaddr_in si_serv2;
    struct sockaddr_in* chosen_serv;
    int retval;
    int i = 0;

    string msg;

    cout << "Client, sending UDP to " << server_ip << ":" << server_port << '\n';

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

    bzero(&si_serv2, sizeof(si_serv));
    si_serv2.sin_family = AF_INET;
    si_serv2.sin_port = htons(server_port);
    retval = inet_pton(AF_INET, server2_ip.c_str(), &si_serv2.sin_addr);
    if (retval != 1) {
        throw std::runtime_error("inet_pton() failed!");
    }

    //cout << "--- Enter strings to send, terminate with ctrl-D (EOF)\n";
    //while (getline(cin, msg))
    string emptystring(1000, ' ');
    for (;;)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        i++;
        msg = std::to_string(i);
        msg += emptystring;
        if (i % 2)
        {
            cout << "--- Sending msg \'" << msg << "\' to server 1" << "...\n";
            chosen_serv = &si_serv;
        }
        else
        {
            cout << "--- Sending msg \'" << msg << "\' to server 2" << "...\n";
            chosen_serv = &si_serv2;
        }

        retval = sendto(swr.get(), msg.c_str(), msg.length()+1, 0,
                        (struct sockaddr*)chosen_serv, sizeof(*chosen_serv));
        if (retval == -1)
        {
            throw std::system_error(errno, std::system_category());
        }
        cout << "--- msg sent.\n";
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

    int pkt_num = -1;
    int prev_pkt_num = -1;
    stringstream ss;

    cout << "Server, listening to UDP on port " << server_port << '\n';
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

        ss.str(buf);
        ss.clear();
        ss >> pkt_num;
        if (ss.fail())
        {
            throw std::runtime_error("Error converting packet number!");
        }

        if (prev_pkt_num != -1 && ((pkt_num - prev_pkt_num) != 2))
        {
            cerr << "Pkt num: " << pkt_num << " prev pkt num: " << prev_pkt_num
                 << '\n';
            throw std::runtime_error("----------------------------\n\n\n\n\n\n\n\n\n\n\n\nERROR! Missing packets!\n\n\n\n\n\n\n\n\n");
        }
        prev_pkt_num = pkt_num;

    }
}

int main(int argc, char* argv[])
{
    Progtype progtype = UNDEF;

    unsigned short server_port = -1;
    string server_ip;
    string server2_ip;
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
    else if (argc == 4)
    {
        // client
        progtype = CLIENT;
        server_ip = argv[1];
        server2_ip = argv[2];
        stringstream ss(argv[3]);
        ss >> server_port;
        if (ss.fail()) {
            throw std::runtime_error("Error converting server port!");
        }
    }
    else if (argc < 2 || argc > 3)
    {
        cerr << "Usage:\n"
            "msg_confirm <recv port> OR\n"
            "msg_confirm <server IPv4> <server port>\n";
    }

    if (progtype == SERVER)
    {
        run_server(server_port);
    }
    else if (progtype == CLIENT)
    {
        run_client(server_ip, server2_ip, server_port);
    }

    return 0;
}

