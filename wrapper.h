#pragma once

#include <string>

namespace cvtsw
{
   template<typename T>
   concept std_string_type = std::same_as<T, std::string> || std::same_as<T, std::wstring>;

   namespace detail {
      template<char ending, int param_count>
      struct super_type {
         int m_start_params[param_count];

         template<typename ... Ts>
         constexpr explicit super_type(const Ts... params);

         template<cvtsw::std_string_type string_type>
         operator string_type() const;
      };

      using pos_type = super_type<'H', 2>;
      using hpos_type = super_type<'G', 1>;
      using vpos_type = super_type<'d', 1>;
      using underline_type = super_type<'m', 1>;
      using reset_type = super_type<'m', 1>;
      using color_type = super_type<'m', 5>;
      using enum_color_type = super_type<'m', 1>;
   }

   template<typename stream_type, char ending, int param_count>
   auto operator<<(stream_type& os, const detail::super_type<ending, param_count>& value) -> stream_type&;

   [[nodiscard]] auto position(const int line, const int column) -> detail::pos_type;
   constexpr size_t   position_max = 3 + 2*3+1;

   [[nodiscard]] auto vposition(const int line) -> detail::vpos_type;
   constexpr size_t   vposition_max = 3 + 3;

   [[nodiscard]] auto hposition(const int column) -> detail::hpos_type;
   constexpr size_t   hposition_max = 3 + 3;

   [[nodiscard]] auto fg_color(const int r, const int g, const int b) -> detail::color_type;
   constexpr size_t   fg_color_max = 3 + 16;

   [[nodiscard]] auto bg_color(const int r, const int g, const int b) -> detail::color_type;
   constexpr size_t   bg_color_max = 3 + 16;

   [[nodiscard]] auto underline(const bool new_value = true) -> detail::underline_type;
   constexpr size_t   underline_max = 3 + 2;

   [[nodiscard]] auto reset_formatting() -> detail::reset_type;

   enum class color {black = 30, red, green, yellow, blue, magenta, cyan, white, reset = 39 };
   [[nodiscard]] auto fg_color(const color color) -> detail::enum_color_type;
   

   namespace detail
   {
      template<cvtsw::std_string_type string_type>
      auto reserve_size(string_type& target, const size_t needed_size) -> void;

      template<cvtsw::std_string_type string_type, char ending, int param_count>
      auto write_to_string(string_type& target, const detail::super_type<ending, param_count>& value) -> void;

      template<int n>
      [[nodiscard]] constexpr auto get_reserve_size(const int(& ary)[n]) -> size_t;

      // std::to_string() or std::to_wstring() depending on the template type
      template<cvtsw::std_string_type string_type, typename T>
      auto to_xstring(const T value) -> string_type;
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


template<cvtsw::std_string_type string_type, typename T>
auto cvtsw::detail::to_xstring(const T value) -> string_type
{
   if constexpr (std::same_as<string_type, std::string>)
      return std::to_string(value);
   else
      return std::to_wstring(value);
}


template<char ending, int param_count>
template<cvtsw::std_string_type string_type>
cvtsw::detail::super_type<ending, param_count>::operator string_type() const
{
   string_type result;
   reserve_size(result, get_reserve_size(m_start_params));
   detail::write_to_string(result, *this);
   return result;
}


template<cvtsw::std_string_type string_type, char ending, int param_count>
auto cvtsw::detail::write_to_string(string_type& target, const detail::super_type<ending, param_count>& value) -> void
{
   using char_type = typename string_type::value_type;
   if constexpr (std::same_as<string_type, std::string>)
      target += "\x1b[";
   else
      target += L"\x1b[";
   for (int i = 0; i < std::size(value.m_start_params); ++i)
   {
      target += to_xstring<string_type>(value.m_start_params[i]);

      // Semicolon between parameters
      if (i != (std::size(value.m_start_params) - 1))
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
auto cvtsw::operator<<(
   stream_type& os,
   const detail::super_type<ending, param_count>& value
) -> stream_type&
{
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
constexpr cvtsw::detail::super_type<ending, param_count>::super_type(
   const Ts... params
)
   : m_start_params{ params... }
{
   // assert all ints
   static_assert(sizeof...(Ts) == param_count, "Wrong number of parameters");
}



#ifdef CM_IMPL

auto cvtsw::position(const int line, const int column) -> detail::pos_type
{
   const int effective_line = line + 1;
   const int effective_column = column + 1;
   return detail::pos_type{ effective_line, effective_column };
}

auto cvtsw::vposition(const int line)->detail::vpos_type
{
   const int effective_line = line + 1;
   return detail::vpos_type{ effective_line };
}

auto cvtsw::hposition(const int column) -> detail::hpos_type
{
   const int effective_column = column + 1;
   return detail::hpos_type{ effective_column };
}

auto cvtsw::fg_color(const int r, const int g, const int b) -> detail::color_type
{
   return detail::color_type{ 38, 2, r, g, b };
}

auto cvtsw::fg_color(const color color) -> detail::enum_color_type
{
   return detail::enum_color_type{ static_cast<int>(color) };
}

auto cvtsw::bg_color(const int r, const int g, const int b) -> detail::color_type
{
   return detail::color_type{ 48, 2, r, g, b };
}

auto cvtsw::underline(const bool new_value) -> detail::underline_type
{
   const int code = new_value == true ? 4 : 24;
   return detail::underline_type{ code };
}

auto cvtsw::reset_formatting() -> detail::reset_type
{
   return detail::reset_type{ 0 };
}

#endif