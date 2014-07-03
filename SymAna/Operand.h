#pragma once
#include "Common.h"
#include "Register.h"
#include <stdint.h>
enum OperandType
{
	REG, IMM, DIRECT_MEM, INDIRECT_MEM
};
struct Operand
{
	OperandType type;
	unsigned int length;
	uint32_t value;//�Ĵ�������������ֵ��ֱ���ڴ棬��ֱ���ڴ��OFFSET

	Register indirectRegisters[2];
	unsigned int indirectMultiplier[2];
	uint32_t num_indirectRegisters;

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