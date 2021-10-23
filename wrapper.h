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

   template<typename string_type>
   auto write_rgb_color(string_type& target, const int r, const int g, const int b) -> void;

   template<typename string_type = std::string>
   [[nodiscard]] auto get_rgb_color(const int r, const int g, const int b) -> string_type;

   template<typename string_type>
   auto write_total_default(string_type& target) -> void;

   template<typename string_type = std::string>
   [[nodiscard]] auto get_total_default() -> string_type;

   namespace detail
   {
      template<typename string_type>
      constexpr auto reserve_target(string_type& target, const int required_size) -> void;
   }

} // namespace cvtsw


template<typename string_type>
auto cvtsw::write_horizontal_pos(string_type& target, const int column) -> void
{
   detail::reserve_target(target, 5);
   const int effective_pos = column + 1;
   if constexpr (std::same_as<string_type, std::string>)
   {
      target += "\x1b[";
      target += std::to_string(effective_pos);
      target += "G";
   }
   else
   {
      target += L"\x1b[";
      target += std::to_wstring(effective_pos);
      target += L"G";
   }
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
   detail::reserve_target(target, 5);
   const int effective_line = line + 1;
   if constexpr (std::same_as<string_type, std::string>)
   {
      target += "\x1b[";
      target += std::to_string(effective_line);
      target += "d";
   }
   else
   {
      target += L"\x1b[";
      target += std::to_wstring(effective_line);
      target += L"d";
   }
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
   // detail::reserve_target(target, 5);
   const int effective_line = line + 1;
   const int effective_column = column + 1;
   if constexpr (std::same_as<string_type, std::string>)
   {
      target += "\x1b[";
      target += std::to_string(effective_line);
      target += ";";
      target += std::to_string(effective_column);
      target += "H";
   }
   else
   {
      target += L"\x1b[";
      target += std::to_wstring(effective_line);
      target += L";";
      target += std::to_wstring(effective_column);
      target += L"H";
   }
}

template<typename string_type>
auto cvtsw::get_pos(const int line, const int column) -> string_type
{
   string_type result;
   write_pos(result, line, column);
   return result;
}


template<typename string_type>
auto cvtsw::write_rgb_color(string_type& target, const int r, const int g, const int b) -> void
{
   detail::reserve_target(target, 19);
   if constexpr (std::same_as<string_type, std::string>)
   {
      target += "\x1b[38;2;";
      target += std::to_string(r);
      target += ";";
      target += std::to_string(g);
      target += ";";
      target += std::to_string(b);
      target += "m";
   }
   else
   {
      target += L"\x1b[38;2;";
      target += std::to_wstring(r);
      target += L";";
      target += std::to_wstring(g);
      target += L";";
      target += std::to_wstring(b);
      target += L"m";
   }
}


template<typename string_type>
auto cvtsw::get_rgb_color(const int r, const int g, const int b) -> string_type
{
   string_type result;
   write_rgb_color(result, r, g, b);
   return result;
}


template<typename string_type>
auto cvtsw::write_total_default(string_type& target) -> void
{
   detail::reserve_target(target, 4);
   if constexpr (std::same_as<string_type, std::string>)
   {
      target += "\x1b[0m";
   }
   else
   {
      target += L"\x1b[0m";
   }
}

template<typename string_type>
auto cvtsw::get_total_default()->string_type
{
   string_type result;
   write_total_default(result);
   return result;
}


template<typename string_type>
constexpr auto cvtsw::detail::reserve_target(
   string_type& target,
   const int required_size
) -> void
{
   const size_t capacity_left = target.capacity() - target.size();
   if (capacity_left < required_size)
   {
      target.reserve(target.capacity() + required_size);
   }
}

