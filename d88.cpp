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



#if ! defined(BENCHMARK_RUNNER) && ! defined(TEST_RUNNER)


#include "clipp.h"

#include <string>
#include <iostream>

using namespace std;
using namespace clipp;

int main(int argc, char* argv[])
{
    bool encode = false, decode = false;
    string in_file = "", out_file = "", password = "";

    auto cli = (
        value("input file", in_file),
        value("output file", out_file),
        option("-p","--password").set(password).doc("encryption password"),
        option("-e", "--encode").set(encode).doc("encode regenerating file"),
        option("-d", "--decode").set(decode).doc("encode regenerating file")
        );

    if (!parse(argc, argv, cli)) cout << make_man_page(cli, argv[0]);
    else
    {
        //Todo expose high level interface
    }

    return 0;
}


#endif //! defined(BENCHMARK_RUNNER) && ! defined(TEST_RUNNER)


