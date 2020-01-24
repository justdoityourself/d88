/* Copyright (C) 2020 D8DATAWORKS - All Rights Reserved */

#pragma once

#include <vector>
#include <tuple>
#include <string>

#include <algorithm>
#include <execution>

#include "../gsl-lite.hpp"
#include "../num.hpp"
#include "../picosha2.hpp"

namespace d88
{
    using namespace std;
    using namespace gsl;

    class nibble
    {
    public:
        nibble() {}
        template <typename T> nibble(T t) { data = t; }

        nibble operator+ (const nibble& r) const { return data + r.data; }
     
        nibble operator* (const nibble& r) const { return data * r.data; }

        nibble operator+= (const nibble& r) { return data += r.data; }
        nibble operator*= (const nibble& r) { return data *= r.data; }

    private:
        unsigned char data:4;
    };

    template<typename T> tuple<T, T, T> extended_gcd(T a, T b)
    {
        if (a == 0)
            return make_tuple(b, 0, 1);

        T gcd, x, y;

        tie(gcd, x, y) = extended_gcd<T>(b % a, a);

        return make_tuple(gcd, (y - (b / a) * x), x);
    }

    template <typename T> class PascalTriangle
    {
    public:
        PascalTriangle(size_t height)
        {
            data.resize(height);

            data[0].push_back(1);

            for (size_t i = 1; i < height; i++)
            {
                data[i].reserve(i + 1);
                data[i].push_back(1);

                for (size_t j = 1; j < i; j++)
                    data[i].push_back(data[i - 1][j - 1] + data[i - 1][j]);

                data[i].push_back(1);
            }
        }

        size_t size() const
        {
            return data.size();
        }

        const vector<T>& operator[](size_t dx) const
        {
            return data[dx];
        }

        vector<vector<T>>& Mutate() { return data; }

    private:
        vector<vector<T>> data;
    };


    template <typename T> void ToPascal(const span<T>& data, const span<T>& output, const PascalTriangle<T>& triangle)
    {
        for (size_t i = 0; i < data.size(); i++)
        {
            T s = 0;
            for (size_t j = 0; j < triangle[i].size(); j++)
                s += triangle[i][j] * data[j];

            output[i] = s;
        }
    }

    template <typename T> void ToPascalR(const span<T>& data, const span<T>& output, const PascalTriangle<T>& triangle)
    {
        for (size_t i = 0,k = data.size()-1; i < data.size(); i++)
        {
            T s = 0;
            for (size_t j = 0; j < triangle[i].size(); j++)
                s += triangle[i][j] * data[k-j];

            output[i] = s;
        }
    }

    template <typename T> void ToPascalPolarR(const span<T>& data, const span<T>& output, const PascalTriangle<T>& triangle)
    {
        for (size_t i = 0, k = data.size() - 1; i < data.size(); i++)
        {
            T s = 0;
            for (size_t j = 0; j < triangle[i].size(); j++)
            {
                T d = ((((i % 2) && !(j % 2)) || (!(i % 2) && (j % 2))) ? -1 : 1);
                T v = triangle[i][j] * d * data[k - j];
                s += v;
            }

            output[i] = s;
        }
    }

    template <typename T> void ToPascalParallel(const span<T>& data, const span<T>& output, const PascalTriangle<T>& triangle)
    {
        for_each(execution::par_unseq, data.begin(), data.end(), [&](auto&& item) mutable
            {
                size_t i = &item - data.data();
                T s = 0;
                for (size_t j = 0; j < triangle[i].size(); j++)
                    s += triangle[i][j] * data[j];

                output[i] = s;
            });
    }

    template <typename T> vector<T> AsPascal(const span<T>& data, const PascalTriangle<T>& triangle)
    {
        vector<T> result(data.size());

        ToPascal(data, result, triangle);

        return result;
    }

    template <typename T> class ElectiveSymmetry
    {
    public:
        ElectiveSymmetry(const span<T>& sym, size_t height=0)
        {
            if (!height)height = sym.size();

            data.resize(height);

            T first = sym[0];
            if (first % 2 == 0)
                first++;

            for (size_t i = 0; i < height; i++)
            {
                data[i].reserve(sym.size());
                data[i].push_back(i % 2 ? ((T)0) - first : first);
            }

            for (size_t i = 1; i < sym.size(); i++)
            {
                for (size_t j = 0; j < height; j++)
                {
                    if (j == 0)
                        data[j].push_back(sym[i]);
                    else
                        data[j].push_back((data[j - 1][i - 1] - data[j - 1][i]));
                }
            }
        }

        size_t size() const { return data.size(); }
        const vector<T>& operator[](size_t dx) const { return data[dx]; }

        vector<vector<T>>& Mutate() { return data; }
        vector<vector<T>> Duplicate() { return data; }

    private:
        vector<vector<T>> data;
    };

    template <typename T> T GetInverse(T i)
    {
        T t = -1;
        switch (t)
        {
        case 255:
            return (T)get<1>(extended_gcd<unsigned short>(i, 256));
        case 65535:
            return (T)get<1>(extended_gcd<unsigned int>(i, 65536));
        case 4294967295:
            return (T)get<1>(extended_gcd<unsigned long long>(i, 4294967296));
        default:
            t = (T)get<1>(extended_gcd<Num>(Num(to_string(i).c_str(), 10), Num("10000000000000000", 16))).words[0];
            return (t * i == 1) ? t : ((T)0) - t;
        }
    }

    template <typename T> class ElectiveTransform
    {
    public:
        ElectiveTransform() {}
        ElectiveTransform(const span<T>& sym)
        {
            Init(sym);
        }

        void Init(const span<T>& sym)
        {
            data.clear();

            T first = sym[0];
            if (first % 2 == 0)
                first++;

            data.resize(sym.size());
            for (size_t i = 0; i < sym.size(); i++)
            {
                for (size_t j = 0; j < i + 1; j++)
                    data[i].push_back((j==0) ? first : sym[j]);
            }

            if (data[0][0] != 1)
            {
                _inverse = GetInverse(data[0][0]);

                T tst = _inverse; tst = tst * data[0][0];
                if (tst != 1)
                    throw "TODO HANDLE GCD FAILURE!";
            }
        }

        size_t size() const { return data.size(); }

        const vector<T>& operator[](size_t dx) const
        {
            return data[dx];
        }

        T inverse() const { return _inverse; }

    private:
        T _inverse = 0;

        vector<vector<T>> data;
    };

    template < typename T, size_t S > vector<T> ImportSymmetry1(const span<T>& source)
    {
        vector<T> result(S);

        for (size_t i = 0; i < S; i++)
            result[i] = source[i % source.size()];

        return result;
    }

    template<typename T, size_t S> vector<T> StringAsSymmetry(string_view v)
    {
        vector<unsigned char> hash(picosha2::k_digest_size);
        picosha2::hash256(v.begin(), v.end(), hash.begin(), hash.end());
        return ImportSymmetry1<T, S>(span<T>((T*)hash.data(),hash.size()/sizeof(T)));
    }

    template <typename T> void ToPolynomial(const span<T>& _pascal, const span<T>& output, const ElectiveTransform<T>& et)
    {
        if (et.inverse())
        {
            for (size_t i = 0, k = _pascal.size() - 1; i < output.size(); i++, k--)
            {
                T s = _pascal[k] * et.inverse();
                for (size_t j = et[i].size() - 1, p = 0; j > 0; j--, p++)
                    s += ((T)0) - (et[i][j] * output[p] * et.inverse());

                output[i] = s;
            }
        }
        else
        {
            for (size_t i = 0; i < output.size(); i++)
            {
                T s = _pascal[i];
                for (size_t j = et[i].size() - 1; j > 0; j--)
                    s += ((T)0) - (et[i][j] * output[j]);

                output[i] = s;
            }
        }
    }

    template <typename T> void ToPolynomialParallel(const span<T>& _pascal, const span<T>& output, const ElectiveTransform<T>& et)
    {
        if (et.inverse())
        {
            //todo clamp iterations based on output.size()
            for_each(execution::seq, _pascal.rbegin(), _pascal.rend(), [&](auto&& item) mutable
                {
                    size_t i = _pascal.size() - 1 - (&item - _pascal.data());

                    T s = item * et.inverse();
                    for (size_t j = et[i].size() - 1, p = 0; j > 0; j--, p++)
                        s += ((T)0) - (et[i][j] * output[p] * et.inverse());

                    output[i] = s;
                });
        }
        else
        {
            throw "TODO";
        }
    }

    template <typename T> vector<T> AsPolynomial(const span<T>& _pascal, const ElectiveTransform<T>& et)
    {
        vector<T> result(_pascal.size());
        ToPolynomial<T>(_pascal, result, et);

        return result;
    }

    template<typename T> void ToFunction(const span<T>& polynomial, const span<T>& output,const ElectiveSymmetry<T>& es)
    {
        for (size_t i = es.size()-output.size(),k=0; i < es.size(); i++,k++)
        {
            T s = 0;

            for (size_t j = 0, p = polynomial.size() - 1; j < polynomial.size(); j++, p--)
                s += es[i][p] * polynomial[j];

            output[k] = s;
        }
    }

    template<typename T> void ToFunctionR(const span<T>& polynomial, const span<T>& output, const ElectiveSymmetry<T>& es)
    {
        for (size_t i = es.size() - output.size(), k = 0; i < es.size(); i++, k++)
        {
            T s = 0;

            for (size_t j = 0; j < polynomial.size(); j++)
                s += es[i][j] * polynomial[j];

            output[k] = s;
        }
    }

    template<typename T> vector<T> AsFunction(const span<T>& polynomial, const ElectiveSymmetry<T>& es)
    {
        vector<T> result;
        result.reserve(es.size());

        ToFunction<T>(polynomial, result, es);

        return result;
    }
}