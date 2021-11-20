#include "text_demo.h"

#include <iostream>

#include "tools.h"

namespace
{

   enum class bold_state{not_bold, bold}; // this is just to avoid std::vector<bool>

   // Capitalized words should be bold
   auto get_bold_state(const std::string& str) -> std::vector<bold_state>
   {
      std::vector<bold_state> bold_states(str.size(), bold_state::not_bold);

      const auto is_looking_for_start = [](const auto start_index) {return start_index == -1; };
      const auto is_start_worthy = [](const char ch) {return std::isupper(ch); };
      const auto is_end_worthy = [](const char ch) {
         if (std::isupper(ch))
            return false;
         return ch != ' ';
      };
      const auto get_end_index = [&](const auto end_index) {return (str[end_index] == ' ') ? (end_index - 1) : end_index; };

      ptrdiff_t bold_start = -1;
      for (int i = 0; i < str.size(); ++i) {
         if (is_looking_for_start(bold_start)){
            if (is_start_worthy(str[i]))
               bold_start = i;
            continue;
         }
         if (is_end_worthy(str[i])) {
            const int end_index = get_end_index(i - 1);
            if ((end_index - bold_start) > 1){
               for (auto j = bold_start; j <= end_index; ++j)
                  bold_states[j] = bold_state::bold;
            }
            bold_start = -1;
         }
      }
      return bold_states;
   }

   struct paragraph_writer{
      std::string m_string;
      std::vector<size_t> m_newline_pos;
      std::vector<bold_state> m_bold_states;
      ranger m_zones;

      paragraph_writer(std::vector<std::string>&& paras)
         : m_zones(-150.0, -25.0, -5.0, 0.0)
      {
         for (std::string& str : paras) {
            m_string += std::move(str);
            m_newline_pos.emplace_back(m_string.size());
         }
         m_bold_states = get_bold_state(m_string);
      }


      auto get_letter_color(
         const int i,
         const int current_index
      ) const -> oof::color
      {
         const double relative_index = i - current_index;
         const std::optional<size_t> zone_index = m_zones.get_zone(relative_index);
         if (zone_index.has_value() == false)
            return oof::color{};
         constexpr auto get_s9w_color = [](const size_t zone_index_v, const double x) {
            constexpr s9w::srgb_u yellow{ 255, 219, 89 };
            constexpr s9w::srgb_u red{ 255, 0, 0 };
            if (zone_index_v == 0) return s9w::mix(s9w::srgb_u{}, yellow, x);
            if (zone_index_v == 2) return s9w::mix(yellow, red, x);
            return yellow;
         };
         return std::bit_cast<oof::color>(get_s9w_color(*zone_index, *m_zones.get_progress(relative_index)));
      }

      auto write(
         oof::screen<std::string>& scr,
         const double seconds
      ) -> void
      {
         const int drawn_letters = std::clamp(static_cast<int>(30.0 * seconds), 0, static_cast<int>(m_string.size()));
         
         int column = 0;
         int line = 0;
         for (int i = 0; i < m_string.size(); ++i)
         {
            scr.get_cell(column, line).m_letter = m_string[i];
            const int format_index = std::clamp(drawn_letters - 1 - i, 0, 4);

            oof::cell_format format{.m_fg_color = get_letter_color(i, drawn_letters)};
            if (m_bold_states[i] == bold_state::bold) {
               format.m_bold = true;
               format.m_underline = true;
            }
            scr.get_cell(column, line).m_format = format;

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
   
   oof::screen scr{ 34, 30, ' ' };
   timer timer;
   while (true) {
      scr.clear();
      writer.write(scr, timer.get_seconds_since_start());

      timer.mark_frame();
      fast_print(scr.get_string());
      const auto fps = timer.get_fps();
      if (fps.has_value())
         set_window_title("FPS: " + std::to_string(*fps));
   }
}
