#pragma once
#include "ConcreteMemory.h"
class ITraceConcreteMemory :public ConcreteMemory
{
public:
	ITraceConcreteMemory();
	virtual ~ITraceConcreteMemory();

	unsigned int get(unsigned int address, unsigned int length);
	unsigned int put(unsigned int address, unsigned int length, unsigned int value);

private:
	unsigned int * memory;
	unsigned int memoryLen;
};