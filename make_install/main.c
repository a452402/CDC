/*--------------------------------------------------------------------*\
C includes
\*--------------------------------------------------------------------*/
#include <stdio.h>
#include <windows.h>
#include <tchar.h>
#include <wchar.h>
#include <time.h>
#include <shlwapi.h>

/*--------------------------------------------------------------------*\
Pro/Toolkit includes
\*--------------------------------------------------------------------*/
#include <ProMode.h>
#include <ProMdl.h>
#include <ProUtil.h>
#include <ProUICmd.h>
#include <ProUI.h>
#include <ProMenuBar.h>
#include <ProToolkitDll.h>

/*--------------------------------------------------------------------*\
Application includes
\*--------------------------------------------------------------------*/
#include "main.h"
#include "mainCustom.h"

static wchar_t  MSGFIL[] =  L"UD_SAVP_msg.txt\0";
int giFlag = 0;
/*===========================================================================*\
Function : AssemblyChangeAccess


○ACCESS_REMOVE			ボタンは表示されません。
						また、含まれているメニューのすべてのメニューボタンにACCESS_REMOVEを返すアクセス関数がある場合、
						含まれているメニューもメニューから削除される可能性があります。
○ACCESS_INVISIBLE		ボタンは表示されません。
○ACCESS_UNAVAILABLE	ボタンは表示されていますが、使用できないため、選択できません。
○ACCESS_DISALLOW		ボタンは使用可能として表示されますが、選択するとコマンドは実行されません。
○ACCESS_AVAILABLE		ボタンは使用可能であり、ユーザーが選択できます。

\*===========================================================================*/
static uiCmdAccessState mainAccess(uiCmdAccessMode access_mode)
{
	return ACCESS_AVAILABLE;
}

/*===========================================================================*\
Function : test
\*===========================================================================*/
static uiCmdAccessState TestAccess(uiCmdAccessMode access_mode)
{
	return ACCESS_AVAILABLE;
}



/*====================================================================*\
FUNCTION : user_initialize()
PURPOSE  : Pro/DEVELOP standard initialize - define menu button
\*====================================================================*/
int user_initialize(
	int argc,
	char* argv[],
	char* version,
	char* build,
	wchar_t errbuf[80])
{
	// 繰り返し実行しないようにフラグで管理する
	if (giFlag == 0) {
		giFlag = 1;
		mainCustom();
	}

//	ProError status;
//	uiCmdCmdId	cmd_id;
//
//	ProUITranslationFilesEnable();
//
//	// メインメニュー
//	status = ProMenubarMenuAdd("ToolKitMenu", "ToolKitMenuLabel", "Info", PRO_B_TRUE, MSGFIL);
//	// メイン アクションの追加
//	status = ProCmdActionAdd("mainCustom", (uiCmdCmdActFn)mainCustom, uiProe2ndImmediate, mainAccess, PRO_B_TRUE, PRO_B_TRUE, &cmd_id);
//	status = ProMenubarmenuPushbuttonAdd("ToolKitMenu", "mainCustom", "mainCustomLabel", "mainCustomComment", "File.psh_rename", PRO_B_TRUE, cmd_id, MSGFIL);
//
//#if 0
//	// アクションの追加
//	status = ProCmdActionAdd("TEST", (uiCmdCmdActFn)TestFunction, uiProe2ndImmediate, TestAccess, PRO_B_TRUE, PRO_B_TRUE, &cmd_id);
//	status = ProMenubarmenuPushbuttonAdd("ToolKitMenu", "TEST", "TestLabel", "TestComment", "File.psh_rename", PRO_B_TRUE, cmd_id, MSGFIL);
//
//#endif // DEBUG
//


	return (0);
}

/*====================================================================*\
FUNCTION : user_terminate()
PURPOSE  : To handle any termination actions
\*====================================================================*/
void user_terminate()
{

}
