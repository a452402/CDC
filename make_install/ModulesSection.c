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
    �}�N��
\*-----------------------------------------------------------------*/
// H
#define MOUNT_RIGHT         L"MOUNT_RIGHT"
#define MOUNT_LEFT          L"MOUNT_LEFT"
#define MOUNT               L"MOUNT"


// H�ɂ�cRefPartNumber(���ƕR�Â����i���)���������݂��邩�J�E���g���邽�߂̍\����
typedef struct
{
    wchar_t wRightRefPartNumber[INPUTFILE_MAXLINE];      // ���ƕR�Â����i���(���O���[�v��)
    wchar_t wLeftRefPartNumber[INPUTFILE_MAXLINE];      // ���ƕR�Â����i���(���O���[�v��)
    wchar_t wBothRefPartNumber[INPUTFILE_MAXLINE];      // ���ƕR�Â����i���(���O���[�v��)
    wchar_t wRefPartNumber[INPUTFILE_MAXLINE];      // ���ƕR�Â����i���(���O���[�v��)
    int iRightCounter;                                   // �J�E���^�[
    int iLeftCounter;                                   // �J�E���^�[
    int iBothCounter;                                   // �J�E���^�[
    int iCounter;                                   // �J�E���^�[
}holeRefCounter;

holeRefCounter* strHoleRefCounter;
holeRefCounter* strHoleRefCounterStart;

/*-----------------------------------------------------------------*\
    �v���g�^�C�v�錾
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
    Purpose:	Module�Z�N�V���� (rename��)
    InputFileRenameFile* strModulesRename   (in)    �R���t�B�O���[�V�����t�@�C���̒l
    int iSectionMaxRows                     (in)    �������ׂ��R���t�B�O���[�V�����t�@�C���̍s��
\*=========================================================================*/
ProError renameInModulesSection(InputFileRenameFile* strModulesRename, int iSectionMaxRows) {
    ProError status;
    wchar_t wAfterName[INPUTFILE_MAXLINE];         // Rename��̖��O
    wchar_t wBeforeName[INPUTFILE_MAXLINE];        // Rename�O�̖��O
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

        // ���O��ύX
        status = renameObject(wBeforeName, wAfterName, type);

        strModulesRename++;
    }
}

/*=========================================================================*\
    Function:	setModulesSection
    Purpose:	Module�Z�N�V���� (���W���[����)
                windchill����A�Z���u�������[�h���A�A�Z���u���Ώۂɑg�ݕt����
    ProMdl* top_asm                 (in)    �A�Z���u���Ώ�
    InputFileModules* strModules    (in)    �R���t�B�O���[�V�����t�@�C���̒l
    InputFileHole* strHole          (in)    �R���t�B�O���[�V�����t�@�C���̒l
    int iSectionMaxRows             (in)    �������ׂ��R���t�B�O���[�V�����t�@�C���̍s��
    int iHoleSectionMaxRows         (in)    �������ׂ��R���t�B�O���[�V�����t�@�C���̍s��
    int iSectionType                (in)    �Ăяo�����Z�N�V����
\*=========================================================================*/
ProError setModulesSection(ProMdl* top_asm, InputFileModules* strModules, InputFileHole* strHole, int iSectionMaxRows, int iHoleSectionMaxRows, int iSectionType) {
    ProError status;
    int iResultV;
    int iResultH;
    int iResultM;
    int iResultX;
    ProMdl mdlCsysPart;
    wchar_t wModules[INPUTFILE_MAXLINE];           // �Ώە��i��
    wchar_t wConstraint[INPUTFILE_MAXLINE];        // �S�����(V, H, M, X)
    wchar_t wConstraintType[INPUTFILE_MAXLINE];    // �S������(CSYS��)

    ProMdl *top_asm_org = top_asm;

    for (int iInputMdlCnt = 0; iInputMdlCnt < iSectionMaxRows; iInputMdlCnt++) {

        // ������
        mdlCsysPart = NULL;
        ProStringToWstring(wConstraint, strModules->cConstraint);
        ProStringToWstring(wConstraintType, strModules->cConstraintType);

        // �S����ނ̊m�F
        ProWstringCompare(L"V", wConstraint, PRO_VALUE_UNUSED, &iResultV);
        ProWstringCompare(L"H", wConstraint, PRO_VALUE_UNUSED, &iResultH);
        ProWstringCompare(L"M", wConstraint, PRO_VALUE_UNUSED, &iResultM);
        ProWstringCompare(L"X", wConstraint, PRO_VALUE_UNUSED, &iResultX);

        if (iResultV == 0) {
            /***********************************************
             V�̏���
            *************************************************/
            if (iSectionType == ENUM_INPUT_SECTION_MODULES) {
                /***********************************************
                 *MODULES 
                 �ȉ�2�ʂ�̑g�t�����s��
                 1,Top�A�Z���u�����̃p�[�c��CSYS����Ƃ���
                 2,1�����s�����ꍇ�ATop�A�Z���u�����̃A�Z���u�����̃p�[�c����CSYS����Ƃ���
                *************************************************/
                // Top�A�Z���u���̏�����
                top_asm = top_asm_org;

                // �S������(CSYS��)�����ƂɑΏۂ�mdl���擾
                status = serachCsysAssy(wConstraintType, iSectionType, top_asm, &mdlCsysPart);
                if (status != PRO_TK_NO_ERROR) {
                    // CSYS���猩�āATop�A�Z���u�����ύX�ɂȂ邽�߁ATop�A�Z���u����ProMdl
                    ProMdl top_asm_temp;

                    //Top�A�Z���u�����̃A�Z���u�����̃p�[�c����CSYS����Ƃ���
                    status = serachCsysAssyForModuleSectoin(wConstraintType, iSectionType, top_asm, &mdlCsysPart, &top_asm_temp);

                    status = setModulesSectionV2nd(top_asm , &top_asm_temp, mdlCsysPart, strModules);
                } else{
                    status = setModulesSectionV(top_asm, mdlCsysPart, strModules);
                }

            } else if (iSectionType == ENUM_INPUT_SECTION_FRONT_AXLE) {
                /***********************************************
                 *FRONT_AXLE
                 �ȉ��̑g�t�����s��
                 �ETop�A�Z���u������CSYS����Ƃ���
                *************************************************/
                setModulesSectionVM(top_asm, strModules);
            }
            else  {
                /***********************************************
                 *MODULES / *FRONT_AXLE �ȊO
                 �ȉ��̑g�t�����s��
                 �ETop�A�Z���u�����̃p�[�c��CSYS����Ƃ���
                *************************************************/
                // �S������(CSYS��)�����ƂɑΏۂ�mdl���擾
                status = serachCsysAssy(wConstraintType, iSectionType, top_asm, &mdlCsysPart);
                if (status != PRO_TK_NO_ERROR) {
                    // �z��O�̍S�������̏ꍇ��MAIN�Ƃ��Ĉ���
                    status = serachCsysAssy(L"MAIN", iSectionType, top_asm, &mdlCsysPart);

                    LOG_PRINT("    : %s : Connect as MAIN because CSYS has an abnormal value.", strModules->cModules);
                    strcpy(strModules->cConstraintType, "MAIN");
                }
                setModulesSectionV(top_asm, mdlCsysPart, strModules);

            }
        }
        else if (iResultH == 0) {
            /***********************************************
             H�̏���
            *************************************************/

            setModulesSectionH(top_asm, strModules, strHole, iHoleSectionMaxRows);
        }
        else if (iResultM == 0) {
            /***********************************************
             M�̏���
            *************************************************/
            if (iSectionType != ENUM_INPUT_SECTION_PROP_SHAFT) {
                setModulesSectionM(top_asm, strModules);
            }
            else {
                // *PROP_SHAFT ��Top�A�Z���u������CSYS����Ƃ���
                setModulesSectionVM(top_asm, strModules);
            }
        }
        else if (iResultX == 0) {
            /***********************************************
             X�̏���
            *************************************************/
            setModulesSectionM(top_asm, strModules);

        }
        else {
            // �z��O�̃G���[. �z��O�̍S�����(V, H, M, X)�ł�
            LOG_PRINT("NOK : %s %s : abnormal value", strModules->cModules, strModules->cConstraint);
        }
        strModules++;
    }
    status = ProSolidRegenerate((ProSolid)*top_asm, PRO_REGEN_FORCE_REGEN);
}

/*=========================================================================*\
    Function:	serachCsysAssyForModuleSectoin
    Purpose:	wConstraintType �����A�Ώۂ�mdl����������
    wchar_t* wConstraintType    (in)  CSYS��
    int iSectionType            (in)  �Ăяo�����Z�N�V����
    ProMdl* top_asm             (in)  �A�Z���u���Ώ�
    ProMdl* mdlCsysPart         (out) CSYS�������f���n���h��
    ProMdl* mdlTopAssyTemp         (out) CSYS�������f����Top�A�Z���u��
\*=========================================================================*/
ProError serachCsysAssyForModuleSectoin(wchar_t* wConstraintType, int iSectionType, ProMdl* top_asm, ProMdl* mdlCsysPart, ProMdl* mdlTopAssyTemp) {
    ProError status;
    UserVCsysAppData	local_app_data;
    wchar_t kakunin[INPUTFILE_MAXLINE];  // �W�F�l���b�N�p�����[�^

    // �l�̏�����
    local_app_data.model = NULL;
    local_app_data.p_csys = NULL;
    local_app_data.topmodelTemp = NULL;
    ProWstringCopy(wConstraintType, local_app_data.csys_name, PRO_VALUE_UNUSED);

    // TOP�A�Z���u���̂��ׂẴR���|�[�l���g���ċA�I�ɖK�₵�A�����t�B�[�`��ID���擾����
    status = ProSolidFeatVisit((ProSolid)*top_asm,
        searchTopAssyForModuleAction,
        searchTopAssyMOnlyFilter,
        (ProAppData)&local_app_data);

    if (local_app_data.model == NULL) {
        // CSYS�����z��O(CSYS��������Ȃ�����)�̏ꍇ�̓G���[
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
    Purpose:	wConstraintType �����A�Ώۂ�mdl����������
    wchar_t* wConstraintType    (in)  CSYS��
    int iSectionType            (in)  �Ăяo�����Z�N�V����
    ProMdl* top_asm             (in)  �A�Z���u���Ώ�
    ProMdl* mdlCsysPart         (out) CSYS�������f���n���h��
\*=========================================================================*/
ProError serachCsysAssy(wchar_t* wConstraintType, int iSectionType, ProMdl* top_asm, ProMdl* mdlCsysPart) {
    ProError status;
    UserVCsysAppData	local_app_data;
    wchar_t kakunin[INPUTFILE_MAXLINE];  // �W�F�l���b�N�p�����[�^

    // �l�̏�����
    local_app_data.model = NULL;
    local_app_data.p_csys = NULL;
    local_app_data.iSectionType = iSectionType;
    ProWstringCopy(wConstraintType, local_app_data.csys_name, PRO_VALUE_UNUSED);

    // TOP�A�Z���u���̂��ׂẴR���|�[�l���g���ċA�I�ɖK�₵�A�����t�B�[�`��ID���擾����
    status = ProSolidFeatVisit((ProSolid)*top_asm,
        searchTopAssyVOnlyAction,
        searchTopAssyVOnlyFilter,
        (ProAppData)&local_app_data);

    if (local_app_data.model == NULL) {
        // CSYS�����z��O(CSYS��������Ȃ�����)�̏ꍇ�̓G���[
        return PRO_TK_BAD_INPUTS;
    }

    *mdlCsysPart = local_app_data.model;

    ProMdlMdlnameGet(*top_asm, kakunin);
    ProMdlMdlnameGet(local_app_data.model, kakunin);

    return PRO_TK_NO_ERROR;
}


/*=========================================================================*\
    Function:	setModulesSectionV
    Purpose:	Module�Z�N�V���� (���W���[���� - V�̂�)
                windchill����A�Z���u�������[�h���A�A�Z���u���Ώۂɑg�ݕt����
    ProMdl top_asm                 (in)    Top�A�Z���u���̃n���h��
    ProMdl mdlCsysPart             (in)    �A�Z���u���Ώ�
    InputFileModules* strModules    (in)    �R���t�B�O���[�V�����t�@�C���̒l

    �ȉ��̂悤�ɁACSYS���m��g�ݕt����

    �g�b�v�A�Z���u��
    |-- �p�[�c (�g�t���� / CSYS�n)
    |    |-- �g�t���Ώ�CSYS
    |
    |-- �A�Z���u�� (�g�t����)
    |    |-- �g�t���Ώ�CSYS

\*=========================================================================*/
ProError setModulesSectionV(ProMdl* top_asm, ProMdl mdlCsysPart, InputFileModules* strModules) {
    ProError status;
    // �����}�g���b�N�X�B�K��
    ProMatrix identity_matrix = { { 1.0, 0.0, 0.0, 0.0 },
                             {0.0, 1.0, 0.0, 0.0},
                             {0.0, 0.0, 1.0, 0.0},
                             {0.0, 0.0, 0.0, 1.0} };

    /***********************************************
    �ǉ�����A�Z���u�����m�F����
    *************************************************/
    ProSolid comp_model = NULL;
    ProAsmcomp asmcomp;
    ProAsmcompconstraint* constraints;
    status = PRO_TK_NO_ERROR;

    wchar_t wModules[INPUTFILE_MAXLINE];           // �Ώە��i��
    wchar_t wConstraintType[INPUTFILE_MAXLINE];    // �S������(CSYS��)

    ProStringToWstring(wModules, strModules->cModules);
    ProStringToWstring(wConstraintType, strModules->cConstraintType);

    // �t�@�C�������[�h�ς݂����m�F����
    ProMdlInit(wModules, PRO_MDL_ASSEMBLY, (ProMdl*)&comp_model);

    if (comp_model == NULL) {
        // ���[�h���Ă��Ȃ��̂ŁAWindchill����A�Z���u�������[�h����
        status = searchAssypathFromWindchill(wModules, SUB_ASSY, PRO_MDLFILE_ASSEMBLY, (ProMdl*)&comp_model);
        if (status != PRO_TK_NO_ERROR) {
            return PRO_TK_GENERAL_ERROR;
        }
    }

    if (status == PRO_TK_NO_ERROR) {
        // �A�Z���u����ǉ�����
        status = ProAsmcompAssemble((ProAssembly)*top_asm, comp_model, identity_matrix, &asmcomp);

        // constraints�z��̏���
        status = ProArrayAlloc(0, sizeof(ProAsmcompconstraint), 1, (ProArray*)&constraints);

        // *FRONT_AXLE �ȊO�� �S�������̐ݒ� (PRO_CSYS)
        status = SetConstraintsForAddPart(mdlCsysPart, comp_model, *top_asm, wConstraintType, &constraints);

        if (status == PRO_TK_E_NOT_FOUND) {
            // SetConstraintsForAddPart���ŃG���[���b�Z�[�W���o���Ă���̂ł����ł͋L�ڂ��Ȃ�
            return PRO_TK_GENERAL_ERROR;
        }else if (status != PRO_TK_NO_ERROR) {
            LOG_PRINT("NOK : %s", strModules->cModules);
            return PRO_TK_GENERAL_ERROR;
        }
        //�A�Z���u���R���|�[�l���g�̍S����ݒ肵�A�A�Z���u�����Đ������܂�
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
    Purpose:	Module�Z�N�V���� (���W���[���� - V�̂�)
                windchill����A�Z���u�������[�h���A�A�Z���u���Ώۂɑg�ݕt����
    ProMdl top_asm                 (in)    Top�A�Z���u���̃n���h��
    ProMdl sub_asm                 (in)    Top�A�Z���u���̃n���h��
    ProMdl mdlCsysPart             (in)    �A�Z���u���Ώ�
    InputFileModules* strModules    (in)    �R���t�B�O���[�V�����t�@�C���̒l

    �ȉ��̂悤�ɁACSYS���m��g�ݕt����

    �g�b�v�A�Z���u��
    |-- Sub�A�Z���u��
    |    |-- �p�[�c (�g�t���� / CSYS�n)
    |        |-- �g�t���Ώ�CSYS
    |
    |-- �A�Z���u�� (�g�t����)
    |    |-- �g�t���Ώ�CSYS

\*=========================================================================*/
ProError setModulesSectionV2nd(ProMdl* top_asm, ProMdl* sub_asm, ProMdl mdlCsysPart, InputFileModules* strModules) {
    ProError status;
    // �����}�g���b�N�X�B�K��
    ProMatrix identity_matrix = { { 1.0, 0.0, 0.0, 0.0 },
                             {0.0, 1.0, 0.0, 0.0},
                             {0.0, 0.0, 1.0, 0.0},
                             {0.0, 0.0, 0.0, 1.0} };

    /***********************************************
    �ǉ�����A�Z���u�����m�F����
    *************************************************/
    ProSolid comp_model = NULL;
    ProAsmcomp asmcomp;
    ProAsmcompconstraint* constraints;
    status = PRO_TK_NO_ERROR;

    wchar_t wModules[INPUTFILE_MAXLINE];           // �Ώە��i��
    wchar_t wConstraintType[INPUTFILE_MAXLINE];    // �S������(CSYS��)

    ProStringToWstring(wModules, strModules->cModules);
    ProStringToWstring(wConstraintType, strModules->cConstraintType);

    // �t�@�C�������[�h�ς݂����m�F����
    ProMdlInit(wModules, PRO_MDL_ASSEMBLY, (ProMdl*)&comp_model);

    if (comp_model == NULL) {
        // ���[�h���Ă��Ȃ��̂ŁAWindchill����A�Z���u�������[�h����
        status = searchAssypathFromWindchill(wModules, SUB_ASSY, PRO_MDLFILE_ASSEMBLY, (ProMdl*)&comp_model);
        if (status != PRO_TK_NO_ERROR) {
            return PRO_TK_GENERAL_ERROR;
        }
    }

    if (status == PRO_TK_NO_ERROR) {
        // �R���|�[�l���g�̒ǉ�������
        status = ProAsmcompAssemble((ProAssembly)*top_asm, comp_model, identity_matrix, &asmcomp);

        // constraints�z��̏���
        status = ProArrayAlloc(0, sizeof(ProAsmcompconstraint), 1, (ProArray*)&constraints);

        // *FRONT_AXLE �ȊO�� �S�������̐ݒ� (PRO_CSYS)
        status = SetConstraintsForAddPart2nd(mdlCsysPart, *top_asm, comp_model, *sub_asm, wConstraintType, &constraints);

        if (status == PRO_TK_E_NOT_FOUND) {
            // SetConstraintsForAddPart2nd���ŃG���[���b�Z�[�W���o���Ă���̂ł����ł͋L�ڂ��Ȃ�
            return PRO_TK_GENERAL_ERROR;
        }
        else if (status != PRO_TK_NO_ERROR) {
            LOG_PRINT("NOK : %s", strModules->cModules);
            return PRO_TK_GENERAL_ERROR;
        }
        //�A�Z���u���R���|�[�l���g�̍S����ݒ肵�A�A�Z���u�����Đ������܂�
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
    Purpose:	Module�Z�N�V���� (*FRONT_AXLE �� V /  *PROP_SHAFT �� M)
                windchill����A�Z���u�������[�h���A�A�Z���u���Ώۂɑg�ݕt����

                TOP�A�Z���u��������CSYS�ƕR�Â���
    ProMdl top_asm                 (in)    Top�A�Z���u���̃n���h��
    InputFileModules* strModules    (in)    �R���t�B�O���[�V�����t�@�C���̒l

    �ȉ��̂悤�ɁACSYS���m��g�ݕt����

    �g�b�v�A�Z���u��
    |-- �g�t���Ώ�CSYS
    |
    |-- �A�Z���u�� (�g�t����)
    |    |-- �g�t���Ώ�CSYS

\*=========================================================================*/
ProError setModulesSectionVM(ProMdl* top_asm, InputFileModules* strModules) {
    ProError status;

    // �����}�g���b�N�X�B�K��
    ProMatrix identity_matrix = { { 1.0, 0.0, 0.0, 0.0 },
                             {0.0, 1.0, 0.0, 0.0},
                             {0.0, 0.0, 1.0, 0.0},
                             {0.0, 0.0, 0.0, 1.0} };


    /***********************************************
    �ǉ�����A�Z���u�����m�F����
    *************************************************/
    ProSolid comp_model = NULL;
    ProAsmcomp asmcomp;
    ProAsmcompconstraint* constraints;
    status = PRO_TK_NO_ERROR;

    wchar_t wModules[INPUTFILE_MAXLINE];           // �Ώە��i��
    wchar_t wConstraintType[INPUTFILE_MAXLINE];    // �S������(�g�t���A�Z���u����CSYS��)

    ProStringToWstring(wModules, strModules->cModules);
    ProStringToWstring(wConstraintType, strModules->cConstraintType);

    // �t�@�C�������[�h�ς݂����m�F����
    ProMdlInit(wModules, PRO_MDL_ASSEMBLY, (ProMdl*)&comp_model);

    if (comp_model == NULL) {
        // ���[�h���Ă��Ȃ��̂ŁAWindchill����A�Z���u�������[�h����
        status = searchAssypathFromWindchill(wModules, SUB_ASSY, PRO_MDLFILE_ASSEMBLY, (ProMdl*)&comp_model);
        if (status != PRO_TK_NO_ERROR) {
            return;
        }

    }

    if (status == PRO_TK_NO_ERROR) {
        // �A�Z���u����ǉ�����
        status = ProAsmcompAssemble((ProAssembly)*top_asm, comp_model, identity_matrix, &asmcomp);

        // constraints�z��̏���
        status = ProArrayAlloc(0, sizeof(ProAsmcompconstraint), 1, (ProArray*)&constraints);

        // *FRONT_AXLE �� �S�������̐ݒ� (PRO_CSYS)
        status = SetConstraintsForAddAssy(*top_asm, wConstraintType, comp_model, wConstraintType, &constraints);

        if (status == PRO_TK_E_NOT_FOUND) {
            // SetConstraintsForAddAssy���ɃG���[���b�Z�[�W���L��
            return;
        }else if (status != PRO_TK_NO_ERROR) {
            LOG_PRINT("NOK : %s", strModules->cModules);
            return;
        }
        //�A�Z���u���R���|�[�l���g�̍S����ݒ肵�A�A�Z���u�����Đ������܂�
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
PURPOSE:  �S�������̐ݒ� (PRO_CSYS)
ProMdl csys_part                        (in)�S����/�p�[�c (�g�t���� / CSYS�n)
ProMdl top_asm                          (in)�S����/TOP�A�Z���u��
ProMdl comp_model                       (in)�A�Z���u�� (�g�t����)
ProMdl sub_asm                          (in)Sub�A�Z���u��
wchar_t wCompSys[INPUTFILE_MAXLINE];    (in)�S����/����Csys��
ProAsmcompconstraint** constraints      (out)constraints�z��

    �ȉ��̂悤�ɁACSYS���m��g�ݕt����

    TOP�A�Z���u��                           top_asm
    |-- Sub�A�Z���u��                       sub_asm
    |    |-- �p�[�c (�g�t���� / CSYS�n)     csys_part
    |        |-- �g�t���Ώ�CSYS
    |
    |-- �A�Z���u�� (�g�t����)                comp_model
    |    |-- �g�t���Ώ�CSYS
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

    // ����m�F�p
    ProPath kakunin1;
    ProPath kakunin2;
    ProPath kakunin3;
    ProPath kakunin4;
    ProMdlMdlnameGet(csys_part, kakunin1);
    ProMdlMdlnameGet(top_asm, kakunin2);
    ProMdlMdlnameGet(comp_model, kakunin3);
    ProMdlMdlnameGet(sub_asm, kakunin4);

    /*-----------------------------------------------------------------*\
        asm_model (�S����/TOP�A�Z���u���ɕR�Â��Ă���CSYS�p�[�c) �� CSYS �̌���
    \*-----------------------------------------------------------------*/
    UserCsysAppData	app_data2;
    int id2;
    ProPath wName;

    // ��������p�[�c�̖��O��ݒ�
    ProMdlMdlnameGet(csys_part, wName);

    ProWstringCopy(wCompSys, app_data2.csys_name, PRO_VALUE_UNUSED);
    app_data2.model = csys_part;

    // CSYS����������
    status = ProSolidCsysVisit((ProSolid)csys_part,
        (ProCsysVisitAction)CsysFindVisitAction,
        (ProCsysFilterAction)CsysFindFilterAction,
        (ProAppData)&app_data2);

    if (status != PRO_TK_USER_ABORT) {
        LOG_PRINT("NOK : %w : Not found CSYS of %w", patName, wName);
        return(PRO_TK_E_NOT_FOUND);
    }

    // Modelitem�̎擾
    status = ProCsysIdGet(app_data2.p_csys, &id2);
    status = ProModelitemInit(app_data2.model, id2, PRO_CSYS, &p_asmintent);

    /*-----------------------------------------------------------------*\
        Asmcomppath ���擾
    \*-----------------------------------------------------------------*/
    DatumAppData	appdataFeature;
    ProIdTable c_id2_table;

    /*-----
     sub�A�Z���u�����̃p�[�c(patName)�̃t�B�[�`��������
    -----*/
    ProWstringCopy(wName, appdataFeature.name, PRO_VALUE_UNUSED);
    appdataFeature.iFindCnt = 0;
    ProSolidFeatVisit((ProSolid)sub_asm, getFeatureIdAction, NULL, (ProAppData)&appdataFeature);
    if (appdataFeature.iFindCnt == 0) {
        // �p�[�c(patName)�̃t�B�[�`����������Ȃ�����
        return PRO_TK_GENERAL_ERROR;
    }
    else {
        c_id2_table[1] = appdataFeature.feature.id;
    }

    /*-----
    Top�A�Z���u������Sub�A�Z���u���̃t�B�[�`��������
    -----*/
    ProMdlMdlnameGet(sub_asm, wName);
    ProWstringCopy(wName, appdataFeature.name, PRO_VALUE_UNUSED);
    appdataFeature.iFindCnt = 0;
    ProSolidFeatVisit((ProSolid)top_asm, getFeatureIdAction, NULL, (ProAppData)&appdataFeature);
    if (appdataFeature.iFindCnt == 0) {
        // �p�[�c(patName)�̃t�B�[�`����������Ȃ�����
        return PRO_TK_GENERAL_ERROR;
    }
    else {
        c_id2_table[0] = appdataFeature.feature.id;
    }

    status = ProAsmcomppathInit((ProSolid)top_asm, c_id2_table, 2, &asm_path);

    /*-----------------------------------------------------------------*\
        comp_model (�S������A�Z���u��)�R���|�[�l���g �� CSYS �̌���
    \*-----------------------------------------------------------------*/
    UserCsysAppData	app_data;
    int id;
    ProIdTable c_id_table;
    c_id_table[0] = -1;

    ProWstringCopy(wCompSys, app_data.csys_name, PRO_VALUE_UNUSED);

    app_data.model = comp_model;

    // CSYS����������
    status = ProSolidCsysVisit((ProSolid)comp_model,
        (ProCsysVisitAction)CsysFindVisitAction,
        (ProCsysFilterAction)CsysFindFilterAction,
        (ProAppData)&app_data);


    if (status != PRO_TK_USER_ABORT) {
        LOG_PRINT("NOK : %w : Not found CSYS of %w", patName, patName);
        return(PRO_TK_E_NOT_FOUND);
    }

    // Modelitem�̎擾
    status = ProCsysIdGet(app_data.p_csys, &id);
    status = ProModelitemInit(app_data.model, id, PRO_CSYS, &p_compintent);

    /*-----------------------------------------------------------------*\
        constraint�̊���
    \*-----------------------------------------------------------------*/
    status = ProSelectionAlloc(NULL, &p_compintent, &comp_sel);
    status = ProSelectionAlloc(&asm_path, &p_asmintent, &asm_sel);

    status = ProAsmcompconstraintAlloc(&constraint);

    // ���W�n�Ɉʒu�����킹��
    status = ProAsmcompconstraintTypeSet(constraint, PRO_ASM_CSYS);

    // �A�Z���u���̕�����ݒ�
    status = ProAsmcompconstraintAsmreferenceSet(constraint, asm_sel, PRO_DATUM_SIDE_YELLOW);

    // �R���|�[�l���g�̕�����ݒ�
    status = ProAsmcompconstraintCompreferenceSet(constraint, comp_sel, PRO_DATUM_SIDE_YELLOW);

    // constraints�z��̖�����constraint��ǉ�����
    status = ProArrayObjectAdd((ProArray*)constraints, PRO_VALUE_UNUSED, 1, &constraint);

    return PRO_TK_NO_ERROR;
}


/*=====================================================================*\
FUNCTION: SetConstraintsForAddPart
PURPOSE:  �S�������̐ݒ� (PRO_CSYS)
ProMdl asm_model                        (in)�S����/TOP�A�Z���u���ɕR�Â��Ă���CSYS�p�[�c
ProMdl comp_model                       (in)�S�����郂�f��
ProMdl* top_asm                         (in)�S�����TOP�A�Z���u��
wchar_t wCompSys[INPUTFILE_MAXLINE];    (in)�S����/����Csys��
ProAsmcompconstraint** constraints      (out)constraints�z��
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
        asm_model (�S����/TOP�A�Z���u���ɕR�Â��Ă���CSYS�p�[�c) �� CSYS �̌���
    \*-----------------------------------------------------------------*/
    UserCsysAppData	app_data2;
    int id2;
    ProPath wName;

    // ��������p�[�c�̖��O��ݒ�
    ProMdlMdlnameGet(asm_model, wName);

    ProWstringCopy(wCompSys, app_data2.csys_name, PRO_VALUE_UNUSED);
    app_data2.model = asm_model;

    // CSYS����������
    status = ProSolidCsysVisit((ProSolid)asm_model,
        (ProCsysVisitAction)CsysFindVisitAction,
        (ProCsysFilterAction)CsysFindFilterAction,
        (ProAppData)&app_data2);

    if (status != PRO_TK_USER_ABORT) {
        LOG_PRINT("NOK : %w : Not found CSYS of %w", patName, wName);
        return(PRO_TK_E_NOT_FOUND);
    }

    // Modelitem�̎擾
    status = ProCsysIdGet(app_data2.p_csys, &id2);
    status = ProModelitemInit(app_data2.model, id2, PRO_CSYS, &p_asmintent);

    /*-----------------------------------------------------------------*\
        Asmcomppath ���擾
    \*-----------------------------------------------------------------*/
    DatumAppData	appdataFeature;

    ProWstringCopy(wName, appdataFeature.name, PRO_VALUE_UNUSED);
    // ����������
    appdataFeature.iFindCnt = 0;

    // Top�p�[�c�̃p�[�c(patName)�̃t�B�[�`��������
    ProSolidFeatVisit((ProSolid)top_asm, getFeatureIdAction, NULL, (ProAppData)&appdataFeature);

    if (appdataFeature.iFindCnt == 0) {
        // �p�[�c(patName)�̃t�B�[�`����������Ȃ�����
        return PRO_TK_GENERAL_ERROR;
    }

    ProIdTable c_id2_table;
    c_id2_table[0] = appdataFeature.feature.id;
    status = ProAsmcomppathInit((ProSolid)top_asm, c_id2_table, 1, &asm_path);

    /*-----------------------------------------------------------------*\
        comp_model (�S������A�Z���u��)�R���|�[�l���g �� CSYS �̌���
    \*-----------------------------------------------------------------*/
    UserCsysAppData	app_data;
    int id;
    ProIdTable c_id_table;
    c_id_table[0] = -1;

    ProWstringCopy(wCompSys, app_data.csys_name, PRO_VALUE_UNUSED);

    app_data.model = comp_model;

    // CSYS����������
    status = ProSolidCsysVisit((ProSolid)comp_model,
        (ProCsysVisitAction)CsysFindVisitAction,
        (ProCsysFilterAction)CsysFindFilterAction,
        (ProAppData)&app_data);


    if (status != PRO_TK_USER_ABORT) {
        LOG_PRINT("NOK : %w : Not found CSYS of %w", patName, patName);
        return(PRO_TK_E_NOT_FOUND);
    }

    // Modelitem�̎擾
    status = ProCsysIdGet(app_data.p_csys, &id);
    status = ProModelitemInit(app_data.model, id, PRO_CSYS, &p_compintent);

    /*-----------------------------------------------------------------*\
        constraint�̊���
    \*-----------------------------------------------------------------*/
    status = ProSelectionAlloc(NULL, &p_compintent, &comp_sel);
    status = ProSelectionAlloc(&asm_path, &p_asmintent, &asm_sel);

    status = ProAsmcompconstraintAlloc(&constraint);

    // ���W�n�Ɉʒu�����킹��
    status = ProAsmcompconstraintTypeSet(constraint, PRO_ASM_CSYS);

    // �A�Z���u���̕�����ݒ�
    status = ProAsmcompconstraintAsmreferenceSet(constraint, asm_sel, PRO_DATUM_SIDE_YELLOW);

    // �R���|�[�l���g�̕�����ݒ�
    status = ProAsmcompconstraintCompreferenceSet(constraint, comp_sel, PRO_DATUM_SIDE_YELLOW);

    // constraints�z��̖�����constraint��ǉ�����
    status = ProArrayObjectAdd((ProArray*)constraints, PRO_VALUE_UNUSED, 1, &constraint);

    return PRO_TK_NO_ERROR;
}

/*=====================================================================*\
FUNCTION: SetConstraintsForAddAssy
PURPOSE:  �S�������̐ݒ� (PRO_CSYS)
ProMdl asm_model                        (in)�S����/TOP�A�Z���u��
wchar_t wAsmCsys[INPUTFILE_MAXLINE];    (in)�S����/TOP�A�Z���u�� / CSYS�̖��O
ProMdl comp_model                       (in)�S����/�S������A�Z���u��(Module�̓��e)
wchar_t wCmpCsys[INPUTFILE_MAXLINE];    (in)�S����/�S������A�Z���u��(Module�̓��e) / CSYS�̖��O
ProAsmcompconstraint** constraints      (out)constraints�z��
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
        comp_model �R���|�[�l���g �� CSYS �̌���
    \*-----------------------------------------------------------------*/
    UserCsysAppData	app_data;
    int id;
    ProPath patName;

    // ��������p�[�c�̖��O��ݒ�
    ProMdlMdlnameGet(comp_model, patName);

    ProWstringCopy(wCmpCsys, app_data.csys_name, PRO_VALUE_UNUSED);

    app_data.model = comp_model;

    // CSYS����������
    status = ProSolidCsysVisit((ProSolid)comp_model,
        (ProCsysVisitAction)CsysFindVisitAction,
        (ProCsysFilterAction)CsysFindFilterAction,
        (ProAppData)&app_data);

    if (status != PRO_TK_USER_ABORT) {
        LOG_PRINT("NOK : %w : Not found CSYS of %w", patName, patName);
        return(PRO_TK_E_NOT_FOUND);
    }

    // Modelitem�̎擾
    status = ProCsysIdGet(app_data.p_csys, &id);
    status = ProModelitemInit(app_data.model, id, PRO_CSYS, &p_compintent);


    /*-----------------------------------------------------------------*\
        asm_model �A�Z���u�� �� CSYS �̌���
    \*-----------------------------------------------------------------*/
    UserCsysAppData	app_data2;
    int id2;
    ProMdl p_owner;
    ProIdTable c_id_table;
    c_id_table[0] = -1;
    ProPath wName;
    // ��������p�[�c�̖��O��ݒ�
    ProMdlMdlnameGet(asm_model, wName);

    ProWstringCopy(wAsmCsys, app_data2.csys_name, PRO_VALUE_UNUSED);
    app_data2.model = asm_model;

    // CSYS����������
    status = ProSolidCsysVisit((ProSolid)asm_model,
        (ProCsysVisitAction)CsysFindVisitAction,
        (ProCsysFilterAction)CsysFindFilterAction,
        (ProAppData)&app_data2);

    if (status != PRO_TK_USER_ABORT) {
        LOG_PRINT("NOK : %w : Not found CSYS of %w", patName , wName);
        return(PRO_TK_E_NOT_FOUND);
    }

    // Modelitem�̎擾
    status = ProCsysIdGet(app_data2.p_csys, &id2);
    status = ProModelitemInit(app_data2.model, id2, PRO_CSYS, &p_asmintent);

    // Asmcomppath ���擾
    status = ProAsmcomppathInit((ProSolid)asm_model, c_id_table, 0, &comp_path);

    /*-----------------------------------------------------------------*\
        constraint�̊���
    \*-----------------------------------------------------------------*/
    status = ProSelectionAlloc(NULL, &p_compintent, &comp_sel);
    status = ProSelectionAlloc(&comp_path, &p_asmintent, &asm_sel);

    status = ProAsmcompconstraintAlloc(&constraint);

    // ���W�n�Ɉʒu�����킹��
    status = ProAsmcompconstraintTypeSet(constraint, PRO_ASM_CSYS);

    // �A�Z���u���̕�����ݒ�
    status = ProAsmcompconstraintAsmreferenceSet(constraint, asm_sel, PRO_DATUM_SIDE_YELLOW);

    // �R���|�[�l���g�̕�����ݒ�
    status = ProAsmcompconstraintCompreferenceSet(constraint, comp_sel, PRO_DATUM_SIDE_YELLOW);

    // constraints�z��̖�����constraint��ǉ�����
    status = ProArrayObjectAdd((ProArray*)constraints, PRO_VALUE_UNUSED, 1, &constraint);

    return PRO_TK_NO_ERROR;
}


/*=========================================================================*\
    Function:	setModulesSectionH
    Purpose:	Module�Z�N�V���� (���W���[���� - H�̂�)
                windchill����A�Z���u�������[�h���A�A�Z���u���Ώۂɑg�ݕt����
    ProMdl top_asm                  (in)    Top�A�Z���u���̃n���h��
    InputFileModules* strModules    (in)    �R���t�B�O���[�V�����t�@�C���̒l
    InputFileHole* strHole          (in)    �R���t�B�O���[�V�����t�@�C���̒l�iHOLE�j
    int iHoleSectionMaxRows         (in)    Hole�̏�������s��


    �ȉ��̂悤�ɁACSYS���m��g�ݕt����
    ����������B�̏ꍇ�͑g�t���A�Z���u����2�񃍁[�h���Ă��ꂼ��őg�ݕt����

    �g�b�v�A�Z���u��
    |-- �O���[�v_CSYS_*** (�g�t���� / ���������ɍ쐬����CSYS)
    |    |-- �g�t���Ώ�CSYS
    |
    |-- �A�Z���u�� (�g�t����)
    |    |-- �g�t���Ώ�CSYS (MOUNT/MOUNT_LEFT/MOUNT_RIGHT)
\*=========================================================================*/
ProError setModulesSectionH(ProMdl* top_asm, InputFileModules* strModules, InputFileHole* strHole, int iHoleSectionMaxRows) {
    ProError status;
    wchar_t wRefPartNumber[INPUTFILE_MAXLINE];      // ���ƕ��i���R�Â��Ă���
    wchar_t wSide[INPUTFILE_MAXLINE];               // �������������. L(��)/R(�E)/B(����)
    wchar_t wHoleGroupName[INPUTFILE_MAXLINE];      // ���O���[�v��
    wchar_t wModules[INPUTFILE_MAXLINE];
    wchar_t wHoleCsysNameL[INPUTFILE_MAXLINE];    // �S������(���O���[�v��CSYS��)
    wchar_t wHoleCsysNameR[INPUTFILE_MAXLINE];    // �S������(���O���[�v��CSYS��)
    wchar_t wHoleCsysNameB[INPUTFILE_MAXLINE];    // �S������(���O���[�v��CSYS��)
    int iResult;
    int iCount = 0;

    ProStringToWstring(wModules, strModules->cModules);

    /***********************************************
      ������H�̎��AcRefPartNumber(���ƕR�Â����i���)���������݂���ꍇ�A
      CSYS�����ς��̂ŁA�����J�E���g����
    *************************************************/
    strHoleRefCounter = (holeRefCounter*)calloc(iHoleSectionMaxRows, sizeof(holeRefCounter));
    if (!strHoleRefCounter) {
        // �������s��
        LOG_PRINT("NOK : Not enough memory");
        return;
    }

    // �J�n�n�_�A�h���X���m��
    strHoleRefCounterStart = strHoleRefCounter;

    for (int iInputMdlCnt = 0; iInputMdlCnt < iHoleSectionMaxRows; iInputMdlCnt++) {

        // ���ƕR�Â����i�����擾
        ProStringToWstring(wRefPartNumber, strHole->cRefPartNumber);

        /***********************************************
          �g�t�����Hole��������
        *************************************************/
        // �S����ނ̊m�F
        ProWstringCompare(wModules, wRefPartNumber, PRO_VALUE_UNUSED, &iResult);

        if (iResult == 0) {
            ProStringToWstring(wHoleGroupName, strHole->cHoleGroupName);

            // �J�n�n�_�̃A�h���X�ɖ߂�
            strHoleRefCounter = strHoleRefCounterStart;

            // ���ƕR�Â����i��񂪕�������ꍇ��CSYS�����ς�邽�߁A�����J�E���g����
            for (int iRefCount = 0; iRefCount < iHoleSectionMaxRows; iRefCount++) {
                ProStringToWstring(wSide, strHole->cSide);
                int iResultR;
                int iResultL;
                int iResultB;
                // �g�t�����CSYS��
                ProWstringCompare(L"R", wSide, PRO_VALUE_UNUSED, &iResultR);
                ProWstringCompare(L"L", wSide, PRO_VALUE_UNUSED, &iResultL);
                ProWstringCompare(L"B", wSide, PRO_VALUE_UNUSED, &iResultB);

                if (iResultR == 0) {
                    ProWstringCompare(strHoleRefCounter->wRightRefPartNumber, L"", PRO_VALUE_UNUSED, &iResult);
                    if (iResult == 0) {
                        // ��̏ꍇ�A�����l������
                        ProWstringCopy(wHoleGroupName, strHoleRefCounter->wRightRefPartNumber, PRO_VALUE_UNUSED);
                        strHoleRefCounter->iRightCounter = 0;
                        strHoleRefCounter->iCounter = strHoleRefCounter->iRightCounter;
                        break;
                    }

                    ProWstringCompare(strHoleRefCounter->wRightRefPartNumber, wHoleGroupName, PRO_VALUE_UNUSED, &iResult);
                    if (iResult == 0) {
                        // ��łȂ��ꍇ�͌��ƕR�Â����i���Ɣ�r���A��v������΃J�E���^����
                        strHoleRefCounter->iRightCounter = strHoleRefCounter->iRightCounter + 1;
                        strHoleRefCounter->iCounter = strHoleRefCounter->iRightCounter;
                        break;
                    }
                }
                else if (iResultL == 0) {
                    ProWstringCompare(strHoleRefCounter->wLeftRefPartNumber, L"", PRO_VALUE_UNUSED, &iResult);
                    if (iResult == 0) {
                        // ��̏ꍇ�A�����l������
                        ProWstringCopy(wHoleGroupName, strHoleRefCounter->wLeftRefPartNumber, PRO_VALUE_UNUSED);
                        strHoleRefCounter->iLeftCounter = 0;
                        strHoleRefCounter->iCounter = strHoleRefCounter->iLeftCounter;
                        break;
                    }

                    ProWstringCompare(strHoleRefCounter->wLeftRefPartNumber, wHoleGroupName, PRO_VALUE_UNUSED, &iResult);
                    if (iResult == 0) {
                        // ��łȂ��ꍇ�͌��ƕR�Â����i���Ɣ�r���A��v������΃J�E���^����
                        strHoleRefCounter->iLeftCounter = strHoleRefCounter->iLeftCounter + 1;
                        strHoleRefCounter->iCounter = strHoleRefCounter->iLeftCounter;
                        break;
                    }
                }
                else if (iResultB == 0) {
                    ProWstringCompare(strHoleRefCounter->wBothRefPartNumber, L"", PRO_VALUE_UNUSED, &iResult);
                    if (iResult == 0) {
                        // ��̏ꍇ�A�����l������
                        ProWstringCopy(wHoleGroupName, strHoleRefCounter->wBothRefPartNumber, PRO_VALUE_UNUSED);
                        strHoleRefCounter->iBothCounter = 0;
                        strHoleRefCounter->iCounter = strHoleRefCounter->iBothCounter;
                        break;
                    }

                    ProWstringCompare(strHoleRefCounter->wBothRefPartNumber, wHoleGroupName, PRO_VALUE_UNUSED, &iResult);
                    if (iResult == 0) {
                        // ��łȂ��ꍇ�͌��ƕR�Â����i���Ɣ�r���A��v������΃J�E���^����
                        strHoleRefCounter->iBothCounter = strHoleRefCounter->iBothCounter + 1;
                        strHoleRefCounter->iCounter = strHoleRefCounter->iBothCounter;
                        break;
                    }
                }
                strHoleRefCounter++;
            }

            // ���O���[�v������CSYS�����擾����
            ProWstringCopy(wHoleGroupName, wHoleCsysNameL, PRO_VALUE_UNUSED);
            ProWstringCopy(wHoleGroupName, wHoleCsysNameR, PRO_VALUE_UNUSED);
            ProWstringCopy(wHoleGroupName, wHoleCsysNameB, PRO_VALUE_UNUSED);
            ProWstringConcatenate(L"_L", wHoleCsysNameL, PRO_VALUE_UNUSED);
            ProWstringConcatenate(L"_R", wHoleCsysNameR, PRO_VALUE_UNUSED);
            ProWstringConcatenate(L"_B", wHoleCsysNameB, PRO_VALUE_UNUSED);

            if (strHoleRefCounter->iCounter != 0) {
                wchar_t wCounter[INPUTFILE_MAXLINE];
                _itow_s(strHoleRefCounter->iCounter, wCounter, sizeof(wCounter), 10);//�ϊ��p�֐�,10�i���ŕϊ�

                ProWstringConcatenate(L"_", wHoleCsysNameL, PRO_VALUE_UNUSED);
                ProWstringConcatenate(L"_", wHoleCsysNameR, PRO_VALUE_UNUSED);
                ProWstringConcatenate(L"_", wHoleCsysNameB, PRO_VALUE_UNUSED);
                ProWstringConcatenate(wCounter, wHoleCsysNameL, PRO_VALUE_UNUSED);
                ProWstringConcatenate(wCounter, wHoleCsysNameR, PRO_VALUE_UNUSED);
                ProWstringConcatenate(wCounter, wHoleCsysNameB, PRO_VALUE_UNUSED);
            }

            // �������������. L(��)/R(�E)/B(����)
            ProStringToWstring(wSide, strHole->cSide);

            int iResultR;
            int iResultL;
            int iResultB;
            // �g�t�����CSYS��
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
    Purpose:	Module�Z�N�V���� (���W���[���� - H�̂�)
                windchill����A�Z���u�������[�h���A�A�Z���u���Ώۂɑg�ݕt����

    ProMdl top_asm                 (in)    Top�A�Z���u���̃n���h��
    InputFileModules* strModules    (in)    �R���t�B�O���[�V�����t�@�C���̒l

\*=========================================================================*/
ProError setModulesSectionH_Assembly(ProMdl* top_asm, wchar_t wModules[INPUTFILE_MAXLINE], wchar_t wHoleCsysName[INPUTFILE_MAXLINE], wchar_t wConstraintType[INPUTFILE_MAXLINE]) {

    ProError status = PRO_TK_NO_ERROR;
    ProAsmcompconstraint* constraints;


    // �����}�g���b�N�X�B�K��
    ProMatrix identity_matrix = { { 1.0, 0.0, 0.0, 0.0 },
                             {0.0, 1.0, 0.0, 0.0},
                             {0.0, 0.0, 1.0, 0.0},
                             {0.0, 0.0, 0.0, 1.0} };

    /***********************************************
    �ǉ�����A�Z���u�����m�F����
    *************************************************/
    ProSolid comp_model = NULL;
    ProAsmcomp asmcomp;

    // �t�@�C�������[�h�ς݂����m�F����
    ProMdlInit(wModules, PRO_MDL_ASSEMBLY, (ProMdl*)&comp_model);

    if (comp_model == NULL) {
        // ���[�h���Ă��Ȃ��̂ŁAWindchill����A�Z���u�������[�h����
        status = searchAssypathFromWindchill(wModules, SUB_ASSY, PRO_MDLFILE_ASSEMBLY, (ProMdl*)&comp_model);
        if (status != PRO_TK_NO_ERROR) {
            return;
        }
    }
    /***********************************************
    �g�t�������J�n
    *************************************************/

    // �A�Z���u����ǉ�����
    status = ProAsmcompAssemble((ProAssembly)*top_asm, comp_model, identity_matrix, &asmcomp);

    // constraints�z��̏���
    status = ProArrayAlloc(0, sizeof(ProAsmcompconstraint), 1, (ProArray*)&constraints);

    // �S�������̐ݒ� (PRO_CSYS)
    status = SetConstraintsForAddAssy(*top_asm, wHoleCsysName, comp_model, wConstraintType, &constraints);

    if (status == PRO_TK_E_NOT_FOUND) {
        // SetConstraintsForAddAssy���ɃG���[���b�Z�[�W���L��
        return;
    }else if (status != PRO_TK_NO_ERROR) {
        LOG_PRINT("NOK : %w", wModules);
        return;
    }
    //�A�Z���u���R���|�[�l���g�̍S����ݒ肵�A�A�Z���u�����Đ������܂�
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
    Purpose:	Module�Z�N�V���� (���W���[���� - M�̂�)
                windchill����A�Z���u�������[�h���A�A�Z���u���Ώۂɑg�ݕt����
    ProMdl top_asm                 (in)    Top�A�Z���u���̃n���h��
    InputFileModules* strModules    (in)    �R���t�B�O���[�V�����t�@�C���̒l

    �ȉ��̂悤�ɁA�A�Z���u������CSYS���m��g�ݕt����

    �g�b�v�A�Z���u��
    |-- �A�Z���u�� (�g�t���� / V��H�Œǉ�)
    |    |-- �g�t���Ώ�CSYS
    |
    |-- �A�Z���u�� (�g�t����)
    |    |-- �g�t���Ώ�CSYS

\*=========================================================================*/
ProError setModulesSectionM(ProMdl* top_asm, InputFileModules* strModules) {
    ProError status;


    // �����}�g���b�N�X�B�K��
    ProMatrix identity_matrix = { { 1.0, 0.0, 0.0, 0.0 },
                             {0.0, 1.0, 0.0, 0.0},
                             {0.0, 0.0, 1.0, 0.0},
                             {0.0, 0.0, 0.0, 1.0} };


    /***********************************************
    �ǉ�����A�Z���u�����m�F����
    *************************************************/
    ProSolid comp_model = NULL;
    ProAsmcomp asmcomp;
    ProAsmcompconstraint* constraints;
    status = PRO_TK_NO_ERROR;

    wchar_t wModules[INPUTFILE_MAXLINE];           // �Ώە��i��
    wchar_t wConstraintType[INPUTFILE_MAXLINE];    // �S������(CSYS��)

    ProStringToWstring(wModules, strModules->cModules);
    ProStringToWstring(wConstraintType, strModules->cConstraintType);

    // �t�@�C�������[�h�ς݂����m�F����
    ProMdlInit(wModules, PRO_MDL_ASSEMBLY, (ProMdl*)&comp_model);

    if (comp_model == NULL) {
        // ���[�h���Ă��Ȃ��̂ŁAWindchill����A�Z���u�������[�h����
        status = searchAssypathFromWindchill(wModules, SUB_ASSY, PRO_MDLFILE_ASSEMBLY, (ProMdl*)&comp_model);
        if (status != PRO_TK_NO_ERROR) {
            return;
        }

    }

    if (status == PRO_TK_NO_ERROR) {
        // �A�Z���u����ǉ�����
        status = ProAsmcompAssemble((ProAssembly)*top_asm, comp_model, identity_matrix, &asmcomp);

        // constraints�z��̏���
        status = ProArrayAlloc(0, sizeof(ProAsmcompconstraint), 1, (ProArray*)&constraints);

        // �S�������̐ݒ� (PRO_CSYS)
        status = SetConstraintsForAddPart_M(comp_model, *top_asm, wConstraintType, &constraints);

        if (status == PRO_TK_E_NOT_FOUND) {
            return;
        }else if (status != PRO_TK_NO_ERROR) {
            LOG_PRINT("NOK : %s", strModules->cModules);
            return;
        }
        //�A�Z���u���R���|�[�l���g�̍S����ݒ肵�A�A�Z���u�����Đ������܂�
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
PURPOSE:  �S�������̐ݒ� (PRO_CSYS)
ProMdl comp_model                       (in)�S�����郂�f��
ProMdl* top_asm                         (in)�S�����TOP�A�Z���u��
wchar_t wCompSys[INPUTFILE_MAXLINE];    (in)�S����/����Csys��
ProAsmcompconstraint** constraints      (out)constraints�z��
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
    // ��������p�[�c�̖��O��ݒ�
    ProMdlMdlnameGet(comp_model, wLogName);
    /*-----------------------------------------------------------------*\
         (�S����/ V��H�Œǉ����ꂽ�A�Z���u��) �� CSYS �̌���
    \*-----------------------------------------------------------------*/
    ProPath wTopName;
    UserCsysAppData	app_data2;
    int id2;

    // ��������p�[�c�̖��O��ݒ�
    ProMdlMdlnameGet(top_asm, wTopName);

    ProWstringCopy(wCompSys, app_data2.csys_name, PRO_VALUE_UNUSED);
    app_data2.p_csys = NULL;

    // VH�̃A�Z���u���ꗗ���m�F����
    // TOP�A�Z���u���̂��ׂẴR���|�[�l���g���ċA�I�ɖK�₵�A�����t�B�[�`��ID���擾����
    status = ProSolidFeatVisit((ProSolid)top_asm,
        searchTopAssyMOnlyAction,
        searchTopAssyMOnlyFilter,
        (ProAppData)&app_data2);

    if (app_data2.p_csys == NULL) {
        LOG_PRINT("NOK : %w : Not found CSYS of %w", wLogName, wTopName);
        return(PRO_TK_E_NOT_FOUND);
    }

    // Modelitem�̎擾
    status = ProCsysIdGet(app_data2.p_csys, &id2);
    status = ProModelitemInit(app_data2.model, id2, PRO_CSYS, &p_asmintent);

    /*-----------------------------------------------------------------*\
        Asmcomppath ���擾
    \*-----------------------------------------------------------------*/
    ProPath patName;
    DatumAppData	appdataFeature;

    // ��������p�[�c�̖��O��ݒ�
    ProMdlMdlnameGet(app_data2.model, patName);
    ProWstringCopy(patName, appdataFeature.name, PRO_VALUE_UNUSED);
    // ����������
    appdataFeature.iFindCnt = 0;

    // Top�p�[�c�̃p�[�c(patName)�̃t�B�[�`��������
    ProSolidFeatVisit((ProSolid)top_asm, getFeatureIdFromCompAction, NULL, (ProAppData)&appdataFeature);

    if (appdataFeature.iFindCnt == 0) {
        // �p�[�c(patName)�̃t�B�[�`����������Ȃ�����
        return PRO_TK_GENERAL_ERROR;
    }

    ProIdTable c_id2_table;
    c_id2_table[0] = appdataFeature.feature.id;


    status = ProAsmcomppathInit((ProSolid)top_asm, c_id2_table, 1, &asm_path);

    /*-----------------------------------------------------------------*\
        comp_model (�S������A�Z���u��)�R���|�[�l���g �� CSYS �̌���
    \*-----------------------------------------------------------------*/
    UserCsysAppData	app_data;
    int id;
    ProIdTable c_id_table;
    c_id_table[0] = -1;

    ProWstringCopy(wCompSys, app_data.csys_name, PRO_VALUE_UNUSED);

    app_data.model = comp_model;

    // CSYS����������
    status = ProSolidCsysVisit((ProSolid)comp_model,
        (ProCsysVisitAction)CsysFindVisitAction,
        (ProCsysFilterAction)CsysFindFilterAction,
        (ProAppData)&app_data);

    if (status != PRO_TK_USER_ABORT) {
        LOG_PRINT("NOK : %w : Not found CSYS of %w", wLogName, wLogName);
        return(PRO_TK_E_NOT_FOUND);
    }

    // Modelitem�̎擾
    status = ProCsysIdGet(app_data.p_csys, &id);
    status = ProModelitemInit(app_data.model, id, PRO_CSYS, &p_compintent);

    /*-----------------------------------------------------------------*\
        constraint�̊���
    \*-----------------------------------------------------------------*/
    status = ProSelectionAlloc(NULL, &p_compintent, &comp_sel);
    status = ProSelectionAlloc(&asm_path, &p_asmintent, &asm_sel);

    status = ProAsmcompconstraintAlloc(&constraint);

    // ���W�n�Ɉʒu�����킹��
    status = ProAsmcompconstraintTypeSet(constraint, PRO_ASM_CSYS);

    // �A�Z���u���̕�����ݒ�
    status = ProAsmcompconstraintAsmreferenceSet(constraint, asm_sel, PRO_DATUM_SIDE_YELLOW);

    // �R���|�[�l���g�̕�����ݒ�
    status = ProAsmcompconstraintCompreferenceSet(constraint, comp_sel, PRO_DATUM_SIDE_YELLOW);

    // constraints�z��̖�����constraint��ǉ�����
    status = ProArrayObjectAdd((ProArray*)constraints, PRO_VALUE_UNUSED, 1, &constraint);

    return PRO_TK_NO_ERROR;
}

/*====================================================*\
  Function : FeatureIDGetFilter()
  Purpose  : Top�A�Z���u�����̃A�Z���u�����擾����
\*====================================================*/
ProError searchTopAssyMOnlyFilter(ProFeature* p_feature, ProAppData app_data)
{

    ProFeattype ftype = NULL;
    ProMdl mdl;
    ProMdlType type;
    ProMdlName name;

    // �t�B�[�`���^�C�v���擾 (�f�[�^��, CSYS, �R���|�[�l���g ...)
    ProFeatureTypeGet(p_feature, &ftype);
    if (ftype != PRO_FEAT_COMPONENT)
    {
        return PRO_TK_CONTINUE;
    }

    ProAsmcompMdlGet((ProAsmcomp*)p_feature, &mdl);

    // �A�Z���u���̏ꍇ�̂ݏ�������
    ProMdlTypeGet(mdl, &type);
    if (type != PRO_MDL_ASSEMBLY)
    {
        return PRO_TK_CONTINUE;
    }
    return(PRO_TK_NO_ERROR);
}
/*====================================================*\
  Function : searchTopAssyVOnlyFilter()
  Purpose  : Top�A�Z���u�����̃p�[�c���擾����
\*====================================================*/
ProError searchTopAssyVOnlyFilter(ProFeature* p_feature, ProAppData app_data)
{

    ProFeattype ftype = NULL;
    ProMdl mdl;
    ProMdlType type;
    ProMdlName name;

    // �t�B�[�`���^�C�v���擾 (�f�[�^��, CSYS, �R���|�[�l���g ...)
    ProFeatureTypeGet(p_feature, &ftype);
    if (ftype != PRO_FEAT_COMPONENT)
    {
        return PRO_TK_CONTINUE;
    }

    ProAsmcompMdlGet((ProAsmcomp*)p_feature, &mdl);

    // �A�Z���u���̏ꍇ�̂ݏ�������
    ProMdlTypeGet(mdl, &type);
    if (type != PRO_MDL_PART)
    {
        return PRO_TK_CONTINUE;
    }
    return(PRO_TK_NO_ERROR);
}


/*====================================================*\
  Function : searchTopAssyMOnlyAction()
  Purpose  : Top�A�Z���u�����̃A�Z���u����CSYS����������
\*====================================================*/
ProError  searchTopAssyMOnlyAction(ProFeature* p_feature, ProError status, ProAppData app_data)
{
    ProMdlfileType     mdltype;
    ProMdlName        w_name;
    int iResult;
    ProMdl mdl;
    ProMdlType		mdl_type;

    UserCsysAppData	local_app_data;

    // �A�Z���u���̃n���h�����擾
    ProAsmcompMdlGet((ProAsmcomp*)p_feature, &mdl);

    if (((UserCsysAppData*)app_data)->p_csys == NULL) {
        ProWstringCopy(((UserCsysAppData*)app_data)->csys_name, local_app_data.csys_name, PRO_VALUE_UNUSED);
        local_app_data.model = mdl;

        local_app_data.p_csys = NULL;

        // CSYS����������
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
    // ���������A�N�Z�X����
    return PRO_TK_NO_ERROR;
}


/*====================================================*\
  Function : searchTopAssyVOnlyAction()
  Purpose  : Top�A�Z���u�����̃A�Z���u����CSYS���������ACoordinateSystems�Ɣ�r���Ĉ�v�������݂̂̂���������
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
        // GEARBOX�̏ꍇ
        strCsys = gstrGearboxCoordinateSystems;
        iCount = giSectionRows[ENUM_INPUT_SECTION_GEARBOX_COORDINATE_SYSTEMS];
    }
    else {
        // GEARBOX�ȊO�̏ꍇ
        strCsys = gstrVehicleCoordinateSystems;
        iCount = giSectionRows[ENUM_INPUT_SECTION_VEHICLE_COORDINATE_SYSTEMS];
    }
    // �A�Z���u���̃n���h�����擾
    ProAsmcompMdlGet((ProAsmcomp*)p_feature, &mdl);

    if (((UserVCsysAppData*)app_data)->p_csys == NULL) {
        wchar_t wGenericInConfigFile[INPUTFILE_MAXLINE];  // �W�F�l���b�N�p�����[�^
        wchar_t wSearchCsysPart[INPUTFILE_MAXLINE];  // �W�F�l���b�N�p�����[�^
        wchar_t wKakunin[INPUTFILE_MAXLINE];  // �m�F�p
        int iReasult;

        for (int iLoop = 0; iLoop < iCount; iLoop++ ) {
            // �����Ώۂ�mdl���擾
            ProStringToWstring(wGenericInConfigFile, strCsys->cInstanceParameter);
            ProMdlMdlnameGet(mdl, wSearchCsysPart);

            ProWstringCompare(wGenericInConfigFile, wSearchCsysPart, PRO_VALUE_UNUSED, &iReasult);

            if (iReasult == NULL) {
                ProWstringCopy(((UserVCsysAppData*)app_data)->csys_name, local_app_data.csys_name, PRO_VALUE_UNUSED);
                local_app_data.model = mdl;
                local_app_data.p_csys = NULL;

                // CSYS����������
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
    // ���������A�N�Z�X����
    return PRO_TK_NO_ERROR;
}


/*====================================================*\
  Function : searchTopAssyForModuleAction()
  Purpose  : Top�A�Z���u�����̃A�Z���u��������.
                CoordinateSystems�Ɣ�r���Ĉ�v�������݂̂̂���������
\*====================================================*/
ProError  searchTopAssyForModuleAction(ProFeature* p_feature, ProError status, ProAppData app_data)
{
    UserVCsysAppData	local_app_data;
    wchar_t kakunin[INPUTFILE_MAXLINE];  // �W�F�l���b�N�p�����[�^
    ProMdl mdl;

    // �A�Z���u���̃n���h�����擾
    status = ProAsmcompMdlGet((ProAsmcomp*)p_feature, &mdl);


    // �l�̏�����
    local_app_data.model = NULL;
    local_app_data.p_csys = NULL;
    
    ProWstringCopy(((UserVCsysAppData*)app_data)->csys_name, local_app_data.csys_name, PRO_VALUE_UNUSED);

    ProMdlMdlnameGet(mdl, kakunin);

    // �A�Z���u���̂��ׂẴR���|�[�l���g���ċA�I�ɖK�₵�A�����t�B�[�`��ID���擾����
    status = ProSolidFeatVisit((ProSolid)mdl,
        searchTopAssyVOnlyActionForModuleSection,
        searchTopAssyVOnlyFilter,
        (ProAppData)&local_app_data);

    if (local_app_data.model != NULL) {
        //CSYS�����������ꍇ,���[�v������
        ((UserVCsysAppData*)app_data)->model = local_app_data.model;
        ((UserVCsysAppData*)app_data)->topmodelTemp = mdl;
        return PRO_TK_BAD_INPUTS;
    }

    ProMdlMdlnameGet(local_app_data.model, kakunin);

    // ���������A�N�Z�X����
    return PRO_TK_NO_ERROR;
}



/*====================================================*\
  Function : searchTopAssyVOnlyActionForModuleSection()
  Purpose  : Top�A�Z���u�����̃A�Z���u���̉��̃p�[�c����CSYS���������ACoordinateSystems�Ɣ�r���Ĉ�v�������݂̂̂���������
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

    // �A�Z���u���̃n���h�����擾
    ProAsmcompMdlGet((ProAsmcomp*)p_feature, &mdl);

    if (((UserVCsysAppData*)app_data)->p_csys == NULL) {
        wchar_t wGenericInConfigFile[INPUTFILE_MAXLINE];  // �W�F�l���b�N�p�����[�^
        wchar_t wSearchCsysPart[INPUTFILE_MAXLINE];  // �W�F�l���b�N�p�����[�^
        wchar_t wKakunin[INPUTFILE_MAXLINE];  // �m�F�p
        int iReasult;


        //-------------------------------------------------------------------
        //  GEARBOX�̏ꍇ
        //-------------------------------------------------------------------
        strCsys = gstrGearboxCoordinateSystems;
        iCount = giSectionRows[ENUM_INPUT_SECTION_GEARBOX_COORDINATE_SYSTEMS];

        for (int iLoop = 0; iLoop < iCount; iLoop++) {
            // �����Ώۂ�mdl���擾
            //ProStringToWstring(wGenericInConfigFile, strCsys->cGenericParameter);
            ProStringToWstring(wGenericInConfigFile, strCsys->cInstanceParameter);
            ProMdlMdlnameGet(mdl, wSearchCsysPart);

            ProWstringCompare(wGenericInConfigFile, wSearchCsysPart, PRO_VALUE_UNUSED, &iReasult);

            if (iReasult == NULL) {
                ProWstringCopy(((UserVCsysAppData*)app_data)->csys_name, local_app_data.csys_name, PRO_VALUE_UNUSED);
                local_app_data.model = mdl;
                local_app_data.p_csys = NULL;

                // CSYS����������
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
        //   GEARBOX�ȊO�̏ꍇ
        //-------------------------------------------------------------------
        strCsys = gstrVehicleCoordinateSystems;
        iCount = giSectionRows[ENUM_INPUT_SECTION_VEHICLE_COORDINATE_SYSTEMS];

        for (int iLoop = 0; iLoop < iCount; iLoop++) {
            // �����Ώۂ�mdl���擾
            //ProStringToWstring(wGenericInConfigFile, strCsys->cGenericParameter);
            ProStringToWstring(wGenericInConfigFile, strCsys->cInstanceParameter);
            ProMdlMdlnameGet(mdl, wSearchCsysPart);

            ProWstringCompare(wGenericInConfigFile, wSearchCsysPart, PRO_VALUE_UNUSED, &iReasult);

            if (iReasult == NULL) {
                ProWstringCopy(((UserVCsysAppData*)app_data)->csys_name, local_app_data.csys_name, PRO_VALUE_UNUSED);
                local_app_data.model = mdl;
                local_app_data.p_csys = NULL;

                // CSYS����������
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
    // ���������A�N�Z�X����
    return PRO_TK_NO_ERROR;
}


/*====================================================*\
  Function : getFeatureIdFromCompAction()
  Purpose  : �����Ŏw�肵��Feature��FeatureId���擾���� (PRO_FEAT_COMPONENT��p)
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
    // �t�B�[�`���^�C�v���擾 (�f�[�^��, CSYS, �R���|�[�l���g ...)
    status = ProFeatureTypeGet(pFeature, &ftype);
    if (ftype != PRO_FEAT_COMPONENT) {
        return PRO_TK_CONTINUE;
    }
    status = ProAsmcompMdlMdlnameGet((ProAsmcomp*)pFeature, &mdltype, wName);

    // �A�Z���u���ȊO�� PRO_TK_CONTINUE
    if (mdltype != PRO_MDLFILE_ASSEMBLY) {
        return PRO_TK_CONTINUE;
    }

    // �t�@�~���[�e�[�u���̊m�F
    ProWstringCompare(wName, ((DatumAppData*)app_data)->name, PRO_VALUE_UNUSED, &iResult);

    if (iResult == 0) {
        // ���O����v�����ꍇ

        // app_data�ɒl���i�[����
        ((DatumAppData*)app_data)->feature = *pFeature;
        // �J�E���g���C���N�������g����
        ((DatumAppData*)app_data)->iFindCnt = ((DatumAppData*)app_data)->iFindCnt + 1;
        return PRO_TK_NO_ERROR;
    }
    else {
        // ���O����v���Ȃ������ꍇ�A�C���X�^���X�Ƃ��Ď����Ă���Έ�v�������ƂƂ���
        ProFamtable famtable;
        ProFaminstance famInstance;
        ProMdl mdlFamTable, mdlInstance;
        ProMdlName		wAssyName;

        status = ProMdlnameInit(wName, PRO_MDLFILE_ASSEMBLY, &mdlFamTable);
        status = ProFamtableInit(mdlFamTable, &famtable);
        status = ProFaminstanceInit(wName, &famtable, &famInstance);
        status = ProFaminstanceRetrieve(&famInstance, &mdlInstance);
        if (status != PRO_TK_NO_ERROR) {
            // �C���X�^���X��������܂���
            return PRO_TK_CONTINUE;
        }
        status = ProMdlMdlnameGet(mdlInstance, wAssyName);

        ProWstringCompare(wAssyName, ((DatumAppData*)app_data)->name, PRO_VALUE_UNUSED, &iResult);
        if (iResult == 0) {
            // ���O����v�����ꍇ

            // app_data�ɒl���i�[����
            ((DatumAppData*)app_data)->feature = *pFeature;
            // �J�E���g���C���N�������g����
            ((DatumAppData*)app_data)->iFindCnt = ((DatumAppData*)app_data)->iFindCnt + 1;
            return PRO_TK_NO_ERROR;
        }

    }
    return PRO_TK_CONTINUE;
}
