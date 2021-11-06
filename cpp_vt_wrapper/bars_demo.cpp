#include "bars_demo.h"

#include <s9w/s9w_rng.h>

#include "../wrapper.h"
using namespace cvtsw;
#include "tools.h"

#include <iostream>

namespace {
   auto get_component(
      const int segment_index,
      const double bar_width
   ) -> uint8_t
   {
      if (segment_index < bar_width)
         return 255ui8;
      else if (segment_index > std::ceil(bar_width))
         return 0ui8;
      else return get_int<uint8_t>(255.0 * std::fmod(bar_width, 1.0));
   }
}

struct pos {
   int column;
   int line;
};
auto get_cursor_pos_xy() -> pos
{
   CONSOLE_SCREEN_BUFFER_INFO cbsi;
   GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cbsi);
   return pos{
      .column = cbsi.dwCursorPosition.X,
      .line = cbsi.dwCursorPosition.Y
   };
}

auto bars_demo() -> void
{
   timer timer;
   std::cout << "Progress: [";
   const pos bar_start = get_cursor_pos_xy();
   screen scr{ 20, 1, bar_start.column, bar_start.line, L'━' };
   std::cout << move_right(scr.get_width()) << "]";
   
   while (true) {
      const double bar_width = std::fmod(10.0 * timer.get_seconds_since_start(), scr.get_width());
      for (int i = 0; i < scr.get_width(); ++i) {
         const uint8_t component = get_component(i, bar_width);
         scr.get(i, 0).m_format.fg_color = color{ component, component, component };
      }

      timer.mark_frame();
      fast_print(scr.get_string());
   }
}
