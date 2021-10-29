#include "radar_demo.h"

#include "color_dot_demo.h"

#include <algorithm>
#include <chrono>
#include <s9w/s9w_rng.h>
#include <s9w/s9w_geom_types.h>
#include <s9w/s9w_geom_alg.h>
#include <s9w/s9w_colors.h>

#include "../wrapper.h"
using namespace cvtsw;
#include "tools.h"

static s9w::rng_state rng;

namespace
{
   auto line_sdf(
      const s9w::dvec2& p,
      const s9w::dvec2& a,
      const s9w::dvec2& b
   ) -> double
   {
      const s9w::dvec2 pa = p - a, ba = b - a;
      const double h = std::clamp(s9w::dot(pa, ba) / s9w::dot(ba, ba), 0.0, 1.0);
      return s9w::get_length(pa - ba * h);
   }

   auto circle_sdf(
      const s9w::dvec2& p,
      const s9w::dvec2& center,
      double r
   ) -> double
   {
      return s9w::get_length(p - center) - r;
   }

}

auto radar_demo() -> void
{
   const auto get_faded = [](const color& col) {
      constexpr int fade_amount = 20;
      return color{
         static_cast<uint8_t>(std::clamp(col.red - fade_amount, 0, 255)),
         static_cast<uint8_t>(std::clamp(col.green - fade_amount, 0, 255)),
         static_cast<uint8_t>(std::clamp(col.blue - fade_amount, 0, 255))
      };
   };

   constexpr int width = 100;
   constexpr int height = 80;
   pixel_screen px{ width, height, 0, 0, color{0, 0, 0} };
   const auto t0 = std::chrono::high_resolution_clock::now();
   for (uint64_t i = 0; ; ++i) {
      const auto t1 = std::chrono::high_resolution_clock::now();
      const double seconds = std::chrono::duration<double>(t1 - t0).count();

      // fade
      for(int i=0; i<5000; ++i){
         auto& choice = rng.choice(px.m_pixels);
         choice = get_faded(choice);
      }

      // new
      constexpr double frequency = 3.0;
      const s9w::dvec2 radar_end_pos{
         0.9 * width / 2.0 * std::sin(-frequency * seconds),
         0.9 * height / 2.0 * std::cos(-frequency * seconds)
      };
      const s9w::dvec2 delayed_radar_end_pos{
         0.9 * width / 2.0 * std::sin(-frequency * (seconds-0.1)),
         0.9 * height / 2.0 * std::cos(-frequency * (seconds-0.1))
      };

      // const bool do_blip = rng.get_flip(0.01);
      // auto& blip_
      for(int y=0; y<height; ++y){
         for (int x = 0; x < width; ++x){
            const s9w::dvec2 pos{x - width/2.0 + 0.5, y - height/2.0 + 0.5};
            const double sdf = line_sdf(pos, s9w::dvec2{}, radar_end_pos);
            if (sdf < 1.0) {
               s9w::hsluv_d line_color{ std::sin(-frequency * seconds/(2*std::numbers::pi_v<double>)) * 360.0, 100, 50 };
               const auto line_color_rgb = s9w::convert_color<s9w::srgb_u>(line_color);

               px.get_color(x, y) = std::bit_cast<color>(line_color_rgb);
               // px.get_color(x, y) = color{ 0, 230, 0 };
            }
         }
      }

      // blop
      if(rng.get_flip(0.1)){
         for(int i=0; i<10000; ++i)
         {
            const int x = rng.get_int(0, width);
            const int y = rng.get_int(0, height);
            const s9w::dvec2 pos{ x - width / 2.0 + 0.5, y - height / 2.0 + 0.5 };
            const double sdf = line_sdf(pos, s9w::dvec2{}, delayed_radar_end_pos);

            // if (sdf < 1.5) {
            //    // const s9w::dvec2 center{ x, y };
            //    const s9w::dvec2 center{ x - width / 2.0 + 0.5, y - height / 2.0 + 0.5 };
            //    for (int y = 0; y < height; ++y) {
            //       for (int x = 0; x < width; ++x) {
            //          const s9w::dvec2 pos{ x - width / 2.0 + 0.5, y - height / 2.0 + 0.5 };
            //          const double circle_dist = circle_sdf(pos, center, 1.0);
            //          if(circle_dist < 3.0)
            //             px.get_color(x, y) = color{ 255, 255, 100 };
            //       }
            //    }
            //
            //    // px.get_color(x, y) = color{ 0, 230, 0 };
            //    break;
            // }
         }
      }

      const std::wstring result_str = px.get_screen(color{ 0, 255, 0 }).get_string();
      fast_print(result_str);
   }
}
