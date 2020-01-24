#pragma once

#include "base.hpp"

namespace d88
{
	namespace analysis
	{
		/*
			Working Example Mod 8

			7		1		3		4

			A		B		C		D			3
													5
			?		?		?		?			0		3	
													0		3
			?		?		?		?			0		6
													6
			?		?		?		?			6


			7A = 3 => A = 5
			3 + B = 3 => B = 0
			3 + 0 + 3C = 5 => C = 6
			3 + 0 + 2 + 4D = 3 => D = 0


			...

			5		0		6		0
			
			3		5		2		6

			5		6		3		4

			3		7		3		7


			Nested Pascal surprisingly is the answer:

			3						* 7 = 5
				0					* 1 = 0
			3		2				* 3 = 6
				2		6			* 4 = 0
			5		0
				6
			3
		*/

		template <typename T> void extract_symmetry(const span<T> & data, const span<T>& poly, const span<T>& temp, const span<T>& sym,const PascalTriangle<T> & pt)
		{
			ToPascal<T>(data, temp, pt);
			ToPascalPolarR<T>(temp, sym, pt);

			for (size_t i = 0; i < data.size(); i++)
				sym[i] = GetInverse<T>(poly[i]) * sym[i];
		}
	}
}