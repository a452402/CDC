/*--------------------------------------------------------------------*\
C includes
\*--------------------------------------------------------------------*/
#include <stdlib.h>
#include <windows.h>
/*--------------------------------------------------------------------*\
Pro/Toolkit includes
\*--------------------------------------------------------------------*/
#include <ProMdl.h>
#include <ProDrawing.h>
#include <ProParameter.h>
#include <ProWstring.h>

/*--------------------------------------------------------------------*\
Application includes
\*--------------------------------------------------------------------*/
#include "ParametersSection.h"
#include "CommonUtil.h"
#include <ProSolid.h>

/*-----------------------------------------------------------------*\
    構造体
\*-----------------------------------------------------------------*/

/*-----------------------------------------------------------------*\
    プロトタイプ宣言
\*-----------------------------------------------------------------*/
ProError ParameterVisitAction(ProParameter* param, ProError err, ProAppData app_data);

/*====================================================================*\
FUNCTION : setParametersSection
PURPOSE  : パラメータの変更
ProMdl mdl                          (in) 変更するパラメータのTopアセンブリ
ProPath layoutName                  (in) 修正するレイアウトファイル名(拡張抜き)
InputFileParamFeature* strFeature   (in) 処理すべきコンフィグレーションファイル *PARAMETERS 系統 - Feature部
InputFileParameters* strParam       (in) 処理すべきコンフィグレーションファイル *PARAMETERS 系統 - Parameter部
int iFeatureSectionMaxRows          (in) 処理すべきコンフィグレーションファイルの行数 *PARAMETERS 系統 - Feature部
int iParameterSectionMaxRows        (in) 処理すべきコンフィグレーションファイルの行数 *PARAMETERS 系統 - Parameter部
\*====================================================================*/
ProError  setParametersSection(ProMdl mdlTopAssy, ProPath layoutName, InputFileParamFeature* strFeature, InputFileParameters* strParam, int iFeatureSectionMaxRows, int iParameterSectionMaxRows)
{
	ProError status;
    ProModelitem modelitem;
    ProMdl notebookMdl = NULL;
    ProPath wTopLayout; // レイアウトファイル名/変更対象(拡張子付き)
    ProCharPath cTopLayout;
    ProMdlfileType filetype = PRO_MDLFILE_UNUSED;
    ProMdlType type = PRO_MDL_UNUSED;


    /*--------------------------------------------------------------------*\
     パラメータの変更対象の確認
    \*--------------------------------------------------------------------*/
    if (iFeatureSectionMaxRows == 0) {
        // パラメータの変更対象がない場合はasmと同等のレイアウトファイル
        ProWstringCopy(layoutName, wTopLayout, PRO_VALUE_UNUSED);
        ProWstringConcatenate(L".lay", wTopLayout, PRO_VALUE_UNUSED);
    }
    else {
        // パラメータの変更対象がある場合は、変更対象を使用
        ProStringToWstring(wTopLayout, strFeature->cFeature);
    }

    LOG_PRINT("ATTENTION : Change the parameters of %w ", wTopLayout);

    // 該当ファイルの拡張子を確認
    ProWstringToString(cTopLayout, wTopLayout);
    if (strstr(cTopLayout, ".prt") != NULL) {
        filetype = PRO_MDLFILE_PART;
        type = PRO_MDL_PART;
    }
    else if (strstr(cTopLayout, ".lay") != NULL) {
        filetype = PRO_MDLFILE_NOTEBOOK;
        type = PRO_MDL_LAYOUT;
    }

    // ファイルがロード済みかを確認する
    ProMdlInit(wTopLayout, type, &notebookMdl);

    if (notebookMdl == NULL) {
        // ファイルをロードする
        if (searchAssypathFromWindchill(wTopLayout, SUB_ASSY, filetype, &notebookMdl) != PRO_TK_NO_ERROR) {
            LOG_PRINT("NOK : %s : Failed to setting parameter", strParam->cParameterName);
            return;
        }
    }
    else {
        // すでにロード済み
    }

	// レイアウトの変更
    status = ProMdlToModelitem(notebookMdl, &modelitem);

    for (int iLoop = 0; iLoop < iParameterSectionMaxRows; iLoop++) {
        status = ProParameterVisit(&modelitem, NULL, (ProParameterAction)ParameterVisitAction, (ProAppData)strParam);
        if (PRO_TK_NO_ERROR == status) {
            // レイアウトが見つからなかった
            LOG_PRINT("NOK : %s : Parameter not found", strParam->cParameterName);
        }
        strParam++;
    }

    // パラメータの適用
    status = ProSolidRegenerate((ProSolid)mdlTopAssy, PRO_REGEN_FORCE_REGEN);

    ProError PRO_TK_NO_ERROR;
}

/*====================================================================*\
    FUNCTION :	 ParameterVisitAction()
    PURPOSE  :   パラメータ一覧の取得
\*====================================================================*/
ProError ParameterVisitAction(ProParameter* param,
    ProError err,
    ProAppData app_data)
{
    ProError status;
    ProCharLine str_param_val;
    ProParamvalue param_val;
    ProParamvalueType param_type;
    ProCharName param_name;
    ProLine w_str_param_val;
    ProParamvalue newValue;

    // パラメータの名前取得
    ProWstringToString(param_name, param->id);

    // パラメータ名を含むパラメータを検索する
    if (strstr(param_name, ((InputFileParameters*)app_data)->cParameterName) == NULL) {
        return PRO_TK_NO_ERROR;
    }

    //パラメータ、パラメータタイプ取得
    status = ProParameterValueGet(param, &param_val);
    status = ProParamvalueTypeGet(&param_val, &param_type);

    newValue.type = param_type;

    if (param_type == PRO_PARAM_DOUBLE) {
        // 実数(少数)
        double dParameteValue = atof(((InputFileParameters*)app_data)->cParameteValue);
        newValue.value.d_val = dParameteValue;

    }else if (param_type == PRO_PARAM_STRING) {
        // 文字列
        ProLine wParameteValue;
        ProStringToWstring(wParameteValue, ((InputFileParameters*)app_data)->cParameteValue);
        ProWstringCopy(wParameteValue, newValue.value.s_val ,PRO_VALUE_UNUSED);

    }else if (param_type == PRO_PARAM_INTEGER || param_type == PRO_PARAM_NOTE_ID) {
        // 整数
        int iParameteValue = atoi(((InputFileParameters*)app_data)->cParameteValue);
        newValue.value.i_val = iParameteValue;

    }else if (param_type == PRO_PARAM_BOOLEAN) {
        // YESNO (boolean型は1/0が入力される)
        short sParameteValue = atoi(((InputFileParameters*)app_data)->cParameteValue);
        newValue.value.l_val = sParameteValue;
    }

    status = ProParameterValueWithUnitsSet(param, &newValue, NULL);


    if (status == PRO_TK_NO_ERROR) {
        LOG_PRINT("OK  : %s : %s", ((InputFileParameters*)app_data)->cParameterName , param_name);
    }else {
        LOG_PRINT("NOK : %s : Parameter change failed", ((InputFileParameters*)app_data)->cParameterName);

    }
    return PRO_TK_CONTINUE;
}
