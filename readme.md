# Oof (omnipotent output friend)
It's common for C++ programs to write output to the console. But consoles are far more capable than what they are usually used for. The magic lies in the so-called [Virtual Terminal sequences](https://docs.microsoft.com/en-us/windows/console/console-virtual-terminal-sequences) (sometimes also confusingly called ["escape codes"](https://en.wikipedia.org/wiki/ANSI_escape_code)): These cryptic character sequences allow complete control over position, color and other properties of written characters. Your omnipotent output friend (*oof*) wraps this in a single C++ header.

On top of that, *oof* provides two special interfaces that can apply two different optimizations to the resulting stream of VT sequences, so that real-time outputs like those below are possible. Note that everything in these videos are letters in a console window:



## Usage details
To use the library, include `oof.h` in your program. As with most header-only libraries, that include must be preceeded with `#define OOF_IMPL` in **one** cpp file. That way, the code is only compiled once and shouldn't meaningfully impact compile times.

The simple interface consists of the functions below. They return a magic type that can allow `operator<<` into `std::cout` and `std::wcout`. That type also implicitly converts into a `std::string` and `std::wstring` so you can build up your own strings with them. Printing these sequences is enough IF you have a look at the OS-specific details below.

```c++
// Sets the foreground RGB color
auto fg_color(const color& col) -> fg_rgb_color_sequence;

// Sets the foreground indexed color. Index must be in [1, 255]. You can define colors with set_index_color().
auto fg_color(const int index) -> fg_index_color_sequence;

// Sets the background RGB color
auto bg_color(const color& col) -> bg_rgb_color_sequence;

// Sets the background indexed color. Index must be in [1, 255]. You can define colors with set_index_color().
auto bg_color(const int index) -> bg_index_color_sequence;

// Sets the indexed color. Index must be in [1, 255].
auto set_index_color(const int index, const color& col) -> set_index_color_sequence;

// Sets the underline state
auto underline(const bool new_value = true) -> underline_sequence;

// Sets the bold state. Warning: Bold is not supported by all console, see readme
auto bold(const bool new_value = true) -> bold_sequence;

// Sets cursor visibility state. Recommended to turn off before doing real-time displays
auto cursor_visibility(const bool new_value) -> cursor_visibility_sequence;

// Resets foreground- and background color
auto reset_formatting()  -> reset_sequence;

// Clears the screen
auto clear_screen() -> clear_screen_sequence;

// Sets the cursor position. Zero-based
auto position(const int line, const int column) -> position_sequence;
auto vposition(const int line)                  -> vposition_sequence;
auto hposition(const int column)                -> hposition_sequence;

// Moves the cursor a certain amount
auto move_left (const int amount) -> move_left_sequence;
auto move_right(const int amount) -> move_right_sequence;
auto move_up   (const int amount) -> move_up_sequence;
auto move_down (const int amount) -> move_down_sequence;
```

Note that the formatting commands change the state of the console. It will be kept until it is changed again, or reset.

Also note that the `oof::color` type is just a `struct color { uint8_t red{}, green{}, blue{}; }`. You're encouraged to `std::bit_cast`, `reinterpret_cast` or `memcpy` your favorite 3-byte RGB color type into this.

## Performance and screen interfaces
Each printing command (regardless of wether it's `printf`, `std::cout` or something OS-specific) is relatively expensive. If performance is a priority, then consider building up your string first, and printing it in one go.

For real-time output, there is even more potential: If a program keeps track of the current state of the screen, it can avoid overriding cells that haven't changed. Even more: Changing the console cursor state (even without printing anything) is expensive. By avoiding unnecessary state changes, the performance can be optimized even more. Both of these optimizations are implemented in the `screen` and `pixel_screen` classes.

`oof::screen` lets you define a rectangle of your console window, and set the state of every single cell. Its `get_string()` and `write_string(string_type&)` methods then output an *optimal* string to achieve the desired state. This assumes that the user didn't interfere - so don't.

`oof::pixel_screen` does the same for a niche case. Consoles always write text, ie letters. With most fonts, a single letter or cell is much taller than wide. By using a very special character that exactly fills the upper half of a cell, the visible area gets effectively transformed into (almost) square pixels. Exactly that's done by the `pixel_screen` class. There you only set colors and give up control of the letters themselves. Note that that type often has `halfline` type parameters. That's due to the fact that a "pixel" is now just half a line high.

## Notes
Consoles display text. Text is displayed via fonts. If you use letters that aren't included in your console font, that will result in visual artifacts - duh. This especially important for the `pixel_display` type, as it uses the mildly special [Block element](https://en.wikipedia.org/wiki/Block_Elements) '▀'. Some fonts may not have them included. Others do, but have them poorly aligned or sized - breaking up the even pixel grid.

| | Font name |
|---|---|
| Great fonts | Fira Mono, Anka/Coder, Cascadia Code, Courier New, Hack, Lucida Console, Source Code pro, Pragmata Pro, Consolas |
| Mediocre | Inconsolata, Iosevka |
| Broken or awful | Input mono, Menlo, Office Code pro |

### Bold
The `bold` sequence is special. Some consoles ignore it entirely, some implement is as actual bold and others implement it as "bright" - slightly altering the colors of affected letters. Check yours before you use it. `cmd.exe` interprets it as bright. The Windows Terminal however correctly implements it as bold since v1.11, if you set it "intenseTextStyle": "bold". See [this article](https://devblogs.microsoft.com/commandline/windows-terminal-preview-1-11-release/#intense-text-style).

### Errors
You can provide an error function that gets called when something goes wrong. This mostly catches obvious usage errors (negative sizes, out of bounds indices etc), so feel free to ignore this. If you want, this is how:
```c++
auto my_error_function(
   const std::string& msg
) -> void
{
   std::cerr << std::format("ERROR! msg: {}\n", msg);
   std::terminate();
}

oof::error_callback = my_error_function;
```

## Performance & OS-Specific details
Under windows, `printf` and `std::cout` are very slow. They're completely fine to use for static outputs. But they're unsuitable real-time displays. It's much faster to use Windows own [WriteConsole()](https://docs.microsoft.com/en-us/windows/console/writeconsole) directly. A ready-to use wrapper that works for `std::string` and `std::wstring` would be:
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

Also note that while the VT sequences are universal, not all consoles programs and operating systems may support them. I only have access to a windows machine so I can't make any claims on other operating systems.

Be warned that the [new Windows Terminal](https://github.com/microsoft/terminal) has some problems with irregular frame pacing. It will report high FPS but "feel" much choppier than good old `cmd.exe`.

## Similar projects
Tere's [FXTUI](https://github.com/ArthurSonzogni/FTXUI) and [imtui](https://github.com/ggerganov/imtui) which write complete text-based user interfaces.

## TODO
- components to write higher-level compoennts
- font comparison
- box character link
- helper functions
