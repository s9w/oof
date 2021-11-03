#include <optional>

#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest.h"

#define CM_IMPL
#include "../wrapper.h"

#include "perf_tests.h"

namespace {

}

auto run_doctest() -> std::optional<int> {
   doctest::Context context;
   const int res = context.run();
   if (context.shouldExit())
      return res;
   return std::nullopt;
}


auto main() -> int
{
   std::cout << "\x1b[?25l"; // no cursor

   const std::optional<int> doctest_result = run_doctest();
   if (doctest_result.has_value())
      return doctest_result.value();

   //run_perf_tests();
   return 0;
}

