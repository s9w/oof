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
   std::cout << choice << ": " << oof::fg_color({ 255, 100, 100 }) << desc << oof::reset_formatting() << "\n";
   ++choice;
}

int main()
{
   oof::error_callback = my_error_function;
   enable_vt_mode();
   std::cout << oof::cursor_visibility(false) << oof::reset_formatting() << oof::clear_screen();

   //std::cout << oof::fg_color({ 255, 100, 100 }) << "This is red\n";
   //std::cout << "Still the same - state was changed!\n";
   //std::cout << oof::reset_formatting() << oof::hposition(10) << "All back to normal\n";

   // oof::screen scr(10, 3, 0, 0, ' ');
   // for(uint64_t i=0; ; ++i){
   //    int j = 0;
   //    for (auto& cell : scr) {
   //       cell.m_letter = 'A' + (j + i) % 26;
   //       cell.m_format.m_bg_color.red = j * 8;
   //       ++j;
   //    }
   //    std::cout << scr.get_string();
   // }

   // oof::pixel_screen screen(10, 10);
   // const auto t0 = std::chrono::high_resolution_clock::now();
   // while(true){
   //    const auto t1 = std::chrono::high_resolution_clock::now();
   //    const double seconds = std::chrono::duration<double>(t1-t0).count();
   //
   //    for (oof::color& pixel : screen) {
   //       pixel.red   = 127.5 + 127.5 * std::sin(1.0 * seconds);
   //       pixel.green = 127.5 + 127.5 * std::sin(2.0 * seconds);
   //       pixel.blue  = 127.5 + 127.5 * std::sin(3.0 * seconds);
   //    }
   //    fast_print(screen.get_string());
   // }

   // 123 Example
   //for (int i = 0; i < 10; ++i) {
   //   std::cout << oof::fg_color(oof::color{ 255 - i * 25 });
   //   std::cout << oof::hposition(2 * i) << std::to_string(i) << "\n";
   //}

   // Values example
   //constexpr double values[]{0.54, 0.88, 0.42, 0.21, 0.33, 0.68, 0.91};
   //for(int i=0; i<std::size(values); ++i){
   //   std::cout << oof::underline(true) << std::format("value {}", i) << oof::reset_formatting() << ": ";
   //   const oof::color color{ 255, static_cast<int>(255 - values[i] * 255), 0 };
   //   std::cout << oof::fg_color(color) << std::format("{:.2f}\n", values[i]) << oof::reset_formatting();
   //}


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
