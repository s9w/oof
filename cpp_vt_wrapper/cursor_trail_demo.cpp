#include "cursor_trail_demo.h"

#include "../wrapper.h"
using namespace cvtsw;
#include "tools.h"

#include <s9w/s9w_rng.h>
#include <s9w/s9w_geom_types.h>

namespace {

   s9w::rng_state rng;

   auto get_columns_rows() -> std::pair<int, int> {
      CONSOLE_SCREEN_BUFFER_INFO csbi;
      int columns, rows;
      GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
      columns = csbi.srWindow.Right - csbi.srWindow.Left + 1;
      rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
      return { columns, rows };
   }

   auto get_font_width_height() -> std::pair<int, int> {
      CONSOLE_FONT_INFOEX result;
      result.cbSize = sizeof(CONSOLE_FONT_INFOEX);
      GetCurrentConsoleFontEx(GetStdHandle(STD_OUTPUT_HANDLE), false, &result);
      return { result.dwFontSize.X, result.dwFontSize.Y };
   }

   auto get_canvas_size() -> std::pair<int, int> {
      const auto& [font_width, font_height] = get_font_width_height();
      const auto& [columns, rows] = get_columns_rows();
      return { columns * font_width, rows * font_height };
   }

   // Source: https://devblogs.microsoft.com/oldnewthing/20131017-00/?p=2903
   auto UnadjustWindowRectEx(LPRECT prc, DWORD dwStyle, BOOL fMenu, DWORD dwExStyle) -> void {
      RECT rc;
      SetRectEmpty(&rc);
      AdjustWindowRectEx(&rc, dwStyle, fMenu, dwExStyle);
      prc->left -= rc.left;
      prc->top -= rc.top;
      prc->right -= rc.right;
      prc->bottom -= rc.bottom;
   }

   auto get_window_rect() -> RECT{
      RECT window_rect;
      HWND const console_window = GetConsoleWindow();
      GetWindowRect(console_window, &window_rect);
      const DWORD dw_style = static_cast<DWORD>(GetWindowLong(console_window, GWL_STYLE));
      const DWORD dw_ex_style = static_cast<DWORD>(GetWindowLong(console_window, GWL_EXSTYLE));
      UnadjustWindowRectEx(&window_rect, dw_style, FALSE, dw_ex_style);
      return window_rect;
   }

   auto get_cursor_pos_xy() -> std::pair<int, int> {
      POINT point;
      GetCursorPos(&point);

      const RECT window_rect = get_window_rect();
      const int x = point.x - window_rect.left;
      const int y = point.y - window_rect.top;
      return { x, y };
   }

   // Ideal if: 0.5 -> 0; 0 -> 0.5; -0.5 -> 1.0
   auto get_sdf_intensity(const double sdf) -> double {
      return std::clamp(-sdf+0.5, 0.0, 1.0);
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
   const auto& [font_width, font_height] = get_font_width_height();
   const auto& [canvas_width, canvas_height] = get_canvas_size();
   const int max_lines = canvas_height / font_height;
   const int max_columns = canvas_width / font_width;

   pixel_screen canvas(max_columns, 2 * max_lines, 0, 0, color{});
   timer timer;
   color draw_color{ 255, 0, 0 };

   double fade_amount = 0.0;
   while (true) {
      const auto cursor_pos = get_cursor_pos_xy();
      const int cursor_column = cursor_pos.first / font_width;
      const int cursor_line = cursor_pos.second / font_height;
      const int cursor_halfline = cursor_pos.second / (font_height/2);

      // Fading
      fade_amount += 200.0 * timer.get_dt();
      if (fade_amount >= 1.0) {
         const int effective_amount = static_cast<int>(fade_amount);
         fade_amount -= effective_amount;

         for (color& col : canvas)
            col = get_faded_color(col, effective_amount);
      }

      const double cursor_x = static_cast<double>(cursor_pos.first) / font_width;
      const double cursor_y = static_cast<double>(cursor_pos.second) / (font_height / 2);
      for (int halfline = 0; halfline < canvas.get_height(); ++halfline) {
         for (int column = 0; column < canvas.get_width(); ++column) {
            const double cell_y = halfline + 0.5;
            const double cell_x = column + 0.5;
            const double dx = cell_x - cursor_x;
            const double dy = cell_y - cursor_y;
            const double dist = std::sqrt(dx * dx + dy * dy);

            constexpr double circle_radius = 5.0;
            const double circle_sdf = dist - circle_radius;

            const color brush_color = get_multiplied_color(draw_color, get_sdf_intensity(circle_sdf));
            canvas.get_color(column, halfline) = get_max_color(canvas.get_color(column, halfline), brush_color);
         }
      }

      timer.mark_frame();
      fast_print(canvas.get_string(color{ 0, 0, 0 }));
      const auto fps = timer.get_fps();
      if (fps.has_value()) {
         set_window_title("FPS: " + std::to_string(*fps));
         draw_color = get_random_color(rng);
      }
   }
}
