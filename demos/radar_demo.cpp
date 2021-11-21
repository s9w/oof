#include "radar_demo.h"

#include "tools.h"



namespace
{
   s9w::rng_state rng{static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count())};
   
   constexpr double two_pi = 2.0 * std::numbers::pi_v<double>;

   // TODO replace fading globally with difference blending in s9w_color
   auto get_faded(const oof::color& col) -> oof::color {
      constexpr int fade_amount = 20;
      return oof::color{
         std::clamp(col.red - fade_amount, 0, 255),
         std::clamp(col.green - fade_amount, 0, 255),
         std::clamp(col.blue - fade_amount, 0, 255)
      };
   };

   auto nonstupid_atan2(const double y, const double x){
      double result = std::atan2(y, x);
      if (result < 0.0)
         result += two_pi;
      return result;
   }
} // namespace {}


auto radar_demo() -> void
{
   const int radar_width = 2 * get_screen_cell_dimensions()[1];
   
   oof::pixel_screen px{ radar_width, radar_width };
   const s9w::dvec2 center{ radar_width / 2.0 };
   constexpr s9w::dvec2 half_pixel_offset{ 0.5 };
   const double radar_radius = radar_width / 2.0 - 5.0;

   timer timer;
   std::wstring string_buffer;
   double dt{};
   while(true){
      { // Randomly fading pixels to black
         const int pixel_count = px.get_halfline_height() * px.get_width();
         const int fade_runs = get_int(30.0 * pixel_count * dt);
         for (int i = 0; i < fade_runs; ++i) {
            oof::color& pixel = rng.choice(px.m_pixels);
            pixel = get_faded(pixel);
         }
      }

      // Radar arm
      constexpr double seconds_for_rotation = 2.0;
      const double radar_phi = std::fmod(timer.get_seconds_since_start() * two_pi / seconds_for_rotation, two_pi);
      for(int y=0; y< radar_width; ++y){
         for (int x = 0; x < radar_width; ++x){
            const s9w::dvec2 rel_pos = s9w::dvec2{ x, y } - center + half_pixel_offset;
            const double pixel_phi = nonstupid_atan2(rel_pos[1], rel_pos[0]);
            if(s9w::equal(pixel_phi, radar_phi, 0.05) && s9w::get_length(rel_pos) < radar_radius)
               px.get_color(x, y) = { 255, 0, 0 };
         }
      }
      
      dt = timer.mark_frame();
      px.get_string(string_buffer);
      fast_print(string_buffer);
      if(const auto fps = timer.get_fps(); fps.has_value())
         set_window_title("FPS: " + std::to_string(*fps));
   }
}
