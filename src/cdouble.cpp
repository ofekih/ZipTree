#include "cdouble.h"

#include <algorithm>

CDouble::CDouble(double d)
	: val(d)
{
}

CDouble::CDouble(const CDouble& d)
	: val(d.val)
{
}

bool CDouble::operator>=(CDouble d) const noexcept
{
	return val >= d.val - EQ_THRESHOLD;
}

bool CDouble::operator<(CDouble d) const noexcept
{
	return val < d.val - EQ_THRESHOLD;
}

bool CDouble::operator==(CDouble d) const noexcept
{
	return std::abs(val - d.val) <= EQ_THRESHOLD;
}

CDouble CDouble::operator-(CDouble d) const noexcept
{
	return val - d.val;
}

CDouble CDouble::operator-=(CDouble d) noexcept
{
	val -= d.val;
	return *this;
}

