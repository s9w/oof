#pragma once

#include <string>
#include <optional>
#include <vector>
#include <variant>

#ifdef TRACY_ENABLE
#include <tracy/Tracy.hpp>
#else
#define ZoneScoped
#define ZoneScopedN(x)
#endif

namespace cvtsw
{
   template<typename T>
   concept std_string_type = std::same_as<T, std::string> || std::same_as<T, std::wstring>;

   // Feel free to bit_cast, reinterpret_cast or memcopy your 3-byte color type into this.
   struct color {
      uint8_t red = 0ui8;
      uint8_t green = 0ui8;
      uint8_t blue = 0ui8;
      friend constexpr auto operator<=>(const color&, const color&) = default;
   };

struct fg_rgb_color_sequence;
struct fg_index_color_sequence;
struct set_index_color_sequence;
struct bg_rgb_color_sequence;
struct bg_index_color_sequence;
struct underline_sequence;
struct bold_sequence;
struct position_sequence;
struct move_left_sequence;
struct move_right_sequence;
struct move_up_sequence;
struct move_down_sequence;
struct char_sequence;
struct wchar_sequence;
struct reset_sequence;

   template<std_string_type string_type>
   using fitting_char_sequence_t = std::conditional_t<std::is_same_v<string_type, std::string>, char_sequence, wchar_sequence>;

   // TODO maybe sequences type that keeps track of precise reserve amount?


   template<typename T, typename variant_type>
   struct is_alternative : std::false_type {};

   template<typename T, typename ... variant_alternatives>
   struct is_alternative<T, std::variant<variant_alternatives...>>
      : std::disjunction<std::is_same<T, variant_alternatives>...> {};

   template<typename T, typename variant_type>
   constexpr bool is_alternative_v = is_alternative<T, variant_type>::value;

   using sequence_variant_type = std::variant<
      fg_rgb_color_sequence, fg_index_color_sequence, bg_index_color_sequence, bg_rgb_color_sequence, set_index_color_sequence,
      underline_sequence, bold_sequence, position_sequence, char_sequence, wchar_sequence, reset_sequence,
      move_left_sequence, move_right_sequence, move_up_sequence, move_down_sequence
   >;

   template<typename T>
   concept sequence_c = is_alternative_v<T, sequence_variant_type>;


   // Generators of sequences
   [[nodiscard]] auto fg_color(const int r, const int g, const int b) -> fg_rgb_color_sequence;
   [[nodiscard]] auto fg_color(const color& col) -> fg_rgb_color_sequence;
   [[nodiscard]] auto fg_color(const int index) -> fg_index_color_sequence;

   [[nodiscard]] auto bg_color(const int r, const int g, const int b) -> bg_rgb_color_sequence;
   [[nodiscard]] auto bg_color(const color& col) -> bg_rgb_color_sequence;
   [[nodiscard]] auto bg_color(const int index) -> bg_index_color_sequence;

   [[nodiscard]] auto set_index_color(const int index, const color& col) -> set_index_color_sequence;
   
   [[nodiscard]] auto underline(const bool new_value = true) -> underline_sequence;
   [[nodiscard]] auto bold(const bool new_value = true) -> bold_sequence;
   [[nodiscard]] auto reset_formatting() -> reset_sequence;

   [[nodiscard]] auto position(const int line, const int column) -> position_sequence;
   [[nodiscard]] auto move_left(const int amount) -> move_left_sequence;
   [[nodiscard]] auto move_right(const int amount) -> move_right_sequence;
   [[nodiscard]] auto move_up(const int amount) -> move_up_sequence;
   [[nodiscard]] auto move_down(const int amount) -> move_down_sequence;
   //[[nodiscard]] auto vposition(const int line) -> detail::vpos_params;
   //[[nodiscard]] auto hposition(const int column) -> detail::hpos_params;

   template<typename stream_type, cvtsw::sequence_c sequence_type>
   auto operator<<(stream_type& os, const sequence_type& sequence) -> stream_type&;

   template<cvtsw::std_string_type string_type, cvtsw::sequence_c sequence_type>
   auto write_sequence_into_string(string_type& target, const sequence_type& sequence) -> void;

   template<cvtsw::std_string_type string_type>
   [[nodiscard]] auto get_string_from_sequences(const std::vector<sequence_variant_type>& sequences) -> string_type;


   struct cell_format {
      bool m_underline = false;
      bool m_bold = false;
      color fg_color{255, 255, 255};
      color bg_color{0, 0, 0};
      friend constexpr auto operator<=>(const cell_format&, const cell_format&) = default;
   };

   template<cvtsw::std_string_type string_type>
   struct cell {
      using char_type = typename string_type::value_type;

      char_type letter{};
      cell_format m_format;
      friend constexpr auto operator<=>(const cell&, const cell&) = default;
   };

   template<cvtsw::std_string_type string_type>
   struct screen{
      using char_type = typename string_type::value_type;

      std::vector<cell<string_type>> m_cells;

   private:
      int m_width = 0;
      int m_height = 0;
      int m_origin_line = 0;
      int m_origin_column = 0;
      mutable std::vector<cell<string_type>> m_old_cells;
      mutable std::vector<sequence_variant_type> m_sequence_buffer;

   public:
      explicit screen(
         const int width, const int height,
         const int start_column, const int start_line, 
         const char_type fill_char
      );

      [[nodiscard]] auto get_width() const -> int;
      [[nodiscard]] auto get_height() const -> int;
      
      [[nodiscard]] auto get(const int column, const int line) -> cell<string_type>&;
      [[nodiscard]] auto is_inside(const int column, const int line) const -> bool;
      [[nodiscard]] auto get_string() const -> string_type;
      auto write_into(const string_type& text, const int column, const int line, const cell_format& formatting) -> void;

      [[nodiscard]] auto begin() const { return std::begin(m_cells); }
      [[nodiscard]] auto begin()       { return std::begin(m_cells); }
      [[nodiscard]] auto end()   const { return std::end(m_cells); }
      [[nodiscard]] auto end()         { return std::end(m_cells); }

   private:
      [[nodiscard]] auto update_sequence_buffer() const -> void;
   };

   // Deduction guide
   template<typename char_type>
   screen(const int width, const int height,
      const int start_column, const int start_line,
      const char_type fill_char
   ) -> screen<std::basic_string<char_type>>;

   // TODO this is broken, never reuses m_old_cells of screen. this needs to do something else
   struct pixel_screen {
   public:
      std::vector<color> m_pixels;

   private:
      int m_halfline_height = 0; // This refers to "pixel" height. Height in lines will be half that.
      int m_origin_column = 0;
      int m_origin_halfline = 0;
      mutable screen<std::wstring> m_screen;

   public:
      explicit pixel_screen(
         const int width, const int halfline_height,
         const int start_column, const int start_halfline,
         const color& fill_color
      );

      [[nodiscard]] auto begin() const { return std::begin(m_pixels); }
      [[nodiscard]] auto begin()       { return std::begin(m_pixels); }
      [[nodiscard]] auto end()   const { return std::end(m_pixels); }
      [[nodiscard]] auto end()         { return std::end(m_pixels); }
      
      [[nodiscard]] auto get_string(const color& frame_color) const -> std::wstring;
      [[nodiscard]] auto get_width() const -> int;
      [[nodiscard]] auto get_height() const -> int;

      // If you want to override something in the screen
      [[nodiscard]] auto get_screen_ref() -> screen<std::wstring>&;

      [[nodiscard]] auto get_color(const int column, const int halfline) const -> const color&;
      [[nodiscard]] auto get_color(const int column, const int halfline) -> color&;
      [[nodiscard]] auto is_in(const int column, const int halfline) const -> bool;

   private:
      [[nodiscard]] auto get_line_height() const -> int;
   };

   struct fg_rgb_color_sequence {
      color m_color;
      operator std::string() const;
      operator std::wstring() const;
   };
   struct fg_index_color_sequence {
      int m_index;
      operator std::string() const;
      operator std::wstring() const;
   };
   struct set_index_color_sequence {
      int m_index{};
      color m_color;
      operator std::string() const;
      operator std::wstring() const;
   };
   struct bg_rgb_color_sequence {
      color m_color;
      operator std::string() const;
      operator std::wstring() const;
   };
   struct bg_index_color_sequence {
      int m_index;
      operator std::string() const;
      operator std::wstring() const;
   };
   struct underline_sequence {
      bool m_underline;
      operator std::string() const;
      operator std::wstring() const;
   };
   struct bold_sequence {
      bool m_bold;
      operator std::string() const;
      operator std::wstring() const;
   };
   struct position_sequence {
      uint8_t m_line;
      uint8_t m_column;
      operator std::string() const;
      operator std::wstring() const;
   };
   struct move_left_sequence {
      uint8_t m_amount;
      operator std::string() const;
      operator std::wstring() const;
   };
   struct move_right_sequence {
      uint8_t m_amount;
      operator std::string() const;
      operator std::wstring() const;
   };
   struct move_up_sequence {
      uint8_t m_amount;
      operator std::string() const;
      operator std::wstring() const;
   };
   struct move_down_sequence {
      uint8_t m_amount;
      operator std::string() const;
      operator std::wstring() const;
   };
   struct char_sequence {
      char m_letter;
      operator std::string() const;
      operator std::wstring() const;
   };
   struct wchar_sequence {
      wchar_t m_letter;
      operator std::string() const;
      operator std::wstring() const;
   };
   struct reset_sequence {
      operator std::string() const;
      operator std::wstring() const;
   };

   namespace detail
   {
      template<cvtsw::sequence_c sequence_type>
      [[nodiscard]] constexpr auto get_sequence_string_size(const sequence_type& sequence) -> size_t;

      template<cvtsw::std_string_type string_type, std::integral int_type>
      auto write_int_to_string(string_type& target, const int_type value, const bool with_leading_semicolon) -> void;

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
         std::optional<cell_pos> m_last_written_pos;
         std::optional<cell_format> m_format;
         
         explicit draw_state(){}

         auto write_sequence(
            std::vector<sequence_variant_type>& m_target_sequences,
            const cell_type& target_cell_state,
            const std::optional<std::reference_wrapper<const cell_type>>& old_cell_state,
            const cell_pos& target_pos,
            const int origin_line,
            const int origin_column
         ) -> void;

      private:
         [[nodiscard]] auto is_position_sequence_necessary(const cell_pos& target_pos) const -> bool;
      };


      template<cvtsw::std_string_type string_type, typename T, typename ... Ts>
      auto write_ints_into_string(
         string_type& target,
         const T& first,
         const Ts&... rest
      ) -> void;

      template<cvtsw::std_string_type string_type>
      [[nodiscard]] auto get_index_color_seq_str(const set_index_color_sequence& sequence) -> string_type;

      template<typename T, typename ... types>
      constexpr bool is_any_of = (std::same_as<T, types> || ...);

   } // namespace cvtsw::detail

} // namespace cvtsw




// Constexpr, therefore defined here
template<cvtsw::sequence_c sequence_type>
[[nodiscard]] constexpr auto cvtsw::detail::get_sequence_string_size(const sequence_type& sequence) -> size_t
{
   constexpr auto get_int_param_str_length = [](const int param) {
      if (param < 10)
         return 1;
      if (param < 100)
         return 2;
      return 3;
   };

   if constexpr (std::is_same_v<sequence_type, char_sequence> || std::is_same_v<sequence_type, wchar_sequence>) {
      return 1;
   }
   else {
      size_t reserve_size = 0;
      constexpr int semicolon_size = 1;
      if constexpr (is_any_of<sequence_type, fg_rgb_color_sequence, bg_rgb_color_sequence>) {
         reserve_size += 2 + semicolon_size + 1 +
            semicolon_size + get_int_param_str_length(sequence.m_color.red) +
            semicolon_size + get_int_param_str_length(sequence.m_color.green) +
            semicolon_size + get_int_param_str_length(sequence.m_color.blue);
      }
      else if constexpr (std::is_same_v<sequence_type, underline_sequence>)
      {
         reserve_size += sequence.m_underline ? 1 : 2;
      }
      else if constexpr (std::is_same_v<sequence_type, bold_sequence>)
      {
         reserve_size += sequence.m_bold ? 1 : 2;
      }
      else if constexpr (std::is_same_v<sequence_type, position_sequence>)
      {
         reserve_size += get_int_param_str_length(sequence.m_line);
         reserve_size += semicolon_size;
         reserve_size += get_int_param_str_length(sequence.m_column);
      }
      
      else if constexpr (std::is_same_v<sequence_type, reset_sequence>)
      {
         reserve_size += 1;
      }
      else if constexpr (is_any_of<sequence_type, move_left_sequence, move_right_sequence, move_up_sequence, move_down_sequence>)
      {
         reserve_size += get_int_param_str_length(sequence.m_amount);
      }
      else if constexpr (std::is_same_v<sequence_type, fg_index_color_sequence>)
      {
         reserve_size += 5; // "38;5;"
         reserve_size += get_int_param_str_length(sequence.m_index);
      }
      else if constexpr (std::is_same_v<sequence_type, set_index_color_sequence>)
      {
         reserve_size += 5; // "38;5;"
         reserve_size += get_int_param_str_length(sequence.m_index);
      }

      reserve_size += 3; // 2 intro, 1 outro
      return reserve_size;
   }
}


// This will deliberately be instantiated at compiletime
template<typename stream_type, cvtsw::sequence_c sequence_type>
auto cvtsw::operator<<(stream_type& os, const sequence_type& sequence) -> stream_type&
{
   using char_type = typename stream_type::char_type;
   using string_type = std::basic_string<char_type>;
   string_type temp;
   temp.reserve(detail::get_sequence_string_size(sequence));
   write_sequence_into_string(temp, sequence);
   os << temp;
   return os;
}


#ifdef CM_IMPL

// Instantiated by write_ints_into_string()
template<cvtsw::std_string_type string_type, std::integral int_type>
auto cvtsw::detail::write_int_to_string(
   string_type& target,
   const int_type value,
   const bool with_leading_semicolon
) -> void
{
   using char_type = typename string_type::value_type;

   if (with_leading_semicolon)
      target += static_cast<char_type>(';');

   const int hundreds = value / 100;
   if (value >= 100)
      target += static_cast<char_type>('0' + hundreds);
   if (value >= 10)
      target += static_cast<char_type>('0' + (value % 100) / 10);
   target += '0' + value % 10;
}


// Instantiated by write_sequence_into_string()
template<cvtsw::std_string_type string_type, typename T, typename ... Ts>
auto cvtsw::detail::write_ints_into_string(string_type& target, const T& first, const Ts&... rest) -> void
{
   detail::write_int_to_string(target, first, false);
   (detail::write_int_to_string(target, rest, true), ...);
}


// Instantiated by get_string_from_sequences()
template<cvtsw::std_string_type string_type, cvtsw::sequence_c sequence_type>
auto cvtsw::write_sequence_into_string(
   string_type& target,
   const sequence_type& sequence
) -> void
{
   if constexpr (std::is_same_v<sequence_type, fitting_char_sequence_t<string_type>>)
   {
      target += sequence.m_letter;
   }
   else
   {
      using char_type = typename string_type::value_type;

      target += static_cast<char_type>('\x1b');
      if constexpr (std::same_as<sequence_type, set_index_color_sequence>)
         target += static_cast<char_type>(']');
      else
         target += static_cast<char_type>('[');

      if constexpr (std::is_same_v<sequence_type, fg_rgb_color_sequence>)
      {
         detail::write_ints_into_string(target, 38, 2, sequence.m_color.red, sequence.m_color.green, sequence.m_color.blue);
         target += static_cast<char_type>('m');
      }
      else if constexpr (std::is_same_v<sequence_type, fg_index_color_sequence>)
      {
         detail::write_ints_into_string(target, 38, 5, sequence.m_index);
         target += static_cast<char_type>('m');
      }
      else if constexpr (std::is_same_v<sequence_type, bg_index_color_sequence>)
      {
         detail::write_ints_into_string(target, 48, 5, sequence.m_index);
         target += static_cast<char_type>('m');
      }
      else if constexpr (std::is_same_v<sequence_type, set_index_color_sequence>)
      {
         detail::write_ints_into_string(target, 4, sequence.m_index);
         target += detail::get_index_color_seq_str<string_type>(sequence);
      }
      else if constexpr (std::is_same_v<sequence_type, bg_rgb_color_sequence>)
      {
         detail::write_ints_into_string(target, 48, 2, sequence.m_color.red, sequence.m_color.green, sequence.m_color.blue);
         target += static_cast<char_type>('m');
      }
      else if constexpr (std::is_same_v<sequence_type, underline_sequence>)
      {
         detail::write_ints_into_string(target, sequence.m_underline ? 4 : 24);
         target += static_cast<char_type>('m');
      }
      else if constexpr (std::is_same_v<sequence_type, bold_sequence>)
      {
         detail::write_ints_into_string(target, sequence.m_bold ? 1 : 22);
         target += static_cast<char_type>('m');
      }
      else if constexpr (std::is_same_v<sequence_type, position_sequence>)
      {
         detail::write_ints_into_string(target, sequence.m_line + 1, sequence.m_column + 1);
         target += static_cast<char_type>('H');
      }
      else if constexpr (std::is_same_v<sequence_type, move_down_sequence>)
      {
         detail::write_ints_into_string(target, sequence.m_amount);
         target += static_cast<char_type>('B');
      }
      else if constexpr (std::is_same_v<sequence_type, move_up_sequence>)
      {
         detail::write_ints_into_string(target, sequence.m_amount);
         target += static_cast<char_type>('A');
      }
      else if constexpr (std::is_same_v<sequence_type, move_left_sequence>)
      {
         detail::write_ints_into_string(target, sequence.m_amount);
         target += static_cast<char_type>('D');
      }
      else if constexpr (std::is_same_v<sequence_type, move_right_sequence>)
      {
         detail::write_ints_into_string(target, sequence.m_amount);
         target += static_cast<char_type>('C');
      }
      else if constexpr (std::is_same_v<sequence_type, reset_sequence>)
      {
         detail::write_ints_into_string(target, 0);
         target += static_cast<char_type>('m');
      }
   }
}


template<cvtsw::std_string_type string_type>
auto cvtsw::detail::get_index_color_seq_str(
   const set_index_color_sequence& sequence
) -> string_type
{
   if (sequence.m_index < 1 || sequence.m_index > 255) {
      // TODO error
      return string_type{};
   }

   using char_type = typename string_type::value_type;

   string_type result;
   if constexpr (std::same_as<string_type, std::string>)
      result = ";rgb:";
   else
      result = L";rgb:";

   constexpr auto write_nibble = [&](const int nibble) {
      if (nibble < 10)
         result += static_cast<char_type>('0' + nibble);
      else
         result += static_cast<char_type>('a' + nibble - 10);
   };
   constexpr auto write_component = [&](const uint8_t component) {
      if (component > 15)
         write_nibble(component >> 4);
      write_nibble(component & 0xf);
   };
   write_component(sequence.m_color.red);
   result += static_cast<char_type>('/');
   write_component(sequence.m_color.green);
   result += static_cast<char_type>('/');
   write_component(sequence.m_color.blue);
   result += static_cast<char_type>('\x1b');
   result += static_cast<char_type>('\x5c');
   return result;
}


template<cvtsw::std_string_type string_type>
auto cvtsw::screen<string_type>::update_sequence_buffer() const -> void
{
   ZoneScoped;

   detail::draw_state<string_type> state{};
   m_sequence_buffer.clear();
   m_sequence_buffer.push_back(reset_sequence{});

   for (detail::cell_pos relative_pos{ this->m_width, this->m_height }; relative_pos.is_end() == false; ++relative_pos)
   {
      const cell<string_type>& target_cell_state = this->m_cells[relative_pos.m_index];

      std::optional<std::reference_wrapper<const cell<string_type>>> old_cell_state;
      if (this->m_old_cells.empty() == false)
         old_cell_state.emplace(this->m_old_cells[relative_pos.m_index]);

      state.write_sequence(
         m_sequence_buffer,
         target_cell_state, old_cell_state,
         relative_pos,
         this->m_origin_line, this->m_origin_column
      );
   }
}


template<cvtsw::std_string_type string_type>
cvtsw::screen<string_type>::screen(
   const int width, const int height,
   const int start_column, const int start_line,
   const char_type fill_char
)
   : m_cells(width* height, cell<string_type>{fill_char})
   , m_width(width)
   , m_height(height)
   , m_origin_line(start_line)
   , m_origin_column(start_column)
{

}


template <cvtsw::std_string_type string_type>
auto cvtsw::screen<string_type>::get_width() const -> int
{
   return m_width;
}


template <cvtsw::std_string_type string_type>
auto cvtsw::screen<string_type>::get_height() const -> int
{
   return m_height;
}


template<cvtsw::std_string_type string_type>
auto cvtsw::screen<string_type>::get_string() const -> string_type
{
   this->update_sequence_buffer();
   string_type result = get_string_from_sequences<string_type>(m_sequence_buffer);
   m_old_cells = m_cells;
   return result;
}


template <cvtsw::std_string_type string_type>
auto cvtsw::screen<string_type>::write_into(
   const string_type& text,
   const int column, const int line,
   const cell_format& formatting
) -> void
{
   const int ending_column = column + static_cast<int>(text.size());
   if (ending_column >= m_width)
   {
      // ERROR
   }
   for (int i = 0; i < text.size(); ++i) {
      m_cells[line * m_width + column + i].letter = text[i];
      m_cells[line * m_width + column + i].m_format = formatting;
   }
}


template<cvtsw::std_string_type string_type>
auto cvtsw::screen<string_type>::is_inside(const int column, const int line) const -> bool
{
   return column >= 0 && column < m_width&& line >= 0 && line < m_height;
}


template<cvtsw::std_string_type string_type>
auto cvtsw::screen<string_type>::get(const int column, const int line) -> cell<string_type>&
{
   const int index = line * m_width + column;
   return m_cells[index];
}
template struct cvtsw::screen<std::string>;
template struct cvtsw::screen<std::wstring>;


// Instantiated by cvtsw::screen<string_type>::get_string()
template<cvtsw::std_string_type string_type>
auto cvtsw::get_string_from_sequences(
   const std::vector<sequence_variant_type>& sequences
) -> string_type
{
   size_t reserve_size{};
   {
      ZoneScopedN("reserve size");
      for (const sequence_variant_type& sequence : sequences)
         std::visit([&](const auto& alternative) { reserve_size += detail::get_sequence_string_size(alternative); }, sequence);
   }

   string_type result_str;
   result_str.reserve(reserve_size);
   {
      ZoneScopedN("writing sequence into string");
      for (const sequence_variant_type& sequence : sequences)
         std::visit([&](const auto& alternative) { write_sequence_into_string(result_str, alternative);  }, sequence);
   }

   return result_str;
}


auto cvtsw::position(const int line, const int column) -> position_sequence {
   return position_sequence{
      .m_line = static_cast<uint8_t>(line),
      .m_column = static_cast<uint8_t>(column)
   };
}


auto cvtsw::move_left(const int amount) -> move_left_sequence
{
   return move_left_sequence{ static_cast<uint8_t>(amount)};
}


auto cvtsw::move_right(const int amount) -> move_right_sequence
{
   return move_right_sequence{ static_cast<uint8_t>(amount) };
}


auto cvtsw::move_up(const int amount) -> move_up_sequence
{
   return move_up_sequence{ static_cast<uint8_t>(amount) };
}


auto cvtsw::move_down(const int amount) -> move_down_sequence
{
   return move_down_sequence{ static_cast<uint8_t>(amount) };
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


auto cvtsw::fg_color(const int r, const int g, const int b) -> fg_rgb_color_sequence {
   return fg_rgb_color_sequence{
      .m_color = color{
         static_cast<uint8_t>(r),
         static_cast<uint8_t>(g),
         static_cast<uint8_t>(b)
      }
   };
}


auto cvtsw::fg_color(const color& col) -> fg_rgb_color_sequence {
   return fg_rgb_color_sequence{ .m_color=col };
}


auto cvtsw::fg_color(const int index) -> fg_index_color_sequence
{
   return fg_index_color_sequence{ index };
}


auto cvtsw::set_index_color(
   const int index,
   const color& col
) -> set_index_color_sequence
{
   return set_index_color_sequence{ index, col };
}


auto cvtsw::bg_color(const int r, const int g, const int b) -> bg_rgb_color_sequence {
   return bg_rgb_color_sequence{
      color{
         static_cast<uint8_t>(r),
         static_cast<uint8_t>(g),
         static_cast<uint8_t>(b)
      }
   };
}


auto cvtsw::bg_color(const color& col) -> bg_rgb_color_sequence
{
   return bg_rgb_color_sequence{ col };
}


auto cvtsw::bg_color(const int index) -> bg_index_color_sequence
{
   return bg_index_color_sequence{ index };
}


auto cvtsw::underline(const bool new_value) -> underline_sequence
{
   return underline_sequence{ new_value };
}


auto cvtsw::bold(const bool new_value) -> bold_sequence
{
   return bold_sequence{ new_value };
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
   : m_pixels(width * halfline_height, fill_color)
   , m_halfline_height(halfline_height)
   , m_origin_column(start_column)
   , m_origin_halfline(start_halfline)
   , m_screen(width, this->get_line_height(), m_origin_column, m_origin_halfline / 2, L'▀')
{

}


auto cvtsw::pixel_screen::get_screen_ref() -> screen<std::wstring>&
{
   return m_screen;
}


auto cvtsw::pixel_screen::get_string(const color& frame_color) const -> std::wstring
{
   int halfline_top = (m_origin_halfline % 2 == 0) ? 0 : -1;
   int halfline_bottom = halfline_top + 1;
   // TODO iterator?
   for (int line = 0; line < m_screen.get_height(); ++line) {
      for (int column = 0; column < m_screen.get_width(); ++column) {
         cell<std::wstring>& target_cell = m_screen.get(column, line);
         target_cell.m_format.fg_color = is_in(column, halfline_top) ? get_color(column, halfline_top) : frame_color;
         target_cell.m_format.bg_color = is_in(column, halfline_bottom) ? get_color(column, halfline_bottom) : frame_color;
      }
      halfline_top += 2;
      halfline_bottom += 2;
   }

   return m_screen.get_string();
}


auto cvtsw::pixel_screen::get_line_height() const -> int
{
   const int first_line = m_origin_halfline / 2;
   const int last_line = (m_origin_halfline - 1 + m_halfline_height) / 2;
   return last_line - first_line + 1;
}


auto cvtsw::pixel_screen::is_in(const int column, const int halfline) const -> bool
{
   const int index = halfline * this->get_width() + column;
   const bool is_out = index < 0 || index >(m_pixels.size() - 1);
   return !is_out;
}


auto cvtsw::pixel_screen::get_color(
   const int column,
   const int halfline
) const -> const color&
{
   const int index = halfline * this->get_width() + column;
   return m_pixels[index];
}


auto cvtsw::pixel_screen::get_color(
   const int column,
   const int halfline
) -> color&
{
   const int index = halfline * this->get_width() + column;
   return m_pixels[index];
}


auto cvtsw::pixel_screen::get_width() const -> int
{
   return m_screen.get_width();
}


auto cvtsw::pixel_screen::get_height() const -> int
{
   return m_halfline_height;
}


template<cvtsw::std_string_type string_type>
auto cvtsw::detail::draw_state<string_type>::write_sequence(
   std::vector<sequence_variant_type>& m_target_sequences,
   const cell_type& target_cell_state,
   const std::optional<std::reference_wrapper<const cell_type>>& old_cell_state,
   const cell_pos& target_pos,
   const int origin_line,
   const int origin_column
) -> void
{
   // As long as there's no difference from last draw, don't do anything 
   if (target_cell_state == old_cell_state)
      return;

   if (m_format.has_value() == false) {
      m_target_sequences.push_back(fg_rgb_color_sequence{ target_cell_state.m_format.fg_color });
      m_target_sequences.push_back(bg_rgb_color_sequence{ target_cell_state.m_format.bg_color });
      m_target_sequences.push_back(underline_sequence{ target_cell_state.m_format.m_underline });
      m_target_sequences.push_back(bold_sequence{ target_cell_state.m_format.m_bold });
   }
   else {
      // Apply differences between console state and the target state
      if (target_cell_state.m_format.fg_color != m_format->fg_color)
         m_target_sequences.push_back(fg_rgb_color_sequence{ target_cell_state.m_format.fg_color });
      if (target_cell_state.m_format.bg_color != m_format->bg_color)
         m_target_sequences.push_back(bg_rgb_color_sequence{ target_cell_state.m_format.bg_color });
      if (target_cell_state.m_format.m_underline != m_format->m_underline)
         m_target_sequences.push_back(underline_sequence{ target_cell_state.m_format.m_underline });
      if (target_cell_state.m_format.m_bold != m_format->m_bold)
         m_target_sequences.push_back(bold_sequence{ target_cell_state.m_format.m_bold });
   }

   if (this->is_position_sequence_necessary(target_pos)) {
      m_target_sequences.push_back(
         position_sequence{
            .m_line = static_cast<uint8_t>(target_pos.get_line() + origin_line),
            .m_column = static_cast<uint8_t>(target_pos.get_column() + origin_column)
         }
      );
   }

   m_target_sequences.push_back(fitting_char_sequence_t<string_type>{ target_cell_state.letter });

   m_last_written_pos = target_pos;
   m_format = target_cell_state.m_format;
}



template<cvtsw::std_string_type string_type>
auto cvtsw::detail::draw_state<string_type>::is_position_sequence_necessary(
   const cell_pos& target_pos
) const -> bool
{
   // There is was nothing written before, hence the cursor position is unknown
   if (m_last_written_pos.has_value() == false)
      return true;

   const cell_pos current_cursor_pos = m_last_written_pos.value() + 1;

   // The cursor position is known to be wrong
   if (target_pos != current_cursor_pos)
      return true;

   // If we're on the "right" position according to the subset of the buffer, the position still
   // needs to be set if there was a line jump.
   if (current_cursor_pos.get_line() != m_last_written_pos->get_line())
      return true;

   return false;
}


cvtsw::fg_rgb_color_sequence::operator std::string() const
{
   std::string result;
   write_sequence_into_string(result, *this);
   return result;
}

cvtsw::fg_index_color_sequence::operator std::string() const
{
   std::string result;
   write_sequence_into_string(result, *this);
   return result;
}

cvtsw::set_index_color_sequence::operator std::string() const
{
   std::string result;
   write_sequence_into_string(result, *this);
   return result;
}

cvtsw::bg_rgb_color_sequence::operator std::string() const
{
   std::string result;
   write_sequence_into_string(result, *this);
   return result;
}

cvtsw::bg_index_color_sequence::operator std::string() const
{
   std::string result;
   write_sequence_into_string(result, *this);
   return result;
}

cvtsw::underline_sequence::operator std::string() const
{
   std::string result;
   write_sequence_into_string(result, *this);
   return result;
}

cvtsw::bold_sequence::operator std::string() const
{
   std::string result;
   write_sequence_into_string(result, *this);
   return result;
}

cvtsw::position_sequence::operator std::string() const
{
   std::string result;
   write_sequence_into_string(result, *this);
   return result;
}

cvtsw::move_left_sequence::operator std::string() const
{
   std::string result;
   write_sequence_into_string(result, *this);
   return result;
}

cvtsw::move_right_sequence::operator std::string() const
{
   std::string result;
   write_sequence_into_string(result, *this);
   return result;
}

cvtsw::move_up_sequence::operator std::string() const
{
   std::string result;
   write_sequence_into_string(result, *this);
   return result;
}

cvtsw::move_down_sequence::operator std::string() const
{
   std::string result;
   write_sequence_into_string(result, *this);
   return result;
}

cvtsw::char_sequence::operator std::string() const
{
   std::string result;
   write_sequence_into_string(result, *this);
   return result;
}

cvtsw::wchar_sequence::operator std::string() const
{
   std::string result;
   write_sequence_into_string(result, *this);
   return result;
}

cvtsw::reset_sequence::operator std::string() const
{
   std::string result;
   write_sequence_into_string(result, *this);
   return result;
}

cvtsw::fg_rgb_color_sequence::operator std::wstring() const
{
   std::wstring result;
   write_sequence_into_string(result, *this);
   return result;
}

cvtsw::fg_index_color_sequence::operator std::wstring() const
{
   std::wstring result;
   write_sequence_into_string(result, *this);
   return result;
}

cvtsw::set_index_color_sequence::operator std::wstring() const
{
   std::wstring result;
   write_sequence_into_string(result, *this);
   return result;
}

cvtsw::bg_rgb_color_sequence::operator std::wstring() const
{
   std::wstring result;
   write_sequence_into_string(result, *this);
   return result;
}

cvtsw::bg_index_color_sequence::operator std::wstring() const
{
   std::wstring result;
   write_sequence_into_string(result, *this);
   return result;
}

cvtsw::underline_sequence::operator std::wstring() const
{
   std::wstring result;
   write_sequence_into_string(result, *this);
   return result;
}

cvtsw::bold_sequence::operator std::wstring() const
{
   std::wstring result;
   write_sequence_into_string(result, *this);
   return result;
}

cvtsw::position_sequence::operator std::wstring() const
{
   std::wstring result;
   write_sequence_into_string(result, *this);
   return result;
}

cvtsw::move_left_sequence::operator std::wstring() const
{
   std::wstring result;
   write_sequence_into_string(result, *this);
   return result;
}

cvtsw::move_right_sequence::operator std::wstring() const
{
   std::wstring result;
   write_sequence_into_string(result, *this);
   return result;
}

cvtsw::move_up_sequence::operator std::wstring() const
{
   std::wstring result;
   write_sequence_into_string(result, *this);
   return result;
}

cvtsw::move_down_sequence::operator std::wstring() const
{
   std::wstring result;
   write_sequence_into_string(result, *this);
   return result;
}

cvtsw::char_sequence::operator std::wstring() const
{
   std::wstring result;
   write_sequence_into_string(result, *this);
   return result;
}

cvtsw::wchar_sequence::operator std::wstring() const
{
   std::wstring result;
   write_sequence_into_string(result, *this);
   return result;
}

cvtsw::reset_sequence::operator std::wstring() const
{
   std::wstring result;
   write_sequence_into_string(result, *this);
   return result;
}


#endif
