# Oof (omnipotent output friend)
It's common for C++ programs to write output to the console. But consoles are far more capable than what they are usually used for. The magic lies in the so-called [Virtual Terminal sequences](https://docs.microsoft.com/en-us/windows/console/console-virtual-terminal-sequences) (sometimes also confusingly called ["escape codes"](https://en.wikipedia.org/wiki/ANSI_escape_code)): These cryptic character sequences allow complete control over position, color and other properties of written characters. *Oof* is a single C++20 header that wraps these in a convenient way.

```c++
for (int i = 0; i < 10; ++i){
   std::cout << oof::fg_color(oof::color{255 - i * 25});
   std::cout << oof::position(i, 2 * i) << std::to_string(i);
}
```
![123_example](https://user-images.githubusercontent.com/6044318/142816762-f1167a81-3d11-4b4a-85fc-d4edcdc06bf6.png)


```c++
constexpr double values[]{0.54, 0.88, 0.42, 0.21, 0.33, 0.68, 0.91};
for(int i=0; i<std::size(values); ++i){
   std::cout << oof::underline(true) << std::format("value {}", i) << oof::reset_formatting() << ": ";
   const oof::color color{ 255, static_cast<int>(255 - values[i] * 255), 0 };
   std::cout << oof::fg_color(color) << std::format("{:.2f}\n", values[i]) << oof::reset_formatting();
}
```
![values_example](https://user-images.githubusercontent.com/6044318/142819817-d1fc16fc-01e7-4a49-a908-33ed4b7a7c37.png)

On top of that, *oof* provides two special interfaces that heavily optimize the resulting stream of VT sequences, so that real-time outputs like those below are possible. The following videos are showcases - everything in them are letters in a console window:

https://user-images.githubusercontent.com/6044318/142469815-ce680909-9151-4322-85aa-01dc9ba29c1d.mp4

https://user-images.githubusercontent.com/6044318/142469820-f7af6525-d1ce-4d09-95c9-458297add315.mp4

https://user-images.githubusercontent.com/6044318/142469849-e359955d-fa3a-47d9-9a81-74905ee2f8fd.mp4

https://user-images.githubusercontent.com/6044318/142469871-39f34712-f05e-4f8a-818a-b023081c1eee.mp4

## Usage
To use the library, include `oof.h`. As with most header-only libraries, that include must be preceeded with `#define OOF_IMPL` in **one** cpp file. That way, the function implementations are only compiled once.

The simple interface consists of the functions below.

```c++
// Sets the foreground RGB color
auto fg_color(const color& col) -> fg_rgb_color_sequence;

// Sets the background RGB color
auto bg_color(const color& col) -> bg_rgb_color_sequence;

// Sets the foreground indexed color. Index must be in [1, 255]
auto fg_color(int index) -> fg_index_color_sequence;

// Sets the background indexed color. Index must be in [1, 255]
auto bg_color(int index) -> bg_index_color_sequence;

// Sets the indexed color. Index must be in [1, 255]
auto set_index_color(int index, const color& col) -> set_index_color_sequence;

// Sets the underline state
auto underline(bool new_value = true) -> underline_sequence;

// Sets the bold state. Warning: Bold is not supported by all console, see readme
auto bold(bool new_value = true) -> bold_sequence;

// Sets cursor visibility state. Recommended to turn off before doing real-time displays
auto cursor_visibility(bool new_value) -> cursor_visibility_sequence;

// Resets foreground- and background color, underline and bold state
auto reset_formatting() -> reset_sequence;

// Clears the screen
auto clear_screen() -> clear_screen_sequence;

// Sets the cursor position. Zero-based ie 0, 0 is first line, first column
auto position(int line, int column) -> position_sequence;
auto vposition(int line) -> vposition_sequence;
auto hposition(int column) -> hposition_sequence;

// Moves the cursor a certain amount
auto move_left (int amount) -> move_left_sequence;
auto move_right(int amount) -> move_right_sequence;
auto move_up   (int amount) -> move_up_sequence;
auto move_down (int amount) -> move_down_sequence;
```

Index colors are simply colors referred to by an index. The colors behind the indices can be set with `set_index_color()`.

All these functions return a magic type that can `operator<<` into `std::cout` and `std::wcout`. Example:
```c++
std::cout << oof::fg_color({ 255, 100, 100 }) << "This is red\n";
std::cout << "Still the same - state was changed!\n";
std::cout << oof::reset_formatting() << oof::hposition(10) << "All back to normal\n";
```

![example](https://user-images.githubusercontent.com/6044318/142437248-a999738c-2191-4ccc-be78-132685e2169c.png)

They also implicitly convert into `std::string` and `std::wstring` so you can build up your own strings with them.

The type [`oof::color`](https://github.com/s9w/oof/blob/master/oof.h#L12-L26) is mostly just a `struct color { uint8_t red{}, green{}, blue{}; }`. It does have convenience constructors for integer component parameters that get automatically `static_cast`ed into `uint8_t`. And it can be constructed with a single value, which will result in a grayscale color. You're encouraged to `std::bit_cast`, `reinterpret_cast` or `memcpy` your favorite 3-byte RGB color type into this.

## Performance and screen interfaces
Each printing command (regardless of wether it's `printf`, `std::cout` or something OS-specific) is pretty expensive. If performance is a priority, then consider building up your string first, and printing it in one go.

If you want real-time output, ie continuously changing what's on the screen, there's even more potential: By keeping track of the current screen state, *oof* avoids writing to cells that haven't changed. And: Changing the console cursor state (even without printing anything) is expensive. Avoiding unnecessary state changes is key. Both of these optimizations are implemented in the `screen` and `pixel_screen` classes.

With [`oof::screen`](https://github.com/s9w/oof/blob/master/oof.h#L147-L175) you define a rectangle in your console window and set the state of every single cell. Its `get_string()` and `write_string(string_type&)` methods then output an optimized string to achieve the desired state. This assumes that the user didn't interfere - so don't. The difference between `get_string()` and `write_string(string_type&)` is that the passed string will be reused to avoid allocating a new string. Almost always, the time to build up the string is tiny vs the time it takes to print, so don't worry about this too much.

Example for `oof::screen` usage:
```c++
oof::screen scr(10, 3, 0, 0, ' ');
for(uint64_t i=0; ; ++i){
   int j = 0;
   for (auto& cell : scr) {
      cell.m_letter = 'A' + (j + i) % 26;
      cell.m_format.m_bg_color.red = j * 8;
      ++j;
   }
   std::cout << scr.get_string();
}
```
![screen_example](https://user-images.githubusercontent.com/6044318/142577018-cc25f98e-0572-4179-ac65-ffa79964d25c.gif)

The API in general is pretty low level compared to [other](https://github.com/ArthurSonzogni/FTXUI) [libraries](https://github.com/ggerganov/imtui), focused on high performance and modularity. You're encouraged to use it to build your own components. A good example for this is [the horizontal bars demo](demos/bars_demo.cpp):

![bars_demo](https://user-images.githubusercontent.com/6044318/142583233-c026da81-815e-4486-9588-b02ecd9c6ac8.gif)

Consoles always write text, ie letters. With most fonts, a single letter or cell is much taller than wide. By using a very special character that exactly fills the upper half of a cell, the visible area gets effectively transformed into (almost) square pixels. Exactly that's done by the `pixel_screen` class. There you only set colors and give up control of the letters themselves. Note that that type often has `halfline` type parameters. That's due to the fact that a "pixel" is now just half a line high.

### `oof::pixel_screen`
Example for [`oof::pixel_screen`](https://github.com/s9w/oof/blob/master/oof.h#L191-L220) usage:
```c++
oof::pixel_screen screen(10, 10);
const auto t0 = std::chrono::high_resolution_clock::now();
while(true){
   const auto t1 = std::chrono::high_resolution_clock::now();
   const double seconds = std::chrono::duration<double>(t1-t0).count();

   for (oof::color& pixel : screen) {
      pixel.red   = 127.5 + 127.5 * std::sin(1.0 * seconds);
      pixel.green = 127.5 + 127.5 * std::sin(2.0 * seconds);
      pixel.blue  = 127.5 + 127.5 * std::sin(3.0 * seconds);
   }
   fast_print(screen.get_string());
}
```
![pixel_screen_example](https://user-images.githubusercontent.com/6044318/142581841-66a235d1-d1e8-4f02-b7e7-2c9889a321e6.gif)

The source code from the demo videos at the beginning is in this repo under [demos/](demos). That code uses a not-included and yet unreleased helper library (`s9w::`) for colors and math. But those aren't crucial if you just want to have a look.

## Notes
Consoles display text. Text is displayed via fonts. If you use letters that aren't included in your console font, that will result in visual artifacts - duh. This especially important for the `pixel_display` type, as it uses the mildly special [Block element](https://en.wikipedia.org/wiki/Block_Elements) '▀'. Some fonts may not have them included. Others do, but have them poorly aligned or sized - breaking up the even pixel grid.

This is a short overview of common monospace fonts and how well they are suited for "pixel" displays. Note that many are great in some sizes but ugly in others.

| | Font name |
|---|---|
| Great fonts | Fira Mono, Anka/Coder, Cascadia Code, Courier New, Hack, Lucida Console, Source Code pro, Pragmata Pro, Consolas |
| Mediocre | Inconsolata, Iosevka |
| Broken or awful | Input mono, Menlo, Office Code pro |

### Bold
The `bold` sequence is special. Some consoles ignore it entirely, some implement is as actual bold and others implement it as "bright" - slightly altering the colors of affected letters. Check yours before you use it. `cmd.exe` interprets it as bright. The Windows Terminal however correctly implements it as bold since v1.11, if you set `"intenseTextStyle": "bold"`. See [this article](https://devblogs.microsoft.com/commandline/windows-terminal-preview-1-11-release/#intense-text-style).

### Errors
You can provide an error function that gets called when something goes wrong. This mostly catches obvious usage errors (negative sizes, out of bounds indices etc), so feel free to ignore this. If you want, this is how:
```c++
auto my_error_function(const std::string& msg) -> void
{
   std::cerr << std::format("ERROR! msg: {}\n", msg);
   std::terminate();
}

oof::error_callback = my_error_function;
```

## Performance & OS-Specific details
Under windows, `printf` and `std::cout` are very slow. They're completely fine to use for static outputs but they're unsuitable real-time displays. It's much faster to use Windows own [WriteConsole()](https://docs.microsoft.com/en-us/windows/console/writeconsole) directly. A ready-to use wrapper that works for `std::string` and `std::wstring` would be:
```c++
template<typename char_type>
auto fast_print(const std::basic_string<char_type>& sss) -> void
{
   HANDLE const output_handle = GetStdHandle(STD_OUTPUT_HANDLE);
   const auto char_count = static_cast<DWORD>(sss.length());
   if constexpr (std::is_same_v<char_type, char>)
      WriteConsoleA(output_handle, sss.c_str(), char_count, nullptr, nullptr);
   else
      WriteConsoleW(output_handle, sss.c_str(), char_count, nullptr, nullptr);
}
```

If *oof* doesn't produce the output expected, it may be that the VT mode is not enabled. To enable it, this function can be used on windows:
```c++
auto enable_vt_mode() -> void
{
   HANDLE const handle = GetStdHandle(STD_OUTPUT_HANDLE);
   if (handle == INVALID_HANDLE_VALUE)
   {
      std::terminate(); // error handling
   }

   DWORD dwMode = 0;
   if (!GetConsoleMode(handle, &dwMode))
   {
      std::terminate(); // error handling
   }

   if (dwMode & ENABLE_VIRTUAL_TERMINAL_PROCESSING)
   {
      // VT mode is already enabled
      return;
   }

   dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
   if (!SetConsoleMode(handle, dwMode))
   {
      std::terminate(); // error handling
   }
}
```

- If you use `pixel_screen` or `screen<std::wstring>` in combination with `std::wcout`, you might not see the output. That's because unicode output might need some magic to enable. Either google that, or use the recommended `fast_print` above as it's faster and doesn't suffer from these problems.
- While the VT sequences are universal, not all consoles programs and operating systems may support them. I only have access to a windows machine so I can't make any claims on other operating systems.
- The [new Windows Terminal](https://github.com/microsoft/terminal) has some problems with irregular frame pacing. It will report high FPS but "feel" much choppier than good old `cmd.exe`.
