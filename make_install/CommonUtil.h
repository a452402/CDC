/*
* CommonUtil.h
*/
#pragma once

#include "InputFile.h"
/*-----------------------------------------------------------------*\
    マクロ定義(Macro definition)
\*-----------------------------------------------------------------*/
#define TOP_ASSY 0
#define SUB_ASSY 1

// 環境変数から取得する値
// GUI
#define ENV_IN_OUT_DIR          					"IN_OUT_DIR"
#define ENV_USER_START_APP_TIME 					"USER_START_APP_TIME"
#define ENV_CONFIGRATION_FILE   					"CONFIGRATION_FILE"
#define ENV_FRAME_TEMPLATE      					"FRAME_TEMPLATE"
#define ENV_E_Mail              					"E_MAIL"
#define ENV_WINDCHILL_CONTEXT             			"WINDCHILL_CONTEXT"
#define ENV_WINDCHILL_FOLDER             			"WINDCHILL_FOLDER"

// 起動バッチ
#define ENV_FRAME_FEATURE_LOG_OK_LIST               "FRAME_FEATURE_LOG_OK_LIST"
#define ENV_SERIAL_NUMBER                           "SERIAL_NUMBER"
#define ENV_WINDCHILL_SERVER      					"WINDCHILL_SERVER"
#define ENV_WINDCHILL_URL      					    "WINDCHILL_URL"
#define ENV_WINDCHILL_WORKSPACE    					"WINDCHILL_WORKSPACE"
#define ENV_WINDCHILL_USERID      					"PROWT_USER"
#define ENV_WINDCHILL_PASSWORD    					"PROWT_PASSWD"
#define ENV_LOG_FILE_PATH                           "LOG_FILE_PATH"


/*-----------------------------------------------------------------*\
  型定義(Type definition)
\*-----------------------------------------------------------------*/
// TOPアセンブリ配下へのアクセス中に使用されるProAppData
typedef struct {
    ProPath		name;       // in  検索対象名
    ProMdlType p_type;      // in  検索対象のタイプ
    int		iLoop;          // out ループ回数(構成部品の置換でのみ使用)
}TopAssyAppData;

// CSYSへのアクセス中に使用されるProAppData
typedef struct {
    ProMdl		    model;      // in  検索対象
    ProName		    csys_name;  // in  検索対象名
    ProCsys		    p_csys;     // out 検索結果
}UserCsysAppData;

// CSYSへのアクセス中に使用されるProAppData
typedef struct {
    ProMdl		    model;          // in  検索対象
    ProName		    csys_name;      // in  検索対象名
    int             iSectionType;   // in   処理セクション(searchTopAssyVOnlyActionのみで使用)
    ProCsys		    p_csys;         // out 検索結果
    ProMdl		    topmodelTemp;   // out 検索結果:p_csysに対するTopアセンブリ
}UserVCsysAppData;


// フィーチャ 検索時に使用するProAppData
typedef struct {
    ProName		    name;		// in  検索対象名
    ProFeature		feature;    // out 検索結果
    int				iFindCnt;   // out 検索数	
}DatumAppData;

typedef struct
{
    ProLine defaultWorkspace;       // 既存で設定されているワークスペース
    ProLine defaultContext;         // 既存で設定されているコンテキスト
    ProLine defaultFolder;          // 既存で設定されているフォルダ
    ProLine settingIniWorkspace;    // Settingのワークスペース
    ProLine settingIniContext;      // Settingのコンテキスト
    ProLine settingIniFolder;       // Settingのフォルダ

    ProPath settingIniUrl   ;       // SettingのURL
    ProLine settingIniUserID;       // SettingのユーザID
    ProLine settingIniPasswords;    // SettingのPasswords
    ProLine settingIniServer;       // Settingのサーバー

}WindchillInfo;


/*-----------------------------------------------------------------*\
    グローバル変数(システムスコープ)
\*-----------------------------------------------------------------*/
extern ProCharPath gcLogFilePath;

/*-----------------------------------------------------------------*\
    プロトタイプ宣言(Prototype declaration)
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