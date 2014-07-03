#pragma once
#include "Common.h"
#include "Register.h"
enum OperandType
{
	REG, IMM, DIRECT_MEM, INDIRECT_MEM
};
struct Operand
{
	OperandType type;
	unsigned int length;
	int value;//�Ĵ�������������ֵ��ֱ���ڴ棬��ֱ���ڴ��OFFSET

	Register indirectRegisters[2];
	int indirectMultiplier[2];
	int num_indirectRegisters;

	Operand(OperandType type, int value, unsigned int length = 0)
	{
		this->type = type;
		this->value = value;
		this->length = length;
	}
	Operand(Register reg)
	{
		this->type = REG;
		this->value = reg;
		this->length = registerLength(reg);
	}
	Operand()
	{
	}
};