/*
* CommonUtil.h
*/
#pragma once

#include "InputFile.h"
/*-----------------------------------------------------------------*\
    �}�N����`(Macro definition)
\*-----------------------------------------------------------------*/
#define TOP_ASSY 0
#define SUB_ASSY 1

// ���ϐ�����擾����l
// GUI
#define ENV_IN_OUT_DIR          					"IN_OUT_DIR"
#define ENV_USER_START_APP_TIME 					"USER_START_APP_TIME"
#define ENV_CONFIGRATION_FILE   					"CONFIGRATION_FILE"
#define ENV_FRAME_TEMPLATE      					"FRAME_TEMPLATE"
#define ENV_E_Mail              					"E_MAIL"
#define ENV_WINDCHILL_CONTEXT             			"WINDCHILL_CONTEXT"
#define ENV_WINDCHILL_FOLDER             			"WINDCHILL_FOLDER"

// �N���o�b�`
#define ENV_FRAME_FEATURE_LOG_OK_LIST               "FRAME_FEATURE_LOG_OK_LIST"
#define ENV_SERIAL_NUMBER                           "SERIAL_NUMBER"
#define ENV_WINDCHILL_SERVER      					"WINDCHILL_SERVER"
#define ENV_WINDCHILL_URL      					    "WINDCHILL_URL"
#define ENV_WINDCHILL_WORKSPACE    					"WINDCHILL_WORKSPACE"
#define ENV_WINDCHILL_USERID      					"PROWT_USER"
#define ENV_WINDCHILL_PASSWORD    					"PROWT_PASSWD"
#define ENV_LOG_FILE_PATH                           "LOG_FILE_PATH"


/*-----------------------------------------------------------------*\
  �^��`(Type definition)
\*-----------------------------------------------------------------*/
// TOP�A�Z���u���z���ւ̃A�N�Z�X���Ɏg�p�����ProAppData
typedef struct {
    ProPath		name;       // in  �����Ώۖ�
    ProMdlType p_type;      // in  �����Ώۂ̃^�C�v
    int		iLoop;          // out ���[�v��(�\�����i�̒u���ł̂ݎg�p)
}TopAssyAppData;

// CSYS�ւ̃A�N�Z�X���Ɏg�p�����ProAppData
typedef struct {
    ProMdl		    model;      // in  �����Ώ�
    ProName		    csys_name;  // in  �����Ώۖ�
    ProCsys		    p_csys;     // out ��������
}UserCsysAppData;

// CSYS�ւ̃A�N�Z�X���Ɏg�p�����ProAppData
typedef struct {
    ProMdl		    model;          // in  �����Ώ�
    ProName		    csys_name;      // in  �����Ώۖ�
    int             iSectionType;   // in   �����Z�N�V����(searchTopAssyVOnlyAction�݂̂Ŏg�p)
    ProCsys		    p_csys;         // out ��������
    ProMdl		    topmodelTemp;   // out ��������:p_csys�ɑ΂���Top�A�Z���u��
}UserVCsysAppData;


// �t�B�[�`�� �������Ɏg�p����ProAppData
typedef struct {
    ProName		    name;		// in  �����Ώۖ�
    ProFeature		feature;    // out ��������
    int				iFindCnt;   // out ������	
}DatumAppData;

typedef struct
{
    ProLine defaultWorkspace;       // �����Őݒ肳��Ă��郏�[�N�X�y�[�X
    ProLine defaultContext;         // �����Őݒ肳��Ă���R���e�L�X�g
    ProLine defaultFolder;          // �����Őݒ肳��Ă���t�H���_
    ProLine settingIniWorkspace;    // Setting�̃��[�N�X�y�[�X
    ProLine settingIniContext;      // Setting�̃R���e�L�X�g
    ProLine settingIniFolder;       // Setting�̃t�H���_

    ProPath settingIniUrl   ;       // Setting��URL
    ProLine settingIniUserID;       // Setting�̃��[�UID
    ProLine settingIniPasswords;    // Setting��Passwords
    ProLine settingIniServer;       // Setting�̃T�[�o�[

}WindchillInfo;


/*-----------------------------------------------------------------*\
    �O���[�o���ϐ�(�V�X�e���X�R�[�v)
\*-----------------------------------------------------------------*/
extern ProCharPath gcLogFilePath;

/*-----------------------------------------------------------------*\
    �v���g�^�C�v�錾(Prototype declaration)
\*-----------------------------------------------------------------*/

char* c_trim(char* cpStr);
ProError   searchAssypathFromWindchill(ProPath objectname, int assytype, int loadtype, ProMdl* assy);
ProError CsysFindFilterAction(ProCsys this_csys, ProAppData	app_data);
ProError CsysFindAllFilterAction(ProCsys this_csys, ProAppData	app_data);
ProError CsysFindVisitAction(ProCsys this_csys, ProError filter_return, ProAppData app_data);
ProError UsrPointAddAction(ProGeomitem* p_handle, ProError status, ProAppData app_data);
ProError loadAssembly(InputFileRenameFile* strModulesRename, int iSectionMaxRows, ProMdl* mdlLoadAssy, wchar_t* wBeforeNamebak, wchar_t* wAfterNamebak);
void str_replace(const char* src, const char* target, const char* replace, char** result);
void LOG_PRINT(const char* log_txt, ...);
ProError  getFeatureIdAction(ProFeature* pFeature, ProError status, ProAppData app_data);
ProError  renameObject(ProPath wOldObject, ProPath wNewObject, ProMdlfileType fileType);
ProError Split(char* cFilenameBefore, wchar_t* wType, wchar_t* wFilenameAfter);

ProError getEnvCustom(ProCharPath env_value, ProPath env_output_value, int* iErrorCnt);
ProError getEnvCustomWithLog(ProCharPath env_value, ProPath env_output_value, int* iErrorCnt);

void TRAIL_PRINT(const char* log_txt, ...);
char* getProErrorMessage(ProError status);
ProError  toLowerCase(ProMdlName* wWords);
ProError setWindchillOnline(WindchillInfo windchillInfo);