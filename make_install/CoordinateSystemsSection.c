/*--------------------------------------------------------------------*\
C includes
\*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*\
Pro/Toolkit includes
\*--------------------------------------------------------------------*/
#include <ProToolkitErrors.h>
#include <ProMdl.h>
#include <ProAssembly.h>
#include <ProArray.h>
#include <ProWindows.h>
#include <ProFamtable.h>
#include <ProFaminstance.h>
#include <ProSolid.h>
#include <ProDrawing.h>
#include <ProFeatType.h>
#include <ProWstring.h>

/*--------------------------------------------------------------------*\
Application includes
\*--------------------------------------------------------------------*/
#include "CommonUtil.h"
#include "InputFile.h"
#include "CoordinateSystemsSection.h"

// TOPアセンブリ配下へのアクセス中に使用されるProAppData
typedef struct {
	ProPath		name;       // in  検索対象名
	ProMdlType p_type;      // in  検索対象のタイプ
	int comp_id_p_arr;		// out 
}FeatureIDGetAppData;

ProError FeatureIDGetFilter(ProFeature* p_feature, ProAppData app_data);
ProError FeatureIDGetAction(ProFeature* p_feature, ProError status, ProAppData app_data);

/*====================================================================*\
FUNCTION : SetCoordinateSystemsSection
PURPOSE  : Csysセクション
ProMdl top_asm			(in)	対象
InputFileCsys* strCsys	(in)	コンフィグレーションファイルの内容
int iSectionMaxRows		(in)	処理すべきコンフィグレーションファイルの行数
\*====================================================================*/
ProError  SetCoordinateSystemsSection(ProMdl mdlTopAssy, InputFileCsys* strCsys, int iSectionMaxRows)
{
	ProError status;
	ProMdl mdlFamTable, mdlInstance;
	int i = 0;
	int* comp_id_p_arr = NULL;
	ProFamtable famtable;
	ProFaminstance famInstance;
	ProMdlName		w_model_name;
	
	wchar_t wInstanceParameter[INPUTFILE_MAXLINE]; // インスタンスパラメータ
	wchar_t wGenericParameter[INPUTFILE_MAXLINE];  // ジェネリックパラメータ


	for (int iInputMdlCnt = 0; iInputMdlCnt < iSectionMaxRows; iInputMdlCnt++) {

		if (strCsys->cInstanceParameter[0] == '0' 
			&& strCsys->cInstanceParameter[1] == '4'
			&& strCsys->cInstanceParameter[2] == '_') {
			// InstanceParameter が 04_ で始まる場合はジェネリックモデルを選択(処理しない)
			LOG_PRINT("OK  : %s : Ignored processing", strCsys->cInstanceParameter);
			strCsys++;
			continue;
		}

		// 値の初期化
		ProStringToWstring(wGenericParameter, strCsys->cGenericParameter);
		ProStringToWstring(wInstanceParameter, strCsys->cInstanceParameter);

		status = ProArrayAlloc(1, sizeof(int*), 1, (ProArray*)&comp_id_p_arr);
		status = ProMdlnameInit(wGenericParameter, PRO_MDLFILE_PART, &mdlFamTable);
		// ハンドルの初期化
		status = ProFamtableInit(mdlFamTable, &famtable);
		status = ProFaminstanceInit(wInstanceParameter, &famtable, &famInstance);
		status = ProFaminstanceRetrieve(&famInstance, &mdlInstance);

		if (status != PRO_TK_NO_ERROR) {
			// InstanceParameter 見つからない場合
			LOG_PRINT("NOK : %s : Instance model not found", strCsys->cInstanceParameter);
			strCsys++;
			continue;
		}
		status = ProMdlMdlnameGet(mdlInstance, w_model_name);

		status = ProMdlLoad(w_model_name, PRO_MDL_PART, PRO_B_FALSE, &mdlFamTable);
		// 内部のフィーチャIdを取得する
		FeatureIDGetAppData	app_data;
		ProWstringCopy(wGenericParameter, app_data.name, PRO_VALUE_UNUSED);
		app_data.p_type = PRO_MDL_PART;

		// TOPアセンブリのすべてのコンポーネントを再帰的に訪問し、内部フィーチャIDを取得する
		status = ProSolidFeatVisit((ProSolid)mdlTopAssy,
			FeatureIDGetAction,
			FeatureIDGetFilter,
			(ProAppData)&app_data);

		// 取得したフィーチャIDを指定し、ファミリーテーブルを変更する
		comp_id_p_arr[0] = app_data.comp_id_p_arr;
		status = ProAssemblyAutointerchange((ProAssembly)mdlTopAssy, comp_id_p_arr, mdlFamTable);

		if (status != PRO_TK_NO_ERROR) {
			// インスタンスモデルの置換に失敗
			LOG_PRINT("NOK : %s : Instance model replacement failed", strCsys->cInstanceParameter);
		}else {
			LOG_PRINT("OK  : %s", strCsys->cInstanceParameter);

		}
		strCsys++;
	}

	// 再生する
	status = ProSolidRegenerate((ProSolid)mdlTopAssy, PRO_REGEN_FORCE_REGEN);

	return status;
}

/*====================================================*\
  Function : FeatureIDGetFilter()
  Purpose  : コンポーネントのフィルタ機能
\*====================================================*/
ProError FeatureIDGetFilter(ProFeature* p_feature, ProAppData app_data)
{
	ProError    status;
	ProFeattype ftype;
	ProMdl p_mdl;
	ProMdlType p_type;

	// フィーチャタイプを取得 (データム, CSYS, コンポーネント ...)
	status = ProFeatureTypeGet(p_feature, &ftype);
	if (status == PRO_TK_NO_ERROR && ftype == PRO_FEAT_COMPONENT)
	{
		status = ProAsmcompMdlGet((ProAsmcomp*)p_feature, &p_mdl);
		if (status == PRO_TK_NO_ERROR)
		{
			// タイプを取得
			status = ProMdlTypeGet(p_mdl, &p_type);
			if (status == PRO_TK_NO_ERROR)
			{
				if (p_type == ((FeatureIDGetAppData*)app_data)->p_type)
				{
					return(PRO_TK_NO_ERROR);
				}
			}
		}
	}
	return PRO_TK_CONTINUE;
}


/*====================================================*\
  Function : FeatureIDGetAction()
  Purpose  : 引数で指定したFeatureのFeatureIdを取得する
\*====================================================*/
ProError  FeatureIDGetAction(ProFeature* p_feature, ProError status, ProAppData app_data)
{
	ProMdlfileType     mdltype;
	ProFamilyMdlName        w_name;
	int iResult;

	// サブアセンブリのタイプと名前を取得する
	status = ProAsmcompMdlMdlnameGet((ProAsmcomp*)p_feature, &mdltype, w_name);
	ProWstringCompare(w_name, ((FeatureIDGetAppData*)app_data)->name, PRO_VALUE_UNUSED, &iResult);

	if (iResult == 0) {
		// app_dataに値を格納する
		((FeatureIDGetAppData*)app_data)->comp_id_p_arr = p_feature->id;
	}
	return status;
}
