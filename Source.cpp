#include <vector>
#include <tuple>
#include <array>
#include <fstream>
#include <type_traits>
#include <bit>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <utility>

#include <immintrin.h>
#if defined(_MSC_VER)
#include <intrin.h>
#define TATAI_FORCE_INLINE __forceinline
#define TATAI_NO_INLINE __declspec(noinline)
#define TATAI_ASSUME(condition) __assume(condition)
#else
#define TATAI_FORCE_INLINE inline __attribute__((always_inline))
#define TATAI_NO_INLINE __attribute__((noinline))
#define TATAI_ASSUME(condition) __builtin_assume(condition)
#endif

#include <chrono>

#if defined(__clang__)
    constexpr bool is_clang{true};
#else
    constexpr bool is_clang{ false };
#endif


struct _Timer {

    const char* name;
    const std::chrono::steady_clock::time_point sTime;

    _Timer() : sTime(std::chrono::steady_clock::now()), name{} {};
    _Timer(const char* name) : sTime(std::chrono::steady_clock::now()), name{ name } {};

    float get_time() {

        return double(double((unsigned long long)(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - sTime).count())) / 1000.0);
    }

    ~_Timer() {

        const auto v = double(double((unsigned long long)(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - sTime).count())) / (1000.0));

        if (name)
            printf("%s> %f\xE6s\n", name, v);
        else printf("\n%.2f\xE6s\n", v);

    }
};


template <typename F> struct on_scope_exit {
private: F func;
public:
    on_scope_exit(const on_scope_exit&) = delete;
    on_scope_exit& operator=(const on_scope_exit&) = delete;
    on_scope_exit(F&& f) : func(std::forward<F>(f)) {}
    ~on_scope_exit() { func(); }
};

#define PCAT0(x, y) PCAT1(x, y)
#define PCAT1(x, y) PCAT2(!, x ## y)
#define PCAT2(x, r) r
#define ON_SCOPE_EXIT(...) on_scope_exit PCAT0(__scope, __LINE__) {[&]{__VA_ARGS__}}

typedef int_fast8_t fu8;
typedef int_fast16_t fu16;
typedef int_fast32_t fu32;
typedef int_fast64_t fu64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

constexpr u32 pext_constexpr(u32 v, u32 m) noexcept {

    u32 r{}, o{ 1 };

    while (m) {
        const u32 bit = m & (0 - m);
        if (v & bit) r |= o;
        m &= m - 1;
        o <<= 1;
    }

    return r;
}

std::vector<u8> read_file(const char* file_name) {

    std::ifstream file(file_name, std::ios::binary | std::ios::ate | std::ios::in);

    if (file.is_open() == 0) [[unlikely]]
        return {};

    const size_t file_size{ (size_t)file.tellg() };

    std::vector<u8> ret(file_size);

    if (file_size) [[likely]] {
        file.seekg(0, std::ios::beg);
        file.read((char*)ret.data(), file_size);
    }
    file.close();

    return ret;
}

struct slider_point {
    int x;
    int y;
};

struct alignas(16) _object_header {
    u32 x, y;
    u32 time;
    u32 type; // hit sound data will be blitted here afterwards
};

struct _spinner_data {
    u32 end_time;
};

struct _slider_data {

    /*const*/ slider_point* point_start, * point_end;
    double length;
    u32 slides;
    u32 curve_type;

};

struct _object_data {

    union {

        _slider_data slider;
        _spinner_data spinner;

    };

}; static_assert(sizeof(_object_data) == 32);


struct _slider_deferral {
    const char* p;
    _slider_data* out;
};


int total_count_opt{};

[[nodiscard]] TATAI_FORCE_INLINE u64 load_u64(const void* p) noexcept {
    u64 v;
    std::memcpy(&v, p, sizeof(v));
    return v;
}

[[nodiscard]] TATAI_FORCE_INLINE u32 load_u32(const void* p) noexcept {
    u32 v;
    std::memcpy(&v, p, sizeof(v));
    return v;
}

[[nodiscard]] TATAI_FORCE_INLINE u16 load_u16(const void* p) noexcept {
    u16 v;
    std::memcpy(&v, p, sizeof(v));
    return v;
}
[[nodiscard]] TATAI_FORCE_INLINE u8 load_u8(const void* p) noexcept {
    u8 v;
    std::memcpy(&v, p, sizeof(v));
    return v;
}

namespace parse_integer_m3 {

    TATAI_FORCE_INLINE u32 fixed_likely_1(u32 x, u32 digits) {

        const u32 d0 = x & 0xf;

        if (digits == 1)
            return d0;

        const u32 d1 = (x >> 8) & 0xf;

        if (digits == 2)
            return d0 * 10 + d1;

        const u32 d2 = x >> 16;
        return d0 * 100 + d1 * 10 + d2;
    }

    TATAI_FORCE_INLINE u32 fixed_likely_3(u32 x, u32 digits) {

        x &= 0x0f0f0f0f;

        const u32 d0 = u8(x);
        const u32 d1 = u8(x >> 8);

        if (digits == 3) {
            const u32 d2 = u8(x >> 16);
            return d0 * 100 + d1 * 10 + d2;
        }

        if (digits == 2)
            return d0 * 10 + d1;

        return d0;
    }

    template<size_t SHIFT = 0>
    TATAI_FORCE_INLINE std::tuple<u32, u32> likely_1(const u32 x) {

        const u32 d0 = u8(x) & 0x0fu;

        if (u8(x >> 8) == ',') [[likely]]
            return { 2 << SHIFT, d0 };

        const u32 masked = x & 0x000f0f0fu;

        //const u32 d1 = u8(masked >> 8);
        //const u32 d2 = u8(masked >> 16);

        if ((masked >> 16) > 9) [[likely]]
            return { 3 << SHIFT, d0 * 10u + u8(masked >> 8) };

        return {
            4 << SHIFT,
            d0 * 100u + u8(masked >> 8) * 10u + u8(masked >> 16)
        };
    }


    //__forceinline std::tuple<u32, u32> likely_3(u32 x) {
    //    constexpr u32 BOTH = 0x00101000;
    //
    //    const u32 digit_bits = x & BOTH;
    //
    //    x &= 0x0f0f0f0fu;
    //
    //    const u32 d0 = u8(x);
    //    const u32 d1 = u8(x >> 8);
    //
    //    if (digit_bits == BOTH) {
    //        const u32 d2 = u8(x >> 16);
    //
    //        return {
    //            4,
    //            d0 * 100 + d1 * 10 + d2
    //        };
    //    }
    //
    //    if (digit_bits & 0x00001000) {
    //        return {
    //            3,
    //            d0 * 10 + d1
    //        };
    //    }
    //
    //    return { 2, d0 };
    //}


}


namespace parse_integer_m2 {


    TATAI_FORCE_INLINE u64 likely_1(u64 x) {

        const u64 d0 = u8(x) & 0x0fu;

        if (u8(x >> 8u) == ',')
            return (2ull << 32u) | d0;

        const u64 d1 = u8(x >> 8u) & 0x0f;

        return (3ull << 32u) | (d0 * 10 + d1);
    }


}

size_t is_valid_slider_type(u32 v) {
    constexpr u32 table =
        (1u << ('B' & 31)) |
        (1u << ('C' & 31)) |
        (1u << ('L' & 31)) |
        (1u << ('P' & 31));
    return (table >> (u32(v) & 31)) & 1;
}

#include "Parse_7_time.h"
#include "Parse_6_time.h"
#include "Parse_5_time.h"
#include "Parse_4_time.h"

#include "Parse_Slider.h"
#include "Parse_Spinner.h"

#include <thread>
#include <chrono>

struct _memory_region {

    u8* object_header_data;
    u8* object_body_data;
    const char** lines;


    _slider_deferral* slider_defer_table;

    slider_point* SLIDER_PATHS;

    u32 note_count;

    void init_memory() {

        constexpr u32 MAX_NOTES{ u16(-1) };
        constexpr u32 NOTE_DATA_OFFSET{ (sizeof(_object_header) * MAX_NOTES) };

        object_header_data = new u8[
            NOTE_DATA_OFFSET +
                (sizeof(_object_data) * MAX_NOTES)
        ];

        object_body_data = object_header_data + NOTE_DATA_OFFSET;
        lines = new const char* [MAX_NOTES];

        SLIDER_PATHS = new slider_point[MAX_NOTES];

        slider_defer_table = new _slider_deferral[MAX_NOTES];

    }

    void print_map_data() {

        _object_header* o{ (_object_header*)object_header_data };
        _slider_data* s{ (_slider_data*)object_body_data };

        for (size_t i{}; i < note_count; ++i) {


            printf("%i> %i,%i | %i  ", o[i].time, o[i].x, o[i].y, o[i].type);

            if (o[i].type & 2) {

                printf("\n  %i %f  ", s[i].slides, s[i].length);
                const auto* p{ s[i].point_start };
                for (; p != s[i].point_end; ++p) {
                    printf("%i:%i|", p->x, p->y);
                }

            }

            printf("\n");

        }

        printf("NOTE_COUNT: %i\n", note_count);

    }


};

template <auto parse_func>
TATAI_NO_INLINE u64 parse_object_loop(
    const char* const* __restrict pos,
    _object_header* __restrict object,
    _slider_data* __restrict object_data,
    slider_point* __restrict slider_data_out
) {

    const auto* start = pos;
    const auto* start_path = slider_data_out;

    for (;;) {

        const char* p = *pos;

        if (p == nullptr)
            break;

        const auto con = parse_func(p, object);

        if (con == 0)
            break;

        if (object->type & 2) {

            parse_slider_body(p + con, slider_data_out, object_data);
            slider_data_out = object_data->point_end;

        }

        ++pos;
        ++object;
        ++object_data;
    }

    return (pos - start) | ((slider_data_out - start_path) << 32);
}

template <auto parse_func>
TATAI_NO_INLINE u64 PAIR_parse_object_loop(
    const char* const* __restrict pos,
    _object_header* __restrict object,
    _slider_data* __restrict object_data,
    slider_point * __restrict slider_data_out
) {

    const auto* start = pos;
    const auto* start_path = slider_data_out;


    for (;;) {

        const char* p = *pos;

        if (p == nullptr)
            break;

        const char* p1 = *(pos + 1);

        if (p1 == nullptr) {
            break;
        }

        const auto t = parse_func(p, p1, object);
        //const auto t = parse_7_time::parse_object_7digit_SIMD_pair(p, p1, object);

        auto con0 = u8(t);

        if (con0 == 0) [[unlikely]]
            break;

        if (object->type & 2) {
            parse_slider_body(p + con0, slider_data_out, object_data);
            slider_data_out = object_data->point_end;
        }

        auto con1 = t >> 8;

        if (con1 == 0) [[unlikely]] {
            pos += 1;
            object += 1;
            object_data += 1;
            break;
        }

        if ((object + 1)->type & 2) {
            parse_slider_body(p1 + con1, slider_data_out, object_data + 1);
            slider_data_out = (object_data + 1)->point_end;
        }

        pos += 2;
        object += 2;
        object_data += 2;

    }

    return (pos - start) | ((slider_data_out - start_path) << 32);
}

void parse_beatmap_from_memory(_memory_region* __restrict MEM, char const* __restrict p, char const* __restrict end) {

    if (MEM == 0)
        return;

    MEM->note_count = 0;

    if (MEM->object_body_data == nullptr)
        MEM->init_memory();


    // get the alignment in the raw data, can be done because of the [HitObjects] line being there, remove the u here once done

    size_t pending{};

    const auto nl = _mm256_set1_epi8('\n');

    _object_header* object_ptr{ (_object_header*)MEM->object_header_data };
    _slider_data* object_data_ptr{ (_slider_data*)MEM->object_body_data };

    slider_point* slider_ptr{ MEM->SLIDER_PATHS };

    const char** line_ptr{ MEM->lines };
    const char** line_ptr_end{ MEM->lines };

    auto* slider_defer_table{ MEM->slider_defer_table };


    {

        const auto P_SAVE = p;

        //_Timer A{};
        //for (size_t CRANK{}; CRANK < 100000; ++CRANK) 
        {

            line_ptr = MEM->lines;
            p = P_SAVE;

            {

                *line_ptr++ = p;

                for (; p + 64 <= end; p += 63) {

                    const auto v0 = _mm256_loadu_si256((__m256i const*)(p + 0x00));
                    const auto v1 = _mm256_loadu_si256((__m256i const*)(p + 0x20));

                    ++p;

                    const auto m0 = (u32)_mm256_movemask_epi8(_mm256_cmpeq_epi8(v0, nl));
                    const auto m1 = (u32)_mm256_movemask_epi8(_mm256_cmpeq_epi8(v1, nl));

                    auto mask = u64(m0) | (u64(m1) << 32);

                    #define DO { const auto bit = _tzcnt_u64(mask); *(line_ptr++) = p + bit; mask = _blsr_u64(mask); }

                    if (mask == 0) continue;
                    DO;
                    if (mask == 0) continue;
                    DO;
                    if (mask == 0) continue;
                    DO;
                    if (mask == 0) continue;
                    DO;
                    while (mask) [[unlikely]] DO

                    #undef DO

                }

                line_ptr_end = line_ptr;

                *line_ptr++ = nullptr;
                *line_ptr = nullptr;

                line_ptr = MEM->lines;

                _mm256_zeroupper();

            }

            for (; line_ptr != line_ptr_end; ++line_ptr) {

                if (*(const u64*)(*line_ptr) == 0x656A624F7469485Bull) {
                    ++line_ptr;
                    break;
                }

            }

            //for (size_t CRANK{}; CRANK < 100000; ++CRANK)
            {
                object_ptr = (_object_header*)MEM->object_header_data;
                object_data_ptr = (_slider_data*)MEM->object_body_data;
                slider_ptr = MEM->SLIDER_PATHS;
                slider_defer_table = MEM->slider_defer_table;
                //line_ptr = MEM->lines;

                for (;;) {

                    if (line_ptr == line_ptr_end)
                        break;

                    const u8* p{ (u8 const*)*line_ptr };

                    //if (p == nullptr) break;

                    const auto m0 = _mm_loadu_si128((__m128i*)p);
                    auto v = (u32)_mm_movemask_epi8(_mm_cmpeq_epi8(m0, _mm_set1_epi8(',')));

                    const auto c0 = (u32)_tzcnt_u32(v);
                    v = _blsr_u32(v);

                    const auto c1 = (u32)_tzcnt_u32(v);
                    v = _blsr_u32(v);

                    const auto c2 = (u32)_tzcnt_u32(v);

                    switch (c2 - c1/*time_digits*/) {

                    case 5: goto parse4;
                    case 6: goto parse5;
                    case 7: goto parse6;
                    case 8: goto parse7;

                    case 1:
                    case 2:
                    case 3:
                    case 4:
                        break;
                    default: goto parse_finished;
                    }

                    ON_SCOPE_EXIT(
                        ++object_ptr;
                        ++object_data_ptr;
                        ++line_ptr;
                    );

                    v = _blsr_u32(v);

                    const auto c3 = (u32)_tzcnt_u32(v);

                    object_ptr->x = parse_integer_m3::fixed_likely_3(load_u32(p), (c0));
                    object_ptr->y = parse_integer_m3::fixed_likely_3(load_u32(p + c0 + 1), (c1 - c0) - 1);
                    object_ptr->time = parse_integer_m3::fixed_likely_3(load_u32(p + c1 + 1), (c2 - c1) - 1);
                    object_ptr->type = parse_integer_m3::fixed_likely_1(load_u32(p + c2 + 1), (c3 - c2) - 1);

                    if (object_ptr->type & 2) {
                        parse_slider_body((const char*)p + c3 + 1, slider_ptr, object_data_ptr);
                        slider_ptr = object_data_ptr->point_end;
                    }

                }

                {
                parse4: //if (*line_ptr == nullptr) goto parse_finished;

                    {

                        const auto res = parse_object_loop<parse_4_time::parse_object_4digit_single>(line_ptr, object_ptr, object_data_ptr, slider_ptr);

                        slider_ptr += u32(res >> 32);
                        const auto count = u32(res);

                        line_ptr += count;
                        object_ptr += count;
                        object_data_ptr += count;

                    }

                parse5: //if (*line_ptr == nullptr) goto parse_finished;

                    {

                    //const auto res = PAIR_parse_object_loop<parse_5_time::parse_object_5digit_pair>(
                        const auto res = parse_object_loop<parse_5_time::parse_object_5digit_single>(
                            line_ptr, object_ptr, object_data_ptr, slider_ptr);

                        slider_ptr += u32(res >> 32);

                        const auto count = u32(res);

                        line_ptr += count;
                        object_ptr += count;
                        object_data_ptr += count;

                    }

                parse6: if (line_ptr == line_ptr_end) goto parse_finished;
                {
                    // only pair that wins for now
                    const auto res = PAIR_parse_object_loop<parse_6_time::parse_object_6digit_pair>(
                    //const auto res = parse_object_loop<parse_6_time::parse_object_6digit_single>(
                        line_ptr, object_ptr, object_data_ptr, slider_ptr);

                    slider_ptr += u32(res >> 32);

                    const auto count = u32(res);

                    line_ptr += count;
                    object_ptr += count;
                    object_data_ptr += count;

                }

                parse7: if (line_ptr == line_ptr_end) goto parse_finished;
                {
                    //const auto res = PAIR_parse_object_loop<parse_7_time::parse_object_7digit_SIMD_pair>(
                    const auto res = parse_object_loop<parse_7_time::parse_object_7digit_single>(
                        line_ptr, object_ptr, object_data_ptr, slider_ptr);

                    slider_ptr += u32(res >> 32);

                    const auto count = u32(res);

                    line_ptr += count;
                    object_ptr += count;
                    object_data_ptr += count;

                }

            }

            parse_finished: int a{};

            }

        }

    }

    MEM->note_count = object_ptr - (_object_header*)MEM->object_header_data;

    return;
}

#include <filesystem>
#include <iostream>

void run_test_folder() {

    return;
    _memory_region MR{};

    u32 XOR_TOTAL{};
    u32 COUNT{};
    {
        //_Timer A{};
        //std::vector<u8> FILE_BUFFER{}; FILE_BUFFER.reserve(u16(-1));
        std::chrono::steady_clock::duration total_elapsed_time = std::chrono::steady_clock::duration::zero();

        _Timer A{};
        for (const auto& file_entry : std::filesystem::directory_iterator("../fast_beatmap_load/map/maps/")) {

            const auto _p{ file_entry.path().native() };

            const auto file_name{ std::string(_p.begin(), _p.end()) };

            if (file_name.find(".osu") == std::string::npos)
                continue;

            printf("%s\n", file_name.c_str());
            std::chrono::steady_clock::time_point start_time = std::chrono::steady_clock::now();

            auto FILE_BUFFER = read_file(file_name.c_str());

            FILE_BUFFER.push_back('\n');
            FILE_BUFFER.resize(FILE_BUFFER.size() + 128);

            ++COUNT;

            //if ((COUNT & ((1<<10)-1)) == 0) printf("%i\n", COUNT);
            //if (COUNT > 30000)
            //    break;

            u32 XOR = 0;

            //for (size_t CRANK{}; CRANK < 1000; ++CRANK)
            parse_beatmap_from_memory(&MR, (char*)FILE_BUFFER.data(), (char*)FILE_BUFFER.data() + FILE_BUFFER.size() - 128);

            //MR.print_map_data();

            total_elapsed_time += std::chrono::steady_clock::now() - start_time;

            //const auto duration = (u64)(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - //start_time)).count();
            ////
            //double nano_seconds{ (double(duration) / 1000.) };
            //double micro_seconds{ nano_seconds / 1000. };

            //printf("%s> %.2f\xE6s (%.2fns)\n", file_name.substr(file_name.find_last_of('/') + 1).c_str(), micro_seconds, nano_seconds / double(MR.note_count ? MR.note_count : 1));

            XOR_TOTAL ^= MR.object_body_data[593];
            XOR_TOTAL ^= MR.object_header_data[4882];
            XOR_TOTAL += XOR ^ MR.object_body_data[52];
            XOR_TOTAL += MR.note_count;;

        }

        const auto duration = (u64)(std::chrono::duration_cast<std::chrono::milliseconds>(total_elapsed_time).count());
        //
        //double nano_seconds{ (double(duration) / 1000.) };
        //double micro_seconds{ nano_seconds / 1000. };
        printf("TOTAL_TIME: %fs\n", double(duration) / 1000.);
    }
    printf("%i\n", XOR_TOTAL);

}

int main() {

    //run_test_folder();
    //return 0;

    auto data = read_file("within_objects.txt");

    data.push_back('\n');
    data.resize(data.size() + 128);

    _memory_region MR{}; MR.init_memory();

    for (size_t warm_up{}; warm_up < 1000; ++warm_up)
        parse_beatmap_from_memory(&MR, (char*)data.data(), (char*)data.data() + data.size() - 128);

    u64 XOR{};
    {
        _Timer A{};
        parse_beatmap_from_memory(&MR, (char*)data.data(), (char*)data.data() + data.size() - 128);
    }

    std::cin.get();
    MR.print_map_data();

    return 0;

}