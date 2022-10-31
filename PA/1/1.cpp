#include <pthread.h>
#include <iostream>
#include <vector>
#include <map>
#include <cmath>
using namespace std;

struct SFECode {
    double px;
    double fx;
    double fbarx;
    string fbarxbinary;
    pthread_t tid;

    SFECode() {}
};

map<char, SFECode> codes;

void FbarBinary(SFECode& sfecode)
{
    string& binary = sfecode.fbarxbinary;
    double fbarx = sfecode.fbarx;
    int i;
    double j;
    for(i = 0, j = 0.5; i < binary.length(); i++, j /= 2)
    {
        if(j > fbarx) continue;
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

void* RunSFE(void* sfecode_arg)
{
    SFECode& sfecode = *(SFECode*)sfecode_arg;
    // set FbarX for this symbol
    
    SetOneFbarx(sfecode);
    
    // set length of fbarbinary
    sfecode.fbarxbinary = string(GetBinaryLength(sfecode.px), '0');

    // set fbarbinary
    FbarBinary(sfecode);

    return nullptr;
}

int main()
{
    string symbols, pxs;
    if(!getline(cin, symbols) || !getline(cin, pxs)){
        return EXIT_FAILURE;
    }
    
    double fx = 0.0;
    int i;
    size_t j, k;
    for(i = 0, j = 0; i < symbols.length(); i+=2, j += k)
    {
        SFECode& sfecode = codes[symbols[i]];
        sfecode.px = stod(pxs.substr(j), &k);
        fx += sfecode.px;
        sfecode.fx = fx;
        pthread_create(&sfecode.tid, NULL, &RunSFE, &sfecode);
    }

    cout << "SHANNON-FANO-ELIAS Codes:" << endl << endl;


    for(auto& [symbol, sfecode] : codes)
    {
        pthread_join(sfecode.tid, NULL);
        cout << "Symbol " << symbol << ", Code: " << sfecode.fbarxbinary << endl;
    }

    return EXIT_SUCCESS;
}
