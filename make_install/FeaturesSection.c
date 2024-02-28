
#include <stdlib.h>
/*--------------------------------------------------------------------*\
Pro/Toolkit includes
\*--------------------------------------------------------------------*/
#include <ProMdl.h>
#include <ProWstring.h>
#include <ProAsmcomp.h>
#include <ProSolid.h>
#include <ProCsys.h>
#include <ProModelitem.h>
#include <ProUtil.h>

/*--------------------------------------------------------------------*\
Application includes
\*--------------------------------------------------------------------*/
#include "InputFile.h"
#include "FeaturesSection.h"
#include "CommonUtil.h"
#include <ProFeatType.h>
#include <ProWindows.h>


// TOPアセンブリ配下へのアクセス中に使用されるProAppData
typedef struct {
    char cValue[INPUTFILE_MAXLINE];     // in  検索対象名
    int iFindCnt;                       // out 検索対象のレジューム数
}FeaturesAppData;


ProError  searchFeaturesAction(ProFeature* p_feature, ProError status, ProAppData app_data);
ProError  searchFeaturesAction2(ProFeature* p_feature, ProError status, ProAppData app_data);

// inputfileの中身を格納するようの構造体一式
FeatureList* gstrFeatureList;

int iFeatureListRowsMax = 0;

/*=========================================================================*\
    Function:	setFeaturesSection
    Purpose:	Featureセクション
    ProMdl* top_asm             (in)    Feature対象
    InputFileFeature* strFeat   (in)    コンフィグレーションファイルの内容
    int iSectionMaxRows         (in)    処理すべきコンフィグレーションファイルの行数

\*=========================================================================*/
ProError setFeaturesSection(ProMdl top_asm , InputFileFeature* strFeat, int iSectionMaxRows, FeatureList* FeatureList) {

    ProError status = PRO_TK_NO_ERROR;
    ProMdlType mdl_type = PRO_MDL_UNUSED;
    FeaturesAppData	app_data;

    // カレントモデルのタイプを取得
    status = ProMdlTypeGet(top_asm, &mdl_type);

    if (mdl_type == PRO_MDL_ASSEMBLY)
    {
        for (int iInputMdlCnt = 0; iInputMdlCnt < iSectionMaxRows; iInputMdlCnt++) {

            // 初期値の設定
            strcpy(app_data.cValue, strFeat->cValue);
            app_data.iFindCnt = 0;

            // TOPアセンブリのすべてのコンポーネントを確認する
           ProSolidFeatVisit((ProSolid)top_asm,
                searchFeaturesAction,
                NULL,
                (ProAppData)&app_data);

            if (app_data.iFindCnt == 0) {

                if (iFeatureListRowsMax != 0) {
                    // Featureが見つからなかった場合, FeatureListにXの記載があるのかを確認
                    FeatureList = gstrFeatureList;
                    ProBool findFlag = PRO_B_FALSE;
                    for (int iList = 0; iList < iFeatureListRowsMax; iList++) {
                        wchar_t wValue[100];
                        ProStringToWstring(wValue, strFeat->cValue);
                        int resultNumber;
                        int resultX;

                        ProWstringCompare(wValue, FeatureList->wFeatureNumber, PRO_VALUE_UNUSED, &resultNumber);
                        ProWstringCompare(L"X", FeatureList->wTargetX, PRO_VALUE_UNUSED, &resultX);

                        if (resultNumber == 0 && resultX == 0) {
                            // FeatureNumberがあり、Target欄にXが記載されている
                            findFlag = PRO_B_TRUE;
                            break;
                        }
                        FeatureList++;
                    }

                    if (findFlag == PRO_B_FALSE) {
                        // Featureが見つからなかった
                        LOG_PRINT("NOK : %s : Feature not found", strFeat->cValue);

                    }
                    else {
                        // Featureが見つからなかった
                        LOG_PRINT("OK  : %s : Feature not found", strFeat->cValue);

                    }
                }
                else {
                    // Featureが見つからなかった
                    LOG_PRINT("NOK : %s : Feature not found", strFeat->cValue);

                }
            }
            strFeat++;
        }

        // ソリッド内のすべてのフィーチャを再生成する
        // これをやらないと正しく反映されない
        status = ProSolidRegenerate(ProMdlToSolid(top_asm), PRO_REGEN_FORCE_REGEN);
    }
    else {
        LOG_PRINT("NOK : All Features fail", strFeat->cValue);

    }
}

/*====================================================*\
  Function : searchFeaturesAction()
  Purpose  :TOPアセンブリ直下フィーチャを確認する
  ProFeature* p_feature (in) フューチャーハンドル
  ProError status       (in) ステータス
  ProAppData app_data   (in) ProSolidFeatVisitから受け取ったデータ
\*====================================================*/
ProError  searchFeaturesAction(ProFeature* p_feature, ProError status, ProAppData app_data)
{
    ProFeattype ftype;
    ProMdl p_mdl;
    ProMdlType p_type;
    ProFamilyMdlName    wName;
    ProCharPath         cName;
    FeaturesAppData	app_data2;

    // フィーチャ名を取得
    status = ProModelitemNameGet(p_feature, wName);
    ProWstringToString(cName, wName);

    // コンフィグレーションファイル内の値を比較し、部分一致した場合にレジュームする
    if (strstr(cName, ((FeaturesAppData*)app_data)->cValue) != NULL) {

        status = ProModelitemMdlGet(p_feature, &p_mdl);
        if (status == PRO_TK_NO_ERROR) {

            ((FeaturesAppData*)app_data)->iFindCnt = ((FeaturesAppData*)app_data)->iFindCnt + 1;

            status = FeatureResume((ProSolid*)&p_mdl, cName);
            if (status == PRO_TK_NO_ERROR) {
                LOG_PRINT("OK  : %s : %s resumd", ((FeaturesAppData*)app_data)->cValue, cName);
            }
            else {
                LOG_PRINT("NOK : %s : Failed to resume", ((FeaturesAppData*)app_data)->cValue);
            }
        }
    }else {

        // アドレスが変わったので、焼きまわし
        strcpy(app_data2.cValue, ((FeaturesAppData*)app_data)->cValue);
        app_data2.iFindCnt = ((FeaturesAppData*)app_data)->iFindCnt;

        /*********************************************
        * よく使用されるフィーチャタイプ(メモ)
        *     916  PRO_FEAT_CUT
        *     923  PRO_FEAT_DATUM
        *     926  PRO_FEAT_DATUM_AXIS
        *     979  PRO_FEAT_CSYS
         **********************************************/
         // フィーチャタイプを取得 (データム, CSYS, コンポーネント ...)
        status = ProFeatureTypeGet(p_feature, &ftype);
        if (status == PRO_TK_NO_ERROR && ftype == PRO_FEAT_COMPONENT)
        {
            status = ProAsmcompMdlGet((ProAsmcomp*)p_feature, &p_mdl);
            if (status == PRO_TK_NO_ERROR)
            {
                // タイプを取得
                status = ProMdlTypeGet(p_mdl, &p_type);
                if (status == PRO_TK_NO_ERROR && p_type == PRO_MDL_PART)
                {
                    // 再起呼び出しができなかったので、同じ内容のActionを呼び出す
                    status = ProSolidFeatVisit((ProSolid)p_mdl,
                        searchFeaturesAction2,
                        NULL,
                        (ProAppData)&app_data2);

                    // アドレスが変わったので、焼きまわし
                    strcpy(((FeaturesAppData*)app_data)->cValue, app_data2.cValue);
                    ((FeaturesAppData*)app_data)->iFindCnt = app_data2.iFindCnt;

                }
            }
        }
    }
    return PRO_TK_CONTINUE;
}

/*====================================================*\
  Function : searchFeaturesAction2()
  Purpose  :TOPアセンブリ直下フィーチャを確認する
  ProFeature* p_feature (in) フューチャーハンドル
  ProError status       (in) ステータス
  ProAppData app_data   (in) ProSolidFeatVisitから受け取ったデータ
  備考：
  再起呼び出しができなかったので、同じ内容のActionを呼び出す
\*====================================================*/
ProError  searchFeaturesAction2(ProFeature* p_feature, ProError status, ProAppData app_data)
{
    ProFeattype ftype;
    ProMdl p_mdl;
    ProMdlType p_type;
    ProFamilyMdlName    wName;
    ProCharPath         cName;

    // フィーチャ名を取得
    status = ProModelitemNameGet(p_feature, wName);
    ProWstringToString(cName, wName);

    // コンフィグレーションファイル内の値を比較し、部分一致した場合にレジュームする
    if (strstr(cName, ((FeaturesAppData*)app_data)->cValue) != NULL) {

        status = ProModelitemMdlGet(p_feature, &p_mdl);
        if (status == PRO_TK_NO_ERROR) {

            // レジュームした回数をカウント
            ((FeaturesAppData*)app_data)->iFindCnt = ((FeaturesAppData*)app_data)->iFindCnt + 1;

            status = FeatureResume((ProSolid*)&p_mdl, cName);

            if (status == PRO_TK_NO_ERROR) {
                LOG_PRINT("OK  : %s : %s resumd", ((FeaturesAppData*)app_data)->cValue, cName);
            }
            else {
                LOG_PRINT("NOK : %s : Failed to resume", ((FeaturesAppData*)app_data)->cValue);
            }
        }
    }
    else {
        /*********************************************
        * よく使用されるフィーチャタイプ(メモ)
        *     916  PRO_FEAT_CUT
        *     923  PRO_FEAT_DATUM
        *     926  PRO_FEAT_DATUM_AXIS
        *     979  PRO_FEAT_CSYS
         **********************************************/
         // フィーチャタイプを取得 (データム, CSYS, コンポーネント ...)
        status = ProFeatureTypeGet(p_feature, &ftype);
        if (status == PRO_TK_NO_ERROR && ftype == PRO_FEAT_COMPONENT)
        {
            status = ProAsmcompMdlGet((ProAsmcomp*)p_feature, &p_mdl);
            if (status == PRO_TK_NO_ERROR)
            {
                // タイプを取得
                status = ProMdlTypeGet(p_mdl, &p_type);
                if (status == PRO_TK_NO_ERROR && p_type == PRO_MDL_PART)
                {

                    // TOPアセンブリのすべてのコンポーネントを再帰的に訪問し、サブアセンブリの数を数える
                    status = ProSolidFeatVisit((ProSolid)p_mdl,
                        searchFeaturesAction,
                        NULL,
                        (ProAppData)&app_data);
                }
            }
        }

    }
    return PRO_TK_CONTINUE;
}

/*====================================================================*\
    FUNCTION :	FeatureResume()
    PURPOSE  :	Command "Resume"
    ProSolid* p_solid               (in) 対象のソリッド(?)
    ProCharPath cTargetFeatureName  (in) 対象のフィーチャ名
    備考：
    PTCのサンプルソースを転記
    対象のソリッドと同階層のフィーチャがすべてレジュームされてしまったので、
    対象フィーチャ名と同じフィーチャ以外をレジュームしないように変更
\*====================================================================*/
ProError FeatureResume(ProSolid* p_solid, ProCharPath cTargetFeatureName)
{
    ProError	    status;
    int* p_feat_id_array;
    ProFeatStatus* p_status_array;
    int		    n_features;
    int		    n_suppressed;
    int		    i;
    ProFeature	    feature;
    ProBoolean	    is_incomplete;
    ProFeatureResumeOptions* resume_options = 0;
    ProPath         wTargetFeatureName;
    ProPath         wFeatureName;
    int result;

    // IDとステータス配列を割り当て
    status = ProArrayAlloc(0, sizeof(int), 1, (ProArray*)&p_feat_id_array);
    status = ProArrayAlloc(0, sizeof(ProFeatStatus), 1, (ProArray*)&p_status_array);

    if (status != PRO_TK_NO_ERROR) {
        return PRO_TK_GENERAL_ERROR;

    }

    // 指定されたソリッドの機能とそのステータスのリストを取得
    status = ProSolidFeatstatusGet(*p_solid, &p_feat_id_array, &p_status_array, &n_features);

    if (status != PRO_TK_NO_ERROR) {
        return PRO_TK_GENERAL_ERROR;

    }

    for (i = n_features - 1, n_suppressed = 0; i >= 0; i--)
    {
        // 機能のハンドルを取得
        status = ProFeatureInit(*p_solid, p_feat_id_array[i], &feature);

        // 該当フィーチャ以外は処理をしない
        status = ProModelitemNameGet(&feature, wFeatureName);
        ProStringToWstring(wTargetFeatureName, cTargetFeatureName);
        ProWstringCompare(wFeatureName, wTargetFeatureName, PRO_VALUE_UNUSED, &result);

        // 機能は完了していますか？
        is_incomplete = PRO_B_FALSE;
        status = ProFeatureIsIncomplete(&feature, &is_incomplete);

        // 配列から不完全な機能を削除
        if (is_incomplete == PRO_B_TRUE ||
            p_status_array[i] != PRO_FEAT_SUPPRESSED ||
            result != 0)
        {
            status = ProArrayObjectRemove((ProArray*)&p_feat_id_array, i, 1);
        }
        else {
            n_suppressed++;
        }
    }

    // レジュームの実施
    status = ProArrayAlloc(1, sizeof(ProFeatureResumeOptions), 1, (ProArray*)&resume_options);

    resume_options[0] = PRO_FEAT_RESUME_INCLUDE_PARENTS;

    status = ProFeatureWithoptionsResume(*p_solid, p_feat_id_array, resume_options, PRO_REGEN_NO_FLAGS);

    // 画面リフレッシュ
    status = ProWindowRepaint(-1);

    status = ProTreetoolRefresh((ProMdl)*p_solid);

    /*-----------------------------------------------------------------*\
        メモリの開放
    \*-----------------------------------------------------------------*/
    status = ProArrayFree((ProArray*)&resume_options);
    status = ProArrayFree((ProArray*)&p_feat_id_array);
    status = ProArrayFree((ProArray*)&p_status_array);

    if (status != PRO_TK_NO_ERROR){
        return PRO_TK_GENERAL_ERROR;
    }

    return PRO_TK_NO_ERROR;
}

/*====================================================================*\
FUNCTION : loadFeatureList
PURPOSE  : FeatureListをロードし、各構造体へ値を格納する
\*====================================================================*/
ProError  loadFeatureList(ProCharPath strFileName)
{
    ProError status;
    FILE* fp;
    char str[MAX_WORDS_IN_CELL*4];
    errno_t error;
   
    int iFeatureListRowsCount = 0;
    iFeatureListRowsMax = 0;

    error = fopen_s(&fp, strFileName, "r");

    if (error != 0 || fp == 0) {
        // ファイルオープンの失敗は checkInitial()で確認済みだが、
        // コンフィグレーションファイルの取得に失敗した場合は以降の処理をしない
        return;
    }


    /***********************************************
     inputFile の 各セクション/区切りの行数をカウントする
    *************************************************/
    while (fgets(str, sizeof(str), fp) != NULL) {
        if (str[0] != '\n') {
            iFeatureListRowsMax++;
        }
    }

    // ファイル位置を先頭に戻す
    fseek(fp, 0, SEEK_SET);

    /***********************************************
     inputFile の値を格納する変数のメモリ確保
    *************************************************/
    // *PARAMETERSのFeature部
    gstrFeatureList = (FeatureList*)calloc((iFeatureListRowsMax), sizeof(FeatureList));
    if (!gstrFeatureList) {
        // メモリ不足
        LOG_PRINT("NOK : Not enough memory");
        return PRO_TK_GENERAL_ERROR;
    }

    /***********************************************
     inputFile の 値取得
    *************************************************/
    while (fgets(str, sizeof(str), fp) != NULL) {
        // 改行のみは処理しない
        if (str[0] != '\n') {
            TRAIL_PRINT("%s(%d) : str = %s", __func__, __LINE__, str);

            getFeatureList(str, iFeatureListRowsCount, gstrFeatureList);
            iFeatureListRowsCount++;
        }

    }

    //ファイルを閉じる
    fclose(fp);

    return PRO_TK_NO_ERROR;

}

/*====================================================================*\
FUNCTION : getInputFile
PURPOSE  : inputFileをロードし、各構造体へ値を格納する
    char        str[MAX_WORDS_IN_CELL]  (in) inputFileから読み取った1行
    int         iRow                    (in) 読み取った行
    InputFile*  wInputFileArray         (out)格納する構造体
\*====================================================================*/
ProError  getFeatureList(char str[MAX_WORDS_IN_CELL], int iRow, FeatureList* FeatureList)
{
    char* cpValue;           // 対象部品名

    if (iRow > 0) {
        for (int iLoop = 0; iLoop < iRow; iLoop++) {
            *FeatureList++;
        }
    }

    // COMMA_SEPARATION 区切りで値を取得する
    cpValue = strtok(str, COMMA_SEPARATION);
    ProStringToWstring(FeatureList->wFeatureNumber, c_trim(cpValue));

    cpValue = strtok(NULL, COMMA_SEPARATION);
    ProStringToWstring(FeatureList->wFeatureName, c_trim(cpValue));

    cpValue = strtok(NULL, COMMA_SEPARATION);
    ProStringToWstring(FeatureList->wTargetX, c_trim(cpValue));

    cpValue = strtok(NULL, COMMA_SEPARATION);
    ProStringToWstring(FeatureList->wMemo, c_trim(cpValue));

}
