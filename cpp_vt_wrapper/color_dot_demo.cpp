#include "color_dot_demo.h"

#include <algorithm>
#include <chrono>
#include <s9w/s9w_rng.h>

#include "../wrapper.h"
using namespace cvtsw;
#include "tools.h"

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
   pixel_screen px{ 60, 60, 0, 0, color{0, 0, 0} };

   constexpr auto get_faded = [](const color& col) {
      constexpr int fade_amount = 1;
      return color{
         static_cast<uint8_t>(std::clamp(col.red - fade_amount, 0, 255)),
         static_cast<uint8_t>(std::clamp(col.green - fade_amount, 0, 255)),
         static_cast<uint8_t>(std::clamp(col.blue - fade_amount, 0, 255))
      };
   };

   timer timer;
   for (uint64_t i = 0; ; ++i) {
      {
         if (i % 10 == 0) {
            for (color& col : px.m_pixels)
               col = get_faded(col);
         }
      }

      if(i%5==0)
         rng.choice(px.m_pixels) = get_random_color();

      timer.mark_frame();
      fast_print(px.get_string());
      const auto fps = timer.get_fps();
      if (fps.has_value())
         set_window_title("FPS: " + std::to_string(*fps));
   }
}
