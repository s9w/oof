#include "snow_demo.h"

#include "../wrapper.h"
using namespace cvtsw;
#include "tools.h"


static s9w::rng_state rng{2};



namespace
{
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

auto snow_demo() -> void
{
   constexpr int width = 40;
   constexpr int height = 60;
   constexpr double max_speed = 20.0;
   pixel_screen px{ width, height, 0, 0, color{0, 0, 0} };

   std::vector<stick_state> neigh(width*height, stick_state::no_stick);
   for(int column=0; column<width; ++column)
   {
      constexpr int row = height - 1;
      const int index = row * width + column;
      neigh[index] = stick_state::can_stick;
   }
   std::vector<snowflake> snowflakes;
   snowflakes.push_back(
      snowflake{
         .m_pos = flake_pos{
            rng.get_real(0.0, width - 1.0),
            -1.0
         },
         .m_frontality = 1.0
      }
   );

   timer timer;
   while (true) {
      // draw bg
      for(int row=0; row<height; ++row){
         for (int column = 0; column < width; ++column){
            const double height_progress = 1.0 * row / (height - 1);
            const color bg_color{
               get_int<uint8_t>(30*height_progress),
               get_int<uint8_t>(30*height_progress),
               get_int<uint8_t>(100*height_progress)
            };
            px.get_color(column, row) = bg_color;
         }
      }

      // draw snow
      for(const snowflake& flake : snowflakes){
         const auto& [column, row] = flake.m_pos.get_indices();
         if (px.is_in(column, row)) {
            constexpr int min_brightness = 50;
            constexpr int delta_brightness = 255 - min_brightness;
            const uint8_t value = get_int<uint8_t>(min_brightness + delta_brightness * flake.m_frontality);
            px.get_color(column, row) = color{ value, value, value };
         }
      }

      // move snow
      for (snowflake& flake : snowflakes)
      {
         if (flake.m_static)
            continue;
         flake.m_pos.m_row += max_speed * flake.m_frontality * timer.get_dt();
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
      if(rng.get_flip(20.0 * timer.get_dt())){
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
         [](const snowflake& flake){ return flake.m_pos.m_row > height; }
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

      timer.mark_frame();
      fast_print(px.get_string());
      if (const auto fps = timer.get_fps(); fps.has_value())
         set_window_title("FPS: " + std::to_string(*fps));
   }
}
