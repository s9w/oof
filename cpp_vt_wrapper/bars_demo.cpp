#include "bars_demo.h"

#include "../oof.h"
using namespace oof;
#include "tools.h"


namespace {

   auto get_color_component(
      const int segment_index,
      const double x
   ) -> uint8_t
   {
      const double lower = segment_index;
      const double upper = lower + 1.0;
      const double clamped_x = std::clamp(x, lower, upper);
      return 100ui8 + get_int<uint8_t>((clamped_x - lower) * 155.0);
   }

   struct bar_widget{
      int m_bar_start_column{};
      int m_bar_width{};
      mutable screen<std::wstring> m_screen;
      double m_progress{};

      bar_widget(const std::wstring& description, const int bar_width, const int line)
         : m_bar_start_column(static_cast<int>(description.size()) + 2)
         , m_bar_width(bar_width)
         , m_screen(m_bar_start_column + bar_width, 1, 0, line, L'━')
      {
         m_screen.write_into(description+L": ", 0, 0, cell_format{.fg_color=color{255, 100, 100}});
      }

      auto set_value(const double value)
      {
         m_progress = value;
      }

      auto print() const -> void
      {
         for (int i = 0; i < m_bar_width; ++i) {
            const uint8_t component = get_color_component(i, m_progress * m_bar_width);
            m_screen.get_cell(i+m_bar_start_column, 0).m_format.fg_color = color{ component, component, component };
         }
         fast_print(m_screen.get_string());
      }
   };
}


auto bars_demo() -> void{
   const timer timer;
   bar_widget linear(L"Linear", 20, 0);
   bar_widget sine_wave(L"Sine wave", 20, 1);
   
   while (true) {
      const double t = timer.get_seconds_since_start();
      linear.set_value(std::fmod(0.1 * t, 1.0));
      sine_wave.set_value(0.5 + 0.5 * std::sin(t));

      linear.print();
      sine_wave.print();
   }
}
