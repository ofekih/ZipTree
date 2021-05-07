#ifndef CDOUBLE_H
#define CDOUBLE_H

#include <limits>

struct CDouble
{
	static constexpr double EQ_THRESHOLD = std::numeric_limits<double>::epsilon();

	CDouble(double d);
	CDouble(const CDouble& d);

	bool operator>=(CDouble d) const noexcept;
	bool operator< (CDouble d) const noexcept;
	bool operator==(CDouble d) const noexcept;
	CDouble operator-(CDouble d) const noexcept;
	CDouble operator-=(CDouble d) noexcept;

	double val;
};

#endif