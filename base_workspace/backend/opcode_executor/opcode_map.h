#ifndef TURBO_SANTA_COMMON_BACK_END_OPCODE_MAP_H_
#define TURBO_SANTA_COMMON_BACK_END_OPCODE_MAP_H_

#include <vector>
#include <map>

#include "backend/opcode_executor/opcodes.h"
#include "backend/opcode_executor/opcode_handlers.h"
#include "backend/opcode_executor/registers.h"

namespace back_end {
namespace opcodes {

extern unsigned char* rA; 
extern unsigned char* rB; 
extern unsigned char* rC; 
extern unsigned char* rD; 
extern unsigned char* rE; 
extern unsigned char* rF; 
extern unsigned char* rH; 
extern unsigned char* rL; 
extern unsigned char* rAF;
extern unsigned char* rBC;
extern unsigned char* rDE;
extern unsigned char* rHL;
extern unsigned char* rPC;
extern unsigned char* rSP;

std::map<unsigned short, Opcode> CreateOpcodeMap(registers::GB_CPU* cpu);

} // namespace opcodes
} // namespace back_end
#endif // TURBO_SANTA_COMMON_BACK_END_OPCODE_MAP_H_
