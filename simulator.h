#include <deque>
#include <string>
#include <vector>
#include <deque>
#include <unordered_map>

using namespace std;
#ifndef SIMULATOR_H
#define SIMULATOR_H

#define NUM_FP 32
#define SIZE_MEM 32 // define the size of memory
#define NUM_FUS 5   // the number of function unit
#define CYCS_IN 1   // the number of cycle to finish an integer instruction
#define EMPTY -1    // for convinence ,define empty as -1
#define ZERO 0.0    // for memory comparison

/*Implement 9 instructions*/
enum Instrs
{
    FLD,
    FSD,
    ADD,
    ADDI,
    FADD,
    FSUB,
    FMUL,
    FDIV,
    BNE
};

struct Instruction
{
    Instrs Op;
    string label;
    string rd;
    string rs;
    string rt;
};

// class declaration.
class Simulator
{
private:
    string filename = "";
    deque<Instruction> instruction_list;
    unordered_map<string, string> mapping_table;
    int physical_mem[SIZE_MEM];

public:
    Simulator();       // constructor
    ~Simulator();      // destructor
    int init_in_gpr(); // Initialize general-purpose integer registers.
    int init_fp_gpr(); // Initialize general-purpose floating-point registers.
    int init_mem();    // Initialize memory;

    bool read_instructions(const char *); // Initialize integer registers, fp registers, and memory from file
};
#endif