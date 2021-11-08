#include "fireworks_demo.h"

#include <numbers>

#include "../wrapper.h"
using namespace cvtsw;
#include "tools.h"

#include <s9w/s9w_geom_types.h>
#include <s9w/s9w_geom_alg.h>
#include <s9w/s9w_rng.h>
#include <s9w/s9w_colors.h>

namespace{
   s9w::rng_state rng{1};
   constexpr double two_pi = 2.0 * std::numbers::pi_v<double>;

   struct rocket{
      s9w::dvec2 m_pos;
      s9w::dvec2 m_velocity;
      bool m_can_explode = false;
      double m_trail_spread = 0.5;
      double m_mass = 50.0;
      s9w::hsluv_d m_color_range0{50.0, 100.0, 70.0};
      s9w::hsluv_d m_color_range1{70.0, 100.0, 100.0};
      

      auto should_explode() const
      {
         return m_can_explode && m_velocity[1] > 0;
      }
   };

   struct particle{
      int m_column;
      int m_row;
      double m_age;
      s9w::hsluv_d m_color;
   };

   auto get_noised_pos(const s9w::dvec2& pos, const double amount) -> s9w::dvec2
   {
      s9w::dvec2 result = pos;
      result[0] += rng.get_real(-amount, amount);
      result[1] += rng.get_real(-amount, amount);
      return result;
   }


   auto get_column_row(const s9w::dvec2& pos) -> std::pair<int, int>{
      return { get_int(pos[0]), get_int(pos[1]) };
   }

   auto get_glitter_color(
      const s9w::hsluv_d& range0,
      const s9w::hsluv_d& range1
   ) -> s9w::hsluv_d
   {
      const double hue = rng.get_real(range0.h, range1.h);
      const double saturation = rng.get_real(range0.s, range1.s);
      const double lightness = rng.get_real(range0.l, range1.l);
      s9w::hsluv_d hsluv_color{ hue, saturation, lightness };
      return hsluv_color;
   }

   auto get_explosion_rockets(const rocket& r, const int n) -> std::vector<rocket>
   {
      std::vector<rocket> result;
      result.reserve(n);

      const double hue0 = rng.get_real(0.0, 330.0);
      const double hue1 = hue0 + 30.0;

      for (int i = 0; i < n; ++i) {
         const double angle = rng.get_real(0.0, two_pi);
         const double launch_speed = rng.get_real(20.0, 80.0);
         const auto velocity = s9w::rotate(s9w::dvec2{ launch_speed, 0.0 }, angle);

         result.push_back(
            rocket{
               .m_pos = r.m_pos,
               .m_velocity = velocity,
               .m_can_explode = false,
               .m_trail_spread = 1.5,
               .m_color_range0{hue0, 100.0, 50.0},
               .m_color_range1{hue1, 100.0, 50.0}
            }
         );
      }
      return result;
   }

} // namespace {}

auto fireworks_demo() -> void
{
   constexpr double gravity = 10.0;

   pixel_screen canvas(120, 80, 0, 0, color{});
   timer timer;

   std::vector<rocket> rockets;
   std::vector<particle> glitter;
   double time_to_next_rocket = 0.0;
   while(true)
   {
      const double dt = timer.get_dt();

      // Clear
      for (color& c : canvas)
         c = color{};

      // Spawn new rockets
      time_to_next_rocket -= dt;
      if(time_to_next_rocket < 0)
      {
         rockets.push_back(
            rocket{
               .m_pos{rng.get_real(10.0, canvas.get_width() - 10.0), canvas.get_height()},
               .m_velocity{rng.get_real(-5.0, 5.0), rng.get_real(-100.0, -50.0)},
               .m_can_explode = true
            }
         );
         time_to_next_rocket = rng.get_real(1.0, 5.0);
      }

      // Forces
      for (rocket& r : rockets){
         // Gravity
         r.m_velocity += s9w::dvec2{0.0, gravity } * dt;

         // Drag
         constexpr double drag_constant = 2.0;
         const double v = s9w::get_length(r.m_velocity);
         const s9w::dvec2 F_drag = v * v * dt * drag_constant * s9w::get_normalized(r.m_velocity);
         const s9w::dvec2 speed_change = F_drag / r.m_mass;
         r.m_velocity -= speed_change;
      }

      // Velocity iteration
      for (rocket& r : rockets)
         r.m_pos += r.m_velocity * dt;

      // Explode rockets
      {
         std::vector<rocket> new_rockets;
         for (const rocket& r : rockets){
            if (r.should_explode() == false)
               continue;
            append_moved(rockets, get_explosion_rockets(r, 15));
         }
         append_moved(rockets, std::move(new_rockets));
      }

      // Remove (exploded) rockets
      remove_from_vector(
         rockets,
         [](const rocket& r) {
            return r.m_can_explode && r.should_explode();
         }
      );

      // Remove out of screen rockets
      remove_from_vector(
         rockets,
         [&](const rocket& r) {
            return r.m_pos[0] < 0.0 || r.m_pos[0] > canvas.get_width() || r.m_pos[1] < 0.0 || r.m_pos[1] > canvas.get_height();
         }
      );

      // Make Glitter
      for (const rocket& r : rockets){
         constexpr double glitter_per_sec = 50.0;
         if(rng.get_flip(0.5 * glitter_per_sec * dt) == false)
            continue;

         const s9w::dvec2 glitter_pos = get_noised_pos(r.m_pos, r.m_trail_spread);
         const auto& [column, row] = get_column_row(glitter_pos);
         if(column<0 ||column > canvas.get_width()-1 || row<0 || row>canvas.get_height()-1)
            continue;
         glitter.push_back(
            particle{
               .m_column = column,
               .m_row = row,
               .m_age = 0,
               .m_color = get_glitter_color(r.m_color_range0, r.m_color_range1)
            }
         );
      }

      // Cleanup glitter
      remove_from_vector(
         glitter,
         [](const particle& part){
            return part.m_age > 1.0;
         }
      );

      // Draw glitter
      for(particle& part : glitter){
         part.m_age += dt;
         const double intensity_factor = std::clamp(1.0 - part.m_age, 0.0, 1.0);
         s9w::hsluv_d faded_color = part.m_color;
         faded_color.l *= intensity_factor;
         canvas.get_color(part.m_column, part.m_row) = std::bit_cast<color>(s9w::convert_color<s9w::srgb_u>(faded_color));
      }


      timer.mark_frame();
      fast_print(canvas.get_string(color{ 0, 0, 0 }));

      const auto fps = timer.get_fps();
      if (fps.has_value())
         set_window_title("FPS: " + std::to_string(*fps));
   }
}

// TODO atmo drag; fadeout 