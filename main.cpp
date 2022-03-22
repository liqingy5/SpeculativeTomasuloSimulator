#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "simulator.h"

#define DEBUG 1
int NF = 4;
int NW = 4;
int NB = 4;
int NR = 16;
string insfile = "";
string memfile = "";
using namespace std;
int main(int argc, char *argv[])
{
    if (argc == 3)
    {
        insfile = argv[1];
        memfile = argv[2];
    }
    else if (argc == 7)
    {
        insfile = argv[1];
        memfile = argv[2];
        NF = stoi(argv[3]);
        NW = stoi(argv[4]);
        NB = stoi(argv[5]);
        NR = stoi(argv[6]);
    }
    else
    {
        printf("please check your number parameters, it should be follow the pattern as: ./main filename NF NW NB NR \n");
        exit(1);
    }
    printf("Input instruction file name is %s, memory file name is %s, with NF: %d, NW: %d, NB: %d, NR: %d \n", insfile.c_str(), memfile.c_str(), NF, NW, NB, NR);
    Simulator *simulator = new Simulator();
    simulator->set_parameter(NF, NW, NB, NR);
    simulator->read_instructions(insfile.c_str());
    simulator->read_memory(memfile.c_str());
    simulator->sim_start();

    if (DEBUG)
    {
        // simulator->print_ins_list();
        // simulator->print_mem_list();
        simulator->print_rename_list();
    }
    return 0;
}