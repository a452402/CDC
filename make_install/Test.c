//#include <windows.h>
//
//
//#include <ProToolkitErrors.h>
//#include <ProSizeConst.h>
//#include <ProObjects.h>
//#include <ProDimension.h>
//#include <ProModelitem.h>
//
//#include <ProFeature.h>
//#include <ProSolid.h>
//#include <ProSelbuffer.h>
//#include <ProMenu.h>
//#include <ProUtil.h>
//#include <ProWindows.h>
//
#include <string.h>
//#include <ProVariantFeat.h>
//#include <ProCsys.h>
//#include <ProWstring.h>
//
#include <ProMdl.h>
#include <ProSolid.h>
#include "CommonUtil.h"
#include <ProUdf.h>
#include <ProModelitem.h>
//#include <ProHole.h>
//#include <ProFeatType.h>
//#include <ProSolid.h>
//#include <ProGroup.h>
//#include <ProDatumdata.h>
//
//#include "CommonUtil.h"
//
//
#include "Test.h"
#include <ProUtil.h>
#include <ProWstring.h>
#include <ProWTUtils.h>
#include <ProSimprep.h>
#include <ProWindows.h>

//
///*-----------------------------------------------------------------*\
//	�v���g�^�C�v�錾
//\*-----------------------------------------------------------------*/
//ProError  createHoleAroundAxis(ProMdl model, ProFeature dtmAxisFeature, ProFeature dtmPlnFeature_Y);
//ProError  createDatumPlane(ProMdl curMdl, int offsetDirection, double offsetValue, ProFeature* dtmPlnFeature);
//
//ProError  Test_Group_Function(ProSolid* p_solid);
//
//ProError createDatumAxis(ProMdl model, ProFeature dtmPlnFeature_X, ProFeature dtmPlnFeature_Z, ProFeature* dtmAxisFeature);
//
//ProError createHoleAroundPlane(ProMdl model,
//	ProFeature dtmPlnFeature_Y,
//	ProFeature dtmPlnFeature_X,
//	ProFeature dtmPlnFeature_Z,
//	double distX,
//	double distZ);
///*-----------------------------------------------------------------*\
//	�}�N��
//\*-----------------------------------------------------------------*/
//
//// INPUT�t�@�C������擾����ł��낤�l
//#define PART_NAME L"PRT_HOLE"	// ����������p�[�c��
//#define POINT_01_X 100.0		// ��01�̍��W
//#define POINT_01_Z 100.0		// ��01�̍��W
//
//
//typedef struct hole_01_info
//{
//	ProSurface rev_srf;
//	int axis_id;
//	double diameter;
//} Hole_01_info;
//
//

ProError SurfActiontest(
	ProSurface surface,
	ProError status,
	ProAppData data);

ProError  createGroupFunctionTest(ProSolid p_solid, ProName wGroupName);
ProError ProUtilFeatByNumberInit(
	ProSolid solid,
	int feature_num,
	ProFeature* p_feature);


#define URL			0
#define HOLE		0
#define LOAD		0
/*====================================================================*\
FUNCTION : TestFunction
PURPOSE  :
\*====================================================================*/
ProError  TestFunction()
{
	ProError	    status;
	ProMdl p_mdl;
	ProSelection* p_sel;
	int			n_sel;


	ProServerCheckinConflicts conflicts;
	wchar_t** removeModelList;
	// ���[�N�X�y�[�X���烂�f�����폜����
	status = ProServerObjectsRemove(NULL, &conflicts);
	if (status != PRO_TK_NO_ERROR && conflicts != NULL)
	{
	    ProServerconflictsFree(conflicts);
	}


#if LOAD
	wchar_t* windchillServer;
	wchar_t* actWorkspace;
	ProPath wimpdirFile;
	//ProMdl assy;
	ProMdl ass;

	// windchill�̃A�N�e�B�u�T�[�o�擾
	status = ProServerActiveGet(&windchillServer);

	// Windchill�̃��[�N�X�y�[�X�擾
	status = ProServerWorkspaceGet(windchillServer, &actWorkspace);

	ProStringToWstring(wimpdirFile, "wtws://");
	ProWstringConcatenate(windchillServer, wimpdirFile, PRO_VALUE_UNUSED);
	ProWstringConcatenate(L"/", wimpdirFile, PRO_VALUE_UNUSED);
	ProWstringConcatenate(actWorkspace, wimpdirFile, PRO_VALUE_UNUSED);
	ProWstringConcatenate(L"/", wimpdirFile, PRO_VALUE_UNUSED);
	ProWstringConcatenate(L"22055656", wimpdirFile, PRO_VALUE_UNUSED);


	// top�A�Z���u�������[�h����
	//status = ProMdlnameRetrieve(wimpdirFile, PRO_MDLFILE_ASSEMBLY, &assy);


	    // �A�Z���u���̎���Empty�\��������
	    status = ProAssemblySimprepMdlnameRetrieve(wimpdirFile, PRO_MDLFILE_ASSEMBLY, L"EMPTY", NULL, &ass);

		status = ProMdlDisplay((ProMdl)ass);

		/***********************************************
		 ���f���c���[���X�V����
		*************************************************/
		int window_id;
		status = ProMdlWindowGet((ProMdl)ass, &window_id);
		status = ProWindowActivate(window_id);

#endif


#if HOLE
	status = ProMdlCurrentGet(&p_mdl);

	status = ProSolidSurfaceVisit(ProMdlToSolid(p_mdl),
		(ProSurfaceVisitAction)SurfActiontest,
		(ProSurfaceFilterAction)NULL,
		(ProAppData)NULL);

#endif

#if URL
	wchar_t* windchillServer;
	wchar_t* actWorkspace;

	wchar_t* alias;
	wchar_t* alias2;
	wchar_t* location;
	wchar_t* codebase;
	wchar_t** allserver;

	wchar_t* codebase2;


	ProPath wimpdirFile;
	ProPath* file_list, * dir_list;
	status = ProArrayAlloc(0, sizeof(ProPath), 1, (ProArray*)&file_list);
	ProCharLine line, str;
	int  n_files;

	ProServerCheckoutOptions co_options;
	ProServerCheckinConflicts conflicts;
	wchar_t* model_url = NULL;


	// �A�N�e�B�u�T�[�o�擾 (UDtrucks-wcadmin)
	status = ProServerActiveGet(&windchillServer);

	// ���[�N�X�y�[�X�擾 (UDTracs_Iwasaki)
	status = ProServerWorkspaceGet(windchillServer, &actWorkspace);

	// �R���e�L�X�g(ud-trucks)
	ProServerContextGet(windchillServer, &alias);

	// ���P�[�V����(http://aec.3750.jp/Windchill)
	ProServerLocationGet(windchillServer, &location);

	//  L"UDtrucks-wcadmin"
	ProServerAliasGet(L"http://aec.3750.jp/Windchill/app/#ptc1/org/listFiles?ContainerOid=OR%3Awt.inf.container.OrgContainer%3A114020&oid=OR%3Awt.folder.Cabinet%3A114071&folderNavigatorParameter=true&u8=1", NULL, &alias2);


	// �N���X Windchill or ProjectLink
	ProServerClassGet(windchillServer, &codebase);

	//  L"UDtrucks-wcadmin"
	ProServersCollect(&allserver);

	ProStringToWstring(wimpdirFile, "wtpub://");
	ProWstringConcatenate(windchillServer, wimpdirFile, PRO_VALUE_UNUSED);
	ProWstringConcatenate(L"/", wimpdirFile, PRO_VALUE_UNUSED);
	ProWstringConcatenate(L"Site", wimpdirFile, PRO_VALUE_UNUSED);
	ProWstringConcatenate(L"/", wimpdirFile, PRO_VALUE_UNUSED);
	ProWstringConcatenate(alias, wimpdirFile, PRO_VALUE_UNUSED);
	ProWstringConcatenate(L"/", wimpdirFile, PRO_VALUE_UNUSED);

	status = ProFilesList(wimpdirFile, L"*.asm", PRO_FILE_LIST_ALL, &file_list, &dir_list);


	//status = ProFileselectionDocNameGet(codebase2);


	if (status == PRO_TK_NO_ERROR)
	{
		status = ProArraySizeGet((ProArray)file_list, &n_files);

		//for (int i = 0; i < n_files - 1; i++)
		//{
		//	ProWstringToString(str, file_list[i]);			
		//	if (strstr(str, "23585707") != NULL) {
		//		printf("aaa");
		//	}
		//}
	}


	ProWstringConcatenate(L"23585707.asm", wimpdirFile, PRO_VALUE_UNUSED);


	wchar_t* url;
	status = ProServerAliasedURLToURL(wimpdirFile,&url);



#endif


	
	return status;

}

/*====================================================================*\
FUNCTION : createGroupFunction
PURPOSE  : �O���[�v �쐬 / TestFeats.c (ProTestLocalGroupCreate)�Q�l (����m�FOK)
ProSolid* p_solid	in	�O���[�v�Ώۃt�B�[�`����������\���b�h
\*====================================================================*/
ProError  createGroupFunctionTest(ProSolid p_solid, ProName wGroupName)
{
	ProError status;
	ProName  w_name;
	int* feat_arr = NULL, n_sel, stop = 0, i, * p_feat_id_array, feat_num;
	//int* feat_arr = NULL, stop = 0, i, * p_feat_id_array, feat_num;
	ProSelection* sel;
	ProModelitem modelitem;
	ProGroup group;

	// ����������
	ProModelitemInit(NULL, NULL, PRO_TYPE_UNUSED, &modelitem);


	status = ProSelect((char*)"feature", -1, NULL, NULL, NULL, NULL, &sel, &n_sel);
	if (status != PRO_TK_NO_ERROR) {
		//break;
		return;

	}

	/* Sort features array by number */
	ProArrayAlloc(0, sizeof(int), 1, (ProArray*)&feat_arr);
	for (i = 0; i < n_sel; i++)
	{
		// �I�������t�B�[�`���̃t�B�[�`���ԍ����擾
		status = ProSelectionModelitemGet(*sel, &modelitem);
		status = ProFeatureNumberGet((ProFeature*)&modelitem, &feat_num);
		if (status != PRO_TK_NO_ERROR) {
			//continue;
			return;
		}

		// �\�[�g�����ɃI�u�W�F�N�g�ǉ�
		status = ProArrayObjectAdd((ProArray*)&feat_arr, PRO_VALUE_UNUSED, 1, &feat_num);
		// �\�[�g���ăI�u�W�F�N�g�ǉ�
		//status = ProUtilSortedIntArrayObjectAdd(&feat_arr, feat_num);
		if (status != PRO_TK_NO_ERROR) {
			//continue;
			return;
		}
	}

	status = ProArraySizeGet((ProArray)feat_arr, &n_sel);

	status = ProArrayAlloc(0, sizeof(int), 1, (ProArray*)&p_feat_id_array);
	if (status != PRO_TK_NO_ERROR) {
		//break;
		return;

	}

	for (i = 0; i < n_sel; i++)
	{
		// �A�N�e�B�u�t�B�[�`���݂̂�modelitem�ɐݒ�
		status = ProUtilFeatByNumberInit(p_solid, feat_arr[i], (ProFeature*)&modelitem);

		ProArrayObjectAdd((ProArray*)&p_feat_id_array, PRO_VALUE_UNUSED, 1, &modelitem.id);
		//ProArrayObjectAdd((ProArray*)&p_feat_id_array, PRO_VALUE_UNUSED, 1, &modelitem->id);
	}

	// �O���[�v�̍쐬
	status = ProLocalGroupCreate((ProSolid)modelitem.owner, p_feat_id_array, n_sel, wGroupName, &group);
	//status = ProLocalGroupCreate((ProSolid)modelitem->owner, p_feat_id_array, n_sel, wGroupName, &group);

	status = ProArrayFree((ProArray*)&feat_arr);

	status = ProArrayFree((ProArray*)&p_feat_id_array);


	return PRO_TK_NO_ERROR;
}


/*====================================================================*\
FUNCTION: SurfAction()
PURPOSE:  Action function called when visiting solid surfaces to
			   attach gtol to.
\*====================================================================*/
ProError SurfActiontest(
	ProSurface surface,
	ProError status,
	ProAppData data)
{
	ProSrftype stype;
	ProGeomitemdata* pDataPtr;

	/*--------------------------------------------------------------------*\
		�T�[�t�F�X�����ʂłȂ��ꍇ�́A�X�L�b�v
	\*--------------------------------------------------------------------*/
	ProSurfaceTypeGet(surface, &stype);
	if (stype != PRO_SRF_PLANE) {
		return(PRO_TK_NO_ERROR);
	}

	/*--------------------------------------------------------------------*\
		�T�[�t�F�X�̏����擾
	\*--------------------------------------------------------------------*/
	ProVector xyz_point;
	ProVector deriv1[2];
	ProVector deriv2[3];
	ProVector normal;



	ProSurfaceDataGet(surface, &pDataPtr);

		ProSurfaceXyzdataEval(surface, pDataPtr->data.p_surface_data->uv_max,
			xyz_point, deriv1, deriv2, normal);

	// �\�ʐς����߂�
	double p_area;
	ProSurfaceAreaEval(surface, &p_area);

	{
		char logtemp[256] = "";
		char temp[256] = "";

		strcat(logtemp, ":min:");
		sprintf(temp, "%.3f", pDataPtr->data.p_surface_data->xyz_min[0]);
		strcat(logtemp, temp);
		strcat(logtemp, ":");
		sprintf(temp, "%.3f", pDataPtr->data.p_surface_data->xyz_min[1]);
		strcat(logtemp, temp);
		strcat(logtemp, ":");
		sprintf(temp, "%.3f", pDataPtr->data.p_surface_data->xyz_min[2]);
		strcat(logtemp, temp);

		strcat(logtemp, ":max:");
		sprintf(temp, "%.3f", pDataPtr->data.p_surface_data->xyz_max[0]);
		strcat(logtemp, temp);
		strcat(logtemp, ":");
		sprintf(temp, "%.3f", pDataPtr->data.p_surface_data->xyz_max[1]);
		strcat(logtemp, temp);
		strcat(logtemp, ":");
		sprintf(temp, "%.3f", pDataPtr->data.p_surface_data->xyz_max[2]);
		strcat(logtemp, temp);

		strcat(logtemp, ":xyz:");
		sprintf(temp, "%.3f", xyz_point[0]);
		strcat(logtemp, temp);
		strcat(logtemp, ":");
		sprintf(temp, "%.3f", xyz_point[1]);
		strcat(logtemp, temp);
		strcat(logtemp, ":");
		sprintf(temp, "%.3f", xyz_point[2]);
		strcat(logtemp, temp);

		strcat(logtemp, ":Housen:");
		sprintf(temp, "%.3f", normal[0]);
		strcat(logtemp, temp);
		strcat(logtemp, ":");
		sprintf(temp, "%.3f", normal[1]);
		strcat(logtemp, temp);
		strcat(logtemp, ":");
		sprintf(temp, "%.3f", normal[2]);
		strcat(logtemp, temp);

		strcat(logtemp, ":Menseki:");
		sprintf(temp, "%.1f", p_area);
		strcat(logtemp, temp);


		sprintf(gcLogFilePath, "E:\\GitFolder_UDTrucks\\01_work\\UD-SAVP\\log\\test.log");

		LOG_PRINT(logtemp);
	}

	ProGeomitemdataFree(&pDataPtr);

	return(PRO_TK_NO_ERROR);
}


//ProError  TestFunction2()
//{
//	ProError status = PRO_TK_NO_ERROR;
//
//
//	ProMdl prtMdl = NULL;
//	ProFeature dtmPlnFeature_X;
//	ProFeature dtmPlnFeature_Y;
//	ProFeature dtmPlnFeature_Z;
//	ProFeature dtmAxisFeature;
//
//	// part�����猊��������p�[�c�����擾
//	status = ProMdlnameInit(PART_NAME, PRO_MDLFILE_PART, &prtMdl);
//
//	if (status != PRO_TK_NO_ERROR) {
//		// �ҏW����p�[�c��������܂���ł����B
//		return status;
//	}
//	//// �f�[�^�����ʂ��쐻
//	status = createDatumPlane(prtMdl, PRO_DTMPLN_OFF_CSYS_X, POINT_01_X ,&dtmPlnFeature_X);
//	status = createDatumPlane(prtMdl, PRO_DTMPLN_OFF_CSYS_Y, 0, &dtmPlnFeature_Y);
//	status = createDatumPlane(prtMdl, PRO_DTMPLN_OFF_CSYS_Z, POINT_01_Z, &dtmPlnFeature_Z);
//
//	// 2���̃f�[�^�����ʂ̌���Ƀf�[�^�������쐬
//	status = createDatumAxis(prtMdl, dtmPlnFeature_X, dtmPlnFeature_Z ,&dtmAxisFeature);
//
//
//	// �f�[�^�����Ɍ��̍쐬
//	status = createHoleAroundAxis(prtMdl, dtmAxisFeature, dtmPlnFeature_Y);
//	// �f�[�^�����ʂ���Ɍ����쐬
//	status = createHoleAroundPlane(prtMdl, dtmPlnFeature_Y, dtmPlnFeature_X, dtmPlnFeature_Z, 500.0, 500.0);
//	status = createHoleAroundPlane(prtMdl, dtmPlnFeature_Y, dtmPlnFeature_X, dtmPlnFeature_Z, 200.0, 500.0);
//	status = createHoleAroundPlane(prtMdl, dtmPlnFeature_Y, dtmPlnFeature_X, dtmPlnFeature_Z, 500.0, 200.0);
//
//	// Select�őI�������t�B�[�`�������[�J���O���[�v�ł܂Ƃ߂� (����m�FOK)
//	//Test_Group_Function((ProSolid*)&prtMdl);
//
//
//
//	return PRO_TK_NO_ERROR;
//}
//
//
///*====================================================================*\
//	FUNCTION :	ProUtilSortedIntArrayObjectAdd
//	PURPOSE  :	�\�[�g���ꂽint�̔z���int�l��ǉ�/ TestFeats.c �Q�l�@(���󖢎g�p)
//\*====================================================================*/
//ProError ProUtilSortedIntArrayObjectAdd(
//	int** p_array,
//	int new_value)
//{
//	ProError status;
//	int i, size;
//
//	status = ProArraySizeGet((ProArray)p_array[0], &size);
//
//	for (i = 0; i<size && new_value>p_array[0][i]; i++) {
//		if (i >= size || new_value < p_array[0][i])
//		{
//			status = ProArrayObjectAdd((ProArray*)p_array, i, 1, &new_value);
//		}
//	}
//
//	return (status);
//}
//
///*====================================================================*\
//	FUNCTION :	ProUtilFeatByNumberInit()
//	PURPOSE  :	�A�N�e�B�uFeture�݂̂�p_feature�ɐݒ肷��/ TestFeats.c �Q�l
//	ProSolid solid,			in
//	int feature_num,		in
//	ProFeature* p_feature	out
//
//\*====================================================================*/
//ProError ProUtilFeatByNumberInit(
//	ProSolid solid,
//	int feature_num,
//	ProFeature* p_feature)
//{
//	ProError err, status = PRO_TK_GENERAL_ERROR;
//	int* p_feat_id_array, n_features;
//	ProFeatStatus* p_status_array;
//	int feature_count;
//	int actual_feat_number = 0;
//	ProBoolean feature_found = PRO_B_FALSE;
//
//
//	/* ID�z��ƃG���[�z��̊��� */
//	err = ProArrayAlloc(0, sizeof(int), 1, (ProArray*)&p_feat_id_array);
//	if (err != PRO_TK_NO_ERROR){
//		return (PRO_TK_GENERAL_ERROR);
//	}
//
//	err = ProArrayAlloc(0, sizeof(ProFeatStatus), 1,(ProArray*)&p_status_array);
//	if (err != PRO_TK_NO_ERROR) {
//		return (PRO_TK_GENERAL_ERROR);
//	}
//
//	/* �w�肳�ꂽ�\���b�h�̃t�B�[�`���ƃt�B�[�`���̃X�e�[�^�X���X�g���擾 */
//	err = ProSolidFeatstatusGet(solid, &p_feat_id_array, &p_status_array,&n_features);
//
//	/* feature_num--; */
//
//	if (err == PRO_TK_NO_ERROR && (n_features > (feature_num - 1)))
//	{
//		for (feature_count = 0; feature_count < n_features; feature_count++)
//		{
//			if (p_status_array[feature_count] == PRO_FEAT_ACTIVE) {
//				actual_feat_number++;
//			}
//			if (actual_feat_number == feature_num)
//			{
//				feature_found = PRO_B_TRUE;
//				break;
//			}
//		}
//
//		if (feature_found)
//		{
//			status = ProFeatureInit(solid, p_feat_id_array[feature_count], p_feature);
//
//			err = ProArrayFree((ProArray*)&p_feat_id_array);
//
//			err = ProArrayFree((ProArray*)&p_status_array);
//			return (status);
//			/* ����ȌĂяo�� */
//		}
//		else
//		{
//			return PRO_TK_E_NOT_FOUND;
//
//			/* ���̏ꍇ�Ɏ��{����
//			1.feature_number�̓��͒l�́A���f���Ŏg�p�\�ȃA�N�e�B�u�ȋ@�\�̐������傫�����A�ő�ID����������
//			*/
//		}
//	}
//	else
//	{
//		return PRO_TK_BAD_CONTEXT;
//		/* ���̏ꍇ�Ɏ��{����
//		1.ProSolidFeatstatusGet�ւ̌Ăяo�������s
//		2.�@�\�ԍ��̓��͒l�����W���ꂽID�̐������傫��
//		*/
//	}
//
//
//}
//
///*====================================================================*\
//FUNCTION : Test_Group_Function
//PURPOSE  : �O���[�v �쐬 / TestFeats.c (ProTestLocalGroupCreate)�Q�l (����m�FOK)
//\*====================================================================*/
//ProError  Test_Group_Function(ProSolid* p_solid)
//{
//	ProError status;
//	ProName  w_name;
//	int* feat_arr = NULL, n_sel, stop = 0, i, * p_feat_id_array, feat_num;
//	ProSelection* sel;
//	ProModelitem modelitem;
//	ProGroup group;
//
//	// ����������
//	ProModelitemInit(NULL, NULL, PRO_TYPE_UNUSED, &modelitem);
//
//
//	//status = ProSelect((char*)"feature", 1, NULL, NULL, NULL, NULL, &sel, &n_sel);
//	status = ProSelect((char*)"feature", -1, NULL, NULL, NULL, NULL, &sel, &n_sel);
//	if (status != PRO_TK_NO_ERROR) {
//			//break;
//			return;
//
//		}
//
//		/* Sort features array by number */
//		ProArrayAlloc(0, sizeof(int), 1, (ProArray*)&feat_arr);
//		for (i = 0; i < n_sel; i++)
//		{
//			// �I�������t�B�[�`���̃t�B�[�`���ԍ����擾
//			status = ProSelectionModelitemGet(sel[i], &modelitem);
//			status = ProFeatureNumberGet((ProFeature*)&modelitem, &feat_num);
//			if (status != PRO_TK_NO_ERROR) {
//				//continue;
//				return;
//			}
//
//			// �\�[�g�����ɃI�u�W�F�N�g�ǉ�
//			status = ProArrayObjectAdd((ProArray*)&feat_arr, PRO_VALUE_UNUSED, 1, &feat_num);
//			// �\�[�g���ăI�u�W�F�N�g�ǉ�
//			//status = ProUtilSortedIntArrayObjectAdd(&feat_arr, feat_num);
//			if (status != PRO_TK_NO_ERROR) {
//				//continue;
//				return;
//			}
//		}
//
//		status = ProArraySizeGet((ProArray)feat_arr, &n_sel);
//
//		status = ProArrayAlloc(0, sizeof(int), 1, (ProArray*)&p_feat_id_array);
//		if (status != PRO_TK_NO_ERROR) {
//			//break;
//			return;
//
//		}
//
//		for (i = 0; i < n_sel; i++)
//		{
//			// �A�N�e�B�u�t�B�[�`���݂̂�modelitem�ɐݒ�
//			status = ProUtilFeatByNumberInit(p_solid[0], feat_arr[i],(ProFeature*)&modelitem);
//
//			ProArrayObjectAdd((ProArray*)&p_feat_id_array, PRO_VALUE_UNUSED,1, &modelitem.id);
//		}
//
//		// �O���[�v�̍쐬
//		status = ProLocalGroupCreate((ProSolid)modelitem.owner, p_feat_id_array, n_sel, L"LOCAL_GROUP", &group);
//
//		status = ProArrayFree((ProArray*)&feat_arr);
//
//		status = ProArrayFree((ProArray*)&p_feat_id_array);
//
//
//
//	// �����I�����Ȃ������ꍇ�̓��f���c���[�̍X�V���s��Ȃ�
//	if (modelitem.type != PRO_TYPE_UNUSED) {
//		// ���f���c���[�̕\���X�V
//		status = ProTreetoolRefresh(modelitem.owner);
//
//	}
//
//
//	return PRO_TK_NO_ERROR;
//}
//
///*====================================================================*\
//FUNCTION : createDatumAxis
//PURPOSE  : �f�[�^�����ʂ̌�����f�[�^�����Ƃ���
//ProMdl model			in	�f�[�^�����ʂ̃I�[�i
//ProFeature feature_1	in	�f�[�^������1
//ProFeature feature_2	in	�f�[�^������2
//ProFeature* dtmAxisFeature	out	�f�[�^����
//\*====================================================================*/
//ProError createDatumAxis(ProMdl model, ProFeature feature_1, ProFeature feature_2, ProFeature* dtmAxisFeature)
//{
//	ProErrorlist            errors;
//	ProModelitem            model_item;
//	ProSelection            model_sel;
//	ProFeature              feature;
//	ProFeatureCreateOptions opts[1];
//	ProAsmcomppath* p_comp_path = NULL;
//	ProValue                value;
//	char                    name[PRO_NAME_SIZE];
//	ProError    status;
//	ProModelitem mdlitem1, mdlitem2;
//	ProSelection pSel1, pSel2;
//
//	ProElement pro_e_feature_tree;
//	ProElement pro_e_feature_type;
//	ProElement pro_e_std_feature_name;
//	ProElement pro_e_dtmaxis_constraints;
//	ProElement pro_e_dtmaxis_constraint;
//	ProElement pro_e_dtmaxis_constr_type;
//	ProElement pro_e_dtmaxis_constr_ref;
//
//	ProName wide_string;
//	ProValueData value_data;
//	ProBoolean is_interactive = PRO_B_TRUE;
//
//
//	/*---------------------------------------------------------------*\
//	Populating root element PRO_E_FEATURE_TREE
//	\*---------------------------------------------------------------*/
//	status = ProElementAlloc(PRO_E_FEATURE_TREE, &pro_e_feature_tree);
//	/*---------------------------------------------------------------*\
//	Populating element PRO_E_FEATURE_TYPE
//	\*---------------------------------------------------------------*/
//	status = ProElementAlloc(PRO_E_FEATURE_TYPE, &pro_e_feature_type);
//	value_data.type = PRO_VALUE_TYPE_INT;
//	value_data.v.i = PRO_FEAT_DATUM_AXIS; /* 926 */
//	status = ProValueAlloc(&value);
//	status = ProValueDataSet(value, &value_data);
//	status = ProElementValueSet(pro_e_feature_type, value);
//	status = ProElemtreeElementAdd(pro_e_feature_tree, NULL, pro_e_feature_type);
//	/*---------------------------------------------------------------*\
//	Populating element PRO_E_DTMAXIS_CONSTRAINTS
//	\*---------------------------------------------------------------*/
//	status = ProElementAlloc(PRO_E_DTMAXIS_CONSTRAINTS, &pro_e_dtmaxis_constraints);
//	status = ProElemtreeElementAdd(pro_e_feature_tree, NULL, pro_e_dtmaxis_constraints);
//	/*---------------------------------------------------------------*\
//	Populating element PRO_E_DTMAXIS_CONSTRAINTS
//	-> PRO_E_DTMAXIS_CONSTRAINT
//	  -> PRO_E_DTMAXIS_CONSTR_TYPE
//	  -> PRO_E_DTMAXIS_CONSTR_REF
//	\*---------------------------------------------------------------*/
//	status = ProElementAlloc(PRO_E_DTMAXIS_CONSTRAINT,&pro_e_dtmaxis_constraint);
//	status = ProElemtreeElementAdd(pro_e_dtmaxis_constraints, NULL, pro_e_dtmaxis_constraint);
//
//	status = ProElementAlloc(PRO_E_DTMAXIS_CONSTR_TYPE, &pro_e_dtmaxis_constr_type);
//	value_data.type = PRO_VALUE_TYPE_INT;
//	value_data.v.i = PRO_DTMAXIS_CONSTR_TYPE_THRU; /* 1 ProDtmaxisConstrType */
//	status = ProValueAlloc(&value);
//	status = ProValueDataSet(value, &value_data);
//	status = ProElementValueSet(pro_e_dtmaxis_constr_type, value);
//	status = ProElemtreeElementAdd(pro_e_dtmaxis_constraint, NULL, pro_e_dtmaxis_constr_type);
//
//	ProSelection* p_select;
//	int 		n_select;
//
//
//	
//	status = ProFeatureGeomitemVisit(&feature_1, PRO_SURFACE, UsrPointAddAction, NULL, (ProAppData)&mdlitem1);
//	//status = ProModelitemInit(feature_1.owner, feature_1.id+1, PRO_SURFACE, &mdlitem1);
//	status = ProSelectionAlloc(NULL, &mdlitem1, &pSel1);
//
//	status = ProElementAlloc(PRO_E_DTMAXIS_CONSTR_REF, &pro_e_dtmaxis_constr_ref);
//	value_data.type = PRO_VALUE_TYPE_SELECTION;
//	value_data.v.r = pSel1;
//	status = ProValueAlloc(&value);
//	status = ProValueDataSet(value, &value_data);
//	status = ProElementValueSet(pro_e_dtmaxis_constr_ref, value);
//	status = ProElemtreeElementAdd(pro_e_dtmaxis_constraint, NULL, pro_e_dtmaxis_constr_ref);
//	/*---------------------------------------------------------------*\
//	Populating element PRO_E_DTMAXIS_CONSTRAINTS
//	-> PRO_E_DTMAXIS_CONSTRAINT
//	  -> PRO_E_DTMAXIS_CONSTR_REF
//      -> PRO_E_DTMAXIS_CONSTR_TYPE
//	\*---------------------------------------------------------------*/
//	status = ProElementAlloc(PRO_E_DTMAXIS_CONSTRAINT, &pro_e_dtmaxis_constraint);
//	status = ProElemtreeElementAdd(pro_e_dtmaxis_constraints, NULL, pro_e_dtmaxis_constraint);
//
//	status = ProElementAlloc(PRO_E_DTMAXIS_CONSTR_TYPE, &pro_e_dtmaxis_constr_type);
//	value_data.type = PRO_VALUE_TYPE_INT;
//	value_data.v.i = PRO_DTMAXIS_CONSTR_TYPE_THRU; /* 1 ProDtmaxisConstrType */
//	status = ProValueAlloc(&value);
//	status = ProValueDataSet(value, &value_data);
//	status = ProElementValueSet(pro_e_dtmaxis_constr_type, value);
//	status = ProElemtreeElementAdd(pro_e_dtmaxis_constraint, NULL, pro_e_dtmaxis_constr_type);
//
//	status = ProFeatureGeomitemVisit(&feature_2, PRO_SURFACE, UsrPointAddAction, NULL, (ProAppData)&mdlitem2);
//	//status = ProModelitemInit(feature_2.owner, feature_2.id + 1, PRO_SURFACE, &mdlitem2);
//	status = ProSelectionAlloc(NULL, &mdlitem2, &pSel2);
//	status = ProElementAlloc(PRO_E_DTMAXIS_CONSTR_REF, &pro_e_dtmaxis_constr_ref);
//	value_data.type = PRO_VALUE_TYPE_SELECTION;
//	value_data.v.r = pSel2;
//	status = ProValueAlloc(&value);
//	status = ProValueDataSet(value, &value_data);
//	status = ProElementValueSet(pro_e_dtmaxis_constr_ref, value);
//	status = ProElemtreeElementAdd(pro_e_dtmaxis_constraint, NULL, pro_e_dtmaxis_constr_ref);
//	/*---------------------------------------------------------------*\
//	Create the feature in the current model.
//	\*---------------------------------------------------------------*/
//
//	if (status != PRO_TK_NO_ERROR) return;
//	status = ProMdlToModelitem(model, &model_item);
//	status = ProSelectionAlloc(p_comp_path, &model_item, &model_sel);
//
//	opts[0] = PRO_FEAT_CR_NO_OPTS;
//
//	status = ProFeatureCreate(model_sel, pro_e_feature_tree, opts, 1, dtmAxisFeature, &errors);
//	status = ProElementFree(&pro_e_feature_tree);
//
//	return status;
//}

///*====================================================================*\
//FUNCTION : createDatumPlane
//PURPOSE  : CSYS����ɂ��ăf�[�^�����ʂ��쐬����
// curMdl			(in) ��ƂȂ�CSYS�������i/�A�Z���u��
// offsetDirection(in) �f�[�^�����ʂ̌���
// offsetValue	(in) �f�[�^�����ʂ̋��� 
// dtmPlnFeature	(out) �f�[�^�����ʂ̃n���h��
//\*====================================================================*/
//ProError  createDatumPlane(ProMdl curMdl, int offsetDirection, double offsetValue, ProFeature* dtmPlnFeature)
//{
//	ProError status = PRO_TK_NO_ERROR;
//
//	ProElement featElemTree;
//	status = ProElementAlloc(PRO_E_FEATURE_TREE, &featElemTree);
//
//	// �t�B�[�`���^�C�v (�f�[�^��) PRO_E_FEATURE_TYPE
//	ProElement featureTypeElem;
//	status = ProElementAlloc(PRO_E_FEATURE_TYPE, &featureTypeElem);
//	status = ProElementIntegerSet(featureTypeElem, PRO_FEAT_DATUM);
//	status = ProElemtreeElementAdd(featElemTree, NULL, featureTypeElem);
//
//	//// �t�B�[�`���� �̐ݒ� PRO_E_STD_FEATURE_NAME ���ݒ�̂��߁A�f�t�H���g
//	//ProElement featureNameElem;
//	//wchar_t* featureName = L"TEST_DTM_PLANE";
//	//status = ProElementAlloc(PRO_E_STD_FEATURE_NAME, &featureNameElem);
//	//status = ProElementWstringSet(featureNameElem, featureName);
//	//status = ProElemtreeElementAdd(featElemTree, NULL, featureNameElem);
//
//	////////////////////////////////////////////////////////////////////
//	// �f�[�^���z�u�̐ݒ� 
//	// 
//	//PRO_E_DTMPLN_CONSTRAINTS
//	//  |--PRO_E_DTMPLN_CONSTRAINT
//	//    |--PRO_E_DTMPLN_CONSTR_TYPE�@CSYS���ɉ������I�t�Z�b�g
//	//    |--PRO_E_DTMPLN_CONSTR_REF   ���ƂȂ�CSYS�̒�`
//	//    |--PRO_E_DTMPLN_OFF_CSYS			�I�t�Z�b�g���� (X,Y,Z)
//	//    |--PRO_E_DTMPLN_OFF_CSYS_OFFSET	�I�t�Z�b�g�l
//
//	ProElement dtmplnConstraintsElem;
//	status = ProElementAlloc(PRO_E_DTMPLN_CONSTRAINTS, &dtmplnConstraintsElem);
//	status = ProElemtreeElementAdd(featElemTree, NULL, dtmplnConstraintsElem);
//
//	ProElement* dtmplnConstraints = NULL;
//	status = ProArrayAlloc(0, sizeof(ProElement), 1, (ProArray*)&dtmplnConstraints);
//
//	ProElement dtmplnConstraintElem1;
//	status = ProElementAlloc(PRO_E_DTMPLN_CONSTRAINT, &dtmplnConstraintElem1);
//
//	// �f�[�^�����ʂ̍쐬��� CSYS���ɉ������I�t�Z�b�g
//	ProElement dtmplnConstrTypeElem;
//	status = ProElementAlloc(PRO_E_DTMPLN_CONSTR_TYPE, &dtmplnConstrTypeElem);
//	status = ProElementIntegerSet(dtmplnConstrTypeElem, PRO_DTMPLN_OFFS);
//	status = ProElemtreeElementAdd(dtmplnConstraintElem1, NULL, dtmplnConstrTypeElem);
//
//	// ���ƂȂ�CSYS�̒�`
//	ProMdlName nm;
//	status = ProMdlMdlnameGet(curMdl, nm);
//
//	ProIdTable idTable;
//	idTable[0] = -1;
//
//	ProAsmcomppath compPath;
//	status = ProAsmcomppathInit((ProSolid)curMdl, idTable, 0, &compPath);
//
//	ProMdl componentMdl;
//	status = ProAsmcomppathMdlGet(&compPath, &componentMdl);
//
//	ProModelitem dtmplnConstrMdlItem;
//	status = ProModelitemInit(componentMdl, 8, PRO_CSYS, &dtmplnConstrMdlItem);
//
//	ProSelection dtmplnConstrSel;
//	status = ProSelectionAlloc(&compPath, &dtmplnConstrMdlItem, &dtmplnConstrSel);
//
//	ProReference dtmplnConstrRef;
//	status = ProSelectionToReference(dtmplnConstrSel, &dtmplnConstrRef);
//
//	ProElement dtmplnConstrRefElem;
//	status = ProElementAlloc(PRO_E_DTMPLN_CONSTR_REF, &dtmplnConstrRefElem);
//	status = ProElementReferenceSet(dtmplnConstrRefElem, dtmplnConstrRef);
//	status = ProElemtreeElementAdd(dtmplnConstraintElem1, NULL, dtmplnConstrRefElem);
//
//	// �I�t�Z�b�g���� Y 
//	ProElement dtmplnOffCsysElem;
//	status = ProElementAlloc(PRO_E_DTMPLN_OFF_CSYS, &dtmplnOffCsysElem);
//	//status = ProElementIntegerSet(dtmplnOffCsysElem, PRO_DTMPLN_OFF_CSYS_X);
//	status = ProElementIntegerSet(dtmplnOffCsysElem, offsetDirection);
//	status = ProElemtreeElementAdd(dtmplnConstraintElem1, NULL, dtmplnOffCsysElem);
//
//	// �I�t�Z�b�g�l 23 
//	ProElement dtmplnOffCsysOffsetElem;
//	status = ProElementAlloc(PRO_E_DTMPLN_OFF_CSYS_OFFSET, &dtmplnOffCsysOffsetElem);
//	//status = ProElementDoubleSet(dtmplnOffCsysOffsetElem, 100.0);
//	status = ProElementDoubleSet(dtmplnOffCsysOffsetElem, offsetValue);
//	status = ProElemtreeElementAdd(dtmplnConstraintElem1, NULL, dtmplnOffCsysOffsetElem);
//
//	status = ProArrayObjectAdd((ProArray*)&dtmplnConstraints, PRO_VALUE_UNUSED, 1, &dtmplnConstraintElem1);
//
//	//Set value for PRO_E_DTMPLN_CONSTRAINTS element.
//	status = ProElementArraySet(dtmplnConstraintsElem, NULL, dtmplnConstraints);
//
//	// ���]���� 
//	ProElement dtmplnFlipDirElem;
//	status = ProElementAlloc(PRO_E_DTMPLN_FLIP_DIR, &dtmplnFlipDirElem);
//	status = ProElementIntegerSet(dtmplnFlipDirElem, PRO_DTMPLN_FLIP_DIR_NO);
//	status = ProElemtreeElementAdd(featElemTree, NULL, dtmplnFlipDirElem);
//
//	// �v�f�c���[����t�B�[�`�����쐬����
//	ProModelitem compMdlModelItem;
//	status = ProMdlToModelitem(componentMdl, &compMdlModelItem);
//
//	ProSelection compMdlSel;
//	status = ProSelectionAlloc(&compPath, &compMdlModelItem, &compMdlSel);
//
//	ProFeatureCreateOptions* featCreationOpts = NULL;
//	status = ProArrayAlloc(1, sizeof(ProFeatureCreateOptions), 1, (ProArray*)&featCreationOpts);
//	featCreationOpts[0] = PRO_FEAT_CR_NO_OPTS;
//
//	//ProFeature dtmPlnFeature;
//	ProErrorlist errorList;
//	status = ProFeatureWithoptionsCreate(
//		compMdlSel,
//		featElemTree,
//		featCreationOpts,
//		PRO_REGEN_NO_FLAGS,
//		dtmPlnFeature,
//		&errorList);
//
//	return status;
//}
//
//
//
//
//
///*====================================================================*\
//FUNCTION : createHoleAroundAxis
//PURPOSE  : �f�[�^�����Ɍ����쐬
//\*====================================================================*/
//ProError  createHoleAroundAxis(ProMdl model, ProFeature dtmAxisFeature, ProFeature dtmPlnFeature_Y)
//{
//
//	ProError status;
//
//	ProElement feat_elemtree;
//	ProElement elem_feattype;
//	ProElement elem_featform;
//
//	ProElement elem_hle_com;
//	ProElement elem_hle_type_new;
//	ProElement elem_hle_stan_type;
//
//	ProElement elem_diameter;
//
//	ProElement elem_hole_std_depth;
//	ProElement elem_hole_depth_to;
//	ProElement elem_hole_depth_to_type;
//	ProElement elem_ext_depth_to_value;
//	ProElement elem_ext_depth_to_ref;
//	ProElement elem_hole_depth_from;
//	ProElement elem_hole_depth_from_type;
//	ProElement elem_ext_depth_from_value;
//	ProElement elem_ext_depth_from_ref;
//
//	ProElement elem_hle_placement;
//	ProElement elem_hle_prim_ref;
//	ProElement elem_hle_pl_type;
//	ProElement elem_hle_dim_ref1;
//	ProElement elem_hle_dim_dist1;
//	ProElement elem_hle_dim_ref2;
//	ProElement elem_hle_dim_dist2;
//
//	ProValue value;
//	ProValueData value_data;
//
//	ProSelection* p_selection;
//	int n_selection;
//
//	ProFeatureCreateOptions* options = 0;
//	ProFeature created_feature;
//	ProErrorlist p_errors;
//	ProModelitem model_item;
//	ProSelection model_selection;
//	ProReference reference;
//
//
//
//
//	/***********************************************
//	 �v�f�c���[�쐬�̊J�n
//	*************************************************/
//
//	/* �v�f�c���[(���[�g�v�f)�̒ǉ� */
//	status = ProElementAlloc(PRO_E_FEATURE_TREE, &feat_elemtree);
//
//	/***********************************************
//	 PRO_E_FEATURE_TYPE
//	*************************************************/
//	/* �v�f�c���[�� �t�B�[�`���^�C�v(��) ��ǉ����� */
//	status = ProElementAlloc(PRO_E_FEATURE_TYPE, &elem_feattype);
//	status = ProElementIntegerSet(elem_feattype, PRO_FEAT_HOLE);
//	status = ProElemtreeElementAdd(feat_elemtree, NULL, elem_feattype);
//
//	/***********************************************
//	 PRO_E_FEATURE_FORM
//	*************************************************/
//	/* �v�f�c���[�� �t�B�[�`���t�H�[��(�X�g���[�g�z�[��) ��ǉ����� */
//	status = ProElementAlloc(PRO_E_FEATURE_FORM, &elem_featform);
//	status = ProElementIntegerSet(elem_featform, PRO_HLE_TYPE_STRAIGHT);
//	status = ProElemtreeElementAdd(feat_elemtree, NULL, elem_featform);
//
//	/***********************************************
//	 PRO_E_HLE_COM
//	*************************************************/
//	/* �v�f�c���[�� �����̋��ʗv�f �̒ǉ�  */
//	status = ProElementAlloc(PRO_E_HLE_COM, &elem_hle_com);
//	status = ProElemtreeElementAdd(feat_elemtree, NULL, elem_hle_com);
//
//	/* ����� (���^�C�v:�X�g���[�g) ���w�肷�� */
//	status = ProElementAlloc(PRO_E_HLE_TYPE_NEW, &elem_hle_type_new);
//	status = ProElementIntegerSet(elem_hle_type_new, PRO_HLE_NEW_TYPE_STRAIGHT);
//	status = ProElemtreeElementAdd(elem_hle_com, NULL, elem_hle_type_new);
//
//	/* ����� (���̒��a:100) ���w�肷�� */
//	status = ProElementAlloc(PRO_E_DIAMETER, &elem_diameter);
//	status = ProElementDoubleSet(elem_diameter, (double)100.0);
//	status = ProElemtreeElementAdd(elem_hle_com, NULL, elem_diameter);
//
//	/* �W���[�x�̗v�f��ǉ�����
//			  |--PRO_E_HOLE_STD_DEPTH						�[���v�f
//			  |    |--PRO_E_HOLE_DEPTH_TO					- �[��2
//			  |    |    |--PRO_E_HOLE_DEPTH_TO_TYPE			- - �[��2
//			  |    |--PRO_E_HOLE_DEPTH_FROM					- �[��1
//			  |         |--PRO_E_HOLE_DEPTH_FROM_TYPE		- - �[��1
//
//	*/
//
//	// �[���v�f
//	status = ProElementAlloc(PRO_E_HOLE_STD_DEPTH, &elem_hole_std_depth);
//	status = ProElemtreeElementAdd(elem_hle_com, NULL, elem_hole_std_depth);
//	// side1 ���F�S�ђ�
//	status = ProElementAlloc(PRO_E_HOLE_DEPTH_TO, &elem_hole_depth_to);
//	status = ProElemtreeElementAdd(elem_hole_std_depth, NULL, elem_hole_depth_to);
//	status = ProElementAlloc(PRO_E_HOLE_DEPTH_TO_TYPE, &elem_hole_depth_to_type);
//	status = ProElementIntegerSet(elem_hole_depth_to_type, PRO_HLE_STRGHT_THRU_NEXT_DEPTH);
//	status = ProElemtreeElementAdd(elem_hole_depth_to, NULL, elem_hole_depth_to_type);
//	// side2 ���F�S�ђ�
//	status = ProElementAlloc(PRO_E_HOLE_DEPTH_FROM, &elem_hole_depth_from);
//	status = ProElemtreeElementAdd(elem_hole_std_depth, NULL, elem_hole_depth_from);
//	status = ProElementAlloc(PRO_E_HOLE_DEPTH_FROM_TYPE, &elem_hole_depth_from_type);
//	status = ProElementIntegerSet(elem_hole_depth_from_type, PRO_HLE_STRGHT_THRU_NEXT_DEPTH);
//	status = ProElemtreeElementAdd(elem_hole_depth_from, NULL, elem_hole_depth_from_type);
//
//
//	/***********************************************
//	 PRO_E_HLE_PLACEMENT
//	*************************************************/
//	/* �z�u�̏ڍׂɊ֘A����v�f��ǉ�����
//
//	 �������Ƃ��铯����
//
//		 |--PRO_E_HLE_PLACEMENT
//		 |    |--PRO_E_HLE_PRIM_REF
//		 |    |--PRO_E_HLE_PL_TYPE  = PRO_HLE_PL_TYPE_COAX
//		 |    |--PRO_E_HLE_PLCMNT_PLANE
//
//	*/
//
//	status = ProElementAlloc(PRO_E_HLE_PLACEMENT, &elem_hle_placement);
//	status = ProElemtreeElementAdd(feat_elemtree, NULL, elem_hle_placement);
//
//	// �ꎟ�I���B�� �@PRO_VALUE_TYPE_SELECTION
//	ProModelitem mdlitem1;
//	ProSelection pSel1;
//	status = ProFeatureGeomitemVisit(&dtmAxisFeature, PRO_AXIS, UsrPointAddAction, NULL, (ProAppData)&mdlitem1);
//	//status = ProModelitemInit(dtmAxisFeature.owner, dtmAxisFeature.id + 3, PRO_AXIS, &mdlitem1);
//	status = ProSelectionAlloc(NULL, &mdlitem1, &pSel1);
//
//	status = ProElementAlloc(PRO_E_HLE_PRIM_REF, &elem_hle_prim_ref);
//	value_data.type = PRO_VALUE_TYPE_SELECTION;
//	value_data.v.r = pSel1;
//	status = ProValueAlloc(&value);
//	status = ProValueDataSet(value, &value_data);
//	status = ProElementValueSet(elem_hle_prim_ref, value);
//	status = ProElemtreeElementAdd(elem_hle_placement, NULL, elem_hle_prim_ref);
//
//
//	// �������Ƃ��铯����
//	status = ProElementAlloc(PRO_E_HLE_PL_TYPE, &elem_hle_pl_type);
//	status = ProElementIntegerSet(elem_hle_pl_type, PRO_HLE_PL_TYPE_COAX);
//	status = ProElemtreeElementAdd(elem_hle_placement, NULL, elem_hle_pl_type);
//
//	// �z�u�ʁ@PRO_VALUE_TYPE_SELECTION (Y������)
//	ProModelitem mdlitem2;
//	ProSelection pSel2;
//	status = ProFeatureGeomitemVisit(&dtmPlnFeature_Y, PRO_SURFACE, UsrPointAddAction, NULL, (ProAppData)&mdlitem2);
//	//status = ProModelitemInit(dtmPlnFeature_Y.owner, dtmPlnFeature_Y.id + 1, PRO_SURFACE, &mdlitem2);
//	status = ProSelectionAlloc(NULL, &mdlitem2, &pSel2);
//
//	status = ProElementAlloc(PRO_E_HLE_PLCMNT_PLANE, &elem_hle_prim_ref);
//	value_data.type = PRO_VALUE_TYPE_SELECTION;
//	value_data.v.r = pSel2;
//	status = ProValueAlloc(&value);
//	status = ProValueDataSet(value, &value_data);
//	status = ProElementValueSet(elem_hle_prim_ref, value);
//	status = ProElemtreeElementAdd(elem_hle_placement, NULL, elem_hle_prim_ref);
//
//	/* �v�f�c���[�쐬�̏I�� */
//
//
//	/***********************************************
//	 �t�B�[�`���[�̍쐬
//	*************************************************/
//	status = ProMdlToModelitem(model, &model_item);
//	status = ProSelectionAlloc(NULL, &model_item, &model_selection);
//	status = ProArrayAlloc(1, sizeof(ProFeatureCreateOptions), 1, (ProArray*)&options);
//	options[0] = PRO_FEAT_CR_NO_OPTS;
//	status = ProFeatureWithoptionsCreate(model_selection, feat_elemtree, options, PRO_REGEN_NO_FLAGS, &created_feature, &p_errors);
//	status = ProArrayFree((ProArray*)&options);
//
//	/***********************************************
//	 ���\�[�X�̉��
//	*************************************************/
//	status = ProElementFree(&feat_elemtree);
//	status = ProSelectionFree(&model_selection);
//
//	return (status);
//}
//
//
//
///*====================================================================*\
//FUNCTION : createHoleAroundPlane
//PURPOSE  : �f�[�^�����ʂ���Ɍ����쐬
//\*====================================================================*/
//ProError createHoleAroundPlane(ProMdl model,
//								ProFeature dtmPlnFeature_Y, 
//								ProFeature dtmPlnFeature_X, 
//								ProFeature dtmPlnFeature_Z,
//								double distX,
//								double distZ)
//{
//	ProError status;
//
//	ProElement feat_elemtree;
//	ProElement elem_feattype;
//	ProElement elem_featform;
//
//	ProElement elem_hle_com;
//	ProElement elem_hle_type_new;
//	ProElement elem_hle_stan_type;
//
//	ProElement elem_diameter;
//
//	ProElement elem_hole_std_depth;
//	ProElement elem_hole_depth_to;
//	ProElement elem_hole_depth_to_type;
//	ProElement elem_ext_depth_to_value;
//	ProElement elem_ext_depth_to_ref;
//	ProElement elem_hole_depth_from;
//	ProElement elem_hole_depth_from_type;
//	ProElement elem_ext_depth_from_value;
//	ProElement elem_ext_depth_from_ref;
//
//	ProElement elem_hle_placement;
//	ProElement elem_hle_prim_ref;
//	ProElement elem_hle_pl_type;
//	ProElement elem_hle_dim_ref1;
//	ProElement elem_hle_dim_dist1;
//	ProElement elem_hle_dim_ref2;
//	ProElement elem_hle_dim_dist2;
//
//	ProValue value;
//	ProValueData value_data;
//
//	ProSelection* p_selection;
//	int n_selection;
//
//	ProFeatureCreateOptions* options = 0;
//	ProFeature created_feature;
//	ProErrorlist p_errors;
//	//ProMdl model;
//	ProModelitem model_item;
//	ProSelection model_selection;
//	ProReference reference;
//
//
//	/***********************************************
//	 �v�f�c���[�쐬�̊J�n
//	*************************************************/
//
//	/* �v�f�c���[(���[�g�v�f)�̒ǉ� */
//	status = ProElementAlloc(PRO_E_FEATURE_TREE, &feat_elemtree);
//
//	/***********************************************
//	 PRO_E_FEATURE_TYPE
//	*************************************************/
//	/* �v�f�c���[�� �t�B�[�`���^�C�v(��) ��ǉ����� */
//	status = ProElementAlloc(PRO_E_FEATURE_TYPE, &elem_feattype);
//	status = ProElementIntegerSet(elem_feattype, PRO_FEAT_HOLE);
//	status = ProElemtreeElementAdd(feat_elemtree, NULL, elem_feattype);
//
//	/***********************************************
//	 PRO_E_FEATURE_FORM
//	*************************************************/
//	/* �v�f�c���[�� �t�B�[�`���t�H�[��(�X�g���[�g�z�[��) ��ǉ����� */
//	status = ProElementAlloc(PRO_E_FEATURE_FORM, &elem_featform);
//	status = ProElementIntegerSet(elem_featform, PRO_HLE_TYPE_STRAIGHT);
//	status = ProElemtreeElementAdd(feat_elemtree, NULL, elem_featform);
//
//
//	/***********************************************
//	 PRO_E_HLE_COM
//	*************************************************/
//	/* �v�f�c���[�� �����̋��ʗv�f �̒ǉ�  */
//	status = ProElementAlloc(PRO_E_HLE_COM, &elem_hle_com);
//	status = ProElemtreeElementAdd(feat_elemtree, NULL, elem_hle_com);
//
//	/* ����� (���^�C�v:�X�g���[�g) ���w�肷�� */
//	status = ProElementAlloc(PRO_E_HLE_TYPE_NEW, &elem_hle_type_new);
//	status = ProElementIntegerSet(elem_hle_type_new, PRO_HLE_NEW_TYPE_STRAIGHT);
//	status = ProElemtreeElementAdd(elem_hle_com, NULL, elem_hle_type_new);
//
//	/* ����� (���̒��a:100) ���w�肷�� */
//	status = ProElementAlloc(PRO_E_DIAMETER, &elem_diameter);
//	status = ProElementDoubleSet(elem_diameter, (double)100.0);
//	status = ProElemtreeElementAdd(elem_hle_com, NULL, elem_diameter);
//
//	/* �W���[�x�̗v�f��ǉ�����
//
//			  |--PRO_E_HOLE_STD_DEPTH
//			  |    |--PRO_E_HOLE_DEPTH_TO
//			  |    |    |--PRO_E_HOLE_DEPTH_TO_TYPE
//			  |    |    |--PRO_E_EXT_DEPTH_TO_VALUE
//			  |    |    |--PRO_E_EXT_DEPTH_TO_REF
//			  |    |--PRO_E_HOLE_DEPTH_FROM
//			  |         |--PRO_E_HOLE_DEPTH_FROM_TYPE
//			  |         |--PRO_E_EXT_DEPTH_FROM_VALUE
//			  |         |--PRO_E_EXT_DEPTH_FROM_REF
//
//	*/
//
//	// �[���v�f
//	status = ProElementAlloc(PRO_E_HOLE_STD_DEPTH, &elem_hole_std_depth);
//	status = ProElemtreeElementAdd(elem_hle_com, NULL, elem_hole_std_depth);
//
//	// side1 ���
//	status = ProElementAlloc(PRO_E_HOLE_DEPTH_TO, &elem_hole_depth_to);
//	status = ProElemtreeElementAdd(elem_hole_std_depth, NULL, elem_hole_depth_to);
//	status = ProElementAlloc(PRO_E_HOLE_DEPTH_TO_TYPE, &elem_hole_depth_to_type);
//	status = ProElementIntegerSet(elem_hole_depth_to_type, PRO_HLE_STRGHT_THRU_ALL_DEPTH);
//	status = ProElemtreeElementAdd(elem_hole_depth_to, NULL, elem_hole_depth_to_type);
//	// side2 ���
//	status = ProElementAlloc(PRO_E_HOLE_DEPTH_FROM, &elem_hole_depth_from);
//	status = ProElemtreeElementAdd(elem_hole_std_depth, NULL, elem_hole_depth_from);
//	status = ProElementAlloc(PRO_E_HOLE_DEPTH_FROM_TYPE, &elem_hole_depth_from_type);
//	status = ProElementIntegerSet(elem_hole_depth_from_type, PRO_HLE_STRGHT_NONE_DEPTH);
//	status = ProElemtreeElementAdd(elem_hole_depth_from, NULL, elem_hole_depth_from_type);
//
//
//	/* �z�u�̏ڍׂɊ֘A����v�f��ǉ�����
//
//	 �������Ƃ��铯����
//
//		 |--PRO_E_HLE_PLACEMENT
//		 |    |--PRO_E_HLE_PRIM_REF
//		 |    |--PRO_E_HLE_PL_TYPE
//		 |    |--PRO_E_HLE_DIM_REF1
//		 |    |--PRO_E_HLE_DIM_DIST1
//		 |    |--PRO_E_HLE_DIM_REF2
//		 |    |--PRO_E_HLE_DIM_DIST2
//
//	*/
//
//	status = ProElementAlloc(PRO_E_HLE_PLACEMENT, &elem_hle_placement);
//	status = ProElemtreeElementAdd(feat_elemtree, NULL, elem_hle_placement);
//
//	// �ꎟ�I�� (��������)
//	ProModelitem mdlitem1;
//	ProSelection pSel1;
//	status = ProFeatureGeomitemVisit(&dtmPlnFeature_Y, PRO_SURFACE, UsrPointAddAction, NULL, (ProAppData)&mdlitem1);
//	//status = ProModelitemInit(dtmPlnFeature_Y.owner, dtmPlnFeature_Y.id + 1, PRO_SURFACE, &mdlitem1);
//	status = ProSelectionAlloc(NULL, &mdlitem1, &pSel1);
//
//	status = ProElementAlloc(PRO_E_HLE_PRIM_REF, &elem_hle_prim_ref);
//	value_data.type = PRO_VALUE_TYPE_SELECTION;
//	value_data.v.r = pSel1;
//	status = ProValueAlloc(&value);
//	status = ProValueDataSet(value, &value_data);
//	status = ProElementValueSet(elem_hle_prim_ref, value);
//	status = ProElemtreeElementAdd(elem_hle_placement, NULL, elem_hle_prim_ref);
//	// ���z�u�I�v�V����
//	status = ProElementAlloc(PRO_E_HLE_PL_TYPE, &elem_hle_pl_type);
//	status = ProElementIntegerSet(elem_hle_pl_type, PRO_HLE_PL_TYPE_LIN);
//	status = ProElemtreeElementAdd(elem_hle_placement, NULL, elem_hle_pl_type);
//
//
//	// �񎟑I�� (X��)
//	ProModelitem mdlitem2;
//	ProSelection pSel2;
//	status = ProFeatureGeomitemVisit(&dtmPlnFeature_X, PRO_SURFACE, UsrPointAddAction, NULL, (ProAppData)&mdlitem2);
//	//status = ProModelitemInit(dtmPlnFeature_X.owner, dtmPlnFeature_X.id + 1, PRO_SURFACE, &mdlitem2);
//	status = ProSelectionAlloc(NULL, &mdlitem2, &pSel2);
//
//	status = ProElementAlloc(PRO_E_HLE_DIM_REF1, &elem_hle_prim_ref);
//	value_data.type = PRO_VALUE_TYPE_SELECTION;
//	value_data.v.r = pSel2;
//	status = ProValueAlloc(&value);
//	status = ProValueDataSet(value, &value_data);
//	status = ProElementValueSet(elem_hle_prim_ref, value);
//	status = ProElemtreeElementAdd(elem_hle_placement, NULL, elem_hle_prim_ref);
//
//	// ���� 1
//	status = ProElementAlloc(PRO_E_HLE_DIM_DIST1, &elem_hle_dim_dist1);
//	status = ProElementDoubleSet(elem_hle_dim_dist1, distX);
//	status = ProElemtreeElementAdd(elem_hle_placement, NULL, elem_hle_dim_dist1);
//
//	// �O���I�� (Y��)
//	ProModelitem mdlitem3;
//	ProSelection pSel3;
//	status = ProFeatureGeomitemVisit(&dtmPlnFeature_Z, PRO_SURFACE, UsrPointAddAction, NULL, (ProAppData)&mdlitem3);
//	//status = ProModelitemInit(dtmPlnFeature_Z.owner, dtmPlnFeature_Z.id + 1, PRO_SURFACE, &mdlitem3);
//	status = ProSelectionAlloc(NULL, &mdlitem3, &pSel3);
//
//	status = ProElementAlloc(PRO_E_HLE_DIM_REF2, &elem_hle_prim_ref);
//	value_data.type = PRO_VALUE_TYPE_SELECTION;
//	value_data.v.r = pSel3;
//	status = ProValueAlloc(&value);
//	status = ProValueDataSet(value, &value_data);
//	status = ProElementValueSet(elem_hle_prim_ref, value);
//	status = ProElemtreeElementAdd(elem_hle_placement, NULL, elem_hle_prim_ref);
//
//	// ���� 2
//	status = ProElementAlloc(PRO_E_HLE_DIM_DIST2, &elem_hle_dim_dist2);
//	status = ProElementDoubleSet(elem_hle_dim_dist2, distZ);
//	status = ProElemtreeElementAdd(elem_hle_placement, NULL, elem_hle_dim_dist2);
//	/* �v�f�c���[�쐬�̏I�� */
//
//
//	/***********************************************
//	 �t�B�[�`���[�̍쐬
//	*************************************************/
//	status = ProMdlToModelitem(model, &model_item);
//	status = ProSelectionAlloc(NULL, &model_item, &model_selection);
//	status = ProArrayAlloc(1, sizeof(ProFeatureCreateOptions),1, (ProArray*)&options);
//	options[0] = PRO_FEAT_CR_NO_OPTS;
//	status = ProFeatureWithoptionsCreate(model_selection, feat_elemtree,options, PRO_REGEN_NO_FLAGS, &created_feature, &p_errors);
//	status = ProArrayFree((ProArray*)&options);
//
//	/***********************************************
//	 ���\�[�X�̉��
//	*************************************************/
//	status = ProElementFree(&feat_elemtree);
//	status = ProSelectionFree(&model_selection);
//
//	return (status);
//}