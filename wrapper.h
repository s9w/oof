#pragma once

#include <string>

#include <windows.h>

namespace cvtsw {
   
   auto enable_vt_mode(HANDLE handle) -> void;

   template<typename string_type>
   auto write_horizontal_pos(string_type& target, const int pos) -> void;

   template<typename string_type = std::string>
   [[nodiscard]] auto get_horizontal_pos(const int pos) -> string_type;

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
auto cvtsw::write_horizontal_pos(string_type& target, const int pos) -> void
{
   detail::reserve_target(target, 5);
   if constexpr (std::same_as<string_type, std::string>)
   {
      target += "\x1b[";
      target += std::to_string(pos);
      target += "G";
   }
   else
   {
      target += L"\x1b[";
      target += std::to_wstring(pos);
      target += L"G";
   }
}


template<typename string_type>
auto cvtsw::get_horizontal_pos(const int pos) -> string_type
{
   string_type result;
   write_horizontal_pos(result, pos);
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


auto cvtsw::enable_vt_mode(HANDLE handle) -> void
{
   HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
   if (handle == INVALID_HANDLE_VALUE)
   {
      std::terminate();
   }

   DWORD dwMode = 0;
   if (!GetConsoleMode(hOut, &dwMode))
   {
      std::terminate();
   }

   const bool is_enabled = dwMode & ENABLE_VIRTUAL_TERMINAL_PROCESSING;
   if (is_enabled)
   {
      return;
   }

   dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
   if (!SetConsoleMode(hOut, dwMode))
   {
      std::terminate();
   }
}

