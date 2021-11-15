#include "cursor_trail_demo.h"

#include "tools.h"


namespace {

   s9w::rng_state rng;

   // We're drawing "half" lines, so the relevant font size is halved in height
   auto get_halfline_font_pixel_size() -> s9w::ivec2
   {
      const s9w::ivec2 full_font_size = get_font_width_height();
      return s9w::ivec2{ full_font_size[0], full_font_size[1]/2 };
   }

   auto get_canvas_size() -> s9w::ivec2 {
      const s9w::ivec2 font_pixel_size = get_font_width_height();
      const s9w::ivec2 scree_cell_dimensions = get_screen_cell_dimensions();
      return scree_cell_dimensions * font_pixel_size;
   }

   
   auto get_window_rect() -> RECT{
      RECT window_rect{};
      GetWindowRect(GetConsoleWindow(), &window_rect);

      // Corrections, see https://devblogs.microsoft.com/oldnewthing/20131017-00/?p=2903
      RECT corrections{};
      const DWORD dwStyle = static_cast<DWORD>(GetWindowLong(GetConsoleWindow(), GWL_STYLE));
      const DWORD dwExStyle = static_cast<DWORD>(GetWindowLong(GetConsoleWindow(), GWL_EXSTYLE));
      constexpr bool fMenu = false;
      AdjustWindowRectEx(&corrections, dwStyle, fMenu, dwExStyle);

      window_rect.left -= corrections.left;
      window_rect.top -= corrections.top;
      window_rect.right -= corrections.right;
      window_rect.bottom -= corrections.bottom;
      return window_rect;
   }

   auto get_cursor_pixel_pos(const RECT& window_rect) -> s9w::ivec2 {
      POINT point;
      GetCursorPos(&point);
      
      return s9w::ivec2{
         static_cast<int>(point.x - window_rect.left),
         static_cast<int>(point.y - window_rect.top)
      };
   }


   // My go-to approximation of mapping SDF values to intensity
   auto get_sdf_intensity(const double sdf) -> double {
      return std::clamp(-sdf + 0.5, 0.0, 1.0);
   }

   auto get_max_color(
      const s9w::srgb_u& a,
      const s9w::srgb_u& b
   ) -> s9w::srgb_u
   {
      return s9w::srgb_u{
         std::max(a.r, b.r),
         std::max(a.g, b.g),
         std::max(a.b, b.b)
      };
   }

   // This might be overkill, but this correctly dims the lightness in an appropriate (non-sRGB) colorspace
   auto get_faded_color(const oof::color& col, const double fade_amount) -> oof::color{
      if (col == oof::color{})
         return oof::color{};
      s9w::oklab_d okl = s9w::convert_color<s9w::oklab_d>(std::bit_cast<s9w::srgb_u>(col));
      okl.L = std::clamp(okl.L - fade_amount, 0.0, 1.0);
      return std::bit_cast<oof::color>(s9w::convert_color<s9w::srgb_u>(okl));
   }

   auto get_draw_color(const double time) -> s9w::srgb_u {
      const double hue = std::fmod(50.0 * time, 360.0);
      constexpr double saturation = 100.0;
      constexpr double lightness = 50.0;
      const s9w::hsluv_d hsluv_color{ hue , saturation, lightness };
      return s9w::convert_color<s9w::srgb_u>(hsluv_color);
   }

} // namespace {}

auto cursor_trail_demo() -> void
{
   const s9w::ivec2 halfline_font_pixel_size = get_halfline_font_pixel_size();
   const s9w::ivec2 canvas_pixel_size = get_canvas_size();
   const s9w::ivec2 canvas_dimensions = canvas_pixel_size / halfline_font_pixel_size;

   const RECT window_rect = get_window_rect();
   oof::pixel_screen canvas(canvas_dimensions[0], canvas_dimensions[1], 0, 0, oof::color{});
   timer timer;
   double fade_amount = 0.0;

   while (true) {
      // Fading
      fade_amount += timer.get_dt();
      if (fade_amount >= 0.01) {
         for (oof::color& col : canvas)
            col = get_faded_color(col, fade_amount);
         fade_amount = 0.0;
      }

      const s9w::srgb_u draw_color = get_draw_color(timer.get_seconds_since_start());

      // Drawing of the circle with SDFs
      const s9w::dvec2 cursor_cell_pos = s9w::dvec2{ get_cursor_pixel_pos(window_rect) } / s9w::dvec2{ halfline_font_pixel_size };
      for (int halfline = 0; halfline < canvas.get_height(); ++halfline) {
         for (int column = 0; column < canvas.get_width(); ++column) {
            const s9w::dvec2 cell_pos = s9w::dvec2{column, halfline} + s9w::dvec2{0.5};

            constexpr double circle_radius = 5.0;
            const double circle_sdf = s9w::get_length(cell_pos - cursor_cell_pos) - circle_radius;

            const s9w::srgb_u effective_brush_color = get_sdf_intensity(circle_sdf) * draw_color;
            const s9w::srgb_u canvas_color = std::bit_cast<s9w::srgb_u>(canvas.get_color(column, halfline));
            const s9w::srgb_u result_color = get_max_color(canvas_color, effective_brush_color);
            canvas.get_color(column, halfline) = std::bit_cast<oof::color>(result_color);
         }
      }

      // Timing things; FPS; change of color
      timer.mark_frame();
      fast_print(canvas.get_string());

      const auto fps = timer.get_fps();
      if (fps.has_value())
         set_window_title("FPS: " + std::to_string(*fps));
   }
}
