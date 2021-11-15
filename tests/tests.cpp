#include <optional>

#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest.h"

#define OOF_IMPL
#include "../oof.h"

namespace {
   auto run_doctest() -> std::optional<int> {
      doctest::Context context;
      const int res = context.run();
      if (context.shouldExit())
         return res;
      return std::nullopt;
   }
}


auto main() -> int
{
   std::cout << oof::cursor_visibility(false) << oof::reset_formatting() << oof::clear_screen_sequence();

   const std::optional<int> doctest_result = run_doctest();
   if (doctest_result.has_value())
      return doctest_result.value();

   return 0;
}

