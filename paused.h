#ifndef PAUSED_H_
#define PAUSED_H_

#define FMT_HEADER_ONLY
#include <string_view>
#include <fmt/base.h>
#include <charconv>
#include <optional>
#include <string>

namespace AWML {
    inline void paused(std::string_view msg, std::string_view cexit);
    inline void paused(std::string_view msg);
    inline void paused();
};

class Stop {
private:
    friend inline void AWML::paused(std::string_view msg, std::string_view cexit);
    friend inline void AWML::paused(std::string_view msg);
    friend inline void AWML::paused();

    static inline std::optional<int> Parse_Cexit(std::string_view str) noexcept;
    static inline std::string find_string(std::string_view str) noexcept;
    static inline constexpr std::string_view \
    CSimplified{"\u8BF7\u6309\u4EFB\u610F\u952E\u7EE7\u7EED. . ."},
    CTraditional{"\u8ACB\u6309\u4EFB\u610F\u9375\u7E7C\u7E8C. . ."},
    English{"Press any key to continue. . ."};

    static inline std::string GetSystemCodePage() noexcept;
    static inline void getch() noexcept;
    static inline std::string GetSystemLanguage() noexcept;

    static inline void paused(std::string_view msg, std::string_view cexit);
    static inline void paused(std::string_view msg);
    static inline void paused();
};

/** AWML - API
 * A -> Automatic Derivation Platform
 * W -> Windows
 * M -> Macos
 * L -> Linux
**/
namespace AWML {
    /**
    * Pauses program execution and waits for keypress, supporting custom prompts and exit commands.
    * Displays user-defined prompt text or exit message when required.
    * Control command string - if formatted as "exit(number)", terminates program with specified exit code;
    * otherwise treated as custom prompt text.
    **/
    inline void paused(std::string_view msg, std::string_view cexit) {Stop::paused(msg, cexit);}
    /**
    * Pauses program execution and waits for a keypress, displaying system-default prompt message based on OS language.
    * Control command string - if formatted as "exit(number)", terminates program with specified exit code;
    * otherwise treated as custom prompt text to display!
    **/
    inline void paused(std::string_view msg) {Stop::paused(msg);}
    /**
    * Pauses program execution and waits for a keypress, displaying system language-specific default prompt message
    * No-parameter version, only shows the default "Press any key to continue" prompt
    **/
    inline void paused() {Stop::paused();}
};

inline std::optional<int> Stop::Parse_Cexit(std::string_view str) noexcept {
    int value;
    constexpr std::string_view prefix = "exit(";
    if (str.compare(0, prefix.size(), prefix) != 0 || str.back() != ')')
        return std::nullopt;
    const size_t num_beg = prefix.size();
    const size_t num_len = str.size() - num_beg - 1;
    if (num_len == 0) return std::nullopt;
    const char* first = str.data() + num_beg;
    const char* last  = first + num_len;
    auto [ptr, ec] = std::from_chars(first, last, value);
    if (ec != std::errc() || ptr != last) return std::nullopt;
    if (value < -2147483648 || value > 2147483647) return std::nullopt;
    return value;
};

#ifdef _WIN32
#pragma comment(lib, "advapi32.lib")
#include <windows.h>
#include <conio.h>
#include <memory>

inline std::string Stop::GetSystemCodePage() noexcept {
    HKEY hKey;
    LONG lResult;
    DWORD dwType = REG_SZ;
    DWORD dwSize = 0;
    lResult = RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Control\\Nls\\CodePage", 0, KEY_READ, &hKey);
    if (lResult != ERROR_SUCCESS) {return "";}
    lResult = RegQueryValueExA(hKey, "OEMCP", NULL, &dwType, NULL, &dwSize);
    if (lResult != ERROR_SUCCESS || dwSize == 0) { RegCloseKey(hKey);
        return "";}
    std::unique_ptr<char[]> buffer(new char[dwSize]);
    lResult = RegQueryValueExA(hKey, "OEMCP", NULL, &dwType, (LPBYTE)buffer.get(), &dwSize);
    RegCloseKey(hKey);
    if (lResult != ERROR_SUCCESS) {return "";}
    return std::string(buffer.get());
};

inline void Stop::paused(std::string_view msg, std::string_view cexit) {
    while (_kbhit()) {_getch();}
    auto result = Stop::Parse_Cexit(cexit);
    if (result.has_value()) {
        int exit_code = result.value();
        fmt::print(stderr, "{}", msg);
        _getch();
        exit(exit_code);}
    fmt::print(stderr, "\x1b[91mError: \x1b[4;97m{}\x1b[0m", cexit);
    _getch();
};

inline void Stop::paused(std::string_view msg) {
    while (_kbhit()) {_getch();}
    auto result = Stop::Parse_Cexit(msg);
    if (result.has_value()) {
        int exit_code = result.value();
        std::string CodePage = Stop::GetSystemCodePage();
        if (CodePage == "936") {fmt::print(stderr, "{}", Stop::CSimplified);}
        else if (CodePage == "950" || CodePage == "938") \
            {fmt::print(stderr, "{}", Stop::CTraditional);}
        else {fmt::print(stderr, "{}", Stop::English);}
        _getch();
        exit(exit_code);}
    fmt::print(stderr, "{}", msg);
    _getch();
};

inline void Stop::paused() {
    while (_kbhit()) {_getch();}
    std::string CodePage = Stop::GetSystemCodePage();
    if (CodePage == "936") {fmt::print(stderr, "{}", Stop::CSimplified);}
    else if (CodePage == "950" || CodePage == "938") \
        {fmt::print(stderr, "{}", Stop::CTraditional);}
    else {fmt::print(stderr, "{}", Stop::English);}
    _getch();
};

#elif defined(__linux__)
#include <termios.h>
#include <unistd.h>
#include <fstream>

inline void Stop::getch() noexcept {
    tcflush(STDIN_FILENO, TCIFLUSH);
    struct termios termios;
    if (tcgetattr(STDIN_FILENO, &termios) != 0) return;
    const tcflag_t originalLflag = termios.c_lflag;
    termios.c_lflag &= ~(ICANON | ECHO);
    if (tcsetattr(STDIN_FILENO, TCSANOW, &termios) != 0) return;
    char ch;
    ssize_t result = read(STDIN_FILENO, &ch, 1);
    tcflush(STDIN_FILENO, TCIFLUSH);
    termios.c_lflag = originalLflag;
    tcsetattr(STDIN_FILENO, TCSANOW, &termios);
    if (result > 0) {fmt::print(stderr, "\n");}
}

inline std::string Stop::GetSystemLanguage() noexcept {
    try {std::ifstream f("/etc/locale.conf");
        if (!f) return "C";
        std::string line;
        while (std::getline(f, line)) {
            if (line.empty() || line[0] == '#') continue;
            const auto eq = line.find('=');
            if (eq == std::string::npos) continue;
            const auto dot = line.find('.', eq + 1);
            const size_t start = eq + 1;
            const size_t len = (dot == std::string::npos) ? std::string::npos : dot - start;
            return line.substr(start, len);}
        return "C";}
    catch (...) {return "C";}
};

inline void Stop::paused(std::string_view msg, std::string_view cexit) {
    tcflush(STDIN_FILENO, TCIFLUSH);
    auto result = Stop::Parse_Cexit(cexit);
    if (result.has_value()) {
        int exit_code = result.value();
        fmt::print(stderr, "{}", msg);
        Stop::getch();
        exit(exit_code);}
    fmt::print(stderr, "\x1b[91mError: \x1b[4;97m{}\x1b[0m", cexit);
    Stop::getch();
};

inline void Stop::paused(std::string_view msg) {
    tcflush(STDIN_FILENO, TCIFLUSH);
    auto result = Stop::Parse_Cexit(msg);
    if (result.has_value()) {
        int exit_code = result.value();
        std::string language = Stop::GetSystemLanguage();
        if (language == "zh_CN") {fmt::print(stderr, "{}", Stop::CSimplified);} 
        else if (language == "zh_TW" || language == "zh_HK") \
            {fmt::print(stderr, "{}", Stop::CTraditional);}
        else {fmt::print(stderr, "{}", Stop::English);}
        Stop::getch();
        exit(exit_code);}
    fmt::print(stderr, "{}", msg);
    Stop::getch();
};

inline void Stop::paused() {
    tcflush(STDIN_FILENO, TCIFLUSH);
    std::string language = Stop::GetSystemLanguage();
    if (language == "zh_CN") {fmt::print(stderr, "{}", Stop::CSimplified);}
    else if (language == "zh_TW" || language == "zh_HK") \
        {fmt::print(stderr, "{}", Stop::CTraditional);}
    else {fmt::print(stderr, "{}", Stop::English);}
    Stop::getch();
};
#endif  // Windows Linux

#endif  // PAUSED_H_
