#include "doctest.h"

#include "../oof.h"
using namespace oof;

namespace {
   template<oof::sequence_c sequence_type>
   auto get_correct_size(const sequence_type& sequence) -> size_t {
      using string_type = std::conditional_t<std::is_same_v<sequence_type, wchar_sequence>, std::wstring, std::string>;
      string_type s;
      write_sequence_into_string(s, sequence);
      return s.size();
   }
}


TEST_CASE("write_int_to_string()")
{
   SUBCASE("std::string") {
      bool all_correct = true;
      for (int i = 0; i < 255; ++i) {
         std::string str;
         detail::write_int_to_string(str, i, false);
         if (str != std::to_string(i)) {
            all_correct = false;
         }
      }
      CHECK(all_correct);
   }

   SUBCASE("std::wstring") {
      bool all_correct = true;
      for (int i = 0; i < 255; ++i) {
         std::wstring str;
         detail::write_int_to_string(str, i, false);
         if (str != std::to_wstring(i)) {
            all_correct = false;
         }
      }
      CHECK(all_correct);
   }
}


TEST_CASE("get_sequence_string_size")
{
   constexpr auto has_correct_size = [](const auto& alternative) {
      const auto correct_size = get_correct_size(alternative);
      const auto calculated_size = detail::get_sequence_string_size(alternative);
      return correct_size == calculated_size;
   };

   CHECK(has_correct_size(reset_sequence{}));
   CHECK(has_correct_size(clear_screen_sequence{}));
   CHECK(has_correct_size(char_sequence{'A'}));
   CHECK(has_correct_size(wchar_sequence{L'A'}));
   CHECK(has_correct_size(position_sequence{0, 0}));
   CHECK(has_correct_size(position_sequence{11, 112}));
   CHECK(has_correct_size(hposition_sequence{1}));
   CHECK(has_correct_size(hposition_sequence{11}));
   CHECK(has_correct_size(vposition_sequence{ 1 }));
   CHECK(has_correct_size(vposition_sequence{ 11 }));
   CHECK(has_correct_size(underline_sequence{false}));
   CHECK(has_correct_size(underline_sequence{true}));
   CHECK(has_correct_size(bold_sequence{true}));
   CHECK(has_correct_size(cursor_visibility_sequence{false}));
   CHECK(has_correct_size(cursor_visibility_sequence{true}));
   CHECK(has_correct_size(move_left_sequence{1}));
   CHECK(has_correct_size(move_right_sequence{11}));
   CHECK(has_correct_size(fg_rgb_color_sequence{ color{10, 110, 6} }));
   CHECK(has_correct_size(fg_index_color_sequence{0}));
   CHECK(has_correct_size(fg_index_color_sequence{11}));
   CHECK(has_correct_size(set_index_color_sequence{ 1, color{1, 12, 255} }));
}
