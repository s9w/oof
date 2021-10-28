#include "doctest.h"

#include "../wrapper.h"
using namespace cvtsw;


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
   SUBCASE("position access") {
      detail::cell_pos p0{ 10, 5 };
      CHECK_EQ(p0.get_column(), 0);
      CHECK_EQ(p0.get_line(), 0);

      ++p0;
      CHECK_EQ(p0.get_column(), 1);
      CHECK_EQ(p0.get_line(), 0);

      p0.m_index = 10;
      CHECK_EQ(p0.get_column(), 0);
      CHECK_EQ(p0.get_line(), 1);
   }

   SUBCASE("jumping")
   {
      const detail::cell_pos p0{ 10, 5 };
      detail::cell_pos jumped = p0 + 10;
      CHECK_EQ(jumped.get_column(), 0);
      CHECK_EQ(jumped.get_line(), 1);

      detail::cell_pos p1 = p0;
      p1.m_index = 10;
      CHECK_EQ(p1, jumped);
   }
}
