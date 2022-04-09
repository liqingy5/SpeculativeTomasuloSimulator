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
#define ZERO 0.0    // for memory comparison

#define INT_LATENCY 1
#define LD_ST_LATENCY 1
#define FADD_LATENCY 3
#define FMULT_LATENCY 4
#define FDIV_LATENCY 8
#define BU_LATENCY 1

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
enum State
{
    Issued,
    Execute,
    Memory,
    WB,
    Commit
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

struct Reservation_station_status
{
    bool busy = false;
    Instrs Op;
    string Vj = "";
    string Vk = "";
    string Qj = "";
    string Qk = "";
    string dest;
    int a = -1;
};

struct ROB_status
{
    string unit;
    string name;
    bool busy = false;
    Instruction ins;
    State state = Issued;
    string dest;
    double value = __DBL_MIN__;
    int cycles = 0;
};

// class declaration.
class Simulator
{
private:
    string filename = "";
    vector<string> unit = {"INT1", "INT2", "INT3", "INT4", "LOAD1", "LOAD2",
                           "STORE1", "STORE2", "FADD1", "FADD2", "FADD3", "FMULT1", "FMULT2", "FMULT3", "FMULT4", "FDIV1", "FDIV2", "BU"};
    vector<string> int_unit = {"INT1", "INT2", "INT3", "INT4"};
    vector<string> load_unit = {"LOAD1", "LOAD2"};
    vector<string> store_unit = {"STORE1", "STORE2"};
    vector<string> fadd_unit = {"FADD1", "FADD2", "FADD3"};
    vector<string> fmul_unit = {"FMULT1", "FMULT2", "FMULT3", "FMULT4"};
    vector<string> fdiv_unit = {"FDIV1", "FDIV2"};
    string bu_unit = "BU";

    deque<Instruction> instruction_list;         // store instruction read from inputfile
    deque<Instruction> fetch_queue;              // store fetched insrtuctions
    deque<Instruction> decode_queue;             // store decoded instructions
    unordered_map<string, string> mapping_table; // key is the register before rename, value is after renamed
    unordered_map<string, int> branch_address;   // store the address of branch instruction
    unordered_map<string, Reservation_station_status> reservation_stations;
    unordered_map<int, pair<int, int>> BTB;
    deque<ROB_status> ROB;
    unordered_map<string, float> CDB;
    unordered_map<string, float> register_status; // register result status
    deque<string> free_list;                      // Free list for avaliable physical registers
    unordered_map<int, int> memory_content;       // store memory values that read from inputfile
    float physical_mem[SIZE_MEM];
    int NF, NW, NB, NR;
    int PC = 0;
    int address = 0;
    int cycles = 0;
    int ROB_head = 0; // is ROB head
    bool mem_busy = false;
    int count_WB = 0;

public:
    Simulator();  // constructor
    ~Simulator(); // destructor

    void initlize();                                    // start initialization
    int init_mem();                                     // Initialize memory;
    int init_reservationStation();                      // Initialize Reservation Sta;
    int init_ROB();                                     // Initialize ROB
    int init_register();                                // Initialize registers
    void sim_start();                                   // start the simulator;
    void set_parameter(int NF, int NW, int NR, int NB); // Set parameters
    bool read_memory(const char *);                     // read memory content from file
    bool read_instructions(const char *);               // read instruction from file
    bool fetch();                                       // Fetch instructions from instruction list;
    bool decode();                                      // Decode instructions from fetch queue
    bool issue();                                       // issue instructions from decode queue to reservation stations
    bool execute();
    double getValue(ROB_status rob);
    string register_rename(string reg, bool des); // perform register rename at decode stage and add renamed instruction into the decode deque.

    /*Debug purpose*/
    void print_ins_list();
    void print_fetch_list();
    void print_mem_list();
    void print_rename_list();
    void print_reservationStation();
    void print_freelist();
    void print_ROB();
    void print_registerStatus();
    void print_CDB();
    void display_data();
};
#endif