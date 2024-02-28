/*
* InputFile.h
*/
#pragma once

/*-----------------------------------------------------------------*\
    マクロ
\*-----------------------------------------------------------------*/
//inputfileの最大文字数
#define INPUTFILE_MAXLINE 100

#define MAX_LINE 512
#define MAX_TOP_ASSY_LENGTH 15

/***********************************************
InputFile
    1セクション内が2部構成のセクションについて、
    下手な単語を定義してヒットしたら嫌なので
    同じ単語を定義している
*************************************************/
#define INPUT_SIGNATURE                             "SIGNATURE:"
#define INPUT_IDENTITY                              "IDENTITY:"
#define INPUT_PROTOM_SPEC_ID                        "PROTOM SPEC ID:"
#define INPUT_OM_NO                                 "OM NO.:"
#define INPUT_FO_NO                                 "FO NO.:"
#define INPUT_SECTION_PARAMETERS_FEATURE            "*PARAMETERS"
#define INPUT_SECTION_PARAMETERS                    "*PARAMETERS"
#define INPUT_SECTION_FEATURES                      "*FEATURES"
#define INPUT_SECTION_VEHICLE_COORDINATE_SYSTEMS    "*VEHICLE_COORDINATE_SYSTEMS"
#define INPUT_SECTION_PARTS                         "*PARTS"
#define INPUT_SECTION_HOLES                         "*HOLES"
#define INPUT_SECTION_HOLE_TABLE                    "*HOLE_TABLE"
#define INPUT_SECTION_INNERLINER_RENAME             "*INNERLINER"
#define INPUT_SECTION_INNERLINER                    "*INNERLINER"
#define INPUT_SECTION_INNERLINER_PARAMETERS_FEATURE "*INNERLINER_PARAMETERS"
#define INPUT_SECTION_INNERLINER_PARAMETERS         "*INNERLINER_PARAMETERS"
#define INPUT_SECTION_INNERLINER_FEATURES           "*INNERLINER_FEATURES"
#define INPUT_SECTION_INNERLINER_HOLES              "*INNERLINER_HOLES"
#define INPUT_SECTION_INNERLINER_HOLE_TABLE         "*INNERLINER_HOLE_TABLE"
#define INPUT_SECTION_MODULES_RENAME                "*MODULES"
#define INPUT_SECTION_MODULES                       "*MODULES"
#define INPUT_SECTION_FRONT_AXLE_RENAME             "*FRONT_AXLE"
#define INPUT_SECTION_FRONT_AXLE                    "*FRONT_AXLE"
#define INPUT_SECTION_FRONT_AXLE_PARAMS_FEATURE     "*FRONT_AXLE_PARAMS"
#define INPUT_SECTION_FRONT_AXLE_PARAMS             "*FRONT_AXLE_PARAMS"
#define INPUT_SECTION_CAB_RENAME                    "*CAB"
#define INPUT_SECTION_CAB                           "*CAB"
#define INPUT_SECTION_ENGINE_RENAME                 "*ENGINE"
#define INPUT_SECTION_ENGINE                        "*ENGINE"
#define INPUT_SECTION_GEARBOX_COORDINATE_SYSTEMS    "*GEARBOX_COORDINATE_SYSTEMS"
#define INPUT_SECTION_GEARBOX_RENAME                "*GEARBOX"
#define INPUT_SECTION_GEARBOX                       "*GEARBOX"
#define INPUT_SECTION_PROP_SHAFT_RENAME             "*PROP_SHAFT"
#define INPUT_SECTION_PROP_SHAFT                    "*PROP_SHAFT"
#define INPUT_SECTION_PROP_SHAFT_PARAMS             "*PROP_SHAFT_PARAMS"
#define INPUT_SECTION_PROP_SHAFT_PARAMS_FEATURE     "*PROP_SHAFT_PARAMS"
#define INPUT_SEPARATION                            " "

enum section {
    ENUM_INPUT_SIGNATURE,                               // 0
    ENUM_INPUT_IDENTITY,                                // 1
    ENUM_INPUT_PROTOM_SPEC_ID,                          // 2
    ENUM_INPUT_OM_NO,                                   // 3
    ENUM_INPUT_FO_NO,                                   // 4
    ENUM_INPUT_SECTION_PARAMETERS_FEATURE,              // 5  *PARAMETERSのFeature部
    ENUM_INPUT_SECTION_PARAMETERS,                      // 6  *PARAMETERSのParameter部
    ENUM_INPUT_SECTION_FEATURES,                        // 7
    ENUM_INPUT_SECTION_VEHICLE_COORDINATE_SYSTEMS,      // 8
    ENUM_INPUT_SECTION_PARTS,                           // 9
    ENUM_INPUT_SECTION_HOLES,                           // 10
    ENUM_INPUT_SECTION_HOLE_TABLE,                      // 11
    ENUM_INPUT_SECTION_INNERLINER_RENAME,               // 12   *INNERLINERのRename部
    ENUM_INPUT_SECTION_INNERLINER,                      // 13   *INNERLINERのModules部
    ENUM_INPUT_SECTION_INNERLINER_PARAMETERS_FEATURE,   // 14  *INNERLINER_PARAMETERSのFeature部
    ENUM_INPUT_SECTION_INNERLINER_PARAMETERS,           // 15  *INNERLINER_PARAMETERSのParameter部
    ENUM_INPUT_SECTION_INNERLINER_FEATURES,             // 16
    ENUM_INPUT_SECTION_INNERLINER_HOLES,                // 17
    ENUM_INPUT_SECTION_INNERLINER_HOLE_TABLE,           // 18
    ENUM_INPUT_SECTION_MODULES_RENAME,                  // 19  *MODULESのRename部
    ENUM_INPUT_SECTION_MODULES,                         // 20  *MODULESのModules部
    ENUM_INPUT_SECTION_FRONT_AXLE_RENAME,               // 21  *FRONT_AXLEのRename部
    ENUM_INPUT_SECTION_FRONT_AXLE,                      // 22  *FRONT_AXLEのModules部
    ENUM_INPUT_SECTION_FRONT_AXLE_PARAMS_FEATURE,       // 23  *FRONT_AXLEのFeature部
    ENUM_INPUT_SECTION_FRONT_AXLE_PARAMS,               // 24  *FRONT_AXLEのParameter部
    ENUM_INPUT_SECTION_CAB_RENAME,                      // 25  *CABのRename部
    ENUM_INPUT_SECTION_CAB,                             // 26  *CABのModules部
    ENUM_INPUT_SECTION_ENGINE_RENAME,                   // 27  *ENGINEのRename部
    ENUM_INPUT_SECTION_ENGINE,                          // 28  *ENGINEのModules部
    ENUM_INPUT_SECTION_GEARBOX_COORDINATE_SYSTEMS,      // 29
    ENUM_INPUT_SECTION_GEARBOX_RENAME,                  // 30  *GEARBOXのRename部
    ENUM_INPUT_SECTION_GEARBOX,                         // 31  *GEARBOXのModules部
    ENUM_INPUT_SECTION_PROP_SHAFT_RENAME,               // 32  *PROP_SHAFTのRename部
    ENUM_INPUT_SECTION_PROP_SHAFT,                      // 33  *PROP_SHAFTのModules部
    ENUM_INPUT_SECTION_PROP_SHAFT_PARAMS_FEATURE,       // 34  *PROP_SHAFT_PARAMSのFeature部
    ENUM_INPUT_SECTION_PROP_SHAFT_PARAMS,               // 35  *PROP_SHAFT_PARAMSのParameter部
    ENUM_INPUT_SECTION_MAX                              // 36  Sectionの最大値
};


/*-----------------------------------------------------------------*\
    構造体
\*-----------------------------------------------------------------*/
// *PARAMETERS 系統 - Feature部
typedef struct
{
    char cFeature[INPUTFILE_MAXLINE];           // パラメータ変更対象
}InputFileParamFeature;

// *PARAMETERS 系統 - Parameter部
typedef struct
{
    char cParameteValue[INPUTFILE_MAXLINE];     // パラメータ値
    char cParameterName[INPUTFILE_MAXLINE];     // パラメータ名
}InputFileParameters;

// *FEATURES 系統
typedef struct
{
    char cValue[INPUTFILE_MAXLINE];             // 対象部品名 (例 22680087)
    char cFeatureName[INPUTFILE_MAXLINE];       // Feature名  (例 L.H. RAIL;INNERLINER START)
}InputFileFeature;

// *COORDINATE_SYSTEMS 系統
typedef struct
{
    char cInstanceParameter[INPUTFILE_MAXLINE]; // インスタンスパラメータ
    char cGenericParameter[INPUTFILE_MAXLINE];  // ジェネリックパラメータ
}InputFileCsys;

// *HOLES 系統
typedef struct
{
    char cHoleGroupName[INPUTFILE_MAXLINE];     // 穴グループ名
    char cXCord[INPUTFILE_MAXLINE];             // 基準位置からのX距離
    char cDatumType[INPUTFILE_MAXLINE];         // X座標の基準データム(0=XM, 2=XRAP, 3=FRAME_END)
    char cSide[INPUTFILE_MAXLINE];              // 穴をあける方向. L(左)/R(右)/B(両方)
    char cRefPartNumber[INPUTFILE_MAXLINE];     // 穴と部品が紐づいている
}InputFileHole;

// *HOLE_TABLE 系統
typedef struct
{
    char cHoleGroupName[INPUTFILE_MAXLINE];     // 穴グループ名
    char cHoleID[INPUTFILE_MAXLINE];            // 穴番号
    char cXCord[INPUTFILE_MAXLINE];             // 基準穴からのX距離
    char cYCord[INPUTFILE_MAXLINE];             // 基準穴からのY距離
    char cZCord[INPUTFILE_MAXLINE];             // 基準穴からのZ距離
    char cFrameDiameter[INPUTFILE_MAXLINE];     // フレーム穴直径
    char cInnerLineDiameter[INPUTFILE_MAXLINE]; // インナーライナーの穴直径
    char cNA1[INPUTFILE_MAXLINE];               // 未使用1
    char cHoleFlag[INPUTFILE_MAXLINE];          // 穴フラグ(1:穴1 , 2: ホールグループ内の穴がすべてフレーム上面)
}InputFileHoleTable;

// *MODULES 系統 - Rename部
typedef struct
{
    char cAfterName[INPUTFILE_MAXLINE];         // Rename後の名前
    char cBeforeName[INPUTFILE_MAXLINE];        // Rename前の名前
}InputFileRenameFile;

// *MODULES 系統 - Modules部
typedef struct
{
    char cModules[INPUTFILE_MAXLINE];           // 対象部品名
    char cConstraint[INPUTFILE_MAXLINE];        // 拘束種類(V, H, M, X)
    char cConstraintType[INPUTFILE_MAXLINE];    // 拘束条件(CSYS名)
    char cModulesName[INPUTFILE_MAXLINE];       // モジュール名
}InputFileModules;

/*-----------------------------------------------------------------*\
    グローバル変数(システムスコープ)
\*-----------------------------------------------------------------*/
// コンフィグレーションファイルから取得したフレーム名
extern ProMdlName gwFramename;
extern int giSectionRows[ENUM_INPUT_SECTION_MAX];

// inputfileの中身を格納するようの構造体一式
extern InputFileParamFeature* gstrParametersFeature;
extern InputFileParameters* gstrParameters;
extern InputFileFeature* gstrFeatures;
extern InputFileCsys* gstrVehicleCoordinateSystems;
extern InputFileHole* gstrHoles;
extern InputFileHoleTable* gstrHoleTable;
extern InputFileRenameFile* gstrInnerlinerRename;
extern InputFileModules* gstrInnerliner;
extern InputFileParamFeature* gstrInnerlinerParametersFeature;
extern InputFileParameters* gstrInnerlinerParameters;
extern InputFileFeature* gstrInnerlinerFeatures;
extern InputFileHole* gstrInnerlinerHoles;
extern InputFileHoleTable* gstrInnerlinerHoleTable;
extern InputFileRenameFile* gstrModulesRename;
extern InputFileModules* gstrModules;
extern InputFileRenameFile* gstrFrontAxleRename;
extern InputFileModules* gstrFrontAxle;
extern InputFileParamFeature* gstrFrontAxleParamsFeature;
extern InputFileParameters* gstrFrontAxleParams;
extern InputFileRenameFile* gstrCabRename;
extern InputFileModules* gstrCab;
extern InputFileRenameFile* gstrEngineRename;
extern InputFileModules* gstrEngine;
extern InputFileCsys* gstrGearboxCoordinateSystems;
extern InputFileRenameFile* gstrGearboxRename;
extern InputFileModules* gstrGearbox;
extern InputFileRenameFile* gstrPropShaftRename;
extern InputFileModules* gstrPropShaft;
extern InputFileParamFeature* gstrPropShaftParamsFeature;
extern InputFileParameters* gstrPropShaftParams;




/*-----------------------------------------------------------------*\
  プロトタイプ宣言(Prototype declaration)
\*-----------------------------------------------------------------*/
ProError  loadInputFile(ProCharPath strFileName, int* iErrorCnt);
