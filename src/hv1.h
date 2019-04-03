#ifndef HV1_H
#define HV1_H

#include "common.h"
#include <array>
#include <stack>
#include <functional>
#include <unordered_map>

#define HV1_MEMORY_SIZE 0x100 // 512 Bytes (256 u16)
#define HV1_PROGRAM_SIZE 8192 // 8 KB

class HV1;
using InstructionCallback = std::function<void(HV1&)>;

struct Instruction {
	std::string name;
	InstructionCallback callback;
	u32 cycles;
};

#define HV1_CYCLE_SIZE 10000000
#define INST(name, cycles, callback) { InstructionCode::name, { #name, callback, ((cycles*HV1_CYCLE_SIZE)+1) } }

enum InstructionCode {
	hlt = 0,	//< Stops the program execution
	rdi,		//< Read input into a memory location
	lma,		//< Loads a value from a memory location into AC
	lca,		//< Loads a constant value into AC
	stm,		//< Stores the value of AC into a memory location
	add,		//< Adds the value of a memory location to AC
	adc,		//< Adds a constant value to AC
	sub,		//< Subtracts the value of a memory location from AC
	sbc,		//< Subtracts a constant value from AC
	mod,		//< Stores the result of mod(AC, mem loc) into AC
	mdc,		//< Stores the result of mod(AC, const) into AC
	jnz,		//< Jumps if AC is *not* zero
	jez,		//< Jumps if AC is zero
	cal,		//< Calls a macro/sub-routine
	ret,		//< Returns from a macro/sub-routine
	psh,		//< Pushes the value of memory location to the stack
	psc,		//< Pushes a constant to the stack
	psa,		//< Pushes AC to the stack
	pop,		//< Pops a value from the stack into a memory location
	out			//< Prints a value from a memory loc
};

extern std::unordered_map<InstructionCode, Instruction> HV1Instructions;

class HV1 {
public:
	HV1();
	~HV1() = default;

	HV1(const std::vector<u8>& program);

	u16 readMem(u8 addr);
	void writeMem(u8 addr, u16 value);

	void run();
	void halt() { m_running = false; }

	u16 next();

	u16 ac() const { return m_ac; }
	void ac(u16 v) { m_ac = v; }

	u16 pc() const { return m_pc; }
	void branch(u16 pos) { m_pc = pos; }

	// Stacks
	std::stack<u16> callStack;
	std::stack<u16> stack;
private:
	// Logic and Storage
	u16 m_ac;
	std::array<u16, HV1_PROGRAM_SIZE + HV1_MEMORY_SIZE> m_mem;
	u16 m_pc;
	u32 m_cyclesTimer;
	bool m_running;

};

#endif // HV1_H