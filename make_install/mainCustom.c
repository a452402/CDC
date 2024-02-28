/*--------------------------------------------------------------------*\
C includes
\*--------------------------------------------------------------------*/
#include <time.h>
#include <wchar.h>
#include <stdio.h>
#include <tchar.h>
#include <direct.h>
#include <shlwapi.h>
#include <windows.h>


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
#include <ProToolkitDll.h>
#include <ProSimprep.h>
#include < ProBrowser.h >

/*--------------------------------------------------------------------*\
Application includes
\*--------------------------------------------------------------------*/
#include "main.h"
#include "CommonUtil.h"
#include "InputFile.h"
#include "ParametersSection.h"
#include "FeaturesSection.h"
#include "ModulesSection.h"
#include "CoordinateSystemsSection.h"
#include "Holes_HoleTableSection.h"
#include "mainCustom.h"
#include <ProCore.h>

/*-----------------------------------------------------------------*\
    プロトタイプ宣言
\*-----------------------------------------------------------------*/
ProError  checkInitial(int* iErrorCnt, ProCharPath* cImpFolderPath, ProPath wTopAssyName, WindchillInfo* windchillInfo, ProCharPath* cFeatureLsitPath);
ProError  endProcessLog(WindchillInfo windchillInfo, ProMdlName wTopAssyNewName, int iErrorFlag);
ProError  renamePart(ProPath wOldPart, ProPath wNewPart, ProPath wLeftRight);
ProError  deleteFamilyInstances(ProPath wPartName);
ProError  renameFrontAxle(InputFileModules* strModules, int iSectionMaxRows, ProMdlName wTopAssyNewName, ProMdlName wType);
ProError  checkInObject(ProMdl top_asm, WindchillInfo windchillInfo);
ProError  renameSameTopAssyName(InputFileRenameFile* strModulesRename, int iSectionMaxRows, ProMdlName wTopAssyNewName, ProMdlName wType);

/*-----------------------------------------------------------------*\
    マクロ
\*-----------------------------------------------------------------*/
//inputfileの最大文字数
#define MAX_LINE 512

// 設定ファイルの固定値
#define SETTING_LOG_INIT_PATH                       ".\\UD-SAVP\\log\\"
#define SETTING_SERIAL_NUMBER_SECTION   			"SERIAL_NUMBER_SECTION"
#define SETTING_PREFIX_KEY       					"PREFIX"
#define SETTING_PREFIX_VALUE     					"NUMBER"

// バージョン情報
#define VERSION_INFO_MODE           "BatchMode"
#define VERSION_INFO_VER            "Ver.11"
#define VERSION_INFO_DATE           "2023/9/5"

/*-----------------------------------------------------------------*\
    グローバル変数
\*-----------------------------------------------------------------*/
// ログファイルパス
ProCharPath gcLogFilePath = "";

/*====================================================================*\
FUNCTION : mainCustom
PURPOSE  : 処理開始
\*====================================================================*/
ProError  mainCustom()
{
    TRAIL_PRINT("%s(%d) : === START ===", __func__, __LINE__);


	ProError status;
    ProMdl top_asm;                 // topアセンブリのハンドル
    int iErrorCnt = 0;              // エラーの数
    ProCharPath cImpFolderPath;     // 設定ファイルから読み込むコンフィグレーションファイルのパス
    ProPath wTopAssyName;          // ファイル から取得するTopAssy

    ProCharPath cFeatureLsitPath;          // Feature が Frame Template に無い場合でもログにNOKと表示させないListPath

    WindchillInfo windchillInfo;
    ProWstringCopy(L"", windchillInfo.defaultWorkspace, PRO_VALUE_UNUSED);
    ProWstringCopy(L"", windchillInfo.defaultContext, PRO_VALUE_UNUSED);
    ProWstringCopy(L"", windchillInfo.defaultFolder, PRO_VALUE_UNUSED);
    ProWstringCopy(L"", windchillInfo.settingIniWorkspace, PRO_VALUE_UNUSED);
    ProWstringCopy(L"", windchillInfo.settingIniContext, PRO_VALUE_UNUSED);
    ProWstringCopy(L"", windchillInfo.settingIniFolder, PRO_VALUE_UNUSED);
    strcpy("", cFeatureLsitPath);

    /***********************************************
     Windchillの接続確認・その他処理を行う上で最低条件の確認
    *************************************************/
    status = checkInitial(&iErrorCnt, &cImpFolderPath, wTopAssyName, &windchillInfo, &cFeatureLsitPath);
    TRAIL_PRINT("%s(%d) : checkInitial = %s", __func__, __LINE__, getProErrorMessage(status));

    /***********************************************
     FeatureLsitファイルのロード
    *************************************************/
    if (strcmp(cFeatureLsitPath, "") != NULL) {
        status = loadFeatureList(cFeatureLsitPath);
        TRAIL_PRINT("%s(%d) : loadFeatureList = %s", __func__, __LINE__, getProErrorMessage(status));
        if (status != PRO_TK_NO_ERROR)
        {
            iErrorCnt++;
        }
    }


    /***********************************************
     コンフィグレーションファイルのロード
    *************************************************/
    status = loadInputFile(cImpFolderPath, &iErrorCnt);
    TRAIL_PRINT("%s(%d) : loadInputFile = %s", __func__, __LINE__, getProErrorMessage(status));
    if (status != PRO_TK_NO_ERROR)
    {
        iErrorCnt++;
    }

    /***********************************************
     ファイルの命名規則の確認／名前に使用する番号を取得する
    *************************************************/
    ProMdlName wTopAssyNameNumber;
    checkNewFrameName(&wTopAssyNameNumber, &iErrorCnt);


    if (iErrorCnt == 0) {
        /***********************************************
         WindchillからTOPアセンブリ（テンプレートフレーム）を検索・ロード
        *************************************************/
        ProPath wTopAssy;
        ProWstringCopy(wTopAssyName, wTopAssy, PRO_VALUE_UNUSED);
        ProWstringConcatenate(L".asm", wTopAssy, PRO_VALUE_UNUSED);
        status = searchAssypathFromWindchill(wTopAssy, TOP_ASSY, PRO_MDLFILE_ASSEMBLY, &top_asm);
        TRAIL_PRINT("%s(%d) : searchAssypathFromWindchill = %s", __func__, __LINE__, getProErrorMessage(status));
        if (status != PRO_TK_NO_ERROR)
        {
            iErrorCnt++;
        }
    }

    /***********************************************
     処理開始確認
     上記でエラーが発生していた場合は処理を開始させない
    *************************************************/
    if (iErrorCnt != 0) {
        endProcessLog(windchillInfo, wTopAssyName, ENUM_MESSAGE_SETTING_FAILED);
        return;
    }

    /***********************************************
         組付け処理の開始
    *************************************************/
    LOG_PRINT("---------------------------------------------------");
    LOG_PRINT("   Start processing");
    LOG_PRINT("---------------------------------------------------");

    /***********************************************
      *PARAMETER セクションの適用
      * INNERLINER_HOLES/INNERLINER_HOLE_TABLE にて*PARAMETER を使用するので、先に実施する
    *************************************************/
    if (giSectionRows[ENUM_INPUT_SECTION_PARAMETERS] != 0) {
        LOG_PRINT("*PARAMETER start");
        setParametersSection(top_asm, wTopAssyName, gstrParametersFeature, gstrParameters, giSectionRows[ENUM_INPUT_SECTION_PARAMETERS_FEATURE], giSectionRows[ENUM_INPUT_SECTION_PARAMETERS]);
    }

    /***********************************************
      *FEATURES セクションの適用
    *************************************************/
    if (giSectionRows[ENUM_INPUT_SECTION_FEATURES] != 0) {
        LOG_PRINT("*FEATURES start");
        setFeaturesSection(top_asm, gstrFeatures, giSectionRows[ENUM_INPUT_SECTION_FEATURES], gstrFeatureList);
    }

    /***********************************************
      *Holes/HoleTable セクションの適用
　    * PARAMETERSにて、フレームの長さが変わる可能性があるため、*PARAMETER セクション後に実施
    *************************************************/
    if (giSectionRows[ENUM_INPUT_SECTION_HOLES] != 0 && giSectionRows[ENUM_INPUT_SECTION_HOLE_TABLE] != 0) {
        LOG_PRINT("*Holes/HoleTable start");
        setHoles_HoleTableSection(&top_asm, &top_asm, wTopAssyName, gstrHoles, gstrHoleTable, giSectionRows[ENUM_INPUT_SECTION_HOLES], giSectionRows[ENUM_INPUT_SECTION_HOLE_TABLE], HOLE);
    }

    /***********************************************
      *INNERLINER のロード
    *************************************************/
    wchar_t* wOldTopInnerLiner = (wchar_t*)calloc(INPUTFILE_MAXLINE, sizeof(wchar_t));
    wchar_t* wNewTopInnerLiner = (wchar_t*)calloc(INPUTFILE_MAXLINE, sizeof(wchar_t));
    ProError statusInnerLinerLoad;

    ProMdl mdlTopInnerLiner;
    statusInnerLinerLoad = loadAssembly(gstrInnerlinerRename, giSectionRows[ENUM_INPUT_SECTION_INNERLINER_RENAME], &mdlTopInnerLiner, wOldTopInnerLiner, wNewTopInnerLiner);

   if (statusInnerLinerLoad == PRO_TK_NO_ERROR) {
       /***********************************************
         *INNERLINER_PARAMETERS セクションの適用
       *************************************************/
       if (giSectionRows[ENUM_INPUT_SECTION_INNERLINER_PARAMETERS] != 0) {
           LOG_PRINT("*INNERLINER_PARAMETERS start");
           setParametersSection(mdlTopInnerLiner, wOldTopInnerLiner, gstrInnerlinerParametersFeature, gstrInnerlinerParameters, giSectionRows[ENUM_INPUT_SECTION_INNERLINER_PARAMETERS_FEATURE], giSectionRows[ENUM_INPUT_SECTION_INNERLINER_PARAMETERS]);
       }

       /***********************************************
         *INNERLINER_HOLES/INNERLINER_HOLE_TABLE セクションの適用
         * PARAMETERSにて、フレームの長さが変わる可能性があるため、*INNERLINER_PARAMETERS セクション後に実施
       *************************************************/
       if (giSectionRows[ENUM_INPUT_SECTION_INNERLINER_HOLES] != 0 && giSectionRows[ENUM_INPUT_SECTION_INNERLINER_HOLE_TABLE] != 0) {
           LOG_PRINT("*INNERLINER_HOLES/INNERLINER_HOLE_TABLE start");
           setHoles_HoleTableSection(&mdlTopInnerLiner, &top_asm, wOldTopInnerLiner, gstrInnerlinerHoles, gstrInnerlinerHoleTable, giSectionRows[ENUM_INPUT_SECTION_INNERLINER_HOLES], giSectionRows[ENUM_INPUT_SECTION_INNERLINER_HOLE_TABLE], INNERLINER_HOLE);
       }

       /***********************************************
         *INNERLINER_FEATURES セクションの適用
         * 
       *************************************************/
       if (giSectionRows[ENUM_INPUT_SECTION_INNERLINER_FEATURES] != 0) {
           LOG_PRINT("*INNERLINER_FEATURES start");
           setFeaturesSection(mdlTopInnerLiner, gstrInnerlinerFeatures, giSectionRows[ENUM_INPUT_SECTION_INNERLINER_FEATURES], gstrFeatureList);
       }

       /***********************************************
         VEHICLE_COORDINATE_SYSTEMS(INNERLINER_PARAMETERS) セクションの適用
         setModulesSection 前に実施
       *************************************************/
       if (giSectionRows[ENUM_INPUT_SECTION_VEHICLE_COORDINATE_SYSTEMS] != 0) {
           LOG_PRINT("*VEHICLE_COORDINATE_SYSTEMS(INNERLINER_PARAMETERS) start");
           SetCoordinateSystemsSection(mdlTopInnerLiner, gstrVehicleCoordinateSystems, giSectionRows[ENUM_INPUT_SECTION_VEHICLE_COORDINATE_SYSTEMS]);
       }

       /***********************************************
         *INNERLINER セクションの適用
       *************************************************/
       if (giSectionRows[ENUM_INPUT_SECTION_INNERLINER_RENAME] != 0 || giSectionRows[ENUM_INPUT_SECTION_INNERLINER] != 0) {
           LOG_PRINT("*INNERLINER start");

           if (giSectionRows[ENUM_INPUT_SECTION_INNERLINER_RENAME] == 0) {
               LOG_PRINT("ATTENTION : No rename object");
           }
           else {
               renameInModulesSection(gstrInnerlinerRename, giSectionRows[ENUM_INPUT_SECTION_INNERLINER_RENAME]);
           }

           if (giSectionRows[ENUM_INPUT_SECTION_INNERLINER] == 0) {
               LOG_PRINT("ATTENTION : No Assembly object");
           }
           else {
               // INNERLINERにModules部は存在しないが、形式美として記載
               setModulesSection(&mdlTopInnerLiner, gstrInnerliner, NULL, giSectionRows[ENUM_INPUT_SECTION_INNERLINER], NULL, ENUM_INPUT_SECTION_INNERLINER);
           }
       }

   }else {
       LOG_PRINT("Stop all INNERLINER processing");
   }

   /***********************************************
     *FRONT_AXLE のロード
   *************************************************/
   wchar_t* wOldTopFrontAxle = (wchar_t*)calloc(INPUTFILE_MAXLINE, sizeof(wchar_t)); // Rename前の名前(拡張子抜き)
   wchar_t* wNewTopFrontAxle = (wchar_t*)calloc(INPUTFILE_MAXLINE, sizeof(wchar_t)); // Rename後の名前(拡張子抜き)

   ProMdl mdlTopFrontAxle;
   status = loadAssembly(gstrFrontAxleRename, giSectionRows[ENUM_INPUT_SECTION_FRONT_AXLE_RENAME], &mdlTopFrontAxle, wOldTopFrontAxle, wNewTopFrontAxle);
   if (status == PRO_TK_NO_ERROR) {
       /***********************************************
         VEHICLE_COORDINATE_SYSTEMS(FRONT_AXLE) セクションの適用
         setModulesSection 前に実施
       *************************************************/
       if (giSectionRows[ENUM_INPUT_SECTION_VEHICLE_COORDINATE_SYSTEMS] != 0) {
           LOG_PRINT("*VEHICLE_COORDINATE_SYSTEMS(FRONT_AXLE) start");
           SetCoordinateSystemsSection(mdlTopFrontAxle, gstrVehicleCoordinateSystems, giSectionRows[ENUM_INPUT_SECTION_VEHICLE_COORDINATE_SYSTEMS]);
       }

       /***********************************************
         *FRONT_AXLE セクションの適用
       *************************************************/
       if (giSectionRows[ENUM_INPUT_SECTION_FRONT_AXLE_RENAME] != 0 || giSectionRows[ENUM_INPUT_SECTION_FRONT_AXLE] != 0) {
           LOG_PRINT("*FRONT_AXLE start");

           if (giSectionRows[ENUM_INPUT_SECTION_FRONT_AXLE] == 0) {
               LOG_PRINT("ATTENTION : No Assembly object");
           }
           else {
               setModulesSection(&mdlTopFrontAxle, gstrFrontAxle, NULL, giSectionRows[ENUM_INPUT_SECTION_FRONT_AXLE], NULL, ENUM_INPUT_SECTION_FRONT_AXLE);
           }

           if (giSectionRows[ENUM_INPUT_SECTION_FRONT_AXLE_RENAME] == 0) {
               LOG_PRINT("ATTENTION : No rename object");
           }
           else {
               renameInModulesSection(gstrFrontAxleRename, giSectionRows[ENUM_INPUT_SECTION_FRONT_AXLE_RENAME]);
           }

       }
       /***********************************************
         *FRONT_AXLE_PARAMS セクションの適用
         *FRONT_AXLEのリネーム処理後に実施
       *************************************************/
       if (giSectionRows[ENUM_INPUT_SECTION_FRONT_AXLE_PARAMS] != 0) {
           LOG_PRINT("*FRONT_AXLE_PARAMS start");
           setParametersSection(mdlTopFrontAxle, wOldTopFrontAxle, gstrFrontAxleParamsFeature, gstrFrontAxleParams, giSectionRows[ENUM_INPUT_SECTION_FRONT_AXLE_PARAMS_FEATURE], giSectionRows[ENUM_INPUT_SECTION_FRONT_AXLE_PARAMS]);
       }

   }
   else {
       LOG_PRINT("Stop all FRONT_AXLE processing ");
   }
   free(wOldTopFrontAxle);
   free(wNewTopFrontAxle);

   /***********************************************
     *CAB のロード
   *************************************************/
   wchar_t* wOldTopCab = (wchar_t*)calloc(INPUTFILE_MAXLINE, sizeof(wchar_t)); // Rename前の名前(拡張子抜き)
   wchar_t* wNewTopCab = (wchar_t*)calloc(INPUTFILE_MAXLINE, sizeof(wchar_t)); // Rename後の名前(拡張子抜き)
   ProMdl mdlTopCab;
   status = loadAssembly(gstrCabRename, giSectionRows[ENUM_INPUT_SECTION_CAB_RENAME], &mdlTopCab, wOldTopCab, wNewTopCab);
   if (status == PRO_TK_NO_ERROR) {
       /***********************************************
         VEHICLE_COORDINATE_SYSTEMS(CAB) セクションの適用
         setModulesSection 前に実施
       *************************************************/
       if (giSectionRows[ENUM_INPUT_SECTION_VEHICLE_COORDINATE_SYSTEMS] != 0) {
           LOG_PRINT("*VEHICLE_COORDINATE_SYSTEMS(CAB) start");
           SetCoordinateSystemsSection(mdlTopCab, gstrVehicleCoordinateSystems, giSectionRows[ENUM_INPUT_SECTION_VEHICLE_COORDINATE_SYSTEMS]);
       }

       /***********************************************
         *CAB セクションの適用
       *************************************************/
       if (giSectionRows[ENUM_INPUT_SECTION_CAB_RENAME] != 0 || giSectionRows[ENUM_INPUT_SECTION_CAB] != 0) {
           LOG_PRINT("*CAB start");

           if (giSectionRows[ENUM_INPUT_SECTION_CAB_RENAME] == 0) {
               LOG_PRINT("ATTENTION : No rename object");
           }
           else {
               renameInModulesSection(gstrCabRename, giSectionRows[ENUM_INPUT_SECTION_CAB_RENAME]);
           }

           if (giSectionRows[ENUM_INPUT_SECTION_CAB] == 0) {
               LOG_PRINT("ATTENTION : No Assembly object");
           }
           else {
               setModulesSection(&mdlTopCab, gstrCab, NULL, giSectionRows[ENUM_INPUT_SECTION_CAB], NULL, ENUM_INPUT_SECTION_CAB);
           }
       }
   }
   else {
       LOG_PRINT("Stop all CAB processing ");
   }
   free(wOldTopCab);
   free(wNewTopCab);

   /***********************************************
     *ENGINE のロード
   *************************************************/
   wchar_t* wOldTopEngine = (wchar_t*)calloc(INPUTFILE_MAXLINE, sizeof(wchar_t)); // Rename前の名前(拡張子抜き)
   wchar_t* wNewTopEngine = (wchar_t*)calloc(INPUTFILE_MAXLINE, sizeof(wchar_t)); // Rename後の名前(拡張子抜き)
   ProMdl mdlTopEngine;
   status = loadAssembly(gstrEngineRename, giSectionRows[ENUM_INPUT_SECTION_ENGINE_RENAME], &mdlTopEngine, wOldTopEngine, wNewTopEngine);
   if (status == PRO_TK_NO_ERROR) {
       /***********************************************
         VEHICLE_COORDINATE_SYSTEMS(ENGINE) セクションの適用
         setModulesSection 前に実施
       *************************************************/
       if (giSectionRows[ENUM_INPUT_SECTION_VEHICLE_COORDINATE_SYSTEMS] != 0) {
           LOG_PRINT("*VEHICLE_COORDINATE_SYSTEMS(ENGINE) start");
           SetCoordinateSystemsSection(mdlTopEngine, gstrVehicleCoordinateSystems, giSectionRows[ENUM_INPUT_SECTION_VEHICLE_COORDINATE_SYSTEMS]);
       }

       /***********************************************
         *ENGINE セクションの適用
       *************************************************/
       if (giSectionRows[ENUM_INPUT_SECTION_ENGINE_RENAME] != 0 || giSectionRows[ENUM_INPUT_SECTION_ENGINE] != 0) {
           LOG_PRINT("*ENGINE start");

           if (giSectionRows[ENUM_INPUT_SECTION_ENGINE_RENAME] == 0) {
               LOG_PRINT("ATTENTION : No rename object");
           }
           else {
               renameInModulesSection(gstrEngineRename, giSectionRows[ENUM_INPUT_SECTION_ENGINE_RENAME]);
           }

           if (giSectionRows[ENUM_INPUT_SECTION_ENGINE] == 0) {
               LOG_PRINT("ATTENTION : No Assembly object");
           }
           else {
               setModulesSection(&mdlTopEngine, gstrEngine, NULL, giSectionRows[ENUM_INPUT_SECTION_ENGINE], NULL, ENUM_INPUT_SECTION_ENGINE);
           }
       }
   }
   else {
       LOG_PRINT("Stop all ENGINE processing ");
   }
   free(wOldTopEngine);
   free(wNewTopEngine);

   /***********************************************
     *GEARBOX のロード
   *************************************************/
   wchar_t* wOldTopGearbox = (wchar_t*)calloc(INPUTFILE_MAXLINE, sizeof(wchar_t)); // Rename前の名前(拡張子抜き)
   wchar_t* wNewTopGearbox = (wchar_t*)calloc(INPUTFILE_MAXLINE, sizeof(wchar_t)); // Rename後の名前(拡張子抜き)
   ProMdl mdlTopGearbox;
   status = loadAssembly(gstrGearboxRename, giSectionRows[ENUM_INPUT_SECTION_GEARBOX_RENAME], &mdlTopGearbox, wOldTopGearbox, wNewTopGearbox);
   if (status == PRO_TK_NO_ERROR) {
       /***********************************************
         *GEARBOX_COORDINATE_SYSTEMS セクションの適用
         * setModulesSection 前に実施
       *************************************************/
       if (giSectionRows[ENUM_INPUT_SECTION_GEARBOX_COORDINATE_SYSTEMS] != 0) {
           LOG_PRINT("*GEARBOX_COORDINATE_SYSTEMS start");
           SetCoordinateSystemsSection(mdlTopGearbox, gstrGearboxCoordinateSystems, giSectionRows[ENUM_INPUT_SECTION_GEARBOX_COORDINATE_SYSTEMS]);
       }

       /***********************************************
         *GEARBOX セクションの適用
       *************************************************/
       if (giSectionRows[ENUM_INPUT_SECTION_GEARBOX_RENAME] != 0 || giSectionRows[ENUM_INPUT_SECTION_GEARBOX] != 0) {
           LOG_PRINT("*GEARBOX start");

           if (giSectionRows[ENUM_INPUT_SECTION_GEARBOX_RENAME] == 0) {
               LOG_PRINT("ATTENTION : No rename object");
           }
           else {
               renameInModulesSection(gstrGearboxRename, giSectionRows[ENUM_INPUT_SECTION_GEARBOX_RENAME]);
           }

           if (giSectionRows[ENUM_INPUT_SECTION_GEARBOX] == 0) {
               LOG_PRINT("ATTENTION : No Assembly object");
           }
           else {
               setModulesSection(&mdlTopGearbox, gstrGearbox, NULL, giSectionRows[ENUM_INPUT_SECTION_GEARBOX], NULL, ENUM_INPUT_SECTION_GEARBOX);
           }
       }
   }
   else {
       LOG_PRINT("Stop all GEARBOX processing ");
   }
   free(wOldTopGearbox);
   free(wNewTopGearbox);

   /***********************************************
     *PROP_SHAFT のロード
   *************************************************/
   wchar_t* wOldTopPropShaft = (wchar_t*)calloc(INPUTFILE_MAXLINE, sizeof(wchar_t)); // Rename前の名前(拡張子抜き)
   wchar_t* wNewTopPropShaft = (wchar_t*)calloc(INPUTFILE_MAXLINE, sizeof(wchar_t)); // Rename後の名前(拡張子抜き)

   ProMdl mdlTopPropShaft;
   status = loadAssembly(gstrPropShaftRename, giSectionRows[ENUM_INPUT_SECTION_PROP_SHAFT_RENAME], &mdlTopPropShaft, wOldTopPropShaft, wNewTopPropShaft);
   if (status == PRO_TK_NO_ERROR) {
       /***********************************************
         VEHICLE_COORDINATE_SYSTEMS(PROP_SHAFT) セクションの適用
         setModulesSection 前に実施
       *************************************************/
       if (giSectionRows[ENUM_INPUT_SECTION_VEHICLE_COORDINATE_SYSTEMS] != 0) {
           LOG_PRINT("*VEHICLE_COORDINATE_SYSTEMS(PROP_SHAFT) start");
           SetCoordinateSystemsSection(mdlTopPropShaft, gstrVehicleCoordinateSystems, giSectionRows[ENUM_INPUT_SECTION_VEHICLE_COORDINATE_SYSTEMS]);
       }
       /***********************************************
         *PROP_SHAFT セクションの適用
       *************************************************/
       if (giSectionRows[ENUM_INPUT_SECTION_PROP_SHAFT_RENAME] != 0 || giSectionRows[ENUM_INPUT_SECTION_PROP_SHAFT] != 0) {
           LOG_PRINT("*PROP_SHAFT start");

           if (giSectionRows[ENUM_INPUT_SECTION_PROP_SHAFT_RENAME] == 0) {
               LOG_PRINT("ATTENTION : No rename object");
           }
           else {
               renameInModulesSection(gstrPropShaftRename, giSectionRows[ENUM_INPUT_SECTION_PROP_SHAFT_RENAME]);
           }

           if (giSectionRows[ENUM_INPUT_SECTION_PROP_SHAFT] == 0) {
               LOG_PRINT("ATTENTION : No Assembly object");
           }
           else {
               setModulesSection(&mdlTopPropShaft, gstrPropShaft, NULL, giSectionRows[ENUM_INPUT_SECTION_PROP_SHAFT], NULL, ENUM_INPUT_SECTION_PROP_SHAFT);
           }
       }

       /***********************************************
         *PROP_SHAFT_PARAMS セクションの適用
         *PROP_SHAFTのリネーム処理後に実施
       *************************************************/
       if (giSectionRows[ENUM_INPUT_SECTION_PROP_SHAFT_PARAMS] != 0) {
           LOG_PRINT("*PROP_SHAFT_PARAMS start");
           setParametersSection(mdlTopPropShaft, wNewTopPropShaft, gstrPropShaftParamsFeature, gstrPropShaftParams, giSectionRows[ENUM_INPUT_SECTION_PROP_SHAFT_PARAMS_FEATURE], giSectionRows[ENUM_INPUT_SECTION_PROP_SHAFT_PARAMS]);
       }
   }
   else {
       LOG_PRINT("Stop all PROP_SHAFT processing ");
   }
   free(wOldTopPropShaft);
   free(wNewTopPropShaft);

   /***********************************************
     *VEHICLE_COORDINATE_SYSTEMS セクションの適用
       MODULES にて、インスタンスモデル置換前のCSYSを使用しているため、
       MODULES 後に実施する
   *************************************************/
   if (giSectionRows[ENUM_INPUT_SECTION_VEHICLE_COORDINATE_SYSTEMS] != 0) {
       LOG_PRINT("*VEHICLE_COORDINATE_SYSTEMS(MODULES) start");
       SetCoordinateSystemsSection(top_asm, gstrVehicleCoordinateSystems, giSectionRows[ENUM_INPUT_SECTION_VEHICLE_COORDINATE_SYSTEMS]);
   }
   
   /***********************************************
    アセンブリをMaster表示＆アクティブにしてから、再生を行う
   *************************************************/
   ProSimprep p_simp_rep;
   LOG_PRINT("Change to master rep and Regenerate");
   // マスター表示の場合は簡略表示の識別子が-1のため、名前はNULLとする
   status = ProSimprepInit(NULL, -1, (ProSolid)top_asm, &p_simp_rep);
   if (status == PRO_TK_NO_ERROR)
   {
       status = ProSimprepActivate((ProSolid)top_asm, &p_simp_rep);
       if (status == PRO_TK_NO_ERROR)
       {
           LOG_PRINT("OK  : Successfully change to master rep");
       }
       else {
           LOG_PRINT("NOK : Failed to change to master rep");
       }
   }
   else {
       LOG_PRINT("NOK : Failed to change master rep because failed to get master rep info");
   }

   status = ProSolidRegenerate((ProSolid)top_asm, PRO_REGEN_FORCE_REGEN);

   if (status == PRO_TK_NO_ERROR)
   {
       LOG_PRINT("OK  : Successfully change to regenerate");
   }
   else {
       LOG_PRINT("NOK : Failed to change to regenerate");
   }

   /***********************************************
      *MODULES セクションの適用
    *************************************************/
   if (giSectionRows[ENUM_INPUT_SECTION_MODULES_RENAME] != 0 || giSectionRows[ENUM_INPUT_SECTION_MODULES] != 0) {
       LOG_PRINT("*MODULES start");

       if (giSectionRows[ENUM_INPUT_SECTION_MODULES_RENAME] == 0) {
           LOG_PRINT("ATTENTION : No rename object");
       }
       else {
           renameInModulesSection(gstrModulesRename, giSectionRows[ENUM_INPUT_SECTION_MODULES_RENAME]);
       }

       if (giSectionRows[ENUM_INPUT_SECTION_MODULES] == 0) {
           LOG_PRINT("ATTENTION : No Assembly object");
       }
       else {
           setModulesSection(&top_asm, gstrModules, gstrHoles, giSectionRows[ENUM_INPUT_SECTION_MODULES], giSectionRows[ENUM_INPUT_SECTION_HOLES], ENUM_INPUT_SECTION_MODULES);
       }
   }

    /***********************************************
     フレームの名前を変更
     初めに名前を変更すると、パラメータが適用されなかったため、
     最後に名前を変更する
      ※InnerLinerのパーツ、レイアウトファイルのリネームが
  　    コンフィグレーションファイルに記載がなかったためここで変更する
      ※同名ファイルは2回以降Widchillへチェックインできないため
  　    Topアセンブリと同様に連番を付与する
    *************************************************/
    LOG_PRINT("Rename assembly and part");

    ProError statusL;
    ProError statusR;
    ProError statusInnerL;
    ProError statusInnerR;

    ProMdlName wTopAssyNewName;
    ProWstringCopy(gwFramename, wTopAssyNewName, PRO_VALUE_UNUSED);
    ProWstringConcatenate(L"_", wTopAssyNewName, PRO_VALUE_UNUSED);
    ProWstringConcatenate(wTopAssyNameNumber, wTopAssyNewName, PRO_VALUE_UNUSED);

    ProMdlName wNewTopAssyForInnerLiner;

    // サイドフレームのリネーム
    status = renameObject(wTopAssyName, wTopAssyNewName, PRO_MDLFILE_ASSEMBLY);
    status = renameObject(wTopAssyName, wTopAssyNewName, PRO_MDLFILE_NOTEBOOK);
    statusL = renamePart(wTopAssyName, wTopAssyNewName, L"_1");
    statusR = renamePart(wTopAssyName, wTopAssyNewName, L"_2");

    // InnerLinerのリネーム
    if (giSectionRows[ENUM_INPUT_SECTION_INNERLINER_RENAME] != 0) {
        renameSameTopAssyName(gstrInnerlinerRename, giSectionRows[ENUM_INPUT_SECTION_INNERLINER_RENAME], wTopAssyNewName, L"FIL");
    }

    // InnerLinerをロードしていない場合は処理をしない
    if (statusInnerLinerLoad == PRO_TK_NO_ERROR) {
        ProWstringCopy(wTopAssyNewName, wNewTopAssyForInnerLiner, PRO_VALUE_UNUSED);
        ProWstringConcatenate(L"_FIL", wNewTopAssyForInnerLiner, PRO_VALUE_UNUSED);

        status = renameObject(wOldTopInnerLiner, wNewTopAssyForInnerLiner, PRO_MDLFILE_NOTEBOOK);
        statusInnerL = renamePart(wOldTopInnerLiner, wNewTopAssyForInnerLiner, L"_1");
        statusInnerR = renamePart(wOldTopInnerLiner, wNewTopAssyForInnerLiner, L"_2");
    }

    if (giSectionRows[ENUM_INPUT_SECTION_FRONT_AXLE_RENAME] != 0) {
        renameSameTopAssyName(gstrFrontAxleRename, giSectionRows[ENUM_INPUT_SECTION_FRONT_AXLE_RENAME], wTopAssyNewName, L"FA");
    }
    if (giSectionRows[ENUM_INPUT_SECTION_FRONT_AXLE] != 0) {
        // *FRONT_AXLE では追加した Module に対して、リネームをする。
        renameFrontAxle(gstrFrontAxle, giSectionRows[ENUM_INPUT_SECTION_FRONT_AXLE], wTopAssyNewName, L"FA");
    }
    if (giSectionRows[ENUM_INPUT_SECTION_CAB_RENAME] != 0) {
        renameSameTopAssyName(gstrCabRename, giSectionRows[ENUM_INPUT_SECTION_CAB_RENAME], wTopAssyNewName, L"CAB");
    }
    if (giSectionRows[ENUM_INPUT_SECTION_ENGINE_RENAME] != 0) {
        renameSameTopAssyName(gstrEngineRename, giSectionRows[ENUM_INPUT_SECTION_ENGINE_RENAME], wTopAssyNewName, L"ENG");
    }
    if (giSectionRows[ENUM_INPUT_SECTION_GEARBOX_RENAME] != 0) {
        renameSameTopAssyName(gstrGearboxRename, giSectionRows[ENUM_INPUT_SECTION_GEARBOX_RENAME], wTopAssyNewName, L"GB");
    }
    if (giSectionRows[ENUM_INPUT_SECTION_PROP_SHAFT_RENAME] != 0) {
        renameSameTopAssyName(gstrPropShaftRename, giSectionRows[ENUM_INPUT_SECTION_PROP_SHAFT_RENAME], wTopAssyNewName, L"PS");
    }

    /***********************************************
     ファミリーテーブルのインスタンスを削除  (SideFrame)
    *************************************************/
    LOG_PRINT("Delete the family table instance");

    ProPath wLeftPart;
    ProPath wRightPart;

    // パーツ名の作成
    ProWstringCopy(wTopAssyNewName, wLeftPart, PRO_VALUE_UNUSED);
    ProWstringConcatenate(L"_1", wLeftPart, PRO_VALUE_UNUSED);
    ProWstringCopy(wTopAssyNewName, wRightPart, PRO_VALUE_UNUSED);
    ProWstringConcatenate(L"_2", wRightPart, PRO_VALUE_UNUSED);

    if (statusL == PRO_TK_NO_ERROR) {
        // インスタンスの削除
        deleteFamilyInstances(wLeftPart);
    }
    else {
        LOG_PRINT("NOK : %w ", wLeftPart);
    }

    if (statusR == PRO_TK_NO_ERROR) {
        deleteFamilyInstances(wRightPart);
    }
    else {
        LOG_PRINT("NOK : %w ", wRightPart);
    }

    /***********************************************
    ファミリーテーブルのインスタンスを削除    (InnerLiner)
    ※InnerLinerをロードしていない場合は処理をしない
    *************************************************/
    if (statusInnerLinerLoad == PRO_TK_NO_ERROR) {
        ProPath wLeftInnerLiner;
        ProPath wRightInnerLiner;

        // パーツ名の作成
        ProWstringCopy(wNewTopAssyForInnerLiner, wLeftInnerLiner, PRO_VALUE_UNUSED);
        ProWstringConcatenate(L"_1", wLeftInnerLiner, PRO_VALUE_UNUSED);
        ProWstringCopy(wNewTopAssyForInnerLiner, wRightInnerLiner, PRO_VALUE_UNUSED);
        ProWstringConcatenate(L"_2", wRightInnerLiner, PRO_VALUE_UNUSED);

        if (statusInnerL == PRO_TK_NO_ERROR) {
            // インスタンスの削除
            deleteFamilyInstances(wLeftInnerLiner);
        }
        else {
            LOG_PRINT("NOK : %w ", wLeftInnerLiner);
        }

        if (statusInnerR == PRO_TK_NO_ERROR) {
            deleteFamilyInstances(wRightInnerLiner);
        }
        else {
            LOG_PRINT("NOK : %w ", wRightInnerLiner);
        }
    }

    /***********************************************
     モデルツリーを更新する (チェックインに失敗した時のために実施)
    *************************************************/
     status = ProMdlDisplay(top_asm);
    int window_id2;
    status = ProMdlWindowGet(top_asm, &window_id2);
    status = ProWindowActivate(window_id2);

    /***********************************************
     チェックイン
    *************************************************/
    status = checkInObject(top_asm, windchillInfo);

    /***********************************************
     メモリ開放
    *************************************************/
    free(gstrParametersFeature);
    free(gstrParameters);
    free(gstrFeatures);
    free(gstrVehicleCoordinateSystems);
    free(gstrHoles);
    free(gstrHoleTable);
    free(gstrInnerlinerRename);
    free(gstrInnerliner);
    free(gstrInnerlinerParametersFeature);
    free(gstrInnerlinerParameters);
    free(gstrInnerlinerFeatures);
    free(gstrInnerlinerHoles);
    free(gstrInnerlinerHoleTable);
    free(gstrModulesRename);
    free(gstrModules);
    free(gstrFrontAxleRename);
    free(gstrFrontAxle);
    free(gstrFrontAxleParamsFeature);
    free(gstrFrontAxleParams);
    free(gstrCabRename);
    free(gstrCab);
    free(gstrEngineRename);
    free(gstrEngine);
    free(gstrGearboxCoordinateSystems);
    free(gstrGearboxRename);
    free(gstrGearbox);
    free(gstrPropShaftRename);
    free(gstrPropShaft);
    free(gstrPropShaftParamsFeature);
    free(gstrPropShaftParams);

    free(wOldTopInnerLiner);
    free(wNewTopInnerLiner);

    if (status == PRO_TK_NO_ERROR) {
        // 画面クリア
        int window_id;
        status = ProMdlWindowGet(top_asm, &window_id);
        status = ProWindowClear(window_id);

        // 非表示消去
        status = ProMdlEraseAll(top_asm);

        // 処理終了
        endProcessLog(windchillInfo, wTopAssyNewName, ENUM_MESSAGE_SUCCESS);

    }
    else {
        // 処理終了
        endProcessLog(windchillInfo, wTopAssyNewName, ENUM_MESSAGE_CHECKIN_FAILED);

    }
 
    
    return(PRO_TK_NO_ERROR);
}

/*====================================================================*\
FUNCTION : checkInitial
PURPOSE  : メイン処理を実施するための必須設定値を確認/取得をする
    int* iErrorCnt                 out     エラーのカウント用
    ProCharPath* cImpFolderPath    out     コンフィグレーションファイルのファイルパス
    ProPath wTopAssyName           out     Topアセンブリ名
    WindchillInfo windchillInfo 	out     WindchillServerの情報
    ProCharPath *cFeatureLsitPath   out     FeatureListのパス格納用

戻り値
    PRO_TK_NO_ERROR
    ※エラーがあった場合はiErrorCntでカウントするため、全て正常終了とする。

\*====================================================================*/
ProError  checkInitial(int* iErrorCnt, ProCharPath* cImpFolderPath, ProPath wTopAssyName, WindchillInfo *windchillInfo, ProCharPath* cFeatureLsitPath)
{
    ProError status;
    DWORD error;
    char cErrorReason[1024];
    memset(cErrorReason, 0, sizeof(cErrorReason));

    /***********************************************
    設定ファイル：ログファイルの生成確認
    *************************************************/
    ProPath     wUserStartAppTime;
    ProCharPath cUserStartAppTime;
    ProCharPath logFilePathTemp;
    int iRet = 0;
    struct stat statBuf;

    // 環境変数からログファイル名を取得
    getEnvCustom(ENV_USER_START_APP_TIME, wUserStartAppTime, &iRet);

    ProWstringConcatenate(L".log", wUserStartAppTime, PRO_VALUE_UNUSED);
    ProWstringToString(cUserStartAppTime, wUserStartAppTime);

    ProPath         wLogFilePath;
    ProCharPath     cLogFilePath;

    // 環境変数からログファイルパスを取得
    iRet = 0;
    getEnvCustom(ENV_LOG_FILE_PATH, wLogFilePath, &iRet);

    if (iRet == 1) {
        // パスが取得できなかった
        *iErrorCnt = *iErrorCnt + 1;

        strcpy_s(logFilePathTemp, sizeof(logFilePathTemp), SETTING_LOG_INIT_PATH);
        strcat_s(logFilePathTemp, sizeof(logFilePathTemp), cUserStartAppTime);
        sprintf(cErrorReason, "ATTENTION : Could not get LOG_FILE_PATH, so set the default path.");


    }else {
        ProWstringToString(cLogFilePath, wLogFilePath);

        // 取得したパスの存在確認
        if (stat(cLogFilePath, &statBuf) != 0) {

            // 取得したパスが存在しないため、デフォルトパスを設定します
            strcpy_s(logFilePathTemp, sizeof(logFilePathTemp), SETTING_LOG_INIT_PATH);
            strcat_s(logFilePathTemp, sizeof(logFilePathTemp), cUserStartAppTime);
            sprintf(cErrorReason, "ATTENTION : LOG_FILE_PATH not exists, so set the default path.");

        }
        else {
            // 取得したパスを設定します
            strcpy_s(logFilePathTemp, sizeof(logFilePathTemp), cLogFilePath);
            strcat_s(logFilePathTemp, sizeof(logFilePathTemp), cUserStartAppTime);
        }
    }
    // ログファイルパスの設定
    strcpy_s(gcLogFilePath, sizeof(gcLogFilePath), logFilePathTemp);

    TRAIL_PRINT("%s(%d) : OK : log file path = %s", __func__, __LINE__, logFilePathTemp);

    /***********************************************
        ログ
    *************************************************/
    LOG_PRINT("---------------------------------------------------");
    LOG_PRINT("   %s %s %s", VERSION_INFO_MODE, VERSION_INFO_VER, VERSION_INFO_DATE);
    LOG_PRINT("---------------------------------------------------");
    LOG_PRINT("   Confirm initial settings");
    LOG_PRINT("---------------------------------------------------");
    
    if (strcmp(cErrorReason, "") != NULL) {
        LOG_PRINT("%s", cErrorReason);
    }

    LOG_PRINT("OK  : LogFilePath=%s", gcLogFilePath);

    /***********************************************
     設定ファイル：コンフィグレーションファイルの読み込み
    *************************************************/
    ProPath wConfigrationFilePath;
    ProCharPath cConfigrationFilePath;
    ProPath InOutDir;
    ProPath userStartAppTime;
    ProPath configrationFile;

    getEnvCustomWithLog(ENV_IN_OUT_DIR, InOutDir, iErrorCnt);
    getEnvCustomWithLog(ENV_USER_START_APP_TIME, userStartAppTime, iErrorCnt);
    getEnvCustomWithLog(ENV_CONFIGRATION_FILE, configrationFile, iErrorCnt);

    // コンフィグレーションファイルパスを取得
    ProWstringCopy(L"", wConfigrationFilePath, PRO_VALUE_UNUSED);
    ProWstringConcatenate(InOutDir, wConfigrationFilePath, PRO_VALUE_UNUSED);
    ProWstringConcatenate(L"/", wConfigrationFilePath, PRO_VALUE_UNUSED);
    ProWstringConcatenate(userStartAppTime, wConfigrationFilePath, PRO_VALUE_UNUSED);
    ProWstringConcatenate(L"/", wConfigrationFilePath, PRO_VALUE_UNUSED);
    ProWstringConcatenate(configrationFile, wConfigrationFilePath, PRO_VALUE_UNUSED);
    // できたコンフィグレーションファイルパスを charに変換しなおす
    ProWstringToString(cConfigrationFilePath, wConfigrationFilePath);

    if (strlen(cConfigrationFilePath) >= 259) {
        // ファイル最大長エラー
        LOG_PRINT("NOK : Configration file path is too long.");
        *iErrorCnt = *iErrorCnt + 1;
    }
    else {
        FILE* fp;
        errno_t error = fopen_s(&fp, cConfigrationFilePath, "r");

        if (error != NULL || fp == NULL) {
            // コンフィグレーションファイルが存在しません
            LOG_PRINT("NOK : Configration file not exists");
            *iErrorCnt = *iErrorCnt + 1;
        }
        else {
            // コンフィグレーションファイルが存在する
            strcpy_s(*cImpFolderPath, sizeof(*cImpFolderPath), cConfigrationFilePath);
            LOG_PRINT("OK  : Configration file path = %s", cConfigrationFilePath);

        }
        if (fp != NULL) {
            fclose(fp);

        }
    }

    /***********************************************
     環境変数：フレームテンプレートの確認
    *************************************************/
    ProPath FrameTemlate;

    getEnvCustomWithLog(ENV_FRAME_TEMPLATE, FrameTemlate, iErrorCnt);
    ProWstringCopy(FrameTemlate, wTopAssyName, PRO_VALUE_UNUSED);

    /***********************************************
     Windchill接続確認
    *************************************************/
    wchar_t* actWorkspace;

    // 環境変数：windchillの 情報 の取得
    getEnvCustomWithLog(ENV_WINDCHILL_WORKSPACE, windchillInfo->settingIniWorkspace, iErrorCnt);
    getEnvCustomWithLog(ENV_WINDCHILL_CONTEXT, windchillInfo->settingIniContext, iErrorCnt);
    getEnvCustomWithLog(ENV_WINDCHILL_FOLDER, windchillInfo->settingIniFolder, iErrorCnt);
    getEnvCustomWithLog(ENV_WINDCHILL_USERID, windchillInfo->settingIniUserID, iErrorCnt);
    getEnvCustomWithLog(ENV_WINDCHILL_PASSWORD, windchillInfo->settingIniPasswords, iErrorCnt);
    getEnvCustomWithLog(ENV_WINDCHILL_SERVER, windchillInfo->settingIniServer, iErrorCnt);
    getEnvCustomWithLog(ENV_WINDCHILL_URL, windchillInfo->settingIniUrl, iErrorCnt);

    // Windchill との接続を確認する
    status = setWindchillOnline(*windchillInfo);

    if (status != PRO_TK_NO_ERROR)
    {
        // Windchillに接続できませんでした。
        LOG_PRINT("NOK : Could not connect to Windchill Server");
        *iErrorCnt = *iErrorCnt + 1;
    }
    else {

        // Windchillのワークスペース取得
        status = ProServerWorkspaceGet(windchillInfo->settingIniServer, &actWorkspace);
        TRAIL_PRINT("%s(%d) : ProServerWorkspaceGet = %s / actWorkspace = %w", __func__, __LINE__, getProErrorMessage(status), actWorkspace);

        ProWstringConcatenate(actWorkspace, windchillInfo->defaultWorkspace, PRO_VALUE_UNUSED);

        int result;
        ProWstringCompare(windchillInfo->defaultWorkspace, windchillInfo->settingIniWorkspace, PRO_VALUE_UNUSED, &result);
        if (result != NULL) {
            // 既にセットされているワークスペースの場合エラーとなるので、
            // 現状と比較し、異なる場合のみワークスペースを変更する
            status = ProServerWorkspaceSet(windchillInfo->settingIniServer, windchillInfo->settingIniWorkspace);
            TRAIL_PRINT("%s(%d) : ProServerWorkspaceSet = %s", __func__, __LINE__, getProErrorMessage(status));
        }

        if (status != PRO_TK_NO_ERROR)
        {
            // Windchillのワークスペースを取得できませんでした。
            LOG_PRINT("NOK : Could not get to Windchill Workspace");
            *iErrorCnt = *iErrorCnt + 1;
        }else {
            /***********************************************
             Windchill ワークスペースの削除
            *************************************************/
            ProServerRemoveConflicts conflicts = NULL;

            // ワークスペースからモデルを削除する
            status = ProServerObjectsRemove(NULL, &conflicts);
            TRAIL_PRINT("%s(%d) : ProServerObjectsRemove = %s", __func__, __LINE__, getProErrorMessage(status));
            if (status != PRO_TK_NO_ERROR && conflicts != NULL)
            {
                ProServerconflictsFree(conflicts);
                LOG_PRINT("NOK : Failed to delete workspace");
                *iErrorCnt = *iErrorCnt + 1;
            }
        }
    }

    /***********************************************
        チェックインフォルダの存在確認
    *************************************************/
    ProPath wCheckinFolder;
    ProCharPath cCheckinFolder;
    struct stat stat_buf;
    int ret;

    ProStringToWstring(wCheckinFolder, "wtpub://");
    ProWstringConcatenate(windchillInfo->settingIniServer, wCheckinFolder, PRO_VALUE_UNUSED);
    ProWstringConcatenate(L"/Libraries/", wCheckinFolder, PRO_VALUE_UNUSED);
    //ProWstringConcatenate(L"/Produsts/", wCheckinFolder, PRO_VALUE_UNUSED);
    ProWstringConcatenate(windchillInfo->settingIniContext, wCheckinFolder, PRO_VALUE_UNUSED);
    ProWstringConcatenate(L"/", wCheckinFolder, PRO_VALUE_UNUSED);
    ProWstringConcatenate(windchillInfo->settingIniFolder, wCheckinFolder, PRO_VALUE_UNUSED);
    ProWstringConcatenate(L"/", wCheckinFolder, PRO_VALUE_UNUSED);

    ProPath *file_list, *dir_list;
    ProArrayAlloc(0, sizeof(ProPath), 1, (ProArray*)&file_list);
    ProArrayAlloc(0, sizeof(ProPath), 1, (ProArray*)&dir_list);
    status = ProFilesList(wCheckinFolder, L"*.*", PRO_FILE_LIST_ALL, &file_list, &dir_list);
    TRAIL_PRINT("%s(%d) : ProFilesList = %s", __func__, __LINE__, getProErrorMessage(status));


    if (status == PRO_TK_NO_ERROR) {
        LOG_PRINT("OK  : Check-in location = %w", wCheckinFolder);

    }
    else {
        LOG_PRINT("NOK : %w : Check-in location does not exist", wCheckinFolder);
        *iErrorCnt = *iErrorCnt + 1;
    }

    status = ProArrayFree((ProArray*)&file_list);
    TRAIL_PRINT("%s(%d) : ProArrayFree = %s", __func__, __LINE__, getProErrorMessage(status));

    status = ProArrayFree((ProArray*)&dir_list);
    TRAIL_PRINT("%s(%d) : ProArrayFree = %s", __func__, __LINE__, getProErrorMessage(status));


    /***********************************************
        FeatureListファイル：Feature が Frame Template に無い場合でもログにNOKと表示させない
    *************************************************/
    ProPath wFeatureLsitPath;
    int iError = 0;
    getEnvCustomWithLog(ENV_FRAME_FEATURE_LOG_OK_LIST, wFeatureLsitPath, &iError);

    if(iError == 0) {

        FILE* fp;
        ProCharPath temppath;

        ProWstringToString(temppath, wFeatureLsitPath);
        errno_t error = fopen_s(&fp, temppath, "r");

        if (error != NULL || fp == NULL) {
            // ファイルが存在しません
            LOG_PRINT("NOK : SETTING_FRAME_FEATURE_LOG_OK_LIST not exists");
            *iErrorCnt = *iErrorCnt + 1;
        }
        else {
            // ファイルが存在する
            strcpy_s(*cFeatureLsitPath, sizeof(*cFeatureLsitPath), temppath);
            LOG_PRINT("OK  : SETTING_FRAME_FEATURE_LOG_OK_LIST = %s", temppath);

        }
        if (fp != NULL) {
            fclose(fp);

        }
    }
    else {
        *iErrorCnt = *iErrorCnt + 1;
    }

    return PRO_TK_NO_ERROR;
}

/*====================================================================*\
FUNCTION : endProcessLog
PURPOSE  : フレームテンプレートの名前を取得する

\*====================================================================*/
ProError  endProcessLog(WindchillInfo windchillInfo, ProMdlName wTopAssyNewName, int iMessageFlag)
{

    ProError status = PRO_TK_NO_ERROR;

    LOG_PRINT("---------------------------------------------------");
    LOG_PRINT("   End processing");
    LOG_PRINT("---------------------------------------------------");
    LOG_PRINT("MessageFlag : %d", iMessageFlag);

    if (iMessageFlag == ENUM_MESSAGE_SUCCESS) {
        LOG_PRINT("TopAssyName : %w.asm", wTopAssyNewName);
        /***********************************************
         TopアセンブリのWindchillURLの取得
        *************************************************/
        ProPath wCheckinFolder;
        wchar_t* server;
        wchar_t* url = NULL;
        ProMdlName wTopAssyNewNameKomoji;

        // サーバーの設定
        status = ProServerActiveGet(&server);
        ProStringToWstring(wCheckinFolder, "wtpub://");
        ProWstringConcatenate(server, wCheckinFolder, PRO_VALUE_UNUSED);
        ProWstringConcatenate(L"/Libraries/", wCheckinFolder, PRO_VALUE_UNUSED);
        ProWstringConcatenate(windchillInfo.settingIniContext, wCheckinFolder, PRO_VALUE_UNUSED);
        ProWstringConcatenate(L"/", wCheckinFolder, PRO_VALUE_UNUSED);
        ProWstringConcatenate(windchillInfo.settingIniFolder, wCheckinFolder, PRO_VALUE_UNUSED);
        ProWstringConcatenate(L"/", wCheckinFolder, PRO_VALUE_UNUSED);

        status = ProServerAliasedURLToURL(wCheckinFolder, &url);
        TRAIL_PRINT("%s(%d) : ProServerAliasedURLToURL = %s / wCheckinFolder = %w", __func__, __LINE__, getProErrorMessage(status), wCheckinFolder);

        if (status != PRO_TK_NO_ERROR) {
            LOG_PRINT("URL : Error!! (%s)", getProErrorMessage(status));
        }
        else {
            LOG_PRINT("URL : %w", url);
        }

        status = ProWstringFree(url);

    }
    else {
        LOG_PRINT("TopAssyName : No data.");
        LOG_PRINT("URL : No data.");

    }

    /***********************************************
     アプリケーションを終了させる
    *************************************************/
    TRAIL_PRINT("%s(%d) : === END ===", __func__, __LINE__);
    //exit(0);
    status = ProEngineerEnd();

}

/*====================================================================*\
FUNCTION : checkDuplicate
PURPOSE  : トップアセンブリの重複確認を行う。
備考 :     トップアセンブリをロードするのは時間がかかるので、パーツで確認する。
char* cGetValue     (in/out)    トップアセンブリの連番部
\*====================================================================*/
ProError checkDuplicate(char* cGetValue) {


    ProError status;
    ProPath wGetValue;
    ProMdlName wNewName;

    while (TRUE) {
        // Value+1 を設定する
        int iGetValue = atoi(cGetValue);
        iGetValue++;
        sprintf(cGetValue, "%03d", iGetValue);
        ProStringToWstring(wGetValue, cGetValue);

        // パーツ名の生成
        //wNewName = wTopAssyName + cKeyCnt;
        ProWstringCopy(gwFramename, wNewName, PRO_VALUE_UNUSED);
        ProWstringConcatenate(L"_", wNewName, PRO_VALUE_UNUSED);
        ProWstringConcatenate(wGetValue, wNewName, PRO_VALUE_UNUSED);
        ProWstringConcatenate(L"_1", wNewName, PRO_VALUE_UNUSED);

        ProMdl		 inst_model;
        status = ProMdlnameRetrieve(wNewName, (ProMdlfileType)PRO_MDLFILE_PART, &inst_model);
        TRAIL_PRINT("%s(%d) : ProMdlnameRetrieve = %s", __func__, __LINE__, getProErrorMessage(status));

        if (status != PRO_TK_NO_ERROR) {
            // 指定パーツがロードできない場合は、重複の心配なし
            return;
        }
        else {
            // 画面クリア
            int window_id;
            status = ProMdlWindowGet(&inst_model, &window_id);
            status = ProWindowClear(window_id);

            // 非表示消去
            status = ProMdlEraseAll(inst_model);
        }
    }

}


/*====================================================================*\
FUNCTION : checkNewFrameName
PURPOSE  : フレームテンプレートの名前の重複を確認する
            カウントを+1して、本処理で使用するカウントを取得する
備考 :
  (IDENTITY:)_(Signature)　or (PROTOM SPEC ID: or OM NO.:or FO NO.:)}_(連番3桁)

ProMdlName* wNameNumber    out     本処理で使用する連番3桁
\*====================================================================*/
ProError  checkNewFrameName(ProMdlName* wNameNumber, int* iErrorCnt)
{

    ProCharPath cGetValue;
    ProPath wGetValue;
    int iResult;
    ProBool Loop = PRO_B_TRUE;
    ProError status;
    ProMdlName wNewName;

    ProPath     wSerialNumberPath;
    ProCharPath cSerialNumberPath;
    int     iError = 0;
    int     iKeyCnt = 1;
    char    cKeyCnt[4];

    // コンフィグレーションファイル から取得するTopアセンブリ名(gwFramename)
    ProCharPath cPrefix;
    ProWstringToString(cPrefix, gwFramename);

    ProCharPath cPrefixKey;   // PREFIX000
    ProCharPath cValueKey;    // NUMBER000

    getEnvCustomWithLog(ENV_SERIAL_NUMBER, wSerialNumberPath, &iError);
    if (iError == 0) {
        ProWstringToString(cSerialNumberPath, wSerialNumberPath);

        while (Loop) {
            // カウントを文字列にし、KEY名を作成
            sprintf(cKeyCnt, "%03d", iKeyCnt);
            strcpy(cPrefixKey, SETTING_PREFIX_KEY);
            strcat(cPrefixKey, cKeyCnt);

            strcpy(cValueKey, SETTING_PREFIX_VALUE);
            strcat(cValueKey, cKeyCnt);

            /***********************************************
             Signatureの値を確認する
            *************************************************/
            DWORD error = GetPrivateProfileString(SETTING_SERIAL_NUMBER_SECTION, cPrefixKey, NULL, cGetValue, sizeof(cGetValue), cSerialNumberPath);

            if (error == NULL) {

                // 求めたい値
                strcpy(cGetValue, "000");

                // 重複確認を行う
                checkDuplicate(cGetValue);

                // SignatureKEY が存在しない場合は、新しくSignature/Valueを追加する
                WritePrivateProfileString(SETTING_SERIAL_NUMBER_SECTION, cPrefixKey, cPrefix, cSerialNumberPath);
                WritePrivateProfileString(SETTING_SERIAL_NUMBER_SECTION, cValueKey, cGetValue, cSerialNumberPath);
                break;
            }
            else {
                // Signatureが存在する場合は、取得したSignatureの値を確認する
                if (strcmp(cPrefix, cGetValue) == 0) {
                    // 設定ファイルの値と一致した場合は、Valueを確認する
                    DWORD error = GetPrivateProfileString(SETTING_SERIAL_NUMBER_SECTION, cValueKey, NULL, cGetValue, sizeof(cGetValue), cSerialNumberPath);

                    // 重複確認を行う
                    checkDuplicate(cGetValue);
                    // 設定ファイルを設定しなおす
                    WritePrivateProfileString(SETTING_SERIAL_NUMBER_SECTION, cValueKey, cGetValue, cSerialNumberPath);
                    break;
                }
                else {
                    // 値が一致しない場合はループを続ける
                    iKeyCnt++;
                }
            }
        }
        ProStringToWstring(wGetValue, cGetValue);

        ProWstringCompare(wGetValue, L"", PRO_VALUE_UNUSED, &iError);
        if (iError == 0) {
            *iErrorCnt = *iErrorCnt + 1;
            LOG_PRINT("NOK : Top assembly duplicate check failure.");

        }
        else {
            ProWstringCopy(gwFramename, wNewName, PRO_VALUE_UNUSED);
            ProWstringConcatenate(L"_", wNewName, PRO_VALUE_UNUSED);
            ProWstringConcatenate(wGetValue, wNewName, PRO_VALUE_UNUSED);

            ProWstringConcatenate(wGetValue, *wNameNumber, PRO_VALUE_UNUSED);
            LOG_PRINT("OK  : Check for duplicate names : %w", wNewName);

        }

    }
    else {
        *iErrorCnt = *iErrorCnt + 1;
    }

}


/*====================================================================*\
FUNCTION : renamePart
PURPOSE  : 左右のパーツ名を変更する
ProPath wOldPart        in  旧オブジェクト名
ProPath wNewPart        in  新オブジェクト名
ProPath wLeftRight      in  左右判別( L"_1" , L"_2")

\*====================================================================*/
ProError  renamePart(ProPath wOldPart, ProPath wNewPart, ProPath wLeftRight)
{

    ProError status;
    ProPath wOldPartName;
    ProPath wNewPartName;

    // 旧パーツ名の作成
    ProWstringCopy(wOldPart, wOldPartName, PRO_VALUE_UNUSED);
    ProWstringConcatenate(wLeftRight, wOldPartName, PRO_VALUE_UNUSED);

    // 新パーツ名の作成
    ProWstringCopy(wNewPart, wNewPartName, PRO_VALUE_UNUSED);
    ProWstringConcatenate(wLeftRight, wNewPartName, PRO_VALUE_UNUSED);

    // rename処理
    status = renameObject(wOldPartName, wNewPartName, PRO_MDLFILE_PART);

    return status;

}

/*===========================================================================*\
 Function:      deleteFamilyInstances
 Purpose:       ファミリーテーブルのインスタンスを削除する
 ※リネームしたパーツにファミリーテーブルがある場合、windchill登録時にコンフリクトエラーとなった
 　サイドフレームのファミリーテーブルは削除することとする
\*===========================================================================*/
ProError deleteFamilyInstances(ProPath wPartName)
{
    ProError            status;
    ProMdl mdlFamTable, mdlInstance;
    ProFamtable famtable;

    // ハンドルの初期化
    status = ProMdlInit(wPartName, PRO_MDL_PART, &mdlFamTable);
    status = ProFamtableInit(mdlFamTable, &famtable);
    // ファミリーテーブル削除
    status = ProFamtableErase(&famtable);

    if (status == PRO_TK_NO_ERROR) {
        LOG_PRINT("OK  : %w ", wPartName);
    }
    else {
        LOG_PRINT("NOK : %w ", wPartName);
    }

    return PRO_TK_NO_ERROR;
}

/*====================================================================*\
FUNCTION : renameFrontAxle
PURPOSE  : オブジェクトの名前を変更する
           FrontAxleに限り、Moduleで追加するアセンブリ名に　接頭辞(ue84fr_bridebb_fa) を追加する


InputFileModules*   strModules          in  コンフィグレーションファイルの値
int                 iSectionMaxRows     in  処理すべきコンフィグレーションファイルの行数
ProPath             wPrefix             in  接頭辞  

\*====================================================================*/
ProError  renameFrontAxle(InputFileModules* strModules, int iSectionMaxRows, ProMdlName wTopAssyNewName, ProMdlName wType)
{

    ProError status;
    ProMdlName wPrefix;
    wchar_t wAfterName[INPUTFILE_MAXLINE];         // Rename後の名前
    wchar_t wBeforeName[INPUTFILE_MAXLINE];        // Rename前の名前

    /***********************************************
     Module追加したものにwPrefixを追加する
    *************************************************/

    for (int iInputMdlCnt = 0; iInputMdlCnt < iSectionMaxRows; iInputMdlCnt++) {

        // 変換前の名前
        ProStringToWstring(wBeforeName, strModules->cModules);

        // 変換後の名前
        ProWstringCopy(wTopAssyNewName, wAfterName, PRO_VALUE_UNUSED);
        ProWstringConcatenate(L"_", wAfterName, PRO_VALUE_UNUSED);
        ProWstringConcatenate(wType, wAfterName, PRO_VALUE_UNUSED);
        ProWstringConcatenate(L"_", wAfterName, PRO_VALUE_UNUSED);
        ProWstringConcatenate(wBeforeName, wAfterName, PRO_VALUE_UNUSED);

        // 名前の変換
        status = renameObject(wBeforeName, wAfterName, PRO_MDLFILE_ASSEMBLY);
        strModules++;
    }
}

/*====================================================================*\
FUNCTION : checkInObject
PURPOSE  : windchillにチェックインする
 ProMdl top_asm
 備考
 ・Windchillの接続確認
 ・iniファイルの確認
\*====================================================================*/
ProError  checkInObject(ProMdl top_asm, WindchillInfo windchillInfo)
{
    ProError status = PRO_TK_NO_ERROR;
   

    LOG_PRINT("Check-in Objects");

    // 1. ビューにモデルを表示する
    status = ProMdlDisplay(top_asm);


    // 2. モデルをワークスペースに保存
    status = ProMdlSave(top_asm);
    if (status == PRO_TK_NO_ERROR)
    {
        LOG_PRINT("OK  : Successfully saved to workspace");
    }
    else {
        LOG_PRINT("NOK : Failed to save to workspace(%s)",getProErrorMessage(status));
        return PRO_TK_GENERAL_ERROR;
    }

    // 3. Windchillへチェックイン
    ProServerCheckinConflicts   conflicts;
    ProServerCheckinOptions     options;
    ProPath wCheckinFolder;
    wchar_t* server;

    // サーバーの設定
    status = ProServerActiveGet(&server);

    ProStringToWstring(wCheckinFolder, "wtpub://");
    ProWstringConcatenate(server, wCheckinFolder, PRO_VALUE_UNUSED);
    ProWstringConcatenate(L"/Libraries/", wCheckinFolder, PRO_VALUE_UNUSED);
    ProWstringConcatenate(windchillInfo.settingIniContext, wCheckinFolder, PRO_VALUE_UNUSED);
    ProWstringConcatenate(L"/", wCheckinFolder, PRO_VALUE_UNUSED);
    ProWstringConcatenate(windchillInfo.settingIniFolder, wCheckinFolder, PRO_VALUE_UNUSED);
    ProWstringConcatenate(L"/", wCheckinFolder, PRO_VALUE_UNUSED);

    status = ProServercheckinoptsAlloc(&options);
    // 欠落している参照を完全に無視することにより、それらを自動解決します
    status = ProServercheckinoptsAutoresolveSet(options, PRO_SERVER_AUTORESOLVE_IGNORE);
    status = ProServercheckinoptsDeflocationSet(options, wCheckinFolder);
    status = ProServerObjectsCheckin(top_asm, options, &conflicts);
    if (status == PRO_TK_CHECKOUT_CONFLICT)
    {
        // コンフリクトをした時の処理
        wchar_t* description;
        ProServerconflictsDescriptionGet(conflicts, &description);
        // LOG_PRINT内にコンフリクト理由 ( description ) を追加しないのは、文字化けをするから

        LOG_PRINT("NOK : failed Check-in");

        status = ProTrailfileCommentWrite(L"CUSTOM_LOG conflicts = ");
        status = ProTrailfileCommentWrite(description);

        ProWstringFree(description);
        ProServerconflictsFree(conflicts);
        ProServercheckinoptsFree(options);
        return PRO_TK_GENERAL_ERROR;
    }
    else {
        LOG_PRINT("OK  : Check-in was successful ");
        ProServercheckinoptsFree(options);
        return PRO_TK_NO_ERROR;
    }
    
}

/*=========================================================================*\
    Function:	renameSameTopAssyName
    Purpose:	Moduleセクション (rename部)
    InputFileRenameFile* strModulesRename   (in)    コンフィグレーションファイルの値
    int iSectionMaxRows                     (in)    処理すべきコンフィグレーションファイルの行数
    ProMdlName wTopAssyNewName              (in)    Topアセンブリの名前
    ProMdlName wType                        (in)    renameファイルの種類(FA/CAB/ENG/GB/FIL/PS)
\*=========================================================================*/
ProError renameSameTopAssyName(InputFileRenameFile* strModulesRename, int iSectionMaxRows, ProMdlName wTopAssyNewName, ProMdlName wType) 
{

    ProError status;
    wchar_t wAfterName[INPUTFILE_MAXLINE];         // Rename後の名前
    wchar_t wBeforeName[INPUTFILE_MAXLINE];        // Rename前の名前
    ProMdl mdlRenameComp;
    ProMdlfileType type;

    for (int iInputMdlCnt = 0; iInputMdlCnt < iSectionMaxRows; iInputMdlCnt++) {
        /*
        *  同名ファイルは2回以降Widchillへチェックインできないため
        *　Topアセンブリと同様に連番を付与する.
        *  コンフィグレーションファイルのAfterNameは使用しない
        */

        ProWstringCopy(wTopAssyNewName, wAfterName, PRO_VALUE_UNUSED);
        ProWstringConcatenate(L"_", wAfterName, PRO_VALUE_UNUSED);
        ProWstringConcatenate(wType, wAfterName, PRO_VALUE_UNUSED);

        Split(strModulesRename->cAfterName, L"", wBeforeName);

        if (strstr(strModulesRename->cBeforeName, ".asm") != NULL) {
            type = PRO_MDLFILE_ASSEMBLY;

            // 末尾が_(数字).(拡張子)の場合の対応。とりあえず9まで対応
            for (int iNum = 1; iNum < 10; iNum++) {
                char num[100];
                char cExt[100];
                wchar_t wExt[100];
                sprintf(num, "_%d.asm", iNum);
                sprintf(cExt, "_%d", iNum);
                ProStringToWstring(wExt, cExt);
                if (strstr(strModulesRename->cBeforeName, num) != NULL) {
                    ProWstringConcatenate(wExt, wAfterName, PRO_VALUE_UNUSED);
                    break;
                }
            }
        }
        else if (strstr(strModulesRename->cBeforeName, ".lay") != NULL) {
            type = PRO_MDLFILE_NOTEBOOK;

            // 末尾が_(数字).(拡張子)の場合の対応。とりあえず9まで対応
            for (int iNum = 1; iNum < 10; iNum++) {
                char num[100];
                char cExt[100];
                wchar_t wExt[100];
                sprintf(num, "_%d.lay", iNum);
                sprintf(cExt, "_%d", iNum);
                ProStringToWstring(wExt, cExt);
                if (strstr(strModulesRename->cBeforeName, num) != NULL) {
                    ProWstringConcatenate(wExt, wAfterName, PRO_VALUE_UNUSED);
                    break;
                }
            }
        }
        else if (strstr(strModulesRename->cBeforeName, ".prt") != NULL) {
            type = PRO_MDLFILE_PART;

            // 末尾が_(数字).(拡張子)の場合の対応。とりあえず9まで対応
            for (int iNum = 1; iNum < 10; iNum++) {
                char num[100];
                char cExt[100];
                wchar_t wExt[100];
                sprintf(num, "_%d.prt", iNum);
                sprintf(cExt, "_%d", iNum);
                ProStringToWstring(wExt, cExt);
                if (strstr(strModulesRename->cBeforeName, num) != NULL) {
                    ProWstringConcatenate(wExt, wAfterName, PRO_VALUE_UNUSED);
                    break;
                }
            }
        }
        else {
            type = PRO_MDLFILE_UNUSED;
        }

        // 名前を変更
        status = renameObject(wBeforeName, wAfterName, type);

        strModulesRename++;
    }

}

