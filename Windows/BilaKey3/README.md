# BilaKey 3 — Anh / Việt / CVNSS4.0 Windows experiment

This is a separate *experimental* layer for the existing Vikey Windows C++ IME in the repository, not a stable replacement for `Windows/App/Core.cpp`.

## Try it on Windows 10/11

Get the full prototype source archive from the accompanying ChatGPT delivery (BilaKey3-Windows-Source-v0.1.zip). Unzip it; install Python 3.10+ with tkinter and Node.js 18+, then run `Run-BilaKey3.bat`. The archive includes the offline preview UI, classifier, optional Windows keyboard hook, tests and this exact official converter. The `Windows/BilaKey3/src/bridge.js` source here implements its line-delimited local interface.

The prototype reads input as English / Vietnamese Telex / CVNSS4.0 and previews simultaneous CQN and CVNSS output. An optional Windows hook replaces *completed words* when Space/Enter/Tab is pressed. It does not translate English into Vietnamese.

**Not yet a validated native Windows build**. The Windows hook is untested on an actual Windows machine in this delivery. Do not use it for passwords, payment fields or sensitive inputs. No measured 99.8% accuracy: this number is a *target*, not a verified benchmark.

The converter is `5.0.0-audit-safe`, implements 56 explicit ambiguity groups and has a built-in selfTest. Changing the target output between CQN and CVNSS is not equivalent to requiring manual switching of input languages; the recognizer can accept mixed input either way.

## Upstream integration plan

1. Add a native CVNSS4.0 token resolver to the existing `Engine/Driver` and Windows low-level keyboard hook without disrupting Vikey's Telex engine.
2. Preserve English words, URLs, program code and secure entry; defer ambiguous CVNSS codes to user choice or contextual confidence.
3. Implement live composition, undo, per-app policy, tray UI and explicit opt-in.
4. Run Windows x64 + ARM64 CI, golden vector tests, full hooked typing tests and independent mixed-language accuracy benchmarks.
5. Build and sign downloadable .exe releases only after these pass.

## Credits

Based on xulytiengviet/Bilakey fork of Vikey by Tony Dinh, based on OpenKey by Tuyen Mai (GPL-3.0). CVNSS4.0 converter supplied by the user; verify its publication terms before redistributing publicly.
