#pragma once

u32 general_parse_slider_points(const char* __restrict p,
	_slider_point* __restrict slider_ptr, _slider_data* const __restrict r) noexcept {

	//called around 390 times in the 94k map corpus	

	r->point_start = slider_ptr;

	std::string_view values[3]{};

	u32 line_size{};

	while(p[line_size] != '\n' && p[line_size] != 0)
		++line_size;

	size_t start_i{}, count{};

	for (size_t i{}; i < line_size; ++i) {

		if (i != (line_size-1) && p[i] != ',')
			continue;

		const bool is_last = (i == line_size - 1) && (p[i] != '\r');

		values[count] = std::string_view(p + start_i, i - start_i + is_last);
		start_i = i+1;

		if (++count == 3)
			break;

	}

	for (const auto& v : values)
		if (v.size() == 0) {
			return 0;
		}

	{

		ON_SCOPE_EXIT(r->point_end = slider_ptr;);

		const auto& POINTS = values[0];

		size_t x_start{}, y_start{};

		for (size_t i{}, size{ POINTS.size() }; i < size; ++i) {

			if (POINTS[i] == '|' || i == (size-1)) [[unlikely]] {

				if (x_start == y_start) {
					return 0;
				}

				const size_t point_end = i + (u8(u8(POINTS[i]) - u8('0')) <= 9);

				const std::string_view
					x{POINTS.data() + x_start, y_start - x_start},
					y{POINTS.data() + y_start + 1, point_end - (y_start + 1) };

				if (x.size() == 0 || y.size() == 0) {
					return 0;
				}					

				{

					const char* first = x.data();
					const char* last = first + x.size();

					const auto [ptr, ec] = std::from_chars(first, last, slider_ptr->x);

					if(ec != std::errc{} || ptr != last) {
						return 0;
					}

				}
				
				{

					const char* first = y.data();
					const char* last = first + y.size();

					const auto [ptr, ec] = std::from_chars(first, last, slider_ptr->y);

					if (ec != std::errc{} || ptr != last) {
						return 0;
					}

				}

				++slider_ptr;

				y_start = (x_start = i+1);				
				continue;
			}

			if (POINTS[i] == ':') {
				y_start = i;
			}

		}

	}

	{

		const char* first = values[1].data();
		const char* last = first + values[1].size();

		const auto [ptr, ec] = std::from_chars(first, last, r->slides);

		if (ec != std::errc{} || ptr != last || r->slides > 9000) {

			((_memory_region_new*)(size_t(r) & POINTER_RESET_MASK))->header.stable_would_refuse = 1;

			return 0;
		}

	}


	{

		//if (values[2].back() == '\r')
		//	values[2].remove_suffix(1);

		const char* first = values[2].data();
		const char* last = first + values[2].size();

		const auto [ptr, ec] = std::from_chars(first, last, r->length);

		if (ec != std::errc{} || ptr != last) {
			return 0;
		}

	}

	return 1;
}