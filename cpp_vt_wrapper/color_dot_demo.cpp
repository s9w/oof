#include "color_dot_demo.h"

#include <algorithm>
#include <chrono>
#include <s9w/s9w_rng.h>

#include "../wrapper.h"
using namespace cvtsw;
#include "tools.h"

#include <tracy/Tracy.hpp>

static s9w::rng_state rng;

namespace{

   auto get_random_color() -> color
   {
      return color{
         static_cast<uint8_t>(rng.get_int(0, 255)),
         static_cast<uint8_t>(rng.get_int(0, 255)),
         static_cast<uint8_t>(rng.get_int(0, 255))
      };
   }

} // namespace {}


auto color_dot_demo() -> void
{
   pixel_screen px{ 80, 80, 0, 0, color{0, 0, 0} };

   constexpr auto get_faded = [](const color& col) {
      constexpr int fade_amount = 15;
      return color{
         static_cast<uint8_t>(std::clamp(col.red - fade_amount, 0, 255)),
         static_cast<uint8_t>(std::clamp(col.green - fade_amount, 0, 255)),
         static_cast<uint8_t>(std::clamp(col.blue - fade_amount, 0, 255))
      };
   };

   const auto t0 = std::chrono::high_resolution_clock::now();
   for (uint64_t i = 0; ; ++i) {
      {
         ZoneScopedN("fading");
         if (i % 10 == 0) {
            for (color& col : px.m_pixels)
               col = get_faded(col);
         }
      }

      for(int i=0; i<2; ++i)
         rng.choice(px.m_pixels) = get_random_color();

      const std::wstring result_str = px.get_screen(color{ 0, 255, 0 }).get_string();
      fast_print(result_str);

      const auto t1 = std::chrono::high_resolution_clock::now();
      const double seconds = std::chrono::duration<double>(t1 - t0).count();
      const double fps = i / seconds;
      
      FrameMark;
   }
}
