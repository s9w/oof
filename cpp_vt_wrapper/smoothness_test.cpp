#include  "smoothness_test.h"

#include "../wrapper.h"
using namespace cvtsw;

#include <cstdint>
#include <string>
#include <iostream>
#include <vector>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

auto get_cursor_pos_str(const int column, const int line) -> std::string{
   std::string result = "\x1b[";
   result += std::to_string(line+1);
   result += ";";
   result += std::to_string(column+1);
   result += "f";
   return result;
}

auto get_fg_color_str(const int r, const int g, const int b) -> std::string {
   std::string result = "\x1b[38;2;";
   result += std::to_string(r);
   result += ";";
   result += std::to_string(g);
   result += ";";
   result += std::to_string(b);
   result += "m";
   return result;
}

auto get_bg_color_str(const int r, const int g, const int b) -> std::string {
   std::string result = "\x1b[48;2;";
   result += std::to_string(r);
   result += ";";
   result += std::to_string(g);
   result += ";";
   result += std::to_string(b);
   result += "m";
   return result;
}

auto get_reset_str() -> std::string{
   return "\x1b[0m";
}

auto get_clear_string() -> std::string
{
   std::string result = get_cursor_pos_str(0, 0) + get_reset_str();
   result += std::string(120 * 20, ' ');
   return result;
}


auto smoothness_test() -> void{
   std::cout << "\x1b[?25l"; // no cursor

   std::string str;
   for (uint64_t frame = 0; ; ++frame) {
      // clear the screen
      str = get_clear_string();

      // Draw a white indicator in the first line, wandering left to right; i position per draw
      const int indicator_pos = frame % 80;
      str += get_cursor_pos_str(indicator_pos, 0);
      str += get_bg_color_str(255, 255, 255);
      str += ' ';
      str += get_reset_str();

      // Fill the rest of the screen with fast-moving changes (so they make things slow)
      str += get_cursor_pos_str(0, 1);
      for(int i=0; i<120*20; ++i){
         str += get_fg_color_str((frame+i)%256, 255, 255);
         str += 'A';
      }

      const auto char_count = static_cast<DWORD>(str.length());
      WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), str.c_str(), char_count, nullptr, 0);
   }
}