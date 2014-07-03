#pragma once
#include "Operand.h"
Operand operandStrToOperand(const char * operandsStr)
{
	Operand retv;
	retv.length = 0;
	unsigned int len = strlen(operandsStr);

	if (operandsStr[0] == '[')
	{
		retv.num_indirectRegisters = 0;
		retv.value = 0;
		int multiplier = 1;
		int startPos = 1;//当前匹配到表达式的元素的开始位置
		for (unsigned int i = 1; i < len; i++)
		{
			if (operandsStr[i] == '+' || operandsStr[i] == '-' || operandsStr[i] == ']')
			{
				if (i - startPos == 0)
				{
					if (operandsStr[i] == '-') multiplier = -1;
					else multiplier = 1;
					startPos = i + 1;
					continue;
				}

				unsigned int j;
				for (j = startPos; j < i; j++)
				if (operandsStr[j] == '*')
					break;

				if (i == j)
				{
					bool isReg = false;//中间不包括乘号的话，可能是IMM也可能是寄存器
					for (unsigned int k = startPos; k < j; k++)
					{
						if (operandsStr[k] < '0' || (operandsStr[k] > '9' && operandsStr[k] < 'a') || operandsStr[k] > 'f')
						{
							isReg = true;
							break;
						}
					}
					if (isReg)
					{
						retv.indirectMultiplier[retv.num_indirectRegisters] = multiplier;
						retv.indirectRegisters[retv.num_indirectRegisters] =
							stringToRegister(&operandsStr[startPos], j - startPos);
						retv.num_indirectRegisters += 1;
					}
					else
					{
						sscanf(&operandsStr[startPos], "%lx", &retv.value);
						retv.value *= multiplier;
					}
				}
				else
				{
					sscanf(&operandsStr[j + 1], "%lu", &retv.indirectMultiplier[retv.num_indirectRegisters]);
					retv.indirectMultiplier[retv.num_indirectRegisters] *= multiplier;
					retv.indirectRegisters[retv.num_indirectRegisters] =
						stringToRegister(&operandsStr[startPos], j - startPos);
					retv.num_indirectRegisters += 1;
				}
				startPos = i + 1;
				if (operandsStr[i] == '-') multiplier = -1;
			}
		}
		retv.type = (retv.num_indirectRegisters ? INDIRECT_MEM : DIRECT_MEM);
	}
	else
	{
		unsigned int i = 0;
		for (i = 0; i < len; i++)
		{
			if (operandsStr[i] < '0' || (operandsStr[i] >'9' && operandsStr[i] < 'a') || operandsStr[i] > 'f')
				break;
		}
		if (i == len)
		{
			retv.type = IMM;
			sscanf(operandsStr, "%lx", &retv.value);
		}
		else
		{
			retv.type = REG;
			retv.value = stringToRegister(operandsStr);
			retv.length = registerLength(Register(retv.value));
		}
	}
	return retv;
}

bool itraceParse(Instruction * destInst, const char * lineBuffer, ConcreteMemory * concreteMemory)
{
	unsigned int regs[8];
	unsigned int instAddr;
	unsigned int threadId;
	char instStr[128];
	char memStr[128];

	int fscanf_retv = 0;
	fscanf_retv = sscanf(lineBuffer, "%lx#%lu|%[^|]|%[^|]|%lx, %lx, %lx, %lx, %lx, %lx, %lx, %lx\n",
		&instAddr, &threadId, instStr, memStr,
		&regs[0], &regs[1], &regs[2], &regs[3], &regs[4], &regs[5], &regs[6], &regs[7]);
	if (fscanf_retv == 3)
	{
		strcpy(memStr, "");
		fscanf_retv = sscanf(lineBuffer, "%lx#%lu|%[^|]||%lx, %lx, %lx, %lx, %lx, %lx, %lx, %lx\n",
			&instAddr, &threadId, instStr,
			&regs[0], &regs[1], &regs[2], &regs[3], &regs[4], &regs[5], &regs[6], &regs[7]);
		fscanf_retv += 1;
	}
	if (fscanf_retv != 12) throw Error("Parser Error");

	concreteMemory->put(Operand(eax), regs[0]);
	concreteMemory->put(Operand(ebx), regs[1]);
	concreteMemory->put(Operand(ecx), regs[2]);
	concreteMemory->put(Operand(edx), regs[3]);
	concreteMemory->put(Operand(edi), regs[4]);
	concreteMemory->put(Operand(esi), regs[5]);
	concreteMemory->put(Operand(ebp), regs[6]);
	concreteMemory->put(Operand(esp), regs[7]);

	Instruction& inst = *(destInst);
	char operaterStr[8];
	char operandStrs[MAX_NUM_OPERANDS][32];
	int num_operandStrs = 0;
	//跳过最后一个空格
	for (unsigned int i = 0; i < strlen(instStr) - 1; i++)
	{
		if (instStr[i] == ' ') num_operandStrs += 1;
	}

	switch (num_operandStrs)
	{
	case 0:
		sscanf(instStr, "%s ", operaterStr);
		break;
	case 1:
		sscanf(instStr, "%s %s", operaterStr, operandStrs[0]);
		break;
	case 2:
		sscanf(instStr, "%s %[^,], %s", operaterStr, operandStrs[0], operandStrs[1]);
		break;
	case 3:
		sscanf(instStr, "%s %[^,], %[^,], %s", operaterStr, operandStrs[0], operandStrs[1], operandStrs[2]);
		break;
	case 4:
		sscanf(instStr, "%s %[^,], %[^,], %[^,], %s", operaterStr, operandStrs[0], operandStrs[1], operandStrs[2], operandStrs[3]);
		break;
	default:
		printf("%s", instStr);
		std::cout << instStr;
		std::cout.flush();
		throw Error("too much operands");
	}
	int memLen = (strlen(memStr) ? memStr[1] - '0' : 0);

	//inst.Operater generation
	if (strcmp(operaterStr, "stosb") == 0)
	{
		strcpy(operaterStr, "mov");
		strcpy(operandStrs[num_operandStrs++], "al");
		memLen = 1;
	}
	if (strcmp(operaterStr, "stosw") == 0)
	{
		strcpy(operaterStr, "mov");
		strcpy(operandStrs[num_operandStrs++], "ax");
		memLen = 2;
	}
	if (strcmp(operaterStr, "stosd") == 0)
	{
		strcpy(operaterStr, "mov");
		strcpy(operandStrs[num_operandStrs++], "eax");
		memLen = 4;
	}

	if (strcmp(operaterStr, "mov") == 0 ||
		strcmp(operaterStr, "movsd") == 0 ||
		strcmp(operaterStr, "movsb") == 0 ||
		strcmp(operaterStr, "movsx") == 0 ||
		strcmp(operaterStr, "movzd") == 0 ||
		strcmp(operaterStr, "movzb") == 0 ||
		strcmp(operaterStr, "movzx") == 0)
	{
		inst.operater = Operater::MOV;
	}
	else if (strcmp(operaterStr, "lea") == 0)
		inst.operater = Operater::LEA;
	else if (strcmp(operaterStr, "xchg") == 0)
		inst.operater = Operater::XCHG;
	else if (strcmp(operaterStr, "imul") == 0 ||
		strcmp(operaterStr, "imul") == 0)
		inst.operater = Operater::MUL;
	else if (strcmp(operaterStr, "idiv") == 0 ||
		strcmp(operaterStr, "div") == 0)
		inst.operater = Operater::DIV;

	else if (strcmp(operaterStr, "push") == 0)
		inst.operater = Operater::PUSH;
	else if (strcmp(operaterStr, "pop") == 0)
		inst.operater = Operater::POP;

	else if (strcmp(operaterStr, "add") == 0)
		inst.operater = Operater::ADD;
	else if (strcmp(operaterStr, "sub") == 0 ||
		strcmp(operaterStr, "sbb") == 0)
		inst.operater = Operater::SUB;
	else if (strcmp(operaterStr, "inc") == 0)
		inst.operater = Operater::INC;
	else if (strcmp(operaterStr, "dec") == 0)
		inst.operater = Operater::DEC;

	else if (strcmp(operaterStr, "or") == 0)
		inst.operater = Operater::OR;
	else if (strcmp(operaterStr, "and") == 0)
		inst.operater = Operater::AND;
	else if (strcmp(operaterStr, "xor") == 0)
		inst.operater = Operater::XOR;

	else if (strcmp(operaterStr, "shl") == 0 ||
		strcmp(operaterStr, "sal") == 0)
		inst.operater = Operater::SHL;
	else if (strcmp(operaterStr, "shr") == 0 ||
		strcmp(operaterStr, "sar") == 0)
		inst.operater = Operater::SHR;
	else if (strcmp(operaterStr, "ror") == 0)
		inst.operater = Operater::ROR;
	else if (strcmp(operaterStr, "rol") == 0)
		inst.operater = Operater::ROL;

	else if (strcmp(operaterStr, "neg") == 0)
		inst.operater = Operater::NEG;
	else if (strcmp(operaterStr, "not") == 0)
		inst.operater = Operater::NOT;

	else if (strcmp(operaterStr, "scasb") == 0)
		inst.operater = Operater::SCASB;

	else if (strcmp(operaterStr, "cdq") == 0)
		inst.operater = Operater::CDQ;
	else if (strcmp(operaterStr, "test") == 0)
		inst.operater = Operater::NOP;
	else if (strcmp(operaterStr, "cmp") == 0 || strcmp(operaterStr, "cmpsb") == 0)
		inst.operater = Operater::NOP;
	else if (strcmp(operaterStr, "call") == 0)
		inst.operater = Operater::NOP;
	else if (strcmp(operaterStr, "jmp") == 0)
		inst.operater = Operater::NOP;
	else if (strcmp(operaterStr, "ret") == 0)
		inst.operater = Operater::NOP;
	else if (strcmp(operaterStr, "setz") == 0) // FIXME:WRONG
		inst.operater = Operater::NOP;
	else if (strcmp(operaterStr, "setle") == 0) // FIXME:WRONG
		inst.operater = Operater::NOP;
	else if (strlen(operaterStr) >= 3 && operaterStr[0] == 's' && operaterStr[1] == 'e' && operaterStr[2] == 't')//FIXME
		inst.operater = Operater::NOP;
	else if (strcmp(operaterStr, "leave") == 0) // FIXME:WRONG
		inst.operater = Operater::NOP;
	else if (operaterStr[0] == 'f') // FIXME:WRONG
		inst.operater = Operater::NOP;
	else if (strcmp(operaterStr, "std") == 0 ||
		strcmp(operaterStr, "sti") == 0 ||
		strcmp(operaterStr, "cld") == 0 ||
		strcmp(operaterStr, "cli") == 0)
	{
		inst.operater = Operater::NOP;
	}
	else if (strcmp(operaterStr, "bt") == 0) // FIXME:WRONG
		inst.operater = Operater::NOP;
	else if (strcmp(operaterStr, "nop") == 0)
		inst.operater = Operater::NOP;
	else if (operaterStr[0] == 'j')
		inst.operater = Operater::NOP;
	else
	{
		char buf[1024];
		strcpy(buf, "Unfinished ");
		strcat(buf, operaterStr);
		throw Error(buf);
	}

	if (inst.operater == NOP)
	{
		inst.num_operands = 0;
		return true;
	}
	inst.num_operands = num_operandStrs;
	//inst.operands
	for (int i = 0; i < num_operandStrs; i++)
	{
		inst.operands[i] = operandStrToOperand(operandStrs[i]);
		if (strlen(operaterStr) == 5 && inst.operater == MOV)
		{
			if (operaterStr[4] == 'x' && inst.operands[i].length == 0)
				inst.operands[i].length = memLen;
		}
		if (inst.operands[i].length == 0)
		{
			inst.operands[i].length = memLen;
		}
	}
	return true;
}