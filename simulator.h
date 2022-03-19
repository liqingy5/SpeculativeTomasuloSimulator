#include <deque>
#include <string>
using namespace std;
#ifndef SIMULATOR_H
#define SIMULATOR_H

#define NUM_IN_GPR 32 // define the number of general purpose integer registers
#define NUM_FP_GPR 32 // define the number of general purpose floating point registers
#define SIZE_MEM 32   // define the size of memory(words)
#define NUM_FUS 5     // the number of function unit
#define CYCS_IN 1     // the number of cycle to finish an integer instruction
#define EMPTY -1      // for convinence ,define empty as -1
#define ZERO 0.0      // for memory comparison
/*Implement 11 instructions where 5 of them are integer instructions, 6 of them are floating-point instructions.*/
enum Instrs
{
    LD,
    SD,
    BEQ,
    BNE,
    ADD,
    ADD_D,
    ADDI,
    SUB,
    SUB_D,
    MULT_D,
    DIV_D
};

struct Instruction
{
    Instrs Op;
    int label;
    int rd;
    int rs;
    int rt;
};

// class declaration.
class Simulator
{
private:
public:
    Simulator();       // constructor
    ~Simulator();      // destructor
    int init_in_gpr(); // Initialize general-purpose integer registers.
    int init_fp_gpr(); // Initialize general-purpose floating-point registers.
    int init_mem();    // Initialize memory;

    bool init_from_file(const char *); // Initialize integer registers, fp registers, and memory from file
};
#endif