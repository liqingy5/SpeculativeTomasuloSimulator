#include "simulator.h"
#include <fstream>
#include <iostream>
#include <string.h>
#include <cstdlib>
#include <iomanip>

const int DEBUG = 1;
const char *delimiter = ", ";
const char *delim_left_parentheses = "(";
const char MAX_INS_LENGTH = 50;
vector<string> ins_name = {"fld", "fsd", "add", "addi", "fadd", "fsub", "fmul", "fdiv", "bne"};
vector<string> state_name = {"Issued",
                             "Execute",
                             "Memory",
                             "WB",
                             "Commit"};
Simulator::Simulator()
{
}
Simulator::~Simulator()
{
}
/*
 * Initlize the simulator.
 */
void Simulator::initlize()
{
    init_mem();
    init_reservationStation();
    // init_ROB();
    init_register();
    mapping_history = deque<unordered_map<string, string>>();
    free_list_history = deque<deque<string>>();
    cout << endl;
}
/*
 * set_parameter
 * set the parameters of the simulator
 * @param NF: number of integer functional units
 * @param NW: number of load functional units
 * @param NR: number of register file
 * @param NB: number of branch units
 */
void Simulator::set_parameter(int _NF, int _NW, int _NR, int _NB)
{
    NF = _NF;
    NW = _NW;
    NR = _NR;
    NB = _NB;
}
/*
 * start the simulator
 */
void Simulator::sim_start()
{
    int cycles = 0;
    while (true)
    {
        cycles++;
        if (DEBUG)
        {
            display_data();
        }
        execute();
        issue();
        decode();
        fetch();
        /// Stop simulation when no instructions in fly and ROB is empty
        if (ROB.size() == 0 && fetch_queue.size() == 0 && decode_queue.size() == 0)
        {
            display_data();
            cout << "Cycles: " << cycles << endl;
            break;
        }
    }
}

int Simulator::init_mem()
{
    for (int i = 0; i < SIZE_MEM; ++i)
    {
        physical_mem[i] = 0;
        free_list.push_back("P" + to_string(i));
    }

    return 0;
}

int Simulator::init_reservationStation()
{
    Reservation_station_status temp;
    for (auto u : unit)
    {
        reservation_stations[u] = temp;
    }
    return 0;
}

// int Simulator::init_ROB()
// {
//     ROB_status rob;
//     for (int i = 0; i < NR; ++i)
//     {
//         string index = "ROB" + to_string(i + 1);
//         rob.name = index;
//         ROB.push_back(rob);
//     }

//     return 0;
// }

int Simulator::init_register()
{
    for (int i = 0; i < SIZE_MEM; ++i)
    {
        register_status["P" + to_string(i)] = 0;
    }
}
/*
 * read memory
 * read the memory value from the mem.dat
 * @param filename: the file name of the memory
 */
bool Simulator::read_memory(const char *inputfile)
{
    filename = inputfile;
    ifstream myfile(filename);
    char line[MAX_INS_LENGTH];
    // read input file
    if (myfile.is_open())
    {
        // tokenize the input
        while (myfile.getline(line, MAX_INS_LENGTH))
        {
            char *word;
            vector<char *> words;
            int loc, val;
            word = strtok(line, delimiter);
            while (word)
            {
                words.push_back(word);
                word = strtok(NULL, delimiter);
            }
            loc = atoi(words[0]);
            val = atoi(words[1]);
            if (DEBUG)
            {
                cout << "memory location: " << loc << " "
                     << "memory value: " << val << endl;
            }
            // push value to the memory content
            memory_content[loc] = val;
        }
    }
    else
    {
        cout << "Unable to open memory file" << endl;
        exit(1);
    }
    return true;
}
/*
 * read instruction
 * read the instruction from the instr.dat
 * @param filename: the file name of the instructions
 */
bool Simulator::read_instructions(const char *inputfile)
{
    filename = inputfile;
    ifstream myfile(filename);
    char line[MAX_INS_LENGTH];
    // read input file
    if (myfile.is_open())
    {
        // tokenize the input
        while (myfile.getline(line, MAX_INS_LENGTH))
        {
            char *word;
            vector<char *> words;
            Instrs opcode;
            string token, branch_name, op, rd, rs, rt;
            int imme = 0;
            word = strtok(line, delimiter);
            while (word)
            {
                words.push_back(word);
                word = strtok(NULL, delimiter);
            }
            token = words[0];
            // check if instruction is the start of the branch.
            if (token[token.size() - 1] == ':')
            {
                branch_name = token.substr(0, token.size() - 1);
                words.erase(words.begin(), words.begin() + 1);
            }
            op = words[0];

            if (op == "fld")
            {
                opcode = FLD;
                rd = words[1]; // Destination
                rs = words[2]; // data source
                if (rs[rs.size() - 1] == '\r')
                {
                    rs.erase(rs.size() - 1);
                }
                for (size_t i = 0; i < rs.size(); ++i)
                {
                    if (rs[i] == '(')
                    {
                        imme = stoi(rs.substr(0, i));
                        rs = rs.substr(i + 1, rs.size() - i - 2);
                        break;
                    }
                }
            }
            else if (op == "fsd")
            {
                opcode = FSD;
                rs = words[1]; // base register
                rt = words[2]; // memory location
                if (rt[rt.size() - 1] == '\r')
                {
                    rt.erase(rt.size() - 1);
                }
                for (size_t i = 0; i < rt.size(); ++i)
                {
                    if (rt[i] == '(')
                    {
                        imme = stoi(rt.substr(0, i));
                        rt = rt.substr(i + 1, rt.size() - i - 2);
                        break;
                    }
                }
            }
            else if (op == "bne")
            {

                rs = words[1]; // first source
                rt = words[2]; // second source
                rd = words[3]; // jump to
                if (rt[rt.size() - 1] == '\r')
                {
                    rt.erase(rt.size() - 1);
                }
                // if (DEBUG && branch_table.count(rt))
                // {
                //     cout << branch_table[rt].rd << endl;
                // }
                opcode = BNE;
            }
            else if (op == "addi")
            {
                rd = words[1];         // destination
                rs = words[2];         // source
                imme = stoi(words[3]); // immediate
                opcode = ADDI;
            }
            else
            {
                rd = words[1]; // destination
                rs = words[2]; // source 1
                rt = words[3]; // source 2
                if (rt[rt.size() - 1] == '\r')
                {
                    rt.erase(rt.size() - 1);
                }
                if (op == "fadd")
                {
                    opcode = FADD;
                }
                else if (op == "fsub")
                {
                    opcode = FSUB;
                }
                else if (op == "fmul")
                {
                    opcode = FMUL;
                }
                else if (op == "fdiv")
                {
                    opcode = FDIV;
                }
                else
                {
                    opcode = ADD;
                }
            }
            // Initiaze the instruction to instruction struct
            struct Instruction ins;
            ins.address = address;
            ins.Op = opcode;
            ins.rd = rd;
            ins.rs = rs;
            ins.rt = rt;
            ins.imme = imme;
            // add branch name to the branch table
            if (branch_name.length() > 0)
            {
                branch_address[branch_name] = address;
            }
            // save instructions to the instruction_list
            instruction_list.push_back(ins);
            address++;
        }
        myfile.close();
        return true;
    }

    else
        cout << "Unable to open file";

    return false;
}
/*
 * Fetch stage
 */
bool Simulator::fetch()
{
    // return false if the instruction list is emtpy
    if (instruction_list.empty())
        return false;
    // iteration the instruction list and fetch no more than NF instructions each time.
    for (int i = 0; i < NF; ++i)
    {
        // Programe counter larger than the instruction list return false
        if (PC >= instruction_list.size())
        {
            return false;
        }
        // fetch the instruction
        Instruction &temp = instruction_list[PC];
        fetch_queue.push_back(instruction_list[PC]);
        // if the instruction is a branch instruction, check if the branch is taken
        // if taken, set the PC to the branch address
        if (temp.Op == BNE && BTB.count(temp.address))
        {
            if (BTB[temp.address].second)
            {
                PC = BTB[temp.address].first;
            }
            else
            {
                PC += 1;
            }
        }
        else
        {
            PC += 1;
        }
    }
    return true;
}
/*
 * Decode stage
 */
bool Simulator::decode()
{
    // return false if the fetch queue is empty
    if (fetch_queue.empty())
        return true;
    while (!fetch_queue.empty())
    {
        // get the instruction from the head of the fetch queue
        Instruction temp_ins = fetch_queue.front();
        string rd = temp_ins.rd;
        string rs = temp_ins.rs;
        string rt = temp_ins.rt;
        int &imme = temp_ins.imme;
        string temp;
        // do the register renaming base on the opcode
        switch (temp_ins.Op)
        {
        case FMUL:
        case FADD:
        case ADD:
        case FDIV:
        case FSUB:
            temp = register_rename(rt, false);
            if (temp == "")
            {
                return false;
            }
            temp_ins.rt = temp;
            temp = register_rename(rs, false);
            if (temp == "")
            {
                return false;
            }
            temp_ins.rs = temp;
            temp = register_rename(rd, true);
            if (temp == "")
            {
                return false;
            }
            temp_ins.rd = temp;
            break;
        case FLD:
            temp = register_rename(rs, false);
            if (temp == "")
            {
                return false;
            }
            temp_ins.rs = temp;
            temp = register_rename(rd, true);
            if (temp == "")
            {
                return false;
            }
            temp_ins.rd = temp;
            break;
        case FSD:
            temp = register_rename(rt, false);
            if (temp == "")
            {
                return false;
            }
            temp_ins.rt = temp;
            temp = register_rename(rs, false);
            if (temp == "")
            {
                return false;
            }
            temp_ins.rs = temp;
            break;
        case BNE:
            temp = register_rename(rs, false);
            if (temp == "")
            {
                return false;
            }
            temp_ins.rs = temp;
            temp = register_rename(rt, false);
            if (temp == "")
            {
                return false;
            }
            temp_ins.rt = temp;
            break;
        default:
            temp = register_rename(rs, false);
            if (temp == "")
            {
                return false;
            }
            temp_ins.rs = temp;
            temp = register_rename(rd, true);
            if (temp == "")
            {
                return false;
            }
            temp_ins.rd = temp;
            break;
        }
        // Push renamed instruciton to the decode queue
        fetch_queue.pop_front();
        decode_queue.push_back(temp_ins);
        // initialize the BTB
        if (temp_ins.Op == BNE)
        {
            branchHistory();

            if (BTB.count(temp_ins.address) == 0)
            {
                BTB[temp_ins.address] = {branch_address[temp_ins.rd],
                                         0};
            }
        }
    }
    return true;
}
/*
 * Issue stage
 */
bool Simulator::issue()
{
    // issue the instructions by NW times
    for (int w = 0; w < NW; ++w)
    {

        Instruction temp;
        string unit = "";
        // return if decode queue is empty
        if (decode_queue.empty())
            return false;
        // return if ROB if full
        if (ROB.size() == NR)
        {

            ++stalled_cycles;
            return false;
        }
        // get the instruction from the head of the decode queue
        temp = decode_queue.front();
        decode_queue.pop_front();
        // put instruction into the reservation station
        switch (temp.Op)
        {
        case ADD:
        case ADDI:
            for (auto i : int_unit)
            {
                if (reservation_stations[i].busy == false)
                {
                    unit = i;
                    break;
                }
            }
            break;
        case FLD:
            for (auto i : load_unit)
            {
                if (reservation_stations[i].busy == false)
                {
                    unit = i;
                    break;
                }
            }
            break;
        case FSD:
            for (auto i : store_unit)
            {
                if (reservation_stations[i].busy == false)
                {
                    unit = i;
                    break;
                }
            }
            break;
        case FADD:
        case FSUB:
            for (auto i : fadd_unit)
            {
                if (reservation_stations[i].busy == false)
                {
                    unit = i;
                    break;
                }
            }
            break;

        case FMUL:
            for (auto i : fmul_unit)
            {
                if (reservation_stations[i].busy == false)
                {
                    unit = i;
                    break;
                }
            }
            break;
        case FDIV:
            for (auto i : fdiv_unit)
            {
                if (reservation_stations[i].busy == false)
                {
                    unit = i;
                    break;
                }
            }
            break;
        case BNE:

            if (reservation_stations[bu_unit].busy == false)
            {
                unit = bu_unit;
                break;
            }

            break;
        default:
            return false;
            break;
        }
        // if ALU in reservation station is full, stall
        if (unit == "")
        {
            decode_queue.push_front(temp);
            ++stalled_cycles;
            return false;
        }

        // set the reservation station to be busy
        reservation_stations[unit].busy = true;
        reservation_stations[unit].Op = temp.Op;
        // initialize the ROB
        ROB_status rob;
        rob.unit = unit;
        rob.ins = temp;
        rob.state = Issued;
        rob.dest = temp.rd;
        rob.busy = true;
        rob.name = "ROB" + to_string(ROB_head + 1);
        reservation_stations[unit].dest = rob.name;

        // fill Qj and Qv with ROB dependcy
        if (temp.rs != "")
        {
            reservation_stations[unit].Vj = temp.rs;
            for (int i = ROB.size() - 1; i >= 0; i--)
            {
                ROB_status rob = ROB[i];
                if (rob.dest == temp.rs && rob.state != Commit && rob.state != WB)
                {
                    reservation_stations[unit].Qj = rob.name;
                    break;
                }
            }
        }
        if (temp.rt != "")
        {
            reservation_stations[unit].Vk = temp.rt;
            for (int i = ROB.size() - 1; i >= 0; i--)
            {
                ROB_status rob = ROB[i];
                if (rob.dest == temp.rt && rob.state != Commit && rob.state != WB)
                {
                    reservation_stations[unit].Qk = rob.name;
                    break;
                }
            }
        }
        // add imme value
        if (temp.Op == ADDI || temp.Op == FLD || temp.Op == FSD)
        {
            reservation_stations[unit].a = temp.imme;
        }
        // push to the ROB queue
        ROB.push_back(rob);
        ROB_head++;
        if (ROB_head == NR)
        {
            ROB_head = 0;
        }
    }
    return true;
}
/*
 * Retuen latency for different operation
 */
int latency(Instrs op)
{
    switch (op)
    {
    case ADD:
    case ADDI:
        return INT_LATENCY;
    case FLD:
    case FSD:
        return LD_ST_LATENCY;
    case FADD:
    case FSUB:
        return FADD_LATENCY;
    case FMUL:
        return FMULT_LATENCY;
    case FDIV:
        return FDIV_LATENCY;
    case BNE:
        return BU_LATENCY;
    default:
        return 0;
    }
}
bool Simulator::execute()
{
    count_WB = 0;
    mem_busy = false;
    // read ROB instruction
    for (auto &i : ROB)
    {
        // if commit, continue and wait for remove
        if (i.state == Commit)
        {
            // if (CDB.size() != NB)
            // {
            //     if (i.ins.Op != FSD && i.ins.Op != BNE)
            //     {
            //         if (CDB.count(i.dest) == 0)
            //         {
            //             CDB[i.dest] = i.value;
            //         }
            //     }
            // }
            continue;
        }
        if (i.state == WB)
        {
            i.state = Commit;
            // if (i.ins.Op == FSD || i.ins.Op == FLD)
            // {
            //     mem_busy = false;
            // }
        }
        // if wait cycles is 0 and the state is execute
        else if (i.cycles == 0 && i.state == Execute)
        {
            if ((i.ins.Op == FSD || i.ins.Op == FLD) && i.cycles == 0)
            {
                if (!mem_busy)
                {
                    // set memory to busy if load/ store
                    mem_busy = true;
                }
                else
                {
                    continue;
                }
            }
            if (count_WB != NB)
            {
                // count how many instruction in WB in this cycle.
                i.state = WB;
                count_WB += 1;
            }
            else
            {
                continue;
            }
            // if the value of instruction is calculated
            if (i.value != __DBL_MIN__)
            {

                if (i.ins.Op == BNE)
                {

                    PC = i.ins.address + 1;
                    // if take the branch, reset the PC
                    if (i.value == 1)
                    {
                        PC = BTB[i.ins.address].first;
                        if (!mapping_history.empty())
                        {
                            mapping_history.pop_front();
                        }
                        if (!free_list_history.empty())
                        {
                            free_list_history.pop_front();
                        }
                    }
                    // flush out the instruction after the branch if the branch predict is wrong
                    if (BTB[i.ins.address].second != i.value)
                    {
                        if (i.value == 0)
                        {
                            cout << endl;
                        }
                        BTB[i.ins.address].second = i.value;

                        // flush out the fetch and decode
                        fetch_queue.clear();
                        decode_queue.clear();
                        historyReset();

                        // flush out the ROB.
                        for (int ind = ROB.size() - 1; ind >= 0; ind--)
                        {
                            ROB_status f = ROB[ind];
                            if (f.ins.address != i.ins.address)
                            {
                                ROB.pop_back();
                                if (reservation_stations.count(f.unit))
                                {
                                    Reservation_station_status r;
                                    reservation_stations[f.unit] = r;
                                }
                                continue;
                            }
                            else
                            {
                                ROB_head = stoi(i.name.substr(3));
                                if (ROB_head == NR)
                                {
                                    ROB_head = 0;
                                }
                                return false;
                            }
                        }
                    }
                }
                else
                {
                    // if (CDB.size() != NB)
                    // {
                    //     if (i.ins.Op != FSD)
                    //     {
                    //         CDB[i.dest] = i.value;
                    //     }
                    // }

                    // add to CDB
                    if (i.ins.Op != FSD)
                    {
                        CDB[i.dest] = i.value;
                    }
                    else
                    {
                        return false;
                    }
                }
            }
            // i.value is ready, set the Vj and Vk of the instructions that use it as operand to that destination
            for (auto &re : reservation_stations)
            {
                if (re.second.Qj == i.name)
                {
                    re.second.Vj = i.dest;
                    re.second.Qj = "";
                }
                if (re.second.Qk == i.name)
                {
                    re.second.Vk = i.dest;
                    re.second.Qk = "";
                }
            }
        }
        else if (i.state == Issued)
        { // if the operand is ready in reservation station, get the value from CDB or other registers.
            if (reservation_stations[i.unit].Qj == "" && reservation_stations[i.unit].Qk == "")
            {

                i.cycles = latency(i.ins.Op);
                i.cycles--;
                i.value = getValue(i);
                i.state = Execute;
            }
            else
            {
                // if not ready in reservation station, check the ROB to see if the operand is ready.
                if (reservation_stations[i.unit].Qj != "")
                {
                    string rob_name = reservation_stations[i.unit].Qj;
                    for (auto r : ROB)
                    {
                        if (rob_name == r.name && (r.state == WB || r.state == Commit))
                        {
                            reservation_stations[i.unit].Qj = "";
                            break;
                        }
                    }
                }
                if (reservation_stations[i.unit].Qk != "")
                {
                    string rob_name = reservation_stations[i.unit].Qk;
                    for (auto r : ROB)
                    {
                        if (rob_name == r.name && (r.state == WB || r.state == Commit))
                        {
                            reservation_stations[i.unit].Qk = "";
                            break;
                        }
                    }
                }
            }
        }
        else
        {
            // decrement the wait cycles
            if (i.cycles > 0)
            {
                i.cycles -= 1;
            }
        }
    }
    if (ROB.size() > 0)
    {
        // if the head of ROB is in Commit, store the value to register or memory
        if (ROB.front().state == Commit)
        {
            ROB_status temp = ROB.front();
            Reservation_station_status new_temp;
            ROB.pop_front();
            reservation_stations[temp.unit] = new_temp;
            switch (temp.ins.Op)
            {
            case FSD:
            {
                double value = __DBL_MIN__;
                if (CDB.count(temp.ins.rs))
                {
                    value = CDB[temp.ins.rs];
                    // free the renamed address
                    reset_address(temp.ins.rs);
                    // erase the CDB after commit.
                    CDB.erase(temp.ins.rs);
                }
                else if (register_status.count(temp.ins.rs))
                {
                    value = register_status[temp.ins.rs];
                }
                // save to memory
                memory_content[temp.value] = value;
                break;
            }
            case BNE:
            {
                break;
            }
            default:
            {
                double value = __DBL_MIN__;
                if (CDB.count(temp.ins.rd))
                {
                    value = CDB[temp.ins.rd];
                    // free the renamed address
                    reset_address(temp.ins.rd);
                    // erase the CDB after commit.
                    CDB.erase(temp.ins.rd);
                }
                else
                {
                    value = temp.value;
                }
                // save to register
                register_status[temp.dest] = value;
            }
            break;
            }
        }
    }
    return true;
}
/*
 * @brief: store mapping_table and free list when branch
 */
void Simulator::branchHistory()
{
    mapping_history.push_back(mapping_table);
    free_list_history.push_back(free_list);
}
/*
 * @brief: restore mapping_table and free list when branch not taken
 */
void Simulator::historyReset()
{
    if (!free_list_history.empty())
    {
        free_list = free_list_history.front();
        free_list_history.clear();
    }
    if (!mapping_history.empty())
    {
        mapping_table = mapping_history.front();
        mapping_history.clear();
    }
}

void Simulator::reset_address(string const &addr)
{
    // free_list.push_back(addr);
    // string temp = "";
    // if (mapping_table.count(addr))
    // {
    //     temp = mapping_table[addr];
    // }
    // string key = "";
    // for (auto &k : mapping_table)
    // {
    //     if (k.second == addr)
    //     {
    //         key = k.first;
    //         break;
    //     }
    // }
    // if (temp == "")
    // {
    //     // mapping_table.erase(key);
    // }
    // else
    // {
    //     mapping_table[key] = temp;
    //     mapping_table.erase(addr);
    // }
    free_list.push_back(addr);
    return;
}
/*
 * @brief: get the value of the operand from CDB or register
 * @param: the operand
 * @return: the value of the operand
 */
double Simulator::getValue(ROB_status rob)
{
    Reservation_station_status &rs = reservation_stations[rob.unit];
    Instruction ins = rob.ins;
    double Vj = __DBL_MIN__;
    double Vk = __DBL_MIN__;
    if (CDB.count(rs.Vj) > 0)
    {
        Vj = CDB[rs.Vj];
    }
    else
    {
        Vj = register_status[rs.Vj];
    }

    if (CDB.count(rs.Vk) > 0)
    {
        Vk = CDB[rs.Vk];
    }
    if (Vk == __DBL_MIN__ && ins.Op != ADDI && ins.Op != FLD)
    {
        if (ins.rt == "$0")
        {
            Vk = 0;
        }
        else
        {
            Vk = register_status[ins.rt];
        }
    }

    switch (ins.Op)
    {
    case ADDI:
        return Vj + ins.imme;
    case ADD:
    case FADD:
        return Vj + Vk;
    case FSUB:
        return Vj - Vk;
    case FMUL:
        return Vj * Vk;
    case FDIV:

        return Vj / Vk;
    case FSD:
        return Vk + ins.imme;
    case FLD:
        return memory_content[Vj + ins.imme];
    case BNE:
        return Vj != Vk;
    default:
        return 0;
    }
}
/*
 * register rename
 *@param reg register name
 *@param dest if it's destination register
 *@return register name
 */
string Simulator::register_rename(string reg, bool des)
{
    // cout << "Was: " << reg << endl;
    if (reg == "$0")
    {
        return "$0";
    }
    if (free_list.empty())
        return "";
    if (mapping_table.count(reg) == 1)
    {
        // reg = mapping_table[reg];
        if (!des)
        {
            // string temp = mapping_table[reg];
            // while (mapping_table.count(temp))
            // {
            //     temp = mapping_table[temp];
            // }
            // reg = temp;
            // // cout << reg << endl;
            // return reg;
            return mapping_table[reg];
        }
        // else
        // {
        //     reg = mapping_table[reg];
        // }
    }

    // else
    // {
    //     string free_register = free_list.front();
    //     free_list.pop_front();
    //     mapping_table[reg] = free_register;
    //     reg = free_register;
    // }
    string free_register = free_list.front();
    free_list.pop_front();
    mapping_table[reg] = free_register;
    reg = free_register;
    // cout << reg << endl;

    return reg;
}

/***********************************/
/*functions below are for debug purpose and stats display*/
void Simulator::print_ins_list()

{
    cout << setw(30) << "Instruction list" << endl;
    for (auto i : instruction_list)
    {
        Instruction ins = i;
        cout << ins_name[ins.Op] << " " << ins.rd << "," << ins.rs << "," << ins.rt << " Imme: " << ins.imme << endl;
        // if (ins.rt == "")
        // {
        //     cout << ins_name[ins.Op] << endl;
        // }
    }
    cout << endl;
}
void Simulator::print_mem_list()
{
    cout << setw(30) << "Memory content" << endl;
    cout << setw(8) << "memory" << setw(8)
         << "value" << endl;
    for (auto i : memory_content)
    {
        cout << i.first << "," << setw(8) << i.second << endl;
    }
    cout << endl;
}
void Simulator::print_fetch_list()
{
    cout << setw(30) << "fetch content" << endl;
    for (auto i : fetch_queue)
    {
        Instruction ins = i;

        cout << ins_name[ins.Op] << " " << ins.rd << "," << ins.rs << "," << ins.rt << " " << ins.imme << endl;
    }
    cout << endl;
}
void Simulator::print_rename_list()
{
    cout << setw(30) << "decode content" << endl;
    for (auto i : decode_queue)
    {
        Instruction ins = i;

        cout << ins_name[ins.Op] << " " << ins.rd << "," << ins.rs << "," << ins.rt << " " << ins.imme << endl;
    }
    cout << endl;
}

void Simulator::print_reservationStation()
{
    cout << setw(30) << "Reservation Station" << endl;
    cout << setw(8) << "Name" << setw(8)
         << "Busy" << setw(8)
         << "OP" << setw(8)
         << "Vj" << setw(8)
         << "Vk" << setw(8)
         << "Qj" << setw(8)
         << "Qk" << setw(8)
         << "Dest." << setw(8)
         << "A" << endl;
    for (auto sta : reservation_stations)
    {
        string name = sta.first;
        Reservation_station_status temp = sta.second;
        string busy = "False";
        string op = "";
        string vj = "";
        string vk = "";
        string qj = "";
        string qk = "";
        string dest = "";
        string a = "";
        if (temp.busy == true)
        {
            busy = "True";
            op = ins_name[temp.Op];
            if (temp.Vj != "")
            {
                vj = temp.Vj;
            }
            if (temp.Vk != "")
            {
                vk = temp.Vk;
            }
            if (temp.Qj != "")
            {
                qj = temp.Qj;
            }
            if (temp.Qk != "")
            {
                qk = temp.Qk;
            }
            dest = temp.dest;
            if (temp.a != -1)
            {
                a = to_string(temp.a);
            }
        }
        cout << setw(8) << name << setw(8) << busy << setw(8) << op << setw(8) << vj << setw(8) << vk << setw(8) << qj << setw(8) << qk << setw(8) << dest << setw(8) << a << endl;
    }
    cout << endl;
}

void Simulator::print_freelist()
{
    cout << setw(30) << "FreeList" << endl;
    for (auto f : free_list)
    {
        cout << f << endl;
    }
    cout << endl;
}

void Simulator::print_ROB()
{
    cout << setw(30) << "ROB" << endl;

    cout << setw(8) << "Name" << setw(8)
         << "Busy" << setw(20)
         << "Instruction" << setw(8)
         << "State" << setw(8)
         << "Dest" << setw(8)
         << "Value" << setw(8)
         << "Unit" << endl;
    for (auto rob : ROB)
    {
        string name = rob.name;
        string busy = "False";
        string instruction = "";
        string state = "";
        string dest = "";
        string value = "";
        string unit = "";
        if (rob.busy == true)
        {
            busy = "True";
            instruction = ins_name[rob.ins.Op] + " " + rob.ins.rd + "," + rob.ins.rs + "," + rob.ins.rt + " " + to_string(rob.ins.imme);
            state = state_name[rob.state];
            dest = rob.dest;
            unit = rob.unit;
            if (rob.value != -1)
            {
                value = to_string(rob.value);
            }
        }
        cout << setw(8) << rob.name << setw(8) << busy << setw(20) << instruction << setw(8) << state << setw(8) << dest << " " << setw(8) << value << setw(8) << unit << endl;
    }
    cout << endl;
}

void Simulator::print_registerStatus()
{
    cout << setw(30) << "Register" << endl;
    cout << setw(8) << "reg" << setw(8)
         << "value" << endl;
    for (auto m : mapping_table)
    {
        cout << m.first << "(" << m.second << ") "
             << setw(8) << register_status[m.second] << endl;
    }
    // for (auto reg : register_status)
    // {
    //     cout << reg.first << "," << setw(8) << reg.second << endl;
    // }
    cout << endl;
}

void Simulator::print_CDB()
{
    cout << setw(30) << "CDB" << endl;
    cout << setw(8) << "ROB" << setw(8)
         << "value" << endl;
    for (auto cdb : CDB)
    {
        cout << cdb.first << "," << setw(8) << cdb.second << endl;
    }
    cout << endl;
}

void Simulator::print_stalled()
{
    cout << "stalled cycles: " << stalled_cycles << endl;
}

void Simulator::display_data()
{
    print_ins_list();
    print_fetch_list();
    print_rename_list();
    print_freelist();
    print_reservationStation();
    print_ROB();
    print_CDB();
    print_registerStatus();
    print_mem_list();
    print_stalled();
}