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
         int m_start_params[param_count];

         template<typename ... Ts>
         constexpr super_type(const Ts... params)
            : m_start_params{ params... }
         {}

         template<cvtsw::std_string_type string_type>
         operator string_type() const;

         auto write(std::string& target) const;
      };

      using pos_type = super_type<'H', 2>;

      //template<cvtsw::std_string_type string_type>
      auto operator<<(std::string& target, const char* value) -> std::string&;
      //template<cvtsw::std_string_type string_type>
      auto operator<<(std::wstring& target, const wchar_t* value) -> std::wstring&;

      template<cvtsw::std_string_type string_type, typename char_type = typename string_type::value_type>
      auto operator<<(string_type& target, const char_type value) -> string_type&;
      //auto operator<<(std::wstring& target, const wchar_t value) -> string_type&;
      template<cvtsw::std_string_type string_type>
      auto operator<<(string_type& target, const int value) -> string_type&;

      template<typename stream_type, char ending, int param_count>
      auto shift_impl(stream_type& os, const detail::super_type<ending, param_count>& value) -> stream_type&;
   }

   template<typename stream_type, char ending, int param_count>
   auto operator<<(stream_type& os, const detail::super_type<ending, param_count>& value) -> stream_type&;

   // CHA; Cursor Horizontal Absolute
   template<cvtsw::std_string_type string_type>
   auto write_horizontal_pos(string_type& target, const int column) -> void;
   template<cvtsw::std_string_type string_type = std::string>
   [[nodiscard]] auto get_horizontal_pos(const int column) -> string_type;

   // VPA; Vertical Line Position Absolute
   template<cvtsw::std_string_type string_type>
   auto write_vertical_pos(string_type& target, const int line) -> void;
   template<cvtsw::std_string_type string_type = std::string>
   [[nodiscard]] auto get_vertical_pos(const int line) -> string_type;

   // Zero-based, i.e. line=0, column=0 is top left
   //template<cvtsw::std_string_type string_type>
   //auto write_pos(string_type& target, const int line, const int column) -> void;
   //template<cvtsw::std_string_type string_type = std::string>
   //[[nodiscard]] auto get_pos(const int line, const int column) -> string_type;

   // Foreground RGB color
   template<cvtsw::std_string_type string_type>
   auto write_fg_rgb_color(string_type& target, const int r, const int g, const int b) -> void;
   template<cvtsw::std_string_type string_type = std::string>
   [[nodiscard]] auto get_fg_rgb_color(const int r, const int g, const int b) -> string_type;

   // Background RGB color
   template<cvtsw::std_string_type string_type>
   auto write_bg_rgb_color(string_type& target, const int r, const int g, const int b) -> void;
   template<cvtsw::std_string_type string_type = std::string>
   [[nodiscard]] auto get_bg_rgb_color(const int r, const int g, const int b)->string_type;

   // Resets colors and underline state
   template<cvtsw::std_string_type string_type>
   auto write_total_default(string_type& target) -> void;
   template<cvtsw::std_string_type string_type = std::string>
   [[nodiscard]] auto get_total_default() -> string_type;

   // Enables underline text
   template<cvtsw::std_string_type string_type>
   auto write_underline(string_type& target) -> void;
   template<cvtsw::std_string_type string_type = std::string>
   [[nodiscard]] auto get_underline() -> string_type;

   [[nodiscard]] auto position(const int line, const int column) -> detail::pos_type;

   template<cvtsw::std_string_type string_type>
   auto position(string_type& target, const int line, const int column) -> void;

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

      //template<typename char_type, typename stream_type = std::basic_ostream<char_type>, char ending, int param_count>
      //stream_type& operator<<(stream_type& os, const super_type<ending, param_count>& value);
      

      template<cvtsw::std_string_type string_type>
      constexpr auto intro_str_v = "\x1b[";
      template<>
      constexpr auto intro_str_v<std::wstring> = L"\x1b[";

      [[nodiscard]] constexpr auto get_param_str_length(const int param) -> size_t;

      template<typename ... Ts>
      [[nodiscard]] constexpr auto get_param_str_lengths(const Ts... ints) -> size_t;

      template<cvtsw::std_string_type string_type, typename T>
      auto write_generic_impl(string_type& target, const char ending, const T last) -> void;

      template<cvtsw::std_string_type string_type, typename T, typename ... Ts>
      auto write_generic_impl( string_type& target, const char ending, const T first, const Ts... rest) -> void;

      template<cvtsw::std_string_type string_type, typename ... Ts>
      auto write_generic(string_type& target, const char ending, const Ts... params) -> void;

      template<cvtsw::std_string_type string_type, typename ... Ts>
      [[nodiscard]] auto get_generic(const char ending, const Ts... params) -> string_type;

      template<cvtsw::std_string_type string_type>
      constexpr auto reserve_target(string_type& target, const size_t required_size) -> void;

      // std::to_string() or std::to_wstring() depending on the template type
      template<cvtsw::std_string_type string_type, typename T>
      auto to_xstring(const T value) -> string_type;
   }

} // namespace cvtsw


template<cvtsw::std_string_type string_type, typename T>
auto cvtsw::detail::write_generic_impl(
   string_type& target,
   const char ending,
   const T last
) -> void
{
   target += to_xstring<string_type>(last);
   target += static_cast<typename string_type::value_type>(ending);
}


template<cvtsw::std_string_type string_type, typename T, typename ... Ts>
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


template<cvtsw::std_string_type string_type, typename ... Ts>
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


template<cvtsw::std_string_type string_type, typename ... Ts>
auto cvtsw::detail::get_generic(
   const char ending,
   const Ts... params
) -> string_type
{
   string_type result;
   write_generic(result, ending, params...);
   return result;
}



template<cvtsw::std_string_type string_type>
auto cvtsw::write_horizontal_pos(string_type& target, const int column) -> void
{
   detail::write_generic(target, 'G', column + 1);
}


template<cvtsw::std_string_type string_type>
auto cvtsw::get_horizontal_pos(const int column) -> string_type
{
   string_type result;
   write_horizontal_pos(result, column);
   return result;
}


template<cvtsw::std_string_type string_type>
auto cvtsw::write_vertical_pos(string_type& target, const int line) -> void
{
   detail::write_generic(target, 'd', line + 1);
}


template<cvtsw::std_string_type string_type>
auto cvtsw::get_vertical_pos(const int line) -> string_type
{
   string_type result;
   write_vertical_pos(result, line);
   return result;
}


//template<cvtsw::std_string_type string_type>
//auto cvtsw::write_pos(string_type& target, const int line, const int column) -> void
//{
//   const int effective_line = line + 1;
//   const int effective_column = column + 1;
//   detail::write_generic(target, 'H', effective_line, effective_column);
//}
//
//
//template<cvtsw::std_string_type string_type>
//auto cvtsw::get_pos(const int line, const int column) -> string_type
//{
//   string_type result;
//   write_pos(result, line, column);
//   return result;
//}


template<cvtsw::std_string_type string_type>
auto cvtsw::write_fg_rgb_color(string_type& target, const int r, const int g, const int b) -> void
{
   detail::write_generic( target, 'm', 38, 2, r, g, b);
}


template<cvtsw::std_string_type string_type>
auto cvtsw::get_fg_rgb_color(const int r, const int g, const int b) -> string_type
{
   string_type result;
   write_fg_rgb_color(result, r, g, b);
   return result;
}


template<cvtsw::std_string_type string_type>
auto cvtsw::write_bg_rgb_color(string_type& target, const int r, const int g, const int b) -> void
{
   detail::write_generic(target, 'm', 48, 2, r, g, b);
}


template<cvtsw::std_string_type string_type>
auto cvtsw::get_bg_rgb_color(const int r, const int g, const int b) -> string_type
{
   string_type result;
   write_bg_rgb_color(result, r, g, b);
   return result;
}


template<cvtsw::std_string_type string_type>
auto cvtsw::write_total_default(string_type& target) -> void
{
   detail::write_generic(target, 'm', 0);
}


template<cvtsw::std_string_type string_type>
auto cvtsw::get_total_default() -> string_type
{
   string_type result;
   write_total_default(result);
   return result;
}


template<cvtsw::std_string_type string_type>
auto cvtsw::write_underline(string_type& target) -> void
{
   detail::write_generic(target, 'm', 4);
}


template<cvtsw::std_string_type string_type>
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


// maybe unite these two with a special concept
//template<cvtsw::std_string_type string_type>
auto cvtsw::detail::operator<<(
   std::string& target,
   const char* value
) -> std::string&
{
   target += value;
   return target;
}


//template<cvtsw::std_string_type string_type>
auto cvtsw::detail::operator<<(
   std::wstring& target,
   const wchar_t* value
   ) -> std::wstring&
{
   target += value;
   return target;
}


template<cvtsw::std_string_type string_type, typename char_type>
auto cvtsw::detail::operator<<(string_type& target, const char_type value)->string_type&
{
   // Necessary because otherwise he picks int overload
   target += value;
   return target;
}


template<cvtsw::std_string_type string_type>
auto cvtsw::detail::operator<<(
   string_type& target,
   const int value
) -> string_type&
{
   target += to_xstring<string_type>(value);
   return target;
}



template<char ending, int param_count>
template<cvtsw::std_string_type string_type>
cvtsw::detail::super_type<ending, param_count>::operator string_type() const
{
   string_type result;
   // TODO reserve
   detail::shift_impl(result, *this);
   return result;
}



template<typename stream_type, char ending, int param_count>
auto cvtsw::detail::shift_impl(
   stream_type& os,
   const detail::super_type<ending, param_count>& value
) -> stream_type&
{
   using char_type = super_char_type_t<stream_type>;
   using string_type = std::basic_string<char_type>;

   // This must be in detail because otherwise the string<< might leak into global namespace (with using decl)
   os << detail::intro_str_v<string_type>;
   for (int i = 0; i < std::size(value.m_start_params); ++i)
   {
      os << value.m_start_params[i];

      // Semicolon between parameters
      if (i != (std::size(value.m_start_params) - 1))
         os << static_cast<char_type>(';');
   }
   os << value.m_ending;
   return os;
}


template<typename stream_type, char ending, int param_count>
stream_type& cvtsw::operator<<(stream_type& os, const detail::super_type<ending, param_count>& value)
{
   return detail::shift_impl(os, value);
}

template<char ending, int param_count>
auto cvtsw::detail::super_type<ending, param_count>::write(std::string& target) const
{
   // TODO reserve
   detail::shift_impl(target, *this);
}


auto cvtsw::position(const int line, const int column) -> detail::pos_type
{
   const int effective_line = line + 1;
   const int effective_column = column + 1;
   return detail::pos_type{ effective_line, effective_column };
}


template<cvtsw::std_string_type string_type>
auto cvtsw::position(string_type& target, const int line, const int column) -> void
{
   return position(line, column).write(target);
}