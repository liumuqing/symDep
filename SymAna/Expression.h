#pragma once
#include <unordered_set>

#include "Operand.h"
class Expression
{

public:

	static void (*onConstruct)(Expression *);
	static void (*onUnconstruct)(Expression *);
	static void (*onDecRefCount)(Expression *);


	static class Expression* create(const enum Operater& operater, unsigned int num_operands = 0, 
										...);
	static class Expression* create(const struct Operand& operand);
	static class Expression* create(class Expression * expression);

	const char * getStr();


	unsigned int num_operands;
	class Expression * operands[MAX_NUM_OPERANDS];
	std::unordered_multiset<class Expression *> parents;
	enum Operater operater;
	Operand operand;

	enum ExpressionType
	{
		EXPRESSION, VALUE, NONE
	} type;


	int refCount = 0;
#ifdef MEMLEAK_CHECK
	int maxRefCount = 0;//refCount打到过的最大值，值为0表明这个节点内存泄露了。。
#endif
	void checkRefCount(){ if (refCount <= 0) delete this; }
	void addRefCount(){ 
		refCount += 1; 
#ifdef MEMLEAK_CHECK
		maxRefCount = (refCount > maxRefCount)?refCount:maxRefCount;
#endif
	}
	void decRefCount(){ 
		refCount -= 1; 
		if (onDecRefCount) onDecRefCount(this);
		if (refCount <= 0) delete this;}

	//void * data = nullptr; 
	std::unordered_set<Expression *> * data = nullptr;
	int deep = 0;


	static Expression noneExpression;

	static void init();

private:
	//私有构造函数，避免绕过create函数直接构造表达式
	Expression(const Operater& operater, unsigned int num_operands = 0, const  Expression ** operands = nullptr);
	Expression(const Operand& operand);
	Expression();
	~Expression();

};





































































































