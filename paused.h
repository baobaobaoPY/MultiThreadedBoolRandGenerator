#ifndef PAUSED_H_
#define PAUSED_H_

#include <fmt/core.h>
#include <optional>
#include <charconv>
#include <string>

using std::string_view;
using std::string;
using fmt::print;

/** AWML - API
 * A -> Automatic Derivation Platform
 * W -> Windows
 * M -> Macos
 * L -> Linux
**/
class AWML {
public:
    static inline void paused(const string& msg, const string& cexit);
    static inline void paused(const string& msg);
    static inline void paused();
private:
    static inline std::optional<int> Parse_Cexit(string_view str) noexcept;
    static inline string find_string(string_view str) noexcept;

    static inline constexpr string_view \
    CSimplified{"\u8BF7\u6309\u4EFB\u610F\u952E\u7EE7\u7EED. . ."},
    CTraditional{"\u8ACB\u6309\u4EFB\u610F\u9375\u7E7C\u7E8C. . ."},
    English{"Press any key to continue. . ."};

    static inline string GetSystemCodePage() noexcept;
    static inline void getch() noexcept;
    static inline string GetSystemLanguage() noexcept;
};

inline std::optional<int> AWML::Parse_Cexit(string_view str) noexcept {
    int value;
    constexpr string_view prefix = "exit(";
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

inline string AWML::GetSystemCodePage() noexcept {
    HKEY hKey;
    LONG lResult;
    DWORD dwType = REG_SZ;
    DWORD dwSize = 0;
    lResult = RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Control\\Nls\\CodePage", 0, KEY_READ, &hKey);
    if (lResult != ERROR_SUCCESS) {return "";}
    lResult = RegQueryValueExA(hKey, "OEMCP", NULL, &dwType, NULL, &dwSize);
    if (lResult != ERROR_SUCCESS || dwSize == 0) {
        RegCloseKey(hKey);
        return "";}
    std::unique_ptr<char[]> buffer(new char[dwSize]);
    lResult = RegQueryValueExA(hKey, "OEMCP", NULL, &dwType, (LPBYTE)buffer.get(), &dwSize);
    RegCloseKey(hKey);
    if (lResult != ERROR_SUCCESS) {return "";}
    return string(buffer.get());
};

/**
 * Pauses program execution and waits for keypress, supporting custom prompts and exit commands.
 * Displays user-defined prompt text or exit message when required.
 * Control command string - if formatted as "exit(number)", terminates program with specified exit code;
 * otherwise treated as custom prompt text.
**/
inline void AWML::paused(const string& msg, const string& cexit) {
    while (_kbhit()) {_getch();}
    auto result = AWML::Parse_Cexit(cexit);
    if (result.has_value()) {
        int exit_code = result.value();
        print("{}", msg);
        _getch();
        exit(exit_code);}
    print("\x1b[91mError: \x1b[4;97m{}\x1b[0m", cexit);
    _getch();
};

/**
 * Pauses program execution and waits for a keypress, displaying system-default prompt message based on OS language.
 * Control command string - if formatted as "exit(number)", terminates program with specified exit code;
 * otherwise treated as custom prompt text to display!
**/
inline void AWML::paused(const string& msg) {
    while (_kbhit()) {_getch();}
    auto result = AWML::Parse_Cexit(msg);
    if (result.has_value()) {
        int exit_code = result.value();
        string CodePage = AWML::GetSystemCodePage();
        if (CodePage == "936") {print("{}", AWML::CSimplified);}
        else if (CodePage == "950" || CodePage == "938") \
            {print("{}", AWML::CTraditional);}
        else {print("{}", AWML::English);}
        _getch();
        exit(exit_code);}
    print("{}", msg);
    _getch();
};

/**
 * Pauses program execution and waits for a keypress, displaying system language-specific default prompt message
 * No-parameter version, only shows the default "Press any key to continue" prompt
**/
inline void AWML::paused() {
    while (_kbhit()) {_getch();}
    string CodePage = AWML::GetSystemCodePage();
    if (CodePage == "936") {print("{}", AWML::CSimplified);}
    else if (CodePage == "950" || CodePage == "938") \
        {print("{}", AWML::CTraditional);}
    else {print("{}", AWML::English);}
    _getch();
};

#elif defined(__linux__)
#include <termios.h>
#include <unistd.h>
#include <fstream>

inline void AWML::getch() noexcept {
    tcflush(STDIN_FILENO, TCIFLUSH);
    fflush(stdout);
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
    if (result > 0) {print("\12");}
}

inline string AWML::GetSystemLanguage() noexcept {
    try {std::ifstream f("/etc/locale.conf");
        if (!f) return "C";
        string line;
        while (std::getline(f, line)) {
            if (line.empty() || line[0] == '#') continue;
            const auto eq = line.find('=');
            if (eq == string::npos) continue;
            const auto dot = line.find('.', eq + 1);
            const size_t start = eq + 1;
            const size_t len = (dot == string::npos) ? string::npos : dot - start;
            return line.substr(start, len);}
        return "C";}
    catch (...) {return "C";}
};

/**
 * Pauses program execution and waits for keypress, supporting custom prompts and exit commands.
 * Displays user-defined prompt text or exit message when required.
 * Control command string - if formatted as "exit(number)", terminates program with specified exit code;
 * otherwise treated as custom prompt text.
**/
inline void AWML::paused(const string& msg, const string& cexit) {
    tcflush(STDIN_FILENO, TCIFLUSH);
    auto result = AWML::Parse_Cexit(cexit);
    if (result.has_value()) {
        int exit_code = result.value();
        print("{}", msg);
        AWML::getch();
        exit(exit_code);}
    print("\x1b[91mError: \x1b[4;97m{}\x1b[0m", cexit);
    AWML::getch();
};

/**
 * Pauses program execution and waits for a keypress, displaying system-default prompt message based on OS language.
 * Control command string - if formatted as "exit(number)", terminates program with specified exit code;
 * otherwise treated as custom prompt text to display!
**/
inline void AWML::paused(const string& msg) {
    tcflush(STDIN_FILENO, TCIFLUSH);
    auto result = AWML::Parse_Cexit(msg);
    if (result.has_value()) {
        int exit_code = result.value();
        string language = AWML::GetSystemLanguage();
        if (language == "zh_CN") {print("{}", AWML::CSimplified);} 
        else if (language == "zh_TW" || language == "zh_HK") \
            {print("{}", AWML::CTraditional);}
        else {print("{}", AWML::English);}
        AWML::getch();
        exit(exit_code);}
    print("{}", msg);
    AWML::getch();
};

/**
 * Pauses program execution and waits for a keypress, displaying system language-specific default prompt message
 * No-parameter version, only shows the default "Press any key to continue" prompt
**/
inline void AWML::paused() {
    tcflush(STDIN_FILENO, TCIFLUSH);
    string language = AWML::GetSystemLanguage();
    if (language == "zh_CN") {print("{}", AWML::CSimplified);}
    else if (language == "zh_TW" || language == "zh_HK") \
        {print("{}", AWML::CTraditional);}
    else {print("{}", AWML::English);}
    AWML::getch();
};
#endif  // Windows Linux

#endif  // PAUSED_H_
