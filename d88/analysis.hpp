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

		template <typename T, typename SHIM> void extract_symmetry(const SHIM& data, const SHIM& poly, const span<T>& temp, const span<T>& sym,const PascalTriangle<T> & pt)
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

		//This prevents the need to refactor the poly to prevent invalid inverse for the constant polynomial term for our mod (2^N).
		//It also protects against a zero tail in the data that prevents the algorithm from working.
		//The easiest implementation requires the data to be copied. More elaborate implementations might shim in the padding.
		//At scale a memcpy would start to show in benchmarking... so lets use the shim
		//

		template<typename T,size_t EX = 1, T VAL = 1> class shim_span
		{
		public:
			shim_span() {}
			template <typename I> shim_span(const I & i) : s((T*)i.data()), l(i.size()) { }
			shim_span(T* _s, size_t _l) : s(_s), l(_l) { }

			//We can't provide this because of the spoofing:
			//

			//const T* begin() const { return s; }
			//const T* end() const { return s+l; }

			//T* data() { return s; } 
			size_t size() const { return l + EX; }

			const T& operator[] (size_t dx) const
			{
				return (dx >= l) ? val : *(s + dx);
			}

		private:
			T val = VAL;
			T* s = nullptr;
			size_t l = 0;
		};

		//temp BUFFER must be data.size()+1, pt must be data.size()+1:
		//

		template <typename T> void padded_symmetry(const span<T>& data, const span<T>& poly, const span<T>& temp, const span<T>& sym, const PascalTriangle<T>& pt)
		{
			extract_symmetry<T>(shim_span<T>(data), shim_span<T>(poly), temp, sym, pt);
		}
	}
}