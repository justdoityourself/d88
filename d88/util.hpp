/* Copyright (C) 2020 D8DATAWORKS - All Rights Reserved */

#pragma once

#include <iostream>
#include <vector>
#include <random>
#include <time.h>

using namespace std;

namespace d88
{
    template < typename T > vector<T> RandomVector(size_t size)
    {
        vector<T> result;
        result.resize(size);

        default_random_engine e((unsigned int)time(0));

        for (size_t i = 0; i < size; i++)
        {
            result[i] = e();
            result[i] <<= 32; //e() doesn't fill top dword of qword
            result[i] += e();
        }

        return result;
    }

    template < typename T > vector<T> GenerateSymmetry(size_t size)
    {
        auto _sym = RandomVector<T>(size);
        while (_sym[0] % 2 == 0) _sym[0]++;

        return _sym;
    }

    template < typename T > vector<T> GenerateSequence(size_t size)
    {
        vector<T> result(size);
        
        for (size_t i = 0; i < size; i++)
            result[i] = i;

        return result;
    }

    template < typename T > void PrintMatrix(const T & v)
    {
        for (size_t i = 0; i < v.size(); i++)
        {
            for (size_t j = 0; j < v[i].size(); j++)
                cout << (size_t)v[i][j] << "\t";
            cout << endl;
        }
        cout << endl << endl;
    }

    template < typename T > void PrintRow(const T& v)
    {
        for (size_t i = 0; i < v.size(); i++)
        {
            cout << (size_t)v[i] << "\t";
        }
        cout << endl << endl;
    }

    template < typename T > vector<vector<T>> GenerateIdentity(size_t size)
    {
        vector<vector<T>> result(size);

        for (size_t i = 0; i < size; i++)
        {
            result[i].resize(size);
            for (size_t j = 0; j < size; j++)
                result[i][j] = 0;

            result[i][i] = 1;
        }

        return result;
    }
}