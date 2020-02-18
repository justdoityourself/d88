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
    bool gen = false, protect = false, recover = false;
    string in_file = "", out_file = "";

    auto cli = (
        option("-p", "--protect").set(protect).doc("Encode a recovery context"),
        option("-r", "--recover").set(recover).doc("Validate and recover file"),
        option("-g", "--input").set(gen).doc("Input File"),
        option("-i", "--input").doc("Input File") & value("Input File", in_file),
        option("-o", "--output").doc("Output File") & value("Output File", out_file)
        );

    if (!parse(argc, argv, cli)) cout << make_man_page(cli, argv[0]);
    else
    {
        if (gen)
        {
            auto sym = d88::RandomVector<uint64_t>(512);

            for (size_t i = 0; i < sym.size(); i++)
                std::cout << sym[i] << ",";

            return 0;
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


