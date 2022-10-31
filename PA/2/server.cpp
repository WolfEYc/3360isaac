#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string>
#include <vector>
#include <string.h>
#include <unistd.h>
#include <cmath>
using namespace std;


namespace Solution {

    struct SFECode {
        char symbol;
        double px;
        double fx;
        double fbarx;
        string fbarxbinary;
        pthread_t tid;

        SFECode() {}
    };

    void FbarBinary(SFECode& sfecode)
    {
        string& binary = sfecode.fbarxbinary;
        double fbarx = sfecode.fbarx;

        int i;
        double j;

        for (i = 0, j = 0.5; i < binary.length(); i++, j /= 2)
        {
            if (j > fbarx) continue;
            binary[i] = '1';
            fbarx -= j;
        }
    }

    unsigned int GetBinaryLength(double& px)
    {
        return ceil(log2(1 / px)) + 1;
    }

    void SetOneFbarx(SFECode& current)
    {
        current.fbarx = (current.fx - current.px) + current.px / 2;
    }

    string RunSFE(string input)
    {
        SFECode sfecode;

        sfecode.symbol = input[0];
        input = input.substr(1);

        sfecode.fx = stod(input);

        input = input.substr(input.find(' '));

        sfecode.px = stod(input);


        SetOneFbarx(sfecode);

        // set length of fbarbinary
        sfecode.fbarxbinary = string(GetBinaryLength(sfecode.px), '0');

        // set fbarbinary
        FbarBinary(sfecode);

        return sfecode.fbarxbinary;
    }
}

namespace MyServer
{
#define BUF_SIZE 32
#define DEFAULT_BACKLOG 5
    const int opt = 1;
    const int domain = AF_INET;
    const int type = SOCK_STREAM;
    const int hostaddress = INADDR_ANY;
    const int protocol = 0;

    enum EventType
    {
        SEND_SYMBOL,
        RECV_LEN,
        RECV_CODE
    };

    void EndChild(int signum)
    {
        wait(NULL);
    }

    class Server;

    class Socket
    {
    private:
        int id;
        char buffer[BUF_SIZE] = { 0 };
        sockaddr_in address;
        int addrlen;
        const Server* server;
        void HandleEvent();
        void on(EventType event, string msg);
    public:
        Socket(const Server* server);
        Socket(const Socket& passive);
        ~Socket()
        {
            close(id);
        }
    };

    class Server
    {
    private:
        int port;
        Socket passive;


    public:
        friend class Socket;
        Server(int port) : port(port), passive(this)
        {
            while(true)
            {
                Socket active(passive);
            }
        }
    };


    //Passive Socket
    Socket::Socket(const Server* server) : server(server)
    {
        addrlen = sizeof(address);
        id = socket(domain, type, protocol);
        setsockopt(id, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));

        address.sin_family = domain;
        address.sin_addr.s_addr = hostaddress;
        address.sin_port = htons(server->port);

        bind(id, (sockaddr*)&address, sizeof(address));

        listen(id, DEFAULT_BACKLOG);
    }

    //Active Socket
    Socket::Socket(const Socket& passive) : server(passive.server)
    {
        id = accept(passive.id, (sockaddr*)&passive.address, (socklen_t*)&passive.addrlen);

        address = passive.address;
        addrlen = passive.addrlen;
        read(id, buffer, BUF_SIZE);


        int pid = fork();
        if(!pid)
        {
            HandleEvent();
            exit(0);
        }
        else
        {
            signal(SIGCHLD, EndChild);
        }

    }

    void Socket::HandleEvent()
    {
        EventType event = (EventType)(buffer[0] - '0');
        string msg(buffer);

        msg = msg.substr(1);
        on(event, msg);

    }

    void Socket::on(EventType event, string msg)
    {
        string response;
        size_t len;

        response = Solution::RunSFE(msg);
        len = htonl(response.length());

        send(id, &len, sizeof(len), 0);
        send(id, response.c_str(),  strlen(response.c_str()), 0);


    }

}

int main (int argc, char** argv)
{
    int port = stoi(argv[1]);
    MyServer::Server server(port);
    return EXIT_SUCCESS;
}
