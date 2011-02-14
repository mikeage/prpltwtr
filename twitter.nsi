; Script based on the facebook-chat nsi file
; Updated to support PortablePidgin

SetCompress auto

; todo: SetBrandingImage
; HM NIS Edit Wizard helper defines
!define PRODUCT_NAME "prpltwtr"
!define PRODUCT_VERSION "0.6.0"
!define PRODUCT_PUBLISHER "Neaveru"
!define PRODUCT_WEB_SITE "http://code.google.com/p/prpltwtr/"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define PRODUCT_UNINST_ROOT_KEY "HKLM"

; MUI 1.67 compatible ------
!include "MUI.nsh"

; MUI Settings
!define MUI_ABORTWARNING
!define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\modern-install.ico"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\modern-uninstall.ico"

; Welcome page
!insertmacro MUI_PAGE_WELCOME
; License page
!insertmacro MUI_PAGE_LICENSE "COPYING"
!define MUI_PAGE_CUSTOMFUNCTION_PRE GetPidginInstPath
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_COMPONENTS
; Instfiles page
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

; Uninstaller pages
;!insertmacro MUI_UNPAGE_INSTFILES

; Language files
!insertmacro MUI_LANGUAGE "English"

; MUI end ------

Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"
OutFile "${PRODUCT_NAME}-${PRODUCT_VERSION}.exe"

ShowInstDetails show
ShowUnInstDetails show

Section "${PRODUCT_NAME} core" SEC01
SectionIn RO
    SetOverwrite try
    
	SetOutPath "$INSTDIR\pixmaps\pidgin"
	File "/oname=protocols\16\prpltwtr.png" "data\prpltwtr16.png"
	File "/oname=protocols\22\prpltwtr.png" "data\prpltwtr22.png"
	File "/oname=protocols\48\prpltwtr.png" "data\prpltwtr48.png"

    SetOverwrite try
	copy:
		ClearErrors
		Delete "$INSTDIR\plugins\prpltwtr.dll"
		IfErrors dllbusy
		SetOutPath "$INSTDIR\plugins"
		File "prpltwtr.dll"
		File "gtkprpltwtr\gtkprpltwtr.dll"
		Goto after_copy
	dllbusy:
		MessageBox MB_RETRYCANCEL "prpltwtr.dll is busy. Please close Pidgin (including tray icon) and try again" IDCANCEL cancel
		Goto copy
	cancel:
		Abort "Installation of prpltwtr aborted"
	after_copy:
SectionEnd

Section "${PRODUCT_NAME} dbgsym" SEC02
		File "prpltwtr.dll.dbgsym"
		File "gtkprpltwtr\gtkprpltwtr.dll.dbgsym"
SectionEnd

Function GetPidginInstPath
  Push $0
  ReadRegStr $0 HKLM "Software\pidgin" ""
	IfFileExists "$0\pidgin.exe" cont
	ReadRegStr $0 HKCU "Software\pidgin" ""
	IfFileExists "$0\pidgin.exe" cont
	StrCpy $0 "C:\PortableApps\PidginPortable\App\Pidgin"	
	IfFileExists "$0\pidgin-portable.exe" cont
	StrCpy $0 "$PROGRAMFILES\PortableApps\PidginPortable\App\Pidgin"	
	IfFileExists "$0\pidgin-portable.exe" cont
	StrCpy $0 "$PROGRAMFILES\PortableApps\PortableApps\PidginPortable\App\Pidgin"	
	IfFileExists "$0\pidgin-portable.exe" cont
	MessageBox MB_OK|MB_ICONINFORMATION "Failed to automatically find a Pidgin installation. Please select your Pidgin directory directly [if you are using PortablePidgin, please select the App\Pidgin subdirectory]"
	Goto done
  cont:
	StrCpy $INSTDIR $0
  done:
FunctionEnd
