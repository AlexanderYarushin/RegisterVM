#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

using namespace std;

#define SPACE 0x20

struct Label {
    int adress;
    string name;
    Label(int _adress, string _name)
        : adress(_adress)
        , name(_name) {};
};

vector<Label> labelStore;

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
uint8_t reg[Registry::RCount];

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
            //   log("getopcode");
            log(s);
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

void compile(const char* inputFileName, const char* outputFileName)
{
    string result;
    ifstream input(inputFileName);
    ofstream output(outputFileName, std::ios::out | std::ios::binary);

    int lineNum = 0;

    input.clear();
    input.seekg(0);

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

            writeBinary(output, numToCharP(opCode));
            writeBinary(output, numToCharP(flag));
            writeBinary(output, numToCharP(leftOpr));
            writeBinary(output, numToCharP(rightOpr));
            break;
        }
        case LOG: {

            string logMsg = result.substr(0, result.length());

            writeBinary(output, numToCharP(opCode));
            writeBinary(output, numToCharP(regAssoc[logMsg]));
            writeBinary(output, numToCharP(Flags[Number]));
            writeBinary(output, numToCharP(Flags[Number]));
            break;
        }
        case JN:
        case JP:
        case JE: {
            int adress = 0;
            string sLabel = result.substr(0, result.length());
            log(sLabel);
            for (size_t i = 0; i < labelStore.size(); ++i) {
                if (sLabel == labelStore[i].name) {
                    adress = labelStore[i].adress;
                    break;
                }
            }

            writeBinary(output, numToCharP(opCode));
            writeBinary(output, numToCharP(adress));
            writeBinary(output, numToCharP(Flags[Number]));
            writeBinary(output, numToCharP(Flags[Number]));
            break;
        }
        default:
            cout << "syntax error" << endl;
            break;
        }
    }

    input.close();
    output.close();
}
