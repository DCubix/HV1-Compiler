#include <iostream>

#include "hv1.h"

int main(int argc, char** argv) {
	std::vector<u8> prog = {
		rdi, 0, // N
		lca, 1,
		stm, 1, // A
		lca, 0,
		stm, 2, // B

		// PRINT B
		out, 2,

		lma, 0,
		jez, 99, // Is zero?

		// PRINT A
		out, 1,

		// Sum A+B
		lma, 1,
		add, 2,
		stm, 2,

		// Swap A<->B
		lma, 1,
		stm, 3,
		lma, 2,
		stm, 1,
		lma, 3,
		stm, 2,

		out, 1,
		out, 2,

		hlt
	};

	HV1 hv1(prog);
	hv1.run();

	return 0;
}
