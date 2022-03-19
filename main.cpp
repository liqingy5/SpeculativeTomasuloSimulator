#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "simulator.h"
int NF = 4;
int NW = 4;
int NB = 4;
int NR = 16;
string inputfile = "";
using namespace std;
int main(int argc, char *argv[])
{
    if (argc == 2)
    {
        inputfile = argv[1];
    }
    else if (argc == 6)
    {
        inputfile = argv[1];
        NF = stoi(argv[2]);
        NW = stoi(argv[3]);
        NB = stoi(argv[4]);
        NR = stoi(argv[5]);
    }
    else
    {
        printf("please check your number parameters, it should be follow as: ./main filename NF NW NB NR \n");
        exit(1);
    }
    printf("Input file name is %s, with NF: %d, NW: %d, NB: %d, NR: %d \n", inputfile.c_str(), NF, NW, NB, NR);
    Simulator *simulator = new Simulator();
    simulator->read_instructions(inputfile.c_str());

    return 0;
}