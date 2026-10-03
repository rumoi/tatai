#pragma once

enum header_id : u8 {
	LetterboxInBreaks = 0,
	Mode = 1,
	WidescreenStoryboard = 2,
	Title = 3,
	AudioFilename = 4,
	StackLeniency = 5,
	ArtistUnicode = 6,
	PreviewTime = 7,
	OverallDifficulty = 8,
	UseSkinSprites = 9,
	ApproachRate = 10,
	Version = 11,
	SampleSet = 12,
	OverlayPosition = 13,
	Artist = 14,
	Countdown = 15,
	Creator = 16,
	HPDrainRate = 17,
	BeatmapSetID = 18,
	CircleSize = 19,
	Source = 20,
	BeatmapID = 21,
	Tags = 22,
	SkinPreference = 23,
	SliderTickRate = 24,
	AudioLeadIn = 25,
	EpilepsyWarning = 26,
	TitleUnicode = 27,
	SamplesMatchPlaybackRate = 28,
	SpecialStyle = 29,
	SliderMultiplier = 31,
};

namespace header_key {

	alignas(64) inline constexpr auto HEADER_INFO = [] {

		const auto pack = [](char first, u32 skip) {
			return (u16(u8(first))) | u16(skip << 8);
		};

		std::array<u16, 32> t{};

		t[0] = pack('L', 19);

		t[2] = pack('W', 22);

		t[4] = pack('A', 15);
		t[5] = pack('S', 15);
		t[6] = pack('A', 14);
		t[7] = pack('P', 13);
		t[8] = pack('O', 18);
		t[9] = pack('U', 16);
		t[10] = pack('A', 13);
		t[11] = pack('V', 8);
		t[12] = pack('S', 11);
		t[13] = pack('O', 17);

		t[15] = pack('C', 11);
		t[16] = pack('C', 8);
		t[17] = pack('H', 12);
		t[18] = pack('B', 13);
		t[19] = pack('C', 11);

		t[21] = pack('B', 10);
		t[23] = pack('S', 16);
		t[24] = pack('S', 16);
		t[25] = pack('A', 13);
		t[26] = pack('E', 17);
		t[27] = pack('T', 13);
		t[28] = pack('S', 26);
		t[29] = pack('S', 14);
		t[31] = pack('S', 18);

		return t;
	}();


	const char** parse_headers_key_index(_memory_region_header* __restrict MEM,
		const char** __restrict start, const char** const __restrict end) {

		MEM->version_number = 0;

		const char* first_line{ *start++ };

		// skip anything before the format line, e.g. a UTF-8 BOM
		for (const char* const limit{ first_line + 16 }; *first_line != 'o' && first_line != limit; ++first_line) {}

		if (load_u64(first_line) == str_to_u64("osu file format v")) [[likely]] {

			MEM->version_number = parse_integer_m3::expect_2(load_u32(first_line + sizeof("osu file format v") - 1));

		}
		//else return end;

		ZeroMemory(&MEM->osu_header_table, sizeof(MEM->osu_header_table));

		for (; start != end; ++start) {

			const char* line_start = *start;

			const u64 key{ load_u64(line_start) };
			
			if (key == str_to_u64("[Events]")) {
				++start;
				goto skip_events;
			}

			if (key == str_to_u64("[TimingPoints]")) {
				++start;
				goto do_timing;
			}

			const char* line_end = (start + 1 == end) ? *start : *(start + 1);

			// underflows on the last line, but this piece of code should not be here at EOF anyway.
			const size_t line_size = (line_end - line_start) - 1;

			u32 index = u32((key * 0xa7c48ebd2da48d17ull) >> 59);

			const auto hi = HEADER_INFO[index];

			u32 skip = hi >> 8;

			// most invalidations is just empty new lines so u8 is enough, fits in one cache line with the skip info like this
			if (u8(hi) != u8(key)) {

				switch (u32(key)) {

					case str_to_u32("Mode"):
						index = 1; skip = 6;
						break;
					case str_to_u32("Title"):
						index = 3; skip = 6;
						break;
					case str_to_u32("Artist"):
						index = 14; skip = 7;
						break;
					case str_to_u32("Source"):
						index = 20; skip = 7;
						break;
					case str_to_u32("Tags"):
						index = 22; skip = 5;
						break;

				default:
					continue;
				}

			}

			MEM->osu_header_table[index] = {
				line_start + skip,
				line_size - skip
			};

		}

		skip_events: {
			for (; start != end; ++start) { // TODO: unroll?

				if (load_u64(*start) != str_to_u64("[TimingPoints]"))
					continue;

				++start;
				break;
			}
		}

		do_timing: {
		
			if (MEM->version_number < 8)
				start = parse_timing_points<1>(MEM, start, end);
			else
				start = parse_timing_points<0>(MEM, start, end);

		}

		return start;
	}
}