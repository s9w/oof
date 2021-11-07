#include "cursor_trail_demo.h"

#include <s9w/s9w_geom_alg.h>

#include "../wrapper.h"
using namespace cvtsw;
#include "tools.h"

#include <s9w/s9w_rng.h>
#include <s9w/s9w_geom_types.h>

namespace {

   s9w::rng_state rng;

   auto get_screen_cell_dimensions() -> s9w::ivec2 {
      CONSOLE_SCREEN_BUFFER_INFO csbi;
      GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
      return s9w::ivec2{
         csbi.srWindow.Right - csbi.srWindow.Left + 1,
         csbi.srWindow.Bottom - csbi.srWindow.Top + 1
      };
   }

   auto get_font_width_height() -> s9w::ivec2 {
      CONSOLE_FONT_INFOEX result;
      result.cbSize = sizeof(CONSOLE_FONT_INFOEX);
      GetCurrentConsoleFontEx(GetStdHandle(STD_OUTPUT_HANDLE), false, &result);
      return s9w::ivec2{ result.dwFontSize.X, result.dwFontSize.Y };
   }

   // We're drawing "half" lines, so the relevant font size is halved in height
   auto get_halfline_font_pixel_size() -> s9w::ivec2
   {
      const s9w::ivec2 full_font_size = get_font_width_height();
      return s9w::ivec2{ full_font_size[0], full_font_size[1]/2 };
   }

   auto get_canvas_size() -> s9w::ivec2 {
      const s9w::ivec2 font_pixel_size = get_font_width_height();
      const s9w::ivec2 scree_cell_dimensions = get_screen_cell_dimensions();
      return scree_cell_dimensions * font_pixel_size;
   }

   // Source: https://devblogs.microsoft.com/oldnewthing/20131017-00/?p=2903
   auto get_window_rect() -> RECT{
      RECT window_rect{};
      GetWindowRect(GetConsoleWindow(), &window_rect);

      RECT corrections{};
      const DWORD dwStyle = static_cast<DWORD>(GetWindowLong(GetConsoleWindow(), GWL_STYLE));
      const DWORD dwExStyle = static_cast<DWORD>(GetWindowLong(GetConsoleWindow(), GWL_EXSTYLE));
      constexpr bool fMenu = false;
      AdjustWindowRectEx(&corrections, dwStyle, fMenu, dwExStyle);

      window_rect.left -= corrections.left;
      window_rect.top -= corrections.top;
      window_rect.right -= corrections.right;
      window_rect.bottom -= corrections.bottom;
      return window_rect;
   }

   auto get_cursor_pixel_pos(const RECT& window_rect) -> s9w::ivec2 {
      POINT point;
      GetCursorPos(&point);
      
      return s9w::ivec2{
         static_cast<int>(point.x - window_rect.left),
         static_cast<int>(point.y - window_rect.top)
      };
   }

   // Ideal if: 0.5 -> 0; 0 -> 0.5; -0.5 -> 1.0
   auto get_sdf_intensity(const double sdf) -> double {
      return std::clamp(-sdf + 0.5, 0.0, 1.0);
   }

   auto get_multiplied_color(
      const color& col,
      const double factor
   ) -> color {
      return color{
         get_int<uint8_t>(std::clamp(col.red * factor, 0.0, 255.0)),
         get_int<uint8_t>(std::clamp(col.green * factor, 0.0, 255.0)),
         get_int<uint8_t>(std::clamp(col.blue * factor, 0.0, 255.0))
      };
   }

   auto get_max_color(
      const color& a,
      const color& b
   ) -> color {
      return color{
         std::max(a.red, b.red),
         std::max(a.green, b.green),
         std::max(a.blue, b.blue)
      };
   }

   auto get_faded_color(const color& col, const int fade_amount) -> color{
      return color{
         static_cast<uint8_t>(std::clamp(col.red - fade_amount, 0, 255)),
         static_cast<uint8_t>(std::clamp(col.green - fade_amount, 0, 255)),
         static_cast<uint8_t>(std::clamp(col.blue - fade_amount, 0, 255))
      };
   }
} // namespace {}

auto cursor_trail_demo() -> void
{
   const s9w::ivec2 halfline_font_pixel_size = get_halfline_font_pixel_size();
   const s9w::ivec2 canvas_pixel_size = get_canvas_size();
   const s9w::ivec2 canvas_dimensions = canvas_pixel_size / halfline_font_pixel_size;

   const RECT window_rect = get_window_rect();
   pixel_screen canvas(canvas_dimensions[0], canvas_dimensions[1], 0, 0, color{});
   timer timer;
   color draw_color{ 255, 0, 0 };
   double fade_amount = 0.0;

   while (true) {
      // Fading
      fade_amount += 200.0 * timer.get_dt();
      if (fade_amount >= 1.0) {
         const int effective_amount = static_cast<int>(fade_amount);
         fade_amount -= effective_amount;

         for (color& col : canvas)
            col = get_faded_color(col, effective_amount);
      }

      // Drawing of the circle with SDFs
      const s9w::dvec2 cursor_cell_pos = s9w::dvec2{ get_cursor_pixel_pos(window_rect) } / s9w::dvec2{ halfline_font_pixel_size };
      for (int halfline = 0; halfline < canvas.get_height(); ++halfline) {
         for (int column = 0; column < canvas.get_width(); ++column) {
            const s9w::dvec2 cell_pos = s9w::dvec2{column, halfline} + s9w::dvec2{0.5};

            constexpr double circle_radius = 5.0;
            const double circle_sdf = s9w::get_length(cell_pos - cursor_cell_pos) - circle_radius;

            const color effective_brush_color = get_multiplied_color(draw_color, get_sdf_intensity(circle_sdf));
            canvas.get_color(column, halfline) = get_max_color(canvas.get_color(column, halfline), effective_brush_color);
         }
      }

      // Timing things; FPS; change of color
      timer.mark_frame();
      fast_print(canvas.get_string(color{ 0, 0, 0 }));
      const auto fps = timer.get_fps();
      if (fps.has_value()) {
         set_window_title("FPS: " + std::to_string(*fps));
         draw_color = get_random_color(rng);
      }
   }
}
