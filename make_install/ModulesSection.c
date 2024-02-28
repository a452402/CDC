/*--------------------------------------------------------------------*\
C includes
\*--------------------------------------------------------------------*/
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
#include <ProFamtable.h>
#include <ProFaminstance.h>
/*--------------------------------------------------------------------*\
Application includes
\*--------------------------------------------------------------------*/
#include "InputFile.h"
#include "CommonUtil.h"
#include <ProFeatType.h>

/*-----------------------------------------------------------------*\
    マクロ
\*-----------------------------------------------------------------*/
// H
#define MOUNT_RIGHT         L"MOUNT_RIGHT"
#define MOUNT_LEFT          L"MOUNT_LEFT"
#define MOUNT               L"MOUNT"


// HにてcRefPartNumber(穴と紐づく部品情報)が複数存在するかカウントするための構造体
typedef struct
{
    wchar_t wRightRefPartNumber[INPUTFILE_MAXLINE];      // 穴と紐づく部品情報(穴グループ名)
    wchar_t wLeftRefPartNumber[INPUTFILE_MAXLINE];      // 穴と紐づく部品情報(穴グループ名)
    wchar_t wBothRefPartNumber[INPUTFILE_MAXLINE];      // 穴と紐づく部品情報(穴グループ名)
    wchar_t wRefPartNumber[INPUTFILE_MAXLINE];      // 穴と紐づく部品情報(穴グループ名)
    int iRightCounter;                                   // カウンター
    int iLeftCounter;                                   // カウンター
    int iBothCounter;                                   // カウンター
    int iCounter;                                   // カウンター
}holeRefCounter;

holeRefCounter* strHoleRefCounter;
holeRefCounter* strHoleRefCounterStart;

/*-----------------------------------------------------------------*\
    プロトタイプ宣言
\*-----------------------------------------------------------------*/
ProError SetConstraintsForAddAssy(ProMdl asm_model, wchar_t wAsmCsys[INPUTFILE_MAXLINE], ProMdl comp_model, wchar_t wCompCsys[INPUTFILE_MAXLINE], ProAsmcompconstraint** constraints);
ProError SetConstraintsForAddPart2nd(ProMdl asm_model, ProMdl sub_asm, ProMdl comp_model, ProMdl top_asm, wchar_t wCompSys[INPUTFILE_MAXLINE], ProAsmcompconstraint** constraints);
ProError setModulesSectionV(ProMdl* top_asm, ProMdl mdlCsysPart, InputFileModules* strModules);
ProError setModulesSectionV2nd(ProMdl* top_asm, ProMdl* sub_asm, ProMdl mdlCsysPart, InputFileModules* strModules);

ProError searchTopAssyMOnlyAction(ProFeature* p_feature, ProError status, ProAppData app_data);
ProError searchTopAssyMOnlyFilter(ProFeature* p_feature, ProAppData app_data);
ProError getFeatureIdFromCompAction(ProFeature* pFeature, ProError status, ProAppData app_data);
ProError serachCsysAssy(wchar_t* wConstraintType, int iSectionType, ProMdl* top_asm, ProMdl* mdlCsysPart);
ProError searchTopAssyVOnlyAction(ProFeature* p_feature, ProError status, ProAppData app_data);
ProError searchTopAssyVOnlyFilter(ProFeature* p_feature, ProAppData app_data);

ProError  searchTopAssyForModuleAction(ProFeature* p_feature, ProError status, ProAppData app_data);
ProError  searchTopAssyVOnlyActionForModuleSection(ProFeature* p_feature, ProError status, ProAppData app_data);


/*=========================================================================*\
    Function:	renameInModulesSection
    Purpose:	Moduleセクション (rename部)
    InputFileRenameFile* strModulesRename   (in)    コンフィグレーションファイルの値
    int iSectionMaxRows                     (in)    処理すべきコンフィグレーションファイルの行数
\*=========================================================================*/
ProError renameInModulesSection(InputFileRenameFile* strModulesRename, int iSectionMaxRows) {
    ProError status;
    wchar_t wAfterName[INPUTFILE_MAXLINE];         // Rename後の名前
    wchar_t wBeforeName[INPUTFILE_MAXLINE];        // Rename前の名前
    ProMdl mdlRenameComp;
    ProMdlfileType type;

    for (int iInputMdlCnt = 0; iInputMdlCnt < iSectionMaxRows; iInputMdlCnt++) {

        if (strstr(strModulesRename->cBeforeName, ".asm") != NULL) {
            type = PRO_MDLFILE_ASSEMBLY;
        }else if (strstr(strModulesRename->cBeforeName, ".lay") != NULL) {
            type = PRO_MDLFILE_NOTEBOOK;
        }else if (strstr(strModulesRename->cBeforeName, ".prt") != NULL) {
            type = PRO_MDLFILE_PART;
        }else {
            type = PRO_MDLFILE_UNUSED;
        }
        ProStringToWstring(wAfterName, strModulesRename->cAfterName);
        ProStringToWstring(wBeforeName, strModulesRename->cBeforeName);


        Split(strModulesRename->cAfterName, L"", wAfterName);
        Split(strModulesRename->cBeforeName, L"", wBeforeName);

        // 名前を変更
        status = renameObject(wBeforeName, wAfterName, type);

        strModulesRename++;
    }
}

/*=========================================================================*\
    Function:	setModulesSection
    Purpose:	Moduleセクション (モジュール部)
                windchillからアセンブリをロードし、アセンブル対象に組み付ける
    ProMdl* top_asm                 (in)    アセンブル対象
    InputFileModules* strModules    (in)    コンフィグレーションファイルの値
    InputFileHole* strHole          (in)    コンフィグレーションファイルの値
    int iSectionMaxRows             (in)    処理すべきコンフィグレーションファイルの行数
    int iHoleSectionMaxRows         (in)    処理すべきコンフィグレーションファイルの行数
    int iSectionType                (in)    呼び出し元セクション
\*=========================================================================*/
ProError setModulesSection(ProMdl* top_asm, InputFileModules* strModules, InputFileHole* strHole, int iSectionMaxRows, int iHoleSectionMaxRows, int iSectionType) {
    ProError status;
    int iResultV;
    int iResultH;
    int iResultM;
    int iResultX;
    ProMdl mdlCsysPart;
    wchar_t wModules[INPUTFILE_MAXLINE];           // 対象部品名
    wchar_t wConstraint[INPUTFILE_MAXLINE];        // 拘束種類(V, H, M, X)
    wchar_t wConstraintType[INPUTFILE_MAXLINE];    // 拘束条件(CSYS名)

    ProMdl *top_asm_org = top_asm;

    for (int iInputMdlCnt = 0; iInputMdlCnt < iSectionMaxRows; iInputMdlCnt++) {

        // 初期化
        mdlCsysPart = NULL;
        ProStringToWstring(wConstraint, strModules->cConstraint);
        ProStringToWstring(wConstraintType, strModules->cConstraintType);

        // 拘束種類の確認
        ProWstringCompare(L"V", wConstraint, PRO_VALUE_UNUSED, &iResultV);
        ProWstringCompare(L"H", wConstraint, PRO_VALUE_UNUSED, &iResultH);
        ProWstringCompare(L"M", wConstraint, PRO_VALUE_UNUSED, &iResultM);
        ProWstringCompare(L"X", wConstraint, PRO_VALUE_UNUSED, &iResultX);

        if (iResultV == 0) {
            /***********************************************
             Vの処理
            *************************************************/
            if (iSectionType == ENUM_INPUT_SECTION_MODULES) {
                /***********************************************
                 *MODULES 
                 以下2通りの組付けを行う
                 1,Topアセンブリ下のパーツのCSYSを基準とする
                 2,1が失敗した場合、Topアセンブリ下のアセンブリ下のパーツからCSYSを基準とする
                *************************************************/
                // Topアセンブリの初期化
                top_asm = top_asm_org;

                // 拘束条件(CSYS名)をもとに対象のmdlを取得
                status = serachCsysAssy(wConstraintType, iSectionType, top_asm, &mdlCsysPart);
                if (status != PRO_TK_NO_ERROR) {
                    // CSYSから見て、Topアセンブリが変更になるため、TopアセンブリのProMdl
                    ProMdl top_asm_temp;

                    //Topアセンブリ下のアセンブリ下のパーツからCSYSを基準とする
                    status = serachCsysAssyForModuleSectoin(wConstraintType, iSectionType, top_asm, &mdlCsysPart, &top_asm_temp);

                    status = setModulesSectionV2nd(top_asm , &top_asm_temp, mdlCsysPart, strModules);
                } else{
                    status = setModulesSectionV(top_asm, mdlCsysPart, strModules);
                }

            } else if (iSectionType == ENUM_INPUT_SECTION_FRONT_AXLE) {
                /***********************************************
                 *FRONT_AXLE
                 以下の組付けを行う
                 ・Topアセンブリ下のCSYSを基準とする
                *************************************************/
                setModulesSectionVM(top_asm, strModules);
            }
            else  {
                /***********************************************
                 *MODULES / *FRONT_AXLE 以外
                 以下の組付けを行う
                 ・Topアセンブリ下のパーツのCSYSを基準とする
                *************************************************/
                // 拘束条件(CSYS名)をもとに対象のmdlを取得
                status = serachCsysAssy(wConstraintType, iSectionType, top_asm, &mdlCsysPart);
                if (status != PRO_TK_NO_ERROR) {
                    // 想定外の拘束条件の場合はMAINとして扱う
                    status = serachCsysAssy(L"MAIN", iSectionType, top_asm, &mdlCsysPart);

                    LOG_PRINT("    : %s : Connect as MAIN because CSYS has an abnormal value.", strModules->cModules);
                    strcpy(strModules->cConstraintType, "MAIN");
                }
                setModulesSectionV(top_asm, mdlCsysPart, strModules);

            }
        }
        else if (iResultH == 0) {
            /***********************************************
             Hの処理
            *************************************************/

            setModulesSectionH(top_asm, strModules, strHole, iHoleSectionMaxRows);
        }
        else if (iResultM == 0) {
            /***********************************************
             Mの処理
            *************************************************/
            if (iSectionType != ENUM_INPUT_SECTION_PROP_SHAFT) {
                setModulesSectionM(top_asm, strModules);
            }
            else {
                // *PROP_SHAFT はTopアセンブリ下のCSYSを基準とする
                setModulesSectionVM(top_asm, strModules);
            }
        }
        else if (iResultX == 0) {
            /***********************************************
             Xの処理
            *************************************************/
            setModulesSectionM(top_asm, strModules);

        }
        else {
            // 想定外のエラー. 想定外の拘束種類(V, H, M, X)です
            LOG_PRINT("NOK : %s %s : abnormal value", strModules->cModules, strModules->cConstraint);
        }
        strModules++;
    }
    status = ProSolidRegenerate((ProSolid)*top_asm, PRO_REGEN_FORCE_REGEN);
}

/*=========================================================================*\
    Function:	serachCsysAssyForModuleSectoin
    Purpose:	wConstraintType を持つ、対象のmdlを検索する
    wchar_t* wConstraintType    (in)  CSYS名
    int iSectionType            (in)  呼び出し元セクション
    ProMdl* top_asm             (in)  アセンブル対象
    ProMdl* mdlCsysPart         (out) CSYSをもつモデルハンドル
    ProMdl* mdlTopAssyTemp         (out) CSYSを持つモデルのTopアセンブリ
\*=========================================================================*/
ProError serachCsysAssyForModuleSectoin(wchar_t* wConstraintType, int iSectionType, ProMdl* top_asm, ProMdl* mdlCsysPart, ProMdl* mdlTopAssyTemp) {
    ProError status;
    UserVCsysAppData	local_app_data;
    wchar_t kakunin[INPUTFILE_MAXLINE];  // ジェネリックパラメータ

    // 値の初期化
    local_app_data.model = NULL;
    local_app_data.p_csys = NULL;
    local_app_data.topmodelTemp = NULL;
    ProWstringCopy(wConstraintType, local_app_data.csys_name, PRO_VALUE_UNUSED);

    // TOPアセンブリのすべてのコンポーネントを再帰的に訪問し、内部フィーチャIDを取得する
    status = ProSolidFeatVisit((ProSolid)*top_asm,
        searchTopAssyForModuleAction,
        searchTopAssyMOnlyFilter,
        (ProAppData)&local_app_data);

    if (local_app_data.model == NULL) {
        // CSYS名が想定外(CSYSが見つからなかった)の場合はエラー
        return PRO_TK_BAD_INPUTS;
    }

    *mdlCsysPart = local_app_data.model;
    *mdlTopAssyTemp = local_app_data.topmodelTemp;

    ProMdlMdlnameGet(*mdlTopAssyTemp, kakunin);
    ProMdlMdlnameGet(local_app_data.model, kakunin);

    return PRO_TK_NO_ERROR;
}


/*=========================================================================*\
    Function:	serachCsysAssy
    Purpose:	wConstraintType を持つ、対象のmdlを検索する
    wchar_t* wConstraintType    (in)  CSYS名
    int iSectionType            (in)  呼び出し元セクション
    ProMdl* top_asm             (in)  アセンブル対象
    ProMdl* mdlCsysPart         (out) CSYSをもつモデルハンドル
\*=========================================================================*/
ProError serachCsysAssy(wchar_t* wConstraintType, int iSectionType, ProMdl* top_asm, ProMdl* mdlCsysPart) {
    ProError status;
    UserVCsysAppData	local_app_data;
    wchar_t kakunin[INPUTFILE_MAXLINE];  // ジェネリックパラメータ

    // 値の初期化
    local_app_data.model = NULL;
    local_app_data.p_csys = NULL;
    local_app_data.iSectionType = iSectionType;
    ProWstringCopy(wConstraintType, local_app_data.csys_name, PRO_VALUE_UNUSED);

    // TOPアセンブリのすべてのコンポーネントを再帰的に訪問し、内部フィーチャIDを取得する
    status = ProSolidFeatVisit((ProSolid)*top_asm,
        searchTopAssyVOnlyAction,
        searchTopAssyVOnlyFilter,
        (ProAppData)&local_app_data);

    if (local_app_data.model == NULL) {
        // CSYS名が想定外(CSYSが見つからなかった)の場合はエラー
        return PRO_TK_BAD_INPUTS;
    }

    *mdlCsysPart = local_app_data.model;

    ProMdlMdlnameGet(*top_asm, kakunin);
    ProMdlMdlnameGet(local_app_data.model, kakunin);

    return PRO_TK_NO_ERROR;
}


/*=========================================================================*\
    Function:	setModulesSectionV
    Purpose:	Moduleセクション (モジュール部 - Vのみ)
                windchillからアセンブリをロードし、アセンブル対象に組み付ける
    ProMdl top_asm                 (in)    Topアセンブリのハンドル
    ProMdl mdlCsysPart             (in)    アセンブル対象
    InputFileModules* strModules    (in)    コンフィグレーションファイルの値

    以下のように、CSYS同士を組み付ける

    トップアセンブリ
    |-- パーツ (組付け先 / CSYS系)
    |    |-- 組付け対象CSYS
    |
    |-- アセンブリ (組付け元)
    |    |-- 組付け対象CSYS

\*=========================================================================*/
ProError setModulesSectionV(ProMdl* top_asm, ProMdl mdlCsysPart, InputFileModules* strModules) {
    ProError status;
    // 初期マトリックス。適当
    ProMatrix identity_matrix = { { 1.0, 0.0, 0.0, 0.0 },
                             {0.0, 1.0, 0.0, 0.0},
                             {0.0, 0.0, 1.0, 0.0},
                             {0.0, 0.0, 0.0, 1.0} };

    /***********************************************
    追加するアセンブリを確認する
    *************************************************/
    ProSolid comp_model = NULL;
    ProAsmcomp asmcomp;
    ProAsmcompconstraint* constraints;
    status = PRO_TK_NO_ERROR;

    wchar_t wModules[INPUTFILE_MAXLINE];           // 対象部品名
    wchar_t wConstraintType[INPUTFILE_MAXLINE];    // 拘束条件(CSYS名)

    ProStringToWstring(wModules, strModules->cModules);
    ProStringToWstring(wConstraintType, strModules->cConstraintType);

    // ファイルがロード済みかを確認する
    ProMdlInit(wModules, PRO_MDL_ASSEMBLY, (ProMdl*)&comp_model);

    if (comp_model == NULL) {
        // ロードしていないので、Windchillからアセンブリをロードする
        status = searchAssypathFromWindchill(wModules, SUB_ASSY, PRO_MDLFILE_ASSEMBLY, (ProMdl*)&comp_model);
        if (status != PRO_TK_NO_ERROR) {
            return PRO_TK_GENERAL_ERROR;
        }
    }

    if (status == PRO_TK_NO_ERROR) {
        // アセンブリを追加する
        status = ProAsmcompAssemble((ProAssembly)*top_asm, comp_model, identity_matrix, &asmcomp);

        // constraints配列の準備
        status = ProArrayAlloc(0, sizeof(ProAsmcompconstraint), 1, (ProArray*)&constraints);

        // *FRONT_AXLE 以外の 拘束条件の設定 (PRO_CSYS)
        status = SetConstraintsForAddPart(mdlCsysPart, comp_model, *top_asm, wConstraintType, &constraints);

        if (status == PRO_TK_E_NOT_FOUND) {
            // SetConstraintsForAddPart内でエラーメッセージを出しているのでここでは記載しない
            return PRO_TK_GENERAL_ERROR;
        }else if (status != PRO_TK_NO_ERROR) {
            LOG_PRINT("NOK : %s", strModules->cModules);
            return PRO_TK_GENERAL_ERROR;
        }
        //アセンブリコンポーネントの拘束を設定し、アセンブリを再生成します
        status = ProAsmcompConstraintsSet(NULL, &asmcomp, constraints);

        if (status == PRO_TK_NO_ERROR) {
            LOG_PRINT("OK  : %s", strModules->cModules);
            return PRO_TK_NO_ERROR;
        }
        else {
            LOG_PRINT("NOK : %s", strModules->cModules);
            return PRO_TK_GENERAL_ERROR;
        }
    }
}


/*=========================================================================*\
    Function:	setModulesSectionV2nd
    Purpose:	Moduleセクション (モジュール部 - Vのみ)
                windchillからアセンブリをロードし、アセンブル対象に組み付ける
    ProMdl top_asm                 (in)    Topアセンブリのハンドル
    ProMdl sub_asm                 (in)    Topアセンブリのハンドル
    ProMdl mdlCsysPart             (in)    アセンブル対象
    InputFileModules* strModules    (in)    コンフィグレーションファイルの値

    以下のように、CSYS同士を組み付ける

    トップアセンブリ
    |-- Subアセンブリ
    |    |-- パーツ (組付け先 / CSYS系)
    |        |-- 組付け対象CSYS
    |
    |-- アセンブリ (組付け元)
    |    |-- 組付け対象CSYS

\*=========================================================================*/
ProError setModulesSectionV2nd(ProMdl* top_asm, ProMdl* sub_asm, ProMdl mdlCsysPart, InputFileModules* strModules) {
    ProError status;
    // 初期マトリックス。適当
    ProMatrix identity_matrix = { { 1.0, 0.0, 0.0, 0.0 },
                             {0.0, 1.0, 0.0, 0.0},
                             {0.0, 0.0, 1.0, 0.0},
                             {0.0, 0.0, 0.0, 1.0} };

    /***********************************************
    追加するアセンブリを確認する
    *************************************************/
    ProSolid comp_model = NULL;
    ProAsmcomp asmcomp;
    ProAsmcompconstraint* constraints;
    status = PRO_TK_NO_ERROR;

    wchar_t wModules[INPUTFILE_MAXLINE];           // 対象部品名
    wchar_t wConstraintType[INPUTFILE_MAXLINE];    // 拘束条件(CSYS名)

    ProStringToWstring(wModules, strModules->cModules);
    ProStringToWstring(wConstraintType, strModules->cConstraintType);

    // ファイルがロード済みかを確認する
    ProMdlInit(wModules, PRO_MDL_ASSEMBLY, (ProMdl*)&comp_model);

    if (comp_model == NULL) {
        // ロードしていないので、Windchillからアセンブリをロードする
        status = searchAssypathFromWindchill(wModules, SUB_ASSY, PRO_MDLFILE_ASSEMBLY, (ProMdl*)&comp_model);
        if (status != PRO_TK_NO_ERROR) {
            return PRO_TK_GENERAL_ERROR;
        }
    }

    if (status == PRO_TK_NO_ERROR) {
        // コンポーネントの追加をする
        status = ProAsmcompAssemble((ProAssembly)*top_asm, comp_model, identity_matrix, &asmcomp);

        // constraints配列の準備
        status = ProArrayAlloc(0, sizeof(ProAsmcompconstraint), 1, (ProArray*)&constraints);

        // *FRONT_AXLE 以外の 拘束条件の設定 (PRO_CSYS)
        status = SetConstraintsForAddPart2nd(mdlCsysPart, *top_asm, comp_model, *sub_asm, wConstraintType, &constraints);

        if (status == PRO_TK_E_NOT_FOUND) {
            // SetConstraintsForAddPart2nd内でエラーメッセージを出しているのでここでは記載しない
            return PRO_TK_GENERAL_ERROR;
        }
        else if (status != PRO_TK_NO_ERROR) {
            LOG_PRINT("NOK : %s", strModules->cModules);
            return PRO_TK_GENERAL_ERROR;
        }
        //アセンブリコンポーネントの拘束を設定し、アセンブリを再生成します
        status = ProAsmcompConstraintsSet(NULL, &asmcomp, constraints);

        if (status == PRO_TK_NO_ERROR) {
            LOG_PRINT("OK  : %s", strModules->cModules);
            return PRO_TK_NO_ERROR;
        }
        else {
            LOG_PRINT("NOK : %s", strModules->cModules);
            return PRO_TK_GENERAL_ERROR;
        }
    }
}


/*=========================================================================*\
    Function:	setModulesSectionVM
    Purpose:	Moduleセクション (*FRONT_AXLE の V /  *PROP_SHAFT の M)
                windchillからアセンブリをロードし、アセンブル対象に組み付ける

                TOPアセンブリ直下のCSYSと紐づける
    ProMdl top_asm                 (in)    Topアセンブリのハンドル
    InputFileModules* strModules    (in)    コンフィグレーションファイルの値

    以下のように、CSYS同士を組み付ける

    トップアセンブリ
    |-- 組付け対象CSYS
    |
    |-- アセンブリ (組付け元)
    |    |-- 組付け対象CSYS

\*=========================================================================*/
ProError setModulesSectionVM(ProMdl* top_asm, InputFileModules* strModules) {
    ProError status;

    // 初期マトリックス。適当
    ProMatrix identity_matrix = { { 1.0, 0.0, 0.0, 0.0 },
                             {0.0, 1.0, 0.0, 0.0},
                             {0.0, 0.0, 1.0, 0.0},
                             {0.0, 0.0, 0.0, 1.0} };


    /***********************************************
    追加するアセンブリを確認する
    *************************************************/
    ProSolid comp_model = NULL;
    ProAsmcomp asmcomp;
    ProAsmcompconstraint* constraints;
    status = PRO_TK_NO_ERROR;

    wchar_t wModules[INPUTFILE_MAXLINE];           // 対象部品名
    wchar_t wConstraintType[INPUTFILE_MAXLINE];    // 拘束条件(組付けアセンブリのCSYS名)

    ProStringToWstring(wModules, strModules->cModules);
    ProStringToWstring(wConstraintType, strModules->cConstraintType);

    // ファイルがロード済みかを確認する
    ProMdlInit(wModules, PRO_MDL_ASSEMBLY, (ProMdl*)&comp_model);

    if (comp_model == NULL) {
        // ロードしていないので、Windchillからアセンブリをロードする
        status = searchAssypathFromWindchill(wModules, SUB_ASSY, PRO_MDLFILE_ASSEMBLY, (ProMdl*)&comp_model);
        if (status != PRO_TK_NO_ERROR) {
            return;
        }

    }

    if (status == PRO_TK_NO_ERROR) {
        // アセンブリを追加する
        status = ProAsmcompAssemble((ProAssembly)*top_asm, comp_model, identity_matrix, &asmcomp);

        // constraints配列の準備
        status = ProArrayAlloc(0, sizeof(ProAsmcompconstraint), 1, (ProArray*)&constraints);

        // *FRONT_AXLE の 拘束条件の設定 (PRO_CSYS)
        status = SetConstraintsForAddAssy(*top_asm, wConstraintType, comp_model, wConstraintType, &constraints);

        if (status == PRO_TK_E_NOT_FOUND) {
            // SetConstraintsForAddAssy内にエラーメッセージを記載
            return;
        }else if (status != PRO_TK_NO_ERROR) {
            LOG_PRINT("NOK : %s", strModules->cModules);
            return;
        }
        //アセンブリコンポーネントの拘束を設定し、アセンブリを再生成します
        status = ProAsmcompConstraintsSet(NULL, &asmcomp, constraints);

        if (status == PRO_TK_NO_ERROR) {
            LOG_PRINT("OK  : %s", strModules->cModules);
        }
        else {
            LOG_PRINT("NOK : %s", strModules->cModules);
        }
    }
}

/*=====================================================================*\
FUNCTION: SetConstraintsForAddPart2nd
PURPOSE:  拘束条件の設定 (PRO_CSYS)
ProMdl csys_part                        (in)拘束先/パーツ (組付け先 / CSYS系)
ProMdl top_asm                          (in)拘束先/TOPアセンブリ
ProMdl comp_model                       (in)アセンブリ (組付け元)
ProMdl sub_asm                          (in)Subアセンブリ
wchar_t wCompSys[INPUTFILE_MAXLINE];    (in)拘束先/元のCsys名
ProAsmcompconstraint** constraints      (out)constraints配列

    以下のように、CSYS同士を組み付ける

    TOPアセンブリ                           top_asm
    |-- Subアセンブリ                       sub_asm
    |    |-- パーツ (組付け先 / CSYS系)     csys_part
    |        |-- 組付け対象CSYS
    |
    |-- アセンブリ (組付け元)                comp_model
    |    |-- 組付け対象CSYS
\*=====================================================================*/
ProError SetConstraintsForAddPart2nd(ProMdl csys_part, ProMdl top_asm, ProMdl comp_model, ProMdl sub_asm, wchar_t wCompSys[INPUTFILE_MAXLINE], ProAsmcompconstraint** constraints)
{
    ProError status;
    ProAsmcompconstraint constraint;
    ProModelitem asm_datum, comp_datum;
    ProSelection asm_sel, comp_sel;
    ProAsmcomppath asm_path;
    ProModelitem p_asmintent, p_compintent;

    ProPath patName;
    ProMdlNameGet(comp_model, patName);

    // 動作確認用
    ProPath kakunin1;
    ProPath kakunin2;
    ProPath kakunin3;
    ProPath kakunin4;
    ProMdlMdlnameGet(csys_part, kakunin1);
    ProMdlMdlnameGet(top_asm, kakunin2);
    ProMdlMdlnameGet(comp_model, kakunin3);
    ProMdlMdlnameGet(sub_asm, kakunin4);

    /*-----------------------------------------------------------------*\
        asm_model (拘束先/TOPアセンブリに紐づいているCSYSパーツ) の CSYS の検索
    \*-----------------------------------------------------------------*/
    UserCsysAppData	app_data2;
    int id2;
    ProPath wName;

    // 検索するパーツの名前を設定
    ProMdlMdlnameGet(csys_part, wName);

    ProWstringCopy(wCompSys, app_data2.csys_name, PRO_VALUE_UNUSED);
    app_data2.model = csys_part;

    // CSYSを検索する
    status = ProSolidCsysVisit((ProSolid)csys_part,
        (ProCsysVisitAction)CsysFindVisitAction,
        (ProCsysFilterAction)CsysFindFilterAction,
        (ProAppData)&app_data2);

    if (status != PRO_TK_USER_ABORT) {
        LOG_PRINT("NOK : %w : Not found CSYS of %w", patName, wName);
        return(PRO_TK_E_NOT_FOUND);
    }

    // Modelitemの取得
    status = ProCsysIdGet(app_data2.p_csys, &id2);
    status = ProModelitemInit(app_data2.model, id2, PRO_CSYS, &p_asmintent);

    /*-----------------------------------------------------------------*\
        Asmcomppath を取得
    \*-----------------------------------------------------------------*/
    DatumAppData	appdataFeature;
    ProIdTable c_id2_table;

    /*-----
     subアセンブリ内のパーツ(patName)のフィーチャを検索
    -----*/
    ProWstringCopy(wName, appdataFeature.name, PRO_VALUE_UNUSED);
    appdataFeature.iFindCnt = 0;
    ProSolidFeatVisit((ProSolid)sub_asm, getFeatureIdAction, NULL, (ProAppData)&appdataFeature);
    if (appdataFeature.iFindCnt == 0) {
        // パーツ(patName)のフィーチャが見つからなかった
        return PRO_TK_GENERAL_ERROR;
    }
    else {
        c_id2_table[1] = appdataFeature.feature.id;
    }

    /*-----
    Topアセンブリ内のSubアセンブリのフィーチャを検索
    -----*/
    ProMdlMdlnameGet(sub_asm, wName);
    ProWstringCopy(wName, appdataFeature.name, PRO_VALUE_UNUSED);
    appdataFeature.iFindCnt = 0;
    ProSolidFeatVisit((ProSolid)top_asm, getFeatureIdAction, NULL, (ProAppData)&appdataFeature);
    if (appdataFeature.iFindCnt == 0) {
        // パーツ(patName)のフィーチャが見つからなかった
        return PRO_TK_GENERAL_ERROR;
    }
    else {
        c_id2_table[0] = appdataFeature.feature.id;
    }

    status = ProAsmcomppathInit((ProSolid)top_asm, c_id2_table, 2, &asm_path);

    /*-----------------------------------------------------------------*\
        comp_model (拘束するアセンブリ)コンポーネント の CSYS の検索
    \*-----------------------------------------------------------------*/
    UserCsysAppData	app_data;
    int id;
    ProIdTable c_id_table;
    c_id_table[0] = -1;

    ProWstringCopy(wCompSys, app_data.csys_name, PRO_VALUE_UNUSED);

    app_data.model = comp_model;

    // CSYSを検索する
    status = ProSolidCsysVisit((ProSolid)comp_model,
        (ProCsysVisitAction)CsysFindVisitAction,
        (ProCsysFilterAction)CsysFindFilterAction,
        (ProAppData)&app_data);


    if (status != PRO_TK_USER_ABORT) {
        LOG_PRINT("NOK : %w : Not found CSYS of %w", patName, patName);
        return(PRO_TK_E_NOT_FOUND);
    }

    // Modelitemの取得
    status = ProCsysIdGet(app_data.p_csys, &id);
    status = ProModelitemInit(app_data.model, id, PRO_CSYS, &p_compintent);

    /*-----------------------------------------------------------------*\
        constraintの割当
    \*-----------------------------------------------------------------*/
    status = ProSelectionAlloc(NULL, &p_compintent, &comp_sel);
    status = ProSelectionAlloc(&asm_path, &p_asmintent, &asm_sel);

    status = ProAsmcompconstraintAlloc(&constraint);

    // 座標系に位置を合わせる
    status = ProAsmcompconstraintTypeSet(constraint, PRO_ASM_CSYS);

    // アセンブリの方向を設定
    status = ProAsmcompconstraintAsmreferenceSet(constraint, asm_sel, PRO_DATUM_SIDE_YELLOW);

    // コンポーネントの方向を設定
    status = ProAsmcompconstraintCompreferenceSet(constraint, comp_sel, PRO_DATUM_SIDE_YELLOW);

    // constraints配列の末尾にconstraintを追加する
    status = ProArrayObjectAdd((ProArray*)constraints, PRO_VALUE_UNUSED, 1, &constraint);

    return PRO_TK_NO_ERROR;
}


/*=====================================================================*\
FUNCTION: SetConstraintsForAddPart
PURPOSE:  拘束条件の設定 (PRO_CSYS)
ProMdl asm_model                        (in)拘束先/TOPアセンブリに紐づいているCSYSパーツ
ProMdl comp_model                       (in)拘束するモデル
ProMdl* top_asm                         (in)拘束先のTOPアセンブリ
wchar_t wCompSys[INPUTFILE_MAXLINE];    (in)拘束先/元のCsys名
ProAsmcompconstraint** constraints      (out)constraints配列
\*=====================================================================*/
ProError SetConstraintsForAddPart(ProMdl asm_model, ProMdl comp_model, ProMdl top_asm, wchar_t wCompSys[INPUTFILE_MAXLINE], ProAsmcompconstraint** constraints)
{
    ProError status;
    ProAsmcompconstraint constraint;
    ProModelitem asm_datum, comp_datum;
    ProSelection asm_sel, comp_sel;
    ProAsmcomppath asm_path;
    ProModelitem p_asmintent, p_compintent;

    ProPath patName;
    ProMdlNameGet(comp_model, patName);

    /*-----------------------------------------------------------------*\
        asm_model (拘束先/TOPアセンブリに紐づいているCSYSパーツ) の CSYS の検索
    \*-----------------------------------------------------------------*/
    UserCsysAppData	app_data2;
    int id2;
    ProPath wName;

    // 検索するパーツの名前を設定
    ProMdlMdlnameGet(asm_model, wName);

    ProWstringCopy(wCompSys, app_data2.csys_name, PRO_VALUE_UNUSED);
    app_data2.model = asm_model;

    // CSYSを検索する
    status = ProSolidCsysVisit((ProSolid)asm_model,
        (ProCsysVisitAction)CsysFindVisitAction,
        (ProCsysFilterAction)CsysFindFilterAction,
        (ProAppData)&app_data2);

    if (status != PRO_TK_USER_ABORT) {
        LOG_PRINT("NOK : %w : Not found CSYS of %w", patName, wName);
        return(PRO_TK_E_NOT_FOUND);
    }

    // Modelitemの取得
    status = ProCsysIdGet(app_data2.p_csys, &id2);
    status = ProModelitemInit(app_data2.model, id2, PRO_CSYS, &p_asmintent);

    /*-----------------------------------------------------------------*\
        Asmcomppath を取得
    \*-----------------------------------------------------------------*/
    DatumAppData	appdataFeature;

    ProWstringCopy(wName, appdataFeature.name, PRO_VALUE_UNUSED);
    // 初期化する
    appdataFeature.iFindCnt = 0;

    // Topパーツのパーツ(patName)のフィーチャを検索
    ProSolidFeatVisit((ProSolid)top_asm, getFeatureIdAction, NULL, (ProAppData)&appdataFeature);

    if (appdataFeature.iFindCnt == 0) {
        // パーツ(patName)のフィーチャが見つからなかった
        return PRO_TK_GENERAL_ERROR;
    }

    ProIdTable c_id2_table;
    c_id2_table[0] = appdataFeature.feature.id;
    status = ProAsmcomppathInit((ProSolid)top_asm, c_id2_table, 1, &asm_path);

    /*-----------------------------------------------------------------*\
        comp_model (拘束するアセンブリ)コンポーネント の CSYS の検索
    \*-----------------------------------------------------------------*/
    UserCsysAppData	app_data;
    int id;
    ProIdTable c_id_table;
    c_id_table[0] = -1;

    ProWstringCopy(wCompSys, app_data.csys_name, PRO_VALUE_UNUSED);

    app_data.model = comp_model;

    // CSYSを検索する
    status = ProSolidCsysVisit((ProSolid)comp_model,
        (ProCsysVisitAction)CsysFindVisitAction,
        (ProCsysFilterAction)CsysFindFilterAction,
        (ProAppData)&app_data);


    if (status != PRO_TK_USER_ABORT) {
        LOG_PRINT("NOK : %w : Not found CSYS of %w", patName, patName);
        return(PRO_TK_E_NOT_FOUND);
    }

    // Modelitemの取得
    status = ProCsysIdGet(app_data.p_csys, &id);
    status = ProModelitemInit(app_data.model, id, PRO_CSYS, &p_compintent);

    /*-----------------------------------------------------------------*\
        constraintの割当
    \*-----------------------------------------------------------------*/
    status = ProSelectionAlloc(NULL, &p_compintent, &comp_sel);
    status = ProSelectionAlloc(&asm_path, &p_asmintent, &asm_sel);

    status = ProAsmcompconstraintAlloc(&constraint);

    // 座標系に位置を合わせる
    status = ProAsmcompconstraintTypeSet(constraint, PRO_ASM_CSYS);

    // アセンブリの方向を設定
    status = ProAsmcompconstraintAsmreferenceSet(constraint, asm_sel, PRO_DATUM_SIDE_YELLOW);

    // コンポーネントの方向を設定
    status = ProAsmcompconstraintCompreferenceSet(constraint, comp_sel, PRO_DATUM_SIDE_YELLOW);

    // constraints配列の末尾にconstraintを追加する
    status = ProArrayObjectAdd((ProArray*)constraints, PRO_VALUE_UNUSED, 1, &constraint);

    return PRO_TK_NO_ERROR;
}

/*=====================================================================*\
FUNCTION: SetConstraintsForAddAssy
PURPOSE:  拘束条件の設定 (PRO_CSYS)
ProMdl asm_model                        (in)拘束先/TOPアセンブリ
wchar_t wAsmCsys[INPUTFILE_MAXLINE];    (in)拘束先/TOPアセンブリ / CSYSの名前
ProMdl comp_model                       (in)拘束元/拘束するアセンブリ(Moduleの内容)
wchar_t wCmpCsys[INPUTFILE_MAXLINE];    (in)拘束元/拘束するアセンブリ(Moduleの内容) / CSYSの名前
ProAsmcompconstraint** constraints      (out)constraints配列
\*=====================================================================*/
ProError SetConstraintsForAddAssy(ProMdl asm_model, wchar_t wAsmCsys[INPUTFILE_MAXLINE], ProMdl comp_model, wchar_t wCmpCsys[INPUTFILE_MAXLINE], ProAsmcompconstraint** constraints)
{
    ProError status;
    ProAsmcompconstraint constraint;
    ProModelitem asm_datum, comp_datum;
    ProSelection asm_sel, comp_sel;
    ProAsmcomppath comp_path;
    ProModelitem p_asmintent, p_compintent;

    /*-----------------------------------------------------------------*\
        comp_model コンポーネント の CSYS の検索
    \*-----------------------------------------------------------------*/
    UserCsysAppData	app_data;
    int id;
    ProPath patName;

    // 検索するパーツの名前を設定
    ProMdlMdlnameGet(comp_model, patName);

    ProWstringCopy(wCmpCsys, app_data.csys_name, PRO_VALUE_UNUSED);

    app_data.model = comp_model;

    // CSYSを検索する
    status = ProSolidCsysVisit((ProSolid)comp_model,
        (ProCsysVisitAction)CsysFindVisitAction,
        (ProCsysFilterAction)CsysFindFilterAction,
        (ProAppData)&app_data);

    if (status != PRO_TK_USER_ABORT) {
        LOG_PRINT("NOK : %w : Not found CSYS of %w", patName, patName);
        return(PRO_TK_E_NOT_FOUND);
    }

    // Modelitemの取得
    status = ProCsysIdGet(app_data.p_csys, &id);
    status = ProModelitemInit(app_data.model, id, PRO_CSYS, &p_compintent);


    /*-----------------------------------------------------------------*\
        asm_model アセンブリ の CSYS の検索
    \*-----------------------------------------------------------------*/
    UserCsysAppData	app_data2;
    int id2;
    ProMdl p_owner;
    ProIdTable c_id_table;
    c_id_table[0] = -1;
    ProPath wName;
    // 検索するパーツの名前を設定
    ProMdlMdlnameGet(asm_model, wName);

    ProWstringCopy(wAsmCsys, app_data2.csys_name, PRO_VALUE_UNUSED);
    app_data2.model = asm_model;

    // CSYSを検索する
    status = ProSolidCsysVisit((ProSolid)asm_model,
        (ProCsysVisitAction)CsysFindVisitAction,
        (ProCsysFilterAction)CsysFindFilterAction,
        (ProAppData)&app_data2);

    if (status != PRO_TK_USER_ABORT) {
        LOG_PRINT("NOK : %w : Not found CSYS of %w", patName , wName);
        return(PRO_TK_E_NOT_FOUND);
    }

    // Modelitemの取得
    status = ProCsysIdGet(app_data2.p_csys, &id2);
    status = ProModelitemInit(app_data2.model, id2, PRO_CSYS, &p_asmintent);

    // Asmcomppath を取得
    status = ProAsmcomppathInit((ProSolid)asm_model, c_id_table, 0, &comp_path);

    /*-----------------------------------------------------------------*\
        constraintの割当
    \*-----------------------------------------------------------------*/
    status = ProSelectionAlloc(NULL, &p_compintent, &comp_sel);
    status = ProSelectionAlloc(&comp_path, &p_asmintent, &asm_sel);

    status = ProAsmcompconstraintAlloc(&constraint);

    // 座標系に位置を合わせる
    status = ProAsmcompconstraintTypeSet(constraint, PRO_ASM_CSYS);

    // アセンブリの方向を設定
    status = ProAsmcompconstraintAsmreferenceSet(constraint, asm_sel, PRO_DATUM_SIDE_YELLOW);

    // コンポーネントの方向を設定
    status = ProAsmcompconstraintCompreferenceSet(constraint, comp_sel, PRO_DATUM_SIDE_YELLOW);

    // constraints配列の末尾にconstraintを追加する
    status = ProArrayObjectAdd((ProArray*)constraints, PRO_VALUE_UNUSED, 1, &constraint);

    return PRO_TK_NO_ERROR;
}


/*=========================================================================*\
    Function:	setModulesSectionH
    Purpose:	Moduleセクション (モジュール部 - Hのみ)
                windchillからアセンブリをロードし、アセンブル対象に組み付ける
    ProMdl top_asm                  (in)    Topアセンブリのハンドル
    InputFileModules* strModules    (in)    コンフィグレーションファイルの値
    InputFileHole* strHole          (in)    コンフィグレーションファイルの値（HOLE）
    int iHoleSectionMaxRows         (in)    Holeの処理する行数


    以下のように、CSYS同士を組み付ける
    ※穴あけがBの場合は組付けアセンブリを2回ロードしてそれぞれで組み付ける

    トップアセンブリ
    |-- グループ_CSYS_*** (組付け先 / 穴あけ時に作成したCSYS)
    |    |-- 組付け対象CSYS
    |
    |-- アセンブリ (組付け元)
    |    |-- 組付け対象CSYS (MOUNT/MOUNT_LEFT/MOUNT_RIGHT)
\*=========================================================================*/
ProError setModulesSectionH(ProMdl* top_asm, InputFileModules* strModules, InputFileHole* strHole, int iHoleSectionMaxRows) {
    ProError status;
    wchar_t wRefPartNumber[INPUTFILE_MAXLINE];      // 穴と部品が紐づいている
    wchar_t wSide[INPUTFILE_MAXLINE];               // 穴をあける方向. L(左)/R(右)/B(両方)
    wchar_t wHoleGroupName[INPUTFILE_MAXLINE];      // 穴グループ名
    wchar_t wModules[INPUTFILE_MAXLINE];
    wchar_t wHoleCsysNameL[INPUTFILE_MAXLINE];    // 拘束条件(穴グループのCSYS名)
    wchar_t wHoleCsysNameR[INPUTFILE_MAXLINE];    // 拘束条件(穴グループのCSYS名)
    wchar_t wHoleCsysNameB[INPUTFILE_MAXLINE];    // 拘束条件(穴グループのCSYS名)
    int iResult;
    int iCount = 0;

    ProStringToWstring(wModules, strModules->cModules);

    /***********************************************
      穴あけHの時、cRefPartNumber(穴と紐づく部品情報)が複数存在する場合、
      CSYS名が変わるので、数をカウントする
    *************************************************/
    strHoleRefCounter = (holeRefCounter*)calloc(iHoleSectionMaxRows, sizeof(holeRefCounter));
    if (!strHoleRefCounter) {
        // メモリ不足
        LOG_PRINT("NOK : Not enough memory");
        return;
    }

    // 開始地点アドレスを確保
    strHoleRefCounterStart = strHoleRefCounter;

    for (int iInputMdlCnt = 0; iInputMdlCnt < iHoleSectionMaxRows; iInputMdlCnt++) {

        // 穴と紐づく部品情報を取得
        ProStringToWstring(wRefPartNumber, strHole->cRefPartNumber);

        /***********************************************
          組付け先のHole情報を検索
        *************************************************/
        // 拘束種類の確認
        ProWstringCompare(wModules, wRefPartNumber, PRO_VALUE_UNUSED, &iResult);

        if (iResult == 0) {
            ProStringToWstring(wHoleGroupName, strHole->cHoleGroupName);

            // 開始地点のアドレスに戻す
            strHoleRefCounter = strHoleRefCounterStart;

            // 穴と紐づく部品情報が複数ある場合はCSYS名が変わるため、数をカウントする
            for (int iRefCount = 0; iRefCount < iHoleSectionMaxRows; iRefCount++) {
                ProStringToWstring(wSide, strHole->cSide);
                int iResultR;
                int iResultL;
                int iResultB;
                // 組付け先のCSYS名
                ProWstringCompare(L"R", wSide, PRO_VALUE_UNUSED, &iResultR);
                ProWstringCompare(L"L", wSide, PRO_VALUE_UNUSED, &iResultL);
                ProWstringCompare(L"B", wSide, PRO_VALUE_UNUSED, &iResultB);

                if (iResultR == 0) {
                    ProWstringCompare(strHoleRefCounter->wRightRefPartNumber, L"", PRO_VALUE_UNUSED, &iResult);
                    if (iResult == 0) {
                        // 空の場合、初期値を入れる
                        ProWstringCopy(wHoleGroupName, strHoleRefCounter->wRightRefPartNumber, PRO_VALUE_UNUSED);
                        strHoleRefCounter->iRightCounter = 0;
                        strHoleRefCounter->iCounter = strHoleRefCounter->iRightCounter;
                        break;
                    }

                    ProWstringCompare(strHoleRefCounter->wRightRefPartNumber, wHoleGroupName, PRO_VALUE_UNUSED, &iResult);
                    if (iResult == 0) {
                        // 空でない場合は穴と紐づく部品情報と比較し、一致があればカウンタを回す
                        strHoleRefCounter->iRightCounter = strHoleRefCounter->iRightCounter + 1;
                        strHoleRefCounter->iCounter = strHoleRefCounter->iRightCounter;
                        break;
                    }
                }
                else if (iResultL == 0) {
                    ProWstringCompare(strHoleRefCounter->wLeftRefPartNumber, L"", PRO_VALUE_UNUSED, &iResult);
                    if (iResult == 0) {
                        // 空の場合、初期値を入れる
                        ProWstringCopy(wHoleGroupName, strHoleRefCounter->wLeftRefPartNumber, PRO_VALUE_UNUSED);
                        strHoleRefCounter->iLeftCounter = 0;
                        strHoleRefCounter->iCounter = strHoleRefCounter->iLeftCounter;
                        break;
                    }

                    ProWstringCompare(strHoleRefCounter->wLeftRefPartNumber, wHoleGroupName, PRO_VALUE_UNUSED, &iResult);
                    if (iResult == 0) {
                        // 空でない場合は穴と紐づく部品情報と比較し、一致があればカウンタを回す
                        strHoleRefCounter->iLeftCounter = strHoleRefCounter->iLeftCounter + 1;
                        strHoleRefCounter->iCounter = strHoleRefCounter->iLeftCounter;
                        break;
                    }
                }
                else if (iResultB == 0) {
                    ProWstringCompare(strHoleRefCounter->wBothRefPartNumber, L"", PRO_VALUE_UNUSED, &iResult);
                    if (iResult == 0) {
                        // 空の場合、初期値を入れる
                        ProWstringCopy(wHoleGroupName, strHoleRefCounter->wBothRefPartNumber, PRO_VALUE_UNUSED);
                        strHoleRefCounter->iBothCounter = 0;
                        strHoleRefCounter->iCounter = strHoleRefCounter->iBothCounter;
                        break;
                    }

                    ProWstringCompare(strHoleRefCounter->wBothRefPartNumber, wHoleGroupName, PRO_VALUE_UNUSED, &iResult);
                    if (iResult == 0) {
                        // 空でない場合は穴と紐づく部品情報と比較し、一致があればカウンタを回す
                        strHoleRefCounter->iBothCounter = strHoleRefCounter->iBothCounter + 1;
                        strHoleRefCounter->iCounter = strHoleRefCounter->iBothCounter;
                        break;
                    }
                }
                strHoleRefCounter++;
            }

            // 穴グループ名からCSYS名を取得する
            ProWstringCopy(wHoleGroupName, wHoleCsysNameL, PRO_VALUE_UNUSED);
            ProWstringCopy(wHoleGroupName, wHoleCsysNameR, PRO_VALUE_UNUSED);
            ProWstringCopy(wHoleGroupName, wHoleCsysNameB, PRO_VALUE_UNUSED);
            ProWstringConcatenate(L"_L", wHoleCsysNameL, PRO_VALUE_UNUSED);
            ProWstringConcatenate(L"_R", wHoleCsysNameR, PRO_VALUE_UNUSED);
            ProWstringConcatenate(L"_B", wHoleCsysNameB, PRO_VALUE_UNUSED);

            if (strHoleRefCounter->iCounter != 0) {
                wchar_t wCounter[INPUTFILE_MAXLINE];
                _itow_s(strHoleRefCounter->iCounter, wCounter, sizeof(wCounter), 10);//変換用関数,10進数で変換

                ProWstringConcatenate(L"_", wHoleCsysNameL, PRO_VALUE_UNUSED);
                ProWstringConcatenate(L"_", wHoleCsysNameR, PRO_VALUE_UNUSED);
                ProWstringConcatenate(L"_", wHoleCsysNameB, PRO_VALUE_UNUSED);
                ProWstringConcatenate(wCounter, wHoleCsysNameL, PRO_VALUE_UNUSED);
                ProWstringConcatenate(wCounter, wHoleCsysNameR, PRO_VALUE_UNUSED);
                ProWstringConcatenate(wCounter, wHoleCsysNameB, PRO_VALUE_UNUSED);
            }

            // 穴をあける方向. L(左)/R(右)/B(両方)
            ProStringToWstring(wSide, strHole->cSide);

            int iResultR;
            int iResultL;
            int iResultB;
            // 組付け先のCSYS名
            ProWstringCompare(L"R", wSide, PRO_VALUE_UNUSED, &iResultR);
            ProWstringCompare(L"L", wSide, PRO_VALUE_UNUSED, &iResultL);
            ProWstringCompare(L"B", wSide, PRO_VALUE_UNUSED, &iResultB);

            if (iResultR == 0) {
                setModulesSectionH_Assembly(top_asm, wModules, wHoleCsysNameR, MOUNT_RIGHT);
            }
            else if (iResultL == 0) {
                setModulesSectionH_Assembly(top_asm, wModules, wHoleCsysNameL, MOUNT_LEFT);
            }
            else if (iResultB == 0) {
                setModulesSectionH_Assembly(top_asm, wModules, wHoleCsysNameB, MOUNT);
            }
            iCount++;
        }
        strHole++;
    }

    if (iCount == 0) {
        LOG_PRINT("NOK : %w : Not found target hole", wModules);
    }
    free(strHoleRefCounterStart);
}

/*=========================================================================*\
    Function:	setModulesSectionH_Assembly
    Purpose:	Moduleセクション (モジュール部 - Hのみ)
                windchillからアセンブリをロードし、アセンブル対象に組み付ける

    ProMdl top_asm                 (in)    Topアセンブリのハンドル
    InputFileModules* strModules    (in)    コンフィグレーションファイルの値

\*=========================================================================*/
ProError setModulesSectionH_Assembly(ProMdl* top_asm, wchar_t wModules[INPUTFILE_MAXLINE], wchar_t wHoleCsysName[INPUTFILE_MAXLINE], wchar_t wConstraintType[INPUTFILE_MAXLINE]) {

    ProError status = PRO_TK_NO_ERROR;
    ProAsmcompconstraint* constraints;


    // 初期マトリックス。適当
    ProMatrix identity_matrix = { { 1.0, 0.0, 0.0, 0.0 },
                             {0.0, 1.0, 0.0, 0.0},
                             {0.0, 0.0, 1.0, 0.0},
                             {0.0, 0.0, 0.0, 1.0} };

    /***********************************************
    追加するアセンブリを確認する
    *************************************************/
    ProSolid comp_model = NULL;
    ProAsmcomp asmcomp;

    // ファイルがロード済みかを確認する
    ProMdlInit(wModules, PRO_MDL_ASSEMBLY, (ProMdl*)&comp_model);

    if (comp_model == NULL) {
        // ロードしていないので、Windchillからアセンブリをロードする
        status = searchAssypathFromWindchill(wModules, SUB_ASSY, PRO_MDLFILE_ASSEMBLY, (ProMdl*)&comp_model);
        if (status != PRO_TK_NO_ERROR) {
            return;
        }
    }
    /***********************************************
    組付け処理開始
    *************************************************/

    // アセンブリを追加する
    status = ProAsmcompAssemble((ProAssembly)*top_asm, comp_model, identity_matrix, &asmcomp);

    // constraints配列の準備
    status = ProArrayAlloc(0, sizeof(ProAsmcompconstraint), 1, (ProArray*)&constraints);

    // 拘束条件の設定 (PRO_CSYS)
    status = SetConstraintsForAddAssy(*top_asm, wHoleCsysName, comp_model, wConstraintType, &constraints);

    if (status == PRO_TK_E_NOT_FOUND) {
        // SetConstraintsForAddAssy内にエラーメッセージを記載
        return;
    }else if (status != PRO_TK_NO_ERROR) {
        LOG_PRINT("NOK : %w", wModules);
        return;
    }
    //アセンブリコンポーネントの拘束を設定し、アセンブリを再生成します
    status = ProAsmcompConstraintsSet(NULL, &asmcomp, constraints);

    if (status == PRO_TK_NO_ERROR) {
        LOG_PRINT("OK  : %w", wModules);
    }
    else {
        LOG_PRINT("NOK : %w", wModules);
    }
}



/*=========================================================================*\
    Function:	setModulesSectionM
    Purpose:	Moduleセクション (モジュール部 - Mのみ)
                windchillからアセンブリをロードし、アセンブル対象に組み付ける
    ProMdl top_asm                 (in)    Topアセンブリのハンドル
    InputFileModules* strModules    (in)    コンフィグレーションファイルの値

    以下のように、アセンブリ下のCSYS同士を組み付ける

    トップアセンブリ
    |-- アセンブリ (組付け先 / VやHで追加)
    |    |-- 組付け対象CSYS
    |
    |-- アセンブリ (組付け元)
    |    |-- 組付け対象CSYS

\*=========================================================================*/
ProError setModulesSectionM(ProMdl* top_asm, InputFileModules* strModules) {
    ProError status;


    // 初期マトリックス。適当
    ProMatrix identity_matrix = { { 1.0, 0.0, 0.0, 0.0 },
                             {0.0, 1.0, 0.0, 0.0},
                             {0.0, 0.0, 1.0, 0.0},
                             {0.0, 0.0, 0.0, 1.0} };


    /***********************************************
    追加するアセンブリを確認する
    *************************************************/
    ProSolid comp_model = NULL;
    ProAsmcomp asmcomp;
    ProAsmcompconstraint* constraints;
    status = PRO_TK_NO_ERROR;

    wchar_t wModules[INPUTFILE_MAXLINE];           // 対象部品名
    wchar_t wConstraintType[INPUTFILE_MAXLINE];    // 拘束条件(CSYS名)

    ProStringToWstring(wModules, strModules->cModules);
    ProStringToWstring(wConstraintType, strModules->cConstraintType);

    // ファイルがロード済みかを確認する
    ProMdlInit(wModules, PRO_MDL_ASSEMBLY, (ProMdl*)&comp_model);

    if (comp_model == NULL) {
        // ロードしていないので、Windchillからアセンブリをロードする
        status = searchAssypathFromWindchill(wModules, SUB_ASSY, PRO_MDLFILE_ASSEMBLY, (ProMdl*)&comp_model);
        if (status != PRO_TK_NO_ERROR) {
            return;
        }

    }

    if (status == PRO_TK_NO_ERROR) {
        // アセンブリを追加する
        status = ProAsmcompAssemble((ProAssembly)*top_asm, comp_model, identity_matrix, &asmcomp);

        // constraints配列の準備
        status = ProArrayAlloc(0, sizeof(ProAsmcompconstraint), 1, (ProArray*)&constraints);

        // 拘束条件の設定 (PRO_CSYS)
        status = SetConstraintsForAddPart_M(comp_model, *top_asm, wConstraintType, &constraints);

        if (status == PRO_TK_E_NOT_FOUND) {
            return;
        }else if (status != PRO_TK_NO_ERROR) {
            LOG_PRINT("NOK : %s", strModules->cModules);
            return;
        }
        //アセンブリコンポーネントの拘束を設定し、アセンブリを再生成します
        status = ProAsmcompConstraintsSet(NULL, &asmcomp, constraints);

        if (status == PRO_TK_NO_ERROR) {
            LOG_PRINT("OK  : %s", strModules->cModules);
        }
        else {
            LOG_PRINT("NOK : %s", strModules->cModules);
        }
    }

}

/*=====================================================================*\
FUNCTION: SetConstraintsForAddPart
PURPOSE:  拘束条件の設定 (PRO_CSYS)
ProMdl comp_model                       (in)拘束するモデル
ProMdl* top_asm                         (in)拘束先のTOPアセンブリ
wchar_t wCompSys[INPUTFILE_MAXLINE];    (in)拘束先/元のCsys名
ProAsmcompconstraint** constraints      (out)constraints配列
\*=====================================================================*/
ProError SetConstraintsForAddPart_M(ProMdl comp_model, ProMdl top_asm, wchar_t wCompSys[INPUTFILE_MAXLINE], ProAsmcompconstraint** constraints)
{
    ProError status;
    ProAsmcompconstraint constraint;
    ProModelitem asm_datum, comp_datum;
    ProSelection asm_sel, comp_sel;
    ProAsmcomppath asm_path;
    ProModelitem p_asmintent, p_compintent;

    ProPath wLogName;
    // 検索するパーツの名前を設定
    ProMdlMdlnameGet(comp_model, wLogName);
    /*-----------------------------------------------------------------*\
         (拘束先/ VやHで追加されたアセンブリ) の CSYS の検索
    \*-----------------------------------------------------------------*/
    ProPath wTopName;
    UserCsysAppData	app_data2;
    int id2;

    // 検索するパーツの名前を設定
    ProMdlMdlnameGet(top_asm, wTopName);

    ProWstringCopy(wCompSys, app_data2.csys_name, PRO_VALUE_UNUSED);
    app_data2.p_csys = NULL;

    // VHのアセンブル一覧を確認する
    // TOPアセンブリのすべてのコンポーネントを再帰的に訪問し、内部フィーチャIDを取得する
    status = ProSolidFeatVisit((ProSolid)top_asm,
        searchTopAssyMOnlyAction,
        searchTopAssyMOnlyFilter,
        (ProAppData)&app_data2);

    if (app_data2.p_csys == NULL) {
        LOG_PRINT("NOK : %w : Not found CSYS of %w", wLogName, wTopName);
        return(PRO_TK_E_NOT_FOUND);
    }

    // Modelitemの取得
    status = ProCsysIdGet(app_data2.p_csys, &id2);
    status = ProModelitemInit(app_data2.model, id2, PRO_CSYS, &p_asmintent);

    /*-----------------------------------------------------------------*\
        Asmcomppath を取得
    \*-----------------------------------------------------------------*/
    ProPath patName;
    DatumAppData	appdataFeature;

    // 検索するパーツの名前を設定
    ProMdlMdlnameGet(app_data2.model, patName);
    ProWstringCopy(patName, appdataFeature.name, PRO_VALUE_UNUSED);
    // 初期化する
    appdataFeature.iFindCnt = 0;

    // Topパーツのパーツ(patName)のフィーチャを検索
    ProSolidFeatVisit((ProSolid)top_asm, getFeatureIdFromCompAction, NULL, (ProAppData)&appdataFeature);

    if (appdataFeature.iFindCnt == 0) {
        // パーツ(patName)のフィーチャが見つからなかった
        return PRO_TK_GENERAL_ERROR;
    }

    ProIdTable c_id2_table;
    c_id2_table[0] = appdataFeature.feature.id;


    status = ProAsmcomppathInit((ProSolid)top_asm, c_id2_table, 1, &asm_path);

    /*-----------------------------------------------------------------*\
        comp_model (拘束するアセンブリ)コンポーネント の CSYS の検索
    \*-----------------------------------------------------------------*/
    UserCsysAppData	app_data;
    int id;
    ProIdTable c_id_table;
    c_id_table[0] = -1;

    ProWstringCopy(wCompSys, app_data.csys_name, PRO_VALUE_UNUSED);

    app_data.model = comp_model;

    // CSYSを検索する
    status = ProSolidCsysVisit((ProSolid)comp_model,
        (ProCsysVisitAction)CsysFindVisitAction,
        (ProCsysFilterAction)CsysFindFilterAction,
        (ProAppData)&app_data);

    if (status != PRO_TK_USER_ABORT) {
        LOG_PRINT("NOK : %w : Not found CSYS of %w", wLogName, wLogName);
        return(PRO_TK_E_NOT_FOUND);
    }

    // Modelitemの取得
    status = ProCsysIdGet(app_data.p_csys, &id);
    status = ProModelitemInit(app_data.model, id, PRO_CSYS, &p_compintent);

    /*-----------------------------------------------------------------*\
        constraintの割当
    \*-----------------------------------------------------------------*/
    status = ProSelectionAlloc(NULL, &p_compintent, &comp_sel);
    status = ProSelectionAlloc(&asm_path, &p_asmintent, &asm_sel);

    status = ProAsmcompconstraintAlloc(&constraint);

    // 座標系に位置を合わせる
    status = ProAsmcompconstraintTypeSet(constraint, PRO_ASM_CSYS);

    // アセンブリの方向を設定
    status = ProAsmcompconstraintAsmreferenceSet(constraint, asm_sel, PRO_DATUM_SIDE_YELLOW);

    // コンポーネントの方向を設定
    status = ProAsmcompconstraintCompreferenceSet(constraint, comp_sel, PRO_DATUM_SIDE_YELLOW);

    // constraints配列の末尾にconstraintを追加する
    status = ProArrayObjectAdd((ProArray*)constraints, PRO_VALUE_UNUSED, 1, &constraint);

    return PRO_TK_NO_ERROR;
}

/*====================================================*\
  Function : FeatureIDGetFilter()
  Purpose  : Topアセンブリ下のアセンブリを取得する
\*====================================================*/
ProError searchTopAssyMOnlyFilter(ProFeature* p_feature, ProAppData app_data)
{

    ProFeattype ftype = NULL;
    ProMdl mdl;
    ProMdlType type;
    ProMdlName name;

    // フィーチャタイプを取得 (データム, CSYS, コンポーネント ...)
    ProFeatureTypeGet(p_feature, &ftype);
    if (ftype != PRO_FEAT_COMPONENT)
    {
        return PRO_TK_CONTINUE;
    }

    ProAsmcompMdlGet((ProAsmcomp*)p_feature, &mdl);

    // アセンブリの場合のみ処理する
    ProMdlTypeGet(mdl, &type);
    if (type != PRO_MDL_ASSEMBLY)
    {
        return PRO_TK_CONTINUE;
    }
    return(PRO_TK_NO_ERROR);
}
/*====================================================*\
  Function : searchTopAssyVOnlyFilter()
  Purpose  : Topアセンブリ下のパーツを取得する
\*====================================================*/
ProError searchTopAssyVOnlyFilter(ProFeature* p_feature, ProAppData app_data)
{

    ProFeattype ftype = NULL;
    ProMdl mdl;
    ProMdlType type;
    ProMdlName name;

    // フィーチャタイプを取得 (データム, CSYS, コンポーネント ...)
    ProFeatureTypeGet(p_feature, &ftype);
    if (ftype != PRO_FEAT_COMPONENT)
    {
        return PRO_TK_CONTINUE;
    }

    ProAsmcompMdlGet((ProAsmcomp*)p_feature, &mdl);

    // アセンブリの場合のみ処理する
    ProMdlTypeGet(mdl, &type);
    if (type != PRO_MDL_PART)
    {
        return PRO_TK_CONTINUE;
    }
    return(PRO_TK_NO_ERROR);
}


/*====================================================*\
  Function : searchTopAssyMOnlyAction()
  Purpose  : Topアセンブリ下のアセンブリのCSYSを検索する
\*====================================================*/
ProError  searchTopAssyMOnlyAction(ProFeature* p_feature, ProError status, ProAppData app_data)
{
    ProMdlfileType     mdltype;
    ProMdlName        w_name;
    int iResult;
    ProMdl mdl;
    ProMdlType		mdl_type;

    UserCsysAppData	local_app_data;

    // アセンブリのハンドルを取得
    ProAsmcompMdlGet((ProAsmcomp*)p_feature, &mdl);

    if (((UserCsysAppData*)app_data)->p_csys == NULL) {
        ProWstringCopy(((UserCsysAppData*)app_data)->csys_name, local_app_data.csys_name, PRO_VALUE_UNUSED);
        local_app_data.model = mdl;

        local_app_data.p_csys = NULL;

        // CSYSを検索する
        status = ProSolidCsysVisit((ProSolid)mdl,
            (ProCsysVisitAction)CsysFindVisitAction,
            (ProCsysFilterAction)CsysFindFilterAction,
            (ProAppData)&local_app_data);

        if (local_app_data.p_csys != NULL) {
            ((UserCsysAppData*)app_data)->p_csys = local_app_data.p_csys;
            ((UserCsysAppData*)app_data)->model = local_app_data.model;
            return PRO_TK_CONTINUE;
        }

    }
    // 引き続きアクセスする
    return PRO_TK_NO_ERROR;
}


/*====================================================*\
  Function : searchTopAssyVOnlyAction()
  Purpose  : Topアセンブリ下のアセンブリのCSYSを検索し、CoordinateSystemsと比較して一致したもののみを処理する
\*====================================================*/
ProError  searchTopAssyVOnlyAction(ProFeature* p_feature, ProError status, ProAppData app_data)
{
    ProMdlfileType     mdltype;
    ProMdlName        w_name;
    int iResult;
    ProMdl mdl;
    ProMdlType		mdl_type;
    UserCsysAppData	local_app_data;

    InputFileCsys* strCsys;
    int iCount;

    if (((UserVCsysAppData*)app_data)->iSectionType == ENUM_INPUT_SECTION_GEARBOX) {
        // GEARBOXの場合
        strCsys = gstrGearboxCoordinateSystems;
        iCount = giSectionRows[ENUM_INPUT_SECTION_GEARBOX_COORDINATE_SYSTEMS];
    }
    else {
        // GEARBOX以外の場合
        strCsys = gstrVehicleCoordinateSystems;
        iCount = giSectionRows[ENUM_INPUT_SECTION_VEHICLE_COORDINATE_SYSTEMS];
    }
    // アセンブリのハンドルを取得
    ProAsmcompMdlGet((ProAsmcomp*)p_feature, &mdl);

    if (((UserVCsysAppData*)app_data)->p_csys == NULL) {
        wchar_t wGenericInConfigFile[INPUTFILE_MAXLINE];  // ジェネリックパラメータ
        wchar_t wSearchCsysPart[INPUTFILE_MAXLINE];  // ジェネリックパラメータ
        wchar_t wKakunin[INPUTFILE_MAXLINE];  // 確認用
        int iReasult;

        for (int iLoop = 0; iLoop < iCount; iLoop++ ) {
            // 検索対象のmdlを取得
            ProStringToWstring(wGenericInConfigFile, strCsys->cInstanceParameter);
            ProMdlMdlnameGet(mdl, wSearchCsysPart);

            ProWstringCompare(wGenericInConfigFile, wSearchCsysPart, PRO_VALUE_UNUSED, &iReasult);

            if (iReasult == NULL) {
                ProWstringCopy(((UserVCsysAppData*)app_data)->csys_name, local_app_data.csys_name, PRO_VALUE_UNUSED);
                local_app_data.model = mdl;
                local_app_data.p_csys = NULL;

                // CSYSを検索する
                status = ProSolidCsysVisit((ProSolid)mdl,
                    (ProCsysVisitAction)CsysFindVisitAction,
                    (ProCsysFilterAction)CsysFindFilterAction,
                    (ProAppData)&local_app_data);

                if (status == PRO_TK_USER_ABORT) {
                    ((UserVCsysAppData*)app_data)->model = mdl;
                    return PRO_TK_CONTINUE;

                }

            }
            strCsys++;
        }

    }
    // 引き続きアクセスする
    return PRO_TK_NO_ERROR;
}


/*====================================================*\
  Function : searchTopAssyForModuleAction()
  Purpose  : Topアセンブリ下のアセンブリを検索.
                CoordinateSystemsと比較して一致したもののみを処理する
\*====================================================*/
ProError  searchTopAssyForModuleAction(ProFeature* p_feature, ProError status, ProAppData app_data)
{
    UserVCsysAppData	local_app_data;
    wchar_t kakunin[INPUTFILE_MAXLINE];  // ジェネリックパラメータ
    ProMdl mdl;

    // アセンブリのハンドルを取得
    status = ProAsmcompMdlGet((ProAsmcomp*)p_feature, &mdl);


    // 値の初期化
    local_app_data.model = NULL;
    local_app_data.p_csys = NULL;
    
    ProWstringCopy(((UserVCsysAppData*)app_data)->csys_name, local_app_data.csys_name, PRO_VALUE_UNUSED);

    ProMdlMdlnameGet(mdl, kakunin);

    // アセンブリのすべてのコンポーネントを再帰的に訪問し、内部フィーチャIDを取得する
    status = ProSolidFeatVisit((ProSolid)mdl,
        searchTopAssyVOnlyActionForModuleSection,
        searchTopAssyVOnlyFilter,
        (ProAppData)&local_app_data);

    if (local_app_data.model != NULL) {
        //CSYSが見つかった場合,ループ抜ける
        ((UserVCsysAppData*)app_data)->model = local_app_data.model;
        ((UserVCsysAppData*)app_data)->topmodelTemp = mdl;
        return PRO_TK_BAD_INPUTS;
    }

    ProMdlMdlnameGet(local_app_data.model, kakunin);

    // 引き続きアクセスする
    return PRO_TK_NO_ERROR;
}



/*====================================================*\
  Function : searchTopAssyVOnlyActionForModuleSection()
  Purpose  : Topアセンブリ下のアセンブリの下のパーツ下のCSYSを検索し、CoordinateSystemsと比較して一致したもののみを処理する
\*====================================================*/
ProError  searchTopAssyVOnlyActionForModuleSection(ProFeature* p_feature, ProError status, ProAppData app_data)
{
    ProMdlfileType     mdltype;
    ProMdlName        w_name;
    int iResult;
    ProMdl mdl;
    ProMdlType		mdl_type;
    UserCsysAppData	local_app_data;

    InputFileCsys* strCsys;
    int iCount;

    // アセンブリのハンドルを取得
    ProAsmcompMdlGet((ProAsmcomp*)p_feature, &mdl);

    if (((UserVCsysAppData*)app_data)->p_csys == NULL) {
        wchar_t wGenericInConfigFile[INPUTFILE_MAXLINE];  // ジェネリックパラメータ
        wchar_t wSearchCsysPart[INPUTFILE_MAXLINE];  // ジェネリックパラメータ
        wchar_t wKakunin[INPUTFILE_MAXLINE];  // 確認用
        int iReasult;


        //-------------------------------------------------------------------
        //  GEARBOXの場合
        //-------------------------------------------------------------------
        strCsys = gstrGearboxCoordinateSystems;
        iCount = giSectionRows[ENUM_INPUT_SECTION_GEARBOX_COORDINATE_SYSTEMS];

        for (int iLoop = 0; iLoop < iCount; iLoop++) {
            // 検索対象のmdlを取得
            //ProStringToWstring(wGenericInConfigFile, strCsys->cGenericParameter);
            ProStringToWstring(wGenericInConfigFile, strCsys->cInstanceParameter);
            ProMdlMdlnameGet(mdl, wSearchCsysPart);

            ProWstringCompare(wGenericInConfigFile, wSearchCsysPart, PRO_VALUE_UNUSED, &iReasult);

            if (iReasult == NULL) {
                ProWstringCopy(((UserVCsysAppData*)app_data)->csys_name, local_app_data.csys_name, PRO_VALUE_UNUSED);
                local_app_data.model = mdl;
                local_app_data.p_csys = NULL;

                // CSYSを検索する
                status = ProSolidCsysVisit((ProSolid)mdl,
                    (ProCsysVisitAction)CsysFindVisitAction,
                    (ProCsysFilterAction)CsysFindFilterAction,
                    (ProAppData)&local_app_data);

                if (status == PRO_TK_USER_ABORT) {
                    ((UserVCsysAppData*)app_data)->model = mdl;
                    return PRO_TK_USER_ABORT;

                }

            }
            strCsys++;
        }

        //-------------------------------------------------------------------
        //   GEARBOX以外の場合
        //-------------------------------------------------------------------
        strCsys = gstrVehicleCoordinateSystems;
        iCount = giSectionRows[ENUM_INPUT_SECTION_VEHICLE_COORDINATE_SYSTEMS];

        for (int iLoop = 0; iLoop < iCount; iLoop++) {
            // 検索対象のmdlを取得
            //ProStringToWstring(wGenericInConfigFile, strCsys->cGenericParameter);
            ProStringToWstring(wGenericInConfigFile, strCsys->cInstanceParameter);
            ProMdlMdlnameGet(mdl, wSearchCsysPart);

            ProWstringCompare(wGenericInConfigFile, wSearchCsysPart, PRO_VALUE_UNUSED, &iReasult);

            if (iReasult == NULL) {
                ProWstringCopy(((UserVCsysAppData*)app_data)->csys_name, local_app_data.csys_name, PRO_VALUE_UNUSED);
                local_app_data.model = mdl;
                local_app_data.p_csys = NULL;

                // CSYSを検索する
                status = ProSolidCsysVisit((ProSolid)mdl,
                    (ProCsysVisitAction)CsysFindVisitAction,
                    (ProCsysFilterAction)CsysFindFilterAction,
                    (ProAppData)&local_app_data);

                if (status == PRO_TK_USER_ABORT) {
                    ((UserVCsysAppData*)app_data)->model = mdl;
                    return PRO_TK_USER_ABORT;

                }

            }
            strCsys++;
        }
    }
    // 引き続きアクセスする
    return PRO_TK_NO_ERROR;
}


/*====================================================*\
  Function : getFeatureIdFromCompAction()
  Purpose  : 引数で指定したFeatureのFeatureIdを取得する (PRO_FEAT_COMPONENT専用)
\*====================================================*/
ProError  getFeatureIdFromCompAction(ProFeature* pFeature, ProError status, ProAppData app_data)
{
    ProMdlName        wName;
    int iResult;
    ProFeattype ftype;
    ProMdlfileType     mdltype;

    if (((DatumAppData*)app_data)->iFindCnt != 0) {
        return PRO_TK_CONTINUE;
    }
    // フィーチャタイプを取得 (データム, CSYS, コンポーネント ...)
    status = ProFeatureTypeGet(pFeature, &ftype);
    if (ftype != PRO_FEAT_COMPONENT) {
        return PRO_TK_CONTINUE;
    }
    status = ProAsmcompMdlMdlnameGet((ProAsmcomp*)pFeature, &mdltype, wName);

    // アセンブリ以外は PRO_TK_CONTINUE
    if (mdltype != PRO_MDLFILE_ASSEMBLY) {
        return PRO_TK_CONTINUE;
    }

    // ファミリーテーブルの確認
    ProWstringCompare(wName, ((DatumAppData*)app_data)->name, PRO_VALUE_UNUSED, &iResult);

    if (iResult == 0) {
        // 名前が一致した場合

        // app_dataに値を格納する
        ((DatumAppData*)app_data)->feature = *pFeature;
        // カウントをインクリメントする
        ((DatumAppData*)app_data)->iFindCnt = ((DatumAppData*)app_data)->iFindCnt + 1;
        return PRO_TK_NO_ERROR;
    }
    else {
        // 名前が一致しなかった場合、インスタンスとして持っていれば一致したこととする
        ProFamtable famtable;
        ProFaminstance famInstance;
        ProMdl mdlFamTable, mdlInstance;
        ProMdlName		wAssyName;

        status = ProMdlnameInit(wName, PRO_MDLFILE_ASSEMBLY, &mdlFamTable);
        status = ProFamtableInit(mdlFamTable, &famtable);
        status = ProFaminstanceInit(wName, &famtable, &famInstance);
        status = ProFaminstanceRetrieve(&famInstance, &mdlInstance);
        if (status != PRO_TK_NO_ERROR) {
            // インスタンスが見つかりません
            return PRO_TK_CONTINUE;
        }
        status = ProMdlMdlnameGet(mdlInstance, wAssyName);

        ProWstringCompare(wAssyName, ((DatumAppData*)app_data)->name, PRO_VALUE_UNUSED, &iResult);
        if (iResult == 0) {
            // 名前が一致した場合

            // app_dataに値を格納する
            ((DatumAppData*)app_data)->feature = *pFeature;
            // カウントをインクリメントする
            ((DatumAppData*)app_data)->iFindCnt = ((DatumAppData*)app_data)->iFindCnt + 1;
            return PRO_TK_NO_ERROR;
        }

    }
    return PRO_TK_CONTINUE;
}
