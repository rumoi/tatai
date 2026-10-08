#pragma once

void parse_spinner(const char* p, _spinner_data* o) {

	{ // hitsound	

		if (EXPECT_PROB(p[1] == ',', 0.9994)) LIKELY_ARM {
			p += 2;

		} else {
			p += 3;

		}

	}

	std::from_chars(p, p + 10, o->end_time);

}