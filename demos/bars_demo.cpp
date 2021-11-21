#include "bars_demo.h"

#include <algorithm>
#include <chrono>

#include "../oof.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h> // for WriteConsole


namespace {

   auto fast_print(const std::wstring& sss) -> void {
      HANDLE const output_handle = GetStdHandle(STD_OUTPUT_HANDLE);
      const auto char_count = static_cast<DWORD>(sss.length());
      WriteConsoleW(output_handle, sss.c_str(), char_count, nullptr, nullptr);
   }


   auto get_color_component(
      const int segment_index,
      const double x
   ) -> double
   {
      const double lower = segment_index;
      const double upper = lower + 1.0;
      const double clamped_x = std::clamp(x, lower, upper);
      return 100.0 + std::round((clamped_x - lower) * 155.0);
   }


   struct bar_widget{
      double m_bar_value{};

   private:
      int m_bar_start_column{};
      int m_bar_width{};
      mutable oof::screen<std::wstring> m_screen;
      
   public:
      bar_widget(const std::wstring& description, const int bar_width, const int line)
         : m_bar_start_column(static_cast<int>(description.size()) + 2)
         , m_bar_width(bar_width)
         , m_screen(m_bar_start_column + bar_width, 1, 1, line, L'━')
      {
         m_screen.write_into(description+L": ", 0, 0, oof::cell_format{.m_fg_color={255, 100, 100}});
      }

      auto print() const -> void
      {
         for (int i = 0; i < m_bar_width; ++i) {
            oof::color col{ static_cast<uint8_t>(get_color_component(i, m_bar_value)) };
            m_screen.get_cell(i+m_bar_start_column, 0).m_format.m_fg_color = col;
         }
         fast_print(m_screen.get_string());
      }
   };
}


auto bars_demo() -> void{
   constexpr int bar_length = 20;
   bar_widget linear(L"Linear", bar_length, 1);
   bar_widget sine_wave(L"Sine wave", bar_length, 2);
   
   const auto t0 = std::chrono::high_resolution_clock::now();
   while (true) {
      const std::chrono::duration<double> double_seconds = std::chrono::high_resolution_clock::now() - t0;
      const double t = double_seconds.count();

      linear.m_bar_value = std::fmod(2.0 * t, bar_length);
      sine_wave.m_bar_value = 10.0 + 10.0 * std::sin(t);

      linear.print();
      sine_wave.print();
   }
}
