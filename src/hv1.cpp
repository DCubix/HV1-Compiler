#include "hv1.h"

#include <iostream>
#include <algorithm>
#include <cassert>
#include <cstring>
#include <fstream>

#if defined(WIN32) || defined(MINGW) || defined(MSVC)
#include <conio.h>
#else
#include <termios.h>
#include <stdio.h>

static struct termios old, new;

/* Initialize new terminal i/o settings */
void initTermios(int echo) {
	tcgetattr(0, &old); /* grab old terminal i/o settings */
	new = old; /* make new settings same as old settings */
	new.c_lflag &= ~ICANON; /* disable buffered i/o */
	if (echo) {
		new.c_lflag |= ECHO; /* set echo mode */
	} else {
		new.c_lflag &= ~ECHO; /* set no echo mode */
	}
	tcsetattr(0, TCSANOW, &new); /* use these new terminal i/o settings now */
}

/* Restore old terminal i/o settings */
void resetTermios(void) {
	tcsetattr(0, TCSANOW, &old);
}

/* Read 1 character - echo defines echo mode */
char getch_(int echo) {
	char ch;
	initTermios(echo);
	ch = getchar();
	resetTermios();
	return ch;
}

/* Read 1 character without echo */
char getch(void) {
	return getch_(0);
}
#endif

std::unordered_map<InstructionCode, Instruction> HV1Instructions = {
	INST(hlt, 0, [](HV1& hv1, internal::Inst i) { hv1.halt(); }),
	INST(rdi, 2, [](HV1& hv1, internal::Inst i) {
		u16 loc = i.data;
		u16 val = 0; std::cin >> val;
		hv1.writeMem(loc, val);
	}),
	INST(rdk, 2, [](HV1& hv1, internal::Inst i) {
		u16 loc = i.data;
		u16 val = getch();
		hv1.writeMem(loc, val);
	}),
	INST(lda, 2, [](HV1& hv1, internal::Inst i) {
		u16 value = 0;
		switch (i.opcode.flag) {
			case Flag::AC: std::cout << "WARNING: Loading AC into AC" << std::endl; break;
			case Flag::None: return;
			case Flag::Mem: value = hv1.readMem(i.data); break;
			case Flag::Const: value = i.data; break;
		}
		hv1.ac(value);
	}),
	INST(stm, 2, [](HV1& hv1, internal::Inst i) { hv1.writeMem(i.data, hv1.ac()); }),
	INST(add, 1, [](HV1& hv1, internal::Inst i) {
		u16 value = 0;
		switch (i.opcode.flag) {
			case Flag::None: return;
			case Flag::Mem: value = hv1.readMem(i.data); break;
			case Flag::Const: value = i.data; break;
			case Flag::AC: value = hv1.ac(); break;
		}
		hv1.ac(hv1.ac() + value);
	}),
	INST(sub, 1, [](HV1& hv1, internal::Inst i) {
		u16 value = 0;
		switch (i.opcode.flag) {
			case Flag::None: return;
			case Flag::Mem: value = hv1.readMem(i.data); break;
			case Flag::Const: value = i.data; break;
			case Flag::AC: value = hv1.ac(); break;
		}
		hv1.ac(hv1.ac() - value);
	}),
	INST(mod, 1, [](HV1& hv1, internal::Inst i) {
		u16 value = 0;
		switch (i.opcode.flag) {
			case Flag::None: return;
			case Flag::Mem: value = hv1.readMem(i.data); break;
			case Flag::Const: value = i.data; break;
			case Flag::AC: value = hv1.ac(); break;
		}
		hv1.ac(hv1.ac() % value);
	}),
	INST(jnz, 1, [](HV1& hv1, internal::Inst i) {
		if (hv1.ac() != 0) hv1.branch(i.data);
	}),
	INST(jez, 1, [](HV1& hv1, internal::Inst i) {
		if (hv1.ac() == 0) hv1.branch(i.data);
	}),
	INST(cal, 1, [](HV1& hv1, internal::Inst i) {
		hv1.callStack.push(hv1.pc());
		hv1.branch(i.data);
	}),
	INST(ret, 1, [](HV1& hv1, internal::Inst i) {
		if (hv1.callStack.empty()) {
			std::cout << "Invalid return!" << std::endl;
			return;
		}
		hv1.branch(hv1.callStack.top());
		hv1.callStack.pop();
	}),
	INST(psh, 1, [](HV1& hv1, internal::Inst i) {
		u16 value = 0;
		switch (i.opcode.flag) {
			case Flag::None: return;
			case Flag::Mem: value = hv1.readMem(i.data); break;
			case Flag::Const: value = i.data; break;
			case Flag::AC: value = hv1.ac(); break;
		}
		hv1.stack.push(value);
	}),
	INST(pop, 1, [](HV1& hv1, internal::Inst i) {
		if (hv1.stack.empty()) {
			std::cerr << "Stack is empty!" << std::endl;
			return;
		}
		hv1.writeMem(i.data, hv1.stack.top());
		hv1.stack.pop();
	}),
	INST(out, 1, [](HV1& hv1, internal::Inst i) {
		u16 value = 0;
		switch (i.opcode.flag) {
			case Flag::None: return;
			case Flag::Mem: value = hv1.readMem(i.data); break;
			case Flag::Const: value = i.data; break;
			case Flag::AC: value = hv1.ac(); break;
		}
		std::cout << value << std::endl;
	}),
	INST(ouc, 1, [](HV1& hv1, internal::Inst i) {
		u16 value = 0;
		switch (i.opcode.flag) {
			case Flag::None: return;
			case Flag::Mem: value = hv1.readMem(i.data); break;
			case Flag::Const: value = i.data; break;
			case Flag::AC: value = hv1.ac(); break;
		}
		printf("%c", char(value));
	})
};

namespace internal {
	Inst decode(u32 data) {
		Inst inst{};
		std::memcpy(&inst, &data, sizeof(Inst));
		return inst;
	}

	u32 encode(Inst inst) {
		u32 enc = 0;
		std::memcpy(&enc, &inst, sizeof(u32));
		return enc;
	}

	u8 getInstruction(const std::string& name) {
		for (auto&& [k, v] : HV1Instructions) {
			if (v.name == name) return k;
		}
		return 0;
	}

	bool hasInstruction(const std::string& name) {
		for (auto&& [k, v] : HV1Instructions) {
			if (v.name == name) return true;
		}
		return false;
	}
}

HV1::HV1()
	: m_ac(0), m_pc(0), m_cyclesTimer(0), m_programSize(0)
{
	m_mem.fill(0);
	m_prog.fill(0);
}

HV1::HV1(const Program& program) : HV1() {
	std::copy(program.begin(), program.end(), m_prog.begin());
	m_programSize = program.size();
}

u16 HV1::readMem(u16 addr) {
	assert(addr < HV1_MEMORY_SIZE && "Invalid address.");
	return m_mem[addr];
}

void HV1::writeMem(u16 addr, u16 value) {
	assert(addr < HV1_MEMORY_SIZE && "Invalid address.");
	m_mem[addr] = value;
}

void HV1::run() {
	m_running = true;
	while (m_running) {
		internal::Inst i = internal::decode(next());
		Instruction inst = HV1Instructions[InstructionCode(i.opcode.op)];

		m_cyclesTimer = inst.cycles;
		while (m_cyclesTimer > 0) {
			m_cyclesTimer--;
		}

		inst.callback(*this, i);
	}
}

u32 HV1::next() {
	return m_prog[m_pc++];
}

void HV1::dump(const std::string& file) const {
	std::ofstream fs(file, std::ios::binary);
	if (fs.good()) {
		fs.write((char*) m_prog.data(), m_programSize * sizeof(u32));
		fs.close();
	}
}