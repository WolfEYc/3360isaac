#include <pthread.h>
#include <iostream>
#include <vector>
#include <map>
#include <cmath>
#include <algorithm>
using namespace std;

struct Synchronization
{
    pthread_mutex_t runSem;    // Mutex semaphore
    int runTurn; // which thread to run

    pthread_mutex_t outputSem;    // Mutex semaphore
    pthread_cond_t waitToOutput;  // Condition variable to control the turn
    int outputTurn; // which thread can output

    Synchronization()
    {
        pthread_mutex_init(&runSem, NULL);
        runTurn = 0;

        pthread_mutex_init(&outputSem, NULL);
        waitToOutput = PTHREAD_COND_INITIALIZER;
        outputTurn = 0;
    }
};

void* RunSFE(void* main_arg);

struct SFECode {
public:
    char symbol;
    double px;
    double fx;
    double fbarx;
    string fbarxbinary;
    pthread_t tid;


    SFECode() {}

private:
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
public:
    void RunCode()
    {
        SetFBarX();
        // set length of fbarbinary
        SetBinaryLen();
        // set fbarbinary
        FbarBinary();
        //output the code using the cooler way (semaphores and stuff)
    }

    void Output()
    {
        cout << "Symbol " << symbol << ", Code: " << fbarxbinary << endl;
    }
};

bool compSFE(SFECode& left, SFECode& right)
{
    return right.px < left.px || left.symbol < right.symbol;
}

class Main {
private:
    Synchronization sync;
    vector<SFECode> codes;

    void Output(const int order)
    {
        pthread_mutex_lock(&sync.outputSem);
        while(sync.outputTurn != order){
            pthread_cond_wait(&sync.waitToOutput, &sync.outputSem);
        }
        pthread_mutex_unlock(&sync.outputSem);

        codes[order].Output();

        pthread_mutex_lock(&sync.outputSem);
        sync.outputTurn++;
        pthread_cond_broadcast(&sync.waitToOutput);
        pthread_mutex_unlock(&sync.outputSem);
    }

    int GetOrder()
    {
        pthread_mutex_lock(&sync.runSem);
        int toRun = sync.runTurn++; // get which code to run on THIS thread
        pthread_mutex_unlock(&sync.runSem);
        return toRun;
    }
    
    void OutputHeader()
    {
        cout << "SHANNON-FANO-ELIAS Codes:" << endl << endl;
    }
    
    void StartThreads()
    {
        for(SFECode& code : codes)
        {
            pthread_create(&code.tid, NULL, RunSFE,this);
        }
    }
    
    void EndThreads()
    {
        for(SFECode& code : codes)
        {
            pthread_join(code.tid, NULL);
        }
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
        codes.resize(freq.size());

        int i = 0;
        double fx = 0.0;

        for (const auto& [symbol, frequency] : freq)
        {
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

public:
    Main() {}

    void RunThread()
    {
        int order = GetOrder();
        codes[order].RunCode();
        Output(order);
    }
    
    void Run()
    {
        input();
        OutputHeader();
        StartThreads();
        EndThreads();
    }
};

void* RunSFE(void* main_arg)
{
    Main& mainArg = *(Main*)main_arg;

    mainArg.RunThread();

    return nullptr;
}


int main()
{
    Main mainArg;

    mainArg.Run();

    return EXIT_SUCCESS;
}
