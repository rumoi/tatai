#pragma once

#if defined(__clang__)

static inline uint64_t _shlx_u64(uint64_t __X, unsigned int __Y) {
    return __X << (__Y & 63u);
}

#endif

namespace parse_double{   

    namespace to_double {

        alignas(64) inline constexpr std::array<u64, 15> POW5 = {
                1ull,
                5ull,
                25ull,
                125ull,
                625ull,
                3125ull,
                15625ull,
                78125ull,
                390625ull,
                1953125ull,
                9765625ull,
                48828125ull,
                244140625ull,
                1220703125ull,
                6103515625ull
        };

        alignas(64) inline constexpr std::array<u8, 15> LOG2_POW5 = { 0, 2, 4, 6, 9, 11, 13, 16, 18, 20, 23, 25, 27, 30, 32 };

        alignas(64) inline constexpr std::array<u64, 15> NORMALIZED_POW5 = [](){

            std::array<u64, 15> result{};

            for (size_t i{}; i < result.size(); ++i)
                result[i] = POW5[i] << (63u - LOG2_POW5[i]);

            return result;
        }();        

        alignas(64) inline constexpr std::array<u64, 15> DECIMAL_R = [](){

            constexpr std::array<u64, 15> table{ 0,
                0xcccccccccccccccdull,
                0xa3d70a3d70a3d70bull,
                0x83126e978d4fdf3cull,
                0xd1b71758e219652cull,
                0xa7c5ac471b478424ull,
                0x8637bd05af6c69b6ull,
                0xd6bf94d5e57a42bdull,
                0xabcc77118461cefdull,
                0x89705f4136b4a598ull,
                0xdbe6fecebdedd5bfull,
                0xafebff0bcb24aaffull,
                0x8cbccc096f5088ccull,
                0xe12e13424bb40e14ull,
                0xb424dc35095cd810ull
            };

            std::array<u64, 15> result{};

            for (size_t i{1}; i < result.size(); ++i) {

                const u64 x = table[i];

                result[i] = (x >> 2) + u64((x & 3ull) != 0);

            }

            return result;
        }();

        alignas(64) inline constexpr std::array<u64, 15> PACKED_POW5 = [](){

            std::array<u64, 15> result{};

            for (size_t i{}; i < result.size(); ++i) {

                const u64 D{ POW5[i] };

                result[i] = (D << 7u) | u64(62u + LOG2_POW5[i]);

            }

            return result;
        }();

        // sending frac 0 here is an error, Can be ~1 ULP off between 0-10
        inline double compute_min_10(const u64 sig, const u32 frac) noexcept {
            // extremely load heavy - look into?

            TATAI_ASSUME(sig != 0);
            TATAI_ASSUME(frac >= 1 && frac <= 14);

            const auto packed = (u64)PACKED_POW5[frac];

            const auto lz = (u32)std::countl_zero(sig);

            const u64 normalized_sig = sig << lz;

            const auto reciprocal_shift = 73u - lz - (u32)(normalized_sig < NORMALIZED_POW5[frac]);

            const auto shift_base = (u32)(packed & 0x7full);
            const auto shift = (shift_base - reciprocal_shift) + frac;

            const u64 d = packed >> 7u;

            //const auto lo = sig << shift;
            // just doing the shift is technically UB in c++ if shift is over 63 (happens in values under 10.) -
            // but masking it causes MSVC to generate an extra AND that isnt needed

            u64 p_h;
#if defined(_MSC_VER)
            const auto p_l = (u64)_umul128(sig, DECIMAL_R[frac], &p_h);
#else
            const auto product = (__uint128_t)sig * DECIMAL_R[frac];
            const auto p_l = (u64)product;
            p_h = (u64)(product >> 64);
#endif
            const auto lo = _shlx_u64(sig, shift);

#if defined(_MSC_VER)
            auto q = (u64)__shiftright128(p_l, p_h, reciprocal_shift);
#else
            auto q = (u64)((((__uint128_t)p_h << 64) | p_l) >> reciprocal_shift);
#endif

            const u64 x = lo - u64(q * d) + u64(d >> 1u);

            q += u64(x >= d);

            const u32 biased_exponent = 1075u - shift + u32(q >> 53);

            const auto fraction = (u64)_bzhi_u64(q, 52u);

            const auto bits = (u64(biased_exponent) << 52u) | fraction;

            // MSVC somehow fails on a normal std::bit_cast
            return _mm_cvtsd_f64(_mm_castsi128_pd(_mm_cvtsi64_si128((long long)(bits))));
        }

    }    

    namespace from_ascii {

        alignas(64) inline constexpr auto DECIMAL_SHUF = []() {

            std::array<std::array<u8, 16>, 17 * 17> table{};

            for (auto& e : table)
                for (auto& x : e)
                    x = 0x80ul;

            for (size_t i{ 2 }; i < 17; ++i) {

                for (size_t x{ 1 }; x < i; ++x) {

                    auto& s{ table[i * 17 + x] };

                    const size_t digits = i - 1;
                    size_t out = 16 - digits;

                    for (u32 in{}; in < i; ++in) {
                        if (in != x)
                            s[out++] = u8(in);
                    }

                }

            }

            return table;
        }();

        alignas(64) inline constexpr auto INTEGER_SHUF = []() {

            std::array<std::array<u8, 16>, 17> result{};

            for (size_t len{}; len < 17; ++len) {

                auto& s = result[len];

                for (auto& v : s)
                    v = 0x80u;

                const auto dst = 16u - len;

                for (u32 i{}; i < len; ++i)
                    s[dst + i] = u8(i);

            }

            return result;
        }();

        alignas(64) inline constexpr auto DECIMAL16_SHUF = []() {

            std::array<std::array<u8, 16>, 16> table{};

            for (size_t dot{}; dot < 16; ++dot) {

                auto& s = table[dot];

                s[0] = 0x80;

                for (size_t src{}, dst{ 1 }; src < 16; ++src) {
                    if (src != dot)
                        s[dst++] = u8(src);
                }
            }

            return table;
        }();

        TATAI_FORCE_INLINE u64 compute_decimal16(__m128i shuff) {

            const auto inter0 = _mm_maddubs_epi16(shuff, _mm_setr_epi8(10, 1, 10, 1, 10, 1, 10, 1, 10, 1, 10, 1, 10, 1, 10, 1));

            const auto inter1 = _mm_madd_epi16(inter0, _mm_setr_epi16(100, 1, 100, 1, 100, 1, 100, 1));

            const auto packed = _mm_packs_epi32(inter1, inter1);

            const auto halves = _mm_madd_epi16(packed, _mm_setr_epi16(10000, 1, 10000, 1, 10000, 1, 10000, 1));

            const u64 res = (u64)_mm_cvtsi128_si64(halves);

            return u64(u32(res)) * 100000000ull + (res >> 32);
        }

        inline u64 load_ascii_decimal_16(const char* p) noexcept {

            const auto raw = _mm_loadu_si128((__m128i const*)p);

            const auto numeric = _mm_sub_epi8(raw, _mm_set1_epi8('0'));

            const auto special = (u32)_mm_movemask_epi8(numeric);

            // TODO: handle 16 digits of int without any .
            // just checking if special is non zero fixes it, but i dont want to gimp the execution speed like that

            if (_blsr_u32(special) == 0) { // assume we are a 15 digit decimal with a '.' 

                const auto dot = (u64)_tzcnt_u32(special);

                const auto shuff = _mm_shuffle_epi8(numeric, _mm_load_si128((__m128i const*)(DECIMAL16_SHUF[dot].data())));

                const auto sig = compute_decimal16(shuff);

                return (sig << 10u) | ((17ull) | (15ull << 6)) - (dot << 6u);
            }

            const auto dots = (u32)_mm_movemask_epi8(_mm_cmpeq_epi8(raw, _mm_set1_epi8('.')));

            if ((_blsi_u32(special) & dots) == 0) { // integer case

                const u32 len = _tzcnt_u32(special | 0x10000u);

                if (len <= 3) {

                    const auto x = (u32)_mm_cvtsi128_si32(numeric);

                    const u32 d0 = x & 0xffu;
                    const u32 d1 = (x >> 8) & 0xffu;
                    const u32 d2 = (x >> 16) & 0xffu;

                    const u32 v2 = d0 * 10u + d1;
                    const u32 v3 = v2 * 10u + d2;

                    // len 1 is mega rare, maybe hoist it out?
                    const u32 value = len == 1u ? d0 : len == 2u ? v2 : v3;

                    return u64(1u + len) | (u64(value) << 10u);
                }

                // 0 case
                //if (len == 0) return 1u;
                
                // general fall through case

                const auto shuff = _mm_shuffle_epi8(numeric, _mm_load_si128((__m128i const*)(INTEGER_SHUF[len].data())));

                const auto sig = compute_decimal16(shuff);

                return (len + 1u) | (sig << 10u);
            }

            const u32 dot = _tzcnt_u32(dots);

            const u32 len = _tzcnt_u32((special ^ dots) | 0x10000u);

            const auto shuff = _mm_shuffle_epi8(numeric, _mm_load_si128((__m128i const*)(DECIMAL_SHUF[len * 17 + dot].data())));

            const auto sig = compute_decimal16(shuff);

            return (sig << 10ull) | (u64(len - dot - 1u) << 6u) | (1ull + len);
        }

    }

}
