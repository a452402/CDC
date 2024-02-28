/*--------------------------------------------------------------------*\
C includes
\*--------------------------------------------------------------------*/
#include <wchar.h>
#include <time.h>
#include <shlwapi.h>
#include <windows.h>
#include <stdio.h>
#include <tchar.h>

/*--------------------------------------------------------------------*\
Pro/Toolkit includes
\*--------------------------------------------------------------------*/
#include <ProMode.h>
#include <ProMdl.h>
#include <ProUtil.h>
#include <ProUICmd.h>
#include <ProAssembly.h>
#include <ProSolid.h>
#include <ProWTUtils.h>
#include <ProWstring.h>
#include <ProFaminstance.h>
#include <ProMfg.h>
#include <ProMessage.h>
#include <ProSelbuffer.h>
#include <ProObjects.h>
#include <ProFeatType.h>
#include <ProModelitem.h>
#include <ProCsys.h>
#include <ProWindows.h>

/*--------------------------------------------------------------------*\
Application includes
\*--------------------------------------------------------------------*/
#include "InputFile.h"
#include "CommonUtil.h"

/*-----------------------------------------------------------------*\
    グローバル変数
\*-----------------------------------------------------------------*/
// inputFile内の区切り一覧。配列の順番はinputFile.h内のenumに合わせること！
// 最長*VEHICLE_COORDINATE_SYSTEMSに合わせる
char gInputSection[ENUM_INPUT_SECTION_MAX][28] = {  
 INPUT_SIGNATURE                                    , // 0
 INPUT_IDENTITY                                     , // 1
 INPUT_PROTOM_SPEC_ID                               , // 2
 INPUT_OM_NO                                        , // 3
 INPUT_FO_NO                                        , // 4
 INPUT_SECTION_PARAMETERS_FEATURE                   , // 5  *PARAMETERSのFeature部
 INPUT_SECTION_PARAMETERS                           , // 6  *PARAMETERSのParameter部
 INPUT_SECTION_FEATURES                             , // 7
 INPUT_SECTION_VEHICLE_COORDINATE_SYSTEMS           , // 8
 INPUT_SECTION_PARTS                                , // 9
 INPUT_SECTION_HOLES                                , // 10
 INPUT_SECTION_HOLE_TABLE                           , // 11
 INPUT_SECTION_INNERLINER_RENAME                    , // 12  *INNERLINERのRename部
 INPUT_SECTION_INNERLINER                           , // 13  *INNERLINERのModules部
 INPUT_SECTION_INNERLINER_PARAMETERS_FEATURE        , // 14 *INNERLINER_PARAMETERSのFeature部
 INPUT_SECTION_INNERLINER_PARAMETERS                , // 15 *INNERLINER_PARAMETERSのParameter部
 INPUT_SECTION_INNERLINER_FEATURES                  , // 16
 INPUT_SECTION_INNERLINER_HOLES                     , // 17
 INPUT_SECTION_INNERLINER_HOLE_TABLE                , // 18
 INPUT_SECTION_MODULES_RENAME                       , // 19 *MODULESのRename部
 INPUT_SECTION_MODULES                              , // 20 *MODULESのModules部
 INPUT_SECTION_FRONT_AXLE_RENAME                    , // 21 *FRONT_AXLEのRename部
 INPUT_SECTION_FRONT_AXLE                           , // 22 *FRONT_AXLEのModules部
 INPUT_SECTION_FRONT_AXLE_PARAMS_FEATURE            , // 23 *PROP_SHAFT_PARAMSのFeature部
 INPUT_SECTION_FRONT_AXLE_PARAMS                    , // 24 *PROP_SHAFT_PARAMSのParameter部
 INPUT_SECTION_CAB_RENAME                           , // 25 *CABのRename部
 INPUT_SECTION_CAB                                  , // 26 *CABのModules部
 INPUT_SECTION_ENGINE_RENAME                        , // 27 *ENGINEのRename部
 INPUT_SECTION_ENGINE                               , // 28 *ENGINEのModules部
 INPUT_SECTION_GEARBOX_COORDINATE_SYSTEMS           , // 29
 INPUT_SECTION_GEARBOX_RENAME                       , // 30 *GEARBOXのRename部
 INPUT_SECTION_GEARBOX                              , // 31 *GEARBOXのModules部
 INPUT_SECTION_PROP_SHAFT_RENAME                    , // 32 *PROP_SHAFTのRename部
 INPUT_SECTION_PROP_SHAFT                           , // 33 *PROP_SHAFTのModules部
 INPUT_SECTION_PROP_SHAFT_PARAMS_FEATURE            , // 34 *FRONT_AXLE_PARAMSのFeature部
 INPUT_SECTION_PROP_SHAFT_PARAMS                      // 35 *FRONT_AXLE_PARAMSのParameter部
};

#define OK_WORDS                      65
wchar_t wOkWords[OK_WORDS] = {L"1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ-_"};

// inputFile内の区切り一覧の各行数を取得する。
int giSectionRows[ENUM_INPUT_SECTION_MAX];
// コンフィグレーションファイルから取得した {(IDENTITY: or PROTOM SPEC ID: or OM NO.: or FO NO.:)}
ProMdlName gwFramename;
// コンフィグレーションファイルから取得したSigunature
// ※ コンフィグレーション内にIDENTITY:が存在する場合は、(IDENTITY:)_(Signature)とするように改修したが、
// (IDENTITY:)のみとするようにとの要望からgwSignatureは現在使用していない
ProMdlName gwSignature;

// inputfileの中身を格納するようの構造体一式
InputFileParamFeature*  gstrParametersFeature;
InputFileParameters*    gstrParameters;
InputFileFeature*       gstrFeatures;
InputFileCsys*          gstrVehicleCoordinateSystems;
InputFileHole*          gstrHoles;
InputFileHoleTable*     gstrHoleTable;
InputFileRenameFile*    gstrInnerlinerRename;
InputFileModules*       gstrInnerliner;
InputFileParamFeature*  gstrInnerlinerParametersFeature;
InputFileParameters*    gstrInnerlinerParameters;
InputFileFeature*       gstrInnerlinerFeatures;
InputFileHole*          gstrInnerlinerHoles;
InputFileHoleTable*     gstrInnerlinerHoleTable;
InputFileRenameFile*    gstrModulesRename;
InputFileModules*       gstrModules;
InputFileRenameFile*    gstrFrontAxleRename;
InputFileModules*       gstrFrontAxle;
InputFileParamFeature*  gstrFrontAxleParamsFeature;
InputFileParameters*    gstrFrontAxleParams;
InputFileRenameFile*    gstrCabRename;
InputFileModules*       gstrCab;
InputFileRenameFile*    gstrEngineRename;
InputFileModules*       gstrEngine;
InputFileCsys*          gstrGearboxCoordinateSystems;
InputFileRenameFile*    gstrGearboxRename;
InputFileModules*       gstrGearbox;
InputFileRenameFile*    gstrPropShaftRename;
InputFileModules*       gstrPropShaft;
InputFileParamFeature*  gstrPropShaftParamsFeature;
InputFileParameters*    gstrPropShaftParams;

/*====================================================================*\
FUNCTION : loadInputFile
PURPOSE  : inputFileをロードし、各構造体へ値を格納する
\*====================================================================*/
ProError  loadInputFile(ProCharPath strFileName, int* iErrorCnt)
{
    ProError status;
    FILE* fp;
    char str[PRO_LINE_SIZE];
    errno_t error;
    int iSectionType = -1;
    int iSectionRow = 0;
    ProBoolean bSection = PRO_B_FALSE;
    int iOtherSectionFlag = 0;

    error = fopen_s(&fp, strFileName, "r");
 
    if (error != 0 || fp == 0) {
        // ファイルオープンの失敗は checkInitial()で確認済みだが、
        // コンフィグレーションファイルの取得に失敗した場合は以降の処理をしない
        return;
    }

    // グローバル変数の初期化
    memset(giSectionRows, 0, sizeof(giSectionRows));
    ProWstringCopy(L"", gwFramename, PRO_VALUE_UNUSED);
    ProWstringCopy(L"", gwSignature, PRO_VALUE_UNUSED);

    /***********************************************
     inputFile の 各セクション/区切りの行数をカウントする
    *************************************************/
    while (fgets(str, sizeof(str), fp) != NULL) {

        if (strstr(str, "//") != NULL && strstr(str, INPUT_SIGNATURE) != NULL) {
            // INPUT_SIGNATUREの場合
            giSectionRows[ENUM_INPUT_SIGNATURE]++;
        }
        else if (strstr(str, "//") != NULL && strstr(str, INPUT_IDENTITY) != NULL) {
            // INPUT_IDENTITYの場合
            giSectionRows[ENUM_INPUT_IDENTITY]++;
        }
        else if (strstr(str, "//") != NULL && strstr(str, INPUT_PROTOM_SPEC_ID) != NULL) {
            // INPUT_PROTOM_SPEC_IDの場合
            giSectionRows[ENUM_INPUT_PROTOM_SPEC_ID]++;
        }
        else if (strstr(str, "//") != NULL && strstr(str, INPUT_OM_NO) != NULL) {
            // INPUT_OM_NOの場合
            giSectionRows[ENUM_INPUT_OM_NO]++;
        
        }
        else if (strstr(str, "//") != NULL && strstr(str, INPUT_FO_NO) != NULL) {
            // INPUT_FO_NOの場合
            giSectionRows[ENUM_INPUT_FO_NO]++;

        }else if (strstr(str, "//") != NULL || strcmp(str, "\n") == NULL) {
            // コメント文, 空白行のみの場合、処理しない
            continue;
        }else {
            int iSectionFlag = 0;
            
            for (int iCnt = 1; iCnt < ENUM_INPUT_SECTION_MAX; iCnt++) {
                // セクション種類 を特定する
                if (strcmp(c_trim(str), gInputSection[iCnt]) == NULL) {
                    iSectionType = iCnt;
                    iSectionFlag = 1;
                    iOtherSectionFlag = 0;
                    break;
                }

            }

            if (iSectionFlag != 1 && str[0] == '*') {
                // セクション行以外のセクション(先頭が*で始まる行)が存在する場合はログに出力する
                LOG_PRINT("ATTENTION : Other section %s", str);
                // セクション行以外のセクションはセクション内すべてをカウントしないため、フラグで制御する
                iOtherSectionFlag = 1;
                // セクション行はカウントしないため、continue する
                continue;
            }

            if (iSectionFlag == 1 || iOtherSectionFlag == 1) {
                // セクション行はカウントしないため、continue する
                continue;
            }
        }

        if (iSectionType == ENUM_INPUT_SECTION_PARAMETERS_FEATURE
            || iSectionType == ENUM_INPUT_SECTION_INNERLINER_RENAME
            || iSectionType == ENUM_INPUT_SECTION_INNERLINER_PARAMETERS_FEATURE
            || iSectionType == ENUM_INPUT_SECTION_MODULES_RENAME
            || iSectionType == ENUM_INPUT_SECTION_FRONT_AXLE_RENAME
            || iSectionType == ENUM_INPUT_SECTION_FRONT_AXLE_PARAMS_FEATURE
            || iSectionType == ENUM_INPUT_SECTION_CAB_RENAME
            || iSectionType == ENUM_INPUT_SECTION_ENGINE_RENAME
            || iSectionType == ENUM_INPUT_SECTION_GEARBOX_RENAME
            || iSectionType == ENUM_INPUT_SECTION_PROP_SHAFT_RENAME
            || iSectionType == ENUM_INPUT_SECTION_PROP_SHAFT_PARAMS_FEATURE
            ) {
            // PARAMETER系統 , MODULES系統の場合　拡張子のある行数は別カウントする
            if ((strstr(str, ".asm") != NULL)
                || (strstr(str, ".prt") != NULL)
                || (strstr(str, ".lay") != NULL)) {
                giSectionRows[iSectionType]++;
            }
            else {
                giSectionRows[iSectionType+1]++;
            } 
        }else if (iSectionType > 0) {
            // 各セクションの行数をカウントする
            giSectionRows[iSectionType]++;
        }

    }

    // ファイル位置を先頭に戻す
    fseek(fp, 0, SEEK_SET);

    /***********************************************
     inputFile の値を格納する変数のメモリ確保
    *************************************************/
    status = mallcInputFile();
    if (status == PRO_TK_GENERAL_ERROR) {
        return status;
    }

    /***********************************************
     inputFile の 値取得
    *************************************************/
    while (fgets(str, sizeof(str), fp) != NULL) {

        TRAIL_PRINT("%s(%d) : inputFile = %s", __func__, __LINE__, str);

        if (strstr(str, "\t") != NULL) {
            char *strtemp;
            // タブを見つけたら半角スペースに変換する
            str_replace(str, "\t", INPUT_SEPARATION, &strtemp);
            strncpy(str, strtemp, sizeof(str));
        }else if (strstr(str, "//") != NULL && strstr(str, INPUT_SIGNATURE) != NULL) {
            // INPUT_SIGNATUREの場合
            iSectionType = ENUM_INPUT_SIGNATURE;
            bSection = PRO_B_TRUE;
        }
        else if (strstr(str, "//") != NULL && strstr(str, INPUT_IDENTITY) != NULL) {
            // INPUT_IDENTITYの場合
            iSectionType = ENUM_INPUT_IDENTITY;
            bSection = PRO_B_TRUE;
        }
        else if (strstr(str, "//") != NULL && strstr(str, INPUT_PROTOM_SPEC_ID) != NULL) {
            // INPUT_PROTOM_SPEC_IDの場合
            iSectionType = ENUM_INPUT_PROTOM_SPEC_ID;
            bSection = PRO_B_TRUE;
        }
        else if (strstr(str, "//") != NULL && strstr(str, INPUT_OM_NO) != NULL) {
            // INPUT_OM_NOの場合
            iSectionType = ENUM_INPUT_OM_NO;
            bSection = PRO_B_TRUE;
        }
        else if (strstr(str, "//") != NULL && strstr(str, INPUT_FO_NO) != NULL) {
            // INPUT_FO_NOの場合
            iSectionType = ENUM_INPUT_FO_NO;
            bSection = PRO_B_TRUE;
        }
        else if (strstr(str, "//") != NULL || strcmp(str, "\n") == NULL) {
            // コメント文, 空白行のみの場合、処理しない
            continue;
        } else {
            for (int iCnt = 1; iCnt < ENUM_INPUT_SECTION_MAX; iCnt++) {
                // セクション種類 を特定する
                if (strcmp(c_trim(str), gInputSection[iCnt]) == NULL) {
                    iSectionType = iCnt;
                    bSection = PRO_B_TRUE;
                    iSectionRow = 0;
                    break;
                }
            }

            if (bSection != PRO_B_TRUE && str[0] == '*') {
                // セクション行以外のセクション(先頭が*で始まる行)が存在する場合は処理しないように初期化する
                iSectionType = -1;
            }
        }

        // セクション種類 を取得した場合に限り処理をする
        if (iSectionType == ENUM_INPUT_SIGNATURE
            || iSectionType == ENUM_INPUT_IDENTITY
            || iSectionType == ENUM_INPUT_PROTOM_SPEC_ID 
            || iSectionType == ENUM_INPUT_OM_NO
            || iSectionType == ENUM_INPUT_FO_NO) {
            // SIGNATURE: / IDENTITY: / PROTOM SPEC ID: / OM NO.: / FO NO.:行は処理する
            getInputFile(iSectionType, str, iSectionRow);
            //  IDENTITY: は1行のみのため、初期化
            iSectionType = -1;
        } else if (iSectionType > 0) {
            if (bSection) {
                // IDENTITY: / PROTOM SPEC ID: / OM NO.: / FO NO.: 以外のセクションの場合,セクション行は処理しない
                bSection = PRO_B_FALSE;
                continue;
            }else {
                iSectionRow++;
                getInputFile(iSectionType, str, iSectionRow);
            }

        }
    }

    //ファイルを閉じる
    fclose(fp);
    
    // コンフィグレーションファイル内のIDENTITY: / PROTOM SPEC ID: / OM NO.:/ FO NO.:が取得できなかったため、エラー
    int iResult;
    ProWstringCompare(L"", gwFramename, PRO_VALUE_UNUSED, &iResult);
    if (iResult == 0) {
        LOG_PRINT("NOK : Could not get to IDENTITY: / PROTOM SPEC ID: / OM NO.: / FO NO.:in ConfigrationFile");
        *iErrorCnt = *iErrorCnt + 1;
    }
    else {

        if (giSectionRows[ENUM_INPUT_PROTOM_SPEC_ID] != 0) {
            // INPUT_PROTOM_SPEC_IDの場合、スペースで区切られた真ん中のIDをUD-SAVP の Top assembly name とする
            int iLength = 0;
            int iLen = 0;
            int iResult2;
            int flag = 0;
            ProMdlName wFramenameTemp;
            ProWstringCopy(L"", wFramenameTemp, PRO_VALUE_UNUSED);

            ProWstringLengthGet(gwFramename, &iLength);

            for (int iLoop = 0; iLoop < iLength; iLoop++) {
                if (gwFramename[iLoop] == ' ') {
                    flag++;
                    continue;
                }
                if (flag == 1) {
                    // スペースを1つ通過した文字列から取得する
                    wFramenameTemp[iLen] = gwFramename[iLoop];
                    iLen++;
                }
            }

            // 取得した文字を格納する
            ProWstringCopy(wFramenameTemp, gwFramename, PRO_VALUE_UNUSED);

        }

        /***********************************************
        * Topアセンブリ名および、各アセンブリやパーツ、レイアウト名は以下の命名規則に従う
      {(IDENTITY: or PROTOM SPEC ID: or OM NO.: or FO NO.:)}_(連番3桁)_(FIL/FA/CAB/ENG/GB/PS)_(1/2)  

          Creoオブジェクト名は拡張子を含まないで最大31文字のため、
      rename後が32文字以上の場合は31文字となるように(IDENTITY: or PROTOM SPEC ID: or OM NO.: or FO NO.:)の末尾を削る
      (IDENTITY: or PROTOM SPEC ID: or OM NO.: or FO NO.:) =< 21文字であることを確認する

          ※追記
          *FRONT_AXLE セクションにて組み付けるアセンブリ一式もリネーム対象
       {(IDENTITY: or PROTOM SPEC ID: or OM NO.: or FO NO.:)}_(連番3桁)_FA_(アセンブリ名8桁)
      (IDENTITY: or PROTOM SPEC ID: or OM NO.: or FO NO.:) =< 15文字であることを確認する
        *************************************************/
        int iLength = 0;
        int iOkWordsCheckFlag = 0;
        ProWstringLengthGet(gwFramename, &iLength);
        if (iLength > MAX_TOP_ASSY_LENGTH) {
            iLength = MAX_TOP_ASSY_LENGTH;
        }
        ProMdlName wFramename;
        // 初期化
        wmemset(wFramename,0,sizeof(wFramename));
        // 頭から MAX_TOP_ASSY_LENGTH 文字を取得
        for (int iLoop = 0; iLoop < iLength; iLoop++) {
            // 初期化
            iOkWordsCheckFlag = 0;
            // 禁止文字が使用されている場合はアンダースコアに置き換える
            for (int iOkWords = 0; iOkWords < OK_WORDS; iOkWords++) {
                if (wOkWords[iOkWords] == gwFramename[iLoop]) {
                    iOkWordsCheckFlag = 1;
                    break;
                }
            }
            if (iOkWordsCheckFlag == 1) {
                wFramename[iLoop] = gwFramename[iLoop];
            }
            else {
                ProWstringConcatenate(L"_", wFramename, PRO_VALUE_UNUSED);
            }
        }
        ProWstringCopy(wFramename, gwFramename, PRO_VALUE_UNUSED);

    }
    
    return PRO_TK_NO_ERROR;

}

/*====================================================================*\
FUNCTION : mallcInputFile
PURPOSE  : ipuutfile内の値を格納する構造体のメモリを確保する

\*====================================================================*/
ProError  mallcInputFile() {

    // *PARAMETERSのFeature部
    gstrParametersFeature = (InputFileParamFeature*)calloc(giSectionRows[ENUM_INPUT_SECTION_PARAMETERS_FEATURE], sizeof(InputFileParamFeature));
    if (!gstrParametersFeature) {
        // メモリ不足
        LOG_PRINT("NOK : Not enough memory");
        return PRO_TK_GENERAL_ERROR;
    }

    // *PARAMETERSのParameter部
    gstrParameters = (InputFileParameters*)calloc(giSectionRows[ENUM_INPUT_SECTION_PARAMETERS], sizeof(InputFileParameters));
    if (!gstrParameters) {
        // メモリ不足
        LOG_PRINT("NOK : Not enough memory");
        return PRO_TK_GENERAL_ERROR;
    }

    // *FEATURES
    gstrFeatures = (InputFileFeature*)calloc(giSectionRows[ENUM_INPUT_SECTION_FEATURES], sizeof(InputFileFeature));
    if (!gstrFeatures) {
        // メモリ不足
        LOG_PRINT("NOK : Not enough memory");
        return PRO_TK_GENERAL_ERROR;
    }

    // *VEHICLE_COORDINATE_SYSTEMS
    gstrVehicleCoordinateSystems = (InputFileCsys*)calloc(giSectionRows[ENUM_INPUT_SECTION_VEHICLE_COORDINATE_SYSTEMS], sizeof(InputFileCsys));
    if (!gstrVehicleCoordinateSystems) {
        // メモリ不足
        LOG_PRINT("NOK : Not enough memory");
        return PRO_TK_GENERAL_ERROR;
    }

    // *PARTS (未使用)
    // *HOLES
    gstrHoles = (InputFileHole*)calloc(giSectionRows[ENUM_INPUT_SECTION_HOLES], sizeof(InputFileHole));
    if (!gstrHoles) {
        // メモリ不足
        LOG_PRINT("NOK : Not enough memory");
        return PRO_TK_GENERAL_ERROR;
    }

    // *HOLE_TABLE
    gstrHoleTable = (InputFileHoleTable*)calloc(giSectionRows[ENUM_INPUT_SECTION_HOLE_TABLE], sizeof(InputFileHoleTable));
    if (!gstrHoleTable) {
        // メモリ不足
        LOG_PRINT("NOK : Not enough memory");
        return PRO_TK_GENERAL_ERROR;
    }

    // *INNERLINERのRename部
    gstrInnerlinerRename = (InputFileRenameFile*)calloc(giSectionRows[ENUM_INPUT_SECTION_INNERLINER_RENAME], sizeof(InputFileRenameFile));
    if (!gstrInnerlinerRename) {
        // メモリ不足
        LOG_PRINT("NOK : Not enough memory");
        return PRO_TK_GENERAL_ERROR;
    }

    // *INNERLINERのModules部
    gstrInnerliner = (InputFileModules*)calloc(giSectionRows[ENUM_INPUT_SECTION_INNERLINER], sizeof(InputFileModules));
    if (!gstrInnerliner) {
        // メモリ不足
        LOG_PRINT("NOK : Not enough memory");
        return PRO_TK_GENERAL_ERROR;
    }

    // *INNERLINER_PARAMETERSのFeature部
    gstrInnerlinerParametersFeature = (InputFileParamFeature*)calloc(giSectionRows[ENUM_INPUT_SECTION_INNERLINER_PARAMETERS_FEATURE], sizeof(InputFileParamFeature));
    if (!gstrInnerlinerParametersFeature) {
        // メモリ不足
        LOG_PRINT("NOK : Not enough memory");
        return PRO_TK_GENERAL_ERROR;
    }

    // *INNERLINER_PARAMETERSのParameter部
    gstrInnerlinerParameters = (InputFileParameters*)calloc(giSectionRows[ENUM_INPUT_SECTION_INNERLINER_PARAMETERS], sizeof(InputFileParameters));
    if (!gstrInnerlinerParameters) {
        // メモリ不足
        LOG_PRINT("NOK : Not enough memory");
        return PRO_TK_GENERAL_ERROR;
    }

    // *INNERLINER_FEATURES
    gstrInnerlinerFeatures = (InputFileFeature*)calloc(giSectionRows[ENUM_INPUT_SECTION_INNERLINER_FEATURES], sizeof(InputFileFeature));
    if (!gstrInnerlinerFeatures) {
        // メモリ不足
        LOG_PRINT("NOK : Not enough memory");
        return PRO_TK_GENERAL_ERROR;
    }

    // *INNERLINER_HOLES
    gstrInnerlinerHoles = (InputFileHole*)calloc(giSectionRows[ENUM_INPUT_SECTION_INNERLINER_HOLES], sizeof(InputFileHole));
    if (!gstrInnerlinerHoles) {
        // メモリ不足
        LOG_PRINT("NOK : Not enough memory");
        return PRO_TK_GENERAL_ERROR;
    }

    // *INNERLINER_HOLE_TABLE
    gstrInnerlinerHoleTable = (InputFileHoleTable*)calloc(giSectionRows[ENUM_INPUT_SECTION_INNERLINER_HOLE_TABLE], sizeof(InputFileHoleTable));
    if (!gstrInnerlinerHoleTable) {
        // メモリ不足
        LOG_PRINT("NOK : Not enough memory");
        return PRO_TK_GENERAL_ERROR;
    }

    // *MODULESのRename部
    gstrModulesRename = (InputFileRenameFile*)calloc(giSectionRows[ENUM_INPUT_SECTION_MODULES_RENAME], sizeof(InputFileRenameFile));
    if (!gstrModulesRename) {
        // メモリ不足
        LOG_PRINT("NOK : Not enough memory");
        return PRO_TK_GENERAL_ERROR;
    }

    // *MODULESのModules部
    gstrModules = (InputFileModules*)calloc(giSectionRows[ENUM_INPUT_SECTION_MODULES], sizeof(InputFileModules));
    if (!gstrModules) {
        // メモリ不足
        LOG_PRINT("NOK : Not enough memory");
        return PRO_TK_GENERAL_ERROR;
    }

    // *FRONT_AXLEのRename部
    gstrFrontAxleRename = (InputFileRenameFile*)calloc(giSectionRows[ENUM_INPUT_SECTION_FRONT_AXLE_RENAME], sizeof(InputFileRenameFile));
    if (!gstrFrontAxleRename) {
        // メモリ不足
        LOG_PRINT("NOK : Not enough memory");
        return PRO_TK_GENERAL_ERROR;
    }

    // *FRONT_AXLEのModules部
    gstrFrontAxle = (InputFileModules*)calloc(giSectionRows[ENUM_INPUT_SECTION_FRONT_AXLE], sizeof(InputFileModules));
    if (!gstrFrontAxle) {
        // メモリ不足
        LOG_PRINT("NOK : Not enough memory");
        return PRO_TK_GENERAL_ERROR;
    }

    // *PROP_SHAFT_PARAMSのFeature部
    gstrFrontAxleParamsFeature = (InputFileParamFeature*)calloc(giSectionRows[ENUM_INPUT_SECTION_FRONT_AXLE_PARAMS_FEATURE], sizeof(InputFileParamFeature));
    if (!gstrFrontAxleParamsFeature) {
        // メモリ不足
        LOG_PRINT("NOK : Not enough memory");
        return PRO_TK_GENERAL_ERROR;
    }

    // *PROP_SHAFT_PARAMSのParameter部
    gstrFrontAxleParams = (InputFileParameters*)calloc(giSectionRows[ENUM_INPUT_SECTION_FRONT_AXLE_PARAMS], sizeof(InputFileParameters));
    if (!gstrFrontAxleParams) {
        // メモリ不足
        LOG_PRINT("NOK : Not enough memory");
        return PRO_TK_GENERAL_ERROR;
    }

    // *CABのRename部
    gstrCabRename = (InputFileRenameFile*)calloc(giSectionRows[ENUM_INPUT_SECTION_CAB_RENAME], sizeof(InputFileRenameFile));
    if (!gstrCabRename) {
        // メモリ不足
        LOG_PRINT("NOK : Not enough memory");
        return PRO_TK_GENERAL_ERROR;
    }

    // *CABのModules部
    gstrCab = (InputFileModules*)calloc(giSectionRows[ENUM_INPUT_SECTION_CAB], sizeof(InputFileModules));
    if (!gstrCab) {
        // メモリ不足
        LOG_PRINT("NOK : Not enough memory");
        return PRO_TK_GENERAL_ERROR;
    }

    // *ENGINEのRename部
    gstrEngineRename = (InputFileRenameFile*)calloc(giSectionRows[ENUM_INPUT_SECTION_ENGINE_RENAME], sizeof(InputFileRenameFile));
    if (!gstrEngineRename) {
        // メモリ不足
        LOG_PRINT("NOK : Not enough memory");
        return PRO_TK_GENERAL_ERROR;
    }

    // *ENGINEのModules部
    gstrEngine = (InputFileModules*)calloc(giSectionRows[ENUM_INPUT_SECTION_ENGINE], sizeof(InputFileModules));
    if (!gstrEngine) {
        // メモリ不足
        LOG_PRINT("NOK : Not enough memory");
        return PRO_TK_GENERAL_ERROR;
    }

    // *GEARBOX_COORDINATE_SYSTEMS
    gstrGearboxCoordinateSystems = (InputFileCsys*)calloc(giSectionRows[ENUM_INPUT_SECTION_GEARBOX_COORDINATE_SYSTEMS], sizeof(InputFileCsys));
    if (!gstrGearboxCoordinateSystems) {
        // メモリ不足
        LOG_PRINT("NOK : Not enough memory");
        return PRO_TK_GENERAL_ERROR;
    }

    // *GEARBOXのRename部
    gstrGearboxRename = (InputFileRenameFile*)calloc(giSectionRows[ENUM_INPUT_SECTION_GEARBOX_RENAME], sizeof(InputFileRenameFile));
    if (!gstrGearboxRename) {
        // メモリ不足
        LOG_PRINT("NOK : Not enough memory");
        return PRO_TK_GENERAL_ERROR;
    }

    // *GEARBOXのModules部
    gstrGearbox = (InputFileModules*)calloc(giSectionRows[ENUM_INPUT_SECTION_GEARBOX], sizeof(InputFileModules));
    if (!gstrGearbox) {
        // メモリ不足
        LOG_PRINT("NOK : Not enough memory");
        return PRO_TK_GENERAL_ERROR;
    }

    // *PROP_SHAFTのRename部
    gstrPropShaftRename = (InputFileRenameFile*)calloc(giSectionRows[ENUM_INPUT_SECTION_PROP_SHAFT_RENAME], sizeof(InputFileRenameFile));
    if (!gstrPropShaftRename) {
        // メモリ不足
        LOG_PRINT("NOK : Not enough memory");
        return PRO_TK_GENERAL_ERROR;
    }

    // *PROP_SHAFTのModules部
    gstrPropShaft = (InputFileModules*)calloc(giSectionRows[ENUM_INPUT_SECTION_PROP_SHAFT], sizeof(InputFileModules));
    if (!gstrPropShaft) {
        // メモリ不足
        LOG_PRINT("NOK : Not enough memory");
        return PRO_TK_GENERAL_ERROR;
    }

    // *FRONT_AXLE_PARAMSのFeature部
    gstrPropShaftParamsFeature = (InputFileParamFeature*)calloc(giSectionRows[ENUM_INPUT_SECTION_PROP_SHAFT_PARAMS_FEATURE], sizeof(InputFileParamFeature));
    if (!gstrPropShaftParamsFeature) {
        // メモリ不足
        LOG_PRINT("NOK : Not enough memory");
        return PRO_TK_GENERAL_ERROR;
    }

    // *FRONT_AXLE_PARAMSのParameter部
    gstrPropShaftParams = (InputFileParameters*)calloc(giSectionRows[ENUM_INPUT_SECTION_PROP_SHAFT_PARAMS], sizeof(InputFileParameters));
    if (!gstrPropShaftParams) {
        // メモリ不足
        LOG_PRINT("NOK : Not enough memory");
        return PRO_TK_GENERAL_ERROR;
    }

}

/*====================================================================*\
FUNCTION : getInputFile
PURPOSE  : inputFileの中身を各構造体へ格納する
  int   iSectionType           (in)    セクション種類
  char  str[PRO_LINE_SIZE]     (in)    inputFile内の1行 
  int   iSectionRow            (in)    セクション内での現在の行数
備考:
Parameter系統/Modules系統のセクションがが2部構成となるため、
1部目の行数をiSectionRowでカウントし、
2部目は (iSectionRow)-(1部目の最大行) でカウントする。

\*====================================================================*/
ProError  getInputFile(int iSectionType, char str[INPUTFILE_MAXLINE], int iSectionRow){

    char cIdentity[PRO_MDLNAME_SIZE];
    char* p;
    // 初期化
    memset(cIdentity, 0, sizeof(cIdentity));

    switch (iSectionType){
    case ENUM_INPUT_SIGNATURE:
        /***********************************************
         SIGNATURE:
        *************************************************/
        // 文字抜き出し
        p = strstr(c_trim(str), INPUT_SIGNATURE);
        strncpy(cIdentity, p + sizeof(INPUT_SIGNATURE), INPUTFILE_MAXLINE);
        // wstring変換
        ProStringToWstring(gwSignature, cIdentity);
    break;
    case ENUM_INPUT_IDENTITY:
        /***********************************************
         IDENTITY:
        *************************************************/
        // 文字抜き出し
        p = strstr(c_trim(str), INPUT_IDENTITY);
        strncpy(cIdentity, p + sizeof(INPUT_IDENTITY), INPUTFILE_MAXLINE);
        // wstring変換
        ProStringToWstring(gwFramename, cIdentity);
        break;
    case ENUM_INPUT_PROTOM_SPEC_ID:
        /***********************************************
         PROTOM SPEC ID:
        *************************************************/
        // 文字抜き出し
        p = strstr(c_trim(str), INPUT_PROTOM_SPEC_ID);
        strncpy(cIdentity, p + sizeof(INPUT_PROTOM_SPEC_ID), INPUTFILE_MAXLINE);
        // wstring変換
        ProStringToWstring(gwFramename, cIdentity);
        break;
    case ENUM_INPUT_OM_NO:
        /***********************************************
         OM NO.:
        *************************************************/
        // 文字抜き出し
        p = strstr(c_trim(str), INPUT_OM_NO);
        strncpy(cIdentity, p + sizeof(INPUT_OM_NO), INPUTFILE_MAXLINE);
        // wstring変換
        ProStringToWstring(gwFramename, cIdentity);
        break;
    case ENUM_INPUT_FO_NO:
        /***********************************************
         FO NO.:
        *************************************************/
        // 文字抜き出し
        p = strstr(c_trim(str), INPUT_FO_NO);
        strncpy(cIdentity, p + sizeof(INPUT_FO_NO), INPUTFILE_MAXLINE);
        // wstring変換
        ProStringToWstring(gwFramename, cIdentity);
        break;

    case ENUM_INPUT_SECTION_PARAMETERS_FEATURE:
        /***********************************************
          *PARAMETERS
        *************************************************/
        getParameter(gstrParametersFeature,
            gstrParameters,
            str,
            iSectionRow,
            ENUM_INPUT_SECTION_PARAMETERS_FEATURE);

        break;
    case ENUM_INPUT_SECTION_FEATURES:
        /***********************************************
          *FEATURES
        *************************************************/
        getFeatures(gstrFeatures,
            str,
            iSectionRow,
            ENUM_INPUT_SECTION_FEATURES);

        break;
    case ENUM_INPUT_SECTION_VEHICLE_COORDINATE_SYSTEMS:
        /***********************************************
          *VEHICLE_COORDINATE_SYSTEMS
        *************************************************/
        getCoordinateSystems(gstrVehicleCoordinateSystems,
            str,
            iSectionRow,
            ENUM_INPUT_SECTION_VEHICLE_COORDINATE_SYSTEMS);

        break;
    case ENUM_INPUT_SECTION_PARTS:
        /***********************************************
          *PARTS 未使用のため、処理なし
        *************************************************/
        break;
    case ENUM_INPUT_SECTION_HOLES:
        /***********************************************
          *HOLES
        *************************************************/
        getHole(gstrHoles,
            str,
            iSectionRow,
            ENUM_INPUT_SECTION_HOLES);

        break;
    case ENUM_INPUT_SECTION_HOLE_TABLE:
        /***********************************************
          *HOLE_TABLE
        *************************************************/
        getHoleTable(gstrHoleTable,
            str,
            iSectionRow,
            ENUM_INPUT_SECTION_HOLE_TABLE);

        break;
    case ENUM_INPUT_SECTION_INNERLINER_RENAME:
        /***********************************************
          *INNERLINER
        *************************************************/
        getModules(gstrInnerlinerRename,
            gstrInnerliner,
            str,
            iSectionRow,
            ENUM_INPUT_SECTION_INNERLINER_RENAME);

        break;
    case ENUM_INPUT_SECTION_INNERLINER_PARAMETERS_FEATURE:
        /***********************************************
          *INNERLINER_PARAMETERS
        *************************************************/
        getParameter(gstrInnerlinerParametersFeature,
            gstrInnerlinerParameters,
            str,
            iSectionRow,
            ENUM_INPUT_SECTION_INNERLINER_PARAMETERS_FEATURE);

        break;
    case ENUM_INPUT_SECTION_INNERLINER_FEATURES:
        /***********************************************
          *INNERLINER_FEATURES
        *************************************************/
        getFeatures(gstrInnerlinerFeatures,
            str,
            iSectionRow,
            ENUM_INPUT_SECTION_INNERLINER_FEATURES);

        break;
    case ENUM_INPUT_SECTION_INNERLINER_HOLES:
        /***********************************************
          *INNERLINER_HOLES
        *************************************************/
        getHole(gstrInnerlinerHoles,
            str,
            iSectionRow,
            ENUM_INPUT_SECTION_INNERLINER_HOLES);

        break;
    case ENUM_INPUT_SECTION_INNERLINER_HOLE_TABLE:
        /***********************************************
          *INNERLINER_HOLE_TABLE
        *************************************************/
        getHoleTable(gstrInnerlinerHoleTable,
            str,
            iSectionRow,
            ENUM_INPUT_SECTION_INNERLINER_HOLE_TABLE);

        break;
    case ENUM_INPUT_SECTION_MODULES_RENAME:
        /***********************************************
          *MODULES
        *************************************************/
        getModules(gstrModulesRename,
            gstrModules,
            str,
            iSectionRow,
            ENUM_INPUT_SECTION_MODULES_RENAME);

        break;
    case ENUM_INPUT_SECTION_FRONT_AXLE_RENAME:
        /***********************************************
          *FRONT_AXLE
        *************************************************/
        getModules(gstrFrontAxleRename,
            gstrFrontAxle,
            str,
            iSectionRow,
            ENUM_INPUT_SECTION_FRONT_AXLE_RENAME);

        break;
    case ENUM_INPUT_SECTION_FRONT_AXLE_PARAMS_FEATURE:
        /***********************************************
          *FRONT_AXLE_PARAMS
        *************************************************/
        getParameter(gstrFrontAxleParamsFeature,
            gstrFrontAxleParams,
            str,
            iSectionRow,
            ENUM_INPUT_SECTION_FRONT_AXLE_PARAMS_FEATURE);

        break;
    case ENUM_INPUT_SECTION_CAB_RENAME:
        /***********************************************
          *CAB
        *************************************************/
        getModules(gstrCabRename,
            gstrCab,
            str,
            iSectionRow,
            ENUM_INPUT_SECTION_CAB_RENAME);

        break;
    case ENUM_INPUT_SECTION_ENGINE_RENAME:
        /***********************************************
          *ENGINE
        *************************************************/
        getModules(gstrEngineRename,
            gstrEngine,
            str,
            iSectionRow,
            ENUM_INPUT_SECTION_ENGINE_RENAME);

        break;
    case ENUM_INPUT_SECTION_GEARBOX_COORDINATE_SYSTEMS:
        /***********************************************
          *GEARBOX_COORDINATE_SYSTEMS
        *************************************************/
        getCoordinateSystems(gstrGearboxCoordinateSystems,
            str,
            iSectionRow,
            ENUM_INPUT_SECTION_GEARBOX_COORDINATE_SYSTEMS);

        break;
    case ENUM_INPUT_SECTION_GEARBOX_RENAME:
        /***********************************************
          *GEARBOX
        *************************************************/
        getModules(gstrGearboxRename,
            gstrGearbox,
            str,
            iSectionRow,
            ENUM_INPUT_SECTION_GEARBOX_RENAME);

        break;
    case ENUM_INPUT_SECTION_PROP_SHAFT_RENAME:
        /***********************************************
          *PROP_SHAFT
        *************************************************/
        getModules(gstrPropShaftRename,
            gstrPropShaft,
            str,
            iSectionRow,
            ENUM_INPUT_SECTION_PROP_SHAFT_RENAME);

        break;
    case ENUM_INPUT_SECTION_PROP_SHAFT_PARAMS_FEATURE:
        /***********************************************
          *PROP_SHAFT_PARAMS
        *************************************************/
        getParameter(gstrPropShaftParamsFeature,
            gstrPropShaftParams,
            str,
            iSectionRow,
            ENUM_INPUT_SECTION_PROP_SHAFT_PARAMS_FEATURE);

        break;

    }

}

/*====================================================================*\
FUNCTION : getParameter
PURPOSE  : PARAMETER系統の値をフィーチャ部とパラメータ部に分けて格納する
  InputFileParamFeature* strParamFeat,  (out) フィーチャ部の格納先
  InputFileParameters* strParam,        (out) パラメータ部の格納先
  char  str[PRO_LINE_SIZE]     (in)    inputFile内の1行
  int   iSectionRow            (in)    セクション内での現在の行数
  int   iEnum                  (in)    フィーチャ部のEnum値

\*====================================================================*/
ProError  getParameter(InputFileParamFeature* strParamFeat,
    InputFileParameters* strParam,
    char str[INPUTFILE_MAXLINE],
    int iSectionRow,
    int iEnum) {


    if (giSectionRows[iEnum] != 0
        && giSectionRows[iEnum] >= iSectionRow) {
        /***********************************************
         フィーチャ部の処理
        *************************************************/

        for (int iLoop = 1; iLoop < iSectionRow; iLoop++) {
            *strParamFeat++;
        }
        // 1行のみのためトリムしてコピーする
        strncpy(strParamFeat->cFeature, c_trim(str), sizeof(strParamFeat->cFeature));

    } else {
        /***********************************************
         パラメータ部の処理
        *************************************************/
        char* cpParameteValue;     // パラメータ値
        char* cpParameterName;     // パラメータ名

        int iSubSectionRow = iSectionRow - giSectionRows[iEnum];

        for (int iLoop = 1; iLoop < iSubSectionRow; iLoop++) {
            *strParam++;
        }

        // 半角スペース区切りで値を取得する
        cpParameteValue = strtok(str, INPUT_SEPARATION);
        strncpy(strParam->cParameteValue, cpParameteValue, sizeof(strParam->cParameteValue));

        // 第一引数に NULL を指定すると前回の str 値の続きから始める
        cpParameterName = strtok(NULL, INPUT_SEPARATION);
        strncpy(strParam->cParameterName, cpParameterName, sizeof(strParam->cParameterName));
    }
}

/*====================================================================*\
FUNCTION : getFeatures
PURPOSE  : FEATURES系統の値を格納する
  InputFileFeature* strFeat,  (out) 格納先
  char  str[PRO_LINE_SIZE]     (in)    inputFile内の1行
  int   iSectionRow            (in)    セクション内での現在の行数
  int   iEnum                  (in)    Enum値

\*====================================================================*/
ProError  getFeatures(InputFileFeature* strFeat,
    char str[INPUTFILE_MAXLINE],
    int iSectionRow,
    int iEnum) {

        char* cpValue;           // 対象部品名
        char* cpFeatureName;     // Feature名
        char cFeatureNameTemp[INPUTFILE_MAXLINE];   // Feature名(一時)


        for (int iLoop = 1; iLoop < iSectionRow; iLoop++) {
            *strFeat++;
        }

        // 半角スペース区切りで値を取得する
        cpValue = strtok(str, INPUT_SEPARATION);
        strncpy(strFeat->cValue, cpValue, sizeof(strFeat->cValue));

        // Feature系統のFeature名は半角スペースで区切らず、1つの値として処理する
        cpFeatureName = strtok(NULL, INPUT_SEPARATION);
        strncpy(cFeatureNameTemp, cpFeatureName, sizeof(cFeatureNameTemp));

        if (cpFeatureName) {
            while (cpFeatureName = strtok(NULL, INPUT_SEPARATION)) {
                strcat_s(cFeatureNameTemp, sizeof(cFeatureNameTemp), INPUT_SEPARATION);
                strcat_s(cFeatureNameTemp, sizeof(cFeatureNameTemp), cpFeatureName);
            }
        }
 
        strncpy(strFeat->cFeatureName, cFeatureNameTemp, sizeof(strFeat->cFeatureName));

}

/*====================================================================*\
FUNCTION : getCoordinateSystems
PURPOSE  : COORDINATE_SYSTEMS系統の値を格納する
  InputFileCsys* strCsys,  (out) 格納先
  char  str[PRO_LINE_SIZE]     (in)    inputFile内の1行
  int   iSectionRow            (in)    セクション内での現在の行数
  int   iEnum                  (in)    Enum値

\*====================================================================*/
ProError  getCoordinateSystems(InputFileCsys* strCsys,
    char str[INPUTFILE_MAXLINE],
    int iSectionRow,
    int iEnum) {

    char* cpInstanceParameter;    // インスタンスパラメータ
    char* cpGenericParameter;     // ジェネリックパラメータ

    for (int iLoop = 1; iLoop < iSectionRow; iLoop++) {
        *strCsys++;
    }

    // 半角スペース区切りで値を取得する
    cpInstanceParameter = strtok(str, INPUT_SEPARATION);
    strncpy(strCsys->cInstanceParameter, cpInstanceParameter, sizeof(strCsys->cInstanceParameter));

    // 第一引数に NULL を指定すると前回の str 値の続きから始める
    cpGenericParameter = strtok(NULL, INPUT_SEPARATION);
    strncpy(strCsys->cGenericParameter, cpGenericParameter, sizeof(strCsys->cGenericParameter));

}

/*====================================================================*\
FUNCTION : getHole
PURPOSE  : HOLES系統の値を格納する
  InputFileHole* strHole,      (out) 格納先
  char  str[PRO_LINE_SIZE]     (in)    inputFile内の1行
  int   iSectionRow            (in)    セクション内での現在の行数
  int   iEnum                  (in)    Enum値

\*====================================================================*/
ProError  getHole(InputFileHole* strHole,
    char str[INPUTFILE_MAXLINE],
    int iSectionRow,
    int iEnum) {

    char* cpHoleGroupName;    // 穴グループ名
    char* cpXCord;            // 基準位置からのX距離
    char* cpNA;               // 未使用
    char* cpSide;             // 穴をあける方向. L(左)/R(右)/B(両方)
    char* cpRefPartNumber;    // 穴と部品が紐づいている

    for (int iLoop = 1; iLoop < iSectionRow; iLoop++) {
        *strHole++;
    }

    // 半角スペース区切りで値を取得する
    cpHoleGroupName = strtok(str, INPUT_SEPARATION);
    strncpy(strHole->cHoleGroupName, cpHoleGroupName, sizeof(strHole->cHoleGroupName));

    // 第一引数に NULL を指定すると前回の str 値の続きから始める
    cpXCord = strtok(NULL, INPUT_SEPARATION);
    strncpy(strHole->cXCord, cpXCord, sizeof(strHole->cXCord));

    cpNA = strtok(NULL, INPUT_SEPARATION);
    strncpy(strHole->cDatumType, cpNA, sizeof(strHole->cDatumType));

    cpSide = strtok(NULL, INPUT_SEPARATION);
    strncpy(strHole->cSide, cpSide, sizeof(strHole->cSide));

    // cpRefPartNumber は存在しないこともあるため、あれば登録する
    cpRefPartNumber = strtok(NULL, INPUT_SEPARATION);
    if (cpRefPartNumber) {
        strncpy(strHole->cRefPartNumber, cpRefPartNumber, sizeof(strHole->cRefPartNumber));
    }

}

/*====================================================================*\
FUNCTION : getHoleTable
PURPOSE  : HOLES系統の値を格納する
  InputFileHoleTable* strHoleTable,      (out) 格納先
  char  str[PRO_LINE_SIZE]     (in)    inputFile内の1行
  int   iSectionRow            (in)    セクション内での現在の行数
  int   iEnum                  (in)    Enum値

\*====================================================================*/
ProError  getHoleTable(InputFileHoleTable* strHoleTable,
    char str[INPUTFILE_MAXLINE],
    int iSectionRow,
    int iEnum) {

    char* cpHoleGroupName;      // 穴グループ名
    char* cpHoleID;             // 穴番号
    char* cpXCord;              // 基準穴からのX距離
    char* cpYCord;              // 基準穴からのY距離
    char* cpZCord;              // 基準穴からのZ距離
    char* cpFrameDiameter;      // フレーム穴直径
    char* cpInnerLineDiameter;  // インナーライナーの穴直径
    char* cpNA1;                // 未使用1
    char* cpNA2;                // 未使用2

    for (int iLoop = 1; iLoop < iSectionRow; iLoop++) {
        *strHoleTable++;
    }

    // 半角スペース区切りで値を取得する
    cpHoleGroupName = strtok(str, INPUT_SEPARATION);
    strncpy(strHoleTable->cHoleGroupName, cpHoleGroupName, sizeof(strHoleTable->cHoleGroupName));

    // 第一引数に NULL を指定すると前回の str 値の続きから始める
    cpHoleID = strtok(NULL, INPUT_SEPARATION);
    strncpy(strHoleTable->cHoleID, cpHoleID, sizeof(strHoleTable->cHoleID));

    cpXCord = strtok(NULL, INPUT_SEPARATION);
    strncpy(strHoleTable->cXCord, cpXCord, sizeof(strHoleTable->cXCord));

    cpYCord = strtok(NULL, INPUT_SEPARATION);
    strncpy(strHoleTable->cYCord, cpYCord, sizeof(strHoleTable->cYCord));

    cpZCord = strtok(NULL, INPUT_SEPARATION);
    strncpy(strHoleTable->cZCord, cpZCord, sizeof(strHoleTable->cZCord));

    cpFrameDiameter = strtok(NULL, INPUT_SEPARATION);
    strncpy(strHoleTable->cFrameDiameter, cpFrameDiameter, sizeof(strHoleTable->cFrameDiameter));

    cpInnerLineDiameter = strtok(NULL, INPUT_SEPARATION);
    strncpy(strHoleTable->cInnerLineDiameter, cpInnerLineDiameter, sizeof(strHoleTable->cInnerLineDiameter));

    cpNA1 = strtok(NULL, INPUT_SEPARATION);
    strncpy(strHoleTable->cNA1, cpNA1, sizeof(strHoleTable->cNA1));

    // cpNA2 は存在しないこともあるため、あれば登録する
    cpNA2 = strtok(NULL, INPUT_SEPARATION);
    if (cpNA2) {
        strncpy(strHoleTable->cHoleFlag, cpNA2, sizeof(strHoleTable->cHoleFlag));
    }

}

/*====================================================================*\
FUNCTION : getModules
PURPOSE  : MODULES系統の値をRename部とモジュール部に分けて格納する
  InputFileRenameFile* strRename,   (out) Rename部の格納先
  InputFileModules* strModules,     (out) モジュール部の格納先
  char  str[PRO_LINE_SIZE]          (in)  inputFile内の1行
  int   iSectionRow                 (in)  セクション内での現在の行数
  int   iEnum                       (in)  Rename部のEnum値

\*====================================================================*/
ProError  getModules(InputFileRenameFile* strRename,
    InputFileModules* strModules,
    char str[INPUTFILE_MAXLINE],
    int iSectionRow,
    int iEnum) {


    if (giSectionRows[iEnum] != 0
        && giSectionRows[iEnum] >= iSectionRow) {
        /***********************************************
         Rename部の処理
        *************************************************/
        char* cpAfterName;    // Rename後の名前
        char* cpBeforeName;   // Rename前の名前

        for (int iLoop = 1; iLoop < iSectionRow; iLoop++) {
            *strRename++;
        }

        // 半角スペース区切りで値を取得する
        cpAfterName = strtok(str, INPUT_SEPARATION);
        strncpy(strRename->cAfterName, cpAfterName, sizeof(strRename->cAfterName));

        // 第一引数に NULL を指定すると前回の str 値の続きから始める
        cpBeforeName = strtok(NULL, INPUT_SEPARATION);
        strncpy(strRename->cBeforeName, cpBeforeName, sizeof(strRename->cBeforeName));
    }
    else {
        /***********************************************
         モジュール部の処理
        *************************************************/
        char* cpModules;        // 対象部品名
        char* cpConstraint;     // 拘束種類
        char* cpConstraintType; // 拘束条件
        char* cpModulesName;    // モジュール名
        char cModulesNameTemp[INPUTFILE_MAXLINE];   // モジュール名(一時)

        int iSubSectionRow = iSectionRow - giSectionRows[iEnum];

        for (int iLoop = 1; iLoop < iSubSectionRow; iLoop++) {
            *strModules++;
        }

        // 半角スペース区切りで値を取得する
        cpModules = strtok(str, INPUT_SEPARATION);
        strncpy(strModules->cModules, cpModules, sizeof(strModules->cModules));

        // 第一引数に NULL を指定すると前回の str 値の続きから始める
        cpConstraint = strtok(NULL, INPUT_SEPARATION);
        strncpy(strModules->cConstraint, cpConstraint, sizeof(strModules->cConstraint));

        // 拘束種類が H 以外の場合に、拘束条件を処理する
        if (strcmp(strModules->cConstraint,"H") != 0) {
            cpConstraintType = strtok(NULL, INPUT_SEPARATION);
            strncpy(strModules->cConstraintType, cpConstraintType, sizeof(strModules->cConstraintType));
        }

        // Modules系統のModules名は半角スペースで区切らず、1つの値として処理する
        cpModulesName = strtok(NULL, INPUT_SEPARATION);
        strncpy(cModulesNameTemp, cpModulesName, sizeof(cModulesNameTemp));

        if (cpModulesName) {
            while (cpModulesName = strtok(NULL, INPUT_SEPARATION)) {
                strcat_s(cModulesNameTemp, sizeof(cModulesNameTemp), INPUT_SEPARATION);
                strcat_s(cModulesNameTemp, sizeof(cModulesNameTemp), cpModulesName);
            }
        }
        strncpy(strModules->cModulesName, cModulesNameTemp, sizeof(strModules->cModulesName));
    }
}
