/* Copyright (C) 2020 D8DATAWORKS - All Rights Reserved */

#pragma once

#include <algorithm>
#include <atomic>
#include <array>
#include <fstream>
#include <string_view>

#include "../mio.hpp"
#include "../gsl-lite.hpp"

#include "encrypt.hpp"
#include "correct.hpp"
#include "consts.hpp"
#include "analysis.hpp"

namespace d88::api
{
	bool compare_files_bytes(std::string_view o, std::string_view t)
	{
		auto file1 = mio::mmap_source(o);
		auto file2 = mio::mmap_source(t);

		if (file1.size() != file2.size())
			return false;

		return std::equal(file1.begin(), file1.end(), file2.begin());
	}

	void allocate_file(std::string_view o, const size_t size)
	{
		std::ofstream ofs((string)o, std::ios::binary | std::ios::out);
		ofs.seekp(size - 1);
		ofs.write("", 1);
	}

	template <typename T, typename I> T& singleton_context(I i)
	{
		static T t(i);

		return t;
	}

	//(?) Analysis needs to solve the INVERSE problem for the constant part of the poly before this becomes 100%, and ready for use.
	//(X) Solved with padded extract symmetry.
	//

	void generate_static(std::string_view _a, std::string_view _b, std::string_view _s,bool parallel = true)
	{
		constexpr unsigned blocks = 128;
		constexpr unsigned chunk = 1024;
		using T = uint64_t;

		PascalTriangle<T> pt(blocks+1);
		vector<T> temp(blocks+1);

		//Alignment not supported ATM, much more complicated with three files.
		mio::mmap_source a(_a);
		mio::mmap_source b(_b);

		if (a.size() % chunk != 0)
		{
			std::cout << "This version requires exact block sizes and does not support padding, stay tuned. ( " << chunk << " bytes )" << std::endl;
			return;
		}

		auto chunks = a.size() / chunk;

		allocate_file(_s, a.size()+chunks*sizeof(T));
		mio::mmap_sink result(_s);	

		if (parallel)
		{
			std::atomic<size_t> identity = 0;
			for_each_n(execution::par_unseq, a.data(), chunks, [&](auto v)
			{
				auto i = identity++;

				vector<T> temp(blocks+1);

				d88::analysis::padded_symmetry<T>(gsl::span<T>((T*)(b.data() + i*chunk), blocks), gsl::span<T>((T*)(a.data() + i*chunk), blocks), temp, gsl::span<T>((T*)(result.data() + i*(chunk+sizeof(T))), blocks+1), pt);
			});
		}
		else
		{
			for (size_t i = 0, k = 0; i < a.size(); k++, i += chunk)
				d88::analysis::padded_symmetry<T>(gsl::span<T>((T*)(b.data() + i), blocks), gsl::span<T>((T*)(a.data() + i), blocks), temp, gsl::span<T>((T*)(result.data() + i+ k* sizeof(T)), blocks+1), pt);
		}
	}

	void forward_static(std::string_view _a, std::string_view _s, std::string_view _r, bool parallel = true)
	{
		constexpr unsigned blocks = 128;
		constexpr unsigned chunk = 1024;
		using T = uint64_t;

		mio::mmap_source a(_a);
		mio::mmap_source s(_s);

		if (a.size() % chunk != 0)
		{
			std::cout << "This version requires exact block sizes and does not support padding, stay tuned. ( " << chunk << " bytes )" << std::endl;
			return;
		}

		auto chunks = a.size() / chunk;

		allocate_file(_r, a.size());
		mio::mmap_sink r(_r);

		if (parallel)
		{
			std::atomic<size_t> identity = 0;
			for_each_n(execution::par_unseq, a.data(), chunks, [&](auto v)
			{
				auto i = identity++;

				ElectiveSymmetry<T> es(gsl::span<T>((T*)(s.data() + i * (chunk + sizeof(T))), blocks + 1));
				ToFunctionR<T>(d88::analysis::shim_span<T>((T*)(a.data() + i * chunk), blocks), gsl::span<T>((T*)(r.data() + i * chunk), blocks), es,1);
			});
		}
		else
		{
			for (size_t i = 0, k = 0; i < a.size(); k++, i += chunk)
			{
				ElectiveSymmetry<T> es(gsl::span<T>((T*)(s.data() + i +k*sizeof(T)), blocks + 1));
				ToFunctionR<T>(d88::analysis::shim_span<T>((T*)(a.data() + i), blocks), gsl::span<T>((T*)(r.data() + i), blocks), es,1);
			}
		}
	}

	void reverse_static(std::string_view _a, std::string_view _s, std::string_view _r, bool parallel = true)
	{

	}

	void print_sym()
	{
		auto sym = d8u::random::Vector<uint64_t>(512);

		for (size_t i = 0; i < sym.size(); i++)
			std::cout << sym[i] << ",";
	}

	//This could be used for faster startup time.
	void print_solution()
	{
		auto ectx = singleton_context<d88::correct::ImmutableShortContext<uint64_t, 512, 16>>(consts::default_symmetry);
		auto& m = ectx.Map();

		std::ofstream s("solution.txt");

		for (auto& e : m)
		{
			s << "{" << e.first << "," << e.second << "},";
			std::cout << "{" << e.first << "," << e.second << "},";
		}
	}

	//This could be used for faster startup time.
	void print_es()
	{
		auto ectx = singleton_context<d88::correct::ImmutableShortContext<uint64_t, 512, 16>>(consts::default_symmetry);
		auto& y = ectx.Symmetry();

		std::ofstream s("es.txt");

		for (auto& e : y.data())
		{
			//todo
		}
	}

	void default_encrypt(std::string_view i, std::string_view o, std::string_view k,bool parallel = true)
	{
		constexpr unsigned blocks = 128;
		constexpr unsigned chunk = 1024;
		using T = uint64_t;

		vector<T> temp(blocks);

		auto sym = StringAsSymmetry<T, blocks>(k);
		d88::security::EncryptContextLong<T, blocks> ec(sym);

		mio::mmap_source file(i);
		size_t rem = (file.size() % chunk);

		allocate_file(o, (rem) ? (file.size()+rem + sizeof(uint64_t)) : file.size());
		mio::mmap_sink result(o);

		if (parallel)
		{
			std::atomic<size_t> identity = 0;
			for_each_n(execution::par_unseq, file.data(), file.size() / chunk, [&](auto v)
			{
				auto i = identity++;

				vector<T> temp(blocks);

				d88::security::block_encrypt_long<T, blocks>(gsl::span<T>((T*)(file.data() + i * chunk), blocks), temp, gsl::span<T>((T*)(result.data() + i * chunk), blocks), ec);
			});
		}
		else
		{
			for (size_t i = 0; i < file.size() - rem; i += chunk)
				d88::security::block_encrypt_long<T, blocks>(gsl::span<T>((T*)(file.data() + i), blocks), temp, gsl::span<T>((T*)(result.data() + i), blocks), ec);
		}


		//Padding:
		//

		if (rem)
		{
			std::vector<uint8_t> tmp = d8u::random::Vector<uint8_t>(chunk), tout(chunk);

			std::copy(file.end() - rem, file.end(), tmp.begin());

			d88::security::block_encrypt_long<T, blocks>(gsl::span<T>((T*)(tmp.data()), blocks), temp, gsl::span<T>((T*)(tout.data()), blocks), ec);

			std::copy(tout.begin(), tout.end(), result.end() - (chunk + sizeof(uint64_t)));

			*(uint64_t*)(result.data() + result.size() - sizeof(uint64_t)) = file.size();
		}
	}

	void default_decrypt(std::string_view i, std::string_view o, std::string_view k, bool parallel = true)
	{
		constexpr unsigned blocks = 128;
		constexpr unsigned chunk = 1024;
		using T = uint64_t;

		auto sym = StringAsSymmetry<T, blocks>(k);
		d88::security::DecryptContextShort<T, blocks> dc(sym);

		mio::mmap_source file(i);

		auto final_size = ((file.size() % chunk) ? (*(uint64_t*)(file.data() + file.size() - sizeof(uint64_t))) : file.size());

		allocate_file(o, final_size);
		mio::mmap_sink result(o);

		size_t rem = (result.size() % chunk);

		if (parallel)
		{
			std::atomic<size_t> identity = 0;
			for_each_n(execution::par_unseq, file.data(), result.size() / chunk, [&](auto v)
			{
				auto i = identity++;
				d88::security::block_decrypt_short<T, blocks>(gsl::span<T>((T*)(file.data() + i * chunk), blocks), gsl::span<T>((T*)(result.data() + i * chunk), blocks), dc);
			});
		}
		else
		{
			for (size_t i = 0; i < result.size() - rem; i += chunk)
				d88::security::block_decrypt_short<T, blocks>(gsl::span<T>((T*)(file.data() + i), blocks), gsl::span<T>((T*)(result.data() + i), blocks), dc);
		}

		//Padding:
		//

		if (rem)
		{
			std::vector<uint8_t> tmp(chunk);

			d88::security::block_decrypt_short<T, blocks>(gsl::span<T>((T*)(file.end()-(chunk+sizeof(uint64_t))), blocks), gsl::span<T>((T*)(tmp.data()), blocks), dc);

			std::copy(tmp.begin(), tmp.begin() + rem, result.end() - rem);
		}
	}

	void default_protect(std::string_view name,std::string_view output, bool parallel = true)
	{
		constexpr unsigned blocks = 512;
		constexpr unsigned rec = 14;
		constexpr unsigned val = 2;
		constexpr unsigned chunk = 4096;
		using T = uint64_t;

		auto ectx = singleton_context<d88::correct::ImmutableShortContext<T, blocks, rec>>(consts::default_symmetry);

		vector<T> temp(blocks);

		mio::mmap_source file(name);
		size_t rem = file.size() % chunk;

		auto final_size = file.size() / chunk;
		if (rem) final_size++;
		final_size *= (rec + val) * sizeof(T);

		allocate_file(output, final_size);
		mio::mmap_sink result(output);

		if (parallel)
		{
			std::atomic<size_t> identity = 0;
			for_each_n(execution::par_unseq, file.data(), file.size() / chunk, [&](auto v)
			{
				auto i = identity++;
				vector<T> temp(blocks);

				d88::correct::immutable_extend_short<T, blocks, rec, val>(gsl::span<T>((T*)(file.data() + i * chunk), blocks), temp, gsl::span<T>((T*)(result.data() + i * (rec + val) * sizeof(T)), rec + val), ectx);
			});
		}
		else
		{
			for (size_t i = 0, k = 0; i < file.size() - rem; k++, i += chunk)
				d88::correct::immutable_extend_short<T, blocks, rec, val>(gsl::span<T>((T*)(file.data() + i), blocks), temp, gsl::span<T>((T*)(result.data() + k * (rec + val) * sizeof(T)), rec+val), ectx);
		}

		//Padding:
		//

		if (rem)
		{
			std::vector<uint8_t> tmp(chunk);

			for (size_t i = 0; i < chunk; i++)
				tmp[i] = 0;

			std::copy(file.end() - rem, file.end(), tmp.begin());

			d88::correct::immutable_extend_short<T, blocks, rec, val>(gsl::span<T>((T*)tmp.data(), blocks), temp, gsl::span<T>((T*)(result.end() - (rec + val) * sizeof(T)), blocks), ectx);
		}
	}

	void default_recover(std::string_view name, std::string_view _check, bool parallel = true)
	{
		constexpr unsigned blocks = 512;
		constexpr unsigned rec = 14;
		constexpr unsigned val = 2;
		constexpr unsigned chunk = 4096;
		using T = uint64_t;
		auto& sym = consts::default_symmetry;

		auto ectx = singleton_context<d88::correct::ImmutableShortContext<T, blocks, rec>>(consts::default_symmetry);
		vector<T> temp(blocks),temp2(blocks);
		vector<T> ex_temp(rec + val);

		mio::mmap_sink file(name);
		mio::mmap_source check(_check);

		size_t rem = file.size() % chunk;

		auto do_validate_and_recover = [&](auto k,auto blk,auto ex,auto &_temp,auto &_temp2,auto &_ex_temp)
		{
			if (!d88::correct::validate_immutable_short<T, blocks, rec, val>(blk, _temp, ex, _ex_temp, ectx))
			{
				std::cout << "Validation Failed for Chunk " << k << std::endl;

				if (!d88::correct::repair_quick2<T, blocks, rec, 1, val>(blk, _temp, _temp2, ex, _ex_temp, sym, ectx))
				{
					std::cout << "Unrecoverable block " << k * chunk << " => " << k * chunk + chunk << std::endl;
					return -1;
				}	
				else
				{
					std::cout << "Recovered Block!" << std::endl;
					return 1;
				}
			}

			return 0;
		};

		if (parallel)
		{
			std::atomic<size_t> identity = 0;
			for_each_n(execution::par_unseq, file.data(), file.size() / chunk, [&](auto v)
			{
				auto i = identity++;
				vector<T> temp(blocks), temp2(blocks);
				vector<T> ex_temp(rec + val);

				do_validate_and_recover(i, gsl::span<T>((T*)(file.data() + i * chunk), blocks), gsl::span<T>((T*)(check.data() + i * (rec + val) * sizeof(T)), rec + val), temp, temp2, ex_temp);
			});
		}
		else
		{
			for (size_t i = 0, k = 0; i < file.size() - rem; i += chunk, k++)
				do_validate_and_recover(k, gsl::span<T>((T*)(file.data() + i), blocks), gsl::span<T>((T*)(check.data() + k * (rec + val) * sizeof(T)), rec + val), temp, temp2, ex_temp);
		}

		//Padding:
		//

		if (rem)
		{
			std::vector<uint8_t> tmp(chunk);

			for (size_t i = 0; i < chunk; i++)
				tmp[i] = 0;

			std::copy(file.end() - rem, file.end(), tmp.begin());

			if(1 == do_validate_and_recover(-1, gsl::span<T>((T*)(tmp.data()), blocks), gsl::span<T>((T*)(check.data() + check.size() - (rec + val) * sizeof(T)), rec + val), temp, temp2, ex_temp))
				std::copy(tmp.begin(), tmp.begin() + rem, file.end() - rem);
		}
	}

	template <typename B> std::vector<uint8_t> protect_block(const B& block, bool parallel = true)
	{
		constexpr unsigned blocks = 512;
		constexpr unsigned rec = 14;
		constexpr unsigned val = 2;
		constexpr unsigned chunk = 4096;
		using T = uint64_t;

		auto ectx = singleton_context<d88::correct::ImmutableShortContext<T, blocks, rec>>(consts::default_symmetry);

		vector<T> temp(blocks);

		size_t rem = block.size() % chunk;

		auto final_size = block.size() / chunk;
		if (rem) final_size++;
		final_size *= (rec + val) * sizeof(T);

		std::vector result(final_size);

		if (parallel)
		{
			std::atomic<size_t> identity = 0;
			for_each_n(execution::par_unseq, block.data(), block.size() / chunk, [&](auto v)
			{
				auto i = identity++;
				vector<T> temp(blocks);

				d88::correct::immutable_extend_short<T, blocks, rec, val>(gsl::span<T>((T*)(block.data() + i * chunk), blocks), temp, gsl::span<T>((T*)(result.data() + i * (rec + val) * sizeof(T)), rec + val), ectx);
			});
		}
		else
		{
			for (size_t i = 0, k = 0; i < block.size() - rem; k++, i += chunk)
				d88::correct::immutable_extend_short<T, blocks, rec, val>(gsl::span<T>((T*)(block.data() + i), blocks), temp, gsl::span<T>((T*)(result.data() + k * (rec + val) * sizeof(T)), rec + val), ectx);
		}

		//Padding:
		//

		if (rem)
		{
			std::vector<uint8_t> tmp(chunk);

			for (size_t i = 0; i < chunk; i++)
				tmp[i] = 0;

			std::copy(block.end() - rem, block.end(), tmp.begin());

			d88::correct::immutable_extend_short<T, blocks, rec, val>(gsl::span<T>((T*)tmp.data(), blocks), temp, gsl::span<T>((T*)(result.end() - (rec + val) * sizeof(T)), blocks), ectx);
		}

		return result;
	}

	template <typename B, typename C> bool recover_block(const B& block, const C& check, bool parallel = true)
	{
		bool valid = true;
		constexpr unsigned blocks = 512;
		constexpr unsigned rec = 14;
		constexpr unsigned val = 2;
		constexpr unsigned chunk = 4096;
		using T = uint64_t;
		auto& sym = consts::default_symmetry;

		auto ectx = singleton_context<d88::correct::ImmutableShortContext<T, blocks, rec>>(consts::default_symmetry);
		vector<T> temp(blocks), temp2(blocks);
		vector<T> ex_temp(rec + val);

		size_t rem = block.size() % chunk;

		auto do_validate_and_recover = [&](auto k, auto blk, auto ex, auto& _temp, auto& _temp2, auto& _ex_temp)
		{
			if (!d88::correct::validate_immutable_short<T, blocks, rec, val>(blk, _temp, ex, _ex_temp, ectx))
			{
				if (!d88::correct::repair_quick2<T, blocks, rec, 1, val>(blk, _temp, _temp2, ex, _ex_temp, sym, ectx))
				{
					valid = false;
					return -1;
				}
				else
					return 1;
			}

			return 0;
		};

		if (parallel)
		{
			std::atomic<size_t> identity = 0;
			for_each_n(execution::par_unseq, block.data(), block.size() / chunk, [&](auto v)
			{
				auto i = identity++;
				vector<T> temp(blocks), temp2(blocks);
				vector<T> ex_temp(rec + val);

				do_validate_and_recover(i, gsl::span<T>((T*)(block.data() + i * chunk), blocks), gsl::span<T>((T*)(check.data() + i * (rec + val) * sizeof(T)), rec + val), temp, temp2, ex_temp);
			});
		}
		else
		{
			for (size_t i = 0, k = 0; i < block.size() - rem; i += chunk, k++)
				do_validate_and_recover(k, gsl::span<T>((T*)(block.data() + i), blocks), gsl::span<T>((T*)(check.data() + k * (rec + val) * sizeof(T)), rec + val), temp, temp2, ex_temp);
		}

		//Padding:
		//

		if (rem)
		{
			std::vector<uint8_t> tmp(chunk);

			for (size_t i = 0; i < chunk; i++)
				tmp[i] = 0;

			std::copy(block.end() - rem, block.end(), tmp.begin());

			if (1 == do_validate_and_recover(-1, gsl::span<T>((T*)(tmp.data()), blocks), gsl::span<T>((T*)(check.data() + check.size() - (rec + val) * sizeof(T)), rec + val), temp, temp2, ex_temp))
				std::copy(tmp.begin(), tmp.begin() + rem, block.end() - rem);
		}

		return valid;
	}
}