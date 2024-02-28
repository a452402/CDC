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
    �v���g�^�C�v�錾
\*-----------------------------------------------------------------*/
ProError  checkInitial(int* iErrorCnt, ProCharPath* cImpFolderPath, ProPath wTopAssyName, WindchillInfo* windchillInfo, ProCharPath* cFeatureLsitPath);
ProError  endProcessLog(WindchillInfo windchillInfo, ProMdlName wTopAssyNewName, int iErrorFlag);
ProError  renamePart(ProPath wOldPart, ProPath wNewPart, ProPath wLeftRight);
ProError  deleteFamilyInstances(ProPath wPartName);
ProError  renameFrontAxle(InputFileModules* strModules, int iSectionMaxRows, ProMdlName wTopAssyNewName, ProMdlName wType);
ProError  checkInObject(ProMdl top_asm, WindchillInfo windchillInfo);
ProError  renameSameTopAssyName(InputFileRenameFile* strModulesRename, int iSectionMaxRows, ProMdlName wTopAssyNewName, ProMdlName wType);

/*-----------------------------------------------------------------*\
    �}�N��
\*-----------------------------------------------------------------*/
//inputfile�̍ő啶����
#define MAX_LINE 512

// �ݒ�t�@�C���̌Œ�l
#define SETTING_LOG_INIT_PATH                       ".\\UD-SAVP\\log\\"
#define SETTING_SERIAL_NUMBER_SECTION   			"SERIAL_NUMBER_SECTION"
#define SETTING_PREFIX_KEY       					"PREFIX"
#define SETTING_PREFIX_VALUE     					"NUMBER"

// �o�[�W�������
#define VERSION_INFO_MODE           "BatchMode"
#define VERSION_INFO_VER            "Ver.11"
#define VERSION_INFO_DATE           "2023/9/5"

/*-----------------------------------------------------------------*\
    �O���[�o���ϐ�
\*-----------------------------------------------------------------*/
// ���O�t�@�C���p�X
ProCharPath gcLogFilePath = "";

/*====================================================================*\
FUNCTION : mainCustom
PURPOSE  : �����J�n
\*====================================================================*/
ProError  mainCustom()
{
    TRAIL_PRINT("%s(%d) : === START ===", __func__, __LINE__);


	ProError status;
    ProMdl top_asm;                 // top�A�Z���u���̃n���h��
    int iErrorCnt = 0;              // �G���[�̐�
    ProCharPath cImpFolderPath;     // �ݒ�t�@�C������ǂݍ��ރR���t�B�O���[�V�����t�@�C���̃p�X
    ProPath wTopAssyName;          // �t�@�C�� ����擾����TopAssy

    ProCharPath cFeatureLsitPath;          // Feature �� Frame Template �ɖ����ꍇ�ł����O��NOK�ƕ\�������Ȃ�ListPath

    WindchillInfo windchillInfo;
    ProWstringCopy(L"", windchillInfo.defaultWorkspace, PRO_VALUE_UNUSED);
    ProWstringCopy(L"", windchillInfo.defaultContext, PRO_VALUE_UNUSED);
    ProWstringCopy(L"", windchillInfo.defaultFolder, PRO_VALUE_UNUSED);
    ProWstringCopy(L"", windchillInfo.settingIniWorkspace, PRO_VALUE_UNUSED);
    ProWstringCopy(L"", windchillInfo.settingIniContext, PRO_VALUE_UNUSED);
    ProWstringCopy(L"", windchillInfo.settingIniFolder, PRO_VALUE_UNUSED);
    strcpy("", cFeatureLsitPath);

    /***********************************************
     Windchill�̐ڑ��m�F�E���̑��������s����ōŒ�����̊m�F
    *************************************************/
    status = checkInitial(&iErrorCnt, &cImpFolderPath, wTopAssyName, &windchillInfo, &cFeatureLsitPath);
    TRAIL_PRINT("%s(%d) : checkInitial = %s", __func__, __LINE__, getProErrorMessage(status));

    /***********************************************
     FeatureLsit�t�@�C���̃��[�h
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
     �R���t�B�O���[�V�����t�@�C���̃��[�h
    *************************************************/
    status = loadInputFile(cImpFolderPath, &iErrorCnt);
    TRAIL_PRINT("%s(%d) : loadInputFile = %s", __func__, __LINE__, getProErrorMessage(status));
    if (status != PRO_TK_NO_ERROR)
    {
        iErrorCnt++;
    }

    /***********************************************
     �t�@�C���̖����K���̊m�F�^���O�Ɏg�p����ԍ����擾����
    *************************************************/
    ProMdlName wTopAssyNameNumber;
    checkNewFrameName(&wTopAssyNameNumber, &iErrorCnt);


    if (iErrorCnt == 0) {
        /***********************************************
         Windchill����TOP�A�Z���u���i�e���v���[�g�t���[���j�������E���[�h
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
     �����J�n�m�F
     ��L�ŃG���[���������Ă����ꍇ�͏������J�n�����Ȃ�
    *************************************************/
    if (iErrorCnt != 0) {
        endProcessLog(windchillInfo, wTopAssyName, ENUM_MESSAGE_SETTING_FAILED);
        return;
    }

    /***********************************************
         �g�t�������̊J�n
    *************************************************/
    LOG_PRINT("---------------------------------------------------");
    LOG_PRINT("   Start processing");
    LOG_PRINT("---------------------------------------------------");

    /***********************************************
      *PARAMETER �Z�N�V�����̓K�p
      * INNERLINER_HOLES/INNERLINER_HOLE_TABLE �ɂ�*PARAMETER ���g�p����̂ŁA��Ɏ��{����
    *************************************************/
    if (giSectionRows[ENUM_INPUT_SECTION_PARAMETERS] != 0) {
        LOG_PRINT("*PARAMETER start");
        setParametersSection(top_asm, wTopAssyName, gstrParametersFeature, gstrParameters, giSectionRows[ENUM_INPUT_SECTION_PARAMETERS_FEATURE], giSectionRows[ENUM_INPUT_SECTION_PARAMETERS]);
    }

    /***********************************************
      *FEATURES �Z�N�V�����̓K�p
    *************************************************/
    if (giSectionRows[ENUM_INPUT_SECTION_FEATURES] != 0) {
        LOG_PRINT("*FEATURES start");
        setFeaturesSection(top_asm, gstrFeatures, giSectionRows[ENUM_INPUT_SECTION_FEATURES], gstrFeatureList);
    }

    /***********************************************
      *Holes/HoleTable �Z�N�V�����̓K�p
�@    * PARAMETERS�ɂāA�t���[���̒������ς��\�������邽�߁A*PARAMETER �Z�N�V������Ɏ��{
    *************************************************/
    if (giSectionRows[ENUM_INPUT_SECTION_HOLES] != 0 && giSectionRows[ENUM_INPUT_SECTION_HOLE_TABLE] != 0) {
        LOG_PRINT("*Holes/HoleTable start");
        setHoles_HoleTableSection(&top_asm, &top_asm, wTopAssyName, gstrHoles, gstrHoleTable, giSectionRows[ENUM_INPUT_SECTION_HOLES], giSectionRows[ENUM_INPUT_SECTION_HOLE_TABLE], HOLE);
    }

    /***********************************************
      *INNERLINER �̃��[�h
    *************************************************/
    wchar_t* wOldTopInnerLiner = (wchar_t*)calloc(INPUTFILE_MAXLINE, sizeof(wchar_t));
    wchar_t* wNewTopInnerLiner = (wchar_t*)calloc(INPUTFILE_MAXLINE, sizeof(wchar_t));
    ProError statusInnerLinerLoad;

    ProMdl mdlTopInnerLiner;
    statusInnerLinerLoad = loadAssembly(gstrInnerlinerRename, giSectionRows[ENUM_INPUT_SECTION_INNERLINER_RENAME], &mdlTopInnerLiner, wOldTopInnerLiner, wNewTopInnerLiner);

   if (statusInnerLinerLoad == PRO_TK_NO_ERROR) {
       /***********************************************
         *INNERLINER_PARAMETERS �Z�N�V�����̓K�p
       *************************************************/
       if (giSectionRows[ENUM_INPUT_SECTION_INNERLINER_PARAMETERS] != 0) {
           LOG_PRINT("*INNERLINER_PARAMETERS start");
           setParametersSection(mdlTopInnerLiner, wOldTopInnerLiner, gstrInnerlinerParametersFeature, gstrInnerlinerParameters, giSectionRows[ENUM_INPUT_SECTION_INNERLINER_PARAMETERS_FEATURE], giSectionRows[ENUM_INPUT_SECTION_INNERLINER_PARAMETERS]);
       }

       /***********************************************
         *INNERLINER_HOLES/INNERLINER_HOLE_TABLE �Z�N�V�����̓K�p
         * PARAMETERS�ɂāA�t���[���̒������ς��\�������邽�߁A*INNERLINER_PARAMETERS �Z�N�V������Ɏ��{
       *************************************************/
       if (giSectionRows[ENUM_INPUT_SECTION_INNERLINER_HOLES] != 0 && giSectionRows[ENUM_INPUT_SECTION_INNERLINER_HOLE_TABLE] != 0) {
           LOG_PRINT("*INNERLINER_HOLES/INNERLINER_HOLE_TABLE start");
           setHoles_HoleTableSection(&mdlTopInnerLiner, &top_asm, wOldTopInnerLiner, gstrInnerlinerHoles, gstrInnerlinerHoleTable, giSectionRows[ENUM_INPUT_SECTION_INNERLINER_HOLES], giSectionRows[ENUM_INPUT_SECTION_INNERLINER_HOLE_TABLE], INNERLINER_HOLE);
       }

       /***********************************************
         *INNERLINER_FEATURES �Z�N�V�����̓K�p
         * 
       *************************************************/
       if (giSectionRows[ENUM_INPUT_SECTION_INNERLINER_FEATURES] != 0) {
           LOG_PRINT("*INNERLINER_FEATURES start");
           setFeaturesSection(mdlTopInnerLiner, gstrInnerlinerFeatures, giSectionRows[ENUM_INPUT_SECTION_INNERLINER_FEATURES], gstrFeatureList);
       }

       /***********************************************
         VEHICLE_COORDINATE_SYSTEMS(INNERLINER_PARAMETERS) �Z�N�V�����̓K�p
         setModulesSection �O�Ɏ��{
       *************************************************/
       if (giSectionRows[ENUM_INPUT_SECTION_VEHICLE_COORDINATE_SYSTEMS] != 0) {
           LOG_PRINT("*VEHICLE_COORDINATE_SYSTEMS(INNERLINER_PARAMETERS) start");
           SetCoordinateSystemsSection(mdlTopInnerLiner, gstrVehicleCoordinateSystems, giSectionRows[ENUM_INPUT_SECTION_VEHICLE_COORDINATE_SYSTEMS]);
       }

       /***********************************************
         *INNERLINER �Z�N�V�����̓K�p
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
               // INNERLINER��Modules���͑��݂��Ȃ����A�`�����Ƃ��ċL��
               setModulesSection(&mdlTopInnerLiner, gstrInnerliner, NULL, giSectionRows[ENUM_INPUT_SECTION_INNERLINER], NULL, ENUM_INPUT_SECTION_INNERLINER);
           }
       }

   }else {
       LOG_PRINT("Stop all INNERLINER processing");
   }

   /***********************************************
     *FRONT_AXLE �̃��[�h
   *************************************************/
   wchar_t* wOldTopFrontAxle = (wchar_t*)calloc(INPUTFILE_MAXLINE, sizeof(wchar_t)); // Rename�O�̖��O(�g���q����)
   wchar_t* wNewTopFrontAxle = (wchar_t*)calloc(INPUTFILE_MAXLINE, sizeof(wchar_t)); // Rename��̖��O(�g���q����)

   ProMdl mdlTopFrontAxle;
   status = loadAssembly(gstrFrontAxleRename, giSectionRows[ENUM_INPUT_SECTION_FRONT_AXLE_RENAME], &mdlTopFrontAxle, wOldTopFrontAxle, wNewTopFrontAxle);
   if (status == PRO_TK_NO_ERROR) {
       /***********************************************
         VEHICLE_COORDINATE_SYSTEMS(FRONT_AXLE) �Z�N�V�����̓K�p
         setModulesSection �O�Ɏ��{
       *************************************************/
       if (giSectionRows[ENUM_INPUT_SECTION_VEHICLE_COORDINATE_SYSTEMS] != 0) {
           LOG_PRINT("*VEHICLE_COORDINATE_SYSTEMS(FRONT_AXLE) start");
           SetCoordinateSystemsSection(mdlTopFrontAxle, gstrVehicleCoordinateSystems, giSectionRows[ENUM_INPUT_SECTION_VEHICLE_COORDINATE_SYSTEMS]);
       }

       /***********************************************
         *FRONT_AXLE �Z�N�V�����̓K�p
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
         *FRONT_AXLE_PARAMS �Z�N�V�����̓K�p
         *FRONT_AXLE�̃��l�[��������Ɏ��{
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
     *CAB �̃��[�h
   *************************************************/
   wchar_t* wOldTopCab = (wchar_t*)calloc(INPUTFILE_MAXLINE, sizeof(wchar_t)); // Rename�O�̖��O(�g���q����)
   wchar_t* wNewTopCab = (wchar_t*)calloc(INPUTFILE_MAXLINE, sizeof(wchar_t)); // Rename��̖��O(�g���q����)
   ProMdl mdlTopCab;
   status = loadAssembly(gstrCabRename, giSectionRows[ENUM_INPUT_SECTION_CAB_RENAME], &mdlTopCab, wOldTopCab, wNewTopCab);
   if (status == PRO_TK_NO_ERROR) {
       /***********************************************
         VEHICLE_COORDINATE_SYSTEMS(CAB) �Z�N�V�����̓K�p
         setModulesSection �O�Ɏ��{
       *************************************************/
       if (giSectionRows[ENUM_INPUT_SECTION_VEHICLE_COORDINATE_SYSTEMS] != 0) {
           LOG_PRINT("*VEHICLE_COORDINATE_SYSTEMS(CAB) start");
           SetCoordinateSystemsSection(mdlTopCab, gstrVehicleCoordinateSystems, giSectionRows[ENUM_INPUT_SECTION_VEHICLE_COORDINATE_SYSTEMS]);
       }

       /***********************************************
         *CAB �Z�N�V�����̓K�p
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
     *ENGINE �̃��[�h
   *************************************************/
   wchar_t* wOldTopEngine = (wchar_t*)calloc(INPUTFILE_MAXLINE, sizeof(wchar_t)); // Rename�O�̖��O(�g���q����)
   wchar_t* wNewTopEngine = (wchar_t*)calloc(INPUTFILE_MAXLINE, sizeof(wchar_t)); // Rename��̖��O(�g���q����)
   ProMdl mdlTopEngine;
   status = loadAssembly(gstrEngineRename, giSectionRows[ENUM_INPUT_SECTION_ENGINE_RENAME], &mdlTopEngine, wOldTopEngine, wNewTopEngine);
   if (status == PRO_TK_NO_ERROR) {
       /***********************************************
         VEHICLE_COORDINATE_SYSTEMS(ENGINE) �Z�N�V�����̓K�p
         setModulesSection �O�Ɏ��{
       *************************************************/
       if (giSectionRows[ENUM_INPUT_SECTION_VEHICLE_COORDINATE_SYSTEMS] != 0) {
           LOG_PRINT("*VEHICLE_COORDINATE_SYSTEMS(ENGINE) start");
           SetCoordinateSystemsSection(mdlTopEngine, gstrVehicleCoordinateSystems, giSectionRows[ENUM_INPUT_SECTION_VEHICLE_COORDINATE_SYSTEMS]);
       }

       /***********************************************
         *ENGINE �Z�N�V�����̓K�p
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
     *GEARBOX �̃��[�h
   *************************************************/
   wchar_t* wOldTopGearbox = (wchar_t*)calloc(INPUTFILE_MAXLINE, sizeof(wchar_t)); // Rename�O�̖��O(�g���q����)
   wchar_t* wNewTopGearbox = (wchar_t*)calloc(INPUTFILE_MAXLINE, sizeof(wchar_t)); // Rename��̖��O(�g���q����)
   ProMdl mdlTopGearbox;
   status = loadAssembly(gstrGearboxRename, giSectionRows[ENUM_INPUT_SECTION_GEARBOX_RENAME], &mdlTopGearbox, wOldTopGearbox, wNewTopGearbox);
   if (status == PRO_TK_NO_ERROR) {
       /***********************************************
         *GEARBOX_COORDINATE_SYSTEMS �Z�N�V�����̓K�p
         * setModulesSection �O�Ɏ��{
       *************************************************/
       if (giSectionRows[ENUM_INPUT_SECTION_GEARBOX_COORDINATE_SYSTEMS] != 0) {
           LOG_PRINT("*GEARBOX_COORDINATE_SYSTEMS start");
           SetCoordinateSystemsSection(mdlTopGearbox, gstrGearboxCoordinateSystems, giSectionRows[ENUM_INPUT_SECTION_GEARBOX_COORDINATE_SYSTEMS]);
       }

       /***********************************************
         *GEARBOX �Z�N�V�����̓K�p
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
     *PROP_SHAFT �̃��[�h
   *************************************************/
   wchar_t* wOldTopPropShaft = (wchar_t*)calloc(INPUTFILE_MAXLINE, sizeof(wchar_t)); // Rename�O�̖��O(�g���q����)
   wchar_t* wNewTopPropShaft = (wchar_t*)calloc(INPUTFILE_MAXLINE, sizeof(wchar_t)); // Rename��̖��O(�g���q����)

   ProMdl mdlTopPropShaft;
   status = loadAssembly(gstrPropShaftRename, giSectionRows[ENUM_INPUT_SECTION_PROP_SHAFT_RENAME], &mdlTopPropShaft, wOldTopPropShaft, wNewTopPropShaft);
   if (status == PRO_TK_NO_ERROR) {
       /***********************************************
         VEHICLE_COORDINATE_SYSTEMS(PROP_SHAFT) �Z�N�V�����̓K�p
         setModulesSection �O�Ɏ��{
       *************************************************/
       if (giSectionRows[ENUM_INPUT_SECTION_VEHICLE_COORDINATE_SYSTEMS] != 0) {
           LOG_PRINT("*VEHICLE_COORDINATE_SYSTEMS(PROP_SHAFT) start");
           SetCoordinateSystemsSection(mdlTopPropShaft, gstrVehicleCoordinateSystems, giSectionRows[ENUM_INPUT_SECTION_VEHICLE_COORDINATE_SYSTEMS]);
       }
       /***********************************************
         *PROP_SHAFT �Z�N�V�����̓K�p
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
         *PROP_SHAFT_PARAMS �Z�N�V�����̓K�p
         *PROP_SHAFT�̃��l�[��������Ɏ��{
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
     *VEHICLE_COORDINATE_SYSTEMS �Z�N�V�����̓K�p
       MODULES �ɂāA�C���X�^���X���f���u���O��CSYS���g�p���Ă��邽�߁A
       MODULES ��Ɏ��{����
   *************************************************/
   if (giSectionRows[ENUM_INPUT_SECTION_VEHICLE_COORDINATE_SYSTEMS] != 0) {
       LOG_PRINT("*VEHICLE_COORDINATE_SYSTEMS(MODULES) start");
       SetCoordinateSystemsSection(top_asm, gstrVehicleCoordinateSystems, giSectionRows[ENUM_INPUT_SECTION_VEHICLE_COORDINATE_SYSTEMS]);
   }
   
   /***********************************************
    �A�Z���u����Master�\�����A�N�e�B�u�ɂ��Ă���A�Đ����s��
   *************************************************/
   ProSimprep p_simp_rep;
   LOG_PRINT("Change to master rep and Regenerate");
   // �}�X�^�[�\���̏ꍇ�͊ȗ��\���̎��ʎq��-1�̂��߁A���O��NULL�Ƃ���
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
      *MODULES �Z�N�V�����̓K�p
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
     �t���[���̖��O��ύX
     ���߂ɖ��O��ύX����ƁA�p�����[�^���K�p����Ȃ��������߁A
     �Ō�ɖ��O��ύX����
      ��InnerLiner�̃p�[�c�A���C�A�E�g�t�@�C���̃��l�[����
  �@    �R���t�B�O���[�V�����t�@�C���ɋL�ڂ��Ȃ��������߂����ŕύX����
      �������t�@�C����2��ȍ~Widchill�փ`�F�b�N�C���ł��Ȃ�����
  �@    Top�A�Z���u���Ɠ��l�ɘA�Ԃ�t�^����
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

    // �T�C�h�t���[���̃��l�[��
    status = renameObject(wTopAssyName, wTopAssyNewName, PRO_MDLFILE_ASSEMBLY);
    status = renameObject(wTopAssyName, wTopAssyNewName, PRO_MDLFILE_NOTEBOOK);
    statusL = renamePart(wTopAssyName, wTopAssyNewName, L"_1");
    statusR = renamePart(wTopAssyName, wTopAssyNewName, L"_2");

    // InnerLiner�̃��l�[��
    if (giSectionRows[ENUM_INPUT_SECTION_INNERLINER_RENAME] != 0) {
        renameSameTopAssyName(gstrInnerlinerRename, giSectionRows[ENUM_INPUT_SECTION_INNERLINER_RENAME], wTopAssyNewName, L"FIL");
    }

    // InnerLiner�����[�h���Ă��Ȃ��ꍇ�͏��������Ȃ�
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
        // *FRONT_AXLE �ł͒ǉ����� Module �ɑ΂��āA���l�[��������B
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
     �t�@�~���[�e�[�u���̃C���X�^���X���폜  (SideFrame)
    *************************************************/
    LOG_PRINT("Delete the family table instance");

    ProPath wLeftPart;
    ProPath wRightPart;

    // �p�[�c���̍쐬
    ProWstringCopy(wTopAssyNewName, wLeftPart, PRO_VALUE_UNUSED);
    ProWstringConcatenate(L"_1", wLeftPart, PRO_VALUE_UNUSED);
    ProWstringCopy(wTopAssyNewName, wRightPart, PRO_VALUE_UNUSED);
    ProWstringConcatenate(L"_2", wRightPart, PRO_VALUE_UNUSED);

    if (statusL == PRO_TK_NO_ERROR) {
        // �C���X�^���X�̍폜
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
    �t�@�~���[�e�[�u���̃C���X�^���X���폜    (InnerLiner)
    ��InnerLiner�����[�h���Ă��Ȃ��ꍇ�͏��������Ȃ�
    *************************************************/
    if (statusInnerLinerLoad == PRO_TK_NO_ERROR) {
        ProPath wLeftInnerLiner;
        ProPath wRightInnerLiner;

        // �p�[�c���̍쐬
        ProWstringCopy(wNewTopAssyForInnerLiner, wLeftInnerLiner, PRO_VALUE_UNUSED);
        ProWstringConcatenate(L"_1", wLeftInnerLiner, PRO_VALUE_UNUSED);
        ProWstringCopy(wNewTopAssyForInnerLiner, wRightInnerLiner, PRO_VALUE_UNUSED);
        ProWstringConcatenate(L"_2", wRightInnerLiner, PRO_VALUE_UNUSED);

        if (statusInnerL == PRO_TK_NO_ERROR) {
            // �C���X�^���X�̍폜
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
     ���f���c���[���X�V���� (�`�F�b�N�C���Ɏ��s�������̂��߂Ɏ��{)
    *************************************************/
     status = ProMdlDisplay(top_asm);
    int window_id2;
    status = ProMdlWindowGet(top_asm, &window_id2);
    status = ProWindowActivate(window_id2);

    /***********************************************
     �`�F�b�N�C��
    *************************************************/
    status = checkInObject(top_asm, windchillInfo);

    /***********************************************
     �������J��
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
        // ��ʃN���A
        int window_id;
        status = ProMdlWindowGet(top_asm, &window_id);
        status = ProWindowClear(window_id);

        // ��\������
        status = ProMdlEraseAll(top_asm);

        // �����I��
        endProcessLog(windchillInfo, wTopAssyNewName, ENUM_MESSAGE_SUCCESS);

    }
    else {
        // �����I��
        endProcessLog(windchillInfo, wTopAssyNewName, ENUM_MESSAGE_CHECKIN_FAILED);

    }
 
    
    return(PRO_TK_NO_ERROR);
}

/*====================================================================*\
FUNCTION : checkInitial
PURPOSE  : ���C�����������{���邽�߂̕K�{�ݒ�l���m�F/�擾������
    int* iErrorCnt                 out     �G���[�̃J�E���g�p
    ProCharPath* cImpFolderPath    out     �R���t�B�O���[�V�����t�@�C���̃t�@�C���p�X
    ProPath wTopAssyName           out     Top�A�Z���u����
    WindchillInfo windchillInfo 	out     WindchillServer�̏��
    ProCharPath *cFeatureLsitPath   out     FeatureList�̃p�X�i�[�p

�߂�l
    PRO_TK_NO_ERROR
    ���G���[���������ꍇ��iErrorCnt�ŃJ�E���g���邽�߁A�S�Đ���I���Ƃ���B

\*====================================================================*/
ProError  checkInitial(int* iErrorCnt, ProCharPath* cImpFolderPath, ProPath wTopAssyName, WindchillInfo *windchillInfo, ProCharPath* cFeatureLsitPath)
{
    ProError status;
    DWORD error;
    char cErrorReason[1024];
    memset(cErrorReason, 0, sizeof(cErrorReason));

    /***********************************************
    �ݒ�t�@�C���F���O�t�@�C���̐����m�F
    *************************************************/
    ProPath     wUserStartAppTime;
    ProCharPath cUserStartAppTime;
    ProCharPath logFilePathTemp;
    int iRet = 0;
    struct stat statBuf;

    // ���ϐ����烍�O�t�@�C�������擾
    getEnvCustom(ENV_USER_START_APP_TIME, wUserStartAppTime, &iRet);

    ProWstringConcatenate(L".log", wUserStartAppTime, PRO_VALUE_UNUSED);
    ProWstringToString(cUserStartAppTime, wUserStartAppTime);

    ProPath         wLogFilePath;
    ProCharPath     cLogFilePath;

    // ���ϐ����烍�O�t�@�C���p�X���擾
    iRet = 0;
    getEnvCustom(ENV_LOG_FILE_PATH, wLogFilePath, &iRet);

    if (iRet == 1) {
        // �p�X���擾�ł��Ȃ�����
        *iErrorCnt = *iErrorCnt + 1;

        strcpy_s(logFilePathTemp, sizeof(logFilePathTemp), SETTING_LOG_INIT_PATH);
        strcat_s(logFilePathTemp, sizeof(logFilePathTemp), cUserStartAppTime);
        sprintf(cErrorReason, "ATTENTION : Could not get LOG_FILE_PATH, so set the default path.");


    }else {
        ProWstringToString(cLogFilePath, wLogFilePath);

        // �擾�����p�X�̑��݊m�F
        if (stat(cLogFilePath, &statBuf) != 0) {

            // �擾�����p�X�����݂��Ȃ����߁A�f�t�H���g�p�X��ݒ肵�܂�
            strcpy_s(logFilePathTemp, sizeof(logFilePathTemp), SETTING_LOG_INIT_PATH);
            strcat_s(logFilePathTemp, sizeof(logFilePathTemp), cUserStartAppTime);
            sprintf(cErrorReason, "ATTENTION : LOG_FILE_PATH not exists, so set the default path.");

        }
        else {
            // �擾�����p�X��ݒ肵�܂�
            strcpy_s(logFilePathTemp, sizeof(logFilePathTemp), cLogFilePath);
            strcat_s(logFilePathTemp, sizeof(logFilePathTemp), cUserStartAppTime);
        }
    }
    // ���O�t�@�C���p�X�̐ݒ�
    strcpy_s(gcLogFilePath, sizeof(gcLogFilePath), logFilePathTemp);

    TRAIL_PRINT("%s(%d) : OK : log file path = %s", __func__, __LINE__, logFilePathTemp);

    /***********************************************
        ���O
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
     �ݒ�t�@�C���F�R���t�B�O���[�V�����t�@�C���̓ǂݍ���
    *************************************************/
    ProPath wConfigrationFilePath;
    ProCharPath cConfigrationFilePath;
    ProPath InOutDir;
    ProPath userStartAppTime;
    ProPath configrationFile;

    getEnvCustomWithLog(ENV_IN_OUT_DIR, InOutDir, iErrorCnt);
    getEnvCustomWithLog(ENV_USER_START_APP_TIME, userStartAppTime, iErrorCnt);
    getEnvCustomWithLog(ENV_CONFIGRATION_FILE, configrationFile, iErrorCnt);

    // �R���t�B�O���[�V�����t�@�C���p�X���擾
    ProWstringCopy(L"", wConfigrationFilePath, PRO_VALUE_UNUSED);
    ProWstringConcatenate(InOutDir, wConfigrationFilePath, PRO_VALUE_UNUSED);
    ProWstringConcatenate(L"/", wConfigrationFilePath, PRO_VALUE_UNUSED);
    ProWstringConcatenate(userStartAppTime, wConfigrationFilePath, PRO_VALUE_UNUSED);
    ProWstringConcatenate(L"/", wConfigrationFilePath, PRO_VALUE_UNUSED);
    ProWstringConcatenate(configrationFile, wConfigrationFilePath, PRO_VALUE_UNUSED);
    // �ł����R���t�B�O���[�V�����t�@�C���p�X�� char�ɕϊ����Ȃ���
    ProWstringToString(cConfigrationFilePath, wConfigrationFilePath);

    if (strlen(cConfigrationFilePath) >= 259) {
        // �t�@�C���ő咷�G���[
        LOG_PRINT("NOK : Configration file path is too long.");
        *iErrorCnt = *iErrorCnt + 1;
    }
    else {
        FILE* fp;
        errno_t error = fopen_s(&fp, cConfigrationFilePath, "r");

        if (error != NULL || fp == NULL) {
            // �R���t�B�O���[�V�����t�@�C�������݂��܂���
            LOG_PRINT("NOK : Configration file not exists");
            *iErrorCnt = *iErrorCnt + 1;
        }
        else {
            // �R���t�B�O���[�V�����t�@�C�������݂���
            strcpy_s(*cImpFolderPath, sizeof(*cImpFolderPath), cConfigrationFilePath);
            LOG_PRINT("OK  : Configration file path = %s", cConfigrationFilePath);

        }
        if (fp != NULL) {
            fclose(fp);

        }
    }

    /***********************************************
     ���ϐ��F�t���[���e���v���[�g�̊m�F
    *************************************************/
    ProPath FrameTemlate;

    getEnvCustomWithLog(ENV_FRAME_TEMPLATE, FrameTemlate, iErrorCnt);
    ProWstringCopy(FrameTemlate, wTopAssyName, PRO_VALUE_UNUSED);

    /***********************************************
     Windchill�ڑ��m�F
    *************************************************/
    wchar_t* actWorkspace;

    // ���ϐ��Fwindchill�� ��� �̎擾
    getEnvCustomWithLog(ENV_WINDCHILL_WORKSPACE, windchillInfo->settingIniWorkspace, iErrorCnt);
    getEnvCustomWithLog(ENV_WINDCHILL_CONTEXT, windchillInfo->settingIniContext, iErrorCnt);
    getEnvCustomWithLog(ENV_WINDCHILL_FOLDER, windchillInfo->settingIniFolder, iErrorCnt);
    getEnvCustomWithLog(ENV_WINDCHILL_USERID, windchillInfo->settingIniUserID, iErrorCnt);
    getEnvCustomWithLog(ENV_WINDCHILL_PASSWORD, windchillInfo->settingIniPasswords, iErrorCnt);
    getEnvCustomWithLog(ENV_WINDCHILL_SERVER, windchillInfo->settingIniServer, iErrorCnt);
    getEnvCustomWithLog(ENV_WINDCHILL_URL, windchillInfo->settingIniUrl, iErrorCnt);

    // Windchill �Ƃ̐ڑ����m�F����
    status = setWindchillOnline(*windchillInfo);

    if (status != PRO_TK_NO_ERROR)
    {
        // Windchill�ɐڑ��ł��܂���ł����B
        LOG_PRINT("NOK : Could not connect to Windchill Server");
        *iErrorCnt = *iErrorCnt + 1;
    }
    else {

        // Windchill�̃��[�N�X�y�[�X�擾
        status = ProServerWorkspaceGet(windchillInfo->settingIniServer, &actWorkspace);
        TRAIL_PRINT("%s(%d) : ProServerWorkspaceGet = %s / actWorkspace = %w", __func__, __LINE__, getProErrorMessage(status), actWorkspace);

        ProWstringConcatenate(actWorkspace, windchillInfo->defaultWorkspace, PRO_VALUE_UNUSED);

        int result;
        ProWstringCompare(windchillInfo->defaultWorkspace, windchillInfo->settingIniWorkspace, PRO_VALUE_UNUSED, &result);
        if (result != NULL) {
            // ���ɃZ�b�g����Ă��郏�[�N�X�y�[�X�̏ꍇ�G���[�ƂȂ�̂ŁA
            // ����Ɣ�r���A�قȂ�ꍇ�̂݃��[�N�X�y�[�X��ύX����
            status = ProServerWorkspaceSet(windchillInfo->settingIniServer, windchillInfo->settingIniWorkspace);
            TRAIL_PRINT("%s(%d) : ProServerWorkspaceSet = %s", __func__, __LINE__, getProErrorMessage(status));
        }

        if (status != PRO_TK_NO_ERROR)
        {
            // Windchill�̃��[�N�X�y�[�X���擾�ł��܂���ł����B
            LOG_PRINT("NOK : Could not get to Windchill Workspace");
            *iErrorCnt = *iErrorCnt + 1;
        }else {
            /***********************************************
             Windchill ���[�N�X�y�[�X�̍폜
            *************************************************/
            ProServerRemoveConflicts conflicts = NULL;

            // ���[�N�X�y�[�X���烂�f�����폜����
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
        �`�F�b�N�C���t�H���_�̑��݊m�F
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
        FeatureList�t�@�C���FFeature �� Frame Template �ɖ����ꍇ�ł����O��NOK�ƕ\�������Ȃ�
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
            // �t�@�C�������݂��܂���
            LOG_PRINT("NOK : SETTING_FRAME_FEATURE_LOG_OK_LIST not exists");
            *iErrorCnt = *iErrorCnt + 1;
        }
        else {
            // �t�@�C�������݂���
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
PURPOSE  : �t���[���e���v���[�g�̖��O���擾����

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
         Top�A�Z���u����WindchillURL�̎擾
        *************************************************/
        ProPath wCheckinFolder;
        wchar_t* server;
        wchar_t* url = NULL;
        ProMdlName wTopAssyNewNameKomoji;

        // �T�[�o�[�̐ݒ�
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
     �A�v���P�[�V�������I��������
    *************************************************/
    TRAIL_PRINT("%s(%d) : === END ===", __func__, __LINE__);
    //exit(0);
    status = ProEngineerEnd();

}

/*====================================================================*\
FUNCTION : checkDuplicate
PURPOSE  : �g�b�v�A�Z���u���̏d���m�F���s���B
���l :     �g�b�v�A�Z���u�������[�h����͎̂��Ԃ�������̂ŁA�p�[�c�Ŋm�F����B
char* cGetValue     (in/out)    �g�b�v�A�Z���u���̘A�ԕ�
\*====================================================================*/
ProError checkDuplicate(char* cGetValue) {


    ProError status;
    ProPath wGetValue;
    ProMdlName wNewName;

    while (TRUE) {
        // Value+1 ��ݒ肷��
        int iGetValue = atoi(cGetValue);
        iGetValue++;
        sprintf(cGetValue, "%03d", iGetValue);
        ProStringToWstring(wGetValue, cGetValue);

        // �p�[�c���̐���
        //wNewName = wTopAssyName + cKeyCnt;
        ProWstringCopy(gwFramename, wNewName, PRO_VALUE_UNUSED);
        ProWstringConcatenate(L"_", wNewName, PRO_VALUE_UNUSED);
        ProWstringConcatenate(wGetValue, wNewName, PRO_VALUE_UNUSED);
        ProWstringConcatenate(L"_1", wNewName, PRO_VALUE_UNUSED);

        ProMdl		 inst_model;
        status = ProMdlnameRetrieve(wNewName, (ProMdlfileType)PRO_MDLFILE_PART, &inst_model);
        TRAIL_PRINT("%s(%d) : ProMdlnameRetrieve = %s", __func__, __LINE__, getProErrorMessage(status));

        if (status != PRO_TK_NO_ERROR) {
            // �w��p�[�c�����[�h�ł��Ȃ��ꍇ�́A�d���̐S�z�Ȃ�
            return;
        }
        else {
            // ��ʃN���A
            int window_id;
            status = ProMdlWindowGet(&inst_model, &window_id);
            status = ProWindowClear(window_id);

            // ��\������
            status = ProMdlEraseAll(inst_model);
        }
    }

}


/*====================================================================*\
FUNCTION : checkNewFrameName
PURPOSE  : �t���[���e���v���[�g�̖��O�̏d�����m�F����
            �J�E���g��+1���āA�{�����Ŏg�p����J�E���g���擾����
���l :
  (IDENTITY:)_(Signature)�@or (PROTOM SPEC ID: or OM NO.:or FO NO.:)}_(�A��3��)

ProMdlName* wNameNumber    out     �{�����Ŏg�p����A��3��
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

    // �R���t�B�O���[�V�����t�@�C�� ����擾����Top�A�Z���u����(gwFramename)
    ProCharPath cPrefix;
    ProWstringToString(cPrefix, gwFramename);

    ProCharPath cPrefixKey;   // PREFIX000
    ProCharPath cValueKey;    // NUMBER000

    getEnvCustomWithLog(ENV_SERIAL_NUMBER, wSerialNumberPath, &iError);
    if (iError == 0) {
        ProWstringToString(cSerialNumberPath, wSerialNumberPath);

        while (Loop) {
            // �J�E���g�𕶎���ɂ��AKEY�����쐬
            sprintf(cKeyCnt, "%03d", iKeyCnt);
            strcpy(cPrefixKey, SETTING_PREFIX_KEY);
            strcat(cPrefixKey, cKeyCnt);

            strcpy(cValueKey, SETTING_PREFIX_VALUE);
            strcat(cValueKey, cKeyCnt);

            /***********************************************
             Signature�̒l���m�F����
            *************************************************/
            DWORD error = GetPrivateProfileString(SETTING_SERIAL_NUMBER_SECTION, cPrefixKey, NULL, cGetValue, sizeof(cGetValue), cSerialNumberPath);

            if (error == NULL) {

                // ���߂����l
                strcpy(cGetValue, "000");

                // �d���m�F���s��
                checkDuplicate(cGetValue);

                // SignatureKEY �����݂��Ȃ��ꍇ�́A�V����Signature/Value��ǉ�����
                WritePrivateProfileString(SETTING_SERIAL_NUMBER_SECTION, cPrefixKey, cPrefix, cSerialNumberPath);
                WritePrivateProfileString(SETTING_SERIAL_NUMBER_SECTION, cValueKey, cGetValue, cSerialNumberPath);
                break;
            }
            else {
                // Signature�����݂���ꍇ�́A�擾����Signature�̒l���m�F����
                if (strcmp(cPrefix, cGetValue) == 0) {
                    // �ݒ�t�@�C���̒l�ƈ�v�����ꍇ�́AValue���m�F����
                    DWORD error = GetPrivateProfileString(SETTING_SERIAL_NUMBER_SECTION, cValueKey, NULL, cGetValue, sizeof(cGetValue), cSerialNumberPath);

                    // �d���m�F���s��
                    checkDuplicate(cGetValue);
                    // �ݒ�t�@�C����ݒ肵�Ȃ���
                    WritePrivateProfileString(SETTING_SERIAL_NUMBER_SECTION, cValueKey, cGetValue, cSerialNumberPath);
                    break;
                }
                else {
                    // �l����v���Ȃ��ꍇ�̓��[�v�𑱂���
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
PURPOSE  : ���E�̃p�[�c����ύX����
ProPath wOldPart        in  ���I�u�W�F�N�g��
ProPath wNewPart        in  �V�I�u�W�F�N�g��
ProPath wLeftRight      in  ���E����( L"_1" , L"_2")

\*====================================================================*/
ProError  renamePart(ProPath wOldPart, ProPath wNewPart, ProPath wLeftRight)
{

    ProError status;
    ProPath wOldPartName;
    ProPath wNewPartName;

    // ���p�[�c���̍쐬
    ProWstringCopy(wOldPart, wOldPartName, PRO_VALUE_UNUSED);
    ProWstringConcatenate(wLeftRight, wOldPartName, PRO_VALUE_UNUSED);

    // �V�p�[�c���̍쐬
    ProWstringCopy(wNewPart, wNewPartName, PRO_VALUE_UNUSED);
    ProWstringConcatenate(wLeftRight, wNewPartName, PRO_VALUE_UNUSED);

    // rename����
    status = renameObject(wOldPartName, wNewPartName, PRO_MDLFILE_PART);

    return status;

}

/*===========================================================================*\
 Function:      deleteFamilyInstances
 Purpose:       �t�@�~���[�e�[�u���̃C���X�^���X���폜����
 �����l�[�������p�[�c�Ƀt�@�~���[�e�[�u��������ꍇ�Awindchill�o�^���ɃR���t���N�g�G���[�ƂȂ���
 �@�T�C�h�t���[���̃t�@�~���[�e�[�u���͍폜���邱�ƂƂ���
\*===========================================================================*/
ProError deleteFamilyInstances(ProPath wPartName)
{
    ProError            status;
    ProMdl mdlFamTable, mdlInstance;
    ProFamtable famtable;

    // �n���h���̏�����
    status = ProMdlInit(wPartName, PRO_MDL_PART, &mdlFamTable);
    status = ProFamtableInit(mdlFamTable, &famtable);
    // �t�@�~���[�e�[�u���폜
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
PURPOSE  : �I�u�W�F�N�g�̖��O��ύX����
           FrontAxle�Ɍ���AModule�Œǉ�����A�Z���u�����Ɂ@�ړ���(ue84fr_bridebb_fa) ��ǉ�����


InputFileModules*   strModules          in  �R���t�B�O���[�V�����t�@�C���̒l
int                 iSectionMaxRows     in  �������ׂ��R���t�B�O���[�V�����t�@�C���̍s��
ProPath             wPrefix             in  �ړ���  

\*====================================================================*/
ProError  renameFrontAxle(InputFileModules* strModules, int iSectionMaxRows, ProMdlName wTopAssyNewName, ProMdlName wType)
{

    ProError status;
    ProMdlName wPrefix;
    wchar_t wAfterName[INPUTFILE_MAXLINE];         // Rename��̖��O
    wchar_t wBeforeName[INPUTFILE_MAXLINE];        // Rename�O�̖��O

    /***********************************************
     Module�ǉ��������̂�wPrefix��ǉ�����
    *************************************************/

    for (int iInputMdlCnt = 0; iInputMdlCnt < iSectionMaxRows; iInputMdlCnt++) {

        // �ϊ��O�̖��O
        ProStringToWstring(wBeforeName, strModules->cModules);

        // �ϊ���̖��O
        ProWstringCopy(wTopAssyNewName, wAfterName, PRO_VALUE_UNUSED);
        ProWstringConcatenate(L"_", wAfterName, PRO_VALUE_UNUSED);
        ProWstringConcatenate(wType, wAfterName, PRO_VALUE_UNUSED);
        ProWstringConcatenate(L"_", wAfterName, PRO_VALUE_UNUSED);
        ProWstringConcatenate(wBeforeName, wAfterName, PRO_VALUE_UNUSED);

        // ���O�̕ϊ�
        status = renameObject(wBeforeName, wAfterName, PRO_MDLFILE_ASSEMBLY);
        strModules++;
    }
}

/*====================================================================*\
FUNCTION : checkInObject
PURPOSE  : windchill�Ƀ`�F�b�N�C������
 ProMdl top_asm
 ���l
 �EWindchill�̐ڑ��m�F
 �Eini�t�@�C���̊m�F
\*====================================================================*/
ProError  checkInObject(ProMdl top_asm, WindchillInfo windchillInfo)
{
    ProError status = PRO_TK_NO_ERROR;
   

    LOG_PRINT("Check-in Objects");

    // 1. �r���[�Ƀ��f����\������
    status = ProMdlDisplay(top_asm);


    // 2. ���f�������[�N�X�y�[�X�ɕۑ�
    status = ProMdlSave(top_asm);
    if (status == PRO_TK_NO_ERROR)
    {
        LOG_PRINT("OK  : Successfully saved to workspace");
    }
    else {
        LOG_PRINT("NOK : Failed to save to workspace(%s)",getProErrorMessage(status));
        return PRO_TK_GENERAL_ERROR;
    }

    // 3. Windchill�փ`�F�b�N�C��
    ProServerCheckinConflicts   conflicts;
    ProServerCheckinOptions     options;
    ProPath wCheckinFolder;
    wchar_t* server;

    // �T�[�o�[�̐ݒ�
    status = ProServerActiveGet(&server);

    ProStringToWstring(wCheckinFolder, "wtpub://");
    ProWstringConcatenate(server, wCheckinFolder, PRO_VALUE_UNUSED);
    ProWstringConcatenate(L"/Libraries/", wCheckinFolder, PRO_VALUE_UNUSED);
    ProWstringConcatenate(windchillInfo.settingIniContext, wCheckinFolder, PRO_VALUE_UNUSED);
    ProWstringConcatenate(L"/", wCheckinFolder, PRO_VALUE_UNUSED);
    ProWstringConcatenate(windchillInfo.settingIniFolder, wCheckinFolder, PRO_VALUE_UNUSED);
    ProWstringConcatenate(L"/", wCheckinFolder, PRO_VALUE_UNUSED);

    status = ProServercheckinoptsAlloc(&options);
    // �������Ă���Q�Ƃ����S�ɖ������邱�Ƃɂ��A�����������������܂�
    status = ProServercheckinoptsAutoresolveSet(options, PRO_SERVER_AUTORESOLVE_IGNORE);
    status = ProServercheckinoptsDeflocationSet(options, wCheckinFolder);
    status = ProServerObjectsCheckin(top_asm, options, &conflicts);
    if (status == PRO_TK_CHECKOUT_CONFLICT)
    {
        // �R���t���N�g���������̏���
        wchar_t* description;
        ProServerconflictsDescriptionGet(conflicts, &description);
        // LOG_PRINT���ɃR���t���N�g���R ( description ) ��ǉ����Ȃ��̂́A�������������邩��

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
    Purpose:	Module�Z�N�V���� (rename��)
    InputFileRenameFile* strModulesRename   (in)    �R���t�B�O���[�V�����t�@�C���̒l
    int iSectionMaxRows                     (in)    �������ׂ��R���t�B�O���[�V�����t�@�C���̍s��
    ProMdlName wTopAssyNewName              (in)    Top�A�Z���u���̖��O
    ProMdlName wType                        (in)    rename�t�@�C���̎��(FA/CAB/ENG/GB/FIL/PS)
\*=========================================================================*/
ProError renameSameTopAssyName(InputFileRenameFile* strModulesRename, int iSectionMaxRows, ProMdlName wTopAssyNewName, ProMdlName wType) 
{

    ProError status;
    wchar_t wAfterName[INPUTFILE_MAXLINE];         // Rename��̖��O
    wchar_t wBeforeName[INPUTFILE_MAXLINE];        // Rename�O�̖��O
    ProMdl mdlRenameComp;
    ProMdlfileType type;

    for (int iInputMdlCnt = 0; iInputMdlCnt < iSectionMaxRows; iInputMdlCnt++) {
        /*
        *  �����t�@�C����2��ȍ~Widchill�փ`�F�b�N�C���ł��Ȃ�����
        *�@Top�A�Z���u���Ɠ��l�ɘA�Ԃ�t�^����.
        *  �R���t�B�O���[�V�����t�@�C����AfterName�͎g�p���Ȃ�
        */

        ProWstringCopy(wTopAssyNewName, wAfterName, PRO_VALUE_UNUSED);
        ProWstringConcatenate(L"_", wAfterName, PRO_VALUE_UNUSED);
        ProWstringConcatenate(wType, wAfterName, PRO_VALUE_UNUSED);

        Split(strModulesRename->cAfterName, L"", wBeforeName);

        if (strstr(strModulesRename->cBeforeName, ".asm") != NULL) {
            type = PRO_MDLFILE_ASSEMBLY;

            // ������_(����).(�g���q)�̏ꍇ�̑Ή��B�Ƃ肠����9�܂őΉ�
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

            // ������_(����).(�g���q)�̏ꍇ�̑Ή��B�Ƃ肠����9�܂őΉ�
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

            // ������_(����).(�g���q)�̏ꍇ�̑Ή��B�Ƃ肠����9�܂őΉ�
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

        // ���O��ύX
        status = renameObject(wBeforeName, wAfterName, type);

        strModulesRename++;
    }

}

