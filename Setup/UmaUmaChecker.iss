#define ApplicationName 'ウマウマチェッカー'
#define ApplicationVersion GetVersionNumbersString('UmaUmaChecker.exe')
[Setup]
AppName={#ApplicationName}
AppVerName={#ApplicationName} {#ApplicationVersion}
VersionInfoVersion={#ApplicationVersion}
OutputBaseFilename=UmaUmaChecker
DefaultDirName=C:\UmaUmaChecker

[Languages]
Name: japanese; MessagesFile: compiler:Languages/Japanese.isl
