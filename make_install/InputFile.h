/*
* InputFile.h
*/
#pragma once

/*-----------------------------------------------------------------*\
    �}�N��
\*-----------------------------------------------------------------*/
//inputfile�̍ő啶����
#define INPUTFILE_MAXLINE 100

#define MAX_LINE 512
#define MAX_TOP_ASSY_LENGTH 15

/***********************************************
InputFile
    1�Z�N�V��������2���\���̃Z�N�V�����ɂ��āA
    ����ȒP����`���ăq�b�g�����猙�Ȃ̂�
    �����P����`���Ă���
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
    ENUM_INPUT_SECTION_PARAMETERS_FEATURE,              // 5  *PARAMETERS��Feature��
    ENUM_INPUT_SECTION_PARAMETERS,                      // 6  *PARAMETERS��Parameter��
    ENUM_INPUT_SECTION_FEATURES,                        // 7
    ENUM_INPUT_SECTION_VEHICLE_COORDINATE_SYSTEMS,      // 8
    ENUM_INPUT_SECTION_PARTS,                           // 9
    ENUM_INPUT_SECTION_HOLES,                           // 10
    ENUM_INPUT_SECTION_HOLE_TABLE,                      // 11
    ENUM_INPUT_SECTION_INNERLINER_RENAME,               // 12   *INNERLINER��Rename��
    ENUM_INPUT_SECTION_INNERLINER,                      // 13   *INNERLINER��Modules��
    ENUM_INPUT_SECTION_INNERLINER_PARAMETERS_FEATURE,   // 14  *INNERLINER_PARAMETERS��Feature��
    ENUM_INPUT_SECTION_INNERLINER_PARAMETERS,           // 15  *INNERLINER_PARAMETERS��Parameter��
    ENUM_INPUT_SECTION_INNERLINER_FEATURES,             // 16
    ENUM_INPUT_SECTION_INNERLINER_HOLES,                // 17
    ENUM_INPUT_SECTION_INNERLINER_HOLE_TABLE,           // 18
    ENUM_INPUT_SECTION_MODULES_RENAME,                  // 19  *MODULES��Rename��
    ENUM_INPUT_SECTION_MODULES,                         // 20  *MODULES��Modules��
    ENUM_INPUT_SECTION_FRONT_AXLE_RENAME,               // 21  *FRONT_AXLE��Rename��
    ENUM_INPUT_SECTION_FRONT_AXLE,                      // 22  *FRONT_AXLE��Modules��
    ENUM_INPUT_SECTION_FRONT_AXLE_PARAMS_FEATURE,       // 23  *FRONT_AXLE��Feature��
    ENUM_INPUT_SECTION_FRONT_AXLE_PARAMS,               // 24  *FRONT_AXLE��Parameter��
    ENUM_INPUT_SECTION_CAB_RENAME,                      // 25  *CAB��Rename��
    ENUM_INPUT_SECTION_CAB,                             // 26  *CAB��Modules��
    ENUM_INPUT_SECTION_ENGINE_RENAME,                   // 27  *ENGINE��Rename��
    ENUM_INPUT_SECTION_ENGINE,                          // 28  *ENGINE��Modules��
    ENUM_INPUT_SECTION_GEARBOX_COORDINATE_SYSTEMS,      // 29
    ENUM_INPUT_SECTION_GEARBOX_RENAME,                  // 30  *GEARBOX��Rename��
    ENUM_INPUT_SECTION_GEARBOX,                         // 31  *GEARBOX��Modules��
    ENUM_INPUT_SECTION_PROP_SHAFT_RENAME,               // 32  *PROP_SHAFT��Rename��
    ENUM_INPUT_SECTION_PROP_SHAFT,                      // 33  *PROP_SHAFT��Modules��
    ENUM_INPUT_SECTION_PROP_SHAFT_PARAMS_FEATURE,       // 34  *PROP_SHAFT_PARAMS��Feature��
    ENUM_INPUT_SECTION_PROP_SHAFT_PARAMS,               // 35  *PROP_SHAFT_PARAMS��Parameter��
    ENUM_INPUT_SECTION_MAX                              // 36  Section�̍ő�l
};


/*-----------------------------------------------------------------*\
    �\����
\*-----------------------------------------------------------------*/
// *PARAMETERS �n�� - Feature��
typedef struct
{
    char cFeature[INPUTFILE_MAXLINE];           // �p�����[�^�ύX�Ώ�
}InputFileParamFeature;

// *PARAMETERS �n�� - Parameter��
typedef struct
{
    char cParameteValue[INPUTFILE_MAXLINE];     // �p�����[�^�l
    char cParameterName[INPUTFILE_MAXLINE];     // �p�����[�^��
}InputFileParameters;

// *FEATURES �n��
typedef struct
{
    char cValue[INPUTFILE_MAXLINE];             // �Ώە��i�� (�� 22680087)
    char cFeatureName[INPUTFILE_MAXLINE];       // Feature��  (�� L.H. RAIL;INNERLINER START)
}InputFileFeature;

// *COORDINATE_SYSTEMS �n��
typedef struct
{
    char cInstanceParameter[INPUTFILE_MAXLINE]; // �C���X�^���X�p�����[�^
    char cGenericParameter[INPUTFILE_MAXLINE];  // �W�F�l���b�N�p�����[�^
}InputFileCsys;

// *HOLES �n��
typedef struct
{
    char cHoleGroupName[INPUTFILE_MAXLINE];     // ���O���[�v��
    char cXCord[INPUTFILE_MAXLINE];             // ��ʒu�����X����
    char cDatumType[INPUTFILE_MAXLINE];         // X���W�̊�f�[�^��(0=XM, 2=XRAP, 3=FRAME_END)
    char cSide[INPUTFILE_MAXLINE];              // �������������. L(��)/R(�E)/B(����)
    char cRefPartNumber[INPUTFILE_MAXLINE];     // ���ƕ��i���R�Â��Ă���
}InputFileHole;

// *HOLE_TABLE �n��
typedef struct
{
    char cHoleGroupName[INPUTFILE_MAXLINE];     // ���O���[�v��
    char cHoleID[INPUTFILE_MAXLINE];            // ���ԍ�
    char cXCord[INPUTFILE_MAXLINE];             // ��������X����
    char cYCord[INPUTFILE_MAXLINE];             // ��������Y����
    char cZCord[INPUTFILE_MAXLINE];             // ��������Z����
    char cFrameDiameter[INPUTFILE_MAXLINE];     // �t���[�������a
    char cInnerLineDiameter[INPUTFILE_MAXLINE]; // �C���i�[���C�i�[�̌����a
    char cNA1[INPUTFILE_MAXLINE];               // ���g�p1
    char cHoleFlag[INPUTFILE_MAXLINE];          // ���t���O(1:��1 , 2: �z�[���O���[�v���̌������ׂăt���[�����)
}InputFileHoleTable;

// *MODULES �n�� - Rename��
typedef struct
{
    char cAfterName[INPUTFILE_MAXLINE];         // Rename��̖��O
    char cBeforeName[INPUTFILE_MAXLINE];        // Rename�O�̖��O
}InputFileRenameFile;

// *MODULES �n�� - Modules��
typedef struct
{
    char cModules[INPUTFILE_MAXLINE];           // �Ώە��i��
    char cConstraint[INPUTFILE_MAXLINE];        // �S�����(V, H, M, X)
    char cConstraintType[INPUTFILE_MAXLINE];    // �S������(CSYS��)
    char cModulesName[INPUTFILE_MAXLINE];       // ���W���[����
}InputFileModules;

/*-----------------------------------------------------------------*\
    �O���[�o���ϐ�(�V�X�e���X�R�[�v)
\*-----------------------------------------------------------------*/
// �R���t�B�O���[�V�����t�@�C������擾�����t���[����
extern ProMdlName gwFramename;
extern int giSectionRows[ENUM_INPUT_SECTION_MAX];

// inputfile�̒��g���i�[����悤�̍\���̈ꎮ
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
  �v���g�^�C�v�錾(Prototype declaration)
\*-----------------------------------------------------------------*/
ProError  loadInputFile(ProCharPath strFileName, int* iErrorCnt);
