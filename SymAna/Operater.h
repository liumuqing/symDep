#pragma once

enum Operater
{
	ADD, SUB, AND, OR, XOR,
	INC, DEC,
	MUL, DIV, MOD,
	SHL, SHR, ROR, ROL,
	NEG, NOT,
	MOV, LEA, XCHG,
	PUSH, POP,
	DEREF,

	SCASB,
	CDQ, CBW, CWD, CWDE,
	NOP,

	CONN, SLICE_01, SLICE_23, SLICE_0, SLICE_1, SLICE_2, SLICE_3,
};

void operater2operaterStr(char * operaterStr, const Operater& operater);
