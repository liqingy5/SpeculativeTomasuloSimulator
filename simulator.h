#include <deque>
#include <string>
#include <vector>
#include <deque>
#include <unordered_map>

using namespace std;
#ifndef SIMULATOR_H
#define SIMULATOR_H

#define NUM_GP 32
#define SIZE_MEM 32 // define the size of memory
#define NUM_FUS 5   // the number of function unit
#define CYCS_IN 1   // the number of cycle to finish an integer instruction
#define EMPTY -1    // for convinence ,define empty as -1
#define ZERO 0.0    // for memory comparison

#define INT_LATENCY = 1
#define LD_ST_LATENCY = 1
#define FADD_LATENCY = 3
#define FMULT_LATENCY = 4
#define FDIV_LATENCY = 8
#define BU_LATENCY = 1

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
    int address;
    Instrs Op;
    string rd;
    string rs;
    string rt;
    int imme;
};

// class declaration.
class Simulator
{
private:
    string filename = "";
    deque<Instruction> instruction_list;
    deque<Instruction> fetch_queue;
    deque<Instruction> decode_queue;
    unordered_map<string, int> mapping_table;
    unordered_map<string, int> branch_address;
    unordered_map<int, pair<int, int>> BTB;
    deque<int> free_list;
    unordered_map<int, int> memory_content;
    float physical_mem[SIZE_MEM];
    int NF, NW, NB, NR;
    int PC = 0;
    int address = 0;
    int cycles = 0;

public:
    Simulator();  // constructor
    ~Simulator(); // destructor

    int init_mem();   // Initialize memory;
    void sim_start(); // start the simulator;
    void set_parameter(int NF, int NW, int NB, int NR);
    bool read_memory(const char *);       // read memory content from file
    bool read_instructions(const char *); // read instruction from file
    bool fetch();                         // Fetch instructions from instruction list;
    bool decode();                        // Decode instructions from fetch queue
    bool issue();                         // issue instructions from decode queue
    bool register_rename(string &reg);
    void print_ins_list();
    void print_mem_list();
    void print_rename_list();
};
#endif