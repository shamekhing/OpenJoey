; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=Warning
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "yanepack.h"
LastPage=0

ClassCount=5
Class1=CYanePackApp
Class2=CYanePackDlg

ResourceCount=4
Resource1=IDD_YANEPACK_DIALOG
Class3=CProgressDlg
Resource2=CG_IDD_PROGRESS
Class4=NameEditDlg
Resource3=IDD_DIALOG1
Class5=Warning
Resource4=IDD_DIALOG2

[CLS:CYanePackApp]
Type=0
BaseClass=CWinApp
HeaderFile=YanePack.h
ImplementationFile=YanePack.cpp
Filter=N
LastObject=CYanePackApp

[CLS:CYanePackDlg]
Type=0
BaseClass=CDialog
HeaderFile=YanePackDlg.h
ImplementationFile=YanePackDlg.cpp
LastObject=IDC_BUTTON4
Filter=D
VirtualFilter=dWC

[DLG:IDD_YANEPACK_DIALOG]
Type=1
Class=CYanePackDlg
ControlCount=13
Control1=IDOK,button,1342242817
Control2=IDC_BUTTON1,button,1342242816
Control3=IDC_BUTTON2,button,1342242816
Control4=IDC_BUTTON3,button,1342242816
Control5=IDC_BUTTON4,button,1342242816
Control6=IDC_BUTTON5,button,1342242816
Control7=IDC_STATIC,static,1342308352
Control8=IDC_STATIC,static,1342308352
Control9=IDC_STATIC,static,1342177283
Control10=IDC_STATIC,static,1342177283
Control11=IDC_STATIC,static,1342177287
Control12=IDC_BUTTON6,button,1342242816
Control13=IDC_STATIC,static,1342308352

[DLG:CG_IDD_PROGRESS]
Type=1
Class=CProgressDlg
ControlCount=4
Control1=IDCANCEL,button,1342242817
Control2=CG_IDC_PROGDLG_PROGRESS,msctls_progress32,1350565888
Control3=CG_IDC_PROGDLG_PERCENT,static,1342308352
Control4=CG_IDC_PROGDLG_STATUS,static,1342308352

[CLS:CProgressDlg]
Type=0
HeaderFile=ProgDlg.h
ImplementationFile=ProgDlg.cpp
BaseClass=CDialog
LastObject=CG_IDC_PROGDLG_PERCENT

[DLG:IDD_DIALOG1]
Type=1
Class=NameEditDlg
ControlCount=5
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_EDIT1,edit,1350631552
Control4=IDC_STATIC,static,1342308352
Control5=IDC_STATIC,static,1342308352

[CLS:NameEditDlg]
Type=0
HeaderFile=NameEditDlg.h
ImplementationFile=NameEditDlg.cpp
BaseClass=CDialog
Filter=D
VirtualFilter=dWC
LastObject=IDC_EDIT1

[DLG:IDD_DIALOG2]
Type=1
Class=Warning
ControlCount=4
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_STATIC,static,1342308352
Control4=IDC_STATIC,static,1342308352

[CLS:Warning]
Type=0
HeaderFile=Warning.h
ImplementationFile=Warning.cpp
BaseClass=CDialog
Filter=D
VirtualFilter=dWC
LastObject=IDOK

