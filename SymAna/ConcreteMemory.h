#pragma once
//Interface, all virtual
#include "Common.h"
#include "Operand.h"
#include "Operater.h"
class ConcreteMemory
{
public:

	virtual unsigned int get(unsigned int address, unsigned int length) = 0;
	virtual unsigned int put(unsigned int address, unsigned int length, unsigned int value) = 0;

	virtual ~ConcreteMemory(){};

	virtual unsigned int get(const struct Operand& operand)
	{
		switch (operand.type)
		{
		case OperandType::DIRECT_MEM:
			return get(operand.value, operand.length);
		case OperandType::REG:
			return get(registerOffset((Register)operand.value), operand.length);
		case OperandType::INDIRECT_MEM: case OperandType::IMM: default:
			throw Error("ConcreteMemory::get Error");
		}
	}
	virtual unsigned int put(const struct Operand& operand, unsigned int value)
	{
		switch (operand.type)
		{
		case OperandType::DIRECT_MEM:
			return put(operand.value, operand.length, value);
		case OperandType::REG:
			return put(registerOffset((Register)operand.value), operand.length, value);
		case OperandType::INDIRECT_MEM: case OperandType::IMM: default:
			throw Error("ConcreteMemory::put Error");
		}
	}
};