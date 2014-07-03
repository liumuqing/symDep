#pragma once
#include <typeinfo>
#include "Common.h"
#include "SymbolicMemory.h"
#include "Operand.h"
#include "Register.h"
#include "Expression.h"
#include "Operater.h"

void(*SymbolicMemory::beforeGet)(SymbolicMemory *, unsigned int, unsigned int) = nullptr;
void(*SymbolicMemory::beforePut)(SymbolicMemory *, unsigned int, unsigned int, class Expression *) = nullptr;

class Expression * SymbolicMemory::get(unsigned int address, unsigned int memoryLength)
{
#ifndef RELEASE
	if (address % memoryLength != 0) return Expression::create(nullptr);
	if (address % memoryLength != 0) throw Error("调用SymbolicMemory.get()时地址参数不是对齐的");
#endif
	//解决未对其的get
	if (beforeGet) beforeGet(this, address, memoryLength);
	if (memoryLength == 4)
	{
		decltype(memory[2].find(address)) temp[4];

		temp[0] = memory[2].find(address);
		if (temp[0] != memory[2].end())
		{
			return temp[0]->second;
		}

		temp[0] = memory[1].find(address);
		temp[1] = memory[1].find(address + 2);
		if (temp[0] != memory[1].end() && temp[1] != memory[1].end())
		{
			return Expression::create(CONN, 2, temp[0]->second, temp[1]->second);
		}

		temp[0] = memory[0].find(address);
		temp[1] = memory[0].find(address + 1);
		temp[2] = memory[0].find(address + 2);
		temp[3] = memory[0].find(address + 3);
		if (temp[0] != memory[0].end() && temp[1] != memory[0].end() && temp[2] != memory[0].end() && temp[3] != memory[0].end())
		{
			return Expression::create(CONN, 4, temp[0]->second, temp[1]->second, temp[2]->second, temp[3]->second);
		}
	}
	else if (memoryLength == 2)
	{
		decltype(memory[2].find(address)) temp[4];

		temp[0] = memory[1].find(address);
		if (temp[0] != memory[1].end())
		{
			return temp[0]->second;
		}

		temp[0] = memory[0].find(address);
		temp[1] = memory[0].find(address + 2);
		if (temp[0] != memory[0].end() && temp[1] != memory[0].end())
		{
			return Expression::create(CONN, 2, temp[0]->second, temp[1]->second);
		}

		temp[0] = memory[2].find(address / 4 * 4);
		if (temp[0] != memory[2].end())
		{
			switch (address % 2)
			{
			case 0:
				return Expression::create(SLICE_01, 1, temp[0]->second);
			case 2:
				return Expression::create(SLICE_23, 1, temp[0]->second);
			default:
				throw Error("Expression::get error");
			}
		}
	}
	else if (memoryLength == 1)
	{
		decltype(memory[2].find(address)) temp[4];

		temp[0] = memory[0].find(address);
		if (temp[0] != memory[0].end())
		{
			return temp[0]->second;
		}

		temp[0] = memory[2].find(address / 4 * 4);
		if (temp[0] != memory[2].end())
		{
			switch (address % 4)
			{
			case 0:
				return Expression::create(SLICE_0, 1, temp[0]->second);
			case 1:
				return Expression::create(SLICE_1, 1, temp[0]->second);
			case 2:
				return Expression::create(SLICE_2, 1, temp[0]->second);
			case 3:
				return Expression::create(SLICE_3, 1, temp[0]->second);
			default:
				throw Error("Expression::get error");
			}
		}

		temp[0] = memory[1].find(address / 2 * 2);
		if (temp[0] != memory[1].end())
		{
			switch (address % 2)
			{
			case 0:
				return Expression::create(SLICE_0, 1, temp[0]->second);
			case 1:
				return Expression::create(SLICE_1, 1, temp[0]->second);
			default:
				throw Error("Expression::get error");
			}
		}
	}
	else
	{
		throw Error("Expression::get ERROR");
	}

	return Expression::create(nullptr);
}

class Expression * SymbolicMemory::get(const Operand& operand)
{
	switch (operand.type)
	{
	case DIRECT_MEM:
		return get(operand.value, operand.length);
	case REG:
		return get(registerOffset(Register(operand.value)), operand.length);
	case IMM: default:
		throw Error("SymbolicMemory::get Error");
	}
}

Expression * SymbolicMemory::put(unsigned int address, unsigned int memoryLength, Expression * expression)
{
#ifndef RELEASE
	if (address % memoryLength != 0) return nullptr;
	if (address % memoryLength != 0) throw Error("调用SymbolicMemory.get()时地址参数不是对齐的");
#endif
	//TODO:将不对齐的put请求拆分成一个一个字节进行处理
	if (beforePut) beforePut(this, address, memoryLength, expression);

	erase(address, memoryLength);

	expression->addRefCount();
	switch (memoryLength)
	{
	case 4:
		return memory[2][address] = expression;
	case 2:
		return memory[1][address] = expression;
	case 1:
		return memory[0][address] = expression;
	default:
		throw Error("Unfinished");
	}
}

Expression * SymbolicMemory::put(const Operand& operand, Expression * expression)
{
	switch (operand.type)
	{
	case DIRECT_MEM:
		return put(operand.value, operand.length, expression);
	case REG:
		return put(registerOffset(Register(operand.value)), operand.length, expression);
	case IMM: default:
		throw Error("SymbolicMemory::put Error");
	}
}

void SymbolicMemory::erase(unsigned int address, unsigned int memoryLength)
{
	switch (memoryLength)
	{
	case 4:
	{
			  auto it = memory[2].find(address);
			  if (it != memory[2].end())
			  {
				  it->second->decRefCount();
				  memory[2].erase(address);
			  }
			  erase(address, 2);
			  erase(address + 2, 2);
	}
		break;

	case 2:
	{
			  auto it = memory[1].find(address);
			  if (it != memory[1].end())
			  {
				  it->second->decRefCount();
				  memory[1].erase(address);
			  }
			  it = memory[2].find(address / 4 * 4);
			  if (it != memory[2].end())
			  {
				  Expression * exp = it->second;
				  exp->addRefCount();//避免接下来的erase把这个表达式delete掉
				  erase(address / 4 * 4, 4);
				  switch (address % 4)
				  {
				  case 0:	put(address + 2, 2, Expression::create(SLICE_23, 1, exp)); break;
				  case 2:	put(address - 2, 2, Expression::create(SLICE_01, 1, exp)); break;
				  default: throw Error("Symbolic::erase 未对齐");
				  }

				  exp->decRefCount();
			  }
			  erase(address, 1);
			  erase(address + 1, 1);
	}
		break;
	case 1:
	{
			  auto it = memory[0].find(address);
			  if (it != memory[0].end())
			  {
				  it->second->decRefCount();
				  memory[0].erase(address);
			  }

			  //将存在的4字节下降下来
			  it = memory[2].find(address / 4 * 4);
			  if (it != memory[2].end())
			  {
				  Expression * exp = it->second;
				  exp->addRefCount();//避免接下来的erase把这个表达式delete掉
				  erase(address / 4 * 4, 4);
				  put(address / 4 * 4, 2, Expression::create(SLICE_01, 1, exp));
				  put(address / 4 * 4 + 2, 2, Expression::create(SLICE_23, 1, exp));
				  exp->decRefCount();//避免接下来的erase把这个表达式delete掉
			  }

			  it = memory[1].find(address / 2 * 2);
			  if (it != memory[1].end())
			  {
				  Expression * exp = it->second;
				  exp->addRefCount();//避免接下来的erase把这个表达式delete掉
				  erase(address / 2 * 2, 2);
				  switch (address % 2)
				  {
				  case 0:	put(address + 1, 2, Expression::create(SLICE_1, 1, exp)); break;
				  case 1:	put(address - 1, 2, Expression::create(SLICE_0, 1, exp)); break;
				  default: throw Error("Symbolic::erase 未对齐");
				  }

				  exp->decRefCount();
			  }
	}
		break;
	default:
		throw Error("SymbolicMemory::erase Error");
	}
}

void SymbolicMemory::erase(const Operand& operand)
{
	switch (operand.type)
	{
	case DIRECT_MEM:
		return erase(operand.value, operand.length);
	case REG:
		return erase(registerOffset(Register(operand.value)), operand.length);
	case IMM: default:
		throw Error("SymbolicMemory::get Error");
	}
}

void SymbolicMemory::clear()
{
	memory[0].clear();
	memory[1].clear();
	memory[2].clear();
}