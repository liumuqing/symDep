#include <algorithm>
#include "Common.h"
#include "ITraceConcreteMemory.h"
#include "Register.h"

ITraceConcreteMemory::ITraceConcreteMemory()
{
	unsigned int maxOffset = 0;
	for (int i = 0; i < REG4_LEN; i++)
	{
		maxOffset = std::max(registerOffset(REG4[i]), maxOffset);
	}
	memoryLen = maxOffset / 4 + 1;
	memory = new unsigned int[memoryLen];
}

ITraceConcreteMemory::~ITraceConcreteMemory()
{
	delete[] memory;
}

unsigned int ITraceConcreteMemory::get(unsigned int address, unsigned int len)
{
#ifndef RELEASE
	if (address % len != 0) throw Error("ITraceConcreteMemory::get 未对齐");
	if (address + len > memoryLen * 4) throw Error("ITraceConcreteMemory::get 溢出");
#endif
	unsigned int returnValue = 0;
	memcpy(&returnValue, ((char*)memory + address), len);
	return returnValue;
}

unsigned int ITraceConcreteMemory::put(unsigned int address, unsigned int len, unsigned int value)
{
#ifndef RELEASE
	if (address % len != 0) throw Error("ITraceConcreteMemory::get 未对齐");
	if (address + len > memoryLen * 4) throw Error("ITraceConcreteMemory::get 溢出");
#endif
	memcpy(((char*)memory + address), &value, len);
	return value;
}