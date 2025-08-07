#ifndef PAUSED_H_
#define PAUSED_H_

#include <fmt/core.h>
#include <string>
#include <regex>

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
    static inline void paused(const string& cexit, const string& message);
    static inline void paused(const string& message);
    static inline void paused();
private:
    static inline const std::regex& filtration() noexcept;
    static inline bool isOutOfRange(const string& str) noexcept;
    static inline string find_string(const string& str) noexcept;
    static inline const string& char_sequence(unsigned char Symbol) noexcept;

    static inline string GetSystemCodePage() noexcept;
    static inline void getch() noexcept;
    static inline string GetSystemLanguage() noexcept;
};

/**
* ^exit  Begins with literal 'exit' (case-sensitive)
* \\(    Matches a left parenthesis
* [+-]?  Optional plus or minus sign (can be '+','-' or omitted for positive numbers)
* \\d+   Matches one or more digits
* \\)$   Ends with a right parenthesis
**/
inline const std::regex& AWML::filtration() noexcept {
    static const std::regex instance("(^exit\\([+-]?\\d+\\)$)");
    return instance;
};

inline bool AWML::isOutOfRange(const string& str) noexcept {
    if (str.empty()) return true;
    size_t i = 0;
    bool negative = false;
    if (str[i] == '-') {negative = true;++i;} 
    else if (str[i] == '+') {++i;}
    if (i >= str.size() || !std::isdigit(str[i])) return true;
    while (i < str.size() && str[i] == '0') ++i;
    if (i == str.size()) return false;
    size_t length = str.size() - i;
    if (length > 10) return true;
    const string max_value = negative ? "2147483648" : "2147483647";
    if (length == 10) {
        for (size_t j = 0; j < 10; ++j) {
            if (str[i + j] > max_value[j]) return true;
            else if (str[i + j] < max_value[j]) break;}}
    return false;
};

inline string AWML::find_string(const string& str) noexcept {
    size_t startPos = str.find('(');
    if (startPos == string::npos) {return "";}
    size_t endPos = str.find(')', startPos + 1);
    if (endPos == string::npos) {return "";}
    return str.substr(startPos + 1, endPos - startPos - 1);
};

inline const string& AWML::char_sequence(unsigned char Symbol) noexcept {
    static const string \
    a{"\350\257\267\346\214\211\344\273\273\346\204\217\351\224\256\347\273\247\347\273\255. . ."},
    b{"\350\253\213\346\214\211\344\273\273\346\204\217\351\215\265\347\271\274\347\272\214. . ."},
    c{"\120\162\145\163\163\040\141\156\171\040\153\145\171\040\164\157\040\143\157\156\164\151\156\165\145. . ."};
    switch(Symbol) {case 1:return a;case 2:return b;default:return c;}
};

#ifdef _WIN32
#pragma comment(lib, "advapi32.lib")
#include <windows.h>
#include <conio.h>

inline string AWML::GetSystemCodePage() noexcept {
    HKEY hKey;
    LONG lResult;
    DWORD dwType = REG_SZ;
    DWORD dwSize = 0;
    char szValue[256] = {0};
    lResult = RegOpenKeyExA (HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Control\\Nls\\CodePage", 0, KEY_READ, &hKey);
    if (lResult != ERROR_SUCCESS) {return "";}
    lResult = RegQueryValueExA (hKey, "OEMCP", NULL, &dwType, NULL, &dwSize);
    if (lResult != ERROR_SUCCESS || dwSize == 0) {RegCloseKey(hKey);return "";}
    lResult = RegQueryValueExA (hKey, "OEMCP", NULL, &dwType, (LPBYTE)szValue, &dwSize);
    RegCloseKey(hKey);
    if (lResult != ERROR_SUCCESS) {return "";}
    return string(szValue);
};

/**
 * Pauses program execution and waits for keypress, supporting custom prompts and exit commands.
 * Displays user-defined prompt text or exit message when required.
 * Control command string - if formatted as "exit(number)", terminates program with specified exit code;
 * otherwise treated as custom prompt text.
**/
inline void AWML::paused(const string& cexit, const string& message) {
    while (_kbhit()) {_getch();}
    if (std::regex_match(message, AWML::filtration())) {
        string string_Texit = AWML::find_string(message);
        int int_Texit;
        if (!AWML::isOutOfRange(string_Texit)) {
            int_Texit = std::stoi(string_Texit);
            print("{}", cexit);
            _getch();
            exit(int_Texit);}}
    print("{}", cexit);
    _getch();
};

/**
 * Pauses program execution and waits for a keypress, displaying system-default prompt message based on OS language.
 * Control command string - if formatted as "exit(number)", terminates program with specified exit code;
 * otherwise treated as custom prompt text to display!
**/
inline void AWML::paused(const string& message) {
    while (_kbhit()) {_getch();}
    if (std::regex_match(message, AWML::filtration())) {
        string string_Texit = AWML::find_string(message);
        int int_Texit;
        if (!AWML::isOutOfRange(string_Texit)) {
            int_Texit = std::stoi(string_Texit);
            string CodePage = AWML::GetSystemCodePage();
            if (CodePage == "936") {print("{}", AWML::char_sequence(1));}
            else if (CodePage == "950" || CodePage == "938") \
                    {print("{}", AWML::char_sequence(2));}
            else {print("{}", AWML::char_sequence(0));}
            _getch();
            exit(int_Texit);}}
    print("{}", message);
    _getch();
};

/**
 * Pauses program execution and waits for a keypress, displaying system language-specific default prompt message
 * No-parameter version, only shows the default "Press any key to continue" prompt
**/
inline void AWML::paused() {
    while (_kbhit()) {_getch();}
    string CodePage = AWML::GetSystemCodePage();
    if (CodePage == "936") {print("{}", AWML::char_sequence(1));}
    else if (CodePage == "950" || CodePage == "938") \
            {print("{}", AWML::char_sequence(2));}
    else {print("{}", AWML::char_sequence(0));}
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
    if (result > 0) {print("\n");}
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
inline void AWML::paused(const string& cexit, const string& message) {
    tcflush(STDIN_FILENO, TCIFLUSH);
    if (std::regex_match(message, AWML::filtration())) {
        string string_Texit = AWML::find_string(message);
        int int_Texit;
        if (!AWML::isOutOfRange(string_Texit)) {
            int_Texit = std::stoi(string_Texit);
            print("{}", cexit);
            AWML::getch();
            exit(int_Texit);}}
    print("{}", cexit);
    AWML::getch();
};

/**
 * Pauses program execution and waits for a keypress, displaying system-default prompt message based on OS language.
 * Control command string - if formatted as "exit(number)", terminates program with specified exit code;
 * otherwise treated as custom prompt text to display!
**/
inline void AWML::paused(const string& message) {
    tcflush(STDIN_FILENO, TCIFLUSH);
    if (std::regex_match(message, AWML::filtration())) {
        string string_Texit = AWML::find_string(message);
        int int_Texit;
        if (!AWML::isOutOfRange(string_Texit)) {
            int_Texit = std::stoi(string_Texit);
            string language = AWML::GetSystemLanguage();
            if (language == "zh_CN") {print("{}", AWML::char_sequence(1));} 
            else if (language == "zh_TW" || language == "zh_HK") \
                    {print("{}", AWML::char_sequence(2));}
            else {print("{}", AWML::char_sequence(0));}
            AWML::getch();
            exit(int_Texit);}}
    print("{}", message);
    AWML::getch();
};

/**
 * Pauses program execution and waits for a keypress, displaying system language-specific default prompt message
 * No-parameter version, only shows the default "Press any key to continue" prompt
**/
inline void AWML::paused() {
    tcflush(STDIN_FILENO, TCIFLUSH);
    string language = AWML::GetSystemLanguage();
    if (language == "zh_CN") {print("{}", AWML::char_sequence(1));}
    else if (language == "zh_TW" || language == "zh_HK") \
            {print("{}", AWML::char_sequence(2));}
    else {print("{}", AWML::char_sequence(0));}
    AWML::getch();
};
#endif  // Windows Linux

#endif  // PAUSED_H_
