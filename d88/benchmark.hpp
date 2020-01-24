#pragma once

#include "../plusaes.hpp"
#include "../picosha2.hpp"
#include "../picobench.hpp"
#include "../ProgressBar.hpp"

#include "encrypt.hpp"
#include "hash.hpp"
#include "util.hpp"
#include "correct.hpp"

namespace d88
{
    using namespace security;
    using namespace correct;

    namespace benchmark
    {
        static const int test_count = 17;
        ProgressBar progressBar(test_count * 25744, 70);

        template <size_t S> void sha256(picobench::state& s)
        {
            auto source = RandomVector<unsigned char>(S);

            {
                picobench::scope scope(s);
                for (auto _ : s)
                {
                    std::vector<unsigned char> hash(picosha2::k_digest_size);
                    picosha2::hash256(source.begin(), source.end(), hash.begin(), hash.end());
                }
            }
            progressBar += s.iterations();  progressBar.display();
        }


        template <size_t S> void aes256_encrypt(picobench::state& s)
        {
            auto source = RandomVector<unsigned char>(S);
            auto pw = GenerateSymmetry<unsigned char>(48);

            {
                picobench::scope scope(s);
                for (auto _ : s)
                {
                    plusaes::Error e;

                    std::vector<unsigned char> encrypted(S);
                    e = plusaes::encrypt_cbc((unsigned char*)source.data(), (unsigned long)source.size(), pw.data(), (int)32, reinterpret_cast<unsigned char(*)[16]>((pw.data() + 32)), &encrypted[0], (unsigned long)encrypted.size(), false);

                    /*std::vector<unsigned char> decrypted(encrypted.size());
                    e = plusaes::decrypt_cbc(&encrypted[0], (unsigned long)encrypted.size(), &key[0], (int)key.size(), iv, &decrypted[0], (unsigned long)decrypted.size(), 0);
                    */

                }
            }
            progressBar += s.iterations();  progressBar.display();
        }



        /*template <typename T, size_t S> void encTSP(picobench::state& s)
        {
            auto source = RandomVector<T>(S);
            vector<T> _pascal(S);
            vector<T> poly(S);
            auto pw = GenerateSymmetry<T>(S);
            ElectiveTransform<T> et(pw);
            static PascalTriangle<T> pt(S);

            {
                picobench::scope scope(s);
                for (auto _ : s)
                {
                    ToPascalParallel<T>(source, _pascal, pt);
                    ToPolynomialParallel<T>(_pascal, poly, et);
                }
            }
        }*/

        template <typename T, size_t S, size_t H> void hash_long(picobench::state& s)
        {
            auto source = RandomVector<T>(S);
            vector<T> temp(S), dest(H);

            static auto sym = GenerateSymmetry<T>(S);
            static HashContextLong<T, S> ctx(sym);

            {
                picobench::scope scope(s);

                for (auto _ : s)
                    block_hash_long<T, S>(source, temp, dest, ctx);
            }
            progressBar += s.iterations();  progressBar.display();
        }

        template <typename T, size_t S, size_t H> void hash_short(picobench::state& s)
        {
            auto source = RandomVector<T>(S);
            vector<T> dest(H);

            static auto sym = GenerateSymmetry<T>(S);
            static HashContextShort<T, S> ctx(sym);

            {
                picobench::scope scope(s);

                for (auto _ : s)
                    block_hash_short<T, S>(source, dest, ctx);
            }
            progressBar += s.iterations();  progressBar.display();
        }

        template <typename T, size_t S> void encrypt_long(picobench::state& s)
        {
            auto source = RandomVector<T>(S);
            vector<T> temp(S), dest(S);

            auto pw = GenerateSymmetry<T>(S);
            static EncryptContextLong<T, S> ctx;
            ctx.Init(pw);

            {
                picobench::scope scope(s);

                for (auto _ : s)
                    block_encrypt_long<T, S>(source, temp, dest, ctx);
            }
            progressBar += s.iterations();  progressBar.display();
        }

        template <typename T, size_t S> void encrypt_short(picobench::state& s)
        {
            auto source = RandomVector<T>(S);
            vector<T> dest(S);

            auto pw = GenerateSymmetry<T>(S);
            EncryptContextShort<T, S> ctx(pw);

            {
                picobench::scope scope(s);

                for (auto _ : s)
                    block_encrypt_short<T, S>(source, dest, ctx);
            }
            progressBar += s.iterations();  progressBar.display();
        }

        template <typename T, size_t S,size_t EX> void extend_short(picobench::state& s)
        {
            auto data = RandomVector<T>(S);
            auto sym = RandomVector<T>(S);

            vector<T> ex(EX), temp(S);

            ImmutableShortContext<T, S, EX> ectx(sym);

            {
                picobench::scope scope(s);

                for (auto _ : s)
                    immutable_extend_short<T, S, EX>(data, temp, ex, ectx);
            }
            progressBar += s.iterations();  progressBar.display();
        }

        auto extsh64x64 = extend_short<unsigned long long, 8,4>;
        auto extsh1024x64 = extend_short<unsigned long long, 128,4>;


        auto aes256x64 = aes256_encrypt<64>;
        auto aes256x1024 = aes256_encrypt<1024>;
        auto aes256x16k = aes256_encrypt<1024 * 16>;

        auto sha256x64 = sha256<64>;
        auto sha256x1024 = sha256<1024>;
        auto sha256x16k = sha256<1024 * 16>;

        auto hash256x64x64 = hash_long<unsigned long long,8,4>;

        auto hash256x64x64s = hash_short<unsigned long long, 8, 4>;

        auto enc64x8 = encrypt_long<unsigned char, 64>;
        auto enc64x16 = encrypt_long<unsigned char, 32>;
        auto enc64x32 = encrypt_long<unsigned int, 16>;
        auto enc64x64 = encrypt_long<unsigned long long, 8>;
        auto enc1024x32 = encrypt_long<unsigned int, 256>;
        auto enc1024x64 = encrypt_long<unsigned long long, 128>;
        auto enc4096x64 = encrypt_long<unsigned long long, 512>;
        auto enc16kx64 = encrypt_long<unsigned long long, 2048>;

        auto enc1024x32s = encrypt_short<unsigned int, 256>;
        auto enc1024x64s = encrypt_short<unsigned long long, 128>;

        auto enc16kx64s = encrypt_short<unsigned long long, 2048>;

        //auto enc1024x64p = encTSP<unsigned long long, 128>;

        PICOBENCH_SUITE("64 bytes ~block sizes ~algorithms");

        PICOBENCH(aes256x64);

        PICOBENCH(sha256x64);
        PICOBENCH(hash256x64x64);
        PICOBENCH(hash256x64x64s);

        PICOBENCH(enc64x8);
        PICOBENCH(enc64x16);
        PICOBENCH(enc64x32);
        PICOBENCH(enc64x64);
        PICOBENCH(extsh64x64);

        PICOBENCH_SUITE("1024 bytes ~block sizes ~algorithms");

        PICOBENCH(aes256x1024);
        PICOBENCH(sha256x1024);

        PICOBENCH(enc1024x32);
        PICOBENCH(enc1024x64);
        PICOBENCH(enc4096x64);
        PICOBENCH(enc1024x32s);
        PICOBENCH(enc1024x64s);
        PICOBENCH(extsh1024x64);

        //PICOBENCH(enc1024x64p);

        //PICOBENCH(aes256x16k);
        //PICOBENCH(enc16kx64s);
        /*PICOBENCH(sha16k);
        PICOBENCH(enc16kx64);*/


    }

}


