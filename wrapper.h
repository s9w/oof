#pragma once

#include <string>
#include <optional>
#include <vector>

#include <cvt/wbuffer>


namespace cvtsw
{
   template<typename T>
   concept std_string_type = std::same_as<T, std::string> || std::same_as<T, std::wstring>;

   namespace detail {
      template<char ending, int param_count>
      struct param_holder {
         int m_start_params[param_count];

         template<typename ... Ts>
         constexpr explicit param_holder(const Ts... params);

         template<cvtsw::std_string_type string_type>
         operator string_type() const;

         template<cvtsw::std_string_type string_type>
         auto write_into(string_type& target) const;
      };

      using pos_params = param_holder<'H', 2>;
      using hpos_params = param_holder<'G', 1>;
      using vpos_params = param_holder<'d', 1>;
      using underline_params = param_holder<'m', 1>;
      using reset_params = param_holder<'m', 1>;
      using rgb_color_params = param_holder<'m', 5>;
      using enum_color_params = param_holder<'m', 1>;
   }
   

   [[nodiscard]] auto position(const int line, const int column) -> detail::pos_params;
   constexpr size_t   position_max = 3 + 2*3+1;

   [[nodiscard]] auto vposition(const int line) -> detail::vpos_params;
   constexpr size_t   vposition_max = 3 + 3;

   [[nodiscard]] auto hposition(const int column) -> detail::hpos_params;
   constexpr size_t   hposition_max = 3 + 3;

   [[nodiscard]] auto fg_color(const int r, const int g, const int b) -> detail::rgb_color_params;
   constexpr size_t   fg_color_max = 3 + 16;

   [[nodiscard]] auto bg_color(const int r, const int g, const int b) -> detail::rgb_color_params;
   constexpr size_t   bg_color_max = 3 + 16;

   [[nodiscard]] auto underline(const bool new_value = true) -> detail::underline_params;
   constexpr size_t   underline_max = 3 + 2;

   [[nodiscard]] auto reset_formatting() -> detail::reset_params;
   constexpr size_t   reset_max = 3 + 1;

   [[nodiscard]] auto reset_formatting()->detail::reset_params;

   enum class color_enum {black = 30, red, green, yellow, blue, magenta, cyan, white, reset = 39 };
   [[nodiscard]] auto fg_color(const color_enum color) -> detail::enum_color_params;

   struct color{
      uint8_t red = 0ui8;
      uint8_t green = 0ui8;
      uint8_t blue = 0ui8;
   };
   [[nodiscard]] constexpr auto operator==(const color& a, const color& b) -> bool;

   template<typename char_type>
   struct cell {
      char_type letter = ' ';
      bool underline = false;
      color fg_color;
      color bg_color;
   };

   template<typename char_type>
   struct screen{
      int m_width = 0;
      int m_height = 0;
      int m_origin_line = 0;
      int m_origin_column = 0;
      std::vector<cell<char_type>> m_cells;

      explicit screen(const int width, const int height, const int start_column, const int start_line, const char_type fill_char);
      [[nodiscard]] auto get_index(const int column, const int line) -> size_t;
      [[nodiscard]] auto get(const int column, const int line) -> cell<char_type>&;
      [[nodiscard]] auto is_inside(const int column, const int line) -> bool;
   };

   // Enables streaming and implicit conversation to strings
   namespace detail{
      template<typename char_type>
      struct magic {
         using string_type = std::basic_string<char_type>;
         string_type m_content;

         operator string_type() const;
      };
   }

   // First draw
   template<typename char_type>
   auto draw_screen(const screen<char_type>& new_screen) -> detail::magic<char_type>;

   // Update draw
   template<typename char_type>
   auto draw_screen(const screen<char_type>& old_screen, const screen<char_type>& new_screen) -> detail::magic<char_type>;
   

   namespace detail
   {
      template<typename stream_type, char ending, int param_count>
      auto operator<<(stream_type& os, const detail::param_holder<ending, param_count>& value) -> stream_type&;

      template<typename stream_type, typename char_type>
      auto operator<<(stream_type& os, const detail::magic<char_type>& value)->stream_type&;

      template<cvtsw::std_string_type string_type>
      auto reserve_size(string_type& target, const size_t needed_size) -> void;

      template<cvtsw::std_string_type string_type, char ending, int param_count>
      auto write_to_string(string_type& target, const detail::param_holder<ending, param_count>& value) -> void;

      template<int n>
      [[nodiscard]] constexpr auto get_reserve_size(const int(& ary)[n]) -> size_t;

      // std::to_string() or std::to_wstring() depending on the template type
      template<cvtsw::std_string_type string_type>
      auto to_xstring(string_type& target, const int value) -> void;
   }

} // namespace cvtsw


template<int n>
[[nodiscard]] constexpr auto cvtsw::detail::get_reserve_size(const int(& ary)[n])->size_t
{
   auto get_int_param_str_length = [](const int param) {
      if (param < 10)
         return 1;
      if (param < 100)
         return 2;
      return 3;
   };

   size_t reserve_size = 0;
   for (const int element : ary)
      reserve_size += get_int_param_str_length(element);

   reserve_size += n-1; // semicolons
   reserve_size += 3; // 2 intro, 1 outro
   return reserve_size;
}


template<cvtsw::std_string_type string_type>
auto cvtsw::detail::to_xstring(string_type& target, const int value) -> void
{
   if constexpr (std::same_as<string_type, std::string>)
      target += std::to_string(value);
   else
      target += std::to_wstring(value);
}


template<char ending, int param_count>
template<cvtsw::std_string_type string_type>
cvtsw::detail::param_holder<ending, param_count>::operator string_type() const
{
   string_type result;
   reserve_size(result, get_reserve_size(m_start_params));
   detail::write_to_string(result, *this);
   return result;
}


template <typename char_type>
cvtsw::detail::magic<char_type>::operator string_type() const
{
   return m_content;
}


template<char ending, int param_count>
template<cvtsw::std_string_type string_type>
auto cvtsw::detail::param_holder<ending, param_count>::write_into(string_type& target) const
{
   reserve_size(target, get_reserve_size(m_start_params));
   detail::write_to_string(target, *this);
}


template<cvtsw::std_string_type string_type, char ending, int param_count>
auto cvtsw::detail::write_to_string(
   string_type& target,
   const detail::param_holder<ending, param_count>& value
) -> void
{
   using char_type = typename string_type::value_type;
   if constexpr (std::same_as<string_type, std::string>)
      target += "\x1b[";
   else
      target += L"\x1b[";
   for (int i = 0; i < param_count; ++i)
   {
      to_xstring(target, value.m_start_params[i]);

      // Semicolon between parameters
      if (i != (param_count - 1))
         target += static_cast<char_type>(';');
   }
   target += ending;
}


template<cvtsw::std_string_type string_type>
auto cvtsw::detail::reserve_size(
   string_type& target,
   const size_t needed_size
) -> void
{
   const size_t capacity_left = target.capacity() - target.size();
   if (capacity_left < needed_size)
   {
      target.reserve(target.capacity() + needed_size);
   }
}


template<typename stream_type, char ending, int param_count>
auto cvtsw::detail::operator<<(
   stream_type& os,
   const param_holder<ending, param_count>& value
) -> stream_type&
{
   // Stream operation is _massively_ slow, therefore write to string and stream that in one go

   using char_type = typename stream_type::char_type;
   using string_type = std::basic_string<char_type>;

   string_type temp;
   detail::reserve_size(temp, detail::get_reserve_size(value.m_start_params));
   detail::write_to_string(temp, value);
   os << temp;
   return os;
}


template<typename stream_type, typename char_type>
auto cvtsw::detail::operator<<(
   stream_type& os,
   const magic<char_type>& value
) -> stream_type&
{
   os << value.m_content;
   return os;
}


template<char ending, int param_count>
template<typename ... Ts>
constexpr cvtsw::detail::param_holder<ending, param_count>::param_holder(
   const Ts... params
)
   : m_start_params{ params... }
{
   // assert all ints
   static_assert(sizeof...(Ts) == param_count, "Wrong number of parameters");
}



#ifdef CM_IMPL

auto cvtsw::position(const int line, const int column) -> detail::pos_params{
   const int effective_line = line + 1;
   const int effective_column = column + 1;
   return detail::pos_params{ effective_line, effective_column };
}

auto cvtsw::vposition(const int line)-> detail::vpos_params{
   const int effective_line = line + 1;
   return detail::vpos_params{ effective_line };
}

auto cvtsw::hposition(const int column) -> detail::hpos_params{
   const int effective_column = column + 1;
   return detail::hpos_params{ effective_column };
}

auto cvtsw::fg_color(const int r, const int g, const int b) -> detail::rgb_color_params{
   return detail::rgb_color_params{ 38, 2, r, g, b };
}

auto cvtsw::fg_color(const color_enum color) -> detail::enum_color_params{
   return detail::enum_color_params{ static_cast<int>(color) };
}

auto cvtsw::bg_color(const int r, const int g, const int b) -> detail::rgb_color_params{
   return detail::rgb_color_params{ 48, 2, r, g, b };
}

auto cvtsw::underline(const bool new_value) -> detail::underline_params{
   const int code = new_value == true ? 4 : 24;
   return detail::underline_params{ code };
}

auto cvtsw::reset_formatting() -> detail::reset_params{
   return detail::reset_params{ 0 };
}

#endif

constexpr auto cvtsw::operator==(const color& a, const color& b) -> bool
{
   return a.red == b.red && a.green == b.green && a.blue == b.blue;
}


template <typename char_type>
cvtsw::screen<char_type>::screen(const int width, const int height, const int start_column, const int start_line, const char_type fill_char)
   : m_width(width)
   , m_height(height)
   , m_origin_line(start_line)
   , m_origin_column(start_column)
   , m_cells(m_width * m_height, cell<char_type>{fill_char})
{
   
}

// TODO comparison with old screen not even in, omegalul

template<typename char_type>
auto cvtsw::draw_screen(const cvtsw::screen<char_type>& new_screen) -> detail::magic<char_type>
{
   std::basic_string<char_type> result_str;
   result_str.reserve(new_screen.m_width * new_screen.m_height * 10); // TODO this could be more sophisticated

   result_str += cvtsw::reset_formatting();
   result_str += cvtsw::position(new_screen.m_origin_line, new_screen.m_origin_column);

   bool is_first_cell = true;
   cell<char_type> last_state;
   int line = 0;
   int column = 0;
   for(int i=0; i<new_screen.m_cells.size(); ++i)
   {
      const cell<char_type>& cell = new_screen.m_cells[i];
      if(is_first_cell)
      {
         fg_color(cell.fg_color.red, cell.fg_color.green, cell.fg_color.blue).write_into(result_str);
         bg_color(cell.bg_color.red, cell.bg_color.green, cell.bg_color.blue).write_into(result_str);
         underline(cell.underline).write_into(result_str);
      }
      else {
         // Without reset, colors only have to be rewritten if they changed
         if (cell.fg_color != last_state.fg_color)
            fg_color(cell.fg_color.red, cell.fg_color.green, cell.fg_color.blue).write_into(result_str);
         if (cell.bg_color != last_state.bg_color)
            bg_color(cell.bg_color.red, cell.bg_color.green, cell.bg_color.blue).write_into(result_str);
         if (cell.underline != last_state.underline)
            underline(cell.underline).write_into(result_str);
      }

      // TODO this could be skipped / jumped over if no other format changes
      result_str += cell.letter;
      // result_str += '▄';
      ++column;
      if (column == new_screen.m_width) {
         position(line+1 + new_screen.m_origin_line, 0 + new_screen.m_origin_column).write_into(result_str);
         ++line;
         column = 0;
      }
      

      last_state = cell;
      is_first_cell = false;
   }
   return detail::magic<char_type>{result_str};
}



template<typename char_type>
auto cvtsw::screen<char_type>::get_index(const int column, const int line) -> size_t
{
   return line * m_width + column;
}



template<typename char_type>
auto cvtsw::screen<char_type>::get(const int column, const int line) -> cell<char_type>&
{
   return m_cells[get_index(column, line)];
}



template<typename char_type>
auto cvtsw::screen<char_type>::is_inside(const int column, const int line) -> bool
{
   return column >= 0 && column < m_width && line >= 0 && line < m_height;
}

