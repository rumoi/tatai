#pragma once

namespace parse_6_time {

	constexpr auto make_shuf6(u32 x_dig, u32 y_dig, u32 type_dig) {

		std::array<u8, 16> r{};

		const u32 x = 0;
		const u32 y = x_dig + 1;
		const u32 t = x_dig + y_dig + 2;
		const u32 ty = t + 7;

		r[0] = 0x80u;

		switch (x_dig) {
			case 1:r[1] = 0x80; r[2] = 0x80u; r[3] = u8(x); break;
			case 2: r[1] = 0x80u; r[2] = u8(x + 0); r[3] = u8(x + 1); break;
			case 3: r[1] = u8(x + 0); r[2] = u8(x + 1); r[3] = u8(x + 2); break;
		}

		switch (y_dig) {
			case 1: r[4] = 0x80u; r[5] = 0x80u; r[6] = 0x80u; r[7] = u8(y); break;
			case 2: r[4] = 0x80u; r[5] = u8(y + 0); r[6] = 0x80u; r[7] = u8(y + 1); break;
			case 3: r[4] = u8(y + 0); r[5] = u8(y + 1); r[6] = 0x80u; r[7] = u8(y + 2); break;
		}

		// time

		r[8] = u8(t + 0);
		r[9] = u8(t + 1);
		r[10] = u8(t + 2);
		r[11] = u8(t + 3);

		r[14] = u8(t + 4);
		r[15] = u8(t + 5);

		//type

		//if (type_dig >= 2u && ty + 1u < 16u) {
		//    r[12] = u8(ty);
		//    r[13] = u8(ty + 1u);
		//}
		//else {
		//    r[12] = 0x80u;
		//    r[13] = u8(ty);
		//}

		// purposely only take 1 digit of type if there is 3 and it fits within the SIMD bound
		// makes the fix code a lot cleaner

		if (type_dig == 2u && ty + 1u < 16u) {
			r[12] = u8(ty);
			r[13] = u8(ty + 1u);
		}
		else {
			r[12] = 0x80u;
			r[13] = u8(ty);
		}

		return r;
	}

	alignas(64) inline constexpr auto SHUF_TBL6 = [] {

		std::array<std::array<u8, 16>, 27> r{};

		u32 i{};

		for (u32 x_dig{ 1 }; x_dig < 4; ++x_dig) {
			for (u32 y_dig{ 1 }; y_dig < 4; ++y_dig) {
				for (u32 type_dig{ 1 }; type_dig < 4; ++type_dig) {

					r[i] = make_shuf6(
						x_dig,
						y_dig,
						type_dig);

					//if (r[i][0] != 0x80) {
					//    i = 0x1000; // something went very wrong, we dont want to compile this
					//}

					// unused for now - unlikely to ever be worth it
					//r[i][0] |= x_dig + y_dig + type_dig + 10u;

					++i;
				}
			}
		}

		return r;
	}();

	alignas(64) inline constexpr std::array<u8, 64 + 256> SHUF_XY_STORAGE2 = [] {

		std::array<u8, 64 + 256> t{};

		auto write = [&t](size_t i, u16 v) {
			t[i] = u8(v & 0xff);
			t[i + 1] = u8((v >> 8) & 0xff);
		};

		const auto pack = [](u32 shuf_base, u32 consumed) {
			return u16((shuf_base << 8) | consumed);
		};

		size_t base = 64 - 10;

		write(base + 0x0a, pack(0 << 1, 13));
		write(base + 0x12, pack(3 << 1, 14));
		write(base + 0x22, pack(6 << 1, 15));

		write(base + 0x14, pack(9 << 1, 14));
		write(base + 0x24, pack(12 << 1, 15));
		write(base + 0x44, pack(15 << 1, 16));

		write(base + 0x28, pack(18 << 1, 15));
		write(base + 0x48, pack(21 << 1, 16));
		write(base + 0x88, pack(24 << 1, 17));

		return t;
	}();

	inline constexpr u8 const* SHUF_XY_INDEX2{ SHUF_XY_STORAGE2.data() + 64 - 10 };

	__forceinline u32 parse_object_6digit_single(const char* __restrict p, _object_header* const __restrict out_object) {

		const auto m0 = _mm_loadu_si128((__m128i const*)p);

		const auto v = (u32)_mm_movemask_epi8(_mm_cmpeq_epi8(m0, _mm_set1_epi8(',')));

		const auto digits = _mm_sub_epi8(m0, _mm_set1_epi8('0'));

		const u32 xy_pair = (u8)v;

		const auto tbl_data = (u32)load_u16(SHUF_XY_INDEX2 + xy_pair);

		if ((v & (xy_pair << 7u)) == 0) [[unlikely]] {

			return 0;
		}

		u32 consumed = u8(tbl_data);
		u32 shuf_base = tbl_data >> 8u;

		// msvc baby sitting
		if (*(p + consumed - 1) != ',') [[unlikely]] {

			shuf_base += 2;

			if (p[consumed++] != ',') [[unlikely]] {

				shuf_base += 2;
				consumed += 0b10000001u; // can only parse up to 2 digits of type, time eats into the other lane

			}

		}

		const u64* const tbl = (u64 const*)SHUF_TBL6.data();
		const auto shuf = _mm_load_si128((__m128i const*)(tbl + shuf_base));

		const auto has_negative = (u32)_mm_movemask_epi8(digits);

		const auto inter0 = _mm_maddubs_epi16(_mm_shuffle_epi8(digits, shuf),
			_mm_setr_epi8(0, 1, 10, 1, 10, 1, 0, 1, 10, 1, 10, 1, 10, 1, 10, 1));

		const auto inter1 = _mm_madd_epi16(inter0, _mm_setr_epi16(100, 1, 10, 1, 100, 1, 1, 256));
		const auto inter2 = _mm_shuffle_epi8(inter1, _mm_setr_epi8(0, 1, -1, -1, 4, 5, -1, -1, 8, 9, 13, -1, 12, -1, -1, -1));

		auto result = _mm_madd_epi16(inter2, _mm_setr_epi16(1, 0, 1, 0, 100, 1, 1, 0));

		result = _mm_min_epu32(result, _mm_setr_epi32(512, 512, -1, -1));

		_mm_store_si128((__m128i*)out_object, result);

		if ((has_negative & ~v) != 0) [[unlikely]] {

			const auto y_start = (u32)_tzcnt_u32(xy_pair);

			if (p[0] == '-') out_object->x = 0;
			if (p[y_start + 1] == '-') out_object->y = 0;

		}

		if (consumed >= 18) [[unlikely]] {

			if (consumed == 18) [[likely]] {
				out_object->type = out_object->type * 10u + (u8(p[16]) & 0x0fu);
			} else {

				consumed &= 0b01111111u;

				const u16 d = load_u16(p + consumed - 3);

				out_object->type = out_object->type * 100u + (d & 0xfu) * 10u + ((d >> 8u) & 0xfu);

			}
		}

		if (consumed > 2) [[likely]] {
			return consumed;

		} else [[unlikely]] {

			push_error_object_header_list(p, out_object);

			return 1;
		}
	}

	__declspec(noinline) u32 NO_INLINE_parse_object_6digit_single(const char* __restrict p,
		_object_header* const __restrict out_object) {
	
		return parse_object_6digit_single(p, out_object);
	}

}