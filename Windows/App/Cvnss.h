// BilaKey 0.0.1: native, offline CVNSS4.0 word recognizer (UTF-16).
#pragma once
#include <string>
namespace bilakey { namespace cvnss {
enum class Script { English, Vietnamese, Cvns };
struct Decision {
    Script script = Script::English;
    std::wstring output;     // Unicode CQN, or code when targetCode=true
    std::wstring unicode;
    std::wstring code;
    bool ambiguous = false;
};
std::wstring decode(const std::wstring& token, bool* recognized = nullptr, bool* ambiguous = nullptr);
std::wstring encode(const std::wstring& token);
Decision classify(const std::wstring& token, bool targetCode = false);
}} // namespace bilakey::cvnss
