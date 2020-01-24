/* Copyright (C) 2020 D8DATAWORKS - All Rights Reserved */

#pragma once

#include "base.hpp"
#include "util.hpp"

namespace d88
{

	template < typename T > void factor(const span<T>& poly)
	{
		static const size_t itr = 64;
		auto u = poly.size();
		vector<T> l(u*itr);

		for (size_t i = 0; i < u-2; i++)
			l[i] = 0;

		l[u - 2] = 1;

		for (size_t i = 0; i < u * itr - u + 1; i++)
		{
			size_t sum = 0;

			for (size_t j = 0, k = u - 1; j < u - 1; j++, k--)
				sum += poly[k] * l[i + j];

			l[u + i - 1] = sum;
		}

		PrintRow(l);

		auto r1 = GetInverse<T>(69);
		T test = 212;
		test *= r1;

		return;
	}
}