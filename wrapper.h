#pragma once

#include <string>
#include <optional>
#include <vector>
#include <variant>

#include <cvt/wbuffer>

#ifdef TRACY_ENABLE
#include <tracy/Tracy.hpp>
#else
#define ZoneScopedN(x)
#endif

namespace cvtsw
{
   template<typename T>
   concept std_string_type = std::same_as<T, std::string> || std::same_as<T, std::wstring>;

   // Feel free to static_cast, reinterpret_cast or memcopy your 3-byte color type into this.
   struct color {
      uint8_t red = 0ui8;
      uint8_t green = 0ui8;
      uint8_t blue = 0ui8;
      friend constexpr auto operator<=>(const color&, const color&) = default;
   };

   struct fg_color_sequence { color m_color; };
   struct bg_color_sequence { color m_color; };
   struct underline_sequence { bool m_underline; };
   struct position_sequence { uint8_t m_line; uint8_t m_column; };
   struct char_sequence { char m_letter; };
   struct wchar_sequence { wchar_t m_letter; };
   struct reset_sequence { };

   template<std_string_type string_type>
   using fitting_char_sequence_t = std::conditional_t<std::is_same_v<string_type, std::string>, char_sequence, wchar_sequence>;

   // TODO maybe sequences type that keeps track of precise reserve amount?

   template<typename T, typename ... Ts>
   concept any_of_c = (std::is_same_v<T, Ts> || ...);

   template<typename T>
   concept sequence_c = any_of_c<T, fg_color_sequence, bg_color_sequence, underline_sequence, position_sequence, char_sequence, wchar_sequence, reset_sequence>;
   using sequence_variant_type = std::variant<fg_color_sequence, bg_color_sequence, underline_sequence, position_sequence, char_sequence, wchar_sequence, reset_sequence>;

   // Generators of sequences
   [[nodiscard]] auto fg_color(const int r, const int g, const int b) -> fg_color_sequence;
   [[nodiscard]] auto fg_color(const color& col) -> fg_color_sequence;
   constexpr size_t   fg_color_max = 3 + 16;

   [[nodiscard]] auto bg_color(const int r, const int g, const int b) -> bg_color_sequence;
   [[nodiscard]] auto bg_color(const color& col) -> bg_color_sequence;
   constexpr size_t   bg_color_max = 3 + 16;
   
   [[nodiscard]] auto underline(const bool new_value = true) -> underline_sequence;
   constexpr size_t   underline_max = 3 + 2;

   [[nodiscard]] auto position(const int line, const int column) -> position_sequence;
   constexpr size_t   position_max = 3 + 2*3+1;

   //[[nodiscard]] auto vposition(const int line) -> detail::vpos_params;
   //constexpr size_t   vposition_max = 3 + 3;

   //[[nodiscard]] auto hposition(const int column) -> detail::hpos_params;
   //constexpr size_t   hposition_max = 3 + 3;

   [[nodiscard]] auto reset_formatting() -> reset_sequence;
   constexpr size_t   reset_max = 3 + 1;

   template<typename stream_type, cvtsw::sequence_c sequence_type>
   auto operator<<(stream_type& os, const sequence_type& sequence)->stream_type&;

   template<cvtsw::std_string_type string_type, cvtsw::sequence_c sequence_type>
   auto write_sequence_into_string(string_type& target, const sequence_type& sequence) -> void;

   template<cvtsw::std_string_type string_type>
   struct cell {
      using char_type = typename string_type::value_type;

      char_type letter{};
      bool underline = false;
      color fg_color;
      color bg_color;

      friend constexpr auto operator<=>(const cell&, const cell&) = default;
   };

   template<cvtsw::std_string_type string_type>
   struct screen{
      using char_type = typename string_type::value_type;

      int m_width = 0;
      int m_height = 0;
      int m_origin_line = 0;
      int m_origin_column = 0;
      std::vector<cell<string_type>> m_cells;
      mutable std::vector<cell<string_type>> m_old_cells;

      explicit screen(const int width, const int height, const int start_column, const int start_line, const char_type fill_char);
      [[nodiscard]] auto get_index(const int column, const int line) const -> size_t;
      [[nodiscard]] auto get(const int column, const int line) -> cell<string_type>&;
      [[nodiscard]] auto is_inside(const int column, const int line) const -> bool;
      [[nodiscard]] auto get_string() const -> string_type;
   };

   // Deduction guide
   template<typename char_type>
   screen(const int width, const int height, const int start_column, const int start_line, const char_type fill_char) -> screen<std::basic_string<char_type>>;


   struct pixel_screen {
      int m_width = 0; // Width is identical between "pixels" and characters
      int m_halfline_height = 0; // This refers to "pixel" height. Height in lines will be half that.
      int m_origin_column = 0;
      int m_origin_halfline = 0;
      std::vector<color> m_pixels;

      explicit pixel_screen(const int width, const int halfline_height, const int start_column, const int start_halfline, const color& fill_color);
      
      [[nodiscard]] auto get_string(const color& frame_color) const->std::wstring;
      // Since pixel screens operate with block characters, this will always return a std::wstring.

      // If you want to override something in the screen
      [[nodiscard]] auto get_screen(const color& frame_color) const -> screen<std::wstring>;

      [[nodiscard]] auto get_color(const int column, const int halfline) const -> const color&;
      [[nodiscard]] auto get_color(const int column, const int halfline)->color&;

   private:
      [[nodiscard]] auto get_line_height() const -> int;
      [[nodiscard]] auto is_in(const int column, const int halfline) const -> bool;
   };

   namespace detail
   {
      template<cvtsw::std_string_type string_type, cvtsw::sequence_c sequence_type>
      auto reserve_string_for_sequence(string_type& target, const sequence_type& sequence) -> void;

      template<cvtsw::sequence_c sequence_type>
      [[nodiscard]] constexpr auto get_sequence_string_size(const sequence_type& sequence) -> size_t;

      // std::to_string() or std::to_wstring() depending on the template type
      template<cvtsw::std_string_type string_type>
      auto write_int_to_string(string_type& target, const int value) -> void;

      struct cell_pos {
         int m_index = 0;
         int m_width = 0;
         int m_height = 0;

         explicit constexpr cell_pos(const int width, const int height)
            : m_width(width)
            , m_height(height)
         {}
         [[nodiscard]] constexpr auto get_column() const -> int {
            return m_index % m_width;
         }
         [[nodiscard]] constexpr auto get_line() const -> int {
            return m_index / m_width;
         }
         [[nodiscard]] constexpr auto is_end() const -> bool {
            return m_index >= (m_width * m_height);
         }
         constexpr cell_pos operator+(const int jump_amount) const {
            cell_pos jumped_pos(m_width, m_height);
            jumped_pos.m_index = m_index + jump_amount;
            return jumped_pos;
         }
         constexpr cell_pos& operator++() {
            ++m_index;
            return *this;
         }
         friend constexpr auto operator<=>(const cell_pos&, const cell_pos&) = default;
      };


      template<cvtsw::std_string_type string_type>
      struct draw_state{
         using cell_type = cell<string_type>;

         std::vector<sequence_variant_type>& m_target_sequences;
         cell_pos m_last_written_pos;

         std::optional<bool> m_underline;
         std::optional<color> m_fg_color;
         std::optional<color> m_bg_color;
         
         explicit draw_state(std::vector<sequence_variant_type>& target_sequence, const int width, const int height)
            : m_target_sequences(target_sequence)
            , m_last_written_pos(width, height)
         {}

         auto take_complete_cell_state(const cell_type& cell) {
            m_underline.emplace(cell.underline);
            m_fg_color.emplace(cell.fg_color);
            m_bg_color.emplace(cell.bg_color);
         }

         auto write_sequence(
            const cell_type& target_cell_state,
            const std::optional<std::reference_wrapper<const cell_type>>& old_cell_state,
            const cell_pos& target_pos,
            const int origin_line,
            const int origin_column
         ) -> size_t
         {
            size_t cell_reserve_size = 0;
            if (target_cell_state == old_cell_state)
               return cell_reserve_size;

            // Apply differences between console state and the target state
            if (target_cell_state.fg_color != m_fg_color)
            {
               m_target_sequences.push_back(fg_color_sequence{ target_cell_state.fg_color });
               cell_reserve_size += fg_color_max;
            }
            if (target_cell_state.bg_color != m_bg_color)
            {
               m_target_sequences.push_back(bg_color_sequence{ target_cell_state.bg_color });
               cell_reserve_size += bg_color_max;
            }
            if (target_cell_state.underline != m_underline)
            {
               m_target_sequences.push_back(underline_sequence{ target_cell_state.underline });
               cell_reserve_size += underline_max;
            }

            if (target_pos != (m_last_written_pos + 1) || target_pos.get_column() == 0) {
               m_target_sequences.push_back(
                  position_sequence{
                     static_cast<uint8_t>(target_pos.get_line() + origin_line),
                     static_cast<uint8_t>(target_pos.get_column() + origin_column)
                  }
               );
               cell_reserve_size += position_max;
            }
            if constexpr (std::is_same_v<string_type, std::string>)
               m_target_sequences.push_back(char_sequence{ target_cell_state.letter });
            else
               m_target_sequences.push_back(wchar_sequence{ target_cell_state.letter });
            cell_reserve_size += 1;
            m_last_written_pos = target_pos;
            take_complete_cell_state(target_cell_state);

            return cell_reserve_size;
         }
      };


      template<cvtsw::std_string_type string_type, typename T>
      auto write_ints_into_string(
         string_type& target,
         const T& last
      ) -> void
      {
         detail::write_int_to_string(target, last);;
      }


      template<cvtsw::std_string_type string_type, typename T, typename ... Ts>
      auto write_ints_into_string(
         string_type& target,
         const T& first,
         const Ts&... rest
      ) -> void
      {
         detail::write_int_to_string(target, first);

         using char_type = typename string_type::value_type;
         target += static_cast<char_type>(';');

         write_ints_into_string(target, rest...);
      }

   } // namespace cvtsw::detail

} // namespace cvtsw


template<cvtsw::std_string_type string_type, cvtsw::sequence_c sequence_type>
auto cvtsw::write_sequence_into_string(
   string_type& target,
   const sequence_type& sequence
) -> void
{
   detail::reserve_string_for_sequence(target, sequence);

   using char_type = typename string_type::value_type;
   //static_assert(); // TODO char type

   if constexpr (std::is_same_v<sequence_type, fitting_char_sequence_t<string_type>>)
   {
      target += sequence.m_letter;
      return;
   }
   else
   {
      
      if constexpr (std::same_as<string_type, std::string>)
         target += "\x1b[";
      else
         target += L"\x1b[";

      if constexpr (std::is_same_v<sequence_type, fg_color_sequence>)
      {
         detail::write_ints_into_string(target, 38, 2, sequence.m_color.red, sequence.m_color.green, sequence.m_color.blue);
         target += static_cast<char_type>('m');
      }
      else if constexpr (std::is_same_v<sequence_type, bg_color_sequence>)
      {
         detail::write_ints_into_string(target, 48, 2, sequence.m_color.red, sequence.m_color.green, sequence.m_color.blue);
         target += static_cast<char_type>('m');
      }
      else if constexpr (std::is_same_v<sequence_type, underline_sequence>)
      {
         detail::write_ints_into_string(target, sequence.m_underline ? 4 : 24);
         target += static_cast<char_type>('m');
      }
      else if constexpr (std::is_same_v<sequence_type, position_sequence>)
      {
         detail::write_ints_into_string(target, sequence.m_line, sequence.m_column);
         target += static_cast<char_type>('H');
      }
      else if constexpr (std::is_same_v<sequence_type, reset_sequence>)
      {
         detail::write_ints_into_string(target, 0);
         target += static_cast<char_type>('m');
      }
   }
}


template<cvtsw::sequence_c sequence_type>
[[nodiscard]] constexpr auto cvtsw::detail::get_sequence_string_size(const sequence_type& sequence) -> size_t
{
   if constexpr (std::is_same_v<sequence_type, char_sequence> || std::is_same_v<sequence_type, wchar_sequence>) {
      return 1;
   }
   else {
      constexpr auto get_int_param_str_length = [](const int param) {
         if (param < 10)
            return 1;
         if (param < 100)
            return 2;
         return 3;
      };

      size_t reserve_size = 0;
      constexpr int semicolon_size = 1;
      if constexpr (std::is_same_v<sequence_type, fg_color_sequence> || std::is_same_v<sequence_type, fg_color_sequence>) {
         reserve_size += 2 + semicolon_size + 1 +
            semicolon_size + get_int_param_str_length(sequence.m_color.red) +
            semicolon_size + get_int_param_str_length(sequence.m_color.green) +
            semicolon_size + get_int_param_str_length(sequence.m_color.blue);
      }
      else if constexpr (std::is_same_v<sequence_type, underline_sequence>)
      {
         reserve_size += sequence.m_underline ? 1 : 2;
      }
      else if constexpr (std::is_same_v<sequence_type, position_sequence>)
      {
         reserve_size += get_int_param_str_length(sequence.m_line) + semicolon_size + get_int_param_str_length(sequence.m_column);
      }
      else if constexpr (std::is_same_v<sequence_type, reset_sequence>)
      {
         reserve_size += 1;
      }

      //reserve_size += n-1; // semicolons
      reserve_size += 3; // 2 intro, 1 outro
      return reserve_size;
   }
}


template<cvtsw::std_string_type string_type>
auto cvtsw::detail::write_int_to_string(string_type& target, const int value) -> void
{
   using char_type = typename string_type::value_type;
   const int hundreds = value / 100;
   if (value >= 100)
      target += static_cast<char_type>('0' + hundreds);
   if (value >= 10)
      target += static_cast<char_type>('0' + (value%100)/10);
   target += '0' + value % 10;
}


template<cvtsw::std_string_type string_type, cvtsw::sequence_c sequence_type>
auto cvtsw::detail::reserve_string_for_sequence(
   string_type& target,
   const sequence_type& sequence
) -> void
{
   const size_t size_neede = get_sequence_string_size(sequence);

   const size_t capacity_left = target.capacity() - target.size();
   if (capacity_left < size_neede)
   {
      target.reserve(target.capacity() + size_neede);
   }
}


template<typename stream_type, cvtsw::sequence_c sequence_type>
auto cvtsw::operator<<(stream_type& os, const sequence_type& sequence) -> stream_type&
{
   using char_type = typename stream_type::char_type;
   using string_type = std::basic_string<char_type>;
   string_type temp;
   detail::reserve_string_for_sequence(temp, sequence);
   write_sequence_into_string(temp, sequence);
   os << temp;
   return os;
}


template<cvtsw::std_string_type string_type>
cvtsw::screen<string_type>::screen(
   const int width, const int height,
   const int start_column, const int start_line,
   const char_type fill_char
)
   : m_width(width)
   , m_height(height)
   , m_origin_line(start_line)
   , m_origin_column(start_column)
   , m_cells(m_width * m_height, cell<string_type>{fill_char})
{
   
}


template<cvtsw::std_string_type string_type>
auto cvtsw::screen<string_type>::get_string() const -> string_type
{
   std::vector<sequence_variant_type> sequences;
   sequences.reserve(this->m_width * this->m_height * 10);
   size_t reserve_size = 0;

   detail::draw_state<string_type> state{ sequences, this->m_width, this->m_height };

   {
      ZoneScopedN("sequences");
      sequences.push_back(reset_sequence{});
      for (detail::cell_pos relative_pos{ this->m_width, this->m_height }; relative_pos.is_end() == false; ++relative_pos)
      {
         const cell<string_type>& target_cell_state = this->m_cells[relative_pos.m_index];

         std::optional<std::reference_wrapper<const cell<string_type>>> old_cell_state;
         if (this->m_old_cells.empty() == false)
            old_cell_state.emplace(this->m_old_cells[relative_pos.m_index]);

         reserve_size += state.write_sequence(
            target_cell_state, old_cell_state,
            relative_pos,
            this->m_origin_line, this->m_origin_column
         );
      }
   }
   string_type result_str;
   {
      ZoneScopedN("sequence to string");
      result_str.reserve(reserve_size);
      for (const sequence_variant_type& sequence : sequences) {
         std::visit([&](const auto& alternative) { write_sequence_into_string(result_str, alternative);  }, sequence);
      }
   }

   m_old_cells = m_cells;
   return result_str;
}


template<cvtsw::std_string_type string_type>
auto cvtsw::screen<string_type>::get_index(const int column, const int line) const -> size_t
{
   return line * m_width + column;
}



template<cvtsw::std_string_type string_type>
auto cvtsw::screen<string_type>::get(const int column, const int line) -> cell<string_type>&
{
   return m_cells[get_index(column, line)];
}



template<cvtsw::std_string_type string_type>
auto cvtsw::screen<string_type>::is_inside(const int column, const int line) const -> bool
{
   return column >= 0 && column < m_width && line >= 0 && line < m_height;
}


#ifdef CM_IMPL
auto cvtsw::position(const int line, const int column) -> position_sequence {
   const int effective_line = line + 1;
   const int effective_column = column + 1;
   return position_sequence{ static_cast<uint8_t>(effective_line), static_cast<uint8_t>(effective_column) };
}


//auto cvtsw::vposition(const int line)-> detail::vpos_params{
//   const int effective_line = line + 1;
//   return detail::vpos_params{ effective_line };
//}
//
//auto cvtsw::hposition(const int column) -> detail::hpos_params{
//   const int effective_column = column + 1;
//   return detail::hpos_params{ effective_column };
//}


auto cvtsw::fg_color(const int r, const int g, const int b) -> fg_color_sequence {
   return fg_color_sequence{
      color{
         static_cast<uint8_t>(r),
         static_cast<uint8_t>(g),
         static_cast<uint8_t>(b)
      }
   };
}


auto cvtsw::fg_color(const color& col) -> fg_color_sequence {
   return fg_color_sequence{ col };
}


auto cvtsw::bg_color(const int r, const int g, const int b) -> bg_color_sequence {
   return bg_color_sequence{
      color{
         static_cast<uint8_t>(r),
         static_cast<uint8_t>(g),
         static_cast<uint8_t>(b)
      }
   };
}


auto cvtsw::bg_color(const color& col) -> bg_color_sequence {
   return bg_color_sequence{ col };
}


auto cvtsw::underline(const bool new_value) -> underline_sequence {
   return underline_sequence{ new_value };
}


auto cvtsw::reset_formatting() -> reset_sequence {
   return reset_sequence{};
}


cvtsw::pixel_screen::pixel_screen(
   const int width,
   const int halfline_height,
   const int start_column,
   const int start_halfline,
   const color& fill_color
)
   : m_width(width)
   , m_halfline_height(halfline_height)
   , m_origin_column(start_column)
   , m_origin_halfline(start_halfline)
   , m_pixels(m_width* m_halfline_height, fill_color)
{

}


auto cvtsw::pixel_screen::get_screen(const color& frame_color) const -> screen<std::wstring>
{
   screen<std::wstring> result{ m_width, get_line_height(), m_origin_column, m_origin_halfline / 2, L'▀' };
   // This means fg color is on top

   int halfline_top = (m_origin_halfline % 2 == 0) ? 0 : -1;
   int halfline_bottom = halfline_top + 1;
   for (int line = 0; line < result.m_height; ++line) {
      for (int column = 0; column < result.m_width; ++column) {
         cell<std::wstring>& target_cell = result.get(column, line);
         target_cell.fg_color = is_in(column, halfline_top) ? get_color(column, halfline_top) : frame_color;
         target_cell.bg_color = is_in(column, halfline_bottom) ? get_color(column, halfline_bottom) : frame_color;
      }
      halfline_top += 2;
      halfline_bottom += 2;
   }
   return result;
}

auto cvtsw::pixel_screen::get_string(const color& frame_color) const -> std::wstring
{
   return get_screen(frame_color).get_string();
}


auto cvtsw::pixel_screen::get_line_height() const -> int
{
   const int first_line = m_origin_halfline / 2;
   const int last_line = (m_origin_halfline - 1 + m_halfline_height) / 2;
   return last_line - first_line + 1;
}


auto cvtsw::pixel_screen::is_in(const int column, const int halfline) const -> bool
{
   const int index = halfline * m_width + column;
   const bool is_out = index < 0 || index >(m_pixels.size() - 1);
   return !is_out;
}


auto cvtsw::pixel_screen::get_color(
   const int column,
   const int halfline
) const -> const color&
{
   const int index = halfline * m_width + column;
   return m_pixels[index];
}


auto cvtsw::pixel_screen::get_color(
   const int column,
   const int halfline
) -> color&
{
   const int index = halfline * m_width + column;
   return m_pixels[index];
}
#endif
