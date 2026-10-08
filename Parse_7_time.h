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
	
	__forceinline u32 parse_object_7digit_single(const char* __restrict  p, _object_header* const __restrict out_object) {
	   
		const auto m0 = _mm_loadu_si128((__m128i const*)p);

		const auto v = (u32)_mm_movemask_epi8(_mm_cmpeq_epi8(m0, _mm_set1_epi8(',')));

		const auto digits = _mm_sub_epi8(m0, _mm_set1_epi8('0'));

		const u32 xy_pair = u8(v);

		const auto tbl_data = (u32)load_u16(SHUF_XY_INDEX_7D + xy_pair);

		if ((v & (xy_pair << 8u)) == 0u) [[unlikely]] {

			return 0;
		}

		const u32 yc = u8(tbl_data);

		if (yc == 0) [[unlikely]] {

			return 1;
		}

		const u32 shuf_base = tbl_data >> 8;

		const u64* const tbl = (u64 const*)SHUF_TBL7;
		const auto shuf = _mm_load_si128((__m128i const*)(tbl + shuf_base));

		const auto has_negative = (u32)_mm_movemask_epi8(digits);

		const auto pairs = _mm_maddubs_epi16(_mm_shuffle_epi8(digits, shuf),
			_mm_setr_epi8(10, 1, 0, 1, 10, 1, 0, 1, 10, 1, 10, 1, 10, 1, 10, 1));

		// weights are so we can represent it as t0 * 4 + t1
		// which can be simplified as a u64 output stream as
		// u32(t) + u32(t >> 30)

		auto result = _mm_madd_epi16(pairs, _mm_setr_epi16(10, 1,10, 1,10, 1, 25000, 250));

		result = _mm_min_epu32(result, _mm_setr_epi32(512, 512, -1, -1));
		
		const u64 t = (u64)_mm_extract_epi64(result, 1);

		*(u64*)out_object = (u64)_mm_extract_epi64(result, 0);
		
		out_object->time = u32(t) + u32(t >> 30);

		if ((has_negative & ~v) != 0) [[unlikely]] {

			const auto y_start = (u32)_tzcnt_u32(xy_pair);

			if (p[0] == '-') out_object->x = 0;
			if (p[y_start + 1] == '-') out_object->y = 0;

		}

		const auto [type_size, type] { parse_integer_m3::likely_1(load_u32(p + yc)) };

		out_object->type = type;

		return yc + type_size;
	}

}