/*--------------------------------------------------------------------*\
C includes
\*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*\
Pro/Toolkit includes
\*--------------------------------------------------------------------*/
#include <ProToolkitErrors.h>
#include <ProMdl.h>
#include <ProAssembly.h>
#include <ProArray.h>
#include <ProWindows.h>
#include <ProFamtable.h>
#include <ProFaminstance.h>
#include <ProSolid.h>
#include <ProDrawing.h>
#include <ProFeatType.h>
#include <ProWstring.h>

/*--------------------------------------------------------------------*\
Application includes
\*--------------------------------------------------------------------*/
#include "CommonUtil.h"
#include "InputFile.h"
#include "CoordinateSystemsSection.h"

// TOP�A�Z���u���z���ւ̃A�N�Z�X���Ɏg�p�����ProAppData
typedef struct {
	ProPath		name;       // in  �����Ώۖ�
	ProMdlType p_type;      // in  �����Ώۂ̃^�C�v
	int comp_id_p_arr;		// out 
}FeatureIDGetAppData;

ProError FeatureIDGetFilter(ProFeature* p_feature, ProAppData app_data);
ProError FeatureIDGetAction(ProFeature* p_feature, ProError status, ProAppData app_data);

/*====================================================================*\
FUNCTION : SetCoordinateSystemsSection
PURPOSE  : Csys�Z�N�V����
ProMdl top_asm			(in)	�Ώ�
InputFileCsys* strCsys	(in)	�R���t�B�O���[�V�����t�@�C���̓��e
int iSectionMaxRows		(in)	�������ׂ��R���t�B�O���[�V�����t�@�C���̍s��
\*====================================================================*/
ProError  SetCoordinateSystemsSection(ProMdl mdlTopAssy, InputFileCsys* strCsys, int iSectionMaxRows)
{
	ProError status;
	ProMdl mdlFamTable, mdlInstance;
	int i = 0;
	int* comp_id_p_arr = NULL;
	ProFamtable famtable;
	ProFaminstance famInstance;
	ProMdlName		w_model_name;
	
	wchar_t wInstanceParameter[INPUTFILE_MAXLINE]; // �C���X�^���X�p�����[�^
	wchar_t wGenericParameter[INPUTFILE_MAXLINE];  // �W�F�l���b�N�p�����[�^


	for (int iInputMdlCnt = 0; iInputMdlCnt < iSectionMaxRows; iInputMdlCnt++) {

		if (strCsys->cInstanceParameter[0] == '0' 
			&& strCsys->cInstanceParameter[1] == '4'
			&& strCsys->cInstanceParameter[2] == '_') {
			// InstanceParameter �� 04_ �Ŏn�܂�ꍇ�̓W�F�l���b�N���f����I��(�������Ȃ�)
			LOG_PRINT("OK  : %s : Ignored processing", strCsys->cInstanceParameter);
			strCsys++;
			continue;
		}

		// �l�̏�����
		ProStringToWstring(wGenericParameter, strCsys->cGenericParameter);
		ProStringToWstring(wInstanceParameter, strCsys->cInstanceParameter);

		status = ProArrayAlloc(1, sizeof(int*), 1, (ProArray*)&comp_id_p_arr);
		status = ProMdlnameInit(wGenericParameter, PRO_MDLFILE_PART, &mdlFamTable);
		// �n���h���̏�����
		status = ProFamtableInit(mdlFamTable, &famtable);
		status = ProFaminstanceInit(wInstanceParameter, &famtable, &famInstance);
		status = ProFaminstanceRetrieve(&famInstance, &mdlInstance);

		if (status != PRO_TK_NO_ERROR) {
			// InstanceParameter ������Ȃ��ꍇ
			LOG_PRINT("NOK : %s : Instance model not found", strCsys->cInstanceParameter);
			strCsys++;
			continue;
		}
		status = ProMdlMdlnameGet(mdlInstance, w_model_name);

		status = ProMdlLoad(w_model_name, PRO_MDL_PART, PRO_B_FALSE, &mdlFamTable);
		// �����̃t�B�[�`��Id���擾����
		FeatureIDGetAppData	app_data;
		ProWstringCopy(wGenericParameter, app_data.name, PRO_VALUE_UNUSED);
		app_data.p_type = PRO_MDL_PART;

		// TOP�A�Z���u���̂��ׂẴR���|�[�l���g���ċA�I�ɖK�₵�A�����t�B�[�`��ID���擾����
		status = ProSolidFeatVisit((ProSolid)mdlTopAssy,
			FeatureIDGetAction,
			FeatureIDGetFilter,
			(ProAppData)&app_data);

		// �擾�����t�B�[�`��ID���w�肵�A�t�@�~���[�e�[�u����ύX����
		comp_id_p_arr[0] = app_data.comp_id_p_arr;
		status = ProAssemblyAutointerchange((ProAssembly)mdlTopAssy, comp_id_p_arr, mdlFamTable);

		if (status != PRO_TK_NO_ERROR) {
			// �C���X�^���X���f���̒u���Ɏ��s
			LOG_PRINT("NOK : %s : Instance model replacement failed", strCsys->cInstanceParameter);
		}else {
			LOG_PRINT("OK  : %s", strCsys->cInstanceParameter);

		}
		strCsys++;
	}

	// �Đ�����
	status = ProSolidRegenerate((ProSolid)mdlTopAssy, PRO_REGEN_FORCE_REGEN);

	return status;
}

/*====================================================*\
  Function : FeatureIDGetFilter()
  Purpose  : �R���|�[�l���g�̃t�B���^�@�\
\*====================================================*/
ProError FeatureIDGetFilter(ProFeature* p_feature, ProAppData app_data)
{
	ProError    status;
	ProFeattype ftype;
	ProMdl p_mdl;
	ProMdlType p_type;

	// �t�B�[�`���^�C�v���擾 (�f�[�^��, CSYS, �R���|�[�l���g ...)
	status = ProFeatureTypeGet(p_feature, &ftype);
	if (status == PRO_TK_NO_ERROR && ftype == PRO_FEAT_COMPONENT)
	{
		status = ProAsmcompMdlGet((ProAsmcomp*)p_feature, &p_mdl);
		if (status == PRO_TK_NO_ERROR)
		{
			// �^�C�v���擾
			status = ProMdlTypeGet(p_mdl, &p_type);
			if (status == PRO_TK_NO_ERROR)
			{
				if (p_type == ((FeatureIDGetAppData*)app_data)->p_type)
				{
					return(PRO_TK_NO_ERROR);
				}
			}
		}
	}
	return PRO_TK_CONTINUE;
}


/*====================================================*\
  Function : FeatureIDGetAction()
  Purpose  : �����Ŏw�肵��Feature��FeatureId���擾����
\*====================================================*/
ProError  FeatureIDGetAction(ProFeature* p_feature, ProError status, ProAppData app_data)
{
	ProMdlfileType     mdltype;
	ProFamilyMdlName        w_name;
	int iResult;

	// �T�u�A�Z���u���̃^�C�v�Ɩ��O���擾����
	status = ProAsmcompMdlMdlnameGet((ProAsmcomp*)p_feature, &mdltype, w_name);
	ProWstringCompare(w_name, ((FeatureIDGetAppData*)app_data)->name, PRO_VALUE_UNUSED, &iResult);

	if (iResult == 0) {
		// app_data�ɒl���i�[����
		((FeatureIDGetAppData*)app_data)->comp_id_p_arr = p_feature->id;
	}
	return status;
}
