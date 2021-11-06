#include "cursor_trail_demo.h"

#include "../wrapper.h"
using namespace cvtsw;
#include "tools.h"

#include <s9w/s9w_rng.h>

#include <iostream>

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
   bool UnadjustWindowRectEx(LPRECT prc, DWORD dwStyle, BOOL fMenu, DWORD dwExStyle) {
      RECT rc;
      SetRectEmpty(&rc);
      BOOL fRc = AdjustWindowRectEx(&rc, dwStyle, fMenu, dwExStyle);
      if (fRc) {
         prc->left -= rc.left;
         prc->top -= rc.top;
         prc->right -= rc.right;
         prc->bottom -= rc.bottom;
      }
      return fRc;
   }

   auto get_cursor_pos_xy() -> std::pair<int, int> {
      POINT point;
      GetCursorPos(&point);

      RECT window_rect;
      const HWND console_window = GetConsoleWindow();
      GetWindowRect(console_window, &window_rect);
      DWORD dwStyle = (DWORD)GetWindowLong(console_window, GWL_STYLE);
      DWORD dwExStyle = (DWORD)GetWindowLong(console_window, GWL_EXSTYLE);
      UnadjustWindowRectEx(&window_rect, dwStyle, FALSE, dwExStyle);

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
         std::clamp(get_int<uint8_t>(col.red * factor), 0ui8, 255ui8),
         std::clamp(get_int<uint8_t>(col.green * factor), 0ui8, 255ui8),
         std::clamp(get_int<uint8_t>(col.blue * factor), 0ui8, 255ui8)
      };
   }
}

auto cursor_trail_demo() -> void
{
   const auto& [font_width, font_height] = get_font_width_height();
   const auto& [canvas_width, canvas_height] = get_canvas_size();
   const int max_lines = canvas_height / font_height;
   const int max_columns = canvas_width / font_width;

   pixel_screen px(max_columns, 2 * max_lines, 0, 0, color{});
   timer timer;

   constexpr auto get_faded_color = [](const color& col, const int fade_amount) {
      return color{
         static_cast<uint8_t>(std::clamp(col.red - fade_amount, 0, 255)),
         static_cast<uint8_t>(std::clamp(col.green - fade_amount, 0, 255)),
         static_cast<uint8_t>(std::clamp(col.blue - fade_amount, 0, 255))
      };
   };

   color draw_color{ 255, 0, 0 };

   double fade_amount = 0.0;
   while (true) {
      const auto cursor_pos = get_cursor_pos_xy();
      const int cursor_column = cursor_pos.first / font_width;
      const int cursor_line = cursor_pos.second / font_height;
      const int cursor_halfline = cursor_pos.second / (font_height/2);

      // Fading
      fade_amount += 50.0 * timer.get_dt();
      if (fade_amount >= 1.0) {
         const int effective_amount = static_cast<int>(fade_amount);
         fade_amount -= effective_amount;

         for (auto& col : px)
            col = get_faded_color(col, effective_amount);
      }

      // Don't progress if cursor is out of window to prevent math explosions
      if (cursor_column >= 0 && cursor_column < max_columns && cursor_halfline >= 0 && cursor_line < max_lines)
      {
         const double cursor_x = static_cast<double>(cursor_pos.first) / font_width;
         const double cursor_y = static_cast<double>(cursor_pos.second) / (font_height / 2);
         for (int halfline = 0; halfline < px.get_height(); ++halfline) {
            for (int column = 0; column < px.get_width(); ++column) {
               const double cell_y = halfline + 0.5;
               const double cell_x = column + 0.5;
               const double dx = cell_x - cursor_x;
               const double dy = cell_y - cursor_y;
               const double dist = std::sqrt(dx * dx + dy * dy);

               constexpr double circle_radius = 9.0;
               const double ring_width = 2.0 + std::sin(3.0 * timer.get_seconds_since_start());
               const double circle_sdf = dist - circle_radius;
               const double ring_sdf = std::abs(circle_sdf) - ring_width;

               px.get_color(column, halfline) = get_multiplied_color(draw_color, get_sdf_intensity(ring_sdf));
            }
         }
      }

      timer.mark_frame();
      fast_print(px.get_string(color{ 0, 0, 0 }));
      const auto fps = timer.get_fps();
      if (fps.has_value()) {
         set_window_title("FPS: " + std::to_string(*fps));
         draw_color = get_random_color(rng);
      }
   }
}
