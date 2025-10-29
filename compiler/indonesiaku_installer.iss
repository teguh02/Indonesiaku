; ============================================================================
; Inno Setup Script - Bahasa Pemrograman Indonesiaku v0.1
; ============================================================================
; Installer profesional untuk Bahasa Pemrograman Indonesiaku
; Otomatis menambahkan ke PATH dan membuat shortcuts
; ============================================================================

[Setup]
; Informasi aplikasi
AppName=Bahasa Pemrograman Indonesiaku
AppVersion=0.1.4
AppPublisher=Teguh Rijanandi
AppPublisherURL=https://github.com/teguhriyan/Indonesiaku
AppSupportURL=https://github.com/teguhriyan/Indonesiaku/issues
AppUpdatesURL=https://github.com/teguhriyan/Indonesiaku/releases

; Setup behavior
DefaultDirName={pf}\Indonesiaku
DefaultGroupName=Bahasa Pemrograman Indonesiaku
AllowNoIcons=yes
UninstallDisplayIcon={app}\bin\indonesiaku.exe

; Compression dan output
Compression=lzma
SolidCompression=yes
OutputBaseFilename=Indonesiaku-Setup-v0.1.4
OutputDir=..\dist

; Permissions
PrivilegesRequired=admin

; Language & appearance
; LangDetectionMethod=uilanguage
ShowLanguageDialog=no
LanguageDetectionMethod=locale

[Messages]
; Custom messages dalam Bahasa Indonesia
WelcomeLabel1=Selamat datang di Installer Bahasa Pemrograman Indonesiaku
WelcomeLabel2=Installer ini akan memasang Bahasa Pemrograman Indonesiaku v0.1 ke komputer Anda.%n%nDisarankan untuk menutup semua aplikasi sebelum melanjutkan.
ReadyLabel1=Installer siap untuk memasang Bahasa Pemrograman Indonesiaku.
ReadyLabel2a=Klik Pasang untuk melanjutkan, atau klik Batal jika Anda ingin melihat atau mengubah pengaturan.
FinishedHeadingLabel=Instalasi Bahasa Pemrograman Indonesiaku Selesai
FinishedLabelNoIcons=Installer telah selesai memasang Bahasa Pemrograman Indonesiaku di komputer Anda.
FinishedLabel=Installer telah selesai memasang Bahasa Pemrograman Indonesiaku di komputer Anda. Anda dapat menjalankan aplikasi dengan menggunakan shortcuts yang telah dibuat.

; Files to install
[Files]
; Executable utama
Source: "bin\indonesiaku.exe"; DestDir: "{app}\bin"; Flags: ignoreversion

; Documentation
Source: "..\docs\*"; DestDir: "{app}\docs"; Flags: recursesubdirs ignoreversion
Source: "..\README.md"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\LICENSE"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\CHANGELOG.md"; DestDir: "{app}"; Flags: ignoreversion

; Examples
Source: "..\examples\*"; DestDir: "{app}\examples"; Flags: recursesubdirs ignoreversion

; Start Menu shortcuts
[Icons]
; Start Menu - Main application
Name: "{group}\Bahasa Indonesiaku (REPL)"; Filename: "{app}\bin\indonesiaku.exe"; IconFilename: "{app}\bin\indonesiaku.exe"; IconIndex: 0; Comment: "Jalankan Bahasa Indonesiaku dalam mode interaktif"

; Start Menu - Documentation
Name: "{group}\Dokumentasi"; Filename: "{app}\docs\README.md"; IconFilename: "{app}\bin\indonesiaku.exe"; Comment: "Baca dokumentasi Bahasa Indonesiaku"
Name: "{group}\Contoh Program"; Filename: "{app}\examples"; IconFilename: "{app}\bin\indonesiaku.exe"; Comment: "Lihat contoh program Indonesiaku"

; Start Menu - Uninstall
Name: "{group}\Copot Instalasi"; Filename: "{uninstallexe}"

; Desktop shortcut (optional)
Name: "{commondesktop}\Bahasa Indonesiaku"; Filename: "{app}\bin\indonesiaku.exe"; IconFilename: "{app}\bin\indonesiaku.exe"; IconIndex: 0; Tasks: desktopicon

; Run section - Post-install actions
[Run]
; Show README setelah install dengan Notepad
Filename: "notepad.exe"; Parameters: "{app}\README.md"; Description: "Lihat README"; Flags: nowait postinstall skipifsilent

; Tasks - Optional installations
[Tasks]
; Create desktop shortcut
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

; Registry - Add to PATH
[Registry]
; Add bin directory to Windows PATH
Root: HKLM; Subkey: "SYSTEM\CurrentControlSet\Control\Session Manager\Environment"; ValueType: expandsz; ValueName: "Path"; ValueData: "{olddata};{app}\bin"; Flags: preservestringtype

; File associations - .idk files
Root: HKCR; Subkey: ".idk"; ValueType: string; ValueName: ""; ValueData: "IndonesiakuFile"; Flags: uninsdeletevalue
Root: HKCR; Subkey: "IndonesiakuFile"; ValueType: string; ValueName: ""; ValueData: "Bahasa Pemrograman Indonesiaku"; Flags: uninsdeletekey
Root: HKCR; Subkey: "IndonesiakuFile\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\bin\indonesiaku.exe,0"
Root: HKCR; Subkey: "IndonesiakuFile\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\bin\indonesiaku.exe"" ""%1"""

; Code section - Custom functions
[Code]
procedure CurStepChanged(CurStep: TSetupStep);
begin
  if CurStep = ssPostInstall then
  begin
    MsgBox(
      'Instalasi Bahasa Pemrograman Indonesiaku selesai!' + #13#13 +
      'Anda sekarang dapat menggunakan "indonesiaku" dari mana saja di Command Prompt atau PowerShell.' + #13#13 +
      'Contoh penggunaan:' + #13 +
      '  - REPL: indonesiaku' + #13 +
      '  - Run file: indonesiaku program.idk' + #13#13 +
      'Dokumentasi tersedia di: {app}\docs\' + #13 +
      'Contoh program tersedia di: {app}\examples\',
      mbInformation,
      MB_OK
    );
  end;
end;