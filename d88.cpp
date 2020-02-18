/* Copyright (C) 2020 D8DATAWORKS - All Rights Reserved */

#ifdef TEST_RUNNER


#define CATCH_CONFIG_RUNNER
#include "catch.hpp"
#include "d88/test.hpp"

int main(int argc, char* argv[])
{
    return Catch::Session().run(argc,argv);
}


#endif //TEST_RUNNER

#ifdef BENCHMARK_RUNNER


#define PICOBENCH_IMPLEMENT
#include "picobench.hpp"
#include "d88/benchmark.hpp"

int main(int argc, char* argv[])
{
    picobench::runner runner;
    return runner.run();
}


#endif //BENCHMARK_RUNNER



#if ! defined(BENCHMARK_RUNNER) && ! defined(TEST_RUNNER) && ! defined(OTHER)


#include "clipp.h"

#include <string>
#include <array>
#include <iostream>

#include "d88/api.hpp"

using namespace std;
using namespace clipp;

int main(int argc, char* argv[])
{
    bool gen = false, protect = false, recover = false, _static = false, encrypt = false, decrypt = false, solve = false;
    string in_file = "", out_file = "", middle = "static", key ="password";

    auto cli = (
        option("-e", "--encrypt").set(encrypt).doc("Encrypt File"),
        option("-d", "--decrypt").set(decrypt).doc("Decrypt File"),
        option("-s", "--static").set(_static).doc("Compute static difference"),
        option("-p", "--protect").set(protect).doc("Encode a recovery context"),
        option("-r", "--recover").set(recover).doc("Validate and recover file"),
        option("-g", "--gensym").set(gen).doc("Print symmetry"),
        option("-v", "--gensol").set(solve).doc("Print solution"),
        option("-k", "--key") & value("Password", key),
        option("-i", "--input") & value("Input File", in_file),
        option("-m", "--middle") & value("Intermediate File", middle),
        option("-o", "--output") & value("Output File", out_file)
        );

    if (!parse(argc, argv, cli)) cout << make_man_page(cli, argv[0]);
    else
    {
        if (gen)
        {
            d88::api::print_sym();
        }
        else if (solve)
        {
            d88::api::print_solution();
        }
        else if (encrypt)
        {
            d88::api::default_encrypt(in_file, out_file, key);
        }
        else if (decrypt)
        {
            d88::api::default_decrypt(in_file, out_file, key);
        }
        else if (_static)
        {

        }
        else if (protect)
        {
            d88::api::default_protect(in_file, out_file);
        }
        else if (recover)
        {
            d88::api::default_recover(in_file, out_file);
        }
    }

    return 0;
}


#endif //! defined(BENCHMARK_RUNNER) && ! defined(TEST_RUNNER)


