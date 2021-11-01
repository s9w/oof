#pragma once

#include "../wrapper.h"
using namespace cvtsw;

#include <cmath>
#include <string>
#include <vector>

#include <s9w/s9w_rng.h>
#include <tracy/Tracy.hpp>

#define NOMINMAX
#include <windows.h>



template<typename char_type>
__declspec(noinline)
auto fast_print(const std::basic_string<char_type>& sss) -> void {
   ZoneScopedS(20);
   LPDWORD chars_written = 0;
   const HANDLE output_handle = GetStdHandle(STD_OUTPUT_HANDLE);
   const auto char_count = static_cast<DWORD>(sss.length());
   if constexpr (std::same_as<char_type, char>)
      WriteConsoleA(output_handle, sss.c_str(), char_count, chars_written, 0);
   else
      WriteConsoleW(output_handle, sss.c_str(), char_count, chars_written, 0);
}


template<typename return_type = int>
auto get_int(const double in) -> return_type {
   return static_cast<return_type>(std::round(in));
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
