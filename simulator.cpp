#include "simulator.h"
#include <fstream>
#include <iostream>
#include <string.h>
#include <cstdlib>
#include <iomanip>

const int DEBUG = 0;
const char *delimiter = ", ";
const char *delim_left_parentheses = "(";
const char MAX_INS_LENGTH = 50;
vector<string> ins_name = {"fld", "fsd", "add", "addi", "fadd", "fsub", "fmul", "fdiv", "bne"};
vector<string> state_name = {"Issued",
                             "Execute",
                             "WB",
                             "Commit"};
Simulator::Simulator()
{
}
Simulator::~Simulator()
{
}
void Simulator::initlize()
{
    init_mem();
    init_reservationStation();
    init_ROB();
    init_register();
}
void Simulator::set_parameter(int _NF, int _NW, int _NR, int _NB)
{
    NF = _NF;
    NW = _NW;
    NR = _NR;
    NB = _NB;
}

void Simulator::sim_start()
{
    int i = 0;
    while (i < 30)
    {
        i++;
        issue();
        decode();
        fetch();
    }
}

int Simulator::init_mem()
{
    for (int i = 0; i < SIZE_MEM; ++i)
    {
        physical_mem[i] = 0;
        free_list.push_back("p" + to_string(i));
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

int Simulator::init_ROB()
{
    ROB_status rob;
    for (int i = 0; i < NR; ++i)
    {
        string index = "ROB" + to_string(i + 1);
        rob.name = index;
        ROB.push_back(rob);
    }

    return 0;
}

int Simulator::init_register()
{
    for (int i = 0; i < SIZE_MEM; ++i)
    {
        register_status["P" + to_string(i)] = -1;
    }
}

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
            if (op == "fld" || op == "fsd")
            {
                if (op == "fld")
                {
                    opcode = FLD;
                }
                else
                {
                    opcode = FSD;
                }
                rd = words[1]; // Destination or base register
                rs = words[2]; // first source or data source
                // extract offset
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
            else if (op == "bne")
            {
                rd = words[1]; // first source
                rs = words[2]; // second source
                rt = words[3]; // jump
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
            struct Instruction ins;
            ins.address = address;
            ins.Op = opcode;
            ins.rd = rd;
            ins.rs = rs;
            ins.rt = rt;
            ins.imme = imme;
            if (branch_name.length() > 0)
            {
                branch_address[branch_name] = address;
            }
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
bool Simulator::fetch()
{
    cycles++;
    if (instruction_list.empty())
        return false;
    for (int i = 0; i < NF; ++i)
    {
        if (PC >= instruction_list.size())
        {
            return false;
        }
        Instruction &temp = instruction_list[PC];
        fetch_queue.push_back(instruction_list[PC]);
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
bool Simulator::decode()
{
    cycles++;
    if (fetch_queue.empty())
        return true;
    while (!fetch_queue.empty())
    {
        Instruction temp_ins = fetch_queue.front();
        string rd = temp_ins.rd;
        string rs = temp_ins.rs;
        string rt = temp_ins.rt;
        int &imme = temp_ins.imme;
        string temp;
        switch (temp_ins.Op)
        {
        case FMUL:
        case FADD:
        case ADD:
        case FDIV:
        case FSUB:
            temp = register_rename(rd, true);
            if (temp == "")
            {
                return false;
            }
            temp_ins.rd = temp;
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
        case FLD:
            temp = register_rename(rd, false);
            if (temp == "")
            {
                return false;
            }
            temp_ins.rd = temp;
            temp = register_rename(rs, false);
            if (temp == "")
            {
                return false;
            }
            temp_ins.rs = temp;

        default:
            temp = register_rename(rd, true);
            if (temp == "")
            {
                return false;
            }
            temp_ins.rd = temp;
            temp = register_rename(rs, false);
            if (temp == "")
            {
                return false;
            }
            temp_ins.rs = temp;
        }
        fetch_queue.pop_front();
        decode_queue.push_back(temp_ins);
        if (temp_ins.Op == BNE && BTB.count(temp_ins.address) == 0)
        {
            BTB[temp_ins.address] = {branch_address[temp_ins.rt],
                                     0};
        }
    }
    return true;
}

bool Simulator::issue()
{

    cycles++;
    for (int w = 0; w < NW; ++w)
    {

        Instruction temp;
        string unit = "";
        if (decode_queue.empty())
            return false;
        if (current_ROB == NR)
            return false;
        temp = decode_queue.front();
        decode_queue.pop_front();

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

        if (unit == "")
            return false;
        reservation_stations[unit].busy = true;
        reservation_stations[unit].Op = temp.Op;

        for (int i = 0; i < NR; ++i)
        {
            ROB_status &rob = ROB[i];
            if (rob.busy == false)
            {
                rob.ins = temp;
                rob.state = Issued;
                rob.dest = temp.rd;
                rob.busy = true;
                reservation_stations[unit].dest = rob.name;

                break;
            }
        }

        if (temp.rs != "")
        {
            reservation_stations[unit].Vj = temp.rs;
            for (int i = NR - 1; i >= 0; i--)
            {
                ROB_status rob = ROB[i];
                if (rob.dest == temp.rs && rob.state != Commit && rob.state != WB)
                {
                    reservation_stations[unit].Qj = i;
                    break;
                }
            }
        }
        if (temp.rt != "")
        {
            reservation_stations[unit].Vk = temp.rt;
            for (int i = NR - 1; i >= 0; i--)
            {
                ROB_status rob = ROB[i];
                if (rob.dest == temp.rt && rob.state != Commit && rob.state != WB)
                {
                    reservation_stations[unit].Qk = i;
                    break;
                }
            }
        }

        if (temp.Op == ADDI || temp.Op == FLD || temp.Op == FSD)
        {
            reservation_stations[unit].a = temp.imme;
        }
    }
    return true;
}

string Simulator::register_rename(string reg, bool des)
{
    // cout << "Was: " << reg << endl;
    if (mapping_table.count(reg) == 1)
    {
        if (!des)
        {
            string temp = mapping_table[reg];
            reg = temp;
            // cout << reg << endl;
            return reg;
        }
        else
        {
            reg = mapping_table[reg];
        }
    }
    if (free_list.empty())
        return "";
    if (reg == "R0")
    {
        return "R0";
    }
    else
    {
        string free_register = free_list.front();
        free_list.pop_front();
        mapping_table[reg] = free_register;
        reg = free_register;
    }
    // cout << reg << endl;

    return reg;
}

void Simulator::print_ins_list()
{
    for (auto i : instruction_list)
    {
        Instruction ins = i;
        cout << ins_name[ins.Op] << " " << ins.rd << "," << ins.rs << "," << ins.rt << ins.imme << endl;
        // if (ins.rt == "")
        // {
        //     cout << ins_name[ins.Op] << endl;
        // }
    }
}
void Simulator::print_mem_list()
{
    for (auto i : memory_content)
    {
        cout << i.first << "," << i.second << endl;
    }
}
void Simulator::print_rename_list()
{
    for (auto i : decode_queue)
    {
        Instruction ins = i;

        cout << ins_name[ins.Op] << " " << ins.rd << "," << ins.rs << "," << ins.rt << " " << ins.imme << endl;
    }
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
            if (temp.Qj != -1)
            {
                qj = "ROB" + to_string(temp.Qj);
            }
            if (temp.Qk != -1)
            {
                qk = "ROB" + to_string(temp.Qk);
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
    for (auto f : free_list)
    {
        cout << f << endl;
    }
}

void Simulator::print_ROB()
{
    cout << setw(30) << "ROB" << endl;

    cout << setw(8) << "Name" << setw(8)
         << "Busy" << setw(20)
         << "Instruction" << setw(8)
         << "State" << setw(8)
         << "Dest" << setw(8)
         << "Value" << endl;
    for (auto rob : ROB)
    {
        string name = rob.name;
        string busy = "False";
        string instruction = "";
        string state = "";
        string dest = "";
        string value = "";
        if (rob.busy == true)
        {
            busy = "True";
            instruction = ins_name[rob.ins.Op] + " " + rob.ins.rd + "," + rob.ins.rs + "," + rob.ins.rt + " " + to_string(rob.ins.imme);
            state = state_name[rob.state];
            dest = rob.dest;
            if (rob.value != -1)
            {
                value = to_string(rob.value);
            }
        }
        cout << setw(8) << rob.name << setw(8) << busy << setw(20) << instruction << setw(8) << state << setw(8) << dest << setw(8) << value << endl;
    }
}

void Simulator::print_registerStatus()
{
    for (auto reg : register_status)
    {
        cout << reg.first << endl;
    }
}