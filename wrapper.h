#pragma once

#include <string>
#include <optional>
#include <vector>

#include <cvt/wbuffer>


namespace cvtsw
{
   template<typename T>
   using optional_ref = std::optional<std::reference_wrapper<const T>>;

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

   struct color {
      uint8_t red = 0ui8;
      uint8_t green = 0ui8;
      uint8_t blue = 0ui8;
      friend constexpr auto operator<=>(const color&, const color&) = default;
   };
   

   [[nodiscard]] auto position(const int line, const int column) -> detail::pos_params;
   constexpr size_t   position_max = 3 + 2*3+1;

   [[nodiscard]] auto vposition(const int line) -> detail::vpos_params;
   constexpr size_t   vposition_max = 3 + 3;

   [[nodiscard]] auto hposition(const int column) -> detail::hpos_params;
   constexpr size_t   hposition_max = 3 + 3;

   [[nodiscard]] auto fg_color(const int r, const int g, const int b) -> detail::rgb_color_params;
   [[nodiscard]] auto fg_color(const color& col) -> detail::rgb_color_params;
   constexpr size_t   fg_color_max = 3 + 16;

   [[nodiscard]] auto bg_color(const int r, const int g, const int b) -> detail::rgb_color_params;
   [[nodiscard]] auto bg_color(const color& col) -> detail::rgb_color_params;
   constexpr size_t   bg_color_max = 3 + 16;

   [[nodiscard]] auto underline(const bool new_value = true) -> detail::underline_params;
   constexpr size_t   underline_max = 3 + 2;

   [[nodiscard]] auto reset_formatting() -> detail::reset_params;
   constexpr size_t   reset_max = 3 + 1;

   [[nodiscard]] auto reset_formatting() -> detail::reset_params;

   enum class color_enum {black = 30, red, green, yellow, blue, magenta, cyan, white, reset = 39 };
   [[nodiscard]] auto fg_color(const color_enum color) -> detail::enum_color_params;



   template<typename char_type>
   struct cell {
      char_type letter{};
      bool underline = false;
      color fg_color;
      color bg_color;

      friend constexpr auto operator<=>(const cell&, const cell&) = default;
   };

   template<typename char_type>
   struct screen{
      using string_type = std::basic_string<char_type>;

      int m_width = 0;
      int m_height = 0;
      int m_origin_line = 0;
      int m_origin_column = 0;
      mutable std::optional<size_t> m_last_string_size;
      // mutable bool m_has_drawn = false;
      std::vector<cell<char_type>> m_cells;
      mutable std::vector<cell<char_type>> m_old_cells;

      explicit screen(const int width, const int height, const int start_column, const int start_line, const char_type fill_char);
      [[nodiscard]] auto get_index(const int column, const int line) const -> size_t;
      [[nodiscard]] auto get(const int column, const int line) -> cell<char_type>&;
      [[nodiscard]] auto is_inside(const int column, const int line) const -> bool;
      [[nodiscard]] auto get_string() const -> string_type;
   };

   struct pixel_screen {
      int m_width = 0; // Width is identical between "pixels" and characters
      int m_halfline_height = 0; // This refers to "pixel" height. Height in lines will be half that.
      int m_origin_column = 0;
      int m_origin_halfline = 0;
      std::vector<color> m_pixels;

      explicit pixel_screen(const int width, const int halfline_height, const int start_column, const int start_halfline, const color& fill_color);
      [[nodiscard]] auto get_screen(const color& frame_color) const -> screen<wchar_t>;
      [[nodiscard]] auto get_line_height() const -> int;
      [[nodiscard]] auto is_in(const int column, const int halfline) const -> bool;
      [[nodiscard]] auto get_color(const int column, const int halfline) const -> const color&;
      [[nodiscard]] auto get_color(const int column, const int halfline) -> color&;
   };
   

   namespace detail
   {
      template<typename stream_type, char ending, int param_count>
      auto operator<<(stream_type& os, const detail::param_holder<ending, param_count>& value) -> stream_type&;

      template<cvtsw::std_string_type string_type>
      auto reserve_size(string_type& target, const size_t needed_size) -> void;

      template<cvtsw::std_string_type string_type, char ending, int param_count>
      auto write_to_string(string_type& target, const detail::param_holder<ending, param_count>& value) -> void;

      template<int n>
      [[nodiscard]] constexpr auto get_reserve_size(const int(& ary)[n]) -> size_t;

      // std::to_string() or std::to_wstring() depending on the template type
      template<cvtsw::std_string_type string_type>
      auto to_xstring(string_type& target, const int value) -> void;

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
         constexpr cell_pos operator+(const int jump_amount) {
            cell_pos jumped(m_width, m_height);
            jumped.m_index = m_index + jump_amount;
            return jumped;
         }
         constexpr cell_pos& operator++() {
            ++m_index;
            return *this;
         }
         friend constexpr auto operator<=>(const cell_pos&, const cell_pos&) = default;
      };

      template<typename char_type>
      struct draw_state{
         using string_type = std::basic_string<char_type>;
         using cell_type = cell<char_type>;

         string_type& m_target_string;
         cell_pos m_last_written_pos;

         std::optional<bool> m_underline;
         std::optional<color> m_fg_color;
         std::optional<color> m_bg_color;
         
         explicit draw_state(string_type& target, const int width, const int height)
            : m_target_string(target)
            , m_last_written_pos(width, height)
         {}

         auto take_complete_cell_state(const cell_type& cell) {
            m_underline.emplace(cell.underline);
            m_fg_color.emplace(cell.fg_color);
            m_bg_color.emplace(cell.bg_color);
         }

         auto do_it(
            const cell_type& target_cell_state,
            const std::optional<std::reference_wrapper<const cell_type>>& old_cell_state,
            const cell_pos& target_pos,
            const int origin_line,
            const int origin_column
         ) -> void
         {
            if (target_cell_state == old_cell_state)
               return;

            // Console state changes necessary, when either
            //  1) There is no old state or
            //  2) Target state different than old AND current state not already correct
            if (old_cell_state.has_value() == false || (old_cell_state->get().fg_color != m_fg_color && target_cell_state.fg_color != m_fg_color))
            {
               fg_color(target_cell_state.fg_color).write_into(m_target_string);
            }
            if (old_cell_state.has_value() == false || (old_cell_state->get().bg_color != m_bg_color && target_cell_state.bg_color != m_bg_color))
            {
               bg_color(target_cell_state.bg_color).write_into(m_target_string);
            }
            if (old_cell_state.has_value() == false || (old_cell_state->get().underline != m_underline && target_cell_state.underline != m_underline))
            {
               underline(target_cell_state.underline).write_into(m_target_string);
            }

            // If a letter needs to be written can be neessary even if there are no console state changes.
            // What matters for this is only if the target state is different from the previous.
            if (target_cell_state != old_cell_state)
            {
               // Explicit position is only necessary if it isn't correct. And if there's a line jump.
               if (target_pos != (m_last_written_pos + 1) || target_pos.get_column() == 0) {
                  cvtsw::position(
                     target_pos.get_line() + origin_line,
                     target_pos.get_column() + origin_column
                  ).write_into(m_target_string);
               }
               m_target_string += target_cell_state.letter;
               m_last_written_pos = target_pos;
            }
            take_complete_cell_state(target_cell_state);
         }
      };

      template<typename char_type>
      [[nodiscard]] auto get_screen_string(const screen<char_type>& scr) -> std::basic_string<char_type>;
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

auto cvtsw::fg_color(const color& col) -> detail::rgb_color_params {
   return detail::rgb_color_params{ 38, 2, col.red, col.green, col.blue };
}

auto cvtsw::fg_color(const color_enum color) -> detail::enum_color_params{
   return detail::enum_color_params{ static_cast<int>(color) };
}

auto cvtsw::bg_color(const int r, const int g, const int b) -> detail::rgb_color_params{
   return detail::rgb_color_params{ 48, 2, r, g, b };
}

auto cvtsw::bg_color(const color& col) -> detail::rgb_color_params {
   return detail::rgb_color_params{ 48, 2, col.red, col.green, col.blue };
}

auto cvtsw::underline(const bool new_value) -> detail::underline_params{
   const int code = new_value == true ? 4 : 24;
   return detail::underline_params{ code };
}

auto cvtsw::reset_formatting() -> detail::reset_params{
   return detail::reset_params{ 0 };
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
   , m_pixels(m_width * m_halfline_height, fill_color)
{

}


auto cvtsw::pixel_screen::get_screen(const color& frame_color) const -> screen<wchar_t>
{
   screen<wchar_t> result{ m_width, get_line_height(), m_origin_column, m_origin_halfline/2, L'▀' };
   // This means fg color is on top

   int halfline_top = (m_origin_halfline % 2 == 0) ? 0 : -1;
   int halfline_bottom = halfline_top + 1;
   for(int line=0; line<result.m_height; ++line){
      for (int column = 0; column<result.m_width; ++column){
         cell<wchar_t>& target_cell = result.get(column, line);
         target_cell.fg_color = is_in(column, halfline_top) ? get_color(column, halfline_top) : frame_color;
         target_cell.bg_color = is_in(column, halfline_bottom) ? get_color(column, halfline_bottom) : frame_color;
      }
      halfline_top += 2;
      halfline_bottom += 2;
   }
   return result;
}


auto cvtsw::pixel_screen::get_line_height() const -> int
{
   const int first_line = m_origin_halfline / 2;
   const int last_line = (m_origin_halfline -1 + m_halfline_height) / 2;
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


template<typename char_type>
auto cvtsw::detail::get_screen_string(
   const screen<char_type>& scr
) -> std::basic_string<char_type>
{
   std::basic_string<char_type> result_str;

   // Reserving. TODO this could be more sophisticated
   if (scr.m_last_string_size.has_value())
      result_str.reserve(scr.m_last_string_size.value() * 2);
   else
      result_str.reserve(scr.m_width * scr.m_height * 10);

   cvtsw::reset_formatting().write_into(result_str);

   draw_state<char_type> state{ result_str, scr.m_width, scr.m_height };

   for (cell_pos relative_pos{scr.m_width, scr.m_height}; relative_pos.is_end() == false; ++relative_pos)
   {
      const cell<char_type>& target_cell_state = scr.m_cells[relative_pos.m_index];
      
      std::optional<std::reference_wrapper<const cell<char_type>>> old_cell_state;
      if (scr.m_old_cells.empty() == false)
         old_cell_state.emplace(scr.m_old_cells[relative_pos.m_index]);

      state.do_it(
         target_cell_state, old_cell_state,
         relative_pos,
         scr.m_origin_line, scr.m_origin_column
      );
   }
   return result_str;
}


template <typename char_type>
cvtsw::screen<char_type>::screen(
   const int width, const int height,
   const int start_column, const int start_line,
   const char_type fill_char
)
   : m_width(width)
   , m_height(height)
   , m_origin_line(start_line)
   , m_origin_column(start_column)
   , m_cells(m_width * m_height, cell<char_type>{fill_char})
{
   
}


template<typename char_type>
auto cvtsw::screen<char_type>::get_string() const -> string_type
{
   string_type result = detail::get_screen_string(*this);
   m_last_string_size = result.size();
   m_old_cells = m_cells;
   return result;
}


template<typename char_type>
auto cvtsw::screen<char_type>::get_index(const int column, const int line) const -> size_t
{
   return line * m_width + column;
}



template<typename char_type>
auto cvtsw::screen<char_type>::get(const int column, const int line) -> cell<char_type>&
{
   return m_cells[get_index(column, line)];
}



template<typename char_type>
auto cvtsw::screen<char_type>::is_inside(const int column, const int line) const -> bool
{
   return column >= 0 && column < m_width && line >= 0 && line < m_height;
}

