# oof (omnipotent output friend)
## Intro
It's common for C++ programs to write to the console. But consoles are far more capable than what they are usually used for. The magic lies in the so-called [Virtual Terminal sequences](https://docs.microsoft.com/en-us/windows/console/console-virtual-terminal-sequences) (sometimes also confusingly called ["escape codes"](https://en.wikipedia.org/wiki/ANSI_escape_code)): These are cryptic character sequences that allow complete control over position, color and other style aspects of written characters. *Oof* wraps this functionality in a single C++ header.

On top of that it provides two special interfaces that optimize the resulting VT sequences heavily so that real-time high-fidelity outputs like these are possible. Note that everything in these videos are letters in a console window:


## Usage details
- components to write higher-level compoennts


Oof uses a simple color type with three `uint8_t` components. Since many codebases use their own code for colors, it's allowed and encouraged to `std::bit_cast`, `reinterpret_cast` or `memcpy` your own three-byte color type into this.

Bold/underline TODO

## Performance & OS-Specific details
Under windows, `printf` and `std::cout` are implemented very inefficiently. Using them for real-time displays is not recommended. It's much faster to use windows own [WriteConsole()](https://docs.microsoft.com/en-us/windows/console/writeconsole) directly. A ready-to use wrapper that works for `std::string` and `std::wstring` would be:
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
- error function
- 