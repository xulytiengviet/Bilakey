// CTest: run in real Windows OS GitHub runners on both x64 and ARM64.
#include "../App/Cvnss.h"
#include <iostream>
#include <string>
using namespace bilakey::cvnss;
static int failures=0;
void equal(const wchar_t* label,const std::wstring& got,const wchar_t* expected) {
    if(got!=expected){std::wcerr<<L"FAIL "<<label<<L"\n";++failures;}
}
void script(const wchar_t* label,const std::wstring& raw,Script expected) {
    if(classify(raw).script!=expected){std::wcerr<<L"script FAIL "<<label<<L"\n";++failures;}
}
int main() {
    equal(L"tôi",decode(L"toiy"),L"tôi");
    equal(L"yêu",decode(L"iwy"),L"yêu");
    equal(L"việt",decode(L"vidf"),L"việt");
    equal(L"tiếng",decode(L"tizb"),L"tiếng");
    equal(L"nghiêng",decode(L"wizy"),L"nghiêng");
    equal(L"encode tôi",encode(L"tôi"),L"toiy");
    equal(L"encode việt",encode(L"việt"),L"vidf");
    equal(L"encode tiếng",encode(L"tiếng"),L"tizb");
    equal(L"canonical ồ",decode(L"od"),L"ồ");
    equal(L"pass-through",decode(L"xyzzz"),L"xyzzz");
    script(L"CVNSS",L"tizb",Script::Cvns);
    script(L"English",L"more",Script::English);
    script(L"English protected",L"with",Script::English);
    script(L"English code",L"string",Script::English);
    script(L"Vietnamese",L"tiếng",Script::Vietnamese);
    script(L"critical collision blocked",L"od",Script::English);
    auto d=classify(L"tizb",true);
    equal(L"target code",d.output,L"tizb");
    if (d.script!=Script::Cvns) ++failures;
    bool recognized=false, ambiguous=false;
    decode(L"od",&recognized,&ambiguous);
    if (!recognized||!ambiguous) ++failures;
    std::cout<<"BilaKey v0.0.1 native CVNSS tests: "<<(failures?"FAIL":"PASS")<<"\n";
    return failures?1:0;
}
