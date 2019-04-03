#include "hv1.h"

#include <iostream>
#include <algorithm>

std::unordered_map<InstructionCode, Instruction> HV1Instructions = {
	INST(hlt, 0, [](HV1& hv1) { hv1.halt(); }),
	INST(rdi, 2, [](HV1& hv1) {
		u16 loc = hv1.next();
		u16 val = 0;
		std::cin >> val;
		hv1.writeMem(loc, val);
	}),
	INST(lma, 2, [](HV1& hv1) { hv1.ac(hv1.readMem(hv1.next())); }),
	INST(lca, 1, [](HV1& hv1) { hv1.ac(hv1.next()); }),
	INST(stm, 2, [](HV1& hv1) { hv1.writeMem(hv1.next(), hv1.ac()); }),
	INST(add, 1, [](HV1& hv1) {
		u16 loc = hv1.next();
		hv1.ac(hv1.ac() + hv1.readMem(loc));
	}),
	INST(adc, 1, [](HV1& hv1) {
		u16 val = hv1.next();
		hv1.ac(hv1.ac() + val);
	}),
	INST(sub, 1, [](HV1& hv1) {
		u16 loc = hv1.next();
		hv1.ac(hv1.ac() - hv1.readMem(loc));
	}),
	INST(sbc, 1, [](HV1& hv1) {
		u16 val = hv1.next();
		hv1.ac(hv1.ac() - val);
	}),
	INST(mod, 1, [](HV1& hv1) {
		u16 loc = hv1.next();
		hv1.ac(hv1.ac() % hv1.readMem(loc));
	}),
	INST(mdc, 1, [](HV1& hv1) {
		u16 val = hv1.next();
		hv1.ac(hv1.ac() % val);
	}),
	INST(jnz, 1, [](HV1& hv1) {
		u16 to = hv1.next();
		if (hv1.ac() != 0) hv1.branch(to);
	}),
	INST(jez, 1, [](HV1& hv1) {
		u16 to = hv1.next();
		if (hv1.ac() == 0) hv1.branch(to);
	}),
	INST(cal, 3, [](HV1& hv1) {
		u16 sub = hv1.next();
		hv1.callStack.push(hv1.pc());
		hv1.branch(sub);
	}),
	INST(ret, 1, [](HV1& hv1) {
		if (hv1.callStack.empty()) {
			std::cerr << "Invalid return!" << std::endl;
			return;
		}
		hv1.branch(hv1.callStack.top());
		hv1.callStack.pop();
	}),
	INST(psh, 1, [](HV1& hv1) {
		u16 loc = hv1.next();
		hv1.stack.push(hv1.readMem(loc));
	}),
	INST(psc, 1, [](HV1& hv1) {
		u16 val = hv1.next();
		hv1.stack.push(val);
	}),
	INST(psa, 1, [](HV1& hv1) {
		hv1.stack.push(hv1.ac());
	}),
	INST(pop, 1, [](HV1& hv1) {
		u16 to = hv1.next();
		if (hv1.stack.empty()) {
			std::cerr << "Stack is empty!" << std::endl;
			return;
		}
		hv1.writeMem(to, hv1.stack.top());
		hv1.stack.pop();
	}),
	INST(out, 1, [](HV1& hv1) {
		u16 loc = hv1.next();
		std::cout << hv1.readMem(loc) << std::endl;
	})
};

HV1::HV1()
	: m_ac(0), m_pc(0), m_cyclesTimer(0)
{
	m_mem.fill(0);
}

HV1::HV1(const std::vector<u8>& program) : HV1() {
	std::copy(program.begin(), program.end(), m_mem.begin());
}

u16 HV1::readMem(u8 addr) {
	return m_mem[u16(addr) + HV1_PROGRAM_SIZE];
}

void HV1::writeMem(u8 addr, u16 value) {
	m_mem[u16(addr) + HV1_PROGRAM_SIZE] = value;
}

void HV1::run() {
	m_running = true;
	while (m_running) {
		u16 op = next();
		Instruction inst = HV1Instructions[InstructionCode(op)];
		m_cyclesTimer = inst.cycles;
		while (m_cyclesTimer > 0) {
			m_cyclesTimer--;
		}
		inst.callback(*this);
	}
}

u16 HV1::next() {
	return m_mem[m_pc++];
}