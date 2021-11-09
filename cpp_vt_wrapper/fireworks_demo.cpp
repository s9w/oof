#include "fireworks_demo.h"

#include <numbers>

#include "tools.h"
#include "../wrapper.h"
using namespace cvtsw;

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
      double m_trail_spread = 0.5;
      double m_mass = 50.0;
      s9w::hsluv_d m_color_range0{ 50.0, 100.0, 70.0 };
      s9w::hsluv_d m_color_range1{ 70.0, 100.0, 100.0 };
      
      rocket(const s9w::dvec2& pos, const s9w::dvec2& velocity, const double trail_spread, const double mass, const s9w::hsluv_d& color_range0, const s9w::hsluv_d& color_range1)
         : m_pos(pos)
         , m_velocity(velocity)
         , m_trail_spread(trail_spread)
         , m_mass(mass)
         , m_color_range0(color_range0)
         , m_color_range1(color_range1)
      {}
      
   };

   struct big_rocket : rocket{
      auto should_explode() const{
         return m_velocity[1] > 0;
      }

      using rocket::rocket;
   };

   struct small_rocket : rocket{
      double m_age = 0.0;

      using rocket::rocket;
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

   auto get_explosion_rockets(const rocket& r, const int n) -> std::vector<small_rocket>
   {
      std::vector<small_rocket> result;
      result.reserve(n);

      const double hue0 = rng.get_real(0.0, 330.0);
      const double hue1 = hue0 + 30.0;

      for (int i = 0; i < n; ++i) {
         const double angle = rng.get_real(0.0, two_pi);
         const double launch_speed = rng.get_real(20.0, 150.0);
         const auto velocity = s9w::rotate(s9w::dvec2{ launch_speed, 0.0 }, angle);

         result.push_back(
            small_rocket(
               r.m_pos, velocity, 1.0, 1.0,
               s9w::hsluv_d{hue0, 100.0, 50.0},
               s9w::hsluv_d{hue1, 100.0, 50.0}
            )
         );
      }
      return result;
   }

   template<typename T>
   auto make_glitter(
      const double dt,
      const pixel_screen& canvas,
      const T& r,
      std::vector<particle>& glitter
   ) -> void
   {
      double intensity = 1.0;
      if constexpr (std::same_as<T, small_rocket>)
         intensity = std::clamp(1.0 - r.m_age/3.0, 0.0, 1.0);

      constexpr double glitter_per_sec = 50.0;
      if (rng.get_flip(0.5 * glitter_per_sec * intensity * dt) == false)
         return;

      const s9w::dvec2 glitter_pos = get_noised_pos(r.m_pos, r.m_trail_spread);
      const auto& [column, row] = get_column_row(glitter_pos);
      if (column<0 || column > canvas.get_width() - 1 || row<0 || row>canvas.get_height() - 1)
         return;
      glitter.push_back(
         particle{
            .m_column = column,
            .m_row = row,
            .m_age = 0,
            .m_color = get_glitter_color(r.m_color_range0, r.m_color_range1)
         }
      );
   }

} // namespace {}

auto fireworks_demo() -> void
{
   constexpr double gravity = 10.0;

   pixel_screen canvas(140, 80, 0, 0, color{});
   timer timer;

   std::vector<small_rocket> small_rockets;
   std::vector<big_rocket> big_rockets;
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
         big_rockets.push_back(
            big_rocket(
               s9w::dvec2{rng.get_real(10.0, canvas.get_width() - 10.0), canvas.get_height()},
               s9w::dvec2{rng.get_real(-5.0, 5.0), rng.get_real(-40.0, -20.0)},
               0.5,
               50.0,
               s9w::hsluv_d{ 50.0, 100.0, 70.0 },
               s9w::hsluv_d{ 70.0, 100.0, 100.0 }
            )
         );
         time_to_next_rocket = rng.get_real(1.0, 5.0);
      }

      // Gravity
      for (rocket& r : big_rockets)
         r.m_velocity += s9w::dvec2{ 0.0, gravity } *dt;
      for (rocket& r : small_rockets)
         r.m_velocity += s9w::dvec2{ 0.0, gravity } *dt;

      // Drag only for small rockets
      for (rocket& r : small_rockets){
         constexpr double drag_constant = 0.1;
         const double v = s9w::get_length(r.m_velocity);
         const s9w::dvec2 F_drag = v * v * dt * drag_constant * s9w::get_normalized(r.m_velocity);
         const s9w::dvec2 speed_change = F_drag / r.m_mass;
         r.m_velocity -= speed_change;
      }

      // Velocity iteration
      for (rocket& r : big_rockets)
         r.m_pos += r.m_velocity * dt;
      for (rocket& r : small_rockets)
         r.m_pos += r.m_velocity * dt;

      // Explode rockets
      {
         std::vector<small_rocket> new_rockets;
         for (const big_rocket& r : big_rockets){
            if (r.should_explode() == false)
               continue;
            append_moved(new_rockets, get_explosion_rockets(r, 20));
         }
         append_moved(small_rockets, std::move(new_rockets));
      }

      // Remove (exploded) rockets
      remove_from_vector(
         big_rockets,
         [](const big_rocket& r) {return r.should_explode(); }
      );

      // Age rockets
      for (small_rocket& r : small_rockets)
         r.m_age += dt;

      // Make Glitter
      for (const small_rocket& r : small_rockets)
         make_glitter(dt, canvas, r, glitter);
      for (const big_rocket& r : big_rockets)
         make_glitter(dt, canvas, r, glitter);

      // Remove glowed out small rockets
      remove_from_vector(
         small_rockets,
         [&](const small_rocket& r) {
            return r.m_age > 3.0;
         }
      );

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

         s9w::srgb_u glitter_color;
         if (part.m_age < 0.2) {
            const double whiteness = std::clamp(1.0 - 5.0*part.m_age, 0.0, 1.0);
            constexpr s9w::srgb_u srgb_white{ 255, 255, 255 };
            glitter_color = s9w::mix(s9w::convert_color<s9w::srgb_u>(part.m_color), srgb_white, whiteness);
         }
         else {
            const double intensity_factor = std::clamp(1.0 - part.m_age, 0.0, 1.0);
            s9w::hsluv_d faded_color = part.m_color;
            faded_color.l *= intensity_factor;
            glitter_color = s9w::convert_color<s9w::srgb_u>(faded_color);
         }
         canvas.get_color(part.m_column, part.m_row) = std::bit_cast<color>(glitter_color);
      }

      timer.mark_frame();
      fast_print(canvas.get_string(color{ 0, 0, 0 }));

      const auto fps = timer.get_fps();
      if (fps.has_value())
         set_window_title("FPS: " + std::to_string(*fps));
   }
}

// TODO fadeout
