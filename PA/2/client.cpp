#include <sys/socket.h>
#include <arpa/inet.h>
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <map>
#include <algorithm>
#include <vector>
#include <netdb.h>
using namespace std;

namespace Solution{

    struct SFECode {
        char symbol;
        double px;
        double fx;
        double fbarx;
        string fbarxbinary;
        pthread_t tid;

        SFECode() {}
    };

    string hostname;
    uint16_t port;
    vector<SFECode> sfeCodes;

    enum EventType
    {
        SEND_SYMBOL,
        RECV_LEN,
        RECV_CODE
    };

    struct Query
    {
        EventType eventType;
        string msg;
        const string ToString() const
        {
            return string(to_string(eventType) + msg);
        }


        Query(EventType e, string msg) : eventType(e), msg(msg) {}
        Query() {}
    };

    class Socket
    {
    private:
        int id;
        int bytesRead;
        sockaddr_in server_address;
    public:

        Socket()
        {
            if ((id = socket(AF_INET, SOCK_STREAM, 0)) < 0)
            {
                cerr << "Socket creation error" << endl;
                return;
            }
            server_address.sin_family = AF_INET;
            server_address.sin_port = htons(port);

            struct hostent *host_entry = gethostbyname(hostname.c_str());

            char* IP = inet_ntoa(*((struct in_addr*) host_entry->h_addr_list[0]));

            if (inet_pton(AF_INET, IP, &server_address.sin_addr) <= 0)
            {
                cerr << "Invalid address" << endl;
                return;
            }
            if (connect(id, (sockaddr*)&server_address, sizeof(server_address)) < 0)
            {
                cerr << "Connection Failed" << endl;
                return;
            }
        }

        void GetCode(SFECode& sfecode)
        {
            Query symbolSend(SEND_SYMBOL, string(1, sfecode.symbol) + to_string(sfecode.fx) + " " + to_string(sfecode.px));

            send(id, symbolSend.ToString().c_str(), strlen(symbolSend.ToString().c_str()), 0);

            size_t tmp, n;

            read(id, &tmp, sizeof(tmp));
            n = ntohl(tmp);

            char* result = new char[n];

            if ((bytesRead = read(id, result, n)) <= 0)
            {
                cerr << "could not receive message properly" << endl;
            }

            string res(result, n);

            delete[] result;

            sfecode.fbarxbinary = res;
        }

        ~Socket()
        {
            close(id);
        }
    };

    bool compSFE(SFECode& left, SFECode& right)
    {
        return right.px < left.px || left.symbol < right.symbol;
    }

    void* RunSFE(void* void_code)
    {
        SFECode& sfecode = *(SFECode*)void_code;

        Socket sock;

        sock.GetCode(sfecode);

        return nullptr;
    }

    void input()
    {
        string symbols;

        getline(cin, symbols);

        map<char, int> freq;

        for (char c : symbols)
        {
            freq[c]++;
        }

        sfeCodes.resize(freq.size());
        int i = 0;
        double fx = 0.0;

        for (const auto& [symbol, frequency] : freq)
        {
            sfeCodes[i].symbol = symbol;
            sfeCodes[i].px = (double)frequency / symbols.size();

            i++;
        }

        sort(sfeCodes.begin(), sfeCodes.end(), compSFE);

        for (i = 0; i < sfeCodes.size(); i++)
        {
            fx += sfeCodes[i].px;
            sfeCodes[i].fx = fx;
        }
    }

    void run()
    {
        for (SFECode& sfecode : sfeCodes)
        {
            pthread_create(&sfecode.tid, NULL, &RunSFE, &sfecode);
        }

        cout << "SHANNON-FANO-ELIAS Codes:" << endl << endl;


        for (SFECode& sfecode : sfeCodes)
        {
            pthread_join(sfecode.tid, NULL);
            cout << "Symbol " << sfecode.symbol << ", Code: " << sfecode.fbarxbinary << endl;
        }


    }
}

int main(int argc, char** argv)
{
    Solution::hostname = argv[1];
    Solution::port = stoi(argv[2]);

    Solution::input();
    Solution::run();

    return EXIT_SUCCESS;
}
