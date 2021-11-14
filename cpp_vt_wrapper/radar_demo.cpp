#include "radar_demo.h"

#include "tools.h"

static s9w::rng_state rng;

namespace
{
   // TODO replace fading globally with difference blending in s9w_color
   auto get_faded(const oof::color& col) -> oof::color {
      constexpr int fade_amount = 20;
      return oof::color{
         static_cast<uint8_t>(std::clamp(col.red - fade_amount, 0, 255)),
         static_cast<uint8_t>(std::clamp(col.green - fade_amount, 0, 255)),
         static_cast<uint8_t>(std::clamp(col.blue - fade_amount, 0, 255))
      };
   };

   auto nonstupid_atan2(const double y, const double x){
      double result = std::atan2(y, x);
      if (result < 0.0)
         result += 2.0 * std::numbers::pi_v<double>;
      return result;
   }
} // namespace {}


auto radar_demo() -> void
{
   const int radar_width = 2 * get_screen_cell_dimensions()[1];
   
   oof::pixel_screen px{ radar_width, radar_width, 0, 0, oof::color{} };
   const s9w::dvec2 center{ radar_width / 2.0 };
   constexpr s9w::dvec2 half_pixel_offset{ 0.5 };
   const double radar_radius = radar_width / 2.0 - 5.0;

   timer timer;
   std::wstring string_buffer;
   while(true){
      { // Randomly fading pixels to black
         const int pixel_count = px.get_height() * px.get_width();
         const int fade_runs = get_int(100.0 * pixel_count * timer.get_dt());
         for (int i = 0; i < fade_runs; ++i) {
            oof::color& pixel = rng.choice(px.m_pixels);
            pixel = get_faded(pixel);
         }
      }

      // Radar arm
      constexpr double speed = 3.0;
      const double radar_phi = std::fmod(speed * timer.get_seconds_since_start(), 2.0 * std::numbers::pi_v<double>);
      for(int y=0; y< radar_width; ++y){
         for (int x = 0; x < radar_width; ++x){
            const s9w::dvec2 rel_pos = s9w::dvec2{ x, y } - center + half_pixel_offset;
            const double pixel_phi = nonstupid_atan2(rel_pos[1], rel_pos[0]);
            if(s9w::equal(pixel_phi, radar_phi, 0.05) && s9w::get_length(rel_pos) < radar_radius)
               px.get_color(x, y) = oof::color{ 255, 0, 0 };
         }
      }
      
      timer.mark_frame();
      px.write_string(string_buffer);
      fast_print(string_buffer);
      if(const auto fps = timer.get_fps(); fps.has_value())
         set_window_title("FPS: " + std::to_string(*fps));
   }
}
