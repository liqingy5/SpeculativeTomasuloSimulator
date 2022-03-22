#include "simulator.h"
#include <fstream>
#include <iostream>
#include <string.h>
#include <cstdlib>

const int DEBUG = 0;
const char *delimiter = ", ";
const char *delim_left_parentheses = "(";
const char MAX_INS_LENGTH = 50;
vector<string> ins_name = {"fld", "fsd", "add", "addi", "fadd", "fsub", "fmul", "fdiv", "bne"};

Simulator::Simulator()
{
    init_mem();
}
Simulator::~Simulator()
{
}

void Simulator::set_parameter(int _NF, int _NW, int _NB, int _NR)
{
    NF = _NF;
    NW = _NW;
    NB = _NB;
    NR = _NR;
}

void Simulator::sim_start()
{
    int i = 0;
    while (i < 30)
    {
        i++;
        decode();
        fetch();
    }
}

int Simulator::init_mem()
{
    for (int i = 0; i < SIZE_MEM; ++i)
    {
        physical_mem[i] = 0;
        if (i > 0)
        {
            free_list.push_back(i);
        }
    }

    return 0;
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
    if (fetch_queue.empty())
        return true;
    while (!fetch_queue.empty())
    {
        Instruction temp_ins = fetch_queue.front();
        fetch_queue.pop_front();
        decode_queue.push_back(temp_ins);
        if (temp_ins.Op == BNE && BTB.count(temp_ins.address) == 0)
        {
            BTB[temp_ins.address] = {branch_address[temp_ins.rt],
                                     0};
        }
    }

    // string &rd = temp_ins.rd;
    // string &rs = temp_ins.rs;
    // string &rt = temp_ins.rt;
    // int &imme = temp_ins.imme;
    // switch (temp_ins.Op)
    // {
    // case FMUL:
    // case ADD:
    // case FDIV:
    // case FSUB:
    //     if (!register_rename(rd))
    //     {
    //         return false;
    //     }
    //     if (!register_rename(rs))
    //     {
    //         return false;
    //     }
    //     if (!register_rename(rt))
    //     {
    //         return false;
    //     }
    //     break;
    // default:
    //     if (!register_rename(rd))
    //     {
    //         return false;
    //     }
    //     if (!register_rename(rs))
    //     {
    //         return false;
    //     }
    // }
    return true;
}

bool Simulator::issue()
{
    return true;
}

bool Simulator::register_rename(string &reg)
{
    cout << reg << endl;
    if (mapping_table.count(reg) == 0)
    {
        if (free_list.empty())
            return false;
        if (reg == "$0")
        {
            mapping_table[reg] = 0;
        }
        else
        {
            int phy_mem = free_list.front();
            free_list.pop_front();
            mapping_table[reg] = phy_mem;
        }
    }
    return true;
}

void Simulator::print_ins_list()
{
    for (auto i : instruction_list)
    {
        Instruction ins = i;
        cout << ins_name[ins.Op] << " " << ins.rd << "," << ins.rs << "," << ins.rt << ins.imme << endl;
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
    for (auto i : instruction_list)
    {
        Instruction ins = i;
        if (ins_name[ins.Op] == "bne")
        {
            cout << ins_name[ins.Op] << " " << mapping_table[ins.rd] << "," << mapping_table[ins.rs] << "," << ins.rt << endl;
            continue;
        }
        int end = mapping_table[ins.rt] ? mapping_table.count(ins.rt) == 1 : -1;
        cout << ins_name[ins.Op] << " " << mapping_table[ins.rd] << "," << mapping_table[ins.rs] << "," << end << " " << ins.imme << endl;
    }
}