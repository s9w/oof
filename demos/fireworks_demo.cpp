#include "fireworks_demo.h"

#include <numbers>

#include "tools.h"

namespace{
   s9w::rng_state rng{static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count())};
   constexpr double two_pi = 2.0 * std::numbers::pi_v<double>;

   struct rocket{
      s9w::dvec2 m_pos;
      s9w::dvec2 m_velocity;
      double m_trail_spread{};
      double m_mass{};
      s9w::hsluv_d m_color_range0{};
      s9w::hsluv_d m_color_range1{};
   };

   struct big_rocket : rocket{
      auto should_explode() const -> bool{
         return m_velocity[1] > 0;
      }

      static constexpr double big_trail_spread = 0.5;
      static constexpr double big_mass = 50.0;
      static constexpr s9w::hsluv_d big_range0{ 50.0, 100.0, 70.0 };
      static constexpr s9w::hsluv_d big_range1{ 70.0, 100.0, 100.0 };
      big_rocket(const s9w::dvec2& pos, const s9w::dvec2& velocity)
         : rocket{ pos, velocity, big_trail_spread, big_mass, big_range0, big_range1 }
      {}
   };

   struct small_rocket : rocket{
      double m_age = 0.0;

      static constexpr double small_trail_spread = 1.0;
      static constexpr double small_mass = 1.0;
      small_rocket(const s9w::dvec2& pos, const s9w::dvec2& velocity, const s9w::hsluv_d& range0, const s9w::hsluv_d& range1)
         : rocket{ pos, velocity, small_trail_spread, small_mass, range0, range1 }
      {}
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
      return s9w::hsluv_d{
         .h = rng.get_real(range0.h, range1.h),
         .s = rng.get_real(range0.s, range1.s),
         .l = rng.get_real(range0.l, range1.l)
      };
   }

   auto get_explosion_rockets(const rocket& r, const int n) -> std::vector<small_rocket>
   {
      std::vector<small_rocket> result;
      result.reserve(n);

      const double hue0 = rng.get_real(0.0, 330.0);
      const double hue1 = hue0 + 30.0;

      for (int i = 0; i < n; ++i) {
         const double angle = rng.get_real(0.0, two_pi);
         const double launch_speed = rng.get_real(5.0, 150.0);
         const auto velocity = s9w::rotate(s9w::dvec2{ launch_speed, 0.0 }, angle);

         result.emplace_back(
            r.m_pos, velocity,
            s9w::hsluv_d{hue0, 100.0, 50.0},
            s9w::hsluv_d{hue1, 100.0, 50.0}
         );
      }
      return result;
   }

   template<typename T>
   auto make_glitter(
      const double dt,
      const oof::pixel_screen& canvas,
      const T& r,
      std::vector<particle>& glitter
   ) -> void
   {
      // Intensity is used to "fade out" small rockets. They emit fewer particles over time
      double intensity = 1.0;
      if constexpr (std::same_as<T, small_rocket>)
         intensity = std::clamp(1.0 - r.m_age/3.0, 0.0, 1.0);

      constexpr double glitter_per_sec = 50.0;
      if (rng.get_flip(0.5 * glitter_per_sec * intensity * dt) == false)
         return;

      const s9w::dvec2 glitter_pos = get_noised_pos(r.m_pos, r.m_trail_spread);
      const auto& [column, row] = get_column_row(glitter_pos);
      if (column<0 || column > canvas.get_width() - 1 || row<0 || row>canvas.get_halfline_height() - 1)
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

   auto get_new_big_rocket(const oof::pixel_screen& canvas) -> big_rocket
   {
      const double x = rng.get_real(10.0, canvas.get_width() - 10.0);
      const double y = canvas.get_halfline_height();

      constexpr double min_rocket_speed = 20.0;
      constexpr double max_rocket_speed = 55.0;
      const double vx = rng.get_real(-5.0, 5.0);
      const double vy = rng.get_real(-max_rocket_speed, -min_rocket_speed);
      return big_rocket(s9w::dvec2{ x, y }, s9w::dvec2{ vx, vy });
   }

} // namespace {}

auto fireworks_demo() -> void
{
   s9w::ivec2 dim = get_screen_cell_dimensions();
   constexpr double gravity = 20.0;
   oof::pixel_screen canvas(dim[0], 2 * dim[1]);
   timer timer;

   std::vector<small_rocket> small_rockets;
   std::vector<big_rocket> big_rockets;
   std::vector<particle> glitter;
   double time_to_next_rocket = 0.0;
   double dt{};
   while(true)
   {
      // Spawn new rockets
      time_to_next_rocket -= dt;
      if(time_to_next_rocket < 0)
      {
         big_rockets.push_back(get_new_big_rocket(canvas));
         time_to_next_rocket = rng.get_real(1.0, 2.0);
      }

      // Gravity
      for (rocket& r : big_rockets)
         r.m_velocity += s9w::dvec2{ 0.0, gravity } * dt;
      for (rocket& r : small_rockets)
         r.m_velocity += s9w::dvec2{ 0.0, gravity } * dt;

      // Drag only for small rockets
      for (rocket& r : small_rockets){
         constexpr double drag_constant = 0.1;
         const double v = s9w::get_length(r.m_velocity);
         const double F_drag = v * v * dt * drag_constant; // drag ~ v�
         const double speed_change = F_drag / r.m_mass; // F=m*a <=> a=F/m
         r.m_velocity -= s9w::get_normalized(r.m_velocity) * speed_change;
      }

      // Velocity iteration
      for (rocket& r : big_rockets)
         r.m_pos += r.m_velocity * dt;
      for (rocket& r : small_rockets)
         r.m_pos += r.m_velocity * dt;

      // Explode rockets
      for(auto it = std::cbegin(big_rockets); it != std::cend(big_rockets); ){
         if (it->should_explode()) {
            append_moved(small_rockets, get_explosion_rockets(*it, 30));
            it = big_rockets.erase(it);
         }
         else
            ++it;
      }

      // Age things
      for (small_rocket& r : small_rockets)
         r.m_age += dt;
      for (particle& part : glitter)
         part.m_age += dt;

      // Make Glitter
      for (const small_rocket& r : small_rockets)
         make_glitter(dt, canvas, r, glitter);
      for (const big_rocket& r : big_rockets)
         make_glitter(dt, canvas, r, glitter);

      // Remove glowed out small rockets
      remove_from_vector(
         small_rockets,
         [&](const small_rocket& r) { return r.m_age > 3.0; }
      );

      // Cleanup glitter
      remove_from_vector(
         glitter,
         [](const particle& part){ return part.m_age > 1.0; }
      );

      // Draw
      canvas.clear();
      for(particle& part : glitter){
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
         canvas.get_color(part.m_column, part.m_row) = std::bit_cast<oof::color>(glitter_color);
      }

      // Printing, timing; FPS; resizing
      dt = timer.mark_frame();
      fast_print(canvas.get_string());
      const auto fps = timer.get_fps();
      if (fps.has_value()) {
         set_window_title("FPS: " + std::to_string(*fps));

         const s9w::ivec2 new_dim = get_screen_cell_dimensions();
         if (new_dim != dim) {
            fast_print(std::string(oof::clear_screen()));
            canvas = oof::pixel_screen(new_dim[0], 2 * new_dim[1]);
            dim = new_dim;
            glitter.clear();
         }
      }
   }
}
