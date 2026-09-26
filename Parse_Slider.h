#pragma once

#include "Slider_Body_Pop2.h"
#include "Slider_Body_Pop4.h"
#include "Slider_Body_Neg.h"

TATAI_FORCE_INLINE void parse_slider_pair_GENERAL(const __m128i m0, const __m128i shuffle, slider_point* const out) noexcept {

    const auto d = _mm_shuffle_epi8(m0, shuffle);
    
    const auto pairs = _mm_maddubs_epi16(d, _mm_setr_epi8(0, 10, 10, 1, 0, 10, 10, 1, 0, 10, 10, 1, 0, 10, 10, 1));
    
    const auto result = _mm_madd_epi16(pairs, _mm_setr_epi16(10, 1, 10, 1, 10, 1, 10, 1));
    
    _mm_storeu_si128((__m128i*)out, result);

}

TATAI_FORCE_INLINE u32 parse_two_slider_points(const char* __restrict p, slider_point* const __restrict out) {

    // still need error flag detection for objects outside the digit range of 1-3
    //      example: 0:1234
    // once maybe returning back 0 just plops into a general parser
    // (potentially staying in that general parser for the rest of the path)

    const auto m0 = _mm_loadu_si128((const __m128i*)p);

    const auto X = (u32)_mm_movemask_epi8(
        _mm_shuffle_epi8(
            _mm_setr_epi8(
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                (char)-1, // :
                0,
                (char)-1, // |,
                (char)-1, // -
                0, 0
            ),
            m0)
    );

    const auto comma_xmm = _mm_cmpeq_epi8(m0, _mm_set1_epi8(',')); // should lifetime extend for free

    const auto first4 = (u32)_pdep_u32(0b1111u, X);

    if (const u32 negative = first4 & ((first4 << 1u) + 1u);
        negative) [[unlikely]] {

        const u32 sep = X & ~negative;

        const auto first2 = (u32)_pdep_u32(0b11, sep);

        const auto end = (u32)_blsi_u32(_blsr_u32(first2));

        const u32 point_negative = negative & (end - 1u);

        if (point_negative){

            return slider_body_neg::parse_slider_point_negative(
                p,
                out,
                first2 | (point_negative << 16u),
                (u32)_mm_movemask_epi8(comma_xmm)
            );
        }

        {

            const auto clean_first4 = (u32)_pdep_u32(0b1111u, sep);

            const auto digits = _mm_sub_epi8(m0, _mm_set1_epi8('0'));

            const u32 key = (first2 * 3u) & 0x6Cu;

            const auto* tbl = (const u32*)(slider_body_pop2::POINT_SINGLE_SHUF_DELIM2.data());
            
            parse_slider_pair_GENERAL(digits, _mm_load_si128((const __m128i*)(tbl + key)), out);

            const u32 consumed = 31u - std::countl_zero(first2);

            return (1u | (1 << 24)) + (consumed << 24u) ;
        }

    }

    const auto commas = (u32)_mm_movemask_epi8(comma_xmm);
    const auto digits = _mm_sub_epi8(m0, _mm_set1_epi8('0'));

    const u32 first = _blsr_u32(first4);
    const u32 second = _blsi_u32(first); // i really dont like this

    if (second & commas) [[unlikely]] { // POP 2

        const u32 key = (first4 * 3u) & 0x6Cu;

        const auto* tbl = (const u32*)(slider_body_pop2::POINT_SINGLE_SHUF_DELIM2.data());

        parse_slider_pair_GENERAL(digits, _mm_load_si128((const __m128i*)(tbl + key)), out);

        return (1u | 0x00ffff00u | (1u << 24u)) + (_tzcnt_u32(second) << 24u);
    }

    { // POP 4

        const u32 key = ((first4 * 27151u) >> 5u) & 0xff0u;

        parse_slider_pair_GENERAL(
            digits,
            _mm_load_si128((const __m128i*)((const u8*)slider_body_pop4::POINT_PAIR_SHUF_DELIM4.table.data() + key)),
            out
        );

        const u32 consumed = 31u - std::countl_zero(first4);

        return ((2u | (1 << 24)) | ((first4 & commas) << 8)) + (consumed << 24);
    }

}

#include "Parse_Double.h"

//__declspec(noinline)
TATAI_FORCE_INLINE
void parse_slider_body(const char*__restrict p, slider_point* __restrict slider_ptr, _slider_data *const __restrict r) {

    { // hitsound

        const auto v = parse_integer_m2::likely_1(load_u32(p));

        p += (v >> 32);
        //p += 4;

    }
    
    r->curve_type = *p;
    p += 2;

    r->point_start = slider_ptr;

    for (;;) {

        const auto result = parse_two_slider_points(p, slider_ptr);

        slider_ptr += u8(result);
        p += (result >> 24);

        if (result & 0x00FFFF00)
            break;

    }

    r->point_end = slider_ptr;

    r->slides = (p[0] & 0x0f);

    if (p[1] == ',') { // X,

        p += 2;

    } else {

        ++p;

        while (*p != ',') {

            r->slides *= 10;
            r->slides += (p[0] & 0x0f);
            ++p;

        }

        ++p;

    }

    const auto v_raw = parse_double::from_ascii::load_ascii_decimal_16(p);

    const auto consumed = fu8(v_raw & 0b11111ull);
    const auto frac = fu32((v_raw) & 0b1111000000ull);
    const u64 sig = (v_raw >> 10u);
    
    p += consumed;

    if (consumed == 17) {

        // sadly, decimal numbers can be rarely be over 16 digits - so we need to fix this

        while ((u32(*(p-1)) - (u32)'0') < 10) [[unlikely]]
            ++p;
    
    }    
    
    r->length = sig == 0 ? 0. : 
        frac == 0 ? double(sig) :
            parse_double::to_double::compute_min_10(sig, frac >> 6);

    return;
}