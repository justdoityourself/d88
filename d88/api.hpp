/* Copyright (C) 2020 D8DATAWORKS - All Rights Reserved */

#pragma once

#include <array>
#include <fstream>
#include <string_view>

#include "../mio.hpp"
#include "../gsl-lite.hpp"
#include "correct.hpp"
#include "consts.hpp"

namespace d88::api
{
	void default_protect(std::string_view name,std::string_view output)
	{
		constexpr unsigned blocks = 512;
		constexpr unsigned rec = 14;
		constexpr unsigned val = 2;
		constexpr unsigned chunk = 4096;
		using T = uint64_t;
		auto &sym = consts::default_symmetry;


		d88::correct::ImmutableShortContext<T, blocks, rec> ectx(sym);
		vector<T> temp(blocks);
		vector<T> ex(rec + val);

		mio::mmap_source file(name);
		std::ofstream check((std::string)output, std::ios::binary);

		size_t rem = file.size() % chunk;

		for (size_t i = 0; i < file.size()-rem; i+= chunk)
		{
			d88::correct::immutable_extend_short<T, blocks, rec, val>(gsl::span<T>((T*)(file.data()+i), blocks), temp, ex, ectx);
			check.write((char*)ex.data(), 128);
		}

		//Padding:
		//

		if (rem)
		{
			std::vector<uint8_t> tmp(chunk);

			for (size_t i = 0; i < chunk; i++)
				tmp[i] = 0;

			std::copy(file.end() - rem, file.end() + rem, tmp.begin());

			d88::correct::immutable_extend_short<T, blocks, rec, val>(gsl::span<T>((T*)tmp.data(), blocks), temp, ex, ectx);
			check.write((char*)ex.data(), 128);
		}
	}

	void default_recover(std::string_view name, std::string_view _check)
	{
		constexpr unsigned blocks = 512;
		constexpr unsigned rec = 14;
		constexpr unsigned val = 2;
		constexpr unsigned chunk = 4096;
		using T = uint64_t;
		auto& sym = consts::default_symmetry;


		d88::correct::ImmutableShortContext<T, blocks, rec> ectx(sym);
		vector<T> temp(blocks),temp2(blocks);
		vector<T> ex_temp(rec + val);

		mio::mmap_sink file(name);
		mio::mmap_source check(_check);

		size_t rem = file.size() % chunk;

		for (size_t i = 0, k = 0; i < file.size() - rem; i += chunk, k++)
		{
			gsl::span<T> ex((T*)(check.data() + k * (rec + val) * sizeof(T)),rec+val);

			if (!d88::correct::validate_immutable_short<T, blocks, rec, val>(gsl::span<T>((T*)(file.data() + i), blocks), temp, ex, ex_temp, ectx))
			{
				std::cout << "Validation Failed for Chunk " << k << std::endl;

				if (!d88::correct::repair_quick2<T, blocks, rec, 1, val>(gsl::span<T>((T*)(file.data() + i), blocks), temp, temp2, ex, ex_temp, sym,ectx))
					std::cout << "Unrecoverable block " << i << " => " << i + chunk << std::endl;
				else
					std::cout << "Recovered Block!" << std::endl;
			}
		}

		//Padding:
		//

		if (rem)
		{
			std::vector<uint8_t> tmp(chunk);

			for (size_t i = 0; i < chunk; i++)
				tmp[i] = 0;

			std::copy(file.end() - rem, file.end() + rem, tmp.begin());

			gsl::span<T> ex((T*)(check.data() + check.size() - (rec + val) * sizeof(T)), rec + val);

			if (!d88::correct::validate_immutable_short<T, blocks, rec, val>(gsl::span<T>((T*)(tmp.data()), blocks), temp, ex, ex_temp, ectx))
			{
				std::cout << "Validation Failed for last Chunk" << std::endl;

				if (!d88::correct::repair_quick2<T, blocks, rec, 1, val>(gsl::span<T>((T*)(tmp.data()), blocks), temp, temp2, ex, ex_temp, sym,ectx))
					std::cout << "Unrecoverable block last block " << std::endl;
				else
				{
					std::cout << "Recovered Block!" << std::endl;
					std::copy(tmp.begin(), tmp.begin() + rem, file.end() - rem);
				}
			}
		}
	}
}