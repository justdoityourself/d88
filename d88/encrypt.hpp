/* Copyright (C) 2020 D8DATAWORKS - All Rights Reserved */

#pragma once

#include "base.hpp"

using namespace std;

namespace d88
{
    namespace security
    {
        /*
            Why can't we just invert this process?
            How can we break the Elective Matrix Symmetry? 
            
            With both Data any Poly we can extract the sym.
            Since we can, there are a lot of very cool things we can do instead of encryption... like cloning algorithms and making them faster in the process.

            But as a common encryption practice, SYM must be mutated a level up, or an initialization vector used each block.

            Knowing the initial block doesn't break encryption unless it reveals other blocks too. AES used without mutating the IV has this same flaw.

            Example:

            Data => ?SYM? => Poly

            1 1 1 => ?SYM? => 3 0 6

            ?SYM? == ??

            S1 + T1 = 0


            S1          S2          S3

            T1          S1-S2       S2-S3

            S1          T1-S1+S2    S1-2S2+S3


            Row3 - Row1, Row2 + Row1 =>


            S1          S2          S3

            0           T1-S1       S1-2S2

            0           S1          S2


            2Row3 + Row2 =>

            0           0           S1

            Row3 - S1 == S2

            

            Implemented in analysis.hpp, at the moment this turned out to be a little more complicated for two reasons, inverse mod 2^n and the common inverse to each row.
            Both of these have solutions but will take more time to implement. I'm glad this turned out to be more complicated as it is a very important procedure and I'd feel
            bad if it was easy and just sat infront of me for years without seeing it :D

        */

        template <typename T, size_t S> class EncryptContextLong
        {
        public:
            EncryptContextLong() :pt(S) {}
            EncryptContextLong(const span<T>& sym):pt(S),et(sym) {}

            void Init(const span<T>& sym)
            {
                et.Init(sym);
            }

            const PascalTriangle<T>& Pascal() const { return pt; }
            const ElectiveTransform<T>& Transform() const { return et; }

        private:
            PascalTriangle<T> pt;             //TODO compile time buffers and initialization
            ElectiveTransform<T> et;
        };

        template <typename T, size_t S> class EncryptContextShort
        {
        public:
            EncryptContextShort(const span<T>& sym) : et(sym) {}

            const ElectiveTransform<T>& Transform() const { return et; }

        private:

            ElectiveTransform<T> et;
        };

        template <typename T, size_t S> class DecryptContextShort
        {
        public:
            DecryptContextShort(const span<T>& sym):es(sym,S) { }

            const ElectiveSymmetry<T>& Symmetry() const { return es; }
        private:
            ElectiveSymmetry<T> es;
        };

        template <typename T, size_t S> class DecryptContextLong
        {
        public:
            DecryptContextLong(const span<T>& sym) :pt(S), es(sym, S) { }

            const ElectiveSymmetry<T>& Symmetry() const { return es; }

            const PascalTriangle<T>& Pascal() const { return pt; }
        private:
            PascalTriangle<T> pt;             //TODO compile time buffers and initialization
            ElectiveSymmetry<T> es;
        };

        template <typename T, size_t S> void block_encrypt_long(const span<T> & source, const span<T>& scratch,const span<T> & dest, const EncryptContextLong<T,S> & context)
        {
            ToPascal<T>(source, scratch, context.Pascal());
            ToPolynomial<T>(scratch, dest,  context.Transform());
        }

        template <typename T, size_t S> void block_decrypt_short(const span<T>& source, const span<T>& dest, const DecryptContextShort<T,S> & context)
        {
            ToFunction<T>(source, dest, context.Symmetry());
        }

        template <typename T, size_t S> void block_encrypt_short(const span<T>& source, const span<T>& dest, const EncryptContextShort<T, S>& context)
        {
            ToPolynomial<T>(source, dest, context.Transform());
        }

        template <typename T, size_t S> void block_decrypt_long(const span<T>& source, const span<T>& scratch, const span<T>& dest, const DecryptContextLong<T, S>& context)
        {
            ToFunction<T>(source, scratch, context.Symmetry());
            ToPascal<T>(scratch, dest, context.Pascal());
        }
    }
}