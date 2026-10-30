// Native CVNSS4.0 decoder; mapping/policies generated from audit-safe v5 JS data.
// Never executes JavaScript or starts a subprocess in the installed Windows app.
#include "Cvnss.h"
#include "CvnssData.inc"
#include <algorithm>
#include <codecvt>
#include <cwctype>
#include <locale>
#include <set>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace bilakey { namespace cvnss {
namespace {
using Pair = std::pair<std::wstring, std::wstring>;
struct Tables {
    std::unordered_map<std::wstring,std::wstring> forward;
    std::unordered_map<std::wstring,std::vector<std::wstring>> reverse;
    std::unordered_map<std::wstring,std::wstring> preferred;
    std::vector<Pair> initial;
    std::vector<std::wstring> codeInitials;
};
std::wstring wide(const std::string& utf8) {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
    return conv.from_bytes(utf8);
}
std::wstring lower(std::wstring w) {
    // Windows CRT lowercasing is locale-sensitive; ASCII input is explicitly normalized.
    for (auto& c : w) {
        if (c >= L'A' && c <= L'Z') c = wchar_t(c + 32);
        else if (c == L'Đ') c = L'đ';
        else c = std::towlower(c);
    }
    return w;
}
bool begins(const std::wstring& s, const std::wstring& prefix) {
    return s.size() >= prefix.size() && s.compare(0, prefix.size(), prefix) == 0;
}
std::vector<Pair> readPairs(const char* src) {
    std::vector<Pair> items;
    std::wistringstream ss(wide(src));
    std::wstring line;
    while (std::getline(ss,line)) {
        if (!line.empty() && line.back()==L'\r') line.pop_back();
        size_t tab=line.find(L'\t');
        if (tab != std::wstring::npos)
            items.emplace_back(line.substr(0,tab),line.substr(tab+1));
    }
    return items;
}
const Tables& tables() {
    static const Tables t=[] {
        Tables t;
        for (const auto& row : readPairs(data::rimes)) {
            t.forward.emplace(row.first,row.second);
            auto& v=t.reverse[row.second];
            if (std::find(v.begin(),v.end(),row.first)==v.end()) v.push_back(row.first);
        }
        for (auto& row:readPairs(data::preferred)) t.preferred.emplace(row);
        for (auto& row:readPairs(data::onsets)) {
            t.initial.push_back(row);
            if (std::find(t.codeInitials.begin(),t.codeInitials.end(),row.second)==t.codeInitials.end())
                t.codeInitials.push_back(row.second);
        }
        std::stable_sort(t.initial.begin(),t.initial.end(),
            [](const Pair& a,const Pair& b){return a.first.size()>b.first.size();});
        std::stable_sort(t.codeInitials.begin(),t.codeInitials.end(),
            [](const auto& a,const auto& b){return a.size()>b.size();});
        return t;
    }();
    return t;
}
std::wstring applyCase(const std::wstring& raw,std::wstring output) {
    if (raw.empty() || output.empty()) return output;
    bool upper=true;
    for (wchar_t ch:raw) if ((ch>=L'a'&&ch<=L'z') ||
      (ch>=L'à' && ch<=L'ỹ') || ch==L'đ') {upper=false;break;}
    if (upper) {
        for (auto& ch:output) {
            if (ch>=L'a' && ch<=L'z') ch=wchar_t(ch-32);
            else if (ch==L'đ') ch=L'Đ';
            else ch=std::towupper(ch);
        }
    } else if (raw[0]>=L'A' && raw[0]<=L'Z') {
        if (output[0]>=L'a'&&output[0]<=L'z') output[0]=wchar_t(output[0]-32);
    }
    return output;
}
bool hasVietnameseMark(const std::wstring& s) {
    for (wchar_t c:s) if (c>127 && (c==L'đ'||c==L'Đ'||(c>=0x00c0 && c<=0x1ef9))) return true;
    return false;
}
std::wstring restoreOnset(const std::wstring& code,const std::wstring& vowel) {
    const wchar_t first=vowel.empty()?0:vowel[0];
    bool front=first==L'i'||first==L'e'||first==L'ê'||first==L'í'||first==L'é';
    if(code==L"q")return L"qu";
    if(code==L"j")return L"gi";
    if(code==L"z")return L"d";
    if(code==L"d")return L"đ";
    if(code==L"f")return L"ph";
    if(code==L"k")return L"kh";
    if(code==L"w")return front?L"ngh":L"ng";
    if(code==L"g")return front?L"gh":L"g";
    if(code==L"c")return front?L"k":L"c";
    return code;
}
const std::unordered_set<std::wstring>& protectedEnglish() {
    static const std::unordered_set<std::wstring> words={
        L"about",L"after",L"all",L"also",L"and",L"any",L"are",L"best",L"both",
        L"can",L"case",L"class",L"close",L"code",L"come",L"data",L"day",L"each",
        L"english",L"even",L"ever",L"file",L"find",L"for",L"form",L"from",L"full",
        L"get",L"github",L"good",L"has",L"have",L"hello",L"here",L"home",L"how",
        L"http",L"https",L"import",L"into",L"just",L"know",L"like",L"line",L"list",
        L"long",L"love",L"main",L"make",L"many",L"more",L"most",L"name",L"need",
        L"node",L"none",L"not",L"now",L"null",L"one",L"only",L"open",L"our",
        L"over",L"part",L"plan",L"please",L"push",L"python",L"read",L"real",
        L"return",L"right",L"run",L"same",L"save",L"school",L"see",L"set",
        L"should",L"some",L"start",L"stop",L"string",L"such",L"system",L"take",
        L"test",L"text",L"than",L"that",L"the",L"their",L"them",L"then",L"there",
        L"these",L"they",L"this",L"three",L"through",L"time",L"today",L"tool",
        L"true",L"type",L"under",L"use",L"user",L"value",L"very",L"view",L"want",
        L"was",L"way",L"web",L"were",L"what",L"when",L"where",L"which",L"while",
        L"who",L"why",L"will",L"windows",L"with",L"word",L"work",L"world",
        L"would",L"write",L"yes",L"you",L"your",
        L"api",L"array",L"bool",L"build",L"cloud",L"const",L"css",L"dev",
        L"docker",L"exe",L"false",L"float",L"function",L"git",L"html",L"int",
        L"java",L"javascript",L"json",L"linux",L"localhost",L"npm",L"rust",
        L"script",L"sql",L"var",L"void",L"xml",L"yaml",L"zip"
    };
    return words;
}
} // namespace

std::wstring decode(const std::wstring& raw,bool* recognized,bool* ambiguous) {
    if(recognized)*recognized=false;
    if(ambiguous)*ambiguous=false;
    if(raw.empty())return raw;
    std::wstring token=lower(raw),initial,vowel;
    const auto& t=tables();
    // Exact rime codes have priority over splitting a coded onset, as in converter v5.
    if(t.reverse.find(token)!=t.reverse.end()) vowel=token;
    else {
        for (const auto& onset:t.codeInitials) {
            if(begins(token,onset) && token.size()>onset.size() &&
               t.reverse.find(token.substr(onset.size()))!=t.reverse.end()) {
                initial=onset; vowel=token.substr(onset.size()); break;
            }
        }
    }
    auto found=t.reverse.find(vowel);
    if(found==t.reverse.end())return raw;
    if(recognized)*recognized=true;
    if(ambiguous)*ambiguous=found->second.size()>1;
    auto pref=t.preferred.find(vowel);
    std::wstring cqnVowel=found->second.front();
    if(pref!=t.preferred.end() &&
       std::find(found->second.begin(),found->second.end(),pref->second)!=found->second.end())
        cqnVowel=pref->second;
    auto onset=restoreOnset(initial,cqnVowel);
    // The CVNSS4.0 formula omits an i after the gi onset in multi-letter rimes.
    if (onset==L"gi" && cqnVowel.size()>1 && cqnVowel[0]==L'i')
        cqnVowel.erase(0,1);
    if (onset==L"qu" && cqnVowel.size()>1 && cqnVowel[0]==L'u')
        cqnVowel.erase(0,1);
    if (onset.empty() && !cqnVowel.empty()) {
        const std::wstring i=L"iìỉĩíị",y=L"yỳỷỹýỵ";
        auto pos=i.find(cqnVowel[0]);
        if(pos!=std::wstring::npos && !begins(cqnVowel,L"ia"))
            cqnVowel[0]=y[pos];
    }
    return applyCase(raw,onset+cqnVowel);
}

std::wstring encode(const std::wstring& raw) {
    if(raw.empty())return raw;
    const auto& t=tables();
    std::wstring word=lower(raw),initial,vowel=word;
    for(const auto& row:t.initial) {
        if(begins(word,row.first)) {
            initial=row.second;vowel=word.substr(row.first.size());break;
        }
    }
    auto found=t.forward.find(vowel);
    if(found==t.forward.end()) {
        // Support common gi + silent i spelling on the same path as the JS converter.
        if(initial==L"j")found=t.forward.find(L"i"+vowel);
    }
    if(found==t.forward.end())return raw;
    return applyCase(raw,initial+found->second);
}
Decision classify(const std::wstring& raw,bool targetCode) {
    Decision d;d.output=raw;d.unicode=raw;d.code=raw;
    if(raw.empty()||raw.size()>48)return d;
    bool asciiLetters=true;
    for(wchar_t c:raw)if(!((c>=L'a'&&c<=L'z')||(c>=L'A'&&c<=L'Z')))asciiLetters=false;
    if(!asciiLetters) {
        if(hasVietnameseMark(raw)) {
            d.script=Script::Vietnamese;
            d.code=encode(raw);
            d.output=targetCode?d.code:raw;
        }
        return d;
    }
    if(protectedEnglish().count(lower(raw)))return d;
    bool recognized=false,ambiguous=false;
    auto decoded=decode(raw,&recognized,&ambiguous);
    d.ambiguous=ambiguous;
    if(!recognized||decoded==raw||!hasVietnameseMark(decoded))return d;
    // Longer context permits a canonical choice among i/y spelling aliases.
    // Short ambiguous syllables are left as English to avoid false positives.
    if(raw.size()<4 && lower(raw)!=L"iwy")return d;
    d.script=Script::Cvns;d.unicode=decoded;d.code=raw;
    d.output=targetCode?raw:decoded;
    return d;
}
}} // namespace
