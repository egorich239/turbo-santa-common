#ifndef TURBO_SANTA_COMMON_BACK_END_OPCODES_H_
#define TURBO_SANTA_COMMON_BACK_END_OPCODES_H_

#include <functional>
#include <map>
#include <vector>

namespace back_end {
namespace opcodes {

struct Opcode;

typedef std::function<int(unsigned char* rom, int instruction_ptr, Opcode opcode)> OpcodeHandler;

struct Opcode {
    unsigned short opcode_name;
    unsigned char* reg1;
    unsigned char* reg2;
    OpcodeHandler handler;
};

struct PartialOpcode {
    unsigned short opcode_name;
    unsigned char* reg1;
};

struct PartialOpcodeExtraReg {
    unsigned short opcode_name;
    unsigned char* reg1;
    unsigned char* reg2;
};

std::vector<Opcode> ToList2(std::vector<PartialOpcodeExtraReg> partial_opcodes, OpcodeHandler handler); 

std::vector<Opcode> ToList1(std::vector<PartialOpcode> partial_opcodes, OpcodeHandler handler);

std::vector<Opcode> ToList(Opcode opcode);

std::vector<Opcode> Flatten(std::vector<std::vector<Opcode>> to_flatten);

std::map<unsigned short, Opcode> ToMap(std::vector<Opcode> opcode_list);

} // namespace back_end
} // namespace opcodes

#endif // TURBO_SANTA_COMMON_BACK_END_OPCODES_H_
