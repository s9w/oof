#include "doctest.h"

#include "../oof.h"
using namespace oof;


TEST_CASE("cell_pos")
{
   SUBCASE("is_end") {
      detail::cell_pos p0{ 10, 5 };
      CHECK_FALSE(p0.is_end());
      p0.m_index = 49;
      CHECK_FALSE(p0.is_end());
      ++p0;
      CHECK(p0.is_end());
   }
}
