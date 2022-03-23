; NSIS file that creates the game's installer
; File Author: Isaiah Hoffman
; File Created: October 1 2017
; File Updated: March 13, 2019
; Use Unicode
Unicode true

; Modern UI 2 Script Header
!include MUI2.nsh
; Macros for determining if we are running on a 64-bit operating
; system
!include x64.nsh

; General Settings
; Name and file
Name "A Shaping War"
OutFile "tower_defense_setup.exe"

function .onInit
  ; Default installation folder.
  ${If} ${RunningX64}
    StrCpy $INSTDIR "$PROGRAMFILES64\Isaiah Hoffman"
  ${Else}
    StrCpy $INSTDIR "$PROGRAMFILES\Isaiah Hoffman"
  ${EndIf}
FunctionEnd

; Get installation folder from registry if available
InstallDirRegKey HKCU "Software\Isaiah Hoffman" ""

; Functions
; Sets the installation directory and other variables after the directory page.
; Paramters: $INSTDIR --> Holds the directory that the user selected.
; Outputs: $0 --> Holds the directory that the user selected.
; Outputs: $1 --> Holds the directory where the source files (if any) will be placed.
; Outputs: $R0 --> See $INSTDIR
; Outputs: $INSTDIR --> Holds the directory where the binary and other program files are installed to.
Function set_install_dir
  StrCpy $0 $INSTDIR
  StrCpy $INSTDIR "$INSTDIR\tower_defense"
  StrCpy $1 "$INSTDIR\src"
  StrCpy $R0 $INSTDIR
FunctionEnd

; Request application privileges for Windows Vista
RequestExecutionLevel admin
; Install Types
InstType "Typical"
InstType "Full"
InstType /NOCUSTOM

; Pages
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE ".\..\..\license.txt"
!insertmacro MUI_PAGE_COMPONENTS
!define MUI_DIRECTORYPAGE_TEXT_TOP "Please select the root folder to install the game to. To choose a different folder, click Browse and select another folder. Note: The actual game is installed in the subfolder $\"tower_defense$\" of this folder. This subfolder is created automatically by the installer; it does not need to already exist."
!define MUI_PAGE_CUSTOMFUNCTION_LEAVE set_install_dir
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH
!insertmacro MUI_UNPAGE_WELCOME
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

; Languages
!insertmacro MUI_LANGUAGE "English"
; Version Information
VIAddVersionKey /LANG=${LANG_ENGLISH} "ProductName" "A Shaping War Installer"
VIAddVersionKey /LANG=${LANG_ENGLISH} "ProductVersion" "3.4.0.0"
VIAddVersionKey /LANG=${LANG_ENGLISH} "CompanyName" "Isaiah Hoffman"
VIAddVersionKey /LANG=${LANG_ENGLISH} "LegalCopyright" "© Isaiah Hoffman 2018-2022"
VIAddVersionKey /LANG=${LANG_ENGLISH} "FileVersion" "1.4.0.0"
VIAddVersionKey /LANG=${LANG_ENGLISH} "FileDescription" "An installer program for A Shaping War."
VIAddVersionKey /LANG=${LANG_ENGLISH} "InternalName" "tower_defense_setup.exe"
VIAddVersionKey /LANG=${LANG_ENGLISH} "OriginalFilename" "tower_defense_setup.exe"
VIProductVersion 3.4.0.0
VIFileVersion 1.4.0.0

; Installer Sections
Section "Required Files" SectionRequired
  SectionIn 1 2 RO
  ; Stuff for uninstalling later
  WriteRegStr HKCU "Software\Isaiah Hoffman" "" $0
  WriteRegStr HKCU "Software\Isaiah Hoffman\tower_defense" "" $R0
  ; WriteRegStr HKCU "Software\Isaiah Hoffman\tower_defense" "uninstall" $R0
  ; Required files
  CreateDirectory "$R0"
  WriteUninstaller $R0\uninstall.exe
  SetOutPath $R0
  File ".\..\..\license.txt"
  File ".\..\..\readme.txt"
  File ".\..\other\version.txt"
  File ".\..\other\contact.txt"
  File ".\..\other\minimum_requirements.txt"

  ; Directory structure
  CreateDirectory "$R0\config"
  CreateDirectory "$R0\resources"
  CreateDirectory "$R0\resources\levels"
  CreateDirectory "$R0\resources\graphs"
  CreateDirectory "$R0\resources\music"
  CreateDirectory "$R0\userdata"
  CreateDirectory "$R0\screenshots"
  CreateDirectory "$R0\Playing Guide"
  SetOutPath "$R0\screenshots"
  File /a /r ".\..\screenshots\*.*"
  SetOutPath "$R0\Playing Guide"
  File /a /r ".\..\other\Playing Guide\*.*"  
SectionEnd

SectionGroup "Binary Files"
Section "Game Binary Files" SectionGame
  SectionIn 1 2 RO
  SetOutPath $R0
  ${If} ${RunningX64}
  File /oname=tower_defense.exe "..\x64_release_towerdefense.exe"
  File /oname=tower_defense_x86.exe "..\x86_release_towerdefense.exe"
  ${Else}
  File /oname=tower_defense.exe "..\x86_release_towerdefense.exe"
  ${EndIf}
  SetOutPath $R0\config
  File ".\config.ini"
SectionEnd

Section "Resource Files" SectionResources
  SectionIn 1 2 RO
  SetOutPath $R0\resources
  File ".\..\resources\enemies.ini"
  File ".\..\resources\enemies.ini.format"  
  File ".\..\resources\shots.ini"
  File ".\..\resources\shots.ini.format"
  File ".\..\resources\towers.ini"
  File ".\..\resources\towers.ini.format"
  File ".\..\resources\tower_upgrades.ini"
  File ".\..\resources\tower_upgrades.ini.format"
  File ".\..\resources\other.ini"
  SetOutPath $R0\resources\levels
  File /a /r "..\resources\levels\level*.ini"
  File ".\..\resources\levels\level0.ini.format"
  ; File ".\..\resources\levels\levels.xlsx"
  File ".\..\resources\levels\global.ini"
  File ".\..\resources\levels\global.ini.format"
  SetOutPath $R0\resources\graphs
  File ".\..\resources\graphs\air_graph_*.txt"
  File ".\..\resources\graphs\ground_graph_*.txt"
  SetOutPath $R0\resources\music
  File /a /r "..\resources\music\*.wav"
SectionEnd
SectionGroupEnd

SectionGroup "Source Files" SectionSource
  Section "Required Source Files" SectionRQSource
  SectionIn 2 RO
  CreateDirectory "$1"
SectionEnd

Section "Tower defense Source Files" SectionTowerSource
  SectionIn 2 RO
  CreateDirectory "$1\TowerDefense"
  SetOutPath "$1\TowerDefense"
  !define towerPath "..\..\TowerDefense"
  File "${towerPath}\*.cpp"
  File "${towerPath}\*.hpp"
  SetOutPath "$4"
  File "${towerPath}\resource.h"
  File "${towerPath}\resource.rc"
  File "${towerPath}\towerdefense.vcxproj"
  !undef towerPath
SectionEnd
Section "-Tower Defense Solution Files" SectionSolutionSource
  SectionIn 2 RO
  CreateDirectory "$1\.git"
  SetOutPath "$1\.git"
  !define myPath "..\.."
  File /a /r "${myPath}\.git\*.*"
  SetOutPath "$1"
  File "${myPath}\.gitignore"
  File "${myPath}\towerdefense.sln"  
  File "${myPath}\.gitattributes"
  File "${myPath}\my_debug_defaults.props"
  File "${myPath}\my_defaults.props"
  File "${myPath}\my_release_defaults.props"
  File "${myPath}\tower_defense.ruleset"
  !undef myPath
SectionEnd
Section "-Tower Defense Installer Files"
  SectionIn 2 RO
  CreateDirectory "$1\install"
  SetOutPath "$1\install"
  !define myPath "..\"
  File "${myPath}\install\install_script.nsi"
  !undef myPath
SectionEnd
SectionGroupEnd

; Uninstaller Sections
Section "un.Uninstall"
  ; Make sure that we are uninstalling from the correct location
  ReadRegStr $INSTDIR HKCU "Software\Isaiah Hoffman\tower_defense" ""
  ; Delete registry keys
  DeleteRegKey HKCU "Software\Isaiah Hoffman\tower_defense"
  RMDir /r $INSTDIR\install
  RMDir /r $INSTDIR\resources
  RMDir /r $INSTDIR\userdata
  RMDir /r $INSTDIR\config
  RMDir /r $INSTDIR\src
  RMDir /r "$INSTDIR\Playing Guide"
  RMDIR /r $INSTDIR\screenshots
  Delete $INSTDIR\**
  RMDir $INSTDIR
  RMDir "$INSTDIR\..\"
  ; RMDir /r "$LOCALAPPDATA\Isaiah Hoffman\tower_defense\resources"
  ; RMDir /r "$LOCALAPPDATA\Isaiah Hoffman\tower_defense\userdata"
  ; RMDir "$LOCALAPPDATA\Isaiah Hoffman\tower_defense"
  ; RMDir "$LOCALAPPDATA\Isaiah Hoffman"
SectionEnd

; Set section language strings
LangString DESC_SectionRequired ${LANG_ENGLISH} "This component is required to install the game. It includes important stuff such as version history, readme, and license files."
LangString DESC_SectionGame ${LANG_ENGLISH} "Install this component if you simply wish to play the game. This component installs the binary files that can be executed to play the game."
LangString DESC_SectionResources ${LANG_ENGLISH} "Install this component if you simply wish to play the game. This component installs the other files (besides the binaries) that are required to execute the game."
LangString DESC_SectionTerrain ${LANG_ENGLISH} "Install this component if you are interested in creating your own terrain maps to play on."
LangString DESC_SectionSource ${LANG_ENGLISH} "Install this component if you wish to examine or compile the game's source code."
LangString DESC_SectionRQSource ${LANG_ENGLISH} "This component must be installed if you decide to install any source files."
LangString DESC_SectionGraphicsSource ${LANG_ENGLISH} "Install this component if you wish to examine or compile the graphics library that is statically linked into the program."
LangString DESC_SectionTerrainSource ${LANG_ENGLISH} "Install this component if you wish to examine or compile the terrain editor's source files. (It should be noted that the terrain editor depends on the graphics library if you want to compile the code.)"
LangString DESC_SectionTowerSource ${LANG_ENGLISH} "Install this component if you wish to examine or compile the game's primary source files. (It should be noted that the game depends on the graphics library if you want to compile the code.)"

!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${SectionRequired} $(DESC_SectionRequired)
  !insertmacro MUI_DESCRIPTION_TEXT ${SectionGame} $(DESC_SectionGame)
  !insertmacro MUI_DESCRIPTION_TEXT ${SectionResources} $(DESC_SectionResources)
  !insertmacro MUI_DESCRIPTION_TEXT ${SectionSource} $(DESC_SectionSource)
  !insertmacro MUI_DESCRIPTION_TEXT ${SectionRQSource} $(DESC_SectionRQSource)
  !insertmacro MUI_DESCRIPTION_TEXT ${SectionTowerSource} $(DESC_SectionTowerSource)
!insertmacro MUI_FUNCTION_DESCRIPTION_END
