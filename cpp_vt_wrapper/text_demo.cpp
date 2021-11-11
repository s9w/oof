#include "text_demo.h"

#include <iostream>

#include "../wrapper.h"
using namespace cvtsw;
#include "tools.h"

namespace
{
   constexpr cell_format format0{ .fg_color = color{255, 0, 0} };
   constexpr cell_format format1{ .fg_color = color{255, 50, 0} };
   constexpr cell_format format2{ .fg_color = color{255, 100, 0} };
   constexpr cell_format format3{ .fg_color = color{255, 150, 0} };
   constexpr cell_format formatN{ .fg_color = color{247, 193, 5 } };
   constexpr cell_format formats[]{ format0, format1, format2, format3, formatN };

   enum class bold_state{not_bold, bold}; // this is just to avoid std::vector<bool>

   // Capitalized words should be bold
   auto get_bold_state(const std::string& str) -> std::vector<bold_state>
   {
      std::vector<bold_state> bold_states;
      bold_states.reserve(str.size());

      // First run: all capitalized letters
      for (const char letter : str)
         bold_states.push_back(std::isupper(letter) ? bold_state::bold : bold_state::not_bold);

      // Then: Eliminate all single letters
      for (int i = 0; i < bold_states.size(); ++i) {
         if(i== bold_states.size()-1)
            continue;
         if (bold_states[i] == bold_state::bold && bold_states[i + 1] == bold_state::not_bold)
            bold_states[i] = bold_state::not_bold;
      }

      return bold_states;
   }


   auto get_dimmed_color(const color& col, const double factor) -> color {
      return color{
         get_int<uint8_t>(factor * col.red),
         get_int<uint8_t>(factor * col.green),
         get_int<uint8_t>(factor * col.blue)
      };
   }

   struct paragraph_writer{
      std::string m_string;
      std::vector<size_t> m_newline_pos;
      std::vector<bold_state> m_bold_states;
      std::vector<double> m_line_intensity;

      paragraph_writer(std::vector<std::string>&& paras){
         for (std::string& str : paras) {
            m_string += std::move(str);
            m_newline_pos.emplace_back(m_string.size());
         }

         m_bold_states = get_bold_state(m_string);
         m_line_intensity = { 1.0 };
      }

      auto write(
         screen<std::string>& scr,
         const double seconds
      ) -> void
      {
         const int drawn_letters = std::clamp(static_cast<int>(30.0 * seconds), 0, static_cast<int>(m_string.size()));
         
         int column = 0;
         int line = 0;
         for (int i = 0; i < drawn_letters; ++i)
         {
            scr.get(column, line).letter = m_string[i];
            const int format_index = std::clamp(drawn_letters - 1 - i, 0, 4);

            cell_format format = formats[format_index];
            if (m_bold_states[i] == bold_state::bold)
               format.m_bold = true;
            format.fg_color = get_dimmed_color(format.fg_color, m_line_intensity[line]);
            scr.get(column, line).m_format = format;

            ++column;
            if(column == scr.get_width())
            {
               column = 0;
               ++line;
            }
            if(std::find(std::cbegin(m_newline_pos), std::cend(m_newline_pos), i+1) != std::cend(m_newline_pos))
            {
               line += 1;
               column = 2;
            }
            if (line >= m_line_intensity.size())
               m_line_intensity.push_back(1.0);
         }

         // Dim all lines but the most recent two
         for (int i = 0; i < m_line_intensity.size(); ++i) {
            if (i >= std::ssize(m_line_intensity) - 2)
               continue;
            m_line_intensity[i] = std::clamp(m_line_intensity[i] - 0.0001, 0.0, 1.0);
         }
      }
   };

}

auto text_demo() -> void
{
   std::vector<std::string> paras;
   paras.emplace_back("It is a period of civil wars in the galaxy. A brave alliance of underground freedom fighters has challenged the tyranny and oppression of the awesome GALACTIC EMPIRE.");
   paras.emplace_back("Striking from a fortress hidden among the billion stars of the galaxy, rebel spaceships have won their first victory in a battle with the powerful Imperial Starfleet. The EMPIRE fears that another defeat could bring a thousand more solar systems into the rebellion, and Imperial control over the galaxy would be lost forever.");
   paras.emplace_back("To crush the rebellion once and for all, the EMPIRE is constructing a sinister new battle station. Powerful enough to destroy an entire planet, its completion spells certain doom for the champions of freedom.");
   paragraph_writer writer(std::move(paras));
   
   screen scr{ 34, 30, 0, 0, ' ' };
   timer timer;
   while (true) {
      for(auto& cell : scr)
      {
         cell.letter = ' ';
         cell.m_format = cell_format{};
      }
      writer.write(scr, timer.get_seconds_since_start());

      timer.mark_frame();
      fast_print(scr.get_string());
      const auto fps = timer.get_fps();
      if (fps.has_value())
         set_window_title("FPS: " + std::to_string(*fps));
   }
}
