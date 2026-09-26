#pragma once

namespace parse_5_time {

    alignas(64) inline constexpr std::array<u8, 16> SHUF_TBL5[27] {
        { 0x80,0x80,0x80,0x00,0x80,0x80,0x80,0x02,0x04,0x05,0x06,0x07,0x80,0x0a,0x80,0x08 },//x
        { 0x80,0x80,0x80,0x00,0x80,0x80,0x80,0x02,0x04,0x05,0x06,0x07,0x0a,0x0b,0x80,0x08 },
        { 0x80,0x80,0x80,0x00,0x80,0x80,0x80,0x02,0x04,0x05,0x06,0x07,0x0a,0x0b,0x80,0x08 },
        { 0x80,0x80,0x80,0x00,0x80,0x02,0x80,0x03,0x05,0x06,0x07,0x08,0x80,0x0b,0x80,0x09 },//x
        { 0x80,0x80,0x80,0x00,0x80,0x02,0x80,0x03,0x05,0x06,0x07,0x08,0x0b,0x0c,0x80,0x09 },
        { 0x80,0x80,0x80,0x00,0x80,0x02,0x80,0x03,0x05,0x06,0x07,0x08,0x0b,0x0c,0x80,0x09 },
        { 0x80,0x80,0x80,0x00,0x02,0x03,0x80,0x04,0x06,0x07,0x08,0x09,0x80,0x0c,0x80,0x0a },//x
        { 0x80,0x80,0x80,0x00,0x02,0x03,0x80,0x04,0x06,0x07,0x08,0x09,0x0c,0x0d,0x80,0x0a },
        { 0x80,0x80,0x80,0x00,0x02,0x03,0x80,0x04,0x06,0x07,0x08,0x09,0x0c,0x0d,0x80,0x0a },
        { 0x80,0x00,0x80,0x01,0x80,0x80,0x80,0x03,0x05,0x06,0x07,0x08,0x80,0x0b,0x80,0x09 },//x
        { 0x80,0x00,0x80,0x01,0x80,0x80,0x80,0x03,0x05,0x06,0x07,0x08,0x0b,0x0c,0x80,0x09 },
        { 0x80,0x00,0x80,0x01,0x80,0x80,0x80,0x03,0x05,0x06,0x07,0x08,0x0b,0x0c,0x80,0x09 },
        { 0x80,0x00,0x80,0x01,0x80,0x03,0x80,0x04,0x06,0x07,0x08,0x09,0x80,0x0c,0x80,0x0a },//x
        { 0x80,0x00,0x80,0x01,0x80,0x03,0x80,0x04,0x06,0x07,0x08,0x09,0x0c,0x0d,0x80,0x0a },
        { 0x80,0x00,0x80,0x01,0x80,0x03,0x80,0x04,0x06,0x07,0x08,0x09,0x0c,0x0d,0x80,0x0a },
        { 0x80,0x00,0x80,0x01,0x03,0x04,0x80,0x05,0x07,0x08,0x09,0x0a,0x80,0x0d,0x80,0x0b },//x
        { 0x80,0x00,0x80,0x01,0x03,0x04,0x80,0x05,0x07,0x08,0x09,0x0a,0x0d,0x0e,0x80,0x0b },
        { 0x80,0x00,0x80,0x01,0x03,0x04,0x80,0x05,0x07,0x08,0x09,0x0a,0x0d,0x0e,0x80,0x0b },
        { 0x00,0x01,0x80,0x02,0x80,0x80,0x80,0x04,0x06,0x07,0x08,0x09,0x80,0x0c,0x80,0x0a },//x
        { 0x00,0x01,0x80,0x02,0x80,0x80,0x80,0x04,0x06,0x07,0x08,0x09,0x0c,0x0d,0x80,0x0a },
        { 0x00,0x01,0x80,0x02,0x80,0x80,0x80,0x04,0x06,0x07,0x08,0x09,0x0c,0x0d,0x80,0x0a },
        { 0x00,0x01,0x80,0x02,0x80,0x04,0x80,0x05,0x07,0x08,0x09,0x0a,0x80,0x0d,0x80,0x0b },//x
        { 0x00,0x01,0x80,0x02,0x80,0x04,0x80,0x05,0x07,0x08,0x09,0x0a,0x0d,0x0e,0x80,0x0b },
        { 0x00,0x01,0x80,0x02,0x80,0x04,0x80,0x05,0x07,0x08,0x09,0x0a,0x0d,0x0e,0x80,0x0b },
        { 0x00,0x01,0x80,0x02,0x04,0x05,0x80,0x06,0x08,0x09,0x0a,0x0b,0x80,0x0e,0x80,0x0c },//x
        { 0x00,0x01,0x80,0x02,0x04,0x05,0x80,0x06,0x08,0x09,0x0a,0x0b,0x0e,0x0f,0x80,0x0c },
        { 0x00,0x01,0x80,0x02,0x04,0x05,0x80,0x06,0x08,0x09,0x0a,0x0b,0x0e,0x0f,0x80,0x0c }//x
    };

    alignas(64) inline constexpr std::array<u8, 64 + 256> SHUF_XY_STORAGE_5D = [] {

        std::array<u8, 64 + 256> t{};

        auto write = [&t](size_t i, u16 v) {
            t[i] = u8(v & 0xff);
            t[i + 1] = u8((v >> 8) & 0xff);
        };

        const auto pack = [](u32 shuf_base, u32 consumed) {
            return u16((shuf_base << 8) | consumed);
        };

        size_t base = 64 - 10;

        write(base + 0x0a, pack(0 << 1, 13 - 1));
        write(base + 0x12, pack(3 << 1, 14 - 1));
        write(base + 0x22, pack(6 << 1, 15 - 1));

        write(base + 0x14, pack(9 << 1, 14 - 1));
        write(base + 0x24, pack(12 << 1, 15 - 1));
        write(base + 0x44, pack(15 << 1, 16 - 1));

        write(base + 0x28, pack(18 << 1, 15 - 1));
        write(base + 0x48, pack(21 << 1, 16 - 1));
        write(base + 0x88, pack(24 << 1, 17 - 1));

        return t;
    }();

    inline constexpr u8 const* SHUF_XY_INDEX_5D{ SHUF_XY_STORAGE_5D.data() + 64 - 10 };

    TATAI_FORCE_INLINE u32 parse_object_5digit_single(const char* __restrict p, _object_header* const __restrict out_object) {

        const auto m0 = _mm_loadu_si128((__m128i const*)p);

        const auto v = (u32)_mm_movemask_epi8(_mm_cmpeq_epi8(m0, _mm_set1_epi8(',')));

        const u32 xy_pair = u8(v);

        //const u32 shuf_index = ((v * 27007u) >> 14u) & 127u;

        auto tbl_data = (u32)load_u16(SHUF_XY_INDEX_5D + xy_pair);// 

        if ((v & (xy_pair << 6u)) == 0) [[unlikely]] {

            return 0;
        }

        const auto digits = _mm_sub_epi8(m0, _mm_set1_epi8('0'));

        u32 consumed = u8(tbl_data);
        u32 shuf_base = tbl_data >> 8u;

        if (((v >> 8u) & (xy_pair - 1u)) == 0) [[unlikely]] {

            shuf_base += 2;

            if (p[consumed++] != ',') [[unlikely]] {

                shuf_base += 2;
                // can only parse up to 2 digits of type, time eats into the other lane
                consumed += 0b10000001u;// sets the flag to consume the 3rd digit                

            }
        
        }

        const u64* const tbl = (u64 const*)SHUF_TBL5;
        const auto shuf = _mm_load_si128((__m128i const*)(tbl + shuf_base));

        const auto pack = _mm_shuffle_epi8(digits, shuf);

        const auto inter0 = _mm_maddubs_epi16(pack, _mm_setr_epi8(10, 1, 0, 1, 10, 1, 0, 1, 10, 1, 10, 1, 10, 1, 0, 1));

        const auto inter1 = _mm_madd_epi16(inter0, _mm_setr_epi16(10, 1, 10, 1, 100, 1, 1, 256));

        const auto inter2 = _mm_shuffle_epi8(inter1, _mm_setr_epi8(0, 1, -1, -1, 4, 5, -1, -1, 8, 9, 13, -1, 12, -1, -1, -1));

        const auto result = _mm_madd_epi16(inter2, _mm_setr_epi16(1, 0, 1, 0, 10, 1, 1, 0));

        _mm_store_si128((__m128i*)(out_object), result);

        if (consumed > 17) [[unlikely]] {

            consumed &= 0b01111111u;
            out_object->type = out_object->type * 10u + (u8(p[consumed - 2]) & 0x0fu);

        }

        return consumed;
    }

    TATAI_NO_INLINE u32 NO_INLINE_parse_object_5digit_single(const char* __restrict p, _object_header* const __restrict out_object) {
        return parse_object_5digit_single(p, out_object);
    }

    TATAI_FORCE_INLINE u32 parse_object_5digit_pair(const char* __restrict p0, const char* __restrict p1,
        _object_header* const __restrict out_object) {


        const auto m0 = _mm_loadu_si128((__m128i const*)p0);
        const auto m1 = _mm_loadu_si128((__m128i const*)p0);

        const auto CMP0 = _mm_cmpeq_epi8(m0, _mm_set1_epi8(','));
        const auto CMP1 = _mm_cmpeq_epi8(m1, _mm_set1_epi8(','));

        const auto v0 = (u32)_mm_movemask_epi8(CMP0);
        const auto v1 = (u32)_mm_movemask_epi8(CMP1);

        const u32 xy_pair0 = u8(v0);
        const u32 xy_pair1 = u8(v1);

        auto tbl_data0 = (u32)load_u16(SHUF_XY_INDEX_5D + xy_pair0);
        auto tbl_data1 = (u32)load_u16(SHUF_XY_INDEX_5D + xy_pair1);

        if ((v0 & (xy_pair0 << 6u)) == 0) [[unlikely]] {
            return 0;
        }

        if ((v1 & (xy_pair1 << 6u)) == 0) [[unlikely]] {
            return NO_INLINE_parse_object_5digit_single(p0, out_object);
        }

        const auto digits0 = _mm_sub_epi8(m0, _mm_set1_epi8('0'));
        const auto digits1 = _mm_sub_epi8(m1, _mm_set1_epi8('0'));

        u32 consumed0 = u8(tbl_data0);
        u32 consumed1 = u8(tbl_data1);

        u32 shuf_base0 = tbl_data0 >> 8u;
        u32 shuf_base1 = tbl_data1 >> 8u;

        if (((v0 >> 8u) & (xy_pair0 - 1u)) == 0) [[unlikely]] {

            shuf_base0 += 2;

            if (p0[consumed0++] != ',') [[unlikely]] {

                shuf_base0 += 2;
                consumed0 += 0b10000001u;

            }

        }

        if (((v1 >> 8u) & (xy_pair1 - 1u)) == 0) [[unlikely]] {

            shuf_base1 += 2;

            if (p1[consumed1++] != ',') [[unlikely]] {

                shuf_base1 += 2;
                consumed1 += 0b10000001u;

            }

        }

        const u64* const tbl = (u64 const*)SHUF_TBL5;
        const auto shuf0 = _mm_load_si128((__m128i const*)(tbl + shuf_base0));
        const auto shuf1 = _mm_load_si128((__m128i const*)(tbl + shuf_base1));

        const auto pack0 = _mm_shuffle_epi8(digits0, shuf0);
        const auto pack1 = _mm_shuffle_epi8(digits1, shuf1);

        const auto inter0 = _mm_maddubs_epi16(pack0, _mm_setr_epi8(10, 1, 0, 1, 10, 1, 0, 1, 10, 1, 10, 1, 10, 1, 0, 1));
        const auto inter1 = _mm_maddubs_epi16(pack1, _mm_setr_epi8(10, 1, 0, 1, 10, 1, 0, 1, 10, 1, 10, 1, 10, 1, 0, 1));

        const auto inter1_0 = _mm_madd_epi16(inter0, _mm_setr_epi16(10, 1, 10, 1, 100, 1, 1, 256));
        const auto inter1_1 = _mm_madd_epi16(inter1, _mm_setr_epi16(10, 1, 10, 1, 100, 1, 1, 256));

        const auto inter2_0 = _mm_shuffle_epi8(inter1_0, _mm_setr_epi8(0, 1, -1, -1, 4, 5, -1, -1, 8, 9, 13, -1, 12, -1, -1, -1));
        const auto inter2_1 = _mm_shuffle_epi8(inter1_1, _mm_setr_epi8(0, 1, -1, -1, 4, 5, -1, -1, 8, 9, 13, -1, 12, -1, -1, -1));

        const auto result0 = _mm_madd_epi16(inter2_0, _mm_setr_epi16(1, 0, 1, 0, 10, 1, 1, 0));
        const auto result1 = _mm_madd_epi16(inter2_1, _mm_setr_epi16(1, 0, 1, 0, 10, 1, 1, 0));

        _mm_store_si128((__m128i*)(out_object), result0);
        _mm_store_si128((__m128i*)(out_object+1), result1);

        if (consumed0 > 17) [[unlikely]] {

            consumed0 &= 0b01111111u;
            out_object->type = out_object->type * 10u + (u8(p0[consumed0 - 2]) & 0x0fu);

        }
        if (consumed1 > 17) [[unlikely]] {

            consumed1 &= 0b01111111u;
            (out_object+1)->type = (out_object+1)->type * 10u + (u8(p1[consumed1 - 2]) & 0x0fu);

        }


        return consumed0 | (consumed1 << 8);


    }

}