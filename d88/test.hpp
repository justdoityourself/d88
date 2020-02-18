/* Copyright (C) 2020 D8DATAWORKS - All Rights Reserved */

#pragma once

#include <vector>
#include <utility>
#include <filesystem>

#include "../catch.hpp"
#include "base.hpp"
#include "encrypt.hpp"
#include "hash.hpp"
#include "util.hpp"
#include "correct.hpp"
#include "factor.hpp"
#include "analysis.hpp"
#include "api.hpp"

#include "../plusaes.hpp"

using namespace d88;
using namespace d88::security;
using namespace d88::correct;
using namespace d88::analysis;
using namespace d88::api;

using namespace std;

TEST_CASE("api encrypt/decrypt", "[d88::api]")
{
    std::filesystem::remove_all("testdata/encrypted");
    std::filesystem::remove_all("testdata/decrypted");

    default_encrypt("testdata/small_file", "testdata/encrypted", "TESTPASSWORD");
    default_decrypt("testdata/encrypted", "testdata/decrypted", "TESTPASSWORD");

    {
        mio::mmap_source before("testdata/small_file");
        mio::mmap_source after("testdata/decrypted");

        CHECK(std::equal(before.begin(),before.end(),after.begin()));
    }

    std::filesystem::remove_all("testdata/encrypted");
    std::filesystem::remove_all("testdata/decrypted");
}

TEST_CASE("api protect/recover", "[d88::api]")
{
    std::filesystem::remove_all("testdata/output");
    std::filesystem::remove_all("testdata/edit");

    default_protect("testdata/small_file", "testdata/output");
    std::filesystem::copy_file("testdata/small_file", "testdata/edit");

    {
        mio::mmap_sink file("testdata/edit");

        file[file.size()-1500]++;
    }

    default_recover("testdata/edit", "testdata/output");

    {
        mio::mmap_source before("testdata/small_file");
        mio::mmap_source after("testdata/edit");

        CHECK(std::equal(before.begin(), before.end(), after.begin()));
    }

    {
        mio::mmap_sink file("testdata/edit");

        file[15]++;
    }

    default_recover("testdata/edit", "testdata/output");

    {
        mio::mmap_source before("testdata/small_file");
        mio::mmap_source after("testdata/edit");

        CHECK(std::equal(before.begin(), before.end(), after.begin()));
    }

    std::filesystem::remove_all("testdata/output");
    std::filesystem::remove_all("testdata/edit");
}

TEST_CASE("aes reverse", "[d88::]")
{
    constexpr size_t S = 16;
    typedef uint8_t U;

    PascalTriangle<U> pt(S);

    auto pw = GenerateSymmetry<unsigned char>(48);
    std::vector<U> temp(S), sym(S);

    std::vector<U> source = { 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,17 };
    std::vector<U> encrypted(S);

    std::vector<U> source2 = { 17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,33 };
    std::vector<U> encrypted2(S);

    plusaes::Error e = plusaes::encrypt_cbc((unsigned char*)source.data(), (unsigned long)source.size(), pw.data(), (int)32, reinterpret_cast<unsigned char(*)[16]>((pw.data() + 32)), &encrypted[0], (unsigned long)encrypted.size(), false);
    e = plusaes::encrypt_cbc((unsigned char*)source2.data(), (unsigned long)source2.size(), pw.data(), (int)32, reinterpret_cast<unsigned char(*)[16]>((pw.data() + 32)), &encrypted2[0], (unsigned long)encrypted2.size(), false);

    auto difference = VXOR<U>(source, encrypted);
    auto d1 = VXOR<U>(difference, encrypted);
    auto d2 = VXOR<U>(difference, encrypted2);

    REQUIRE(true == equal(source.begin(), source.end(), d1.begin()));
    REQUIRE(true != equal(source2.begin(), source2.end(), d2.begin()));

    extract_symmetry<U>(encrypted , source, temp, sym, pt);

    ElectiveSymmetry<U> es(sym);
    ToFunctionR<U>(source, temp, es);

    REQUIRE(true == equal(encrypted.begin(), encrypted.end(), temp.begin()));

    ToFunctionR<U>(source2, temp, es);

    REQUIRE(true != equal(encrypted2.begin(), encrypted2.end(), temp.begin()));
}

//Polynomial factoring doesn't work the same way in a finite field :(
//More to explore here.
/*TEST_CASE("factor", "[d88::]")
{
    typedef unsigned char T;

    vector<T> data({ 1,12,35 });

    factor<T>(data);
}*/

TEST_CASE("static analysis basics", "[d88::]")
{
    typedef unsigned char T;
    static const size_t S = 4;

    vector<T> poly({ 7,1,3,5 });
    vector<T> data({ 3,0,0,6 });
    vector<T> temp(S);
    vector<T> sym(S);

    PascalTriangle<T> pt(S);

    extract_symmetry<T>(data,poly, temp,sym,pt);

    ElectiveSymmetry<T> es(sym);
    ToFunctionR<T>(poly, temp, es);

    REQUIRE(true == equal(data.begin(), data.end(), temp.begin()));
}

TEST_CASE("static analysis random", "[d88::]")
{
    typedef unsigned long long T;
    static const size_t S = 128;

    auto poly = RandomVector<T>(S);
    if (poly[S - 1] % 2 == 0) poly[S - 1]++;

    auto data = RandomVector<T>(S);
    vector<T> temp(S);
    vector<T> sym(S);

    PascalTriangle<T> pt(S);

    extract_symmetry<T>(data, poly, temp, sym, pt);

    ElectiveSymmetry<T> es(sym);
    ToFunctionR<T>(poly, temp, es);

    REQUIRE(true == equal(data.begin(), data.end(), temp.begin()));
}


TEST_CASE("encrypt_long and decrypt_short pair works", "[d88::encrypt]") 
{
    auto plain = RandomVector<unsigned long long>(64);

    vector<unsigned long long> temp(64);
    vector<unsigned long long> enc(64);
    vector<unsigned long long> plain2(64);

    auto sym = StringAsSymmetry<unsigned long long,64>("PASSWORD");
    EncryptContextLong<unsigned long long, 64> ec(sym);
    DecryptContextShort<unsigned long long, 64> dc(sym);

    block_encrypt_long<unsigned long long,64>(plain, temp, enc, ec);
    block_decrypt_short<unsigned long long, 64>(enc, plain2, dc);

    REQUIRE_THAT(plain, Catch::Matchers::Equals(plain2));
}

TEST_CASE("encrypt_short and decrypt_long pair works", "[d88::encrypt]")
{
    auto plain = RandomVector<unsigned long long>(64);

    vector<unsigned long long> temp(64);
    vector<unsigned long long> enc(64);
    vector<unsigned long long> plain2(64);

    auto sym = StringAsSymmetry<unsigned long long, 64>("PASSWORD");
    EncryptContextShort<unsigned long long, 64> ec(sym);
    DecryptContextLong<unsigned long long, 64> dc(sym);

    block_encrypt_short<unsigned long long, 64>(plain, enc, ec);
    block_decrypt_long<unsigned long long, 64>(enc,temp, plain2, dc);

    REQUIRE_THAT(plain, Catch::Matchers::Equals(plain2));
}

TEST_CASE("block_feedback_hash basically acts like a hash", "[d88::hash]")
{
    auto data1 = RandomVector<unsigned long long>(64);
    vector<unsigned long long> data2({4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,5,4,4,4,4,4,4,4,4,4,4, 4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4, 4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4});

    vector<unsigned long long> h1(4);
    vector<unsigned long long> h2(4);
    vector<unsigned long long> h3(4);
    vector<unsigned long long> h4(4);
    vector<unsigned long long> temp(64);

    HashContextFeedback<unsigned long long, 64> hc;

    block_feedback_hash<unsigned long long, 64>(data1, temp, h1, hc);
    block_feedback_hash<unsigned long long, 64>(data2, temp, h2, hc);
    block_feedback_hash<unsigned long long, 64>(data1, temp, h3, hc);
    block_feedback_hash<unsigned long long, 64>(data2, temp, h4, hc);

    REQUIRE_THAT(h1, Catch::Matchers::Equals(h3));
    REQUIRE_THAT(h2, Catch::Matchers::Equals(h4));
    REQUIRE_THAT(h1, !Catch::Matchers::Equals(h2));
    REQUIRE_THAT(h3, !Catch::Matchers::Equals(h4));
}


TEST_CASE("block_hash_long acts like a hash", "[d88::hash]")
{
    auto data1 = RandomVector<unsigned long long>(64);
    auto sym = GenerateSymmetry<unsigned long long>(64);
    vector<unsigned long long> data2({ 4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,5,4,4,4,4,4,4,4,4,4,4, 4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4, 4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4 });

    vector<unsigned long long> h1(4);
    vector<unsigned long long> h2(4);
    vector<unsigned long long> h3(4);
    vector<unsigned long long> h4(4);
    vector<unsigned long long> temp(64);

    HashContextLong<unsigned long long, 64> hc(sym);

    block_hash_long<unsigned long long, 64>(data1, temp, h1, hc);
    block_hash_long<unsigned long long, 64>(data2, temp, h2, hc);
    block_hash_long<unsigned long long, 64>(data1, temp, h3, hc);
    block_hash_long<unsigned long long, 64>(data2, temp, h4, hc);

    REQUIRE_THAT(h1, Catch::Matchers::Equals(h3));
    REQUIRE_THAT(h2, Catch::Matchers::Equals(h4));
    REQUIRE_THAT(h1, !Catch::Matchers::Equals(h2));
    REQUIRE_THAT(h3, !Catch::Matchers::Equals(h4));
}

TEST_CASE("block_hash_short acts like a hash", "[d88::hash]")
{
    auto data1 = RandomVector<unsigned long long>(64);
    auto sym = GenerateSymmetry<unsigned long long>(64);
    vector<unsigned long long> data2({ 4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,5,4,4,4,4,4,4,4,4,4,4, 4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4, 4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4 });

    vector<unsigned long long> h1(4);
    vector<unsigned long long> h2(4);
    vector<unsigned long long> h3(4);
    vector<unsigned long long> h4(4);

    HashContextShort<unsigned long long, 64> hc(sym);

    block_hash_short<unsigned long long, 64>(data1, h1, hc);
    block_hash_short<unsigned long long, 64>(data2, h2, hc);
    block_hash_short<unsigned long long, 64>(data1, h3, hc);
    block_hash_short<unsigned long long, 64>(data2, h4, hc);

    REQUIRE_THAT(h1, Catch::Matchers::Equals(h3));
    REQUIRE_THAT(h2, Catch::Matchers::Equals(h4));
    REQUIRE_THAT(h1, !Catch::Matchers::Equals(h2));
    REQUIRE_THAT(h3, !Catch::Matchers::Equals(h4));
}

// The pascal method is broken for Mod 2^n, to be clear it works if the correct pascal rows are selected or computed. It might be the most computationally efficient actually with predefined rows.
/*TEST_CASE("extend_pascal supports basic permutations", "[d88::correct]")
{
    static const size_t S = 4;
    static const size_t E = 2;
    typedef unsigned char T;
    for (size_t i = 0; i < S; i+=E)
    {
        DYNAMIC_SECTION("@i: " << i) 
        {
            //auto data = RandomVector<T>(S);
            vector<T> data({ 4,5,1,2 });
            vector<T> ex(E);

            ExtendPascalContext<T, S, E> ectx;

            extend_pascal<T, S, E>(data, ex, ectx);

            T og1 = data[i];
            T og2 = data[i+1];
            data[i] = ex[0];
            data[i+1] = ex[1];

            auto dx = GenerateSequence<T>(S);
            dx[i] = S;
            dx[i+1] = S+1;

            RecoverPascalContext<T, S, E> rctx(dx);

            recover_pascal<T, S, E>(data, rctx);

            REQUIRE(og1 == data[i]);
            REQUIRE(og2 == data[i+1]);
        }
    }
}*/

TEST_CASE("extend_short/recover_short supports basic permutations", "[d88::correct]")
{
    static const size_t sz = 64;
    typedef unsigned long long reg_unit;

    for (size_t i = 0; i < 64; i += 2)
    {
        DYNAMIC_SECTION("@i: " << i)
        {
            auto data = RandomVector<reg_unit>(sz);
            auto sym = RandomVector<reg_unit>(sz);

            vector<reg_unit> ex(2), temp(sz);

            ExtendShortContext<reg_unit, sz, 2> ectx(sym);

            extend_short<reg_unit, sz, 2>(data, temp, ex, ectx);

            reg_unit og1 = data[i];
            reg_unit og2 = data[i + 1];
            data[i] = ex[0];
            data[i + 1] = ex[1];

            auto dx = GenerateSequence<reg_unit>(sz);
            dx[i] = sz;
            dx[i + 1] = sz + 1;

            RecoverShortContext<reg_unit, sz, 2> rctx(sym);

            recover_short<reg_unit, sz, 2>(data, dx,rctx);

            REQUIRE(og1 == data[i]);
            REQUIRE(og2 == data[i + 1]);
        }
    }
}

TEST_CASE("immutable_extend_short/recover_short supports basic permutations", "[d88::correct]")
{
    static const size_t S = 64;
    typedef unsigned long long T;

    for (size_t i = 0; i < 64; i += 2)
    {
        DYNAMIC_SECTION("@i: " << i)
        {
            auto data = RandomVector<T>(S);
            auto sym = RandomVector<T>(S);

            vector<T> ex(2), temp(S);

            ImmutableShortContext<T, S, 2> ectx(sym);

            immutable_extend_short<T, S, 2>(data, temp, ex, ectx);

            T og1 = data[i];
            T og2 = data[i + 1];
            data[i] = ex[0];
            data[i + 1] = ex[1];

            auto dx = GenerateSequence<T>(S);
            dx[i] = S;
            dx[i + 1] = S + 1;

            RecoverShortContext<T, S, 2> rctx(sym);

            recover_short<T, S, 2>(data, dx, rctx);

            REQUIRE(og1 == data[i]);
            REQUIRE(og2 == data[i + 1]);
        }
    }
}

TEST_CASE("extend_short/recover_short corruption detection", "[d88::correct]")
{
    static const size_t S = 64;
    static const size_t E = 2;
    typedef unsigned long long T;

    auto data = RandomVector<T>(S);
    auto sym = RandomVector<T>(S);

    vector<T> ex(E), temp(S), ex_temp(E);

    //These are currently 1 time use... Testing this here, but in production use ImmutableShortContext && immutable_extend_short
    ExtendShortContext<T, S, E> ectx(sym);

    extend_short<T, S, E>(data, temp, ex, ectx);

    ExtendShortContext<T, S, E> ectx2(sym);


    REQUIRE(true == validate_short<T, S, E>(data, temp, ex, ex_temp, ectx2));


    ExtendShortContext<T, S, E> ectx3(sym);

    data[27] = 0xfffff2351243256;

    REQUIRE(false == validate_short<T, S, E>(data, temp, ex, ex_temp, ectx2));
}

TEST_CASE("repair_quick can recover expected cases", "[d88::correct]")
{
    static const size_t S = 64;
    static const size_t E = 3;
    static const size_t C = 2;
    static const size_t MIN = 1;
    static const size_t MAX = 3;
    typedef unsigned long long T;

    vector<T> data = RandomVector<T>(S);
    auto sym = RandomVector<T>(S);

    vector<T> ex(E+C), ex_temp(E+C), temp1(S), temp2(S);

    ImmutableShortContext<T, S, E> ectx(sym);

    immutable_extend_short<T, S, E, C>(data, temp1, ex, ectx);

    vector<T> current(S);


    //Full alignment scan should work:
    //

    copy(data.begin(),data.end(), current.begin());

    current[11] = 0xfefefefefefefefe;
    current[12] = 0xfefefefefefefefe;
    current[13] = 0xfcfcfcfcfcfcfcfc;

    REQUIRE(true == repair_quick<T, S, E, MIN,C>(current,temp1,temp2, ex, ex_temp, sym));
    REQUIRE(true == equal(data.begin(), data.end(), current.begin()));


    //Rigid allignment scan should fail with same offsets:
    //

    copy(data.begin(), data.end(), current.begin());

    current[11] = 0xfefefefefefefefe;
    current[12] = 0xfefefefefefefefe;
    current[13] = 0xfcfcfcfcfcfcfcfc;

    REQUIRE(false == repair_quick<T, S, E, MAX,C>(current,temp1,temp2, ex, ex_temp, sym));


    //Rigid allignment should succeed with better alignment:
    //

    copy(data.begin(), data.end(), current.begin());

    current[12] = 0xfefefefefefefefe;
    current[13] = 0xfcfcfcfcfcfcfcfc;
    current[14] = 0xfcfcfcfcfcfcfcfc;

    REQUIRE(true == repair_quick<T, S, E, MAX, C>(current, temp1, temp2, ex, ex_temp, sym));
    REQUIRE(true == equal(data.begin(), data.end(), current.begin()));


    //Sporadic damage can only be recovered with full permute, not recover_quick:
    //

    copy(data.begin(), data.end(), current.begin());

    current[1] = 0xfefefefefefefefe;
    current[9] = 0xfefefefefefefefe;
    current[15] = 0xfcfcfcfcfcfcfcfc;

    REQUIRE(false == repair_quick<T, S, E, MIN, C>(current, temp1, temp2, ex, ex_temp, sym));
}
