#pragma once

//namespace FIXED_SWAR{
//
//	template<size_t n>
//	__forceinline u32 raw_iter(u64 v) {
//		return[v]<size_t... i>(std::index_sequence<i...>) {
//			u32 r = 0;
//			((r = r * 10 + u32(u8(v >> (8 * i)))), ...);
//			return r;
//		}(std::make_index_sequence<n>{});
//	}
//
//	template<size_t digit_count>
//	__forceinline u32 parse_int(u64 v) {
//
//		static_assert(digit_count >= 1 && digit_count <= 8);
//
//		if constexpr (digit_count == 1) {
//
//			return u32(v) & 0x0f;
//
//		} else if constexpr (digit_count == 2) {
//
//			u32 d = u32(v) & 0x0f0f;
//
//			return ((d * 2561) >> 8) & 0xff;
//
//		} else if constexpr (digit_count == 3) {
//
//			u32 d = u32(v) & 0x0f0f0f;
//			u32 p = ((d * 2561) >> 8) & 0xff;
//
//			return p * 10 + (d >> 16);
//
//		} else if constexpr (digit_count == 4) {
//
//			u32 d = u32(v) & 0x0f0f0f0f;
//			d = (d * 2561) >> 8;
//
//			return ((d & 0x00ff00ff) * 6553601u) >> 16;
//
//		} else {
//
//			constexpr size_t dc = digit_count - 4;
//
//			return parse_int<dc>(v) * 10000u + parse_int<4>(v >> (8u * dc));
//		}
//	}
//
//}

template<size_t min_digit>
void parse_spinner(const char* p, _spinner_data* o) {

	{ // hitsound	

		if (EXPECT_PROB(p[1] == ',', 0.9994)) LIKELY_ARM {
			p += 2;

		} else {
			p += 3;

		}

	}	

	//if (EXPECT_PROB(u8(p[min_digit]) < u8('0'), 0.95)) LIKELY_ARM{	
	//	o->end_time = FIXED_SWAR::parse_int<min_digit>(load_u64(p));
	//	return;
	//}

	std::from_chars(p, p + 10, o->end_time);

}