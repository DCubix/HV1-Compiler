#ifndef HV1_H
#define HV1_H

#include "common.h"
#include <array>
#include <stack>
#include <functional>
#include <unordered_map>

#define HV1_MEMORY_SIZE 0x100 // 512 Bytes (256 u16)
#define HV1_PROGRAM_SIZE 8192 // 8 KB

namespace internal {
	struct OpCode {
		// FLAG
		// 00: None, 01: Memory Location, 10: Constant, 11: AC
		u8 flag : 2;
		u8 op : 6;
	};

	struct Inst {
		OpCode opcode; u8 pad0;
		u16 data;
	};

	Inst decode(u32 data);
	u32 encode(Inst inst);

	u8 getInstruction(const std::string& name);
	bool hasInstruction(const std::string& name);
}

class HV1;
using InstructionCallback = std::function<void(HV1&, internal::Inst)>;

struct Instruction {
	u8 code;
	std::string name;
	InstructionCallback callback;
	u32 cycles;
};

#define HV1_CYCLE_SIZE 700000
#define INST(name, cycles, callback) { InstructionCode::name, { u8(InstructionCode::name), #name, callback, ((cycles*HV1_CYCLE_SIZE)+1) } }

enum Flag {
	None = 0b00,
	Mem = 0b01,
	Const = 0b10,
	AC = 0b11
};

#define I(flag, op, data) (internal::encode({ { flag, op }, 0, data }))
#define M(op, loc) I(Flag::Mem, op, loc)
#define C(op, val) I(Flag::Const, op, val)
#define A(op) I(Flag::AC, op, 0)
#define N(op) I(Flag::None, op, 0)

enum InstructionCode {
	hlt = 0,	//< Stops the program execution
	rdi,		//< Read input into a memory location
	rdk,		//< Read key into a memory location
	lda,		//< Loads a value into AC
	stm,		//< Stores the value of AC into a memory location
	add,		//< Adds a value to AC
	sub,		//< Subtracts a value from AC
	mod,		//< Stores the result of mod(AC, value) into AC
	jnz,		//< Jumps if AC is *not* zero
	jez,		//< Jumps if AC is zero
	cal,		//< Calls a macro/sub-routine
	ret,		//< Returns from a macro/sub-routine
	psh,		//< Pushes a value to the stack
	pop,		//< Pops a value from the stack into a memory location
	out,		//< Prints a value
	ouc			//< Prints an ascii char
};

extern std::unordered_map<InstructionCode, Instruction> HV1Instructions;

using Program = std::vector<u32>;

class HV1 {
public:
	HV1();
	~HV1() = default;

	HV1(const Program& program);

	u16 readMem(u16 addr);
	void writeMem(u16 addr, u16 value);

	void run();
	void halt() { m_running = false; }

	u16 ac() const { return m_ac; }
	void ac(u16 v) { m_ac = v; }

	u16 pc() const { return m_pc; }
	void branch(u16 pos) { m_pc = pos; }

	void dump(const std::string& file) const;

	// Stacks
	std::stack<u16> callStack;
	std::stack<u16> stack;
private:
	u32 next();

	// Logic and Storage
	u16 m_ac;
	std::array<u16, HV1_MEMORY_SIZE> m_mem;
	std::array<u32, HV1_PROGRAM_SIZE> m_prog;
	u16 m_programSize;

	u16 m_pc;
	u32 m_cyclesTimer;
	bool m_running;

};

#endif // HV1_H