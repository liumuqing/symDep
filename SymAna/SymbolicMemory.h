#pragma once
#include <unordered_map>
#include "Register.h"
class SymbolicMemory
{
public:
	static void(*beforeGet)(SymbolicMemory *, unsigned int address, unsigned int memoryLength);
	static void(*beforePut)(SymbolicMemory *, unsigned int address, unsigned int memoryLength, class Expression * expression);
	class Expression * get(const struct Operand& operand);
	class Expression * get(unsigned int address, unsigned int memoryLenght);

	class Expression * put(const struct Operand& operand, Expression * expression);
	class Expression * put(unsigned int address, unsigned int memoryLength, Expression * expression);

	void erase(const struct Operand& operand);
	void erase(unsigned int address, unsigned int memoryLength);

	void clear();
	std::unordered_map<unsigned int, class Expression *> memory[3];//分别是以2^0, 2^1, 2^2对齐的三个内存池

private:

};