; weasel installation script
!include FileFunc.nsh
!include LogicLib.nsh
!include MUI2.nsh
!include x64.nsh

Unicode true

!ifndef WEASEL_VERSION
!define WEASEL_VERSION 1.0.0
!endif

!ifndef WEASEL_BUILD
!define WEASEL_BUILD 0
!endif

!define WEASEL_ROOT $INSTDIR\weasel-${WEASEL_VERSION}

; The name of the installer
Name "小狼毫 ${WEASEL_VERSION}"

; The file to write
OutFile "archives\weasel-${WEASEL_VERSION}.${WEASEL_BUILD}-installer.exe"

VIProductVersion "${WEASEL_VERSION}.${WEASEL_BUILD}"
VIAddVersionKey /LANG=2052 "ProductName" "小狼毫"
VIAddVersionKey /LANG=2052 "Comments" "Powered by RIME | 中州韵输入法引擎"
VIAddVersionKey /LANG=2052 "CompanyName" "式恕堂"
VIAddVersionKey /LANG=2052 "LegalCopyright" "Copyleft RIME Developers"
VIAddVersionKey /LANG=2052 "FileDescription" "小狼毫输入法"
VIAddVersionKey /LANG=2052 "FileVersion" "${WEASEL_VERSION}"

!define MUI_ICON ..\resource\weasel.ico
SetCompressor /SOLID lzma

; The default installation directory
InstallDir $PROGRAMFILES\Rime

; Registry key to check for directory (so if you install again, it will
; overwrite the old one automatically)
InstallDirRegKey HKLM "Software\Rime\Weasel" "InstallDir"

; Request application privileges for Windows Vista
RequestExecutionLevel admin

;--------------------------------

; Pages

!insertmacro MUI_PAGE_LICENSE "LICENSE.txt"
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

;--------------------------------

; Languages

!insertmacro MUI_LANGUAGE "SimpChinese"
!insertmacro MUI_LANGUAGE "TradChinese"
!insertmacro MUI_LANGDLL_DISPLAY

;--------------------------------
; 自定义字符串双语化
;--------------------------------

; 升级提示
LangString UPGRADE_MSG ${LANG_SIMPCHINESE} "安装前，我先卸载旧版本的小狼毫。$\n$\n按下「确定」移除旧版本，按下「取消」放弃本次安装。"
LangString UPGRADE_MSG ${LANG_TRADCHINESE} "安裝前，我打盤先卸載舊版本的小狼毫。$\n$\n按下「確定」移除舊版本，按下「取消」放棄本次安裝。"

; 输入法名称
LangString IME_NAME ${LANG_SIMPCHINESE} "小狼毫输入法"
LangString IME_NAME ${LANG_TRADCHINESE} "小狼毫輸入法"

; 开始菜单快捷方式
LangString SHORTCUT_README ${LANG_SIMPCHINESE} "【小狼毫】说明书"
LangString SHORTCUT_README ${LANG_TRADCHINESE} "【小狼毫】說明書"

LangString SHORTCUT_SETTINGS ${LANG_SIMPCHINESE} "【小狼毫】输入法设定"
LangString SHORTCUT_SETTINGS ${LANG_TRADCHINESE} "【小狼毫】輸入法設定"

LangString SHORTCUT_DICT ${LANG_SIMPCHINESE} "【小狼毫】用户词典管理"
LangString SHORTCUT_DICT ${LANG_TRADCHINESE} "【小狼毫】用戶詞典管理"

LangString SHORTCUT_SYNC ${LANG_SIMPCHINESE} "【小狼毫】用户资料同步"
LangString SHORTCUT_SYNC ${LANG_TRADCHINESE} "【小狼毫】用戶資料同步"

LangString SHORTCUT_DEPLOY ${LANG_SIMPCHINESE} "【小狼毫】重新部署"
LangString SHORTCUT_DEPLOY ${LANG_TRADCHINESE} "【小狼毫】重新部署"

LangString SHORTCUT_SERVER ${LANG_SIMPCHINESE} "小狼毫算法服务"
LangString SHORTCUT_SERVER ${LANG_TRADCHINESE} "小狼毫算法服務"

LangString SHORTCUT_USERDIR ${LANG_SIMPCHINESE} "【小狼毫】用户文件夹"
LangString SHORTCUT_USERDIR ${LANG_TRADCHINESE} "【小狼毫】用戶文件夾"

LangString SHORTCUT_WEASELDIR ${LANG_SIMPCHINESE} "【小狼毫】程序文件夹"
LangString SHORTCUT_WEASELDIR ${LANG_TRADCHINESE} "【小狼毫】程序文件夾"

LangString SHORTCUT_UPDATE ${LANG_SIMPCHINESE} "【小狼毫】检查新版本"
LangString SHORTCUT_UPDATE ${LANG_TRADCHINESE} "【小狼毫】檢查新版本"

LangString SHORTCUT_SETUP ${LANG_SIMPCHINESE} "【小狼毫】安装选项"
LangString SHORTCUT_SETUP ${LANG_TRADCHINESE} "【小狼毫】安裝選項"

LangString SHORTCUT_UNINSTALL ${LANG_SIMPCHINESE} "卸载小狼毫"
LangString SHORTCUT_UNINSTALL ${LANG_TRADCHINESE} "卸載小狼毫"

;--------------------------------

Function .onInit
  !insertmacro MUI_LANGDLL_DISPLAY

  ReadRegStr $R0 HKLM \
  "Software\Microsoft\Windows\CurrentVersion\Uninstall\Weasel" \
  "UninstallString"
  StrCmp $R0 "" done

  StrCpy $0 "Upgrade"
  IfSilent uninst 0
  MessageBox MB_OKCANCEL|MB_ICONINFORMATION \
  $(UPGRADE_MSG) \
  IDOK uninst
  Abort

uninst:
  ; Backup data directory from previous installation, user files may exist
  ReadRegStr $R1 HKLM SOFTWARE\Rime\Weasel "WeaselRoot"
  StrCmp $R1 "" call_uninstaller
  IfFileExists $R1\data\*.* 0 call_uninstaller
  CreateDirectory $TEMP\weasel-backup
  CopyFiles $R1\data\*.* $TEMP\weasel-backup

call_uninstaller:
  ExecWait '$R0 /S'
  Sleep 800

done:
FunctionEnd

; The stuff to install
Section "Weasel"

  SectionIn RO

  ; Write the new installation path into the registry
  WriteRegStr HKLM SOFTWARE\Rime\Weasel "InstallDir" "$INSTDIR"

  ; Reset INSTDIR for the new version
  StrCpy $INSTDIR "${WEASEL_ROOT}"

  IfFileExists "$INSTDIR\WeaselServer.exe" 0 +2
  ExecWait '"$INSTDIR\WeaselServer.exe" /quit'

  SetOverwrite try
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR

  IfFileExists $TEMP\weasel-backup\*.* 0 program_files
  CreateDirectory $INSTDIR\data
  CopyFiles $TEMP\weasel-backup\*.* $INSTDIR\data
  RMDir /r $TEMP\weasel-backup

program_files:
  File "LICENSE.txt"
  File "README.txt"
  File "7-zip-license.txt"
  File "7z.dll"
  File "7z.exe"
  File "COPYING-curl.txt"
  File "curl.exe"
  File "curl-ca-bundle.crt"
  File "rime-install.bat"
  File "rime-install-config.bat"
  File "weasel.dll"
  ${If} ${RunningX64}
    File "weaselx64.dll"
  ${EndIf}
  File "weaselt.dll"
  ${If} ${RunningX64}
    File "weaseltx64.dll"
  ${EndIf}
  File "weasel.ime"
  ${If} ${RunningX64}
    File "weaselx64.ime"
  ${EndIf}
  File "weaselt.ime"
  ${If} ${RunningX64}
    File "weaseltx64.ime"
  ${EndIf}
  File "WeaselDeployer.exe"
  File "WeaselServer.exe"
  File "WeaselSetup.exe"
  File "rime.dll"
  File "WinSparkle.dll"
  ; shared data files
  SetOutPath $INSTDIR\data
  File "data\default.yaml"
  File "data\key_bindings.yaml"
  File "data\punctuation.yaml"
  File "data\symbols.yaml"
  File "data\weasel.yaml"
  File "data\essay.txt"
  File "data\*.schema.yaml"
  File "data\*.dict.yaml"
  ; opencc data files
  SetOutPath $INSTDIR\data\opencc
  File "data\opencc\*.json"
  File "data\opencc\*.ocd"
  ; images
  SetOutPath $INSTDIR\data\preview
  File "data\preview\*.png"

  SetOutPath $INSTDIR

  ; 根据安装程序语言选择决定 WeaselSetup 参数
  ${If} $LANGUAGE == 2052     ; 简体中文
    StrCpy $R2 "/s"
  ${ElseIf} $LANGUAGE == 1028  ; 繁体中文
    StrCpy $R2 "/t"
  ${Else}
    StrCpy $R2 "/i"
  ${EndIf}
  ; 命令行参数可覆盖语言选择（保留静默安装兼容性）
  ${GetParameters} $R0
  ClearErrors
  ${GetOptions} $R0 "/S" $R1
  IfErrors +2 0
  StrCpy $R2 "/s"
  ${GetOptions} $R0 "/T" $R1
  IfErrors +2 0
  StrCpy $R2 "/t"

  ExecWait '"$INSTDIR\WeaselSetup.exe" $R2'

  ; run as user...
  ExecWait "$INSTDIR\WeaselDeployer.exe /install"

  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Weasel" "DisplayName" "$(IME_NAME)"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Weasel" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Weasel" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Weasel" "NoRepair" 1
  WriteUninstaller "$INSTDIR\uninstall.exe"

  ; Write autorun key
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Run" "WeaselServer" "$INSTDIR\WeaselServer.exe"
  ; Start WeaselServer
  Exec "$INSTDIR\WeaselServer.exe"

  ; Prompt reboot
  StrCmp $0 "Upgrade" 0 +2
  SetRebootFlag true

SectionEnd

; Optional section (can be disabled by the user)
Section "Start Menu Shortcuts"
  SetShellVarContext all
  CreateDirectory "$SMPROGRAMS\$(IME_NAME)"
  CreateShortCut "$SMPROGRAMS\$(IME_NAME)\$(SHORTCUT_README).lnk" "$INSTDIR\README.txt"
  CreateShortCut "$SMPROGRAMS\$(IME_NAME)\$(SHORTCUT_SETTINGS).lnk" "$INSTDIR\WeaselDeployer.exe" "" "$SYSDIR\shell32.dll" 21
  CreateShortCut "$SMPROGRAMS\$(IME_NAME)\$(SHORTCUT_DICT).lnk" "$INSTDIR\WeaselDeployer.exe" "/dict" "$SYSDIR\shell32.dll" 6
  CreateShortCut "$SMPROGRAMS\$(IME_NAME)\$(SHORTCUT_SYNC).lnk" "$INSTDIR\WeaselDeployer.exe" "/sync" "$SYSDIR\shell32.dll" 26
  CreateShortCut "$SMPROGRAMS\$(IME_NAME)\$(SHORTCUT_DEPLOY).lnk" "$INSTDIR\WeaselDeployer.exe" "/deploy" "$SYSDIR\shell32.dll" 144
  CreateShortCut "$SMPROGRAMS\$(IME_NAME)\$(SHORTCUT_SERVER).lnk" "$INSTDIR\WeaselServer.exe" "" "$INSTDIR\WeaselServer.exe" 0
  CreateShortCut "$SMPROGRAMS\$(IME_NAME)\$(SHORTCUT_USERDIR).lnk" "$INSTDIR\WeaselServer.exe" "/userdir" "$SYSDIR\shell32.dll" 126
  CreateShortCut "$SMPROGRAMS\$(IME_NAME)\$(SHORTCUT_WEASELDIR).lnk" "$INSTDIR\WeaselServer.exe" "/weaseldir" "$SYSDIR\shell32.dll" 19
  CreateShortCut "$SMPROGRAMS\$(IME_NAME)\$(SHORTCUT_UPDATE).lnk" "$INSTDIR\WeaselServer.exe" "/update" "$SYSDIR\shell32.dll" 13
  CreateShortCut "$SMPROGRAMS\$(IME_NAME)\$(SHORTCUT_SETUP).lnk" "$INSTDIR\WeaselSetup.exe" "" "$SYSDIR\shell32.dll" 162
  CreateShortCut "$SMPROGRAMS\$(IME_NAME)\$(SHORTCUT_UNINSTALL).lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0

SectionEnd

;--------------------------------

; Uninstaller

Section "Uninstall"

  ExecWait '"$INSTDIR\WeaselServer.exe" /quit'

  ExecWait '"$INSTDIR\WeaselSetup.exe" /u'

  ; Remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Weasel"
  DeleteRegValue HKLM "Software\Microsoft\Windows\CurrentVersion\Run" "WeaselServer"
  DeleteRegKey HKLM SOFTWARE\Rime

  ; Remove files and uninstaller
  SetOutPath $TEMP
  Delete /REBOOTOK "$INSTDIR\data\opencc\*.*"
  Delete /REBOOTOK "$INSTDIR\data\preview\*.*"
  Delete /REBOOTOK "$INSTDIR\data\*.*"
  Delete /REBOOTOK "$INSTDIR\*.*"
  RMDir /REBOOTOK "$INSTDIR\data\opencc"
  RMDir /REBOOTOK "$INSTDIR\data\preview"
  RMDir /REBOOTOK "$INSTDIR\data"
  RMDir /REBOOTOK "$INSTDIR"
  SetShellVarContext all
  Delete /REBOOTOK "$SMPROGRAMS\$(IME_NAME)\*.*"
  RMDir /REBOOTOK "$SMPROGRAMS\$(IME_NAME)"

  ; Prompt reboot
  SetRebootFlag true

SectionEnd
