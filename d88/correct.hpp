/* Copyright (C) 2020 D8DATAWORKS - All Rights Reserved */

#pragma once

#include <vector>
#include <utility>

#include "base.hpp"
#include "util.hpp"

using namespace std;

namespace d88
{
    namespace correct
    {
        template < typename T, typename A, typename B > vector<T> row_add(const A& a, const B& b)
        {
            vector<T> c(a.size());

            for (size_t i = 0; i < a.size(); i++)
                c[i] = a[i] + b[i];

            return c;
        }

        template < typename A, typename B > void row_add_eq(A& a, const B& b)
        {
            for (size_t i = 0; i < a.size(); i++)
                a[i] += b[i];
        }

        template < typename T, typename A > vector<T> row_mul(const A& a, T m)
        {
            vector<T> c(a.size());

            for (size_t i = 0; i < a.size(); i++)
                c[i] = a[i] * m;

            return c;
        }

        template < typename T, typename A > void row_mul_eq(A& a, T m)
        {
            for (size_t i = 0; i < a.size(); i++)
                a[i] *= m;
        }

        template < typename T, typename A, typename B > void row_mul_sub_eq(A& a, const B& b, T s)
        {
            for (size_t i = 0; i < a.size(); i++)
                a[i] -= b[i] * s;
        }

        template <typename T> void row_solve(vector<vector<T>> & m, span<T> s)
        {
            auto Inverse = [&](size_t i)
            {
                auto inv = GetInverse(m[i][i]);
                row_mul_eq(m[i], inv);
                s[i] = s[i] * inv;
            };

            for (size_t i = 0; i < m.size(); i++)
            {
                bool valid = false;

                auto forward = [&]()
                {
                    for (size_t j = i; j < m.size(); j++)
                    {
                        if (m[j][i] != 0)
                        {
                            if (i != j)
                            {
                                swap(m[i], m[j]);
                                swap(s[i], s[j]);
                            }

                            valid = true;
                            break;
                        }
                    }
                };

                auto backward = [&]()
                {
                    for (size_t j = 0; j < i && !valid; j++)
                    {
                        if (m[j][i] != 0)
                        {
                            auto tm = row_add<T>(m[i + 1], m[j]);
                            auto ts = s[i + 1] + s[j];

                            row_add_eq(m[i], m[j]);
                            row_add_eq(m[i], tm);

                            s[i] = s[i] + s[j];
                            s[i] = s[i] + ts;

                            valid = true;
                            break;
                        }
                    }
                };

                while (!valid)
                {
                    forward();
                    if (valid)break;

                    backward();
                    if (!valid) break;

                    valid = false;
                }

                if (!valid) throw "matrix error";

                if (m[i][i] != 1)
                    Inverse(i);

                for (size_t j = 0; j < m.size(); j++)
                {
                    if (i == j) continue;

                    if (m[j][i])
                    {
                        s[j] -= s[i] * m[j][i];
                        row_mul_sub_eq(m[j], m[i], m[j][i]);
                    }
                }
            }
        }


        /*template <typename T, size_t S, size_t E> class ExtendPascalContext
        {
        public:
            ExtendPascalContext() :pt(S+E) 
            {
                auto& m = pt.Mutate();

                for (size_t i = S; i < S + E; i++)
                    for (size_t j = 0; j < m[i].size(); j++)
                        if (m[i][j] % 2 == 0)
                            m[i][j] ++;
            }

            const PascalTriangle<T>& Pascal() const { return pt; }

        private:
            PascalTriangle<T> pt;             //TODO compile time buffers and initialization
        };


        template<typename T, size_t S, size_t E> void extend_pascal(const span<T>& source, const span<T>& dest, const ExtendPascalContext<T,S,E> & ctx)
        {
            auto p = ctx.Pascal();

            for (size_t i = 0; i < E; i++)
            {
                T s = 0;
                for (size_t j = 0; j < source.size() && j < p[S + i].size(); j++)
                    s += source[j] * p[S + i][j];

                dest[i] = s;
            }
        }


        //todo scroll the triangle, optimize
        template <typename T, size_t S, size_t E> class RecoverPascalContext
        {
        public:
            RecoverPascalContext(const span<T>& dx) :pt(S + E)
            {
                auto& mu = pt.Mutate();

                for (size_t i = S; i < S + E; i++)
                    for (size_t j = 0; j < mu[i].size(); j++)
                        if (mu[i][j] % 2 == 0)
                            mu[i][j] ++;

                m = GenerateIdentity<T>(S);

                for (size_t i = 0; i < dx.size(); i++)
                {
                    if (dx[i] != i)
                    {
                        m[i] = pt[dx[i]];

                        while (m[i].size() < S) m[i].push_back(0);
                        m[i].resize(S);
                    }
                }
            }

            const PascalTriangle<T>& Pascal() const { return pt; }
            vector<vector<T>> & Solution() { return m; }

        private:
            vector<vector<T>> m;
            PascalTriangle<T> pt;             //TODO compile time buffers and initialization
        };

        //recover_short lacks the proper symmetry to be able to recover where the row/col of the solution matrix colide with an EVEN element of the pascal row.
        //This is due to 2^n being our prime base.
        template<typename T, size_t S, size_t E> void recover_pascal(span<T> source, RecoverPascalContext<T, S, E>& ctx)
        {
            row_solve(ctx.Solution(),source);
        }*/



        template <typename T, size_t S, size_t E> class ExtendShortContext
        {
        public:
            ExtendShortContext(const span<T>& sym) : /*et(sym),*/ es(sym, S * 2) {}

            //const ElectiveTransform<T>& Transform() const { return et; }
            const ElectiveSymmetry<T>& Symmetry() const { return es; }

            vector<vector<T>> & Mutate() { return es.Mutate(); }

        private:
            //ElectiveTransform<T> et;
            ElectiveSymmetry<T> es;
        };

        /*
        ElectiveSymmetry Matrix Solutions Mod 2^n

        Odd Power Configuration
        1
            1
        1       1
            1   1   1
        1               1
            1           1   1
        1       1       1       1

        Solution for all rows is valid ever S rows. If we are not keeping all S we must combine them so solution is present in E blocks instead.
        */

        template <typename T, size_t S, size_t E> void InterleaveElectiveMatrix(vector<vector<T>>& s, const span<T>& temp)
        {
            //Solve matrix for even / odd identity
            for (size_t i = 0; i < S; i++)
            {
                //Find odd row:
                //

                size_t odd_dx = -1;
                for (size_t j = i; j < S; j++)
                {
                    if (s[S + j][i] % 2)
                    {
                        odd_dx = j;
                        break;
                    }
                }

                if (odd_dx == -1)
                    throw "Symmetry Error, no odd row/column";

                //Make [i][i] odd:
                //

                if (odd_dx != i)
                {
                    row_add_eq(s[S + i], s[S + odd_dx]);
                    temp[i] += temp[odd_dx];
                }

                //Eleminate all other odd items in column:
                //

                for (size_t j = 0; j < S; j++)
                {
                    if (j == i)
                        continue;

                    if (s[S + j][i] % 2)
                    {
                        row_add_eq(s[S + j], s[S + i]);
                        temp[j] += temp[i];
                    }
                }
            }

            //Interleave mod E:
            //

            for (size_t i = E; i < S; i++)
            {
                row_add_eq(s[S + i % E], s[S + i]);
                temp[i % E] += temp[i];
            }
        }

        template <typename T, size_t S, size_t E> vector<pair<size_t, size_t>> MapInterleaveElectiveMatrix(vector<vector<T>>& s)
        {
            vector<pair<size_t, size_t>> result;

            //Solve matrix for even / odd identity
            for (size_t i = 0; i < S; i++)
            {
                //Find odd row:
                //

                size_t odd_dx = -1;
                for (size_t j = i; j < S; j++)
                {
                    if (s[S + j][i] % 2)
                    {
                        odd_dx = j;
                        break;
                    }
                }

                if (odd_dx == -1)
                    throw "Symmetry Error, no odd row/column";

                //Make [i][i] odd:
                //

                if (odd_dx != i)
                {
                    row_add_eq(s[S + i], s[S + odd_dx]);
                    result.push_back(make_pair(i,odd_dx));
                }

                //Eleminate all other odd items in column:
                //

                for (size_t j = 0; j < S; j++)
                {
                    if (j == i)
                        continue;

                    if (s[S + j][i] % 2)
                    {
                        row_add_eq(s[S + j], s[S + i]);
                        result.push_back(make_pair(j, i));
                    }
                }
            }

            //Interleave mod E:
            //

            for (size_t i = E; i < S; i++)
            {
                row_add_eq(s[S + i % E], s[S + i]);
                result.push_back(make_pair(i % E, i));
            }

            return result;
        }

        template <typename T, size_t S, size_t E> void RunMappedInterleave(const vector<pair<size_t,size_t>> & map, const span<T>& column)
        {
            for (auto& ab : map)
                column[ab.first] += column[ab.second];
        }

        template <typename T, size_t S, size_t E> void ComputeInterleaveElectiveMatrix(vector<vector<T>>& s)
        {
            //Solve matrix for even / odd identity
            for (size_t i = 0; i < S; i++)
            {
                //Find odd row:
                //

                size_t odd_dx = -1;
                for (size_t j = i; j < S; j++)
                {
                    if (s[S + j][i] % 2)
                    {
                        odd_dx = j;
                        break;
                    }
                }

                if (odd_dx == -1)
                    throw "Symmetry Error, no odd row/column";

                //Make [i][i] odd:
                //

                if (odd_dx != i)
                    row_add_eq(s[S + i], s[S + odd_dx]);

                //Eleminate all other odd items in column:
                //

                for (size_t j = 0; j < S; j++)
                {
                    if (j == i)
                        continue;

                    if (s[S + j][i] % 2)
                        row_add_eq(s[S + j], s[S + i]);
                }
            }

            //Interleave mod E:
            //

            for (size_t i = E; i < S; i++)
                row_add_eq(s[S + i % E], s[S + i]);
        }

        template<typename T, size_t S, size_t E, size_t C = 0> void extend_short(const span<T>& source, const span<T> & temp, const span<T>& dest, ExtendShortContext<T, S, E>& ctx)
        {
            auto & s = ctx.Mutate();
            //ToFunction<T>(source, temp, ctx.Symmetry());
            ToFunctionR<T>(source, temp, ctx.Symmetry());

            InterleaveElectiveMatrix<T, S, E>(s, temp);

            //Output result:
            //

            for (size_t i = 0; i < E + C; i++)
                dest[i] = temp[i];
        }


        template <typename T, size_t S, size_t E> class ImmutableShortContext
        {
        public:
            ImmutableShortContext(const span<T>& sym) : es(sym, S * 2) 
            {
                auto copy = es.Duplicate();
                map = MapInterleaveElectiveMatrix<T,S,E>(copy);
            }

            const ElectiveSymmetry<T>& Symmetry() const { return es; }

            const vector<pair<size_t, size_t>>& Map() { return map; }

        private:
            vector<pair<size_t, size_t>> map;
            ElectiveSymmetry<T> es;
        };

        template<typename T, size_t S, size_t E,size_t C=0> void immutable_extend_short(const span<T>& source, const span<T>& temp, const span<T>& dest, ImmutableShortContext<T, S, E>& ctx)
        {
            ToFunctionR<T>(source, temp, ctx.Symmetry());

            RunMappedInterleave<T,S,E>(ctx.Map(), temp);

            //Output result:
            //

            for (size_t i = 0; i < E+C; i++)
                dest[i] = temp[i];
        }

        template <typename T, size_t S, size_t E> class RecoverShortContext
        {
        public:
            RecoverShortContext(const span<T>& sym) : es(sym, S * 2) 
            {
                auto & s = es.Mutate();
                ComputeInterleaveElectiveMatrix<T, S, E>(s);
            }

            const ElectiveSymmetry<T>& Symmetry() const { return es; }

        private:
            ElectiveSymmetry<T> es;
        };

        /*
        Working Explanation

        Elective Symmetry Mod 8
        3 5 1
        2 4 7
        2 3 1
        1 6 7
        5 1 1
        4 0 7
        
        Elective Transform
        1       =>      3
        5 1     =>      2
        3 5 1   =>      1

        Number Triangle
        4
            1
        5       3
            2
        5       5
            3
        6       3
            0
        2
        
        Rotated Solution
        3 B 1 = F()

        1 5B 1 = F(1)
        6 4B 7 = F(2)
        6 3B 1 = F(3)
        3 6B 7 = 6
        7 1B 1 = 2

        ... ( MOD 8, Remember )

          6B 2 = 6 ( No Solution, Remember 2^N relative prime fields requires solution pairs, but provides same bit coverage )
          1B 0 = 2 ( SOVLED )

        ...

        ( 6B 2 = 6 ) + ( 1B 0 = 2 ) ( using pair rule )

        7B 2 = 0

        ...

        7B = 6 ( GCD => 7 * ? == 1 ... ? = 7 )

        ...

        B = 2
           
        */

        template<typename T, size_t S, size_t E> void recover_short(const span<T> & source, const span<T>& dx, RecoverShortContext<T, S, E>& ctx)
        {
            vector<vector<T>> m;
            vector<T> s , t;
            auto es = ctx.Symmetry();

            for (size_t i = 0; i < dx.size(); i++)
            {
                if (dx[i] != i)
                {
                    m.push_back(es[dx[i]]);
                    s.push_back(source[i]);
                    source[i] = 0;
                    t.push_back(i);
                }
            }

            vector<vector<T>> m2(m.size());
            vector<T> s2;

            for (size_t i = 0; i < m.size(); i++)
            {
                if (m[i][t[i]] % 2 == 0)
                {
                    size_t swapdx = -1;
                    for (size_t j = i+1; j < t.size(); j++)
                    {
                        if (m[j][t[i]] % 2 != 0)
                        {
                            swapdx = j;
                            break;
                        }
                    }

                    if (-1 == swapdx)
                        throw "Interleave exception, bad symmetry detected.";

                    swap(m[i], m[swapdx]);
                    swap(s[i], s[swapdx]);
                }

                T sum = 0;
                for (size_t j = 0; j < source.size(); j++)
                {
                    for (size_t k = 0; k < t.size(); k++)
                    {
                        if (j == t[k])
                        {
                            m2[i].push_back(m[i][j]);
                            goto CONTINUE;
                        }
                    }

                    sum += (((T)0) - source[j]*m[i][j]);

                CONTINUE:
                    continue;
                }

                s2.push_back(sum+s[i]);
            }

            for (size_t i = 0; i < m2.size(); i++)
            {
                size_t inv = GetInverse<T>(m2[i][i]);
                row_mul_eq(m2[i], inv);
                s2[i] *= inv;
            }

            row_solve<T>(m2, s2);

            for (size_t i = 0; i < t.size(); i++)
                source[t[i]] = s2[i];
        }

        template <typename T,size_t S, size_t E,size_t C=0> bool validate_short(const span<T>& source, const span<T>& temp, const span<T>& ex, const span<T>& ex_temp, ExtendShortContext<T, S, E>& ctx)
        {
            extend_short<T,S,E,C>(source,temp, ex_temp, ctx);

            return equal(ex_temp.begin(), ex_temp.end(), ex.begin());
        }

        template <typename T, size_t S, size_t E,size_t C=0> bool validate_immutable_short(const span<T>& source, const span<T>& temp, const span<T>& ex, const span<T>& ex_temp, ImmutableShortContext<T, S, E>& ctx)
        {
            immutable_extend_short<T, S, E,C>(source, temp, ex_temp, ctx);

            return equal(ex_temp.begin(), ex_temp.end(), ex.begin());
        }

        template <typename T, size_t S, size_t E, size_t W,size_t C> bool repair_quick(const span<T>& source, const span<T>& temp1, const span<T>& temp2, const span<T>& ex, const span<T>& ex_temp, const span<T>& sym)
        {
            ImmutableShortContext<T,S,E> ctx(sym);

            for (size_t i = 0; i < S-E; i += W)
            {
                copy(source.begin(), source.end(), temp1.begin());

                auto dx = GenerateSequence<T>(S);
                for (size_t j = 0; j < E; j++)
                {
                    dx[i + j] = S + j;
                    temp1[i + j] = ex[j];
                }

                RecoverShortContext<T, S, E> rctx(sym);

                recover_short<T, S, E>(temp1, dx, rctx);

                if (validate_immutable_short<T, S, E,C>(temp1, temp2, ex, ex_temp, ctx))
                {
                    copy(temp1.begin(), temp1.end(), source.begin());
                    return true;
                }
            }

            return false;
        }

        template <typename T, size_t S, size_t E, size_t W, size_t C> bool repair_quick2(const span<T>& source, const span<T>& temp1, const span<T>& temp2, const span<T>& ex, const span<T>& ex_temp, const span<T>& sym, ImmutableShortContext<T, S, E> &ctx)
        {
            for (size_t i = 0; i < S - E; i += W)
            {
                copy(source.begin(), source.end(), temp1.begin());

                auto dx = GenerateSequence<T>(S);
                for (size_t j = 0; j < E; j++)
                {
                    dx[i + j] = S + j;
                    temp1[i + j] = ex[j];
                }

                RecoverShortContext<T, S, E> rctx(sym);

                recover_short<T, S, E>(temp1, dx, rctx);

                if (validate_immutable_short<T, S, E, C>(temp1, temp2, ex, ex_temp, ctx))
                {
                    copy(temp1.begin(), temp1.end(), source.begin());
                    return true;
                }
            }

            return false;
        }

        void repair_all()
        {
            //Will most likely not implement this, Takes a huge amount of time to complete and is the most unlikely form of corruption.
            //http://www.cplusplus.com/reference/algorithm/next_permutation/
        }


        //Todo optimize ether fast-encode/slow-decode or slow-encode/fast-decode... This is pretty tricky, will get to this later.
        /*template <typename T, size_t S, size_t E> class ExtendLongContext
        {
        public:
            ExtendLongContext(const span<T>& sym) : es(sym, S + E) {}

            const ElectiveSymmetry<T>& Symmetry() const { return es; }

        private:
            ElectiveSymmetry<T> es;
        };


        template<typename T, size_t S, size_t E> void extend_long(const span<T>& source, const span<T>& dest, const ExtendLongContext<T, S, E>& ctx)
        {
            ToFunction<T>(source, dest, ctx.Symmetry());
        }


        template <typename T, size_t S, size_t E> class RecoverLongContext
        {
        public:
            RecoverLongContext(const span<T>& dx, const span<T>& sym) : es(sym, S + E), m(S)
            {
                for (size_t i = 0; i < dx.size(); i++)
                {
                    if (dx[i] != i)
                        m[i] = es[dx[i]];
                    else
                        m[i] = es[i];
                }
            }

            const ElectiveSymmetry<T>& Symmetry() const { return es; }
            vector<vector<T>>& Solution() { return m; }

        private:
            vector<vector<T>> m;
            ElectiveSymmetry<T> es;
        };

        template<typename T, size_t S, size_t E> void recover_long(span<T> source, RecoverLongContext<T, S, E>& ctx)
        {
            row_solve(ctx.Solution(), source);
        }*/
    }
}