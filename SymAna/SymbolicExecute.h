#pragma once
#include "Operater.h"
#include "Operand.h"
struct Instruction
{
	Operater operater;
	Operand operands[MAX_NUM_OPERANDS];
	class Expression * operandExpressions[MAX_NUM_OPERANDS];
	Instruction()
	{
		for (unsigned int i = 0; i < MAX_NUM_OPERANDS; i++)
		{
			operandExpressions[i] = nullptr;
		}
	}
#ifndef RELEASE
	~Instruction()
	{
		for (unsigned int i = 0; i < num_operands; i++)
		{
			if (operandExpressions[i])
			{
				throw Error("Instruction::operandExpressions has an non-null element.");
			}
		}
	};
#endif
	unsigned int num_operands;
};
void SymbolicExecute(const Instruction& inst, class SymbolicMemory * symbolicMemory, class ConcreteMemory * concreteMemory);