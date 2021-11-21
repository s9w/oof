#pragma once

#include <functional>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace oof
{
   // Feel free to bit_cast, reinterpret_cast or memcpy your 3-byte color type into this.
   struct color {
      uint8_t red{}, green{}, blue{};
      constexpr color() = default;

      template<std::integral component_type>
      constexpr color(component_type r, component_type g, component_type b)
         : red{ static_cast<uint8_t>(r) }
         , green{ static_cast<uint8_t>(g) }
         , blue{ static_cast<uint8_t>(b) }
      {}

      template<std::integral component_type>
      constexpr color(component_type component)
         : color(component, component, component)
      {}

      friend constexpr auto operator==(const color&, const color&) -> bool = default;
   };
   

   // Necessary forward declarations
   struct fg_rgb_color_sequence; struct fg_index_color_sequence;
   struct bg_rgb_color_sequence; struct bg_index_color_sequence;
   struct set_index_color_sequence;
   struct bold_sequence; struct cursor_visibility_sequence; struct underline_sequence;
   struct position_sequence; struct hposition_sequence; struct vposition_sequence;
   struct move_left_sequence; struct move_right_sequence; struct move_up_sequence; struct move_down_sequence;
   struct char_sequence; struct wchar_sequence;
   struct reset_sequence; struct clear_screen_sequence;

   // Sets the foreground RGB color
   [[nodiscard]] auto fg_color(const color& col) -> fg_rgb_color_sequence;

   // Sets the foreground indexed color. Index must be in [1, 255]. You can define colors with set_index_color().
   [[nodiscard]] auto fg_color(int index) -> fg_index_color_sequence;

   // Sets the background RGB color
   [[nodiscard]] auto bg_color(const color& col) -> bg_rgb_color_sequence;

   // Sets the background indexed color. Index must be in [1, 255]. You can define colors with set_index_color().
   [[nodiscard]] auto bg_color(int index) -> bg_index_color_sequence;

   // Sets the indexed color. Index must be in [1, 255].
   [[nodiscard]] auto set_index_color(int index, const color& col) -> set_index_color_sequence;

   // Sets the underline state of the console
   [[nodiscard]] auto underline(bool new_value = true) -> underline_sequence;

   // Sets the bold state of the console. Warning: Bold is not supported by all console, see readme
   [[nodiscard]] auto bold(bool new_value = true) -> bold_sequence;

   // Sets cursor visibility state. Recommended to turn off before doing real-time displays
   [[nodiscard]] auto cursor_visibility(bool new_value) -> cursor_visibility_sequence;

   // Resets foreground- and background color, underline and bold state
   [[nodiscard]] auto reset_formatting() -> reset_sequence;

   // Clears the screen
   [[nodiscard]] auto clear_screen() -> clear_screen_sequence;

   // Sets the cursor position. Zero-based
   [[nodiscard]] auto position(int line, int column) -> position_sequence;
   [[nodiscard]] auto vposition(int line) -> vposition_sequence;
   [[nodiscard]] auto hposition(int column) -> hposition_sequence;

   // Moves the cursor a certain amount
   [[nodiscard]] auto move_left(int amount) -> move_left_sequence;
   [[nodiscard]] auto move_right(int amount) -> move_right_sequence;
   [[nodiscard]] auto move_up(int amount) -> move_up_sequence;
   [[nodiscard]] auto move_down(int amount) -> move_down_sequence;


   using error_callback_type = void(*)(const std::string& msg);
   inline error_callback_type error_callback = nullptr;

   template<typename T, typename ... types>
   constexpr bool is_any_of = (std::same_as<T, types> || ...);
   template<typename T>
   concept std_string_type = is_any_of<T, std::string, std::wstring>;

   template<typename T, typename variant_type>
   struct is_alternative : std::false_type {};
   template<typename T, typename ... variant_alternatives>
   struct is_alternative<T, std::variant<variant_alternatives...>>
      : std::disjunction<std::is_same<T, variant_alternatives>...> {};
   template<typename T, typename variant_type>
   constexpr bool is_alternative_v = is_alternative<T, variant_type>::value;

   using sequence_variant_type = std::variant<
      fg_rgb_color_sequence, fg_index_color_sequence, bg_index_color_sequence, bg_rgb_color_sequence, set_index_color_sequence,
      position_sequence, hposition_sequence, vposition_sequence,
      underline_sequence, bold_sequence, char_sequence, wchar_sequence, reset_sequence, clear_screen_sequence, cursor_visibility_sequence,
      move_left_sequence, move_right_sequence, move_up_sequence, move_down_sequence
   >;

   template<typename T>
   concept sequence_c = is_alternative_v<T, sequence_variant_type>;

   
   // Writes a single sequence type into a string
   template<oof::std_string_type string_type, oof::sequence_c sequence_type>
   auto write_sequence_into_string(string_type& target, const sequence_type& sequence) -> void;

   // Returns a sing from a sequence type
   template<oof::std_string_type string_type, oof::sequence_c sequence_type>
   [[nodiscard]] auto get_string_from_sequence(const sequence_type& sequence) -> string_type;

   // Returns a string from a vector of sequence types
   template<oof::std_string_type string_type>
   [[nodiscard]] auto get_string_from_sequences(const std::vector<sequence_variant_type>& sequences) -> string_type;

   // Returns the exact size a string from this vector of sequence types
   [[nodiscard]] auto get_string_reserve_size(const std::vector<sequence_variant_type>& sequences) -> size_t;
   

   struct cell_format {
      bool m_underline = false;
      bool m_bold = false;
      color m_fg_color{255, 255, 255};
      color m_bg_color{0, 0, 0};
      friend constexpr auto operator==(const cell_format&, const cell_format&) -> bool = default;
   };


   template<oof::std_string_type string_type>
   struct cell {
      using char_type = typename string_type::value_type;

      char_type m_letter{};
      cell_format m_format{};
      friend constexpr auto operator==(const cell&, const cell&) -> bool = default;
   };


   template<oof::std_string_type string_type>
   struct screen{
      using char_type = typename string_type::value_type;

      explicit screen(int width, int height, int start_column, int start_line, const cell<string_type>& background);

      // This constructor taking a fill_char implies black background, white foreground color
      explicit screen(int width, int height, int start_column, int start_line, char_type fill_char);

      // This constructor taking a fill_char implies black background, white foreground color and top left start
      explicit screen(int width, int height, char_type fill_char);

      [[nodiscard]] auto get_width() const -> int;
      [[nodiscard]] auto get_height() const -> int;
      
      [[nodiscard]] auto get_cell (int column, int line) -> cell<string_type>&;
      [[nodiscard]] auto is_inside(int column, int line) const -> bool;
      [[nodiscard]] auto get_string(                   ) const -> string_type;
                    auto get_string(string_type& buffer) const -> void;

      // This writes a text into the screen cells
      auto write_into(const string_type& text, int column, int line, const cell_format& formatting) -> void;

      // Override all cells with the background state
      auto clear() -> void;

      [[nodiscard]] auto begin() const { return std::begin(m_cells); }
      [[nodiscard]] auto begin()       { return std::begin(m_cells); }
      [[nodiscard]] auto end()   const { return std::end(m_cells); }
      [[nodiscard]] auto end()         { return std::end(m_cells); }

   private:
      auto update_sequence_buffer() const -> void;

      int m_width = 0;
      int m_height = 0;
      int m_origin_line = 0;
      int m_origin_column = 0;
      cell<string_type> m_background;
      std::vector<cell<string_type>> m_cells;
      mutable std::vector<cell<string_type>> m_old_cells;
      mutable std::vector<sequence_variant_type> m_sequence_buffer;
   };
   

   struct pixel_screen {
      std::vector<color> m_pixels;

      explicit pixel_screen(int width, int halfline_height, int start_column, int start_halfline, const color& fill_color);

      // This will init with black fill color
      explicit pixel_screen(int width, int halfline_height, int start_column, int start_halfline);

      // This will init with black fill color and starting at the top left
      explicit pixel_screen(int width, int halfline_height);

      [[nodiscard]] auto begin() const { return std::begin(m_pixels); }
      [[nodiscard]] auto begin()       { return std::begin(m_pixels); }
      [[nodiscard]] auto end()   const { return std::end(m_pixels); }
      [[nodiscard]] auto end()         { return std::end(m_pixels); }
      
      [[nodiscard]] auto get_string(                    ) const -> std::wstring;
                    auto get_string(std::wstring& buffer) const -> void;
      [[nodiscard]] auto get_width() const -> int;
      [[nodiscard]] auto get_halfline_height() const -> int;

      // If you want to override something in the screen
      [[nodiscard]] auto get_screen_ref() -> screen<std::wstring>&;

      // Override all pixels with the fill color
                    auto clear() -> void;
      
      [[nodiscard]] auto get_color(int column, int halfline) const -> const color&;
      [[nodiscard]] auto get_color(int column, int halfline)       -> color&;
      [[nodiscard]] auto is_in    (int column, int halfline) const -> bool;

   private:
      [[nodiscard]] auto get_line_height() const -> int;
      auto compute_result() const -> void;

      color m_fill_color{};
      int m_halfline_height = 0; // This refers to "pixel" height. Height in lines will be half that.
      int m_origin_column = 0;
      int m_origin_halfline = 0;
      mutable screen<std::wstring> m_screen;
   };



   // Deduction guide
   template<typename char_type>
   screen(int, int, int, int, char_type fill_char) -> screen<std::basic_string<char_type>>;
   template<typename char_type>
   screen(int, int, char_type fill_char) -> screen<std::basic_string<char_type>>;

   template<typename stream_type, oof::sequence_c sequence_type>
   auto operator<<(stream_type& os, const sequence_type& sequence) -> stream_type&;

   namespace detail
   {

      // CRTP to extend the numerous sequence types with convenience member functions without using runtime
      // polymorphism or repeating the code
      template<typename T>
      struct extender {
         operator std::string() const;
         operator std::wstring() const;
         [[nodiscard]] auto operator+(const std::string&  other) const -> std::string;
         [[nodiscard]] auto operator+(const std::wstring& other) const -> std::wstring;
      };

      auto error(const std::string& msg) -> void;

      [[nodiscard]] auto get_pixel_background(const color& fill_color) -> cell<std::wstring>;

      template<oof::std_string_type string_type>
      auto write_sequence_string_no_reserve(const std::vector<sequence_variant_type>& sequences, string_type& target) -> void;

      template<oof::sequence_c sequence_type>
      [[nodiscard]] constexpr auto get_sequence_string_size(const sequence_type& sequence) -> size_t;

      template<oof::std_string_type string_type, std::integral int_type>
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
         [[nodiscard]] constexpr auto operator+(const int jump_amount) const -> cell_pos {
            cell_pos jumped_pos(m_width, m_height);
            jumped_pos.m_index = m_index + jump_amount;
            return jumped_pos;
         }
         constexpr auto operator++() -> cell_pos& {
            ++m_index;
            return *this;
         }
         friend constexpr auto operator==(const cell_pos&, const cell_pos&) -> bool = default;
      };


      template<oof::std_string_type string_type>
      struct draw_state{
         using cell_type = cell<string_type>;
         std::optional<cell_pos> m_last_written_pos;
         std::optional<cell_format> m_format;
         
         explicit draw_state() = default;

         auto write_sequence(
            std::vector<sequence_variant_type>& sequence_buffer,
            const cell_type& target_cell_state,
            const std::optional<std::reference_wrapper<const cell_type>>& old_cell_state,
            const cell_pos& target_pos,
            const int origin_line,
            const int origin_column
         ) -> void;

      private:
         [[nodiscard]] auto is_position_sequence_necessary(const cell_pos& target_pos) const -> bool;
      };

      template<oof::std_string_type string_type, typename T, typename ... Ts>
      auto write_ints_into_string(string_type& target, const T& first, const Ts&... rest) -> void;

      template<oof::std_string_type string_type>
      [[nodiscard]] auto get_index_color_seq_str(const set_index_color_sequence& sequence) -> string_type;

      template<std_string_type string_type>
      using fitting_char_sequence_t = std::conditional_t<std::is_same_v<string_type, std::string>, char_sequence, wchar_sequence>;

   } // namespace detail


   struct fg_rgb_color_sequence : detail::extender<fg_rgb_color_sequence> {
      color m_color;
   };
   struct fg_index_color_sequence : detail::extender<fg_index_color_sequence> {
      int m_index;
   };
   struct set_index_color_sequence : detail::extender<set_index_color_sequence> {
      int m_index{};
      color m_color;
   };
   struct bg_rgb_color_sequence : detail::extender<bg_rgb_color_sequence> {
      color m_color;
   };
   struct bg_index_color_sequence : detail::extender<bg_index_color_sequence> {
      int m_index;
   };
   struct underline_sequence : detail::extender<underline_sequence> {
      bool m_underline;
   };
   struct bold_sequence : detail::extender<bold_sequence> {
      bool m_bold;
   };
   struct cursor_visibility_sequence : detail::extender<cursor_visibility_sequence> {
      bool m_visibility;
   };
   struct position_sequence : detail::extender<position_sequence> {
      uint8_t m_line;
      uint8_t m_column;
   };
   struct hposition_sequence : detail::extender<hposition_sequence> {
      uint8_t m_column;
   };
   struct vposition_sequence : detail::extender<vposition_sequence> {
      uint8_t m_line;
   };
   struct move_left_sequence : detail::extender<move_left_sequence> {
      uint8_t m_amount;
   };
   struct move_right_sequence : detail::extender<move_right_sequence> {
      uint8_t m_amount;
   };
   struct move_up_sequence : detail::extender<move_up_sequence> {
      uint8_t m_amount;
   };
   struct move_down_sequence : detail::extender<move_down_sequence> {
      uint8_t m_amount;
   };
   struct char_sequence : detail::extender<char_sequence> {
      char m_letter;
   };
   struct wchar_sequence : detail::extender<wchar_sequence> {
      wchar_t m_letter;
   };
   struct reset_sequence : detail::extender<reset_sequence> {};
   struct clear_screen_sequence : detail::extender<clear_screen_sequence> {};

} // namespace oof


// Constexpr, therefore defined here
template<oof::sequence_c sequence_type>
constexpr auto oof::detail::get_sequence_string_size(const sequence_type& sequence) -> size_t
{
   constexpr auto get_int_param_str_length = [](const int param) -> int {
      if (param < 10)  return 1;
      if (param < 100) return 2;
                       return 3;
   };

   if constexpr (is_any_of<sequence_type, char_sequence, wchar_sequence>) {
      return 1;
   }
   else if constexpr (std::is_same_v<sequence_type, set_index_color_sequence>) {
      size_t reserve_size = 0;
      reserve_size += 4; // \x1b]4;
      reserve_size += get_int_param_str_length(sequence.m_index); // <i>;
      reserve_size += 4; // ;rgb:

      constexpr auto get_component_str_size = [](const uint8_t component) {
         return component < 15 ? 2 : 1;
      };
      reserve_size += get_component_str_size(sequence.m_color.red);
      reserve_size += 1; // /
      reserve_size += get_component_str_size(sequence.m_color.green);
      reserve_size += 1; // /
      reserve_size += get_component_str_size(sequence.m_color.blue);
      reserve_size += 2; // <ST>

      return reserve_size;
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
      else if constexpr (std::is_same_v<sequence_type, hposition_sequence>) {
         reserve_size += get_int_param_str_length(sequence.m_column);
      }
      else if constexpr (std::is_same_v<sequence_type, vposition_sequence>) {
         reserve_size += get_int_param_str_length(sequence.m_line);
      }
      else if constexpr (is_any_of<sequence_type, reset_sequence, clear_screen_sequence>)
      {
         reserve_size += 1;
      }
      else if constexpr (is_any_of<sequence_type, cursor_visibility_sequence>)
      {
         reserve_size += 3;
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

      reserve_size += 3; // 2 intro, 1 outro
      return reserve_size;
   }
}


// This will deliberately be instantiated at compiletime
template<typename stream_type, oof::sequence_c sequence_type>
auto oof::operator<<(stream_type& os, const sequence_type& sequence) -> stream_type&
{
   using char_type = typename stream_type::char_type;
   using string_type = std::basic_string<char_type>;
   string_type temp_string{};
   temp_string.reserve(detail::get_sequence_string_size(sequence));
   write_sequence_into_string(temp_string, sequence);
   os << temp_string;
   return os;
}


#ifdef OOF_IMPL

// Instantiated by write_ints_into_string()
template<oof::std_string_type string_type, std::integral int_type>
auto oof::detail::write_int_to_string(
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
template<oof::std_string_type string_type, typename T, typename ... Ts>
auto oof::detail::write_ints_into_string(string_type& target, const T& first, const Ts&... rest) -> void
{
   detail::write_int_to_string(target, first, false);
   (detail::write_int_to_string(target, rest, true), ...);
}


// Instantiated by write_sequence_string_no_reserve()
template<oof::std_string_type string_type, oof::sequence_c sequence_type>
auto oof::write_sequence_into_string(
   string_type& target,
   const sequence_type& sequence
) -> void
{
   if constexpr (std::is_same_v<sequence_type, detail::fitting_char_sequence_t<string_type>>)
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
      else if constexpr (std::is_same_v<sequence_type, cursor_visibility_sequence>)
      {
         target += static_cast<char_type>('?');
         detail::write_ints_into_string(target, 25);
         target += static_cast<char_type>(sequence.m_visibility ? 'h' : 'l');
      }
      else if constexpr (std::is_same_v<sequence_type, position_sequence>)
      {
         detail::write_ints_into_string(target, sequence.m_line + 1, sequence.m_column + 1);
         target += static_cast<char_type>('H');
      }
      else if constexpr (std::is_same_v<sequence_type, hposition_sequence>)
      {
         detail::write_ints_into_string(target, sequence.m_column + 1);
         target += static_cast<char_type>('G');
      }
      else if constexpr (std::is_same_v<sequence_type, vposition_sequence>)
      {
         detail::write_ints_into_string(target, sequence.m_line + 1);
         target += static_cast<char_type>('d');
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
      else if constexpr (std::is_same_v<sequence_type, clear_screen_sequence>)
      {
         detail::write_ints_into_string(target, 2);
         target += static_cast<char_type>('J');
      }
   }
}


template<oof::std_string_type string_type>
auto oof::detail::get_index_color_seq_str(
   const set_index_color_sequence& sequence
) -> string_type
{
   using char_type = typename string_type::value_type;

   string_type result;
   if constexpr (std::same_as<string_type, std::string>)
      result = ";rgb:";
   else
      result = L";rgb:";

   const auto write_nibble = [&](const int nibble) {
      if (nibble < 10)
         result += static_cast<char_type>('0' + nibble);
      else
         result += static_cast<char_type>('a' + nibble - 10);
   };
   const auto write_component = [&](const uint8_t component) {
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


template<oof::std_string_type string_type>
auto oof::screen<string_type>::update_sequence_buffer() const -> void
{
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


template<oof::std_string_type string_type>
oof::screen<string_type>::screen(
   const int width, const int height,
   const int start_column, const int start_line,
   const cell<string_type>& background
)
   : m_width(width)
   , m_height(height)
   , m_origin_line(start_line)
   , m_origin_column(start_column)
   , m_background(background)
   , m_cells(width* height, background)
{
   if (width <= 0)
   {
      const std::string msg = "Width can't be negative";
      ::oof::detail::error(msg);
   }
   if (height <= 0)
   {
      const std::string msg = "Height can't be negative";
      ::oof::detail::error(msg);
   }
}


template<oof::std_string_type string_type>
oof::screen<string_type>::screen(
   const int width, const int height,
   const int start_column, const int start_line,
   const char_type fill_char
)
   : screen(width, height, start_column, start_line, cell<string_type>{fill_char})
{
   
}


template<oof::std_string_type string_type>
oof::screen<string_type>::screen(
   const int width, const int height,
   const char_type fill_char
)
   : screen(width, height, 0, 0, fill_char)
{

}


template <oof::std_string_type string_type>
auto oof::screen<string_type>::get_width() const -> int
{
   return m_width;
}


template <oof::std_string_type string_type>
auto oof::screen<string_type>::get_height() const -> int
{
   return m_height;
}


template<oof::std_string_type string_type>
auto oof::screen<string_type>::get_string() const -> string_type
{
   this->update_sequence_buffer();
   string_type result = ::oof::get_string_from_sequences<string_type>(m_sequence_buffer);
   m_old_cells = m_cells;
   return result;
}


template<oof::std_string_type string_type>
auto oof::screen<string_type>::get_string(string_type& buffer) const -> void
{
   this->update_sequence_buffer();

   // Reserve if the string buffer is still empty (on the first call)
   if (buffer.empty())
      buffer.reserve(::oof::get_string_reserve_size(m_sequence_buffer));

   buffer.clear();

   ::oof::detail::write_sequence_string_no_reserve(m_sequence_buffer, buffer);
   m_old_cells = m_cells;
}


template <oof::std_string_type string_type>
auto oof::screen<string_type>::write_into(
   const string_type& text,
   const int column, const int line,
   const cell_format& formatting
) -> void
{
   if (line < 0 || line >= m_height)
   {
      std::string msg = "Line is out of range. Height is ";
      msg += std::to_string(m_height);
      msg += ", line was: ";
      msg += std::to_string(line);
      ::oof::detail::error(msg);
      return;
   }
   if (column < 0 || column >= m_width)
   {
      std::string msg = "Column is out of range. Width is ";
      msg += std::to_string(m_width);
      msg += ", column was: ";
      msg += std::to_string(column);
      ::oof::detail::error(msg);
      return;
   }

   const int ending_column = column + static_cast<int>(text.size());
   if (ending_column >= m_width)
   {
      ::oof::detail::error("Trying to write_into() with a text that won't fit.");
      return;
   }
   for (size_t i = 0; i < text.size(); ++i) {
      cell<string_type>& cell = m_cells[line * m_width + column + i];
      cell.m_letter = text[i];
      cell.m_format = formatting;
   }
}


template<oof::std_string_type string_type>
auto oof::screen<string_type>::is_inside(const int column, const int line) const -> bool
{
   return column >= 0 && column < m_width&& line >= 0 && line < m_height;
}


template<oof::std_string_type string_type>
auto oof::screen<string_type>::get_cell(const int column, const int line) -> cell<string_type>&
{
   if ( line < 0 || line >= m_height)
   {
      std::string msg = "Line is out of range. Height is ";
      msg += std::to_string(m_height);
      msg += ", line was: ";
      msg += std::to_string(line);
      ::oof::detail::error(msg);
      return m_cells[0];
   }
   if (column < 0 || column >= m_width)
   {
      std::string msg = "Column is out of range. Width is ";
      msg += std::to_string(m_width);
      msg += ", column was: ";
      msg += std::to_string(column);
      ::oof::detail::error(msg);
      return m_cells[0];
   }

   const int index = line * m_width + column;
   return m_cells[index];
}
template struct oof::screen<std::string>;
template struct oof::screen<std::wstring>;


auto oof::get_string_reserve_size(const std::vector<sequence_variant_type>& sequences) -> size_t
{
   size_t reserve_size{};
   for (const sequence_variant_type& sequence : sequences)
      std::visit([&](const auto& alternative) { reserve_size += detail::get_sequence_string_size(alternative); }, sequence);
   return reserve_size;
}


// Instantiated by oof::screen<string_type>::get_string()
template<oof::std_string_type string_type>
auto oof::detail::write_sequence_string_no_reserve(
   const std::vector<sequence_variant_type>& sequences,
   string_type& target
) -> void
{
   for (const sequence_variant_type& sequence : sequences)
      std::visit([&](const auto& alternative) { write_sequence_into_string(target, alternative);  }, sequence);
}


template<oof::std_string_type string_type>
auto oof::get_string_from_sequences(
   const std::vector<sequence_variant_type>& sequences
) -> string_type
{
   string_type result_str{};
   result_str.reserve(::oof::get_string_reserve_size(sequences));
   ::oof::detail::write_sequence_string_no_reserve(sequences, result_str);
   return result_str;
}
template auto oof::get_string_from_sequences(const std::vector<sequence_variant_type>& sequences) -> std::string;
template auto oof::get_string_from_sequences(const std::vector<sequence_variant_type>& sequences) -> std::wstring;


auto oof::position(const int line, const int column) -> position_sequence {
   return position_sequence{
      .m_line = static_cast<uint8_t>(line),
      .m_column = static_cast<uint8_t>(column)
   };
}


auto oof::vposition(const int line) -> vposition_sequence {
   return vposition_sequence{ .m_line = static_cast<uint8_t>(line) };
}


auto oof::hposition(const int column) -> hposition_sequence {
   return hposition_sequence{ .m_column = static_cast<uint8_t>(column) };
}


auto oof::move_left(const int amount) -> move_left_sequence
{
   return move_left_sequence{ .m_amount = static_cast<uint8_t>(amount)};
}


auto oof::move_right(const int amount) -> move_right_sequence
{
   return move_right_sequence{ .m_amount = static_cast<uint8_t>(amount) };
}


auto oof::move_up(const int amount) -> move_up_sequence
{
   return move_up_sequence{ .m_amount = static_cast<uint8_t>(amount) };
}


auto oof::move_down(const int amount) -> move_down_sequence
{
   return move_down_sequence{ .m_amount = static_cast<uint8_t>(amount) };
}


auto oof::fg_color(const color& col) -> fg_rgb_color_sequence {
   return fg_rgb_color_sequence{ .m_color = col };
}


auto oof::fg_color(const int index) -> fg_index_color_sequence
{
   if (index < 1 || index > 255)
   {
      std::string msg = "Index must be in [1, 255], was: ";
      msg += std::to_string(index);
      ::oof::detail::error(msg);
      return fg_index_color_sequence{ .m_index=1 };
   }
   return fg_index_color_sequence{ .m_index = index };
}


auto oof::set_index_color(
   const int index,
   const color& col
) -> set_index_color_sequence
{
   if (index < 1 || index > 255)
   {
      std::string msg = "Index must be in [1, 255], was: ";
      msg += std::to_string(index);
      ::oof::detail::error(msg);
      return set_index_color_sequence{ .m_index=1, .m_color=col };
   }
   return set_index_color_sequence{ .m_index=index, .m_color=col };
}


auto oof::bg_color(const color& col) -> bg_rgb_color_sequence
{
   return bg_rgb_color_sequence{ .m_color=col };
}


auto oof::bg_color(const int index) -> bg_index_color_sequence
{
   if (index < 1 || index > 255)
   {
      ::oof::detail::error("Index must be in [1, 255]");
      return bg_index_color_sequence{ .m_index=1 };
   }
   return bg_index_color_sequence{ .m_index=index };
}


auto oof::underline(const bool new_value) -> underline_sequence
{
   return underline_sequence{ .m_underline=new_value };
}


auto oof::bold(const bool new_value) -> bold_sequence
{
   return bold_sequence{ .m_bold=new_value };
}


auto oof::cursor_visibility(const bool new_value) -> cursor_visibility_sequence
{
   return cursor_visibility_sequence{ .m_visibility=new_value };
}


auto oof::reset_formatting() -> reset_sequence {
   return reset_sequence{};
}


auto oof::clear_screen() -> clear_screen_sequence {
   return clear_screen_sequence{};
}


oof::pixel_screen::pixel_screen(
   const int width,
   const int halfline_height,
   const int start_column,
   const int start_halfline,
   const color& fill_color
)
   : m_fill_color(fill_color)
   , m_halfline_height(halfline_height)
   , m_origin_column(start_column)
   , m_origin_halfline(start_halfline)
   , m_screen(width, this->get_line_height(), m_origin_column, m_origin_halfline / 2, detail::get_pixel_background(fill_color))
   , m_pixels(width * halfline_height, fill_color)
{

}


oof::pixel_screen::pixel_screen(
   const int width,
   const int halfline_height,
   const int start_column,
   const int start_halfline
)
   : pixel_screen(width, halfline_height, start_column, start_halfline, color{})
{

}


oof::pixel_screen::pixel_screen(
   const int width,
   const int halfline_height
)
   : pixel_screen(width, halfline_height, 0, 0, color{})
{

}


auto oof::pixel_screen::get_screen_ref() -> screen<std::wstring>&
{
   return m_screen;
}


auto oof::pixel_screen::compute_result() const -> void
{
   int halfline_top = (m_origin_halfline % 2 == 0) ? 0 : -1;
   int halfline_bottom = halfline_top + 1;
   // TODO iterator?
   for (int line = 0; line < m_screen.get_height(); ++line) {
      for (int column = 0; column < m_screen.get_width(); ++column) {
         cell<std::wstring>& target_cell = m_screen.get_cell(column, line);
         target_cell.m_format.m_fg_color = is_in(column, halfline_top) ? get_color(column, halfline_top) : m_fill_color;
         target_cell.m_format.m_bg_color = is_in(column, halfline_bottom) ? get_color(column, halfline_bottom) : m_fill_color;
      }
      halfline_top += 2;
      halfline_bottom += 2;
   }
}


auto oof::pixel_screen::get_string() const -> std::wstring
{
   compute_result();
   return m_screen.get_string();
}


auto oof::pixel_screen::get_string(std::wstring& buffer) const -> void
{
   compute_result();
   m_screen.get_string(buffer);
}


auto oof::pixel_screen::get_line_height() const -> int
{
   const int first_line = m_origin_halfline / 2;
   const int last_line = (m_origin_halfline - 1 + m_halfline_height) / 2;
   return last_line - first_line + 1;
}


auto oof::pixel_screen::is_in(const int column, const int halfline) const -> bool
{
   const size_t index = halfline * this->get_width() + column;
   return index >= 0 && index < m_pixels.size();
}


auto oof::pixel_screen::get_color(
   const int column,
   const int halfline
) const -> const color&
{
   const size_t index = halfline * this->get_width() + column;
   return m_pixels[index];
}


auto oof::pixel_screen::get_color(
   const int column,
   const int halfline
) -> color&
{
   const size_t index = halfline * this->get_width() + column;
   return m_pixels[index];
}


auto oof::pixel_screen::get_width() const -> int
{
   return m_screen.get_width();
}


auto oof::pixel_screen::get_halfline_height() const -> int
{
   return m_halfline_height;
}


auto oof::pixel_screen::clear() -> void
{
   for (color& pixel : m_pixels)
      pixel = m_fill_color;
}


template<oof::std_string_type string_type>
auto oof::screen<string_type>::clear() -> void
{
   for (cell<string_type>& cell : *this)
      cell = m_background;
}


template<oof::std_string_type string_type>
auto oof::detail::draw_state<string_type>::write_sequence(
   std::vector<sequence_variant_type>& sequence_buffer,
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
      sequence_buffer.push_back(fg_rgb_color_sequence{ .m_color=target_cell_state.m_format.m_fg_color });
      sequence_buffer.push_back(bg_rgb_color_sequence{ .m_color=target_cell_state.m_format.m_bg_color });
      sequence_buffer.push_back(underline_sequence{ .m_underline=target_cell_state.m_format.m_underline });
      sequence_buffer.push_back(bold_sequence{ .m_bold=target_cell_state.m_format.m_bold });
   }
   else {
      // Apply differences between console state and the target state
      if (target_cell_state.m_format.m_fg_color != m_format->m_fg_color)
         sequence_buffer.push_back(fg_rgb_color_sequence{ .m_color=target_cell_state.m_format.m_fg_color });
      if (target_cell_state.m_format.m_bg_color != m_format->m_bg_color)
         sequence_buffer.push_back(bg_rgb_color_sequence{ .m_color=target_cell_state.m_format.m_bg_color });
      if (target_cell_state.m_format.m_underline != m_format->m_underline)
         sequence_buffer.push_back(underline_sequence{ .m_underline=target_cell_state.m_format.m_underline });
      if (target_cell_state.m_format.m_bold != m_format->m_bold)
         sequence_buffer.push_back(bold_sequence{ .m_bold=target_cell_state.m_format.m_bold });
   }

   if (this->is_position_sequence_necessary(target_pos)) {
      sequence_buffer.push_back(
         position_sequence{
            .m_line = static_cast<uint8_t>(target_pos.get_line() + origin_line),
            .m_column = static_cast<uint8_t>(target_pos.get_column() + origin_column)
         }
      );
   }

   sequence_buffer.push_back(fitting_char_sequence_t<string_type>{ .m_letter=target_cell_state.m_letter });

   m_last_written_pos = target_pos;
   m_format = target_cell_state.m_format;
}


template<oof::std_string_type string_type>
auto oof::detail::draw_state<string_type>::is_position_sequence_necessary(
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


template<oof::std_string_type string_type, oof::sequence_c sequence_type>
auto oof::get_string_from_sequence(const sequence_type& sequence) -> string_type
{
   string_type result{};
   write_sequence_into_string(result, sequence);
   return result;
}


auto oof::detail::error(const std::string& msg) -> void
{
   if(error_callback != nullptr)
   {
      error_callback(msg);
   }
}


auto oof::detail::get_pixel_background(const color& fill_color) -> cell<std::wstring>
{
   return cell<std::wstring>{
      .m_letter = L'▀',
      .m_format = {
         .m_fg_color = fill_color,
         .m_bg_color = fill_color
      }
   };
}


template<typename sequence_type>
oof::detail::extender<sequence_type>::operator std::string() const{
   const sequence_type& sequence = static_cast<const sequence_type&>(*this);
   return get_string_from_sequence<std::string>(sequence);
}


template<typename sequence_type>
oof::detail::extender<sequence_type>::operator std::wstring() const {
   const sequence_type& sequence = static_cast<const sequence_type&>(*this);
   return get_string_from_sequence<std::wstring>(sequence);
}


template<typename sequence_type>
auto oof::detail::extender<sequence_type>::operator+(const std::string& other) const -> std::string{
   const sequence_type& sequence = static_cast<const sequence_type&>(*this);
   return std::string(sequence) + other;
}


template<typename sequence_type>
auto oof::detail::extender<sequence_type>::operator+(const std::wstring& other) const -> std::wstring{
   const sequence_type& sequence = static_cast<const sequence_type&>(*this);
   return std::wstring(sequence) + other;
}


// This is just to mass-instantiate the extender functions by abusing std::visit 
auto impl_fun() -> void {
   std::visit([](const auto& altern) {return altern + std::string{};  }, oof::sequence_variant_type{});
   std::visit([](const auto& altern) {return altern + std::wstring{}; }, oof::sequence_variant_type{});
}

#endif // OOF_IMPL
