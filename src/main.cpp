#include <iostream>

#include "hv1.h"
#include "asm.h"

int main(int argc, char** argv) {
	std::string progs = R"(
	rdi $0
	lda 1

	stm $1
	lda 0
	stm $2

	out $2
	lda $1
	jez end

	cal dec0

loop:
	out $1

	lda $1
	add $2
	stm $2

	lda $1
	stm $3
	lda $2
	stm $1
	lda $3
	stm $2

	cal dec0

	jnz loop

end:
	rdk $100
	hlt

dec0:
	lda $0
	sub 1
	stm $0
	ret

)";

	Program prog = ASM::assemble(progs);

	HV1 hv1(prog);
	hv1.dump("prog.hvb");
	hv1.run();

	return 0;
}
