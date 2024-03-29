/* Copyright (C) 2020 D8DATAWORKS - All Rights Reserved */

#pragma once

#include <vector>
#include <tuple>
#include <string>

#include <type_traits>
#include <algorithm>
#include <execution>

#include <atomic>

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


    template <typename T, typename I, typename O> void ToPascal(const I& data, O& output, const PascalTriangle<T>& triangle)
    {
        for (size_t i = 0; i < data.size(); i++)
        {
            output[i] = 0;
            for (size_t j = 0; j < triangle[i].size(); j++)
                output[i] += triangle[i][j] * data[j];
        }
    }

    template <typename T> void DerivativeR(const span<T>& data, const span<T>& output)
    {
        for (size_t i = data.size()-1, k = 0; i != 0; i--,k++)
        {
            output[k] = data[i - 1] - data[i];
        }
    }


    template <typename T> void ExecutePascal(const span<T>& data, const span<T>& output, const PascalTriangle<T>& triangle)
    {
        for (size_t i = 0; i < output.size() && i < triangle.size(); i++)
        {
            T s = 0;
            for (size_t j = 0; j < triangle[i].size(); j++)
                s += triangle[i][j] * data[j];

            output[i] = s;
        }
    }

    template <typename T> void ExecutePolarPascal(const span<T>& data, const span<T>& output, const PascalTriangle<T>& triangle)
    {
        for (size_t i = 0; i < output.size(); i++)
        {
            T s = 0;
            for (size_t j = 0; j < triangle[i].size() && j < data.size(); j++)
            {
                T d = ((((i % 2) && !(j % 2)) || (!(i % 2) && (j % 2))) ? -1 : 1);
                s += triangle[i][j] * d * data[j];
            }       

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

            _data.resize(height);

            {
                T first = sym[0];
                if (first % 2 == 0)
                    ++first;

                T inv(0);
                inv -= first;

                for (size_t i = 0; i < height; i++)
                {
                    _data[i].reserve(sym.size());
                    _data[i].push_back(i % 2 ? inv : first);
                }
            }

            for (size_t i = 1; i < sym.size(); i++)
            {
                for (size_t j = 0; j < height; j++)
                {
                    if (j == 0)
                        _data[j].push_back(sym[i]);
                    else
                        _data[j].push_back((_data[j - 1][i - 1] - _data[j - 1][i]));
                }
            }
        }

        size_t size() const { return _data.size(); }
        const vector<T>& operator[](size_t dx) const { return _data[dx]; }

        vector<vector<T>>& Mutate() { return _data; }
        vector<vector<T>> Duplicate() { return _data; }

        const vector<vector<T>>& data() const { return _data; }

    private:
        vector<vector<T>> _data;
    };

    template <typename T> T GetInverse(T i)
    {
        if constexpr(std::is_class<T>())
            return i.MultiplicativeInverse();
        else
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
                ++first;

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
        if (et.inverse()) //is the constant term of the poly not 1
        {
            for (size_t i = 0, k = _pascal.size() - 1; i < output.size(); i++, k--)
            {
                output[i] = _pascal[k]; output[i] *= et.inverse();
                for (size_t j = et[i].size() - 1, p = 0; j > 0; j--, p++)
                {
                    if constexpr (std::is_class<T>())
                        output[i].FM3IAD(et[i][j],output[p],et.inverse());
                    else
                        output[i] += ((T)0) - (et[i][j] * output[p] * et.inverse());
                }
                    
            }
        }
        else
        {
            for (size_t i = 0; i < output.size(); i++)
            {
                output[i] = _pascal[i];
                for (size_t j = et[i].size() - 1; j > 0; j--)
                {
                    if constexpr (std::is_class<T>())
                        output[i].FM2IAD(et[i][j], output[j]);
                    else
                        output[i] += ((T)0) - (et[i][j] * output[j]);
                }
            }
        }
    }

    template <typename T> vector<T> AsPolynomial(const span<T>& _pascal, const ElectiveTransform<T>& et)
    {
        vector<T> result(_pascal.size());
        ToPolynomial<T>(_pascal, result, et);

        return result;
    }

    template<typename T> void ToFunction(const span<T>& polynomial, const span<T>& output,const ElectiveSymmetry<T>& es,bool P = false)
    {
        auto core = [&](size_t k, size_t i)
        {
            output[k] = 0;

            for (size_t j = 0, p = polynomial.size() - 1; j < polynomial.size(); j++, p--)
            {
                if constexpr (std::is_class<T>())
                    output[k].FMADD(es[i][p], polynomial[j]);
                else
                    output[k] += es[i][p] * polynomial[j];
            }
        };

        if (P)
        {
            std::atomic<size_t> identity = 0;
            std::for_each_n(std::execution::par, output.data(), es.size() - es.size() - output.size(), [&](auto v)
            {
                auto k = identity++;

                core(k, es.size() - output.size() + k);
            });
        }
        else
            for (size_t i = es.size() - output.size(), k = 0; i < es.size(); i++, k++)
                core(k, i);
    }

    template<typename T, typename SHIM> void ToFunctionR(const SHIM& polynomial, const span<T>& output, const ElectiveSymmetry<T>& es, size_t offset = 0, bool P = false)
    {
        auto core = [&](size_t k, size_t i)
        {
            output[k] = 0;

            for (size_t j = 0; j < polynomial.size(); j++)
            {
                if constexpr (std::is_class<T>())
                    output[k].FMADD(es[i][j], polynomial[j]);
                else
                    output[k] += es[i][j] * polynomial[j];
            }
        };

        if (P)
        {
            std::atomic<size_t> identity = 0;
            std::for_each_n(std::execution::par, output.data(), (es.size() - offset) - (es.size() - output.size() - offset), [&](auto v)
            {
                auto k = identity++;
                
                core(k, es.size() - output.size() - offset + k);
            });
        }
        else
            for (size_t i = es.size() - output.size() - offset, k = 0; i < es.size() - offset; i++, k++)
                core(k, i);
    }

    template<typename T> vector<T> AsFunction(const span<T>& polynomial, const ElectiveSymmetry<T>& es)
    {
        vector<T> result;
        result.reserve(es.size());

        ToFunction<T>(polynomial, result, es);

        return result;
    }
}