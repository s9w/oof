#pragma once

#include <string>

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

   enum class color {black = 30, red, green, yellow, blue, magenta, cyan, white, reset = 39 };
   [[nodiscard]] auto fg_color(const color color) -> detail::enum_color_params;
   

   namespace detail
   {
      template<typename stream_type, char ending, int param_count>
      auto operator<<(stream_type& os, const detail::param_holder<ending, param_count>& value)->stream_type&;

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
   int rest = value;
   if (rest >= 100) {
      target += '0' + (rest / 100);
      rest = rest % 100;
   }
   if (rest >= 10) {
      target += '0' + (rest / 10);
      rest = rest % 10;
   }
   target += '0' + rest;
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
   const detail::param_holder<ending, param_count>& value
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

auto cvtsw::fg_color(const color color) -> detail::enum_color_params{
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