#include "stdafx.h"
#include "fol.hpp"

// ----------------------------------------------------------------------------
// AtomicFormula

Formula AtomicFormula::releaseIff()
{
	return shared_from_this();
}

Formula AtomicFormula::absorbConstants()
{
	return shared_from_this();
}

// END AtomicFormula
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// LogicConstant

// END LogicConstant
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// True

void True::printFormula(ostream & ostr) const
{
	ostr << "true";
}

BaseFormula::Type True::getType() const
{
	return T_TRUE;
}

// it is possible to regard true as an abbreviation for the formula p \/ ~p
Formula True::transformToDisjunction() const
{
	Formula p = make_shared<Atom>("p");
	return make_shared<Or>(p, make_shared<Not>(p));
}

// END True
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// False

void False::printFormula(ostream & ostr) const
{
	ostr << "false";
}

BaseFormula::Type False::getType() const
{
	return T_FALSE;
}

// it is possible to regard false as an abbreviation for the formula p /\ ~p
Formula False::transformToConjunction() const
{
	Formula p = make_shared<Atom>("p");
	return make_shared<And>(p, make_shared<Not>(p));
}

// END False
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Atom

Atom::Atom(const PredicateSymbol & p,
	const vector<Term> & ops)
	:_p(p),
	_ops(ops)
{}

const PredicateSymbol & Atom::getSymbol() const
{
	return _p;
}

const vector<Term> & Atom::getOperands() const
{
	return _ops;
}

void Atom::printFormula(ostream & ostr) const
{
	ostr << _p;
	for (unsigned i = 0; i < _ops.size(); i++)
	{
		if (i == 0)
			ostr << "(";
		_ops[i]->printTerm(ostr);
		if (i != _ops.size() - 1)
			ostr << ",";
		else
			ostr << ")";
	}
}

BaseFormula::Type Atom::getType() const
{
	return T_ATOM;
}

// END Atom
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// UnaryConjective

UnaryConjective::UnaryConjective(const Formula & op)
	:_op(op)
{}

const Formula & UnaryConjective::getOperand() const
{
	return _op;
}

// END UnaryConjective
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Not

Not::Not(const Formula & op)
	:UnaryConjective(op)
{}

void Not::printFormula(ostream & ostr) const
{
	ostr << "~";
	Type op_type = _op->getType();

	if (op_type == T_AND || op_type == T_OR ||
		op_type == T_IMP || op_type == T_IFF)
		ostr << "(";

	_op->printFormula(ostr);

	if (op_type == T_AND || op_type == T_OR ||
		op_type == T_IMP || op_type == T_IFF)
		ostr << ")";
}

BaseFormula::Type Not::getType() const
{
	return T_NOT;
}

Formula Not::releaseIff()
{
	Formula releasedIffOp = _op->releaseIff();
	return make_shared<Not>(releasedIffOp);
}

Formula Not::absorbConstants()
{
	Formula absOp = _op->absorbConstants();

	if (absOp->getType() == T_TRUE)
	{
		return make_shared<False>();
	}
	else if (absOp->getType() == T_FALSE)
	{
		return make_shared<True>();
	}
	else
	{
		return make_shared<Not>(absOp);
	}
}

// END Not
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// BinaryConjective

BinaryConjective::BinaryConjective(const Formula & op1, const Formula & op2)
	:_op1(op1),
	_op2(op2)
{}

const Formula & BinaryConjective::getOperand1() const
{
	return _op1;
}

const Formula & BinaryConjective::getOperand2() const
{
	return _op2;
}

// END BinaryConjective
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// And

And::And(const Formula & op1, const Formula & op2)
	:BinaryConjective(op1, op2)
{}

void And::printFormula(ostream & ostr) const
{
	Type op1_type = _op1->getType();
	Type op2_type = _op2->getType();

	if (op1_type == T_OR || op1_type == T_IMP ||
		op1_type == T_IFF)
		ostr << "(";

	_op1->printFormula(ostr);

	if (op1_type == T_OR || op1_type == T_IMP ||
		op1_type == T_IFF)
		ostr << ")";

	ostr << " & ";

	if (op2_type == T_OR || op2_type == T_IMP ||
		op2_type == T_IFF || op2_type == T_AND)
		ostr << "(";

	_op2->printFormula(ostr);

	if (op2_type == T_OR || op2_type == T_IMP ||
		op2_type == T_IFF || op2_type == T_AND)
		ostr << ")";
}

BaseFormula::Type And::getType() const
{
	return T_AND;
}

Formula And::releaseIff()
{
	Formula releasedIffOp1 = _op1->releaseIff();
	Formula releasedIffOp2 = _op2->releaseIff();
	return make_shared<And>(releasedIffOp1, releasedIffOp2);
}

Formula And::absorbConstants()
{
	Formula absOp1 = _op1->absorbConstants();
	Formula absOp2 = _op2->absorbConstants();

	if (absOp1->getType() == T_FALSE || absOp2->getType() == T_FALSE)
	{
		return make_shared<False>();
	}
	else if (absOp1->getType() == T_TRUE)
	{
		return absOp2;
	}
	else if (absOp2->getType() == T_TRUE)
	{
		return absOp1;
	}
	else
	{
		return make_shared<And>(absOp1, absOp2);
	}
}

// END And
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Or

Or::Or(const Formula & op1, const Formula & op2)
	:BinaryConjective(op1, op2)
{}

void Or::printFormula(ostream & ostr) const
{
	Type op1_type = _op1->getType();
	Type op2_type = _op2->getType();

	if (op1_type == T_IMP || op1_type == T_IFF)
		ostr << "(";

	_op1->printFormula(ostr);

	if (op1_type == T_IMP || op1_type == T_IFF)
		ostr << ")";

	ostr << " | ";

	if (op2_type == T_IMP ||
		op2_type == T_IFF || op2_type == T_OR)
		ostr << "(";

	_op2->printFormula(ostr);

	if (op2_type == T_IMP ||
		op2_type == T_IFF || op2_type == T_OR)
		ostr << ")";
}

BaseFormula::Type Or::getType() const
{
	return T_OR;
}

Formula Or::releaseIff()
{
	Formula releasedIffOp1 = _op1->releaseIff();
	Formula releasedIffOp2 = _op2->releaseIff();
	return make_shared<Or>(releasedIffOp1, releasedIffOp2);
}

Formula Or::absorbConstants()
{
	Formula absOp1 = _op1->absorbConstants();
	Formula absOp2 = _op2->absorbConstants();

	if (absOp1->getType() == T_TRUE || absOp2->getType() == T_TRUE)
	{
		return make_shared<True>();
	}
	else if (absOp1->getType() == T_FALSE)
	{
		return absOp2;
	}
	else if (absOp2->getType() == T_FALSE)
	{
		return absOp1;
	}
	else
	{
		return make_shared<Or>(absOp1, absOp2);
	}
}

// END Or
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Imp

Imp::Imp(const Formula & op1, const Formula & op2)
	:BinaryConjective(op1, op2)
{}

void Imp::printFormula(ostream & ostr) const
{
	Type op1_type = _op1->getType();
	Type op2_type = _op2->getType();

	if (op1_type == T_IFF)
		ostr << "(";

	_op1->printFormula(ostr);

	if (op1_type == T_IFF)
		ostr << ")";

	ostr << " => ";

	if (op2_type == T_IMP || op2_type == T_IFF)
		ostr << "(";

	_op2->printFormula(ostr);

	if (op2_type == T_IMP || op2_type == T_IFF)
		ostr << ")";
}

BaseFormula::Type Imp::getType() const
{
	return T_IMP;
}

Formula Imp::releaseIff()
{
	Formula releasedIffOp1 = _op1->releaseIff();
	Formula releasedIffOp2 = _op2->releaseIff();
	return make_shared<Imp>(releasedIffOp1, releasedIffOp2);
}

Formula Imp::absorbConstants()
{
	Formula absOp1 = _op1->absorbConstants();
	Formula absOp2 = _op2->absorbConstants();

	if (absOp1->getType() == T_TRUE)
	{
		return absOp2;
	}
	else if (absOp2->getType() == T_TRUE)
	{
		return make_shared<True>();
	}
	else if (absOp1->getType() == T_FALSE)
	{
		return make_shared<True>();
	}
	else if (absOp2->getType() == T_FALSE)
	{
		return make_shared<Not>(absOp1);
	}
	else
	{
		return make_shared<Imp>(absOp1, absOp2);
	}
}

// END Imp
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Iff

Iff::Iff(const Formula & op1, const Formula & op2)
	:BinaryConjective(op1, op2)
{}

void Iff::printFormula(ostream & ostr) const
{
	Type op1_type = _op1->getType();
	Type op2_type = _op2->getType();

	_op1->printFormula(ostr);

	ostr << " => ";

	if (op2_type == T_IFF)
		ostr << "(";

	_op2->printFormula(ostr);

	if (op2_type == T_IFF)
		ostr << ")";
}

BaseFormula::Type Iff::getType() const
{
	return T_IFF;
}

Formula Iff::releaseIff()
{
	Formula releasedIffOp1 = _op1->releaseIff();
	Formula releasedIffOp2 = _op2->releaseIff();
	return make_shared<And>(
		make_shared<Imp>(releasedIffOp1, releasedIffOp2),
		make_shared<Imp>(releasedIffOp2, releasedIffOp1)
		);
}

Formula Iff::absorbConstants()
{
	Formula absOp1 = _op1->absorbConstants();
	Formula absOp2 = _op2->absorbConstants();

	if (absOp1->getType() == T_FALSE || absOp2->getType() == T_FALSE)
	{
		return make_shared<True>();
	}
	else if (absOp1->getType() == T_TRUE)
	{
		return absOp2;
	}
	else if (absOp2->getType() == T_TRUE)
	{
		return absOp1;
	}
	else if (absOp1->getType() == T_FALSE)
	{
		return make_shared<Not>(absOp2);
	}
	else if (absOp2->getType() == T_FALSE)
	{
		return make_shared<Not>(absOp1);
	}
	else
	{
		return make_shared<Iff>(absOp1, absOp2);
	}
}

// END Iff
// ----------------------------------------------------------------------------
