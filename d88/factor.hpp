/* Copyright (C) 2020 D8DATAWORKS - All Rights Reserved */

#pragma once

#include "base.hpp"
#include "util.hpp"

#include <intrin.h>
#pragma intrinsic(_umul128)

#include <array>

namespace d88
{
	template <typename I, size_t N, size_t M> void poly_mul(const std::array<I,N> & o, const std::array<I, M> & t, std::array<I, N+M-1 > & r)
	{
		for (size_t i = 0; i < r.size(); i++)
			r[i] = 0;

		for (size_t i = 0; i < N; i++)
		{
			for (size_t j = 0; j < M; j++)
			{
				r[i + j] += o[i] * t[j];
			}
		}
	}

	//WARNING: Little Endian
	template <typename S, typename L> auto mut(const S& s1, const S& s2)
	{
		union
		{
			S s[2];
			L l;
		};

		l = (L)s1 * (L)s2;

		return std::make_pair(s[1], s[0]);
	}

	template < typename T > auto mul(const T& t1, const T& t2)
	{
		if constexpr (std::is_same<T, uint8_t>())
			return mut<T, uint16_t>(t1, t2);
		if constexpr (std::is_same<T, uint16_t>())
			return mut<T, uint32_t>(t1, t2);
		if constexpr (std::is_same<T, uint32_t>())
			return mut<T, uint64_t>(t1, t2);
		if constexpr (std::is_same<T, uint64_t>())
		{
			uint64_t lw, hh;
			lw = _umul128(t1, t2, &hh);

			return std::make_pair(hh, lw);
		}
	}

	template < typename T1, typename T2 > bool add(T1& t1, const T2& t2)
	{
		t1 += t2;
		return t2 > t1;
	}

	//TODO intrinsic https://stackoverflow.com/questions/29229371/addcarry-u64-and-addcarryx-u64-with-msvc-and-icc#:~:text=The%20documentation%20for%20MSVC%20lists,_addcarry_u64%20has%20no%20listed%20technology.&text=The%20_addcarry_u64%20intrinsic%20documentation%20says,produce%20either%20adcx%20or%20adox%20.
	template < typename C, typename T > bool vad(C& c, size_t i, const T& v)
	{
		bool carry = add(c[i], v);

		while (carry && --i != -1)
			carry = add(c[i], T(1));

		return carry;
	}

	template <typename int_t, size_t N> void array_mul_element(const std::array<int_t, N>& array, size_t cap, const int_t & element, std::array<int_t, N>& output)
	{
		for (size_t i = 0; i < cap && i < N-1; i++)
		{
			auto [h, l] = mul(array[i], element);

			vad(output, i, l);
			vad(output, i + 1, h);
		}
	}

	/*template < typename int_t, size_t degree_s > void round(const int_t& x, const std::array<int_t, degree_s>& payload, std::array<int_t, degree_s>& output)
	{
		for (size_t i = 0; i < degree_s; i++)
		{
			array_mul_element
			output
			payload
		}
	}*/

	template <typename I, size_t N, size_t M> std::array<I, N + M - 1 > poly_mul(const std::array<I, N>& o, const std::array<I, M>& t)
	{
		std::array<I, N + M - 1 > r = { 0 };

		for (size_t i = 0; i < N; i++)
		{
			for (size_t j = 0; j < M; j++)
			{
				r[i + j] += o[i] * t[j];
			}
		}

		return r;
	}

	void matrix()
	{
		for (uint8_t i = 1; i != 0; i++)
		{
			for (uint8_t j = 1; j != 0; j++)
			{
				auto r = poly_mul<>(std::array<uint8_t, 2>{ 1, i }, std::array<uint8_t, 2>{ 1, j });
				std::cout << (int)r[0] << " " << (int)r[1] << " " << (int)r[2] << std::endl;
			}
		}
	}

	template <typename I, size_t N> I poly_at(const I& x, const std::array<I, N>& poly)
	{
		I result = 0;
		I pow = 1;
		for (auto i = poly.size() - 1; i != -1; i--, pow *= x) {
			result += poly[i] * pow;
		}

		return result;
	}

	template <typename I, size_t N, typename F> void poly_zero(const std::array<I, N>& poly, F && f, size_t range)
	{
		for (size_t i = 0; i < range; i++) {
			auto re = poly_at((I)i, poly);

			if (re == (I)0 && !f(i))
				break;
		}
	}

	template <typename I, size_t N, typename F> void poly_exec(const std::array<I, N>& poly, F&& f, size_t range)
	{
		for (size_t i = 0; i < range; i++) {
			auto re = poly_at((I)i, poly);

			if (!f(re)) break;
		}
	}

	template <size_t M, typename I, size_t N> auto poly_exec(const std::array<I, N>& poly)
	{
		size_t i = 0;
		std::array<I, M> result = { 0 };

		poly_exec(poly, [&](auto n) {
			result[i++] = n;

			return true;
			}, M);

		return result;
	}

	template <typename I, size_t N> std::array<I, N> poly_zero(const std::array<I, N>& poly, size_t range)
	{
		size_t i = 0;
		std::array<I, N> result = { 0 };

		poly_zero(poly, [&](auto n) {
			result[i++] = n;

			return true;
		}, range);

		return result;
	}

	template <typename I, size_t N> std::array<I, N> poly_factor(const std::array<I, N>& poly, size_t range)
	{
		size_t i = 0;
		std::array<I, N> result = { 0 };

		poly_zero(poly, [&](auto n) {
			result[i++] = 0-n;

			return true;
			}, range);

		return result;
	}
	
	template <typename I, size_t M> auto poly_div(const I & den, const std::array<I, M>& num)
	{
		std::array<I, M-1> result;
		auto n = num[0];

		for (size_t i = 0; i < M - 1; i++)
		{
			result[i] = n;

			n = num[i+1] - den * result[i];
		}

		return std::make_pair(result,n);
	}

	void ff_root()
	{

	}

	/*
	
	x^2 + bx + c = 15
	x^2 + bx + c = 25

	b + c = 14
	3b + c = 16
	3b + 3c = 42	
	     2c = 26
		 c = 13

	15
	19
	25
	
	*/


	template < typename T, size_t S > bool is_level(std::array<T, S>& v) {
		auto spec = spectrum(v);

		for (auto i : spec) {
			if (i > 1) return false;
		}
		return true;
	}

	template < size_t B,  typename T, size_t S > auto spectrum(const std::array<T, S>& v) {
		std::array<T, B> result = {};

		for (auto i : v)
			result[i % B] ++;

		return result;
	}

	template < typename T, size_t S > auto level2(std::array<T, S>& v) {
		std::array<T, S> result = {};

		for (auto& i : v) {
			if (result[i % S] ++)
				i++;
		}

		return result;
	}

	template < typename T, size_t S > auto level_all(std::array<T, S>& v) {
		auto spec = spectrum<S>(v);

		auto next_gap = [&](auto start) {
			for (size_t i = 0; i < v.size(); i++) {
				auto dx = (start + i) % S;
				if (spec[dx] == 0)
					return (T)dx;
			}

			return (T)-1;
		};

		for (size_t _i = 0; _i < v.size(); _i++) {
			auto i = (_i % 2) ? (S - _i) : _i;
			if (spec[v[i]]-- > 1)
				v[i] = next_gap(v[i]);
		}

		return spec;
	}

	template < typename T, size_t S, size_t R > auto map(const std::array<T, S>& v, const std::array<T, R>& lookup) {
		std::array<T, S> result = {};

		for (size_t i = 0; i < v.size(); i++)
			result[i] = lookup[v[i]];

		return result;
	}

	template < typename T, size_t S > auto lookup(const std::array<T, S>& v) {
		std::array<T, S> result = {};

		for (size_t i = 0; i < v.size(); i++)
			result[v[i]] = i;

		return result;
	}

	template < typename T, size_t S, size_t R > auto unmap(const std::array<T, S>& v, const std::array<T, R>& translation) {
		std::array<T, S> result = {};

		for (size_t i = 0; i < v.size(); i++)
			result[i] = translation[v[i]];

		return result;
	}

	/*
		
	*/

	void test()
	{
		auto printarr = [](auto a) {
			for (size_t i = 0; i < a.size(); i++) {
				std::cout << (uint64_t)a[i] << " ";
			}
			std::cout << std::endl;
		};

		auto buf = poly_exec<256>(std::array<uint8_t, 8>{ 1, 73, 167, 99, 32, 99, 45, 23 });
		auto spec1 = spectrum<256>(buf); 
		level_all(buf);
		auto spec2 = spectrum<256>(buf);

		auto l1 = lookup(buf);
		auto mm = map(std::array<uint8_t, 4>{ 12, 51, 129 ,2 }, l1);
		auto um = unmap(mm, buf);


		PascalTriangle<uint64_t> pt(5);
		std::array<uint64_t, 5> scratch;
		std::array<uint64_t, 5> source{ 9, 6, 7, 9, 4 };
		printarr(source);
		for (size_t i = 0; i < 8; i++) {
			ToPascal<uint64_t>(source, scratch, pt);
			printarr(scratch);
			ToPascal<uint64_t>(scratch, source, pt);
			printarr(source);
		}

		auto res = poly_mul<>(std::array<uint16_t, 2>{ 1, 57939 }, std::array<uint16_t, 2>{ 1, 7600 });
		auto res1 = poly_mul<>(std::array<uint16_t, 2>{ 1, 25171 }, res);
		auto res2 = poly_mul<>(std::array<uint16_t, 2>{ 1, 25171 }, std::array<uint16_t, 2>{ 1, 7600 });
		auto res3 = poly_mul<>(std::array<uint16_t, 2>{ 1, 57939 }, std::array<uint16_t, 2>{ 1, 25171 });

		auto z1 = poly_zero(std::array<uint16_t, 10>{ 1, 3, 6, 34663, 4574, 342, 7532, 9811, 19, 27 }, 256 * 256);

		auto z2 = poly_factor(std::array<uint16_t, 3>{ 1, 3, 16 }, 256 * 256);
		auto z3 = poly_zero(std::array<uint16_t, 3>{ 1, 2, 3 }, 256 * 256);

		auto res6 = poly_mul<>(std::array<uint16_t, 3>{ 1, 2, 3 }, std::array<uint16_t, 3>{ 1, 2, 3 });
		auto resz4 = poly_zero(res6, 256 * 256);

		auto asfdr = poly_mul<>(std::array<uint16_t, 3>{ 1, 3, 16 }, std::array<uint16_t, 3>{ 1, 2, 3 });
		auto [res4, n] = poly_div<>((uint16_t) 57939 , asfdr);
		auto [res5, n2] = poly_div<>((uint16_t) 7600, res4);
		auto z4 = poly_zero(asfdr, 256*256);

		/*asfdr = poly_mul2<>(std::array<uint8_t, 2>{ 1, 13 }, std::array<uint8_t, 2>{ 1, 74 });
		find_zero(asfdr, 256);

		asfdr = poly_mul2<>(std::array<uint8_t, 2>{ 1, 121 }, std::array<uint8_t, 2>{ 1, 8 });
		find_zero(asfdr, 256);*/

		/*for (size_t i = 1; i < 256; i+=2) {
			for (size_t j = 0; j < 256; j++) {
				poly_zero( poly_mul<>(std::array<uint8_t, 2>{ 1, (uint8_t) i }, std::array<uint8_t, 2>{ 1, (uint8_t) j }), 256);
			}
		}*/


		uint8_t test[256];
		uint16_t test2[256 * 256];

		auto print = [](auto a, size_t sz = 16) {
			for (size_t i = 0; i < sz; i++) {
				for (size_t j = 0; j < sz; j++) {
					std::cout << (int)a[i * sz + j] << " ";
				}

				std::cout << std::endl;
			}
			std::cout << std::endl;
		};

		auto init = [](auto a, size_t lim = 256) {
			for (size_t i = 0; i < lim; i++) {
				a[i] = i;
			}
		};

		auto iter = [&](auto a, size_t side= 16, size_t lim = 256, size_t depth=16) {
			for (size_t i = 0; i < depth; i++)
			{


				for (size_t i = 0; i < lim; i++) {
					a[i] *= i;
				}

				print(a, side);
			}
		};

		init(test);
		iter(test);
		//init(test2, 256 * 256);
		//iter(test2, 256, 256*256, 4);


		auto r = poly_mul<>(std::array<uint8_t, 2>{ 1, 1 }, std::array<uint8_t, 2>{ 1, 2 });
		auto r2 = poly_mul<>(r, std::array<uint8_t, 2>{ 1, 3 });

		matrix();
	}

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