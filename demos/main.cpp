#include <iostream>
#include <string>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include "tools.h"

#include "radar_demo.h"
#include "snow_demo.h"
#include "text_demo.h"
#include "bars_demo.h"
#include "cursor_trail_demo.h"
#include "fireworks_demo.h"


auto enable_vt_mode() -> void
{
   HANDLE const handle = GetStdHandle(STD_OUTPUT_HANDLE);
   if (handle == INVALID_HANDLE_VALUE)
      std::terminate(); // error handling

   DWORD dwMode = 0;
   if (!GetConsoleMode(handle, &dwMode))
      std::terminate(); // error handling

   if (dwMode & ENABLE_VIRTUAL_TERMINAL_PROCESSING)
      return; // VT mode is already enabled

   dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
   if (!SetConsoleMode(handle, dwMode))
      std::terminate(); // error handling
}

#include <format>

int main()
{
   enable_vt_mode();
   std::cout << oof::cursor_visibility(false) << oof::reset_formatting() << oof::clear_screen();

   //[[maybe_unused]] std::string s = std::format("abc {}", std::string(reset_formatting()));
   //s = reset_formatting();
   
    //bars_demo();
   //text_demo();

    radar_demo();
    //snow_demo();
    //cursor_trail_demo();
   //fireworks_demo();

    std::cout << oof::reset_formatting();
   return 0;
}
