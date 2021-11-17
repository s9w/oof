#include <iostream>

#include "tools.h"

#include "radar_demo.h"
#include "snow_demo.h"
#include "text_demo.h"
#include "bars_demo.h"
#include "cursor_trail_demo.h"
#include "fireworks_demo.h"


auto my_error_function(
   const std::string& msg
) -> void
{
   std::cerr << std::format("ERROR! msg: {}\n", msg);
   std::terminate();
}

auto print_choice(const char* desc)
{
   static int choice = 0;
   std::cout << choice << ": " << oof::fg_color(oof::color{ 255, 100, 100 }) << desc << oof::reset_formatting() << "\n";
   ++choice;
}

int main()
{
   oof::error_callback = my_error_function;
   enable_vt_mode();
   std::cout << oof::cursor_visibility(false) << oof::reset_formatting() << oof::clear_screen();

   std::cout << oof::fg_color(oof::color{ 255, 100, 100 }) << oof::underline() << "This is reddish and underlined\n";
   std::cout << "Still the same - state was changed!\n";
   std::cout << oof::reset_formatting() << oof::hposition(10) << "All back to normal\n";

   std::string s = oof::underline() + "abc";
   std::wcout << oof::underline() + L"def";

   int demo_choice = 0;
   print_choice("Bars");
   print_choice("Text crawl");
   print_choice("Radar");
   print_choice("Snow");
   print_choice("Cursor trail");
   print_choice("Fireworks");

   std::cout << "Choice: ";
   std::cin >> demo_choice;

   switch(demo_choice)
   {
   case 0:
      std::cout << oof::clear_screen();
      bars_demo();
      break;
   case 1:
      text_demo();
      break;
   case 2:
      radar_demo();
      break;
   case 3:
      snow_demo();
      break;
   case 4:
      std::cout << oof::clear_screen();
      cursor_trail_demo();
      break;
   case 5:
      fireworks_demo();
      break;
   default:
      std::cout << "really clever\n";
      break;
   }

   std::cout << oof::reset_formatting();
   return 0;
}
