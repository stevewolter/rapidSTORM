; The right way, but does not work somehow. Did no investigation.
; [Run]
; Filename: "java.exe"; Parameters: "-jar {app}\share\java\rapidSTORM.jar --set-install-dir-key-system '{app}'; pause.exe"; Flags: waituntilidle

[Registry]
; Try to install system-wide key. Should work for admin users and fail silently
; for others
Root: HKLM; Subkey: "Software\JavaSoft\Prefs\de\uni_bielefeld\physik\rapid/S/T/O/R/M\/D/Storm"; ValueType: string; ValueName: "install_dir"; ValueData: "{app}"; Flags: noerror uninsdeletevalue uninsdeletekeyifempty
Root: HKCU; Subkey: "Software\JavaSoft\Prefs\de\uni_bielefeld\physik\rapid/S/T/O/R/M\/D/Storm"; ValueType: string; ValueName: "install_dir"; ValueData: "{app}"; Flags: uninsdeletevalue uninsdeletekeyifempty
