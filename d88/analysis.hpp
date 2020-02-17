/* Copyright (C) 2020 D8DATAWORKS - All Rights Reserved */

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

		//SYMMETRY
		/*
			A	B	C	D
			a	b	c	d

			aD

			aC + bD

			aB + bC + cD

			aA + bB + cC + dD
		*/

		/*template <typename T> void force(T o, T t, T l)
		{
			for (T i = 1; i < l; i++)
			{
				if (((o * i) % l) == t)
				{
					std::cout << i << std::endl;
				}
			}
		}*/

		/*DerivativeR(temp,span<T>(sym.data()+1,sym.size()-1));
		sym[0] = temp[temp.size()-1];

		for (size_t i = 0; i < data.size(); i++)
		{
			force<int>(poly[i], 1, 256);
			sym[i] = GetInverse<T>(poly[i]) * sym[i];
		}*/

		template <typename T> void extract_symmetry(const span<T> & data, const span<T>& poly, const span<T>& temp, const span<T>& sym,const PascalTriangle<T> & pt)
		{
			ToPascal<T>(data, temp, pt);

			T inv = GetInverse<T>(poly[data.size()-1]);

			for (size_t i = 0, k = data.size() - 1; i < data.size(); k--, i++)
			{
				sym[i] = temp[k];

				for (size_t j = 0; j < i; j++)
					sym[i] -= sym[j] * poly[k + j];

				sym[i] *= inv;
			}
		}
	}
}