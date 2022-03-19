#include "simulator.h"
#include <fstream>
#include <iostream>
#include <string.h>
#include <cstdlib>

const int DEBUG = 1;
const char *delimiter = ", ";
const char *delim_left_parentheses = "(";
const char MAX_INS_LENGTH = 50;

Simulator::Simulator()
{
}
Simulator::~Simulator()
{
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
            int rs_loc, rs_val;
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

            if (words.size() <= 3)
            {
                rs_loc = atoi(words[0]);
                rs_val = atoi(words[1]);
                if (DEBUG)
                {
                    cout << "Register location: " << rs_loc << " "
                         << "Register value: " << rs_val << endl;
                }
                continue;
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
                        rt = rs.substr(0, i);
                        rs = rs.substr(i + 1, rs.size() - rt.size() - 2);
                        break;
                    }
                }
            }
            else if (op == "bne")
            {
                rd = words[1]; // first source
                rs = words[2]; // second source
                rt = words[3]; // jump
                opcode = BNE;
            }
            else if (op == "addi")
            {
                rd = words[1]; // destination
                rs = words[2]; // source
                rt = words[3]; // immediate
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
            ins.Op = opcode;
            ins.label = branch_name;
            ins.rd = rd;
            ins.rs = rs;
            ins.rt = rt;
            instruction_list.push_back(ins);
        }
        myfile.close();
        return true;
    }

    else
        cout << "Unable to open file";

    return false;
}