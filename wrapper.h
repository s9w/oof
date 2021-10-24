#pragma once

#include <string>

namespace cvtsw
{

   // CHA; Cursor Horizontal Absolute
   template<typename string_type>
   auto write_horizontal_pos(string_type& target, const int column) -> void;
   template<typename string_type = std::string>
   [[nodiscard]] auto get_horizontal_pos(const int column) -> string_type;

   // VPA; Vertical Line Position Absolute
   template<typename string_type>
   auto write_vertical_pos(string_type& target, const int line) -> void;
   template<typename string_type = std::string>
   [[nodiscard]] auto get_vertical_pos(const int line) -> string_type;

   // Zero-based, i.e. line=0, column=0 is top left
   template<typename string_type>
   auto write_pos(string_type& target, const int line, const int column) -> void;
   template<typename string_type = std::string>
   [[nodiscard]] auto get_pos(const int line, const int column) -> string_type;

   // Foreground RGB color
   template<typename string_type>
   auto write_fg_rgb_color(string_type& target, const int r, const int g, const int b) -> void;
   template<typename string_type = std::string>
   [[nodiscard]] auto get_fg_rgb_color(const int r, const int g, const int b) -> string_type;

   // Background RGB color
   template<typename string_type>
   auto write_bg_rgb_color(string_type& target, const int r, const int g, const int b) -> void;
   template<typename string_type = std::string>
   [[nodiscard]] auto get_bg_rgb_color(const int r, const int g, const int b)->string_type;

   // Resets colors and underline state
   template<typename string_type>
   auto write_total_default(string_type& target) -> void;
   template<typename string_type = std::string>
   [[nodiscard]] auto get_total_default() -> string_type;

   template<typename string_type>
   auto write_underline(string_type& target) -> void;
   template<typename string_type = std::string>
   [[nodiscard]] auto get_underline() -> string_type;

   namespace detail
   {
      template<typename string_type>
      constexpr auto intro_str_v = "\x1b[";
      template<>
      constexpr auto intro_str_v<std::wstring> = L"\x1b[";

      [[nodiscard]] constexpr auto get_param_str_length(const int param) -> size_t;

      template<typename ... Ts>
      [[nodiscard]] constexpr auto get_param_str_lengths(const Ts... ints) -> size_t;

      template<typename string_type, typename T>
      auto write_generic_impl(string_type& target, const char ending, const T last) -> void;

      template<typename string_type, typename T, typename ... Ts>
      auto write_generic_impl( string_type& target, const char ending, const T first, const Ts... rest) -> void;

      template<typename string_type, typename ... Ts>
      auto write_generic(string_type& target, const char ending, const Ts... params) -> void;

      template<typename string_type, typename ... Ts>
      [[nodiscard]] auto get_generic(const char ending, const Ts... params) -> string_type;

      template<typename string_type>
      constexpr auto reserve_target(string_type& target, const size_t required_size) -> void;

      // std::to_string() or std::to_wstring() depending on the template type
      template<typename string_type, typename T>
      auto to_xstring(const T value) -> string_type;
   }

} // namespace cvtsw


template<typename string_type, typename T>
auto cvtsw::detail::write_generic_impl(
   string_type& target,
   const char ending,
   const T last
) -> void
{
   target += to_xstring<string_type>(last);
   target += static_cast<typename string_type::value_type>(ending);
}


template<typename string_type, typename T, typename ... Ts>
auto cvtsw::detail::write_generic_impl(
   string_type& target,
   const char ending,
   const T first,
   const Ts... rest
) -> void
{
   target += to_xstring<string_type>(first);
   target += static_cast<typename string_type::value_type>(';');
   write_generic_impl(target, ending, rest...);
}


template<typename string_type, typename ... Ts>
auto cvtsw::detail::write_generic(
   string_type& target,
   const char ending,
   const Ts... params
) -> void
{
   static_assert((std::same_as<Ts, int> && ...), "Parameters are not ints.");
   {
      const size_t param_string_size = get_param_str_lengths(params...);
      constexpr size_t semicolon_size = sizeof...(Ts) - 1;
      constexpr size_t intro_size = 2;
      constexpr size_t outro_size = 1;
      const size_t reserve_size = intro_size + param_string_size + semicolon_size + outro_size;
      detail::reserve_target(target, reserve_size);
   }
   target += intro_str_v<string_type>;

   write_generic_impl(target, ending, params...);
}


template<typename string_type, typename ... Ts>
auto cvtsw::detail::get_generic(
   const char ending,
   const Ts... params
) -> string_type
{
   string_type result;
   write_generic(result, ending, params...);
   return result;
}



template<typename string_type>
auto cvtsw::write_horizontal_pos(string_type& target, const int column) -> void
{
   detail::write_generic(target, 'G', detail::to_xstring<string_type>(column + 1).c_str());
}


template<typename string_type>
auto cvtsw::get_horizontal_pos(const int column) -> string_type
{
   string_type result;
   write_horizontal_pos(result, column);
   return result;
}


template<typename string_type>
auto cvtsw::write_vertical_pos(string_type& target, const int line) -> void
{
   detail::write_generic(target, 'd', detail::to_xstring<string_type>(line + 1).c_str());
}


template<typename string_type>
auto cvtsw::get_vertical_pos(const int line) -> string_type
{
   string_type result;
   write_vertical_pos(result, line);
   return result;
}


template<typename string_type>
auto cvtsw::write_pos(string_type& target, const int line, const int column) -> void
{
   const int effective_line = line + 1;
   const int effective_column = column + 1;
   detail::write_generic(target, 'H', detail::to_xstring<string_type>(effective_line).c_str(), detail::to_xstring<string_type>(effective_column).c_str());
}


template<typename string_type>
auto cvtsw::get_pos(const int line, const int column) -> string_type
{
   string_type result;
   write_pos(result, line, column);
   return result;
}


template<typename string_type>
auto cvtsw::write_fg_rgb_color(string_type& target, const int r, const int g, const int b) -> void
{
   detail::write_generic( target, 'm', 38, 2, r, g, b);
}


template<typename string_type>
auto cvtsw::get_fg_rgb_color(const int r, const int g, const int b) -> string_type
{
   string_type result;
   write_fg_rgb_color(result, r, g, b);
   return result;
}


template<typename string_type>
auto cvtsw::write_bg_rgb_color(string_type& target, const int r, const int g, const int b) -> void
{
   detail::write_generic(target, 'm', 48, 2, r, g, b);
}


template<typename string_type>
auto cvtsw::get_bg_rgb_color(const int r, const int g, const int b) -> string_type
{
   string_type result;
   write_bg_rgb_color(result, r, g, b);
   return result;
}


template<typename string_type>
auto cvtsw::write_total_default(string_type& target) -> void
{
   detail::write_generic(target, 'm', 0);
}


template<typename string_type>
auto cvtsw::get_total_default() -> string_type
{
   string_type result;
   write_total_default(result);
   return result;
}


template<typename string_type>
auto cvtsw::write_underline(string_type& target) -> void
{
   detail::write_generic(target, 'm', 4);
}


template<typename string_type>
auto cvtsw::get_underline() -> string_type
{
   string_type result;
   write_underline(result);
   return result;
}


constexpr auto cvtsw::detail::get_param_str_length(const int param)->size_t
{
   if (param < 10)
      return 1;
   if (param < 100)
      return 2;
   return 3;
}


template <typename ... Ts>
constexpr auto cvtsw::detail::get_param_str_lengths(const Ts... ints) -> size_t
{
   return (get_param_str_length(ints) + ...);
}


template<typename string_type>
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


template<typename string_type, typename T>
auto cvtsw::detail::to_xstring(const T value) -> string_type
{
   if constexpr (std::same_as<string_type, std::string>)
      return std::to_string(value);
   else
      return std::to_wstring(value);
}
