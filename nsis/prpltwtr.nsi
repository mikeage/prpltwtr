; Script based on the facebook-chat nsi file
; Updated to support PortablePidgin

SetCompress auto

; todo: SetBrandingImage
; HM NIS Edit Wizard helper defines
!define PRODUCT_NAME "prpltwtr"
!define PRODUCT_PUBLISHER "Neaveru"
!define PRODUCT_WEB_SITE "http://code.google.com/p/prpltwtr/"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define PRODUCT_UNINST_ROOT_KEY "HKLM"

!define BASEDIR ".."

; MUI 1.67 compatible ------
!include "MUI.nsh"

; MUI Settings
!define MUI_ABORTWARNING
!define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\modern-install.ico"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\modern-uninstall.ico"

; Welcome page
!insertmacro MUI_PAGE_WELCOME
; License page
!insertmacro MUI_PAGE_LICENSE "${BASEDIR}\COPYING"
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

Name "${PRODUCT_NAME} ${PACKAGE_VERSION}"
OutFile "${BASEDIR}\${PRODUCT_NAME}-${PACKAGE_VERSION}.exe"

ShowInstDetails show
ShowUnInstDetails show



Section "${PRODUCT_NAME} core" SEC01
SectionIn RO
    SetOverwrite try
    
	SetOutPath "$INSTDIR\pixmaps\pidgin"
	SetOutPath "$INSTDIR\pixmaps\pidgin\protocols\16\"
	File "${BASEDIR}\data\16\prpltwtr.png"
	SetOutPath "$INSTDIR\pixmaps\pidgin\protocols\22\"
	File "${BASEDIR}\data\22\prpltwtr.png"
	SetOutPath "$INSTDIR\pixmaps\pidgin\protocols\48\"
	File "${BASEDIR}\data\48\prpltwtr.png"

    SetOverwrite try
	copy:
		ClearErrors
# Original names
		Delete "$INSTDIR\plugins\prpltwtr.dll"
		Delete "$INSTDIR\plugins\prpltwtr.dll.dbgsym"
		Delete "$INSTDIR\plugins\gtkprpltwtr.dll"
		Delete "$INSTDIR\plugins\gtkprpltwtr.dll.dbgsym"

# New names
		Delete "$INSTDIR\plugins\libprpltwtr_statusnet.dll"
		Delete "$INSTDIR\plugins\libprpltwtr_statusnet.dll.dbgsym"
		Delete "$INSTDIR\plugins\libprpltwtr_twitter.dll"
		Delete "$INSTDIR\plugins\libprpltwtr_twitter.dll.dbgsym"
		Delete "$INSTDIR\plugins\libgtkprpltwtr.dll.dbgsym"
		Delete "$INSTDIR\plugins\libgtkprpltwtr.dll"

		Delete "$INSTDIR\libprpltwtr.dll"
		Delete "$INSTDIR\libprpltwtr.dll.dbgsym"
		IfErrors dllbusy
		SetOutPath "$INSTDIR"
		File "${BASEDIR}\src\prpltwtr\libprpltwtr.dll"
		SetOutPath "$INSTDIR\plugins"
		File "${BASEDIR}\src\prpltwtr\libprpltwtr_statusnet.dll"
		File "${BASEDIR}\src\prpltwtr\libprpltwtr_twitter.dll"
		File "${BASEDIR}\src\gtkprpltwtr\libgtkprpltwtr.dll"
		Goto after_copy
	dllbusy:
		MessageBox MB_RETRYCANCEL "prpltwtr.dll is busy. Please close Pidgin (including tray icon) and try again" IDCANCEL cancel
		Goto copy
	cancel:
		Abort "Installation of prpltwtr aborted"
	after_copy:
SectionEnd

Section "${PRODUCT_NAME} dbgsym" SEC02
		SetOutPath "$INSTDIR"
		File "${BASEDIR}\src\prpltwtr\libprpltwtr.dll.dbgsym"
		SetOutPath "$INSTDIR\plugins"
		File "${BASEDIR}\src\prpltwtr\libprpltwtr_twitter.dll.dbgsym"
		File "${BASEDIR}\src\prpltwtr\libprpltwtr_statusnet.dll.dbgsym"
		File "${BASEDIR}\src\gtkprpltwtr\libgtkprpltwtr.dll.dbgsym"
SectionEnd

Section "${PRODUCT_NAME} localizations" SEC03
	!include po_prpltwtr.nsi
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
