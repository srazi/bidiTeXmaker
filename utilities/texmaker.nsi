Name "Texmaker"
OutFile "texmakerwin32_install.exe"

Icon "install.ico"
UninstallIcon "uninstall.ico"

LicenseText "You must agree to the GPL license before installing."
LicenseData "license.txt"


DirText "Choose directory for installing Texmaker"

InstallDir "$PROGRAMFILES\Texmaker"

Section "Install"
  SetOutPath $INSTDIR
  File "texmakerwin32\AUTHORS"
  File "texmakerwin32\CHANGELOG.txt"
  File "texmakerwin32\COPYING"
  File "texmakerwin32\doc10.png"
  File "texmakerwin32\doc11.png"
  File "texmakerwin32\doc12.png"
  File "texmakerwin32\doc13.png"
  File "texmakerwin32\doc14.png"
  File "texmakerwin32\doc15.png"
  File "texmakerwin32\doc16.png"
  File "texmakerwin32\doc17.png"
  File "texmakerwin32\doc18.png"
  File "texmakerwin32\doc19.png"
  File "texmakerwin32\doc1.png"
  File "texmakerwin32\doc2.png"
  File "texmakerwin32\doc3.png"
  File "texmakerwin32\doc4.png"
  File "texmakerwin32\doc5.png"
  File "texmakerwin32\doc6.png"
  File "texmakerwin32\doc7.png"
  File "texmakerwin32\doc8.png"
  File "texmakerwin32\doc9.png"
  File "texmakerwin32\doc6bis.png"
  File "texmakerwin32\doc20.png"
  File "texmakerwin32\doc21.png"
  File "texmakerwin32\doc22.png"
  File "texmakerwin32\doc10hu.png"
  File "texmakerwin32\doc11hu.png"
  File "texmakerwin32\doc12hu.png"
  File "texmakerwin32\doc13hu.png"
  File "texmakerwin32\doc14hu.png"
  File "texmakerwin32\doc15hu.png"
  File "texmakerwin32\doc16hu.png"
  File "texmakerwin32\doc17hu.png"
  File "texmakerwin32\doc1hu.png"
  File "texmakerwin32\doc20hu.png"
  File "texmakerwin32\doc21hu.png"
  File "texmakerwin32\doc22hu.png"
  File "texmakerwin32\doc2hu.png"
  File "texmakerwin32\doc3hu.png"
  File "texmakerwin32\doc4hu.png"
  File "texmakerwin32\doc5hu.png"
  File "texmakerwin32\doc6ahu.png"
  File "texmakerwin32\doc6hu.png"
  File "texmakerwin32\doc7hu.png"
  File "texmakerwin32\doc8hu.png"
  File "texmakerwin32\doc9hu.png"
  File "texmakerwin32\usermanual_en.html"
  File "texmakerwin32\usermanual_fr.html"
  File "texmakerwin32\usermanual_hu.html"
  File "texmakerwin32\usermanual_ru.html"
  File "texmakerwin32\latexhelp.html"
  File "texmakerwin32\mingwm10.dll"
  File "texmakerwin32\libgcc_s_dw2-1.dll"
  File "texmakerwin32\QtCore4.dll"
  File "texmakerwin32\QtGui4.dll"
  File "texmakerwin32\QtNetwork4.dll"
  File "texmakerwin32\QtXml4.dll"
  File "texmakerwin32\QtWebKit4.dll"
  File "texmakerwin32\QtXmlPatterns4.dll"
  File "texmakerwin32\phonon4.dll"
  File "texmakerwin32\libfreetype-6.dll"
  File "texmakerwin32\libgcc_s_sjlj-1.dll"
  File "texmakerwin32\libjpeg-8.dll"
  File "texmakerwin32\libpng14-14.dll"
  File "texmakerwin32\libpoppler-6.dll"
  File "texmakerwin32\libstdc++-6.dll"
  File "texmakerwin32\zlib1.dll"
  File "texmakerwin32\texmaker.exe"
  File "texmakerwin32\texmaker.ico"
  File "texmakerwin32\qt_cs.qm"
  File "texmakerwin32\qt_de.qm"
  File "texmakerwin32\qt_es.qm"
  File "texmakerwin32\qt_fr.qm"
  File "texmakerwin32\qt_pt.qm"
  File "texmakerwin32\qt_ru.qm"
  File "texmakerwin32\qt_zh_CN.qm"
  File "texmakerwin32\qt_zh_TW.qm"
  File "texmakerwin32\qt_fa.qm"
  File "texmakerwin32\qt_pl.qm"
  File "texmakerwin32\qt_nl.qm"
  File "texmakerwin32\texmaker_de.qm"
  File "texmakerwin32\texmaker_es.qm"
  File "texmakerwin32\texmaker_fa.qm"
  File "texmakerwin32\texmaker_fr.qm"
  File "texmakerwin32\texmaker_gl.qm"
  File "texmakerwin32\texmaker_it.qm"
  File "texmakerwin32\texmaker_pt_BR.qm"
  File "texmakerwin32\texmaker_ru.qm"
  File "texmakerwin32\texmaker_zh_CN.qm"
  File "texmakerwin32\texmaker_zh_TW.qm"
  File "texmakerwin32\texmaker_cs.qm"
  File "texmakerwin32\texmaker_nl.qm"
  File "texmakerwin32\texmaker_pl.qm"
  File "texmakerwin32\texmaker_hu.qm"
  File "texmakerwin32\texmaker_sr.qm"
  File "texmakerwin32\texmaker_el.qm"
  File "texmakerwin32\nl_NL.aff"
  File "texmakerwin32\nl_NL.dic"
  File "texmakerwin32\de_DE.aff"
  File "texmakerwin32\de_DE.dic"
  File "texmakerwin32\en_GB.aff"
  File "texmakerwin32\en_GB.dic"
  File "texmakerwin32\es_ES.aff"
  File "texmakerwin32\es_ES.dic"
  File "texmakerwin32\fr_FR.aff"
  File "texmakerwin32\fr_FR.dic"
  File "texmakerwin32\it_IT.aff"
  File "texmakerwin32\it_IT.dic"
  File "texmakerwin32\hu_HU.aff"
  File "texmakerwin32\hu_HU.dic"
  WriteRegStr HKCR ".tex" "" "texmaker.doc"
  WriteRegStr HKCR "texmaker.doc" "" "LaTeX document"
  WriteRegStr HKCR "texmaker.doc\DefaultIcon" "" "$INSTDIR\texmaker.ico"
  WriteRegStr HKCR "texmaker.doc\shell" "" "open"
  WriteRegStr HKCR "texmaker.doc\shell\open\command" "" '$INSTDIR\texmaker.exe "%1"'
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Texmaker" "DisplayName" "Texmaker"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Texmaker" "UninstallString" "$INSTDIR\uninstall.exe"
  WriteUninstaller "uninstall.exe"
SectionEnd

Section "Shortcuts"
  SetOutPath "$SMPROGRAMS\Texmaker"
  CreateShortCut "$SMPROGRAMS\Texmaker\Texmaker.lnk""$INSTDIR\texmaker.exe"
  SetOutPath "$DESKTOP"
  CreateShortCut "$DESKTOP\Texmaker.lnk""$INSTDIR\texmaker.exe"
SectionEnd

UninstallText "This will uninstall Texmaker from your system"

Section "Uninstall"
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Texmaker"
  Delete "$SMPROGRAMS\Texmaker\Texmaker.lnk"
  RmDir "$SMPROGRAMS\Texmaker"
  Delete "$DESKTOP\Texmaker.lnk"
  Delete "$INSTDIR\AUTHORS"
  Delete "$INSTDIR\CHANGELOG.txt"
  Delete "$INSTDIR\COPYING"
  Delete "$INSTDIR\doc10.png"
  Delete "$INSTDIR\doc11.png"
  Delete "$INSTDIR\doc12.png"
  Delete "$INSTDIR\doc13.png"
  Delete "$INSTDIR\doc14.png"
  Delete "$INSTDIR\doc15.png"
  Delete "$INSTDIR\doc16.png"
  Delete "$INSTDIR\doc17.png"
  Delete "$INSTDIR\doc18.png"
  Delete "$INSTDIR\doc19.png"
  Delete "$INSTDIR\doc1.png"
  Delete "$INSTDIR\doc2.png"
  Delete "$INSTDIR\doc3.png"
  Delete "$INSTDIR\doc4.png"
  Delete "$INSTDIR\doc5.png"
  Delete "$INSTDIR\doc6.png"
  Delete "$INSTDIR\doc7.png"
  Delete "$INSTDIR\doc8.png"
  Delete "$INSTDIR\doc9.png"
  Delete "$INSTDIR\doc6bis.png"
  Delete "$INSTDIR\doc20.png"
  Delete "$INSTDIR\doc21.png"
  Delete "$INSTDIR\doc22.png"
  Delete "$INSTDIR\doc10hu.png"
  Delete "$INSTDIR\doc11hu.png"
  Delete "$INSTDIR\doc12hu.png"
  Delete "$INSTDIR\doc13hu.png"
  Delete "$INSTDIR\doc14hu.png"
  Delete "$INSTDIR\doc15hu.png"
  Delete "$INSTDIR\doc16hu.png"
  Delete "$INSTDIR\doc17hu.png"
  Delete "$INSTDIR\doc1hu.png"
  Delete "$INSTDIR\doc20hu.png"
  Delete "$INSTDIR\doc21hu.png"
  Delete "$INSTDIR\doc22hu.png"
  Delete "$INSTDIR\doc2hu.png"
  Delete "$INSTDIR\doc3hu.png"
  Delete "$INSTDIR\doc4hu.png"
  Delete "$INSTDIR\doc5hu.png"
  Delete "$INSTDIR\doc6ahu.png"
  Delete "$INSTDIR\doc6hu.png"
  Delete "$INSTDIR\doc7hu.png"
  Delete "$INSTDIR\doc8hu.png"
  Delete "$INSTDIR\doc9hu.png"
  Delete "$INSTDIR\usermanual_en.html"
  Delete "$INSTDIR\usermanual_fr.html"
  Delete "$INSTDIR\usermanual_hu.html"
  Delete "$INSTDIR\usermanual_ru.html"
  Delete "$INSTDIR\latexhelp.html"
  Delete "$INSTDIR\mingwm10.dll"
  Delete "$INSTDIR\libgcc_s_dw2-1.dll"
  Delete "$INSTDIR\QtCore4.dll"
  Delete "$INSTDIR\QtGui4.dll"
  Delete "$INSTDIR\QtNetwork4.dll"
  Delete "$INSTDIR\QtXml4.dll"
  Delete "$INSTDIR\QtWebKit4.dll"
  Delete "$INSTDIR\QtXmlPatterns4.dll"
  Delete "$INSTDIR\phonon4.dll"
  Delete "$INSTDIR\libfreetype-6.dll"
  Delete "$INSTDIR\libgcc_s_sjlj-1.dll"
  Delete "$INSTDIR\libjpeg-8.dll"
  Delete "$INSTDIR\libpng14-14.dll"
  Delete "$INSTDIR\libpoppler-6.dll"
  Delete "$INSTDIR\libstdc++-6.dll"
  Delete "$INSTDIR\zlib1.dll"
  Delete "$INSTDIR\texmaker.exe"
  Delete "$INSTDIR\texmaker.ico"
  Delete "$INSTDIR\qt_cs.qm"
  Delete "$INSTDIR\qt_de.qm"
  Delete "$INSTDIR\qt_es.qm"
  Delete "$INSTDIR\qt_fr.qm"
  Delete "$INSTDIR\qt_pt.qm"
  Delete "$INSTDIR\qt_ru.qm"
  Delete "$INSTDIR\qt_zh_CN.qm"
  Delete "$INSTDIR\qt_zh_TW.qm"
  Delete "$INSTDIR\qt_fa.qm"
  Delete "$INSTDIR\qt_pl.qm"
  Delete "$INSTDIR\qt_nl.qm"
  Delete "$INSTDIR\texmaker_de.qm"
  Delete "$INSTDIR\texmaker_es.qm"
  Delete "$INSTDIR\texmaker_fa.qm"
  Delete "$INSTDIR\texmaker_fr.qm"
  Delete "$INSTDIR\texmaker_gl.qm"
  Delete "$INSTDIR\texmaker_it.qm"
  Delete "$INSTDIR\texmaker_pt_BR.qm"
  Delete "$INSTDIR\texmaker_ru.qm"
  Delete "$INSTDIR\texmaker_zh_CN.qm"
  Delete "$INSTDIR\texmaker_zh_TW.qm"
  Delete "$INSTDIR\texmaker_cs.qm"
  Delete "$INSTDIR\texmaker_nl.qm"
  Delete "$INSTDIR\texmaker_pl.qm"
  Delete "$INSTDIR\texmaker_hu.qm"
  Delete "$INSTDIR\texmaker_sr.qm"
  Delete "$INSTDIR\texmaker_el.qm"
  Delete "$INSTDIR\nl_NL.aff"
  Delete "$INSTDIR\nl_NL.dic"
  Delete "$INSTDIR\de_DE.aff"
  Delete "$INSTDIR\de_DE.dic"
  Delete "$INSTDIR\en_GB.aff"
  Delete "$INSTDIR\en_GB.dic"
  Delete "$INSTDIR\es_ES.aff"
  Delete "$INSTDIR\es_ES.dic"
  Delete "$INSTDIR\fr_FR.aff"
  Delete "$INSTDIR\fr_FR.dic"
  Delete "$INSTDIR\it_IT.aff"
  Delete "$INSTDIR\it_IT.dic"
  Delete "$INSTDIR\hu_HU.aff"
  Delete "$INSTDIR\hu_HU.dic"
  Delete "$INSTDIR\uninstall.exe"
  RmDir "$INSTDIR"
SectionEnd
