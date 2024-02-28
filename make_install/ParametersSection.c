/*--------------------------------------------------------------------*\
C includes
\*--------------------------------------------------------------------*/
#include <stdlib.h>
#include <windows.h>
/*--------------------------------------------------------------------*\
Pro/Toolkit includes
\*--------------------------------------------------------------------*/
#include <ProMdl.h>
#include <ProDrawing.h>
#include <ProParameter.h>
#include <ProWstring.h>

/*--------------------------------------------------------------------*\
Application includes
\*--------------------------------------------------------------------*/
#include "ParametersSection.h"
#include "CommonUtil.h"
#include <ProSolid.h>

/*-----------------------------------------------------------------*\
    �\����
\*-----------------------------------------------------------------*/

/*-----------------------------------------------------------------*\
    �v���g�^�C�v�錾
\*-----------------------------------------------------------------*/
ProError ParameterVisitAction(ProParameter* param, ProError err, ProAppData app_data);

/*====================================================================*\
FUNCTION : setParametersSection
PURPOSE  : �p�����[�^�̕ύX
ProMdl mdl                          (in) �ύX����p�����[�^��Top�A�Z���u��
ProPath layoutName                  (in) �C�����郌�C�A�E�g�t�@�C����(�g������)
InputFileParamFeature* strFeature   (in) �������ׂ��R���t�B�O���[�V�����t�@�C�� *PARAMETERS �n�� - Feature��
InputFileParameters* strParam       (in) �������ׂ��R���t�B�O���[�V�����t�@�C�� *PARAMETERS �n�� - Parameter��
int iFeatureSectionMaxRows          (in) �������ׂ��R���t�B�O���[�V�����t�@�C���̍s�� *PARAMETERS �n�� - Feature��
int iParameterSectionMaxRows        (in) �������ׂ��R���t�B�O���[�V�����t�@�C���̍s�� *PARAMETERS �n�� - Parameter��
\*====================================================================*/
ProError  setParametersSection(ProMdl mdlTopAssy, ProPath layoutName, InputFileParamFeature* strFeature, InputFileParameters* strParam, int iFeatureSectionMaxRows, int iParameterSectionMaxRows)
{
	ProError status;
    ProModelitem modelitem;
    ProMdl notebookMdl = NULL;
    ProPath wTopLayout; // ���C�A�E�g�t�@�C����/�ύX�Ώ�(�g���q�t��)
    ProCharPath cTopLayout;
    ProMdlfileType filetype = PRO_MDLFILE_UNUSED;
    ProMdlType type = PRO_MDL_UNUSED;


    /*--------------------------------------------------------------------*\
     �p�����[�^�̕ύX�Ώۂ̊m�F
    \*--------------------------------------------------------------------*/
    if (iFeatureSectionMaxRows == 0) {
        // �p�����[�^�̕ύX�Ώۂ��Ȃ��ꍇ��asm�Ɠ����̃��C�A�E�g�t�@�C��
        ProWstringCopy(layoutName, wTopLayout, PRO_VALUE_UNUSED);
        ProWstringConcatenate(L".lay", wTopLayout, PRO_VALUE_UNUSED);
    }
    else {
        // �p�����[�^�̕ύX�Ώۂ�����ꍇ�́A�ύX�Ώۂ��g�p
        ProStringToWstring(wTopLayout, strFeature->cFeature);
    }

    LOG_PRINT("ATTENTION : Change the parameters of %w ", wTopLayout);

    // �Y���t�@�C���̊g���q���m�F
    ProWstringToString(cTopLayout, wTopLayout);
    if (strstr(cTopLayout, ".prt") != NULL) {
        filetype = PRO_MDLFILE_PART;
        type = PRO_MDL_PART;
    }
    else if (strstr(cTopLayout, ".lay") != NULL) {
        filetype = PRO_MDLFILE_NOTEBOOK;
        type = PRO_MDL_LAYOUT;
    }

    // �t�@�C�������[�h�ς݂����m�F����
    ProMdlInit(wTopLayout, type, &notebookMdl);

    if (notebookMdl == NULL) {
        // �t�@�C�������[�h����
        if (searchAssypathFromWindchill(wTopLayout, SUB_ASSY, filetype, &notebookMdl) != PRO_TK_NO_ERROR) {
            LOG_PRINT("NOK : %s : Failed to setting parameter", strParam->cParameterName);
            return;
        }
    }
    else {
        // ���łɃ��[�h�ς�
    }

	// ���C�A�E�g�̕ύX
    status = ProMdlToModelitem(notebookMdl, &modelitem);

    for (int iLoop = 0; iLoop < iParameterSectionMaxRows; iLoop++) {
        status = ProParameterVisit(&modelitem, NULL, (ProParameterAction)ParameterVisitAction, (ProAppData)strParam);
        if (PRO_TK_NO_ERROR == status) {
            // ���C�A�E�g��������Ȃ�����
            LOG_PRINT("NOK : %s : Parameter not found", strParam->cParameterName);
        }
        strParam++;
    }

    // �p�����[�^�̓K�p
    status = ProSolidRegenerate((ProSolid)mdlTopAssy, PRO_REGEN_FORCE_REGEN);

    ProError PRO_TK_NO_ERROR;
}

/*====================================================================*\
    FUNCTION :	 ParameterVisitAction()
    PURPOSE  :   �p�����[�^�ꗗ�̎擾
\*====================================================================*/
ProError ParameterVisitAction(ProParameter* param,
    ProError err,
    ProAppData app_data)
{
    ProError status;
    ProCharLine str_param_val;
    ProParamvalue param_val;
    ProParamvalueType param_type;
    ProCharName param_name;
    ProLine w_str_param_val;
    ProParamvalue newValue;

    // �p�����[�^�̖��O�擾
    ProWstringToString(param_name, param->id);

    // �p�����[�^�����܂ރp�����[�^����������
    if (strstr(param_name, ((InputFileParameters*)app_data)->cParameterName) == NULL) {
        return PRO_TK_NO_ERROR;
    }

    //�p�����[�^�A�p�����[�^�^�C�v�擾
    status = ProParameterValueGet(param, &param_val);
    status = ProParamvalueTypeGet(&param_val, &param_type);

    newValue.type = param_type;

    if (param_type == PRO_PARAM_DOUBLE) {
        // ����(����)
        double dParameteValue = atof(((InputFileParameters*)app_data)->cParameteValue);
        newValue.value.d_val = dParameteValue;

    }else if (param_type == PRO_PARAM_STRING) {
        // ������
        ProLine wParameteValue;
        ProStringToWstring(wParameteValue, ((InputFileParameters*)app_data)->cParameteValue);
        ProWstringCopy(wParameteValue, newValue.value.s_val ,PRO_VALUE_UNUSED);

    }else if (param_type == PRO_PARAM_INTEGER || param_type == PRO_PARAM_NOTE_ID) {
        // ����
        int iParameteValue = atoi(((InputFileParameters*)app_data)->cParameteValue);
        newValue.value.i_val = iParameteValue;

    }else if (param_type == PRO_PARAM_BOOLEAN) {
        // YESNO (boolean�^��1/0�����͂����)
        short sParameteValue = atoi(((InputFileParameters*)app_data)->cParameteValue);
        newValue.value.l_val = sParameteValue;
    }

    status = ProParameterValueWithUnitsSet(param, &newValue, NULL);


    if (status == PRO_TK_NO_ERROR) {
        LOG_PRINT("OK  : %s : %s", ((InputFileParameters*)app_data)->cParameterName , param_name);
    }else {
        LOG_PRINT("NOK : %s : Parameter change failed", ((InputFileParameters*)app_data)->cParameterName);

    }
    return PRO_TK_CONTINUE;
}
