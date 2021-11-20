#include "snow_demo.h"

#include "tools.h"


namespace
{
   s9w::rng_state rng{static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count())};

   struct flake_pos{
      double m_column{};
      double m_row{};
      auto get_indices() const -> std::pair<int, int>{
         return { get_int(m_column - 0.5) , get_int(m_row - 0.5) };
      }
   };

   struct snowflake{
      flake_pos m_pos;
      double m_frontality;
      bool m_static = false;
   };

   enum class stick_state{no_stick, can_stick };
}

auto draw_partial_snowflake(
   const double brightness,
   const double ratio,
   oof::pixel_screen& px,
   const int column,
   const int row
) -> void
{
   if (px.is_in(column, row) == false)
      return;
   const uint8_t component = get_int<uint8_t>(brightness);
   const uint8_t alpha = get_int<uint8_t>(ratio * 255.0);
   const s9w::srgba_u snow_color{ component, component, component, alpha };

   oof::color& target = px.get_color(column, row);
   const s9w::srgb_u blend_result = s9w::blend(std::bit_cast<s9w::srgb_u>(target), snow_color);
   target = std::bit_cast<oof::color>(blend_result);
}


auto draw_snowflake(
   const snowflake& flake,
   oof::pixel_screen& px
) -> void
{
   constexpr int min_brightness = 50;
   constexpr int delta_brightness = 255 - min_brightness;
   const double target_brightness = min_brightness + delta_brightness * flake.m_frontality;

   const int column = get_int(flake.m_pos.m_column - 0.5);
   const int upper_row = get_int(flake.m_pos.m_row - 0.5);
   const double second_ratio = std::fmod(flake.m_pos.m_row, 1.0);

   draw_partial_snowflake(target_brightness, 1.0-second_ratio, px, column, upper_row);
   draw_partial_snowflake(target_brightness, second_ratio, px, column, upper_row+1);
}


auto snow_demo() -> void
{
   //const int width = get_screen_cell_dimensions()[0];
    constexpr int width = 60;
   const int height = 2 * get_screen_cell_dimensions()[1];
   constexpr double max_speed = 20.0;
   double dt{};
   oof::pixel_screen px{ width, height };

   std::vector<stick_state> neigh(width*height, stick_state::no_stick);
   for(int column=0; column<width; ++column)
   {
      const int row = height - 1;
      const int index = row * width + column;
      neigh[index] = stick_state::can_stick;
   }
   std::vector<snowflake> snowflakes;

   timer timer;
   while (true) {
      // draw bg
      for(int row=0; row<height; ++row){
         for (int column = 0; column < width; ++column){
            const double height_progress = 1.0 * row / (height - 1);
            const oof::color bg_color{
               get_int<uint8_t>(30*height_progress),
               get_int<uint8_t>(30*height_progress),
               get_int<uint8_t>(100*height_progress)
            };
            px.get_color(column, row) = bg_color;
         }
      }

      // draw snow
      for(const snowflake& flake : snowflakes)
         draw_snowflake(flake, px);

      // move snow
      for (snowflake& flake : snowflakes)
      {
         if (flake.m_static)
            continue;
         flake.m_pos.m_row += max_speed * flake.m_frontality * dt;
      }

      // sticking
      for (snowflake& flake : snowflakes)
      {
         if(flake.m_frontality<0.5)
            continue;
         const int column = get_int(flake.m_pos.m_column - 0.5);
         const int row = get_int(flake.m_pos.m_row - 0.5);
         if(row<=0)
            continue;
         const int index = row * width + column;
         const bool lower_left_ok = column == 0 || neigh[index-1] == stick_state::can_stick;
         const bool lower_right_ok = column == (width-1) || neigh[index+1] == stick_state::can_stick;
         if (neigh[index] == stick_state::can_stick && lower_left_ok && lower_right_ok)
         {
            flake.m_static = true;
            neigh[index - width] = stick_state::can_stick;
         }
      }

      // New snow
      if(rng.get_flip(0.5 * px.get_width() * dt)){
         snowflakes.push_back(
            snowflake{
               .m_pos = flake_pos{
                  rng.get_real(0.5, width-0.5),
                  -1.0
               },
               .m_frontality = rng.get_real(0.2, 1.0)
            }
         );
      }

      // Remove flakes that run out of the bottom (for performance)
      remove_from_vector(
         snowflakes,
         [&](const snowflake& flake){ return flake.m_pos.m_row > height; }
      );

      // Remove flakes that touch a sticking spot but couldn't stick (so they don't fly over the sticky snow)
      remove_from_vector(
         snowflakes,
         [&](const snowflake& flake) {
            if (flake.m_static)
               return false;
            const auto& [column, row] = flake.m_pos.get_indices();
            if (row < 0)
               return false;
            const int index = row * width + column;
            return neigh[index] == stick_state::can_stick;
         }
      );

      dt = timer.mark_frame();
      fast_print(px.get_string());
      if (const auto fps = timer.get_fps(); fps.has_value())
         set_window_title("FPS: " + std::to_string(*fps));
   }
}
