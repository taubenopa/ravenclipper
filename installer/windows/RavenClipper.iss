; RavenClipper Windows-Installer (Inno Setup 6).  Build:  ISCC installer\windows\RavenClipper.iss
#define AppName    "RavenClipper"
#ifndef AppVersion
  #define AppVersion "1.0.0"
#endif
#define BundleDir  "..\..\build\RavenClipper_artefacts\Release\VST3\RavenClipper.vst3"

[Setup]
AppId={{4F4E3230-DA5B-4E40-BF51-DB4D7BC04DC0}
AppName={#AppName}
AppVersion={#AppVersion}
AppPublisher=RavenClipper
; VST3-Plugins gehören nach C:\Program Files\Common Files\VST3
DefaultDirName={commoncf64}\VST3
AppendDefaultDirName=no
DirExistsWarning=no
DisableProgramGroupPage=yes
PrivilegesRequired=admin
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
UninstallFilesDir={commonappdata}\RavenClipper
UninstallDisplayName={#AppName} (VST3)
UninstallDisplayIcon={commonappdata}\RavenClipper\unins000.exe
SetupIconFile=..\..\assets\icon.ico
WizardImageFile=..\..\assets\wizard.bmp
WizardSmallImageFile=..\..\assets\wizard_small.bmp
WizardImageBackColor=$120A0B
WizardStyle=modern
Compression=lzma2
SolidCompression=yes
OutputDir=..\..\dist
OutputBaseFilename=RavenClipper-{#AppVersion}-Windows-Setup

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"
Name: "german";  MessagesFile: "compiler:Languages\German.isl"

[Messages]
english.WelcomeLabel1=The raven is about to land
english.WelcomeLabel2=This will install [name/ver] (VST3) on your computer.%n%nSharp transients, no harsh edges: a soft clipper for beats, watched over by ravens.
english.FinishedHeadingLabel=The raven has landed
english.FinishedLabel=RavenClipper is installed. Rescan your plugins in your DAW (FL Studio: Options > Manage plugins > Find plugins) and look for RavenClipper.
german.WelcomeLabel1=Der Rabe setzt zur Landung an
german.WelcomeLabel2=Dieser Assistent installiert [name/ver] (VST3) auf deinem Computer.%n%nKnackige Transienten, keine harten Kanten: ein Soft Clipper für Beats, bewacht von Raben.
german.FinishedHeadingLabel=Der Rabe ist gelandet
german.FinishedLabel=RavenClipper ist installiert. Scanne in deiner DAW die Plugins neu (FL Studio: Optionen > Plugins verwalten > Plugins suchen) und suche nach RavenClipper.

[Files]
Source: "{#BundleDir}\*"; DestDir: "{app}\RavenClipper.vst3"; Flags: ignoreversion recursesubdirs createallsubdirs
