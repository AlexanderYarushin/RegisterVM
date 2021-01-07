#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

using namespace std;

#define SPACE 0x20
#define PROGRAM_END 0xFF

struct Label {
    int adress;
    string name;
    Label(int _adress, string _name)
        : adress(_adress)
        , name(_name) {};
};

vector<Label> labelStore;

enum Sign {
    Equal = 0,
    Positive,
    Negative
};

enum EFlags {
    Number,
    Register,
    FCount
};

char Flags[EFlags::FCount] = { Number, Register };

enum Registry {
    R_AX = 1,
    R_BX,
    R_CX,
    RCount,
    R_PC,
    R_COND,
    RCountAll
};

enum Command {
    MOV = 1,
    ADD,
    SUB,
    MUL,
    DIV,
    JMP,
    JN,
    JP,
    JE,
    CMP,
    LOG,
    Invalid,
    CCount
};

const string stringCommand[] = { "mov", "add", "sub", "mul", "div", "jmp", "jn", "jp", "je", "cmp", "log" };
const string stringRegister[] = { "ax", "bx", "cx" };

std::map<string, Registry> regAssoc = { { "ax", R_AX }, { "bx", R_BX }, { "cx", R_CX } };

uint8_t memory[1024];
uint8_t reg[Registry::RCountAll];

void compile(const char* inputFileName, const char* outputFileName);
void execute(const char* outputFileName);

int main()
{
    compile("input.vms", "output.vmb");
    execute("output.vmb");
    return 0;
}

void writeBinary(ofstream& file, const char* data)
{
    file.write(data, sizeof(uint8_t));
}

template <typename T>
const char* numToCharP(T& val)
{
    return (const char*)&val;
}

template <typename T>
void log(T message)
{
    cout << message << endl;
}

Command getOpCode(string& s)
{
    string twoOP = s.substr(0, 2);
    string threeOP = s.substr(0, 3);
    for (int i = 0; i < CCount; ++i) {
        if (twoOP == stringCommand[i] || threeOP == stringCommand[i]) {
            if (twoOP == stringCommand[i])
                s = s.substr(3, s.size());
            if (threeOP == stringCommand[i])
                s = s.substr(4, s.size());
            return static_cast<Command>(i + 1);
        }
    }
    return Invalid;
}

bool isRegistry(const string& s)
{
    for (int i = 0; i < RCount; ++i) {
        if (s == stringRegister[i])
            return true;
    }
    return false;
}

void clearString(string& s)
{
    while (true) {
        if (s[0] == SPACE) {
            s.erase(0, 1);
            continue;
        }

        break;
    }
}

void memoryPrint()
{
    int i = 0;
    while (true) {
        cout << (int)memory[i] << " ";
        if (memory[i] == PROGRAM_END)
            break;
        i++;
    }
}

void execute(const char* inputFileName)
{
    ifstream input(inputFileName, std::ios::in | std::ios::binary);

    string result;

    int i = 0;
    while (!input.eof()) {
        char c;
        input.read((char*)&c, sizeof(uint8_t));
        memory[i] = c;
        i++;
    }
    memory[i - 1] = PROGRAM_END;

    //memoryPrint();

    cout << "Size: " << i - 1 << " byte" << endl;
    log("Run:");

    reg[R_PC] = 0;
    while (true) {

        if (memory[reg[R_PC]] == PROGRAM_END)
            break;

        uint8_t opCode = memory[reg[R_PC]];
        uint8_t flag = memory[reg[R_PC] + 1];
        uint8_t leftOp = memory[reg[R_PC] + 2];
        uint8_t rightOp = memory[reg[R_PC] + 3];

        switch (opCode) {
        case MOV: {
            if (flag == Number) {
                reg[leftOp] = rightOp;
            } else {
                reg[leftOp] = reg[rightOp];
            }
            reg[R_PC] += 4;
            break;
        }
        case ADD: {
            if (flag == Number) {
                reg[leftOp] += rightOp;
            } else {
                reg[leftOp] += reg[rightOp];
            }

            reg[R_PC] += 4;
            break;
        }
        case LOG: {

            log((short)reg[leftOp]);

            reg[R_PC] += 4;
            break;
        }
        case CMP: {
            if (flag == Number) {
                if (reg[leftOp] == rightOp)
                    reg[R_COND] = Equal;
                if (reg[leftOp] < rightOp)
                    reg[R_COND] = Negative;
                if (reg[leftOp] > rightOp)
                    reg[R_COND] = Positive;

            } else {
                reg[R_CX] = leftOp - reg[rightOp];
            }
            reg[R_PC] += 4;

            break;
        }

        case JN: {
            if (reg[R_COND] == Negative) {
                reg[R_PC] = leftOp;
            } else {
                reg[R_PC] += 4;
            }
            break;
        }

        default:
            log("Error");
            break;
        }
    }
}

void writeCommand(ofstream& file, const char* opCode, const char* flag, const char* leftOpr, const char* rightOpr)
{
    writeBinary(file, opCode);
    writeBinary(file, flag);
    writeBinary(file, leftOpr);
    writeBinary(file, rightOpr);
}

void compile(const char* inputFileName, const char* outputFileName)
{
    string result;
    ifstream input(inputFileName);
    ofstream output(outputFileName, std::ios::out | std::ios::binary);

    int lineNum = 0;

    while (getline(input, result)) {
        if (result[result.length() - 1] == ':') {
            labelStore.push_back(Label(lineNum << 2, result.substr(0, result.length() - 1)));
        }
        lineNum++;
    }

    input.clear();
    input.seekg(0);

    while (getline(input, result)) {

        if (result[result.length() - 1] == ':' || result.empty())
            continue;

        clearString(result);

        Command opCode = getOpCode(result);

        Registry leftOpr;
        EFlags flag;

        switch (opCode) {
        case MOV:
        case MUL:
        case DIV:
        case ADD:
        case CMP: {

            int rightOpr;
            leftOpr = regAssoc[result.substr(0, 2)];
            result = result.substr(4, result.size());
            if (isRegistry(result)) {
                flag = Register;
                rightOpr = regAssoc[result];
            } else {
                flag = Number;
                rightOpr = stoi(result);
            }

            writeCommand(output, numToCharP(opCode), numToCharP(flag), numToCharP(leftOpr), numToCharP(rightOpr));
            break;
        }
        case LOG: {

            string logMsg = result.substr(0, result.length());

            writeCommand(output, numToCharP(opCode), numToCharP(flag), numToCharP(regAssoc[logMsg]), numToCharP(Flags[Number]));
            break;
        }
        case JN:
        case JP:
        case JE: {
            int adress = 0;
            string sLabel = result.substr(0, result.length());
            for (size_t i = 0; i < labelStore.size(); ++i) {
                if (sLabel == labelStore[i].name) {
                    adress = labelStore[i].adress;
                    break;
                }
            }

            writeCommand(output, numToCharP(opCode), numToCharP(Flags[Number]), numToCharP(adress), numToCharP(Flags[Number]));
            break;
        }
        default:

            log("Syntax error");
            return;
        }
    }

    input.close();
    output.close();

    log("Compile success");
}
