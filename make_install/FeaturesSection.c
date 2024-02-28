
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


// TOP�A�Z���u���z���ւ̃A�N�Z�X���Ɏg�p�����ProAppData
typedef struct {
    char cValue[INPUTFILE_MAXLINE];     // in  �����Ώۖ�
    int iFindCnt;                       // out �����Ώۂ̃��W���[����
}FeaturesAppData;


ProError  searchFeaturesAction(ProFeature* p_feature, ProError status, ProAppData app_data);
ProError  searchFeaturesAction2(ProFeature* p_feature, ProError status, ProAppData app_data);

// inputfile�̒��g���i�[����悤�̍\���̈ꎮ
FeatureList* gstrFeatureList;

int iFeatureListRowsMax = 0;

/*=========================================================================*\
    Function:	setFeaturesSection
    Purpose:	Feature�Z�N�V����
    ProMdl* top_asm             (in)    Feature�Ώ�
    InputFileFeature* strFeat   (in)    �R���t�B�O���[�V�����t�@�C���̓��e
    int iSectionMaxRows         (in)    �������ׂ��R���t�B�O���[�V�����t�@�C���̍s��

\*=========================================================================*/
ProError setFeaturesSection(ProMdl top_asm , InputFileFeature* strFeat, int iSectionMaxRows, FeatureList* FeatureList) {

    ProError status = PRO_TK_NO_ERROR;
    ProMdlType mdl_type = PRO_MDL_UNUSED;
    FeaturesAppData	app_data;

    // �J�����g���f���̃^�C�v���擾
    status = ProMdlTypeGet(top_asm, &mdl_type);

    if (mdl_type == PRO_MDL_ASSEMBLY)
    {
        for (int iInputMdlCnt = 0; iInputMdlCnt < iSectionMaxRows; iInputMdlCnt++) {

            // �����l�̐ݒ�
            strcpy(app_data.cValue, strFeat->cValue);
            app_data.iFindCnt = 0;

            // TOP�A�Z���u���̂��ׂẴR���|�[�l���g���m�F����
           ProSolidFeatVisit((ProSolid)top_asm,
                searchFeaturesAction,
                NULL,
                (ProAppData)&app_data);

            if (app_data.iFindCnt == 0) {

                if (iFeatureListRowsMax != 0) {
                    // Feature��������Ȃ������ꍇ, FeatureList��X�̋L�ڂ�����̂����m�F
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
                            // FeatureNumber������ATarget����X���L�ڂ���Ă���
                            findFlag = PRO_B_TRUE;
                            break;
                        }
                        FeatureList++;
                    }

                    if (findFlag == PRO_B_FALSE) {
                        // Feature��������Ȃ�����
                        LOG_PRINT("NOK : %s : Feature not found", strFeat->cValue);

                    }
                    else {
                        // Feature��������Ȃ�����
                        LOG_PRINT("OK  : %s : Feature not found", strFeat->cValue);

                    }
                }
                else {
                    // Feature��������Ȃ�����
                    LOG_PRINT("NOK : %s : Feature not found", strFeat->cValue);

                }
            }
            strFeat++;
        }

        // �\���b�h���̂��ׂẴt�B�[�`�����Đ�������
        // ��������Ȃ��Ɛ��������f����Ȃ�
        status = ProSolidRegenerate(ProMdlToSolid(top_asm), PRO_REGEN_FORCE_REGEN);
    }
    else {
        LOG_PRINT("NOK : All Features fail", strFeat->cValue);

    }
}

/*====================================================*\
  Function : searchFeaturesAction()
  Purpose  :TOP�A�Z���u�������t�B�[�`�����m�F����
  ProFeature* p_feature (in) �t���[�`���[�n���h��
  ProError status       (in) �X�e�[�^�X
  ProAppData app_data   (in) ProSolidFeatVisit����󂯎�����f�[�^
\*====================================================*/
ProError  searchFeaturesAction(ProFeature* p_feature, ProError status, ProAppData app_data)
{
    ProFeattype ftype;
    ProMdl p_mdl;
    ProMdlType p_type;
    ProFamilyMdlName    wName;
    ProCharPath         cName;
    FeaturesAppData	app_data2;

    // �t�B�[�`�������擾
    status = ProModelitemNameGet(p_feature, wName);
    ProWstringToString(cName, wName);

    // �R���t�B�O���[�V�����t�@�C�����̒l���r���A������v�����ꍇ�Ƀ��W���[������
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

        // �A�h���X���ς�����̂ŁA�Ă��܂킵
        strcpy(app_data2.cValue, ((FeaturesAppData*)app_data)->cValue);
        app_data2.iFindCnt = ((FeaturesAppData*)app_data)->iFindCnt;

        /*********************************************
        * �悭�g�p�����t�B�[�`���^�C�v(����)
        *     916  PRO_FEAT_CUT
        *     923  PRO_FEAT_DATUM
        *     926  PRO_FEAT_DATUM_AXIS
        *     979  PRO_FEAT_CSYS
         **********************************************/
         // �t�B�[�`���^�C�v���擾 (�f�[�^��, CSYS, �R���|�[�l���g ...)
        status = ProFeatureTypeGet(p_feature, &ftype);
        if (status == PRO_TK_NO_ERROR && ftype == PRO_FEAT_COMPONENT)
        {
            status = ProAsmcompMdlGet((ProAsmcomp*)p_feature, &p_mdl);
            if (status == PRO_TK_NO_ERROR)
            {
                // �^�C�v���擾
                status = ProMdlTypeGet(p_mdl, &p_type);
                if (status == PRO_TK_NO_ERROR && p_type == PRO_MDL_PART)
                {
                    // �ċN�Ăяo�����ł��Ȃ������̂ŁA�������e��Action���Ăяo��
                    status = ProSolidFeatVisit((ProSolid)p_mdl,
                        searchFeaturesAction2,
                        NULL,
                        (ProAppData)&app_data2);

                    // �A�h���X���ς�����̂ŁA�Ă��܂킵
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
  Purpose  :TOP�A�Z���u�������t�B�[�`�����m�F����
  ProFeature* p_feature (in) �t���[�`���[�n���h��
  ProError status       (in) �X�e�[�^�X
  ProAppData app_data   (in) ProSolidFeatVisit����󂯎�����f�[�^
  ���l�F
  �ċN�Ăяo�����ł��Ȃ������̂ŁA�������e��Action���Ăяo��
\*====================================================*/
ProError  searchFeaturesAction2(ProFeature* p_feature, ProError status, ProAppData app_data)
{
    ProFeattype ftype;
    ProMdl p_mdl;
    ProMdlType p_type;
    ProFamilyMdlName    wName;
    ProCharPath         cName;

    // �t�B�[�`�������擾
    status = ProModelitemNameGet(p_feature, wName);
    ProWstringToString(cName, wName);

    // �R���t�B�O���[�V�����t�@�C�����̒l���r���A������v�����ꍇ�Ƀ��W���[������
    if (strstr(cName, ((FeaturesAppData*)app_data)->cValue) != NULL) {

        status = ProModelitemMdlGet(p_feature, &p_mdl);
        if (status == PRO_TK_NO_ERROR) {

            // ���W���[�������񐔂��J�E���g
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
        * �悭�g�p�����t�B�[�`���^�C�v(����)
        *     916  PRO_FEAT_CUT
        *     923  PRO_FEAT_DATUM
        *     926  PRO_FEAT_DATUM_AXIS
        *     979  PRO_FEAT_CSYS
         **********************************************/
         // �t�B�[�`���^�C�v���擾 (�f�[�^��, CSYS, �R���|�[�l���g ...)
        status = ProFeatureTypeGet(p_feature, &ftype);
        if (status == PRO_TK_NO_ERROR && ftype == PRO_FEAT_COMPONENT)
        {
            status = ProAsmcompMdlGet((ProAsmcomp*)p_feature, &p_mdl);
            if (status == PRO_TK_NO_ERROR)
            {
                // �^�C�v���擾
                status = ProMdlTypeGet(p_mdl, &p_type);
                if (status == PRO_TK_NO_ERROR && p_type == PRO_MDL_PART)
                {

                    // TOP�A�Z���u���̂��ׂẴR���|�[�l���g���ċA�I�ɖK�₵�A�T�u�A�Z���u���̐��𐔂���
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
    ProSolid* p_solid               (in) �Ώۂ̃\���b�h(?)
    ProCharPath cTargetFeatureName  (in) �Ώۂ̃t�B�[�`����
    ���l�F
    PTC�̃T���v���\�[�X��]�L
    �Ώۂ̃\���b�h�Ɠ��K�w�̃t�B�[�`�������ׂă��W���[������Ă��܂����̂ŁA
    �Ώۃt�B�[�`�����Ɠ����t�B�[�`���ȊO�����W���[�����Ȃ��悤�ɕύX
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

    // ID�ƃX�e�[�^�X�z������蓖��
    status = ProArrayAlloc(0, sizeof(int), 1, (ProArray*)&p_feat_id_array);
    status = ProArrayAlloc(0, sizeof(ProFeatStatus), 1, (ProArray*)&p_status_array);

    if (status != PRO_TK_NO_ERROR) {
        return PRO_TK_GENERAL_ERROR;

    }

    // �w�肳�ꂽ�\���b�h�̋@�\�Ƃ��̃X�e�[�^�X�̃��X�g���擾
    status = ProSolidFeatstatusGet(*p_solid, &p_feat_id_array, &p_status_array, &n_features);

    if (status != PRO_TK_NO_ERROR) {
        return PRO_TK_GENERAL_ERROR;

    }

    for (i = n_features - 1, n_suppressed = 0; i >= 0; i--)
    {
        // �@�\�̃n���h�����擾
        status = ProFeatureInit(*p_solid, p_feat_id_array[i], &feature);

        // �Y���t�B�[�`���ȊO�͏��������Ȃ�
        status = ProModelitemNameGet(&feature, wFeatureName);
        ProStringToWstring(wTargetFeatureName, cTargetFeatureName);
        ProWstringCompare(wFeatureName, wTargetFeatureName, PRO_VALUE_UNUSED, &result);

        // �@�\�͊������Ă��܂����H
        is_incomplete = PRO_B_FALSE;
        status = ProFeatureIsIncomplete(&feature, &is_incomplete);

        // �z�񂩂�s���S�ȋ@�\���폜
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

    // ���W���[���̎��{
    status = ProArrayAlloc(1, sizeof(ProFeatureResumeOptions), 1, (ProArray*)&resume_options);

    resume_options[0] = PRO_FEAT_RESUME_INCLUDE_PARENTS;

    status = ProFeatureWithoptionsResume(*p_solid, p_feat_id_array, resume_options, PRO_REGEN_NO_FLAGS);

    // ��ʃ��t���b�V��
    status = ProWindowRepaint(-1);

    status = ProTreetoolRefresh((ProMdl)*p_solid);

    /*-----------------------------------------------------------------*\
        �������̊J��
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
PURPOSE  : FeatureList�����[�h���A�e�\���̂֒l���i�[����
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
        // �t�@�C���I�[�v���̎��s�� checkInitial()�Ŋm�F�ς݂����A
        // �R���t�B�O���[�V�����t�@�C���̎擾�Ɏ��s�����ꍇ�͈ȍ~�̏��������Ȃ�
        return;
    }


    /***********************************************
     inputFile �� �e�Z�N�V����/��؂�̍s�����J�E���g����
    *************************************************/
    while (fgets(str, sizeof(str), fp) != NULL) {
        if (str[0] != '\n') {
            iFeatureListRowsMax++;
        }
    }

    // �t�@�C���ʒu��擪�ɖ߂�
    fseek(fp, 0, SEEK_SET);

    /***********************************************
     inputFile �̒l���i�[����ϐ��̃������m��
    *************************************************/
    // *PARAMETERS��Feature��
    gstrFeatureList = (FeatureList*)calloc((iFeatureListRowsMax), sizeof(FeatureList));
    if (!gstrFeatureList) {
        // �������s��
        LOG_PRINT("NOK : Not enough memory");
        return PRO_TK_GENERAL_ERROR;
    }

    /***********************************************
     inputFile �� �l�擾
    *************************************************/
    while (fgets(str, sizeof(str), fp) != NULL) {
        // ���s�݂̂͏������Ȃ�
        if (str[0] != '\n') {
            TRAIL_PRINT("%s(%d) : str = %s", __func__, __LINE__, str);

            getFeatureList(str, iFeatureListRowsCount, gstrFeatureList);
            iFeatureListRowsCount++;
        }

    }

    //�t�@�C�������
    fclose(fp);

    return PRO_TK_NO_ERROR;

}

/*====================================================================*\
FUNCTION : getInputFile
PURPOSE  : inputFile�����[�h���A�e�\���̂֒l���i�[����
    char        str[MAX_WORDS_IN_CELL]  (in) inputFile����ǂݎ����1�s
    int         iRow                    (in) �ǂݎ�����s
    InputFile*  wInputFileArray         (out)�i�[����\����
\*====================================================================*/
ProError  getFeatureList(char str[MAX_WORDS_IN_CELL], int iRow, FeatureList* FeatureList)
{
    char* cpValue;           // �Ώە��i��

    if (iRow > 0) {
        for (int iLoop = 0; iLoop < iRow; iLoop++) {
            *FeatureList++;
        }
    }

    // COMMA_SEPARATION ��؂�Œl���擾����
    cpValue = strtok(str, COMMA_SEPARATION);
    ProStringToWstring(FeatureList->wFeatureNumber, c_trim(cpValue));

    cpValue = strtok(NULL, COMMA_SEPARATION);
    ProStringToWstring(FeatureList->wFeatureName, c_trim(cpValue));

    cpValue = strtok(NULL, COMMA_SEPARATION);
    ProStringToWstring(FeatureList->wTargetX, c_trim(cpValue));

    cpValue = strtok(NULL, COMMA_SEPARATION);
    ProStringToWstring(FeatureList->wMemo, c_trim(cpValue));

}
