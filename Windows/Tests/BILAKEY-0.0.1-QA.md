# BilaKey 0.0.1 — Windows release / physical-PC QA

BilaKey 0.0.1 is a native C++ Windows preview, not the prior Python/Node prototype. Mapping: 758 base rows + 324 patch-54 rows + 12 patch-56 rows = 1,094 rows; 56 ambiguity policies. The installed BilaKey.exe does NOT require Python, Node.js, a cloud service, or API keys.

Existing Vikey/OpenKey C++ engine performs English/Telex auto-switch. The new C++ CVNSS recognizer transforms eligible complete words in the Windows English branch at Space/Enter; it preserves known English, code context and short ambiguous tokens.

## Windows OS build (x64 or ARM64 Developer PowerShell)

    cmake -S Windows -B Windows/build -G Ninja -DCMAKE_BUILD_TYPE=Release -DVIKEY_VERSION=0.0.1 -DBILAKEY_BRAND=ON
    cmake --build Windows/build --target vikey bilakey_cvnss_test
    ctest --test-dir Windows/build -R bilakey_cvnss -C Release --output-on-failure
    pwsh Windows/check.ps1

GitHub Actions Windows runners are Windows virtual machines, NOT physical end-user PCs. Physical QA remains to be performed on matching hardware.

## Physical-PC acceptance checklist

1. Run BilaKey.exe and verify its tray menu and CVNSS auto-recognition toggle.
2. In Notepad, with English mode active, enter: I love toiy ; expect I love tôi .
3. Enter: I love tizb ; expect I love tiếng .
4. Enter: iwy ; expect yêu .
5. Enter: I love more ; expect unchanged English.
6. Test Telex: treen and truowfng via the inherited Vikey engine in the appropriate auto-switch context.
7. Try a real password field: no rewriting; test terminals/code editors: no rewriting.
8. Test keyboard focus, cursor/mouse movement, Caps Lock, address bar, browser, Word, RDP and undo.
9. Test both installer architectures on appropriate PCs and uninstall/launch-at-login/coexistence with Vikey.
10. Record errors and create a reproducible bug issue.

Warning: auto recognition is not yet independent of the original Vikey English/VI internal language state. Full three-way composition in all fields is not certified. An accuracy figure of 99.8% has NOT been measured.

## Release assets

Successful pipeline produces BilaKey-0.0.1-x64-Setup.exe, BilaKey-0.0.1-arm64-Setup.exe, two portable ZIP archives and SHA256SUMS.txt. Release is created ONLY after both Windows OS runner tests pass. The preview is unsigned and may trigger SmartScreen.
