#include <fstream>
#include <iostream>
#include <map>
#include <string>

using namespace std;

enum Flags {
    Number,
    Register,
    Count
};

char Flags[Flags::Count] = { Number, Register };

enum Registry {
    R_AX = 1,
    R_BX,
    R_CX,
    R_PC,
    R_COND,
    COUNT
};

enum Command {
    MOV = 1,
    ADD,
    SUB,
    MUL,
    JMP,
    JN,
    JP,
    JE
};

std::map<string, Command> commAssoc = { { "mov", MOV }, { "add", ADD } };
std::map<string, Registry> regAssoc = { { "ax", R_AX }, { "bx", R_BX } };

uint8_t memory[1024];
uint8_t reg[Registry::COUNT];

void compile(const char* inputFileName, const char* outputFileName);

int main()
{
    compile("input.vms", "output.vmb");

    return 0;
}

void writeBinary(ofstream& file, const char* data)
{
    file.write(data, sizeof(char));
}

template <typename T>
const char* numToCharP(T& val)
{
    return (const char*)&val;
}

void compile(const char* inputFileName, const char* outputFileName)
{
    string result;
    ifstream input(inputFileName);
    ofstream output(outputFileName, std::ios::out | std::ios::binary);

    while (getline(input, result)) {
        for (size_t i = 0; i < result.length(); ++i) {
            if (isspace(result[i])) {
                string opName = result.substr(0, i);
                writeBinary(output, numToCharP(commAssoc[opName]));

                result = result.substr(i + 1, result.size());
                i = 0;
                continue;
            }
            if (result[i] == ',') {
                string leftOP = result.substr(0, i);
                string rightOP = result.substr(i + 2, result.size());

                const char* leftOpData = numToCharP(regAssoc[leftOP]);
                const char* rightOpData;
                const char* flag;

                if (rightOP == "ax" || rightOP == "bx" || rightOP == "cx") {
                    flag = numToCharP(Flags[Register]);
                    rightOpData = numToCharP(regAssoc[rightOP]);
                } else {
                    flag = numToCharP(Flags[Number]);
                    int val = std::stoi(rightOP);
                    rightOpData = numToCharP(val);
                }

                writeBinary(output, flag);
                writeBinary(output, leftOpData);
                writeBinary(output, rightOpData);

                break;
            }
        }
    }

    input.close();
    output.close();
}
