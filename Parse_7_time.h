#pragma once

namespace parse_7_time {

    alignas(64) inline constexpr std::array<u8, 16> SHUF_TBL7[9]{
    { 0x80,0x80,0x80,0x00,0x80,0x80,0x80,0x02,0x08,0x09,0x80,0x0a,0x04,0x05,0x06,0x07 },
    { 0x80,0x80,0x80,0x00,0x80,0x02,0x80,0x03,0x09,0x0a,0x80,0x0b,0x05,0x06,0x07,0x08 },
    { 0x80,0x80,0x80,0x00,0x02,0x03,0x80,0x04,0x0a,0x0b,0x80,0x0c,0x06,0x07,0x08,0x09 },
    { 0x80,0x00,0x80,0x01,0x80,0x80,0x80,0x03,0x09,0x0a,0x80,0x0b,0x05,0x06,0x07,0x08 },
    { 0x80,0x00,0x80,0x01,0x80,0x03,0x80,0x04,0x0a,0x0b,0x80,0x0c,0x06,0x07,0x08,0x09 },
    { 0x80,0x00,0x80,0x01,0x03,0x04,0x80,0x05,0x0b,0x0c,0x80,0x0d,0x07,0x08,0x09,0x0a },
    { 0x00,0x01,0x80,0x02,0x80,0x80,0x80,0x04,0x0a,0x0b,0x80,0x0c,0x06,0x07,0x08,0x09 },
    { 0x00,0x01,0x80,0x02,0x80,0x04,0x80,0x05,0x0b,0x0c,0x80,0x0d,0x07,0x08,0x09,0x0a },
    { 0x00,0x01,0x80,0x02,0x04,0x05,0x80,0x06,0x0c,0x0d,0x80,0x0e,0x08,0x09,0x0a,0x0b }
    };

    alignas(64) inline constexpr std::array<u8, 64 + 256> SHUF_XY_STORAGE_7D = [] {

        std::array<u8, 64 + 256> t{};

        auto write = [&t](size_t i, u16 v) {
            t[i] = u8(v & 0xff);
            t[i + 1] = u8((v >> 8) & 0xff);
        };

        const auto pack = [](u32 shuf_base, u32 consumed) {
            return u16((shuf_base << 8) | consumed);
        };

        size_t base = 64 - 10;

        write(base + 0x0a, pack(0 << 1, 13-1));
        write(base + 0x12, pack(1 << 1, 14-1));
        write(base + 0x22, pack(2 << 1, 15-1));

        write(base + 0x14, pack(3  << 1, 14-1));
        write(base + 0x24, pack(4 << 1, 15-1));
        write(base + 0x44, pack(5 << 1, 16-1));

        write(base + 0x28, pack(6 << 1, 15-1));
        write(base + 0x48, pack(7 << 1, 16-1));
        write(base + 0x88, pack(8 << 1, 17-1));

        return t;
    }();

    inline constexpr u8 const* SHUF_XY_INDEX_7D{ SHUF_XY_STORAGE_7D.data() + 64 - 10 };
    
    TATAI_FORCE_INLINE u32 parse_object_7digit_single(const char* __restrict  p, _object_header* const __restrict out_object) {
       
        const auto m0 = _mm_loadu_si128((__m128i const*)p);

        const auto v = (u32)_mm_movemask_epi8(_mm_cmpeq_epi8(m0, _mm_set1_epi8(',')));

        const auto digits = _mm_sub_epi8(m0, _mm_set1_epi8('0'));

        const u32 xy_pair = u8(v);

        const auto tbl_data = (u32)load_u16(SHUF_XY_INDEX_7D + xy_pair);

        if ((v & (xy_pair << 8u)) == 0u) [[unlikely]]
            return 0;

        const u32 yc = u8(tbl_data);
        const u32 shuf_base = tbl_data >> 8;

        const u64* const tbl = (u64 const*)SHUF_TBL7;
        const auto shuf = _mm_load_si128((__m128i const*)(tbl + shuf_base));

        const auto pairs = _mm_maddubs_epi16(_mm_shuffle_epi8(digits, shuf),
            _mm_setr_epi8(10, 1, 0, 1, 10, 1, 0, 1, 10, 1, 10, 1, 10, 1, 10, 1));

        // weights are so we can represent it as t0 * 4 + t1
        // which can be simplified as a u64 output stream as
        // u32(t) + u32(t >> 30)

        const auto result = _mm_madd_epi16(pairs, _mm_setr_epi16(10, 1,10, 1,10, 1, 25000, 250));

        if constexpr (is_clang) {

            const auto time_fix = _mm_srli_epi64(result, 30);
            const auto time = _mm_add_epi32(result, time_fix);
            const auto result_time = _mm_blend_epi16(result, time, 0x30);

            _mm_store_si128((__m128i*)out_object, result_time);

        } else {
        
            const u64 t = (u64)_mm_extract_epi64(result, 1);
        
            *(u64*)out_object = (u64)_mm_extract_epi64(result, 0);
        
            out_object->time = u32(t) + u32(t >> 30);
        
        }
        
        const auto [type_size, type] { parse_integer_m3::likely_1(load_u32(p + yc)) };

        out_object->type = type;

        return yc + type_size;
	}

    TATAI_NO_INLINE u32 NO_INLINE_parse_object_7digit_SIMD_single(const char* __restrict p, _object_header* const __restrict out_object) {
        return parse_object_7digit_single(p, out_object);
    }
    
      TATAI_FORCE_INLINE u32 parse_object_7digit_SIMD_pair(const char* __restrict p0, const char* __restrict p1,
        _object_header* __restrict out_object) {

        const auto m0 = _mm_loadu_si128((__m128i const*)p0);
        const auto m1 = _mm_loadu_si128((__m128i const*)p1);

        const auto v0 = (u32)_mm_movemask_epi8(_mm_cmpeq_epi8(m0, _mm_set1_epi8(',')));
        const auto v1 = (u32)_mm_movemask_epi8(_mm_cmpeq_epi8(m1, _mm_set1_epi8(',')));

        const auto digits0 = _mm_sub_epi8(m0, _mm_set1_epi8('0'));
        const auto digits1 = _mm_sub_epi8(m1, _mm_set1_epi8('0'));

        const u32 xy_pair0 = u8(v0);
        const u32 xy_pair1 = u8(v1);

        const auto tbl_data0 = (u32)load_u16(SHUF_XY_INDEX_7D + xy_pair0);
        const auto tbl_data1 = (u32)load_u16(SHUF_XY_INDEX_7D + xy_pair1);

        if ((v0 & (xy_pair0 << 8u)) == 0u) [[unlikely]] {

            return 0;
        }

        if ((v1 & (xy_pair1 << 8u)) == 0u) [[unlikely]] {

            return NO_INLINE_parse_object_7digit_SIMD_single(p0, out_object);
        }

        const u32 yc1 = u8(tbl_data1);
        const u32 yc0 = u8(tbl_data0);

        const u32 shuf_base1 = tbl_data1 >> 8;
        const u32 shuf_base0 = tbl_data0 >> 8;

        const u64* const tbl = (u64 const*)SHUF_TBL7;

        const auto shuf1 = _mm_load_si128((__m128i const*)(tbl + shuf_base1));
        const auto shuf0 = _mm_load_si128((__m128i const*)(tbl + shuf_base0));

        const auto shufed1 = _mm_shuffle_epi8(digits1, shuf1);
        const auto shufed0 = _mm_shuffle_epi8(digits0, shuf0);

        const auto pairs1 = _mm_maddubs_epi16(shufed1, _mm_setr_epi8(10, 1, 0, 1, 10, 1, 0, 1, 10, 1, 10, 1, 10, 1, 10, 1));
        const auto pairs0 = _mm_maddubs_epi16(shufed0, _mm_setr_epi8(10, 1, 0, 1, 10, 1, 0, 1, 10, 1, 10, 1, 10, 1, 10, 1));

        const auto result1 = _mm_madd_epi16(pairs1, _mm_setr_epi16(10, 1, 10, 1, 10, 1, 25000, 250));
        const auto result0 = _mm_madd_epi16(pairs0, _mm_setr_epi16(10, 1, 10, 1, 10, 1, 25000, 250));

        const auto time_fix1 = _mm_srli_epi64(result1, 30);
        const auto time_fix0 = _mm_srli_epi64(result0, 30);

        const auto time1 = _mm_add_epi32(result1, time_fix1);
        const auto time0 = _mm_add_epi32(result0, time_fix0);

        const auto result0_time1 = _mm_blend_epi16(result1, time1, 0x30);
        const auto result0_time0 = _mm_blend_epi16(result0, time0, 0x30);

        _mm_store_si128((__m128i*)(out_object + 1), result0_time1);
        _mm_store_si128((__m128i*)out_object, result0_time0);

        //*(u64*)(out_object + 1) = (u64)_mm_extract_epi64(result1, 0);
        //*(u64*)out_object = (u64)_mm_extract_epi64(result0, 0);

        const auto [type_size1, type1] { parse_integer_m3::likely_1(load_u32(p1 + yc1)) };

        (out_object+1)->type = type1;
        u32 consumed1 = (yc1 + type_size1) << 8;

        const auto [type_size0, type0] { parse_integer_m3::likely_1(load_u32(p0 + yc0)) };

        out_object->type = type0;

        return (yc0 + type_size0) | consumed1;
    }


}
