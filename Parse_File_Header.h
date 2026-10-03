#pragma once

consteval u64 str_to_u64(std::string_view s) {

	if (s.size() < 8)
		return *(u64*)0;	

	u64 ret{};

	for (size_t i{}; i < 8; ++i)
		ret |= u64(s[i]) << (i * 8);

	return ret;
}

consteval u32 str_to_u32(std::string_view s) {

	if (s.size() < 4)
		return *(u32*)0;

	u32 ret{};

	for (size_t i{}; i < 4; ++i)
		ret |= u32(s[i]) << (i * 8);

	return ret;
}


__forceinline u32 parse_ascii_SWAR(u64 x, u32 digits) {

	x = _shlx_u64(x, (8u - digits) * 8u);

	x &= 0x0f0f0f0f0f0f0f0full;

	x = ((x * 0x0a01ull) >> 8) & 0x00ff00ff00ff00ffull;

	x = ((x * 0x00640001ull) >> 16) & 0x0000ffff0000ffffull;

	x = (x * 0x0000271000000001ull) >> 32;

	return (u32)x;
}

template<bool is_under_v8>
const char** parse_timing_points(_memory_region_header* __restrict MEM,
	const char** __restrict start, const char** const __restrict end) {

	_timing_point* timing_point{ MEM->get_timing_point()};

	double last_anchor{ 0. };
	double last_values[2]{ -1.,-1. };

	u32 digit_count{ 8 };
	u64 last_value = 0;

	for (; start != end; ++start) {

		const auto* line_start = *start;

		const auto line64 = load_u64(line_start);

		u64 check;
		u32 time;

		{
			const u8 is_first_digit = u8(*line_start) - u8('0');

			if (is_first_digit > 9u) {

				if (line64 == str_to_u64("[HitObjects]"))
					break;

				if (u8(line64) != '-')
					continue;

				// negative times only lead a section, so find the comma directly and leave digit_count to the positive times
				line_start += _tzcnt_u32((u32)_mm_movemask_epi8(_mm_cmpeq_epi8(_mm_loadu_si128((__m128i const*)line_start), _mm_set1_epi8(',')))) + 1;

				check = load_u64(line_start);

				if (last_value == check)
					continue;

				// negative times are stored as 0
				time = 0;

				goto parse_beat_length;
			}

		}

		{
			while (digit_count < 64 && ((u8)(line64 >> digit_count) != ','))
				digit_count += 8;

			const auto digit_actual{ digit_count >> 3 };

			line_start += digit_actual + 1;

			// the key starts at the sign so an inherited and an uninherited line never compare equal
			check = load_u64(line_start);

			if (last_value == check)
				continue;

			time = parse_ascii_SWAR(line64, digit_actual);
		}

	parse_beat_length:

		last_value = check;

		const bool is_inherited = (*line_start == '-');

		line_start += is_inherited;

		timing_point->time = time;

		const auto f = u32(check >> (is_inherited * 8)) == str_to_u32("100,") ? 100. : parse_double::from_ascii::parse_decimal_16(line_start);

		if (is_inherited) {

			timing_point->beat_length = last_anchor * (0.01 * f);

		} else {

			timing_point->beat_length = f;

			last_anchor = f;

		}

		// an uninherited line is its own anchor, so v8+ ticks always follow last_anchor
		if constexpr (is_under_v8) {

			timing_point->tick_beat_length = timing_point->beat_length;

		} else {

			timing_point->tick_beat_length = last_anchor;

		}

		const u32 is_different = (last_values[0] != timing_point->beat_length || last_values[1] != timing_point->tick_beat_length);

		last_values[0] = timing_point->beat_length;
		last_values[1] = timing_point->tick_beat_length;

		timing_point += is_different;

	}

	MEM->ELEM_COUNT[MEM_timing_point] = timing_point - MEM->get_timing_point();

	return start;
}

#include "Header_Key_System.h"

const char** parse_beatmap_header(_memory_region_header*__restrict MEM,
	const char**__restrict start, const char** const __restrict end) {

	if (start == end) [[unlikely]]
		return end;

	return header_key::parse_headers_key_index(MEM, start, end);

	ZeroMemory(&MEM->osu_header_table, sizeof(MEM->osu_header_table));

	if (const auto s{ *start }; load_u64(s) == str_to_u64("osu file format v")) [[likely]] {

		MEM->version_number = parse_integer_m3::expect_2(load_u32(s + sizeof("osu file format v") - 1));

	}

	#define DO_SPACE(x) case str_to_u64(#x": "): \
			MEM->osu_header_table[header_id::x] = std::string_view(line_start + sizeof(#x ": ") - 1, line_size - (sizeof(#x ": ") - 1));\
			break;

	#define DO_SPACE_32(x) case str_to_u32(#x": "): \
			MEM->osu_header_table[header_id::x] = std::string_view(line_start + sizeof(#x ": ") - 1, line_size - (sizeof(#x ": ") - 1));\
			break;

	#define DO(x) case str_to_u64(#x":"): \
			MEM->osu_header_table[header_id::x] = std::string_view(line_start + sizeof(#x ":") - 1, line_size - (sizeof(#x ":") - 1));\
			break;

	#define DO_32(x) case str_to_u32(#x":"): \
			MEM->osu_header_table[header_id::x] = std::string_view(line_start + sizeof(#x ":") - 1, line_size - (sizeof(#x ":") - 1));\
			break;

	for (; start != end; ++start) {

		const auto* line_start = *start;
		const auto* line_end = (start + 1 == end) ? *start : (*(start + 1));

		size_t line_size = (line_end - line_start) - 1;
		const auto line_load = load_u64(*start);

		switch (line_load) {

			case str_to_u64("[Events]"): ++start; goto skip_events; break;
			case str_to_u64("[TimingPoints]"): ++start; goto do_timing; break;

			DO(HPDrainRate);
			DO(CircleSize);
			DO(OverallDifficulty);
			DO(ApproachRate);
			DO_SPACE(SliderMultiplier);
			DO_SPACE(SliderTickRate);

			DO_SPACE(AudioFilename);
			DO_SPACE(AudioLeadIn);
			DO_SPACE(PreviewTime); 
			DO_SPACE(Countdown);
			//DO_SPACE(CountdownOffset);
			DO_SPACE(SampleSet);
			DO_SPACE(StackLeniency);
			DO_SPACE(LetterboxInBreaks);
			DO_SPACE(UseSkinSprites);
			DO_SPACE(OverlayPosition);
			DO_SPACE(SkinPreference);
			DO_SPACE(EpilepsyWarning);
			DO_SPACE(SpecialStyle);
			DO_SPACE(WidescreenStoryboard);
			DO_SPACE(SamplesMatchPlaybackRate);

			DO(TitleUnicode);
			DO(ArtistUnicode);
			DO(Creator);
			DO(Version);
			DO(BeatmapID);
			DO(BeatmapSetID);

			default: {

				switch (u32(line_load)) {

					DO_SPACE_32(Mode);

					DO_32(Title)
					DO_32(Artist);
					DO_32(Source);
					DO_32(Tags);

					default:break;
				}

				break;
			}
			
		}

	}

	#undef DO_SPACE
	#undef DO_SPACE_32
	#undef DO
	#undef DO_32

	skip_events:

	for (; start != end; ++start) { // TODO: unroll?

		if (load_u64(*start) != str_to_u64("[TimingPoints]"))
			continue;

		++start;
		break;
	}

do_timing:

	if(MEM->version_number < 8)
		start = parse_timing_points<0>(MEM, start, end);
	else 
		start = parse_timing_points<1>(MEM, start, end);

	return start;
}