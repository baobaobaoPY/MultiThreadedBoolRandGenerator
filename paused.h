#ifndef PAUSED_H_
#define PAUSED_H_

#include <fmt/core.h>
#include <regex>

namespace {
    /**
    * ^exit  Begins with literal 'exit' (case-sensitive)
    * \\(    Matches a left parenthesis
    * [+-]?  Optional plus or minus sign (can be '+','-' or omitted for positive numbers)
    * \\d+   Matches one or more digits
    * \\)$   Ends with a right parenthesis
    **/
    static const std::regex filtration("(^exit\\([+-]?\\d+\\)$)");

    bool isOutOfRange(const std::string& str) noexcept {
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
        const std::string max_value = negative ? "2147483648" : "2147483647";
        if (length == 10) {
            for (size_t j = 0; j < 10; ++j) {
                if (str[i + j] > max_value[j]) return true;
                else if (str[i + j] < max_value[j]) break;}}
        return false;}

    std::string find_string(const std::string& str) {
        size_t startPos = str.find('(');
        if (startPos == std::string::npos) {return "";}
        size_t endPos = str.find(')', startPos + 1);
        if (endPos == std::string::npos) {return "";}
        return str.substr(startPos + 1, endPos - startPos - 1);}
};

#ifdef _WIN32
#include <windows.h>
#include <conio.h>

/** AWML - API 
 * A -> Automatic Derivation Platform
 * W -> Windows
 * M -> Macos
 * L -> Linux
**/
class AWML {
public:
    static inline void paused(const std::string& PROMPT, const std::string& TR_TO_SK) noexcept;
    static inline void paused(const std::string& TR_TO_SK) noexcept;
    static inline void paused() noexcept;
private:
    static std::string GetSystemCodePage();
};

std::string AWML::GetSystemCodePage() {
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
    return std::string(szValue);
};

inline void AWML::paused(const std::string& PROMPT, const std::string& TR_TO_SK) noexcept {
    while (_kbhit()) {_getch();}
    if (std::regex_match(TR_TO_SK, filtration)) {
        std::string string_Texit = find_string(TR_TO_SK);
        int int_Texit;
        if (!isOutOfRange(string_Texit)) {
            int_Texit = std::stoi(string_Texit);
            fmt::print("{}", PROMPT);
            _getch();
            exit(int_Texit);}}
    fmt::print("{}", PROMPT);
    _getch();
};

inline void AWML::paused(const std::string& TR_TO_SK) noexcept {
    while (_kbhit()) {_getch();}
    if (std::regex_match(TR_TO_SK, filtration)) {
        std::string string_Texit = find_string(TR_TO_SK);
        int int_Texit;
        if (!isOutOfRange(string_Texit)) {
            int_Texit = std::stoi(string_Texit);
            std::string CodePage = AWML::GetSystemCodePage();
            if (CodePage == "936") {fmt::print("请按任意键继续. . .");}
            else if (CodePage == "950" || CodePage == "938") {fmt::print("請按任意鍵繼續. . .");}
            else {fmt::print("Press any key to continue. . .");}
            _getch();
            exit(int_Texit);}}
    fmt::print("{}", TR_TO_SK);
    _getch();
};

inline void AWML::paused() noexcept {
    while (_kbhit()) {_getch();}
    std::string CodePage = AWML::GetSystemCodePage();
    if (CodePage == "936") {fmt::print("请按任意键继续. . .");}
    else if (CodePage == "950" || CodePage == "938") {fmt::print("請按任意鍵繼續. . .");}
    else {fmt::print("Press any key to continue. . .");}
    _getch();
};

#elif defined(__linux__) || defined(__APPLE__)
#include <termios.h>
#include <unistd.h>
#include <fstream>

/** AWML - API 
 * A -> Automatic Derivation Platform
 * W -> Windows
 * M -> Macos
 * L -> Linux
**/
class AWML {
public:
    static inline void paused(const std::string& PROMPT, const std::string& TR_TO_SK) noexcept;
    static inline void paused(const std::string& TR_TO_SK) noexcept;
    static inline void paused() noexcept;
private:
    static inline void getch();
    static inline std::string GetSystemLanguage();
};

inline void AWML::getch() {
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
    if (result > 0) {fmt::print("\n");}
}

inline std::string AWML::GetSystemLanguage() {
    std::ifstream OpenFile("/etc/locale.conf");
    if (!OpenFile.is_open()) {return "C";}
    std::string line;
    while (std::getline(OpenFile, line)) {
        if (line.empty() || line[0] == '#')
            continue;
        size_t equal_pos = line.find('=');
        size_t dot_pos = line.find('.', equal_pos + 1);
        size_t start = equal_pos + 1;
        size_t length = (dot_pos == std::string::npos) ? std::string::npos : dot_pos - start;
        return line.substr(start, length);}
    OpenFile.close();
    return "C";
};

inline void AWML::paused(const std::string& PROMPT, const std::string& TR_TO_SK) noexcept {
    tcflush(STDIN_FILENO, TCIFLUSH);
    if (std::regex_match(TR_TO_SK, filtration)) {
        std::string string_Texit = find_string(TR_TO_SK);
        int int_Texit;
        if (!isOutOfRange(string_Texit)) {
            int_Texit = std::stoi(string_Texit);
            fmt::print("{}", PROMPT);
            AWML::getch();
            exit(int_Texit);}}
    fmt::print("{}", PROMPT);
    AWML::getch();
};

inline void AWML::paused(const std::string& TR_TO_SK) noexcept {
    tcflush(STDIN_FILENO, TCIFLUSH);
    if (std::regex_match(TR_TO_SK, filtration)) {
        std::string string_Texit = find_string(TR_TO_SK);
        int int_Texit;
        if (!isOutOfRange(string_Texit)) {
            int_Texit = std::stoi(string_Texit);
            std::string language = AWML::GetSystemLanguage();
            if (language == "zh_CN") {fmt::print("请按任意键继续. . .");} 
            else if (language == "zh_TW" || language == "zh_HK") {fmt::print("請按任意鍵繼續. . .");}
            else {fmt::print("Press any key to continue. . .");}
            AWML::getch();
            exit(int_Texit);}}
    fmt::print("{}", TR_TO_SK);
    AWML::getch();
};

inline void AWML::paused() noexcept {
    tcflush(STDIN_FILENO, TCIFLUSH);
    std::string language = AWML::GetSystemLanguage();
    if (language == "zh_CN") {fmt::print("请按任意键继续. . .");} 
    else if (language == "zh_TW" || language == "zh_HK") {fmt::print("請按任意鍵繼續. . .");}
    else {fmt::print("Press any key to continue. . .");}
    AWML::getch();
};
#endif  // Windows Linux

#endif  // PAUSE_H_
