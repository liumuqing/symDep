#include "SymbolicExecute.h"
#include "Expression.h"
#include "symbolicMemory.h"
#include "concreteMemory.h"
//把inst.operands 变成directd的，并将对应的地址表达式放入对应的indirectExpressions中(如果有的话)
void fillOperandExpressions(Instruction& inst, class SymbolicMemory * symbolicMemory, class ConcreteMemory * concreteMemory)
{
	Expression * indirectExpressions[MAX_NUM_OPERANDS] = { 0 };
	for (unsigned int i = 0; i < inst.num_operands; i++)
	{
#ifndef RELEASE
		if (inst.operandExpressions[i]) throw Error("Error");
#endif
		if (inst.operands[i].type == INDIRECT_MEM)
		{
			Expression * addrExpression = Expression::create(Operand(OperandType::IMM, inst.operands[i].value));
			for (int j = 0; j < inst.operands[i].num_indirectRegisters; j++)
			{
				Expression * temp = Expression::create(
					MUL,
					2,
					symbolicMemory->get(inst.operands[i].indirectRegisters[j]),
					Expression::create(Operand(IMM, inst.operands[i].indirectRegisters[j]))
					);
				addrExpression = Expression::create(ADD, 2, addrExpression, temp);
				inst.operands[i].value += concreteMemory->get(inst.operands[i].indirectRegisters[j]) * inst.operands[i].indirectMultiplier[j];
			}
			indirectExpressions[i] = addrExpression;
			inst.operands[i].type = DIRECT_MEM;
		}
		else
		{
			indirectExpressions[i] = nullptr;
		}

		if (inst.operater == Operater::LEA)
		{
			inst.operandExpressions[i] = Expression::create(indirectExpressions[i]);
		}
		else
		{
			if (indirectExpressions[i])
				inst.operandExpressions[i] = Expression::create(DEREF, 2, indirectExpressions[i], symbolicMemory->get(inst.operands[i]));
			else
			{
				if (inst.operands[i].type == IMM)
					inst.operandExpressions[i] = Expression::create(inst.operands[i]);
				else
					inst.operandExpressions[i] = Expression::create(symbolicMemory->get(inst.operands[i]));
			}
		}
	}
}

void generic_arithmetic_handle(Instruction& inst, SymbolicMemory * symbolicMemory, ConcreteMemory * concreteMemory);

void mov_handle(Instruction& inst, SymbolicMemory * symbolicMemory, ConcreteMemory * concreteMemory)
{
#ifndef RELEASE
	if (inst.num_operands != 2) throw Error("MOV_handle num_operands != 2");
#endif
	symbolicMemory->put(inst.operands[0], inst.operandExpressions[1]);
}
void xchg_handle(Instruction& inst, SymbolicMemory * symbolicMemory, ConcreteMemory * concreteMemory)
{
#ifndef RELEASE
	if (inst.num_operands != 2) throw Error("xhcg_handle num_operands != 2");
#endif
	symbolicMemory->put(inst.operands[0], inst.operandExpressions[1]);
	symbolicMemory->put(inst.operands[1], inst.operandExpressions[0]);
}
void nop_handle(Instruction& inst, SymbolicMemory * symbolicMemory, ConcreteMemory * concreteMemory)
{
	return;
}
void push_handle(Instruction& inst, SymbolicMemory * symbolicMemory, ConcreteMemory * concreteMemory)
{
	if (inst.num_operands != 1)
		throw Error("push_handle, inst should has only one operand?");
	else
	{
		symbolicMemory->put(
			concreteMemory->get(Operand(esp)) - inst.operands[0].length,
			inst.operands[0].length,
			inst.operandExpressions[0]
			);
	}
}
void pop_handle(Instruction& inst, SymbolicMemory * symbolicMemory, ConcreteMemory * concreteMemory)
{
	if (inst.num_operands != 1)
		throw Error("push_handle, inst should has only one operand?");
	else
	{
		symbolicMemory->put(inst.operands[0], symbolicMemory->get(concreteMemory->get(Operand(esp)), inst.operands[0].length));
	}
}
void and_handle(Instruction& inst, SymbolicMemory * symbolicMemory, ConcreteMemory * concreteMemory)
{
	generic_arithmetic_handle(inst, symbolicMemory, concreteMemory);
}
void or_handle(Instruction& inst, SymbolicMemory * symbolicMemory, ConcreteMemory * concreteMemory)
{
	generic_arithmetic_handle(inst, symbolicMemory, concreteMemory);
}
void xor_handle(Instruction& inst, SymbolicMemory * symbolicMemory, ConcreteMemory * concreteMemory)
{
	generic_arithmetic_handle(inst, symbolicMemory, concreteMemory);
}
void generic_arithmetic_handle(Instruction& inst, SymbolicMemory * symbolicMemory, ConcreteMemory * concreteMemory)
{
#ifndef RELEASE
	if (inst.num_operands != 2 && inst.num_operands != 1) throw Error("generic_arithmetic_handle num_operands != 2");
#endif
	switch (inst.num_operands)
	{
	case 2:
		symbolicMemory->put(inst.operands[0],
			Expression::create(inst.operater, 2, inst.operandExpressions[0], inst.operandExpressions[1])
			);
		break;
	case 1:
		symbolicMemory->put(inst.operands[0],
			Expression::create(inst.operater, 1, inst.operandExpressions[0])
			);
		break;
	}
}
void scasb_handle(Instruction& inst, SymbolicMemory * symbolicMemory, ConcreteMemory * concreteMemory)
{
	nop_handle(inst, symbolicMemory, concreteMemory);
}
void mul_handle(Instruction& inst, SymbolicMemory * symbolicMemory, ConcreteMemory * concreteMemory)
{
	if (inst.num_operands != 1 && inst.num_operands != 2 && inst.num_operands != 3) throw Error("MUL unfinished");
	if (inst.num_operands == 2) return generic_arithmetic_handle(inst, symbolicMemory, concreteMemory);
	if (inst.num_operands == 3)
	{
		symbolicMemory->put(
			inst.operands[0],
			Expression::create(
			MUL, 2,
			inst.operandExpressions[1],
			inst.operandExpressions[2]
			)
			);
	}
	switch (inst.operands[0].length)
	{
	case 1:
		symbolicMemory->put(Operand(ax),
			Expression::create(MUL, 2, symbolicMemory->get(Operand(al)), inst.operandExpressions[0])
			);
		break;
	case 2:
		symbolicMemory->put(Operand(ax),
			Expression::create(MUL, 2, symbolicMemory->get(Operand(ax)), inst.operandExpressions[0])
			);
		//FIXME:Wrong
		break;
	case 4:
		symbolicMemory->put(Operand(eax),
			Expression::create(MUL, 2, symbolicMemory->get(Operand(eax)), inst.operandExpressions[0])
			);
		//FIXME:Wrong
		break;
	default:
		throw Error("MUL error");
	}
}
void div_handle(Instruction& inst, SymbolicMemory * symbolicMemory, ConcreteMemory * concreteMemory)
{
	if (inst.num_operands != 1) throw Error("MUL unfinished");
	Expression * q, *r;
	switch (inst.operands[0].length)
	{
	case 1:
		q = Expression::create(DIV, 2, symbolicMemory->get(ax), inst.operandExpressions[0]);
		r = Expression::create(MOD, 2, symbolicMemory->get(ax), inst.operandExpressions[0]);
		symbolicMemory->put(al, q);
		symbolicMemory->put(ah, r);
		break;
	case 2:
		q = Expression::create(DIV, 2, symbolicMemory->get(ax), inst.operandExpressions[0]);
		r = Expression::create(MOD, 2, symbolicMemory->get(ax), inst.operandExpressions[0]);
		symbolicMemory->put(ax, q);
		symbolicMemory->put(dx, r);
		//FIXME:Wrong
		break;
	case 4:
		q = Expression::create(DIV, 2, symbolicMemory->get(eax), inst.operandExpressions[0]);
		r = Expression::create(MOD, 2, symbolicMemory->get(eax), inst.operandExpressions[0]);
		symbolicMemory->put(eax, q);
		symbolicMemory->put(edx, r);
		//FIXME:Wrong
		break;
	default:
		throw Error("DIV error");
	}
}
void cdq_handle(Instruction& inst, SymbolicMemory * symbolicMemory, ConcreteMemory * concreteMemory)
{
	//FIXME:UNFINISHED
	nop_handle(inst, symbolicMemory, concreteMemory);
}
void SymbolicExecute(const Instruction& inst123, class SymbolicMemory * symbolicMemory, class ConcreteMemory * concreteMemory)
{
	Instruction inst = inst123;
	if (inst.operater == NOP)
	{
		nop_handle(inst, symbolicMemory, concreteMemory);
		return;
	}
	fillOperandExpressions(inst, symbolicMemory, concreteMemory);
	for (unsigned int i = 0; i < inst.num_operands; i++)
	{
		inst.operandExpressions[i]->addRefCount();
	}
	switch (inst.operater)
	{
	case Operater::MOV: case LEA:
		mov_handle(inst, symbolicMemory, concreteMemory);
		break;
	case Operater::XCHG:
		xchg_handle(inst, symbolicMemory, concreteMemory);
		break;
	case AND:
		and_handle(inst, symbolicMemory, concreteMemory);
		break;
	case OR:
		or_handle(inst, symbolicMemory, concreteMemory);
		break;
	case XOR:
		xor_handle(inst, symbolicMemory, concreteMemory);
		break;
	case MUL:
		mul_handle(inst, symbolicMemory, concreteMemory);
		break;
	case DIV:
		div_handle(inst, symbolicMemory, concreteMemory);
		break;
	case INC: case DEC:
	case ADD: case SUB:
	case NEG: case NOT:
	case SHR: case SHL:
	case ROR: case ROL:
		generic_arithmetic_handle(inst, symbolicMemory, concreteMemory);
		break;
	case PUSH:
		push_handle(inst, symbolicMemory, concreteMemory);
		break;
	case POP:
		pop_handle(inst, symbolicMemory, concreteMemory);
		break;

	case SCASB:
		scasb_handle(inst, symbolicMemory, concreteMemory);
		break;
	case CDQ:
		cdq_handle(inst, symbolicMemory, concreteMemory);
		break;
	case NOP:
		nop_handle(inst, symbolicMemory, concreteMemory);
		break;
	default:
		throw Error("SymbolicExecute: Unrecognized Operater");
	}
	for (unsigned int i = 0; i < inst.num_operands; i++)
	{
		inst.operandExpressions[i]->decRefCount();
		inst.operandExpressions[i] = nullptr;
	}
}