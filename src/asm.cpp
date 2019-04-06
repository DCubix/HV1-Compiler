#include "asm.h"

#include "scanner.h"

#include <iostream>
#include <cctype>
#include <cstring>
#include <map>

struct Token {
	enum Type {
		Number = 0,
		Mem,
		AC,
		Inst,
		Label,
		EOP
	} type;
	std::string lex;
	u16 value;
	u16 line;

	Token(Type type, std::string lex, u16 value, u16 line)
		: type(type), lex(lex), value(value), line(line)
	{}
};

namespace ASM {
	Program assemble(const std::string& src) {
		std::map<std::string, u16> labels;
		StringScanner sc{ src };

		u16 line = 0;
		std::vector<Token> tokens;

		while (sc.hasNext()) {
			char c = sc.peek().value();
			if (::isalpha(c) || c == '_' || c == '$') {
				std::string val = sc.scanString([](char k) {
					return ::isalpha(k) ||
							::isdigit(k) ||
							k == 'x' || k == 'X' ||
							k == '_' || k == '$' || k == ':';
				});
				if (val == "AC") {
					tokens.push_back(Token(Token::AC, val, 0, line));
				} else if (internal::hasInstruction(val)) {
					tokens.push_back(Token(Token::Inst, val, internal::getInstruction(val), line++));
				} else if (val[0] == '$') {
					tokens.push_back(Token(Token::Mem, val, u16(std::stoi(val.substr(1))), line));
				} else if (val.back() == ':') {
					labels[val.substr(0, val.size()-1)] = line;
				} else {
					tokens.push_back(Token(Token::Label, val, 0, line));
				}
			} else if (::isdigit(c)) {
				std::string val = sc.scanString([](char k) { return ::isdigit(k) || k == 'x' || k == 'X'; });
				tokens.push_back(Token(Token::Number, val, u16(std::stoi(val)), line));
			} else if (c == '"') {
				sc.scan();
				std::string val = sc.scanString([](char k) { return k != '"'; });

				u8 oucI = internal::getInstruction("ouc");
				bool first = true;
				for (char l : val) {
					if (!first) {
						tokens.push_back(Token(Token::Inst, "ouc", oucI, line++));
					} else {
						first = false;
					}
					tokens.push_back(Token(Token::Number, "STR", u16(l), line));
				}
				sc.scan();
			} else {
				sc.scan();
			}
		}
		tokens.push_back(Token(Token::EOP, "EOF", 0, line));

		// Scan tokens
		Program prog{};
		Scanner<Token> ts{ tokens };

		internal::Inst inst{};
		std::memset(&inst, 0, sizeof(internal::Inst));
		inst.opcode.op = 63;

		while (ts.hasNext()) {
			Token t = ts.peek().value();
			if (t.type == Token::Inst || t.type == Token::EOP) {
				if (inst.opcode.op != 63) {
					prog.push_back(internal::encode(inst));
					inst = internal::Inst();
					std::memset(&inst, 0, sizeof(internal::Inst));
					inst.opcode.op = 63;
				}
				inst.opcode.op = t.value;
			} else if (t.type == Token::AC) {
				inst.opcode.flag = Flag::AC;
			} else if (t.type == Token::Mem) {
				inst.opcode.flag = Flag::Mem;
				inst.data = t.value;
			} else if (t.type == Token::Number) {
				inst.opcode.flag = Flag::Const;
				inst.data = t.value;
			} else if (t.type == Token::Label) {
				inst.opcode.flag = Flag::Const;
				inst.data = labels[t.lex];
			}
			ts.scan();
		}

		return prog;
	}
}