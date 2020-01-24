/* Copyright (C) 2020 D8DATAWORKS - All Rights Reserved */

#pragma once

#include "base.hpp"
#include "encrypt.hpp"

using namespace std;

namespace d88
{
    namespace security
    {
        template <typename T, size_t S> class HashContextFeedback
        {
        public:
            HashContextFeedback() :pt(S) {}

            const PascalTriangle<T>& Pascal() const { return pt; }

        private:
            PascalTriangle<T> pt;             //TODO compile time buffers and initialization
        };

        template <typename T, size_t S> using HashContextLong = EncryptContextLong<T,S>;

        template <typename T, size_t S> using HashContextShort = EncryptContextShort<T, S>;

        //NOTE, this method does very badly when it comes to simple and basic patterns
        template <typename T, size_t S> void block_feedback_hash(const span<T>& source, const span<T>& scratch, const span<T>& dest, const HashContextFeedback<T, S>& context)
        {
            ToPascal<T>(source, scratch, context.Pascal());
            ElectiveTransform<T> et(source);
            ToPolynomial<T>(scratch, dest, et);
        }

        template <typename T, size_t S> void block_hash_long(const span<T>& source, const span<T>& scratch, const span<T>& dest, const HashContextLong<T, S>& context)
        {
            ToPascal<T>(source, scratch, context.Pascal());
            ToPolynomial<T>(scratch, dest, context.Transform());
        }

        template <typename T, size_t S> void block_hash_short(const span<T>& source, const span<T>& dest, const HashContextShort<T, S>& context)
        {
            ToPolynomial<T>(source, dest, context.Transform());
        }
    }
}