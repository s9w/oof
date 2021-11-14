#include "color_dot_demo.h"

#include "tools.h"

static s9w::rng_state rng;

namespace{

   auto get_random_color() -> oof::color
   {
      return oof::color{
         static_cast<uint8_t>(rng.get_int(0, 255)),
         static_cast<uint8_t>(rng.get_int(0, 255)),
         static_cast<uint8_t>(rng.get_int(0, 255))
      };
   }

} // namespace {}


auto color_dot_demo() -> void
{
   oof::pixel_screen px{ 60, 60, 0, 0, oof::color{0, 0, 0} };

   constexpr auto get_faded = [](const oof::color& col) {
      constexpr int fade_amount = 1;
      return oof::color{
         static_cast<uint8_t>(std::clamp(col.red - fade_amount, 0, 255)),
         static_cast<uint8_t>(std::clamp(col.green - fade_amount, 0, 255)),
         static_cast<uint8_t>(std::clamp(col.blue - fade_amount, 0, 255))
      };
   };

   timer timer;
   for (uint64_t i = 0; ; ++i) {
      {
         if (i % 10 == 0) {
            for (oof::color& col : px)
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
