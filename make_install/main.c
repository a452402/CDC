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


��ACCESS_REMOVE			�{�^���͕\������܂���B
						�܂��A�܂܂�Ă��郁�j���[�̂��ׂẴ��j���[�{�^����ACCESS_REMOVE��Ԃ��A�N�Z�X�֐�������ꍇ�A
						�܂܂�Ă��郁�j���[�����j���[����폜�����\��������܂��B
��ACCESS_INVISIBLE		�{�^���͕\������܂���B
��ACCESS_UNAVAILABLE	�{�^���͕\������Ă��܂����A�g�p�ł��Ȃ����߁A�I���ł��܂���B
��ACCESS_DISALLOW		�{�^���͎g�p�\�Ƃ��ĕ\������܂����A�I������ƃR�}���h�͎��s����܂���B
��ACCESS_AVAILABLE		�{�^���͎g�p�\�ł���A���[�U�[���I���ł��܂��B

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
	// �J��Ԃ����s���Ȃ��悤�Ƀt���O�ŊǗ�����
	if (giFlag == 0) {
		giFlag = 1;
		mainCustom();
	}

//	ProError status;
//	uiCmdCmdId	cmd_id;
//
//	ProUITranslationFilesEnable();
//
//	// ���C�����j���[
//	status = ProMenubarMenuAdd("ToolKitMenu", "ToolKitMenuLabel", "Info", PRO_B_TRUE, MSGFIL);
//	// ���C�� �A�N�V�����̒ǉ�
//	status = ProCmdActionAdd("mainCustom", (uiCmdCmdActFn)mainCustom, uiProe2ndImmediate, mainAccess, PRO_B_TRUE, PRO_B_TRUE, &cmd_id);
//	status = ProMenubarmenuPushbuttonAdd("ToolKitMenu", "mainCustom", "mainCustomLabel", "mainCustomComment", "File.psh_rename", PRO_B_TRUE, cmd_id, MSGFIL);
//
//#if 0
//	// �A�N�V�����̒ǉ�
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
