#pragma once

#include "../oof.h"

#include <cmath>
#include <string>
#include <vector>
#include <chrono>

#include <s9w/s9w_geom_types.h>
#include <s9w/s9w_geom_alg.h>
#include <s9w/s9w_rng.h>
#include <s9w/s9w_colors.h>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>


inline auto enable_vt_mode() -> void
{
   HANDLE const handle = GetStdHandle(STD_OUTPUT_HANDLE);
   if (handle == INVALID_HANDLE_VALUE)
      std::terminate(); // error handling

   DWORD dwMode = 0;
   if (!GetConsoleMode(handle, &dwMode))
      std::terminate(); // error handling

   if (dwMode & ENABLE_VIRTUAL_TERMINAL_PROCESSING)
      return; // VT mode is already enabled

   dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
   if (!SetConsoleMode(handle, dwMode))
      std::terminate(); // error handling
}

template<typename char_type>
auto fast_print(const std::basic_string<char_type>& sss) -> void {
   HANDLE const output_handle = GetStdHandle(STD_OUTPUT_HANDLE);
   const auto char_count = static_cast<DWORD>(sss.length());
   if constexpr (std::is_same_v<char_type, char>)
      WriteConsoleA(output_handle, sss.c_str(), char_count, nullptr, nullptr);
   else
      WriteConsoleW(output_handle, sss.c_str(), char_count, nullptr, nullptr);
}


// Returns width and height in cell count
inline auto get_screen_cell_dimensions() -> s9w::ivec2 {
   CONSOLE_SCREEN_BUFFER_INFO csbi;
   GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
   return s9w::ivec2{
      csbi.srWindow.Right - csbi.srWindow.Left + 1,
      csbi.srWindow.Bottom - csbi.srWindow.Top + 1
   };
}


// Returns position of console cursor. Zero-based
inline auto get_cursor_pos_xy() -> s9w::ivec2 {
   CONSOLE_SCREEN_BUFFER_INFO cbsi;
   GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cbsi);
   return s9w::ivec2{
      cbsi.dwCursorPosition.X,
      cbsi.dwCursorPosition.Y - 1 // This is correct, this function is weird
   };
}


// Returns font width and height in pixel
inline auto get_font_width_height() -> s9w::ivec2 {
   CONSOLE_FONT_INFOEX result;
   result.cbSize = sizeof(CONSOLE_FONT_INFOEX);
   GetCurrentConsoleFontEx(GetStdHandle(STD_OUTPUT_HANDLE), false, &result);
   return s9w::ivec2{ result.dwFontSize.X, result.dwFontSize.Y };
}


inline auto set_window_title(const std::string& title) -> void {
   SetConsoleTitleA(title.c_str());
}


template<typename return_type = int>
auto get_int(const double in) -> return_type {
   return static_cast<return_type>(std::round(in));
}


template <class T>
void append_moved(std::vector<T>& dst, std::vector<T>&& src) {
   if (dst.empty())
      dst = std::move(src);
   else {
      dst.reserve(dst.size() + src.size());
      std::move(std::begin(src), std::end(src), std::back_inserter(dst));
      src.clear();
   }
}


template<typename T, typename pred_type>
auto remove_from_vector(
   std::vector<T>& container,
   const pred_type& pred
) -> void
{
   container.erase(
      std::remove_if(
         container.begin(),
         container.end(),
         pred
      ),
      container.end()
   );
}


inline auto get_random_color(s9w::rng_state& rng) -> oof::color
{
   return oof::color{
      rng.get_int(0, 255),
      rng.get_int(0, 255),
      rng.get_int(0, 255)
   };
}


// Sets up double ranges
struct ranger {
   std::vector<double> m_borders;

   template<typename ... border_types>
   explicit ranger(const border_types&... borders) {
      static_assert((std::same_as<border_types, double> && ...), "All borders must be double values");
      static_assert(sizeof...(borders) >= 2, "Needs at least two border values");

      ((m_borders.emplace_back(borders)), ...);
      for (int i = 1; i < std::size(m_borders); ++i) {
         if (m_borders[i - 1] < m_borders[i] == false)
            std::terminate();
      }
   }

   auto is_in(const double value) const -> bool {
      const bool is_out = value < m_borders.front() || value > m_borders.back();
      return is_out == false;
   }

   auto get_zone(const double value) const -> std::optional<size_t> {
      if (is_in(value) == false)
         return std::nullopt;
      return get_zone_direct(value);
   }

   auto get_progress(const double value) const -> std::optional<double> {
      if (is_in(value) == false)
         return std::nullopt;
      const double zone_min = get_zone_min(value);
      const double zone_max = get_zone_max(value);
      return (value - zone_min) / (zone_max - zone_min);
   }

private:
   // Assumes that value is in
   auto get_zone_min(const double value) const -> double {
      return m_borders[this->get_zone_direct(value)];
   }
   auto get_zone_max(const double value) const -> double {
      if (value == m_borders.back())
         return m_borders.back();
      const size_t zone_index = this->get_zone_direct(value);
      return m_borders[zone_index + 1];
   }
   auto get_zone_direct(const double value) const -> size_t {
      // Last border is inclusive
      if (value == m_borders.back())
         return m_borders.size() - 2;

      for (size_t i = 0; i < m_borders.size(); ++i) {
         if (value < m_borders[i])
            return i - 1;
      }
      std::terminate();
   }
};


// provides:
// - Time since start (to feed into sin() etc)
// - Time since last frame (for time-delta actions)
// - FPS functionality
struct timer {
private:
   std::chrono::high_resolution_clock::time_point m_start_time;
   std::chrono::high_resolution_clock::time_point m_last_tick_time;
   std::chrono::high_resolution_clock::time_point m_last_fps_time;
   int m_fps_frame_count = 0;
   std::string m_description;
   
public:
   inline timer(std::string description);
   inline timer();
   inline auto mark_frame() -> double;
   inline auto get_seconds_since_start() const -> double;

   // in seconds
   inline auto get_fps() -> std::optional<double>;
   inline auto get_total_ms() const -> std::string;
};


timer::timer(std::string description)
   : m_start_time(std::chrono::high_resolution_clock::now())
   , m_last_tick_time(std::chrono::high_resolution_clock::now())
   , m_last_fps_time(std::chrono::high_resolution_clock::now())
   , m_description(std::move(description))
{

}

timer::timer()
   : timer("")
{

}


auto timer::get_fps() -> std::optional<double>
{
   const auto now = std::chrono::high_resolution_clock::now();
   auto seconds = std::chrono::duration_cast<std::chrono::microseconds>(now - m_last_fps_time).count() / 1'000'000.0;
   if (seconds < 1.0)
      return std::nullopt;
   const double fps = m_fps_frame_count / seconds;
   m_fps_frame_count = 0;
   m_last_fps_time = now;
   return fps;
}


auto timer::mark_frame() -> double
{
   ++m_fps_frame_count;
   const auto now = std::chrono::high_resolution_clock::now();
   const auto seconds_dt = std::chrono::duration_cast<std::chrono::microseconds>(now - m_last_tick_time).count() / 1'000'000.0;
   m_last_tick_time = now;
   return seconds_dt;
}


auto timer::get_total_ms() const -> std::string
{
   const auto now = std::chrono::high_resolution_clock::now();
   const auto ms = std::chrono::duration_cast<std::chrono::microseconds>(now - m_start_time).count() / 1000.0;
   std::string result = m_description;
   result += ": ";
   result += std::to_string(ms);
   result += " ms";
   return result;
}


auto timer::get_seconds_since_start() const -> double
{
   const auto now = std::chrono::high_resolution_clock::now();
   const auto seconds = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_start_time).count() / 1000.0;
   return seconds;
}
