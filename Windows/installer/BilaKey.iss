; BilaKey for Windows installer (Inno Setup 6). Per-user, no admin prompt, so the app can update itself.
;   iscc /DVersion=1.0.14 /DArch=x64 /DSrc=..\build\App BilaKey.iss
#ifndef Version
  #define Version "0.0.0"
#endif
#ifndef Arch
  #define Arch "x64"
#endif
#ifndef Src
  #define Src "..\build\App"
#endif

[Setup]
AppId={{E9EAF0D5-02D4-4FE0-8A69-EB31F16A9411}
AppName=BilaKey
AppVersion={#Version}
AppVerName=BilaKey {#Version}
AppPublisher=BilaKey contributors
AppPublisherURL=https://github.com/xulytiengviet/Bilakey
AppSupportURL=https://github.com/xulytiengviet/Bilakey/faq
AppUpdatesURL=https://github.com/xulytiengviet/Bilakey/download
VersionInfoVersion={#Version}
DefaultDirName={localappdata}\Programs\BilaKey
DisableDirPage=yes
DisableProgramGroupPage=yes
PrivilegesRequired=lowest
OutputDir=.\out
OutputBaseFilename=BilaKey-{#Version}-{#Arch}-Setup
SetupIconFile=..\App\Vikey.ico
UninstallDisplayIcon={app}\BilaKey.exe
UninstallDisplayName=BilaKey
Compression=lzma2/max
SolidCompression=yes
WizardStyle=modern
CloseApplications=yes
CloseApplicationsFilter=BilaKey.exe
RestartApplications=no
#if Arch == "arm64"
ArchitecturesAllowed=arm64
ArchitecturesInstallIn64BitMode=arm64
#else
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
#endif

[Languages]
Name: "en"; MessagesFile: "compiler:Default.isl"

[Messages]
; the few screens people see, in Vietnamese like the app
WelcomeLabel1=Cài đặt BilaKey
WelcomeLabel2=BilaKey {#Version} sẽ được cài cho tài khoản của bạn, không cần quyền quản trị.%n%nBấm Next để tiếp tục.
FinishedHeadingLabel=Đã cài xong BilaKey
FinishedLabel=BilaKey ở khay hệ thống. Tự nhận diện Anh/Việt/CVNSS4.0 được bật mặc định; Alt+Z chuyển chế độ Telex.
ConfirmUninstall=Gỡ BilaKey khỏi máy?

[Files]
Source: "{#Src}\BilaKey.exe"; DestDir: "{app}"; Flags: ignoreversion

[Icons]
Name: "{autoprograms}\BilaKey"; Filename: "{app}\BilaKey.exe"

[Registry]
; the app's own "Mở BilaKey khi đăng nhập" entry goes away with the app
Root: HKCU; Subkey: "Software\Microsoft\Windows\CurrentVersion\Run"; ValueName: "BilaKey"; Flags: uninsdeletevalue dontcreatekey

[Run]
Filename: "{app}\BilaKey.exe"; Description: "Mở BilaKey"; Flags: nowait postinstall skipifsilent
; No automatic updater is enabled for BilaKey 0.0.1.


[UninstallRun]
Filename: "{sys}\taskkill.exe"; Parameters: "/IM BilaKey.exe /F"; Flags: runhidden; RunOnceId: "KillBilaKey"

