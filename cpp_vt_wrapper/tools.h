#pragma once

#include "../wrapper.h"
using namespace cvtsw;

#include <cmath>
#include <string>
#include <vector>
#include <chrono>

#include <s9w/s9w_rng.h>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>


template<typename char_type>
auto fast_print(const std::basic_string<char_type>& sss) -> void {
   HANDLE const output_handle = GetStdHandle(STD_OUTPUT_HANDLE);
   const auto char_count = static_cast<DWORD>(sss.length());
   if constexpr (std::same_as<char_type, char>)
      WriteConsoleA(output_handle, sss.c_str(), char_count, nullptr, nullptr);
   else
      WriteConsoleW(output_handle, sss.c_str(), char_count, nullptr, nullptr);
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


inline auto get_random_color(s9w::rng_state& rng) -> color
{
   return color{
      static_cast<uint8_t>(rng.get_int(0, 255)),
      static_cast<uint8_t>(rng.get_int(0, 255)),
      static_cast<uint8_t>(rng.get_int(0, 255))
   };
}


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
   inline auto mark_frame() -> void;
   inline auto get_seconds_since_start() const -> double;

   // in seconds
   inline auto get_dt() const -> double;
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


auto timer::get_dt() const -> double
{
   const auto now = std::chrono::high_resolution_clock::now();
   auto seconds = std::chrono::duration_cast<std::chrono::microseconds>(now - m_last_tick_time).count() / 1'000'000.0;
   return seconds;
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


auto timer::mark_frame() -> void
{
   ++m_fps_frame_count;
   m_last_tick_time = std::chrono::high_resolution_clock::now();
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