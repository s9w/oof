#pragma once

#include <string>

namespace cvtsw
{
   template<typename T>
   concept std_string_type = std::same_as<T, std::string> || std::same_as<T, std::wstring>;

   namespace detail {
      template<char ending, int param_count>
      struct super_type {
         constexpr static char m_ending = ending;
         size_t m_reserve_size;
         int m_start_params[param_count];

         template<typename ... Ts>
         constexpr explicit super_type(const Ts... params);

         template<cvtsw::std_string_type string_type>
         operator string_type() const;

         template<cvtsw::std_string_type string_type>
         auto write(string_type& target) const;

         template<cvtsw::std_string_type string_type>
         auto reserve(string_type& target) const -> void;
      };

      using pos_type = super_type<'H', 2>;
      using hpos_type = super_type<'G', 1>;
      using vpos_type = super_type<'d', 1>;
      using underline_type = super_type<'m', 1>;
      using reset_type = super_type<'m', 1>;
      using color_type = super_type<'m', 5>;
      using enum_color_type = super_type<'m', 1>;

      template<cvtsw::std_string_type string_type, typename value_type>
      auto operator<<(string_type& target, const value_type value) -> string_type&;

      template<cvtsw::std_string_type string_type, char ending, int param_count>
      auto write_to_string(string_type& target, const detail::super_type<ending, param_count>& value) -> void;
   }

   template<typename stream_type, char ending, int param_count>
   auto operator<<(stream_type& os, const detail::super_type<ending, param_count>& value) -> stream_type&;


   template<cvtsw::std_string_type string_type>
   auto position(string_type& target, const int line, const int column) -> void;
   [[nodiscard]] auto position(const int line, const int column) -> detail::pos_type;

   template<cvtsw::std_string_type string_type>
   auto vposition(string_type& target, const int line) -> void;
   [[nodiscard]] auto vposition(const int line) -> detail::vpos_type;

   template<cvtsw::std_string_type string_type>
   auto hposition(string_type& target, const int column) -> void;
   [[nodiscard]] auto hposition(const int column) -> detail::hpos_type;

   template<cvtsw::std_string_type string_type>
   auto fg_color(string_type& target, const int r, const int g, const int b) -> void;
   [[nodiscard]] auto fg_color(const int r, const int g, const int b) -> detail::color_type;

   template<cvtsw::std_string_type string_type>
   auto bg_color(string_type& target, const int r, const int g, const int b) -> void;
   [[nodiscard]] auto bg_color(const int r, const int g, const int b)->detail::color_type;

   template<cvtsw::std_string_type string_type>
   auto underline(string_type& target, const bool new_value = true) -> void;
   [[nodiscard]] auto underline(const bool new_value = true) -> detail::underline_type;

   template<cvtsw::std_string_type string_type>
   auto reset_formatting(string_type& target) -> void;
   [[nodiscard]] auto reset_formatting() -> detail::reset_type;

   enum class color {
      black = 30, red, green, yellow, blue, magenta, cyan, white,
      reset = 39
   };
   template<cvtsw::std_string_type string_type>
   auto fg_color(string_type& target, const color color) -> void;
   [[nodiscard]] auto fg_color(const color color) -> detail::enum_color_type;
   

   namespace detail
   {
      template<typename T>
      struct super_char_type {};
      template<typename T>
      struct super_char_type<std::basic_ostream<T>> {
         using type = typename  std::basic_ostream<T>::char_type;
      };
      template<typename T>
      struct super_char_type<std::basic_string<T>> {
         using type = typename std::basic_string<T>::value_type;
      };
      template<typename T>
      using super_char_type_t = typename super_char_type<T>::type;

      template<cvtsw::std_string_type string_type>
      constexpr auto intro_str_v = "\x1b[";
      template<>
      constexpr auto intro_str_v<std::wstring> = L"\x1b[";

      [[nodiscard]] constexpr auto get_param_str_length(const int param) -> size_t;

      template<typename ... Ts>
      [[nodiscard]] constexpr auto get_reserve_size(const Ts... ints) -> size_t;

      template<cvtsw::std_string_type string_type>
      constexpr auto reserve_target(string_type& target, const size_t required_size) -> void;

      // std::to_string() or std::to_wstring() depending on the template type
      template<cvtsw::std_string_type string_type, typename T>
      auto to_xstring(const T value) -> string_type;
   }

} // namespace cvtsw


constexpr auto cvtsw::detail::get_param_str_length(const int param)->size_t
{
   if (param < 10)
      return 1;
   if (param < 100)
      return 2;
   return 3;
}


template<typename ... Ts>
constexpr auto cvtsw::detail::get_reserve_size(const Ts... ints) -> size_t
{
   const size_t param_str_len = (get_param_str_length(ints) + ...);
   constexpr size_t semicolon_size = sizeof...(Ts) - 1;
   constexpr size_t intro_size = 2;
   constexpr size_t outro_size = 1;
   const size_t reserve_size = intro_size + param_str_len + semicolon_size + outro_size;
   return reserve_size;
}


template<cvtsw::std_string_type string_type>
constexpr auto cvtsw::detail::reserve_target(
   string_type& target,
   const size_t required_size
) -> void
{
   const size_t capacity_left = target.capacity() - target.size();
   if (capacity_left < required_size)
   {
      target.reserve(target.capacity() + required_size);
   }
}


template<cvtsw::std_string_type string_type, typename T>
auto cvtsw::detail::to_xstring(const T value) -> string_type
{
   if constexpr (std::same_as<string_type, std::string>)
      return std::to_string(value);
   else
      return std::to_wstring(value);
}


template<cvtsw::std_string_type string_type, typename value_type>
auto cvtsw::detail::operator<<(
   string_type& target,
   const value_type value
) -> string_type&
{
   // There's three cases being covered by this:
   // 1) int parameters. Those need to be explicitly converted to the string type
   // 2) const char* / const wchar_t* for the starting character sequence. Those can just be appended
   // 3) char / wchar_t for semicolons. Also appended, but need to make sure this is not caught in the int overload.
   //    This does the trick.
   if constexpr (std::same_as<value_type, int>) {
      target += to_xstring<string_type>(value);
      return target;
   }
   else {
      target += value;
      return target;

   }
}


template<char ending, int param_count>
template<cvtsw::std_string_type string_type>
cvtsw::detail::super_type<ending, param_count>::operator string_type() const
{
   string_type result;
   reserve(result);
   detail::write_to_string(result, *this);
   return result;
}


template<cvtsw::std_string_type string_type, char ending, int param_count>
auto cvtsw::detail::write_to_string(string_type& target, const detail::super_type<ending, param_count>& value) -> void
{
   using char_type = typename string_type::value_type;
   target += intro_str_v<string_type>;
   for (int i = 0; i < std::size(value.m_start_params); ++i)
   {
      target += to_xstring<string_type>(value.m_start_params[i]);

      // Semicolon between parameters
      if (i != (std::size(value.m_start_params) - 1))
         target += static_cast<char_type>(';');
   }
   target += value.m_ending;
}


template<typename stream_type, char ending, int param_count>
 auto cvtsw::operator<<(
   stream_type& os,
   const detail::super_type<ending, param_count>& value
) -> stream_type&
{
   using char_type = detail::super_char_type_t<stream_type>;
   using string_type = std::basic_string<char_type>;

   string_type temp;
   value.reserve(temp);
   detail::write_to_string(temp, value);
   os << temp;
   return os;
}


template<char ending, int param_count>
template<cvtsw::std_string_type string_type>
auto cvtsw::detail::super_type<ending, param_count>::write(string_type& target) const
{
   reserve(target);
   detail::write_to_string(target, *this);
}


template<char ending, int param_count>
template<typename ... Ts>
constexpr cvtsw::detail::super_type<ending, param_count>::super_type(
   const Ts... params
)
   : m_start_params{ params... }
   , m_reserve_size(get_reserve_size(params...))
{
   static_assert(sizeof...(Ts) == param_count, "Wrong number of parameters");
}


template<char ending, int param_count>
template<cvtsw::std_string_type string_type>
auto cvtsw::detail::super_type<ending, param_count>::reserve(
   string_type& target
) const -> void
{
   detail::reserve_target(target, m_reserve_size);
}


template<cvtsw::std_string_type string_type>
auto cvtsw::position(
   string_type& target,
   const int line,
   const int column
) -> void
{
   return position(line, column).write(target);
}


template<cvtsw::std_string_type string_type>
auto cvtsw::vposition(string_type& target, const int line) -> void
{
   return vposition(line).write(target);
}


template<cvtsw::std_string_type string_type>
auto cvtsw::hposition(string_type& target, const int column) -> void
{
   return hposition(column).write(target);
}


template<cvtsw::std_string_type string_type>
auto cvtsw::fg_color(string_type& target, const int r, const int g, const int b) -> void
{
   return fg_color(r, g, b).write(target);
}

template<cvtsw::std_string_type string_type>
auto cvtsw::fg_color(string_type& target, const color color) -> void
{
   return fg_color(color).write(target);
}


template<cvtsw::std_string_type string_type>
auto cvtsw::bg_color(string_type& target, const int r, const int g, const int b) -> void
{
   return bg_color(r, g, b).write(target);
}


template<cvtsw::std_string_type string_type>
auto cvtsw::underline(string_type& target, const bool new_value) -> void
{
   return underline(new_value).write(target);
}


template<cvtsw::std_string_type string_type>
auto cvtsw::reset_formatting(string_type& target) -> void
{
   return underline().write(target);
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