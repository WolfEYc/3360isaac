#include <pthread.h>
#include <iostream>
#include <vector>
#include <map>
#include <cmath>
#include <algorithm>
using namespace std;

static pthread_mutex_t bsem;    // Mutex semaphore
static pthread_cond_t waitTurn;  // Condition variable to control the turn
static int turn;

struct SFECode {
    int order;
    char symbol;
    double px;
    double fx;
    double fbarx;
    string fbarxbinary;
    pthread_t tid;

    SFECode() {}

    void SetFBarX()
    {
        fbarx = (fx - px) + px / 2;
    }

    void SetBinaryLen()
    {
        fbarxbinary = string(ceil(log2(1 / px)) + 1, '0');
    }

    void FbarBinary()
    {
        double fbx = fbarx;
        int i;
        double j;
        for(i = 0, j = 0.5; i < fbarxbinary.length(); i++, j /= 2)
        {
            if(j > fbx) continue;
            fbarxbinary[i] = '1';
            fbx -= j;
        }
    }

    void Output()
    {
        pthread_mutex_lock(&bsem);
        while(turn != order){
            pthread_cond_wait(&waitTurn, &bsem);
        }
        pthread_mutex_unlock(&bsem);

        cout << "Symbol " << symbol << ", Code: " << fbarxbinary << endl;

        pthread_mutex_lock(&bsem);
        turn++;
        pthread_cond_broadcast(&waitTurn);
        pthread_mutex_unlock(&bsem);
    }

    void Test()
    {
        cout << "Order:" << order << " Symbol:" << symbol << " px:" << px << " fx:" << fx << endl;
    }
};

bool compSFE(SFECode& left, SFECode& right)
{
    return right.px < left.px || left.symbol < right.symbol;
}


void* RunSFE(void* sfecode_arg)
{
    SFECode& sfecode = *(SFECode*)sfecode_arg;
    // set FbarX for this symbol
    sfecode.SetFBarX();
    // set length of fbarbinary
    sfecode.SetBinaryLen();
    // set fbarbinary
    sfecode.FbarBinary();
    //output the code using the cooler way (semaphores and stuff)
    sfecode.Output();

    return nullptr;
}

void InitStatic()
{
    pthread_mutex_init(&bsem, NULL);
    waitTurn = PTHREAD_COND_INITIALIZER;
    turn = 0;
}

void input(vector<SFECode>& codes)
{
    string symbols;

    getline(cin, symbols);

    map<char, int> freq;

    for (char c : symbols)
    {
        freq[c]++;
    }
    codes.resize(freq.size());

    int i = 0;
    double fx = 0.0;

    for (const auto& [symbol, frequency] : freq)
    {
        codes[i].order = i;
        codes[i].symbol = symbol;
        codes[i].px = (double)frequency / symbols.size();

        i++;
    }

    sort(codes.begin(), codes.end(), compSFE);

    for (i = 0; i < codes.size(); i++)
    {
        fx += codes[i].px;
        codes[i].fx = fx;
    }
}

void StartThreads(vector<SFECode>& codes)
{
    cout << "SHANNON-FANO-ELIAS Codes:" << endl << endl;
    for(SFECode& code : codes)
    {
        pthread_create(&code.tid, NULL, RunSFE, &code);
    }
}

void EndThreads(vector<SFECode>& codes)
{
    for(SFECode& code : codes)
    {
        pthread_join(code.tid, NULL);
    }
}

int main()
{
    InitStatic();

    vector<SFECode> codes;

    input(codes);
    StartThreads(codes);
    EndThreads(codes);

    return EXIT_SUCCESS;
}
