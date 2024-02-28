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
    �O���[�o���ϐ�
\*-----------------------------------------------------------------*/
// inputFile���̋�؂�ꗗ�B�z��̏��Ԃ�inputFile.h����enum�ɍ��킹�邱�ƁI
// �Œ�*VEHICLE_COORDINATE_SYSTEMS�ɍ��킹��
char gInputSection[ENUM_INPUT_SECTION_MAX][28] = {  
 INPUT_SIGNATURE                                    , // 0
 INPUT_IDENTITY                                     , // 1
 INPUT_PROTOM_SPEC_ID                               , // 2
 INPUT_OM_NO                                        , // 3
 INPUT_FO_NO                                        , // 4
 INPUT_SECTION_PARAMETERS_FEATURE                   , // 5  *PARAMETERS��Feature��
 INPUT_SECTION_PARAMETERS                           , // 6  *PARAMETERS��Parameter��
 INPUT_SECTION_FEATURES                             , // 7
 INPUT_SECTION_VEHICLE_COORDINATE_SYSTEMS           , // 8
 INPUT_SECTION_PARTS                                , // 9
 INPUT_SECTION_HOLES                                , // 10
 INPUT_SECTION_HOLE_TABLE                           , // 11
 INPUT_SECTION_INNERLINER_RENAME                    , // 12  *INNERLINER��Rename��
 INPUT_SECTION_INNERLINER                           , // 13  *INNERLINER��Modules��
 INPUT_SECTION_INNERLINER_PARAMETERS_FEATURE        , // 14 *INNERLINER_PARAMETERS��Feature��
 INPUT_SECTION_INNERLINER_PARAMETERS                , // 15 *INNERLINER_PARAMETERS��Parameter��
 INPUT_SECTION_INNERLINER_FEATURES                  , // 16
 INPUT_SECTION_INNERLINER_HOLES                     , // 17
 INPUT_SECTION_INNERLINER_HOLE_TABLE                , // 18
 INPUT_SECTION_MODULES_RENAME                       , // 19 *MODULES��Rename��
 INPUT_SECTION_MODULES                              , // 20 *MODULES��Modules��
 INPUT_SECTION_FRONT_AXLE_RENAME                    , // 21 *FRONT_AXLE��Rename��
 INPUT_SECTION_FRONT_AXLE                           , // 22 *FRONT_AXLE��Modules��
 INPUT_SECTION_FRONT_AXLE_PARAMS_FEATURE            , // 23 *PROP_SHAFT_PARAMS��Feature��
 INPUT_SECTION_FRONT_AXLE_PARAMS                    , // 24 *PROP_SHAFT_PARAMS��Parameter��
 INPUT_SECTION_CAB_RENAME                           , // 25 *CAB��Rename��
 INPUT_SECTION_CAB                                  , // 26 *CAB��Modules��
 INPUT_SECTION_ENGINE_RENAME                        , // 27 *ENGINE��Rename��
 INPUT_SECTION_ENGINE                               , // 28 *ENGINE��Modules��
 INPUT_SECTION_GEARBOX_COORDINATE_SYSTEMS           , // 29
 INPUT_SECTION_GEARBOX_RENAME                       , // 30 *GEARBOX��Rename��
 INPUT_SECTION_GEARBOX                              , // 31 *GEARBOX��Modules��
 INPUT_SECTION_PROP_SHAFT_RENAME                    , // 32 *PROP_SHAFT��Rename��
 INPUT_SECTION_PROP_SHAFT                           , // 33 *PROP_SHAFT��Modules��
 INPUT_SECTION_PROP_SHAFT_PARAMS_FEATURE            , // 34 *FRONT_AXLE_PARAMS��Feature��
 INPUT_SECTION_PROP_SHAFT_PARAMS                      // 35 *FRONT_AXLE_PARAMS��Parameter��
};

#define OK_WORDS                      65
wchar_t wOkWords[OK_WORDS] = {L"1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ-_"};

// inputFile���̋�؂�ꗗ�̊e�s�����擾����B
int giSectionRows[ENUM_INPUT_SECTION_MAX];
// �R���t�B�O���[�V�����t�@�C������擾���� {(IDENTITY: or PROTOM SPEC ID: or OM NO.: or FO NO.:)}
ProMdlName gwFramename;
// �R���t�B�O���[�V�����t�@�C������擾����Sigunature
// �� �R���t�B�O���[�V��������IDENTITY:�����݂���ꍇ�́A(IDENTITY:)_(Signature)�Ƃ���悤�ɉ��C�������A
// (IDENTITY:)�݂̂Ƃ���悤�ɂƂ̗v�]����gwSignature�͌��ݎg�p���Ă��Ȃ�
ProMdlName gwSignature;

// inputfile�̒��g���i�[����悤�̍\���̈ꎮ
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
PURPOSE  : inputFile�����[�h���A�e�\���̂֒l���i�[����
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
        // �t�@�C���I�[�v���̎��s�� checkInitial()�Ŋm�F�ς݂����A
        // �R���t�B�O���[�V�����t�@�C���̎擾�Ɏ��s�����ꍇ�͈ȍ~�̏��������Ȃ�
        return;
    }

    // �O���[�o���ϐ��̏�����
    memset(giSectionRows, 0, sizeof(giSectionRows));
    ProWstringCopy(L"", gwFramename, PRO_VALUE_UNUSED);
    ProWstringCopy(L"", gwSignature, PRO_VALUE_UNUSED);

    /***********************************************
     inputFile �� �e�Z�N�V����/��؂�̍s�����J�E���g����
    *************************************************/
    while (fgets(str, sizeof(str), fp) != NULL) {

        if (strstr(str, "//") != NULL && strstr(str, INPUT_SIGNATURE) != NULL) {
            // INPUT_SIGNATURE�̏ꍇ
            giSectionRows[ENUM_INPUT_SIGNATURE]++;
        }
        else if (strstr(str, "//") != NULL && strstr(str, INPUT_IDENTITY) != NULL) {
            // INPUT_IDENTITY�̏ꍇ
            giSectionRows[ENUM_INPUT_IDENTITY]++;
        }
        else if (strstr(str, "//") != NULL && strstr(str, INPUT_PROTOM_SPEC_ID) != NULL) {
            // INPUT_PROTOM_SPEC_ID�̏ꍇ
            giSectionRows[ENUM_INPUT_PROTOM_SPEC_ID]++;
        }
        else if (strstr(str, "//") != NULL && strstr(str, INPUT_OM_NO) != NULL) {
            // INPUT_OM_NO�̏ꍇ
            giSectionRows[ENUM_INPUT_OM_NO]++;
        
        }
        else if (strstr(str, "//") != NULL && strstr(str, INPUT_FO_NO) != NULL) {
            // INPUT_FO_NO�̏ꍇ
            giSectionRows[ENUM_INPUT_FO_NO]++;

        }else if (strstr(str, "//") != NULL || strcmp(str, "\n") == NULL) {
            // �R�����g��, �󔒍s�݂̂̏ꍇ�A�������Ȃ�
            continue;
        }else {
            int iSectionFlag = 0;
            
            for (int iCnt = 1; iCnt < ENUM_INPUT_SECTION_MAX; iCnt++) {
                // �Z�N�V������� ����肷��
                if (strcmp(c_trim(str), gInputSection[iCnt]) == NULL) {
                    iSectionType = iCnt;
                    iSectionFlag = 1;
                    iOtherSectionFlag = 0;
                    break;
                }

            }

            if (iSectionFlag != 1 && str[0] == '*') {
                // �Z�N�V�����s�ȊO�̃Z�N�V����(�擪��*�Ŏn�܂�s)�����݂���ꍇ�̓��O�ɏo�͂���
                LOG_PRINT("ATTENTION : Other section %s", str);
                // �Z�N�V�����s�ȊO�̃Z�N�V�����̓Z�N�V���������ׂĂ��J�E���g���Ȃ����߁A�t���O�Ő��䂷��
                iOtherSectionFlag = 1;
                // �Z�N�V�����s�̓J�E���g���Ȃ����߁Acontinue ����
                continue;
            }

            if (iSectionFlag == 1 || iOtherSectionFlag == 1) {
                // �Z�N�V�����s�̓J�E���g���Ȃ����߁Acontinue ����
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
            // PARAMETER�n�� , MODULES�n���̏ꍇ�@�g���q�̂���s���͕ʃJ�E���g����
            if ((strstr(str, ".asm") != NULL)
                || (strstr(str, ".prt") != NULL)
                || (strstr(str, ".lay") != NULL)) {
                giSectionRows[iSectionType]++;
            }
            else {
                giSectionRows[iSectionType+1]++;
            } 
        }else if (iSectionType > 0) {
            // �e�Z�N�V�����̍s�����J�E���g����
            giSectionRows[iSectionType]++;
        }

    }

    // �t�@�C���ʒu��擪�ɖ߂�
    fseek(fp, 0, SEEK_SET);

    /***********************************************
     inputFile �̒l���i�[����ϐ��̃������m��
    *************************************************/
    status = mallcInputFile();
    if (status == PRO_TK_GENERAL_ERROR) {
        return status;
    }

    /***********************************************
     inputFile �� �l�擾
    *************************************************/
    while (fgets(str, sizeof(str), fp) != NULL) {

        TRAIL_PRINT("%s(%d) : inputFile = %s", __func__, __LINE__, str);

        if (strstr(str, "\t") != NULL) {
            char *strtemp;
            // �^�u���������甼�p�X�y�[�X�ɕϊ�����
            str_replace(str, "\t", INPUT_SEPARATION, &strtemp);
            strncpy(str, strtemp, sizeof(str));
        }else if (strstr(str, "//") != NULL && strstr(str, INPUT_SIGNATURE) != NULL) {
            // INPUT_SIGNATURE�̏ꍇ
            iSectionType = ENUM_INPUT_SIGNATURE;
            bSection = PRO_B_TRUE;
        }
        else if (strstr(str, "//") != NULL && strstr(str, INPUT_IDENTITY) != NULL) {
            // INPUT_IDENTITY�̏ꍇ
            iSectionType = ENUM_INPUT_IDENTITY;
            bSection = PRO_B_TRUE;
        }
        else if (strstr(str, "//") != NULL && strstr(str, INPUT_PROTOM_SPEC_ID) != NULL) {
            // INPUT_PROTOM_SPEC_ID�̏ꍇ
            iSectionType = ENUM_INPUT_PROTOM_SPEC_ID;
            bSection = PRO_B_TRUE;
        }
        else if (strstr(str, "//") != NULL && strstr(str, INPUT_OM_NO) != NULL) {
            // INPUT_OM_NO�̏ꍇ
            iSectionType = ENUM_INPUT_OM_NO;
            bSection = PRO_B_TRUE;
        }
        else if (strstr(str, "//") != NULL && strstr(str, INPUT_FO_NO) != NULL) {
            // INPUT_FO_NO�̏ꍇ
            iSectionType = ENUM_INPUT_FO_NO;
            bSection = PRO_B_TRUE;
        }
        else if (strstr(str, "//") != NULL || strcmp(str, "\n") == NULL) {
            // �R�����g��, �󔒍s�݂̂̏ꍇ�A�������Ȃ�
            continue;
        } else {
            for (int iCnt = 1; iCnt < ENUM_INPUT_SECTION_MAX; iCnt++) {
                // �Z�N�V������� ����肷��
                if (strcmp(c_trim(str), gInputSection[iCnt]) == NULL) {
                    iSectionType = iCnt;
                    bSection = PRO_B_TRUE;
                    iSectionRow = 0;
                    break;
                }
            }

            if (bSection != PRO_B_TRUE && str[0] == '*') {
                // �Z�N�V�����s�ȊO�̃Z�N�V����(�擪��*�Ŏn�܂�s)�����݂���ꍇ�͏������Ȃ��悤�ɏ���������
                iSectionType = -1;
            }
        }

        // �Z�N�V������� ���擾�����ꍇ�Ɍ��菈��������
        if (iSectionType == ENUM_INPUT_SIGNATURE
            || iSectionType == ENUM_INPUT_IDENTITY
            || iSectionType == ENUM_INPUT_PROTOM_SPEC_ID 
            || iSectionType == ENUM_INPUT_OM_NO
            || iSectionType == ENUM_INPUT_FO_NO) {
            // SIGNATURE: / IDENTITY: / PROTOM SPEC ID: / OM NO.: / FO NO.:�s�͏�������
            getInputFile(iSectionType, str, iSectionRow);
            //  IDENTITY: ��1�s�݂̂̂��߁A������
            iSectionType = -1;
        } else if (iSectionType > 0) {
            if (bSection) {
                // IDENTITY: / PROTOM SPEC ID: / OM NO.: / FO NO.: �ȊO�̃Z�N�V�����̏ꍇ,�Z�N�V�����s�͏������Ȃ�
                bSection = PRO_B_FALSE;
                continue;
            }else {
                iSectionRow++;
                getInputFile(iSectionType, str, iSectionRow);
            }

        }
    }

    //�t�@�C�������
    fclose(fp);
    
    // �R���t�B�O���[�V�����t�@�C������IDENTITY: / PROTOM SPEC ID: / OM NO.:/ FO NO.:���擾�ł��Ȃ��������߁A�G���[
    int iResult;
    ProWstringCompare(L"", gwFramename, PRO_VALUE_UNUSED, &iResult);
    if (iResult == 0) {
        LOG_PRINT("NOK : Could not get to IDENTITY: / PROTOM SPEC ID: / OM NO.: / FO NO.:in ConfigrationFile");
        *iErrorCnt = *iErrorCnt + 1;
    }
    else {

        if (giSectionRows[ENUM_INPUT_PROTOM_SPEC_ID] != 0) {
            // INPUT_PROTOM_SPEC_ID�̏ꍇ�A�X�y�[�X�ŋ�؂�ꂽ�^�񒆂�ID��UD-SAVP �� Top assembly name �Ƃ���
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
                    // �X�y�[�X��1�ʉ߂��������񂩂�擾����
                    wFramenameTemp[iLen] = gwFramename[iLoop];
                    iLen++;
                }
            }

            // �擾�����������i�[����
            ProWstringCopy(wFramenameTemp, gwFramename, PRO_VALUE_UNUSED);

        }

        /***********************************************
        * Top�A�Z���u��������сA�e�A�Z���u����p�[�c�A���C�A�E�g���͈ȉ��̖����K���ɏ]��
      {(IDENTITY: or PROTOM SPEC ID: or OM NO.: or FO NO.:)}_(�A��3��)_(FIL/FA/CAB/ENG/GB/PS)_(1/2)  

          Creo�I�u�W�F�N�g���͊g���q���܂܂Ȃ��ōő�31�����̂��߁A
      rename�オ32�����ȏ�̏ꍇ��31�����ƂȂ�悤��(IDENTITY: or PROTOM SPEC ID: or OM NO.: or FO NO.:)�̖��������
      (IDENTITY: or PROTOM SPEC ID: or OM NO.: or FO NO.:) =< 21�����ł��邱�Ƃ��m�F����

          ���ǋL
          *FRONT_AXLE �Z�N�V�����ɂđg�ݕt����A�Z���u���ꎮ�����l�[���Ώ�
       {(IDENTITY: or PROTOM SPEC ID: or OM NO.: or FO NO.:)}_(�A��3��)_FA_(�A�Z���u����8��)
      (IDENTITY: or PROTOM SPEC ID: or OM NO.: or FO NO.:) =< 15�����ł��邱�Ƃ��m�F����
        *************************************************/
        int iLength = 0;
        int iOkWordsCheckFlag = 0;
        ProWstringLengthGet(gwFramename, &iLength);
        if (iLength > MAX_TOP_ASSY_LENGTH) {
            iLength = MAX_TOP_ASSY_LENGTH;
        }
        ProMdlName wFramename;
        // ������
        wmemset(wFramename,0,sizeof(wFramename));
        // ������ MAX_TOP_ASSY_LENGTH �������擾
        for (int iLoop = 0; iLoop < iLength; iLoop++) {
            // ������
            iOkWordsCheckFlag = 0;
            // �֎~�������g�p����Ă���ꍇ�̓A���_�[�X�R�A�ɒu��������
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
PURPOSE  : ipuutfile���̒l���i�[����\���̂̃��������m�ۂ���

\*====================================================================*/
ProError  mallcInputFile() {

    // *PARAMETERS��Feature��
    gstrParametersFeature = (InputFileParamFeature*)calloc(giSectionRows[ENUM_INPUT_SECTION_PARAMETERS_FEATURE], sizeof(InputFileParamFeature));
    if (!gstrParametersFeature) {
        // �������s��
        LOG_PRINT("NOK : Not enough memory");
        return PRO_TK_GENERAL_ERROR;
    }

    // *PARAMETERS��Parameter��
    gstrParameters = (InputFileParameters*)calloc(giSectionRows[ENUM_INPUT_SECTION_PARAMETERS], sizeof(InputFileParameters));
    if (!gstrParameters) {
        // �������s��
        LOG_PRINT("NOK : Not enough memory");
        return PRO_TK_GENERAL_ERROR;
    }

    // *FEATURES
    gstrFeatures = (InputFileFeature*)calloc(giSectionRows[ENUM_INPUT_SECTION_FEATURES], sizeof(InputFileFeature));
    if (!gstrFeatures) {
        // �������s��
        LOG_PRINT("NOK : Not enough memory");
        return PRO_TK_GENERAL_ERROR;
    }

    // *VEHICLE_COORDINATE_SYSTEMS
    gstrVehicleCoordinateSystems = (InputFileCsys*)calloc(giSectionRows[ENUM_INPUT_SECTION_VEHICLE_COORDINATE_SYSTEMS], sizeof(InputFileCsys));
    if (!gstrVehicleCoordinateSystems) {
        // �������s��
        LOG_PRINT("NOK : Not enough memory");
        return PRO_TK_GENERAL_ERROR;
    }

    // *PARTS (���g�p)
    // *HOLES
    gstrHoles = (InputFileHole*)calloc(giSectionRows[ENUM_INPUT_SECTION_HOLES], sizeof(InputFileHole));
    if (!gstrHoles) {
        // �������s��
        LOG_PRINT("NOK : Not enough memory");
        return PRO_TK_GENERAL_ERROR;
    }

    // *HOLE_TABLE
    gstrHoleTable = (InputFileHoleTable*)calloc(giSectionRows[ENUM_INPUT_SECTION_HOLE_TABLE], sizeof(InputFileHoleTable));
    if (!gstrHoleTable) {
        // �������s��
        LOG_PRINT("NOK : Not enough memory");
        return PRO_TK_GENERAL_ERROR;
    }

    // *INNERLINER��Rename��
    gstrInnerlinerRename = (InputFileRenameFile*)calloc(giSectionRows[ENUM_INPUT_SECTION_INNERLINER_RENAME], sizeof(InputFileRenameFile));
    if (!gstrInnerlinerRename) {
        // �������s��
        LOG_PRINT("NOK : Not enough memory");
        return PRO_TK_GENERAL_ERROR;
    }

    // *INNERLINER��Modules��
    gstrInnerliner = (InputFileModules*)calloc(giSectionRows[ENUM_INPUT_SECTION_INNERLINER], sizeof(InputFileModules));
    if (!gstrInnerliner) {
        // �������s��
        LOG_PRINT("NOK : Not enough memory");
        return PRO_TK_GENERAL_ERROR;
    }

    // *INNERLINER_PARAMETERS��Feature��
    gstrInnerlinerParametersFeature = (InputFileParamFeature*)calloc(giSectionRows[ENUM_INPUT_SECTION_INNERLINER_PARAMETERS_FEATURE], sizeof(InputFileParamFeature));
    if (!gstrInnerlinerParametersFeature) {
        // �������s��
        LOG_PRINT("NOK : Not enough memory");
        return PRO_TK_GENERAL_ERROR;
    }

    // *INNERLINER_PARAMETERS��Parameter��
    gstrInnerlinerParameters = (InputFileParameters*)calloc(giSectionRows[ENUM_INPUT_SECTION_INNERLINER_PARAMETERS], sizeof(InputFileParameters));
    if (!gstrInnerlinerParameters) {
        // �������s��
        LOG_PRINT("NOK : Not enough memory");
        return PRO_TK_GENERAL_ERROR;
    }

    // *INNERLINER_FEATURES
    gstrInnerlinerFeatures = (InputFileFeature*)calloc(giSectionRows[ENUM_INPUT_SECTION_INNERLINER_FEATURES], sizeof(InputFileFeature));
    if (!gstrInnerlinerFeatures) {
        // �������s��
        LOG_PRINT("NOK : Not enough memory");
        return PRO_TK_GENERAL_ERROR;
    }

    // *INNERLINER_HOLES
    gstrInnerlinerHoles = (InputFileHole*)calloc(giSectionRows[ENUM_INPUT_SECTION_INNERLINER_HOLES], sizeof(InputFileHole));
    if (!gstrInnerlinerHoles) {
        // �������s��
        LOG_PRINT("NOK : Not enough memory");
        return PRO_TK_GENERAL_ERROR;
    }

    // *INNERLINER_HOLE_TABLE
    gstrInnerlinerHoleTable = (InputFileHoleTable*)calloc(giSectionRows[ENUM_INPUT_SECTION_INNERLINER_HOLE_TABLE], sizeof(InputFileHoleTable));
    if (!gstrInnerlinerHoleTable) {
        // �������s��
        LOG_PRINT("NOK : Not enough memory");
        return PRO_TK_GENERAL_ERROR;
    }

    // *MODULES��Rename��
    gstrModulesRename = (InputFileRenameFile*)calloc(giSectionRows[ENUM_INPUT_SECTION_MODULES_RENAME], sizeof(InputFileRenameFile));
    if (!gstrModulesRename) {
        // �������s��
        LOG_PRINT("NOK : Not enough memory");
        return PRO_TK_GENERAL_ERROR;
    }

    // *MODULES��Modules��
    gstrModules = (InputFileModules*)calloc(giSectionRows[ENUM_INPUT_SECTION_MODULES], sizeof(InputFileModules));
    if (!gstrModules) {
        // �������s��
        LOG_PRINT("NOK : Not enough memory");
        return PRO_TK_GENERAL_ERROR;
    }

    // *FRONT_AXLE��Rename��
    gstrFrontAxleRename = (InputFileRenameFile*)calloc(giSectionRows[ENUM_INPUT_SECTION_FRONT_AXLE_RENAME], sizeof(InputFileRenameFile));
    if (!gstrFrontAxleRename) {
        // �������s��
        LOG_PRINT("NOK : Not enough memory");
        return PRO_TK_GENERAL_ERROR;
    }

    // *FRONT_AXLE��Modules��
    gstrFrontAxle = (InputFileModules*)calloc(giSectionRows[ENUM_INPUT_SECTION_FRONT_AXLE], sizeof(InputFileModules));
    if (!gstrFrontAxle) {
        // �������s��
        LOG_PRINT("NOK : Not enough memory");
        return PRO_TK_GENERAL_ERROR;
    }

    // *PROP_SHAFT_PARAMS��Feature��
    gstrFrontAxleParamsFeature = (InputFileParamFeature*)calloc(giSectionRows[ENUM_INPUT_SECTION_FRONT_AXLE_PARAMS_FEATURE], sizeof(InputFileParamFeature));
    if (!gstrFrontAxleParamsFeature) {
        // �������s��
        LOG_PRINT("NOK : Not enough memory");
        return PRO_TK_GENERAL_ERROR;
    }

    // *PROP_SHAFT_PARAMS��Parameter��
    gstrFrontAxleParams = (InputFileParameters*)calloc(giSectionRows[ENUM_INPUT_SECTION_FRONT_AXLE_PARAMS], sizeof(InputFileParameters));
    if (!gstrFrontAxleParams) {
        // �������s��
        LOG_PRINT("NOK : Not enough memory");
        return PRO_TK_GENERAL_ERROR;
    }

    // *CAB��Rename��
    gstrCabRename = (InputFileRenameFile*)calloc(giSectionRows[ENUM_INPUT_SECTION_CAB_RENAME], sizeof(InputFileRenameFile));
    if (!gstrCabRename) {
        // �������s��
        LOG_PRINT("NOK : Not enough memory");
        return PRO_TK_GENERAL_ERROR;
    }

    // *CAB��Modules��
    gstrCab = (InputFileModules*)calloc(giSectionRows[ENUM_INPUT_SECTION_CAB], sizeof(InputFileModules));
    if (!gstrCab) {
        // �������s��
        LOG_PRINT("NOK : Not enough memory");
        return PRO_TK_GENERAL_ERROR;
    }

    // *ENGINE��Rename��
    gstrEngineRename = (InputFileRenameFile*)calloc(giSectionRows[ENUM_INPUT_SECTION_ENGINE_RENAME], sizeof(InputFileRenameFile));
    if (!gstrEngineRename) {
        // �������s��
        LOG_PRINT("NOK : Not enough memory");
        return PRO_TK_GENERAL_ERROR;
    }

    // *ENGINE��Modules��
    gstrEngine = (InputFileModules*)calloc(giSectionRows[ENUM_INPUT_SECTION_ENGINE], sizeof(InputFileModules));
    if (!gstrEngine) {
        // �������s��
        LOG_PRINT("NOK : Not enough memory");
        return PRO_TK_GENERAL_ERROR;
    }

    // *GEARBOX_COORDINATE_SYSTEMS
    gstrGearboxCoordinateSystems = (InputFileCsys*)calloc(giSectionRows[ENUM_INPUT_SECTION_GEARBOX_COORDINATE_SYSTEMS], sizeof(InputFileCsys));
    if (!gstrGearboxCoordinateSystems) {
        // �������s��
        LOG_PRINT("NOK : Not enough memory");
        return PRO_TK_GENERAL_ERROR;
    }

    // *GEARBOX��Rename��
    gstrGearboxRename = (InputFileRenameFile*)calloc(giSectionRows[ENUM_INPUT_SECTION_GEARBOX_RENAME], sizeof(InputFileRenameFile));
    if (!gstrGearboxRename) {
        // �������s��
        LOG_PRINT("NOK : Not enough memory");
        return PRO_TK_GENERAL_ERROR;
    }

    // *GEARBOX��Modules��
    gstrGearbox = (InputFileModules*)calloc(giSectionRows[ENUM_INPUT_SECTION_GEARBOX], sizeof(InputFileModules));
    if (!gstrGearbox) {
        // �������s��
        LOG_PRINT("NOK : Not enough memory");
        return PRO_TK_GENERAL_ERROR;
    }

    // *PROP_SHAFT��Rename��
    gstrPropShaftRename = (InputFileRenameFile*)calloc(giSectionRows[ENUM_INPUT_SECTION_PROP_SHAFT_RENAME], sizeof(InputFileRenameFile));
    if (!gstrPropShaftRename) {
        // �������s��
        LOG_PRINT("NOK : Not enough memory");
        return PRO_TK_GENERAL_ERROR;
    }

    // *PROP_SHAFT��Modules��
    gstrPropShaft = (InputFileModules*)calloc(giSectionRows[ENUM_INPUT_SECTION_PROP_SHAFT], sizeof(InputFileModules));
    if (!gstrPropShaft) {
        // �������s��
        LOG_PRINT("NOK : Not enough memory");
        return PRO_TK_GENERAL_ERROR;
    }

    // *FRONT_AXLE_PARAMS��Feature��
    gstrPropShaftParamsFeature = (InputFileParamFeature*)calloc(giSectionRows[ENUM_INPUT_SECTION_PROP_SHAFT_PARAMS_FEATURE], sizeof(InputFileParamFeature));
    if (!gstrPropShaftParamsFeature) {
        // �������s��
        LOG_PRINT("NOK : Not enough memory");
        return PRO_TK_GENERAL_ERROR;
    }

    // *FRONT_AXLE_PARAMS��Parameter��
    gstrPropShaftParams = (InputFileParameters*)calloc(giSectionRows[ENUM_INPUT_SECTION_PROP_SHAFT_PARAMS], sizeof(InputFileParameters));
    if (!gstrPropShaftParams) {
        // �������s��
        LOG_PRINT("NOK : Not enough memory");
        return PRO_TK_GENERAL_ERROR;
    }

}

/*====================================================================*\
FUNCTION : getInputFile
PURPOSE  : inputFile�̒��g���e�\���̂֊i�[����
  int   iSectionType           (in)    �Z�N�V�������
  char  str[PRO_LINE_SIZE]     (in)    inputFile����1�s 
  int   iSectionRow            (in)    �Z�N�V�������ł̌��݂̍s��
���l:
Parameter�n��/Modules�n���̃Z�N�V��������2���\���ƂȂ邽�߁A
1���ڂ̍s����iSectionRow�ŃJ�E���g���A
2���ڂ� (iSectionRow)-(1���ڂ̍ő�s) �ŃJ�E���g����B

\*====================================================================*/
ProError  getInputFile(int iSectionType, char str[INPUTFILE_MAXLINE], int iSectionRow){

    char cIdentity[PRO_MDLNAME_SIZE];
    char* p;
    // ������
    memset(cIdentity, 0, sizeof(cIdentity));

    switch (iSectionType){
    case ENUM_INPUT_SIGNATURE:
        /***********************************************
         SIGNATURE:
        *************************************************/
        // ���������o��
        p = strstr(c_trim(str), INPUT_SIGNATURE);
        strncpy(cIdentity, p + sizeof(INPUT_SIGNATURE), INPUTFILE_MAXLINE);
        // wstring�ϊ�
        ProStringToWstring(gwSignature, cIdentity);
    break;
    case ENUM_INPUT_IDENTITY:
        /***********************************************
         IDENTITY:
        *************************************************/
        // ���������o��
        p = strstr(c_trim(str), INPUT_IDENTITY);
        strncpy(cIdentity, p + sizeof(INPUT_IDENTITY), INPUTFILE_MAXLINE);
        // wstring�ϊ�
        ProStringToWstring(gwFramename, cIdentity);
        break;
    case ENUM_INPUT_PROTOM_SPEC_ID:
        /***********************************************
         PROTOM SPEC ID:
        *************************************************/
        // ���������o��
        p = strstr(c_trim(str), INPUT_PROTOM_SPEC_ID);
        strncpy(cIdentity, p + sizeof(INPUT_PROTOM_SPEC_ID), INPUTFILE_MAXLINE);
        // wstring�ϊ�
        ProStringToWstring(gwFramename, cIdentity);
        break;
    case ENUM_INPUT_OM_NO:
        /***********************************************
         OM NO.:
        *************************************************/
        // ���������o��
        p = strstr(c_trim(str), INPUT_OM_NO);
        strncpy(cIdentity, p + sizeof(INPUT_OM_NO), INPUTFILE_MAXLINE);
        // wstring�ϊ�
        ProStringToWstring(gwFramename, cIdentity);
        break;
    case ENUM_INPUT_FO_NO:
        /***********************************************
         FO NO.:
        *************************************************/
        // ���������o��
        p = strstr(c_trim(str), INPUT_FO_NO);
        strncpy(cIdentity, p + sizeof(INPUT_FO_NO), INPUTFILE_MAXLINE);
        // wstring�ϊ�
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
          *PARTS ���g�p�̂��߁A�����Ȃ�
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
PURPOSE  : PARAMETER�n���̒l���t�B�[�`�����ƃp�����[�^���ɕ����Ċi�[����
  InputFileParamFeature* strParamFeat,  (out) �t�B�[�`�����̊i�[��
  InputFileParameters* strParam,        (out) �p�����[�^���̊i�[��
  char  str[PRO_LINE_SIZE]     (in)    inputFile����1�s
  int   iSectionRow            (in)    �Z�N�V�������ł̌��݂̍s��
  int   iEnum                  (in)    �t�B�[�`������Enum�l

\*====================================================================*/
ProError  getParameter(InputFileParamFeature* strParamFeat,
    InputFileParameters* strParam,
    char str[INPUTFILE_MAXLINE],
    int iSectionRow,
    int iEnum) {


    if (giSectionRows[iEnum] != 0
        && giSectionRows[iEnum] >= iSectionRow) {
        /***********************************************
         �t�B�[�`�����̏���
        *************************************************/

        for (int iLoop = 1; iLoop < iSectionRow; iLoop++) {
            *strParamFeat++;
        }
        // 1�s�݂̂̂��߃g�������ăR�s�[����
        strncpy(strParamFeat->cFeature, c_trim(str), sizeof(strParamFeat->cFeature));

    } else {
        /***********************************************
         �p�����[�^���̏���
        *************************************************/
        char* cpParameteValue;     // �p�����[�^�l
        char* cpParameterName;     // �p�����[�^��

        int iSubSectionRow = iSectionRow - giSectionRows[iEnum];

        for (int iLoop = 1; iLoop < iSubSectionRow; iLoop++) {
            *strParam++;
        }

        // ���p�X�y�[�X��؂�Œl���擾����
        cpParameteValue = strtok(str, INPUT_SEPARATION);
        strncpy(strParam->cParameteValue, cpParameteValue, sizeof(strParam->cParameteValue));

        // �������� NULL ���w�肷��ƑO��� str �l�̑�������n�߂�
        cpParameterName = strtok(NULL, INPUT_SEPARATION);
        strncpy(strParam->cParameterName, cpParameterName, sizeof(strParam->cParameterName));
    }
}

/*====================================================================*\
FUNCTION : getFeatures
PURPOSE  : FEATURES�n���̒l���i�[����
  InputFileFeature* strFeat,  (out) �i�[��
  char  str[PRO_LINE_SIZE]     (in)    inputFile����1�s
  int   iSectionRow            (in)    �Z�N�V�������ł̌��݂̍s��
  int   iEnum                  (in)    Enum�l

\*====================================================================*/
ProError  getFeatures(InputFileFeature* strFeat,
    char str[INPUTFILE_MAXLINE],
    int iSectionRow,
    int iEnum) {

        char* cpValue;           // �Ώە��i��
        char* cpFeatureName;     // Feature��
        char cFeatureNameTemp[INPUTFILE_MAXLINE];   // Feature��(�ꎞ)


        for (int iLoop = 1; iLoop < iSectionRow; iLoop++) {
            *strFeat++;
        }

        // ���p�X�y�[�X��؂�Œl���擾����
        cpValue = strtok(str, INPUT_SEPARATION);
        strncpy(strFeat->cValue, cpValue, sizeof(strFeat->cValue));

        // Feature�n����Feature���͔��p�X�y�[�X�ŋ�؂炸�A1�̒l�Ƃ��ď�������
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
PURPOSE  : COORDINATE_SYSTEMS�n���̒l���i�[����
  InputFileCsys* strCsys,  (out) �i�[��
  char  str[PRO_LINE_SIZE]     (in)    inputFile����1�s
  int   iSectionRow            (in)    �Z�N�V�������ł̌��݂̍s��
  int   iEnum                  (in)    Enum�l

\*====================================================================*/
ProError  getCoordinateSystems(InputFileCsys* strCsys,
    char str[INPUTFILE_MAXLINE],
    int iSectionRow,
    int iEnum) {

    char* cpInstanceParameter;    // �C���X�^���X�p�����[�^
    char* cpGenericParameter;     // �W�F�l���b�N�p�����[�^

    for (int iLoop = 1; iLoop < iSectionRow; iLoop++) {
        *strCsys++;
    }

    // ���p�X�y�[�X��؂�Œl���擾����
    cpInstanceParameter = strtok(str, INPUT_SEPARATION);
    strncpy(strCsys->cInstanceParameter, cpInstanceParameter, sizeof(strCsys->cInstanceParameter));

    // �������� NULL ���w�肷��ƑO��� str �l�̑�������n�߂�
    cpGenericParameter = strtok(NULL, INPUT_SEPARATION);
    strncpy(strCsys->cGenericParameter, cpGenericParameter, sizeof(strCsys->cGenericParameter));

}

/*====================================================================*\
FUNCTION : getHole
PURPOSE  : HOLES�n���̒l���i�[����
  InputFileHole* strHole,      (out) �i�[��
  char  str[PRO_LINE_SIZE]     (in)    inputFile����1�s
  int   iSectionRow            (in)    �Z�N�V�������ł̌��݂̍s��
  int   iEnum                  (in)    Enum�l

\*====================================================================*/
ProError  getHole(InputFileHole* strHole,
    char str[INPUTFILE_MAXLINE],
    int iSectionRow,
    int iEnum) {

    char* cpHoleGroupName;    // ���O���[�v��
    char* cpXCord;            // ��ʒu�����X����
    char* cpNA;               // ���g�p
    char* cpSide;             // �������������. L(��)/R(�E)/B(����)
    char* cpRefPartNumber;    // ���ƕ��i���R�Â��Ă���

    for (int iLoop = 1; iLoop < iSectionRow; iLoop++) {
        *strHole++;
    }

    // ���p�X�y�[�X��؂�Œl���擾����
    cpHoleGroupName = strtok(str, INPUT_SEPARATION);
    strncpy(strHole->cHoleGroupName, cpHoleGroupName, sizeof(strHole->cHoleGroupName));

    // �������� NULL ���w�肷��ƑO��� str �l�̑�������n�߂�
    cpXCord = strtok(NULL, INPUT_SEPARATION);
    strncpy(strHole->cXCord, cpXCord, sizeof(strHole->cXCord));

    cpNA = strtok(NULL, INPUT_SEPARATION);
    strncpy(strHole->cDatumType, cpNA, sizeof(strHole->cDatumType));

    cpSide = strtok(NULL, INPUT_SEPARATION);
    strncpy(strHole->cSide, cpSide, sizeof(strHole->cSide));

    // cpRefPartNumber �͑��݂��Ȃ����Ƃ����邽�߁A����Γo�^����
    cpRefPartNumber = strtok(NULL, INPUT_SEPARATION);
    if (cpRefPartNumber) {
        strncpy(strHole->cRefPartNumber, cpRefPartNumber, sizeof(strHole->cRefPartNumber));
    }

}

/*====================================================================*\
FUNCTION : getHoleTable
PURPOSE  : HOLES�n���̒l���i�[����
  InputFileHoleTable* strHoleTable,      (out) �i�[��
  char  str[PRO_LINE_SIZE]     (in)    inputFile����1�s
  int   iSectionRow            (in)    �Z�N�V�������ł̌��݂̍s��
  int   iEnum                  (in)    Enum�l

\*====================================================================*/
ProError  getHoleTable(InputFileHoleTable* strHoleTable,
    char str[INPUTFILE_MAXLINE],
    int iSectionRow,
    int iEnum) {

    char* cpHoleGroupName;      // ���O���[�v��
    char* cpHoleID;             // ���ԍ�
    char* cpXCord;              // ��������X����
    char* cpYCord;              // ��������Y����
    char* cpZCord;              // ��������Z����
    char* cpFrameDiameter;      // �t���[�������a
    char* cpInnerLineDiameter;  // �C���i�[���C�i�[�̌����a
    char* cpNA1;                // ���g�p1
    char* cpNA2;                // ���g�p2

    for (int iLoop = 1; iLoop < iSectionRow; iLoop++) {
        *strHoleTable++;
    }

    // ���p�X�y�[�X��؂�Œl���擾����
    cpHoleGroupName = strtok(str, INPUT_SEPARATION);
    strncpy(strHoleTable->cHoleGroupName, cpHoleGroupName, sizeof(strHoleTable->cHoleGroupName));

    // �������� NULL ���w�肷��ƑO��� str �l�̑�������n�߂�
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

    // cpNA2 �͑��݂��Ȃ����Ƃ����邽�߁A����Γo�^����
    cpNA2 = strtok(NULL, INPUT_SEPARATION);
    if (cpNA2) {
        strncpy(strHoleTable->cHoleFlag, cpNA2, sizeof(strHoleTable->cHoleFlag));
    }

}

/*====================================================================*\
FUNCTION : getModules
PURPOSE  : MODULES�n���̒l��Rename���ƃ��W���[�����ɕ����Ċi�[����
  InputFileRenameFile* strRename,   (out) Rename���̊i�[��
  InputFileModules* strModules,     (out) ���W���[�����̊i�[��
  char  str[PRO_LINE_SIZE]          (in)  inputFile����1�s
  int   iSectionRow                 (in)  �Z�N�V�������ł̌��݂̍s��
  int   iEnum                       (in)  Rename����Enum�l

\*====================================================================*/
ProError  getModules(InputFileRenameFile* strRename,
    InputFileModules* strModules,
    char str[INPUTFILE_MAXLINE],
    int iSectionRow,
    int iEnum) {


    if (giSectionRows[iEnum] != 0
        && giSectionRows[iEnum] >= iSectionRow) {
        /***********************************************
         Rename���̏���
        *************************************************/
        char* cpAfterName;    // Rename��̖��O
        char* cpBeforeName;   // Rename�O�̖��O

        for (int iLoop = 1; iLoop < iSectionRow; iLoop++) {
            *strRename++;
        }

        // ���p�X�y�[�X��؂�Œl���擾����
        cpAfterName = strtok(str, INPUT_SEPARATION);
        strncpy(strRename->cAfterName, cpAfterName, sizeof(strRename->cAfterName));

        // �������� NULL ���w�肷��ƑO��� str �l�̑�������n�߂�
        cpBeforeName = strtok(NULL, INPUT_SEPARATION);
        strncpy(strRename->cBeforeName, cpBeforeName, sizeof(strRename->cBeforeName));
    }
    else {
        /***********************************************
         ���W���[�����̏���
        *************************************************/
        char* cpModules;        // �Ώە��i��
        char* cpConstraint;     // �S�����
        char* cpConstraintType; // �S������
        char* cpModulesName;    // ���W���[����
        char cModulesNameTemp[INPUTFILE_MAXLINE];   // ���W���[����(�ꎞ)

        int iSubSectionRow = iSectionRow - giSectionRows[iEnum];

        for (int iLoop = 1; iLoop < iSubSectionRow; iLoop++) {
            *strModules++;
        }

        // ���p�X�y�[�X��؂�Œl���擾����
        cpModules = strtok(str, INPUT_SEPARATION);
        strncpy(strModules->cModules, cpModules, sizeof(strModules->cModules));

        // �������� NULL ���w�肷��ƑO��� str �l�̑�������n�߂�
        cpConstraint = strtok(NULL, INPUT_SEPARATION);
        strncpy(strModules->cConstraint, cpConstraint, sizeof(strModules->cConstraint));

        // �S����ނ� H �ȊO�̏ꍇ�ɁA�S����������������
        if (strcmp(strModules->cConstraint,"H") != 0) {
            cpConstraintType = strtok(NULL, INPUT_SEPARATION);
            strncpy(strModules->cConstraintType, cpConstraintType, sizeof(strModules->cConstraintType));
        }

        // Modules�n����Modules���͔��p�X�y�[�X�ŋ�؂炸�A1�̒l�Ƃ��ď�������
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
