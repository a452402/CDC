/*--------------------------------------------------------------------*\
C includes
\*--------------------------------------------------------------------*/
#include <windows.h>
#include <float.h>

/*--------------------------------------------------------------------*\
Pro/Toolkit includes
\*--------------------------------------------------------------------*/
#include <ProToolkitErrors.h>
#include <ProSizeConst.h>
#include <ProObjects.h>
#include <ProDimension.h>
#include <ProModelitem.h>
#include <ProFeature.h>
#include <ProSolid.h>
#include <ProSelbuffer.h>
#include <ProMenu.h>
#include <ProUtil.h>
#include <ProWindows.h>
#include <string.h>
#include <ProVariantFeat.h>
#include <ProCsys.h>
#include <ProWstring.h>
#include <ProMdl.h>
#include <ProHole.h>
#include <ProFeatType.h>
#include <ProSolid.h>
#include <ProGroup.h>
#include <ProDatumdata.h>
#include <ProUdf.h>
#include <ProDtmCsys.h>
#include <ProDtmPnt.h>

/*--------------------------------------------------------------------*\
Application includes
\*--------------------------------------------------------------------*/
#include "CommonUtil.h"
#include "InputFile.h"
#include "Holes_HoleTableSection.h"


/*-----------------------------------------------------------------*\
	�v���g�^�C�v�錾
\*-----------------------------------------------------------------*/
ProError createDatumPlaneX(ProMdl* curMdl, double offsetValue, ProFeature* dtmPlnFeature, int iDatumType);
ProError createDatumPlaneY(ProMdl* curMdl, ProMdl mdlPart, double offsetValue, ProFeature* dtmPlnFeature);
ProError createDatumPlaneZ(ProMdl* curMdl, ProMdl mdlPart, double offsetValue, ProModelitem dtmplnConstrMdlItem, ProFeature* dtmPlnFeature);
ProError createGroupFunction(ProSolid p_solid, ProName wGroupName, ProFeature featModelitem[], int iSel, ProGroup* group);
ProError createHoleAroundPlane(ProMdl top_asm, ProMdl part,	ProFeature dtmPlnFeature_X,	ProFeature dtmPlnFeature_Y,
	ProSelection selSurf, ProFeature dtmPlnFeature_selSurf, double distX, double distY, double dDiameter, ProFeature* holeFeature);

ProError createHoleBaseHoleOne(ProMdl top_asm, ProMdl part,ProFeature dtmPlnFeature_X,	ProFeature dtmPlnFeature_Y,	ProSelection selSurf, ProFeature dtmPlnFeature_selSurf,
	double distX, double distY,	double dDiameter, ProFeature* holeFeature);
ProError createHoleBaseHoleOneOther(ProMdl top_asm,	ProMdl part,ProFeature dtmPlnFeature_X, ProFeature selOffsetSurf,	ProSelection selSurf,
	double distX, double distY,	double dDiameter, ProFeature* holeFeature);
ProError getFeatureIdAction(ProFeature* pFeature, ProError status, ProAppData app_data);
ProError getUpperSurfaceFromYpointAction(ProSurface surface, ProError filt_status, ProAppData data);
ProError getLowerSurfaceFromYpointAction(ProSurface surface, ProError filt_status, ProAppData data);
ProError getSurfZpointAction(ProSurface surface, ProError status, ProAppData app_data);
ProError getHoleTableInfOfSameNum(InputFileHole* strHole, InputFileHoleTable* strHoleTableInput, int iHoleTableSectionMaxRows, int* iHoleTableCnt);
ProError getAsmcompathIncludePart(ProMdl top_asm, ProMdl part, ProAsmcomppath* compPath);
ProError  convertGroupToFeature(ProFeature* pFeature, ProError status, ProAppData app_data);
ProError displayError(InputFileHole* localHole, int iHoleTableCnt);
ProError CsysNameFindFilterAction(ProFeature* p_feature, ProAppData app_data);

ProError searchParameters(ProName wParameterName, ProMdl mdlCurrent, double* dParam);
ProError getParametersValue(ProParameter* param, ProError err, ProAppData app_data);
/*-----------------------------------------------------------------*\
	�}�N��
\*-----------------------------------------------------------------*/
#define Z_DATUM_NAME L"XZ_MAIN"			// Z���W�̊�ƂȂ�f�[�^�����ʂ̖��O

#define X_DATUM_NAME_0 L"XM"			// X���W�̊�ƂȂ�f�[�^�����ʂ̖��O
#define X_DATUM_NAME_1 L"FRAME_FRONT"	// X���W�̊�ƂȂ�f�[�^�����ʂ̖��O
#define X_DATUM_NAME_2 L"XRAP"			// X���W�̊�ƂȂ�f�[�^�����ʂ̖��O
#define X_DATUM_NAME_3 L"FRAME_END"		// X���W�̊�ƂȂ�f�[�^�����ʂ̖��O
#define X_DATUM_NAME_4 L"FAP"			// X���W�̊�ƂȂ�f�[�^�����ʂ̖��O

// �O���[�v��(L)
#define REF_HOLE_LEFT_BLIND		L"REF_HOLE_LEFT_BLIND"	// ��1�̃O���[�v��
#define HOLE_LEFT_BLIND			L"HOLE_LEFT_BLIND"		// ��2�ȍ~�̃O���[�v��
#define CSYS_DTM_LEFT_OUTER		L"CSYS_DTM_LEFT_OUTER"	// �f�[�^�����ʂ̃O���[�v��
// �O���[�v��(R)
#define REF_HOLE_RIGHT_BLIND		L"REF_HOLE_RIGHT_BLIND"	// ��1�̃O���[�v��
#define HOLE_RIGHT_BLIND			L"HOLE_RIGHT_BLIND"		// ��2�ȍ~�̃O���[�v��
#define CSYS_DTM_RIGHT_OUTER		L"CSYS_DTM_RIGHT_OUTER"	// �f�[�^�����ʂ̃O���[�v��
// �擾�p�����[�^
#define PARAMETER_FRAME_HEIGHT		L"FRAME_HEIGHT"	// �t���[���̍���
#define PARAMETER_THICKNESS			L"THICKNESS"	// �t���[���̌���

// ���̑�
#define BOTH_FRAME		0
#define LEFT_FRAME		1
#define RIGHT_FRAME		2

#define MAX_SURFACE		10

// �T�[�t�F�C�X �������Ɏg�p����ProAppData
typedef struct {
	ProSurface		surface;		// out ��������
	double			dYPoint;		// out Y���W�i�v�f���͓K���Ȓl�j
}SurfaceAppData;

// �t���[���̕\�ʂ̃T�[�t�F�C�X���
// �Ƃ肠����10�ʕ���p�ӂ������A10�Ƃ��������ɈӖ��͂Ȃ��B�����������g���b�N�f�[�^���͍ő�3�ʂ�����
typedef struct {
	double			dSurfZ[MAX_SURFACE * 2];	// out Z���W�i�[�p:����������t���[���̋��E���B�P�\�ʂɂ�2���W�i�[�B10�ʕ��p�ӁB
	ProSurface		surface[MAX_SURFACE];		// out �e�ʂ̃T�[�t�F�C�X���
	int				iCounter;					// out counter
}MainSurfaceInfo;


// �������ʊ֘A�̍\����
typedef struct {
	int				iLRFlag;			// in  1:L, 2:R
	double			dSurfY[MAX_SURFACE];	// in/out :MainSurfaceInfo�� Y���W�i�[�p/Back�𒲂ׂ�Ƃ��Ɏg�p

	MainSurfaceInfo strFrontInfo;
	MainSurfaceInfo strBackInfo;
}SurfaceInfoAppData;


// ProGroup��ProFeatur�ϊ�
typedef struct {
	ProGroup		group;		// in �����Ώ�
	ProMdlName		wName;		// in �����Ώۖ�
	ProFeature		feature;	// out ��������
}strGroupToFeature;

typedef struct {
	ProName wParameterName;		// in �����Ώ�
	double			dParam;		// Out ��������
}strSearchParameter;

static InputFileHoleTable* staticHoleTable;	// �R���t�B�O���[�V�����t�@�C����HoleTable���
static ProFeature* staticHoleGroupAll;		// �����̌��O���[�v��1�̃O���[�v�ɂ܂Ƃ߂�

static ProMdl staticMdlSideFrame;
/*====================================================================*\
FUNCTION : setHoles_HoleTableSection
PURPOSE  : Holes/HoleTable�Z�N�V����
	ProMdl* top_asm						(in) �������p�[�c���܂�Top�A�Z���u��
	ProMdl* mdlSideFrame				(in) �������p�[�c���܂�Top�A�Z���u��
	ProPath patName						(in) �������p�[�c�̊��
	InputFileHole* strHole				(in) �R���t�B�O���[�V�����t�@�C���̓��e(Holes)
	InputFileHoleTable* strHoleTable	(in) �R���t�B�O���[�V�����t�@�C���̓��e(Hole_Table)
	int iHoleSectionMaxRows				(in) �������ׂ��R���t�B�O���[�V�����t�@�C���̍s��(Holes)
	int iHoleTableSectionMaxRows		(in) �������ׂ��R���t�B�O���[�V�����t�@�C���̍s��(Hole_Table)
	int iHoletype						(in) 0:Hole , 1:innerlinerHole
*====================================================================*/
ProError  setHoles_HoleTableSection(ProMdl* top_asm, ProMdl* mdlSideFrame, ProPath patName, InputFileHole* strHole, InputFileHoleTable* strHoleTable, int iHoleSectionMaxRows, int iHoleTableSectionMaxRows, int iHoletype)
{
	ProError status = PRO_TK_NO_ERROR;
	int iResultL;
	int iResultR;
	int iResultB;
	wchar_t wSide[INPUTFILE_MAXLINE];           // �������������. L(��)/R(�E)/B(����)
	ProPath wRightPart;
	ProPath wLeftPart;

	// �O���[�o���ϐ��̏�����
	staticMdlSideFrame = *mdlSideFrame;

	// ���E�̌������p�[�c����ݒ�
	ProWstringCopy(patName, wLeftPart, PRO_VALUE_UNUSED);
	ProWstringConcatenate(L"_1", wLeftPart, PRO_VALUE_UNUSED);
	ProWstringCopy(patName, wRightPart, PRO_VALUE_UNUSED);
	ProWstringConcatenate(L"_2", wRightPart, PRO_VALUE_UNUSED);

	/*--------------------------------------------------------------------*\
		part�����猊��������p�[�c���̊m�F
	\*--------------------------------------------------------------------*/
	ProError status1 = PRO_TK_NO_ERROR;
	ProError status2 = PRO_TK_NO_ERROR;
	ProMdl mdlLeftPart = NULL;
	ProMdl mdlRightPart = NULL;

	status1 = ProMdlnameInit(wLeftPart, PRO_MDLFILE_PART, &mdlLeftPart);
	TRAIL_PRINT("%s(%d) : ProMdlnameInit = %s", __func__, __LINE__, getProErrorMessage(status1));

	if (status1 != PRO_TK_NO_ERROR) {
		// �ҏW����p�[�c��������܂���ł����B
		ProCharPath cPatName;
		ProWstringToString(cPatName, wLeftPart);
		LOG_PRINT("NOK : %s not found", cPatName);
	}

	status2 = ProMdlnameInit(wRightPart, PRO_MDLFILE_PART, &mdlRightPart);
	TRAIL_PRINT("%s(%d) : ProMdlnameInit = %s", __func__, __LINE__, getProErrorMessage(status2));

	if (status2 != PRO_TK_NO_ERROR) {
		// �ҏW����p�[�c��������܂���ł����B
		ProCharPath cPatName;
		ProWstringToString(cPatName, wRightPart);
		LOG_PRINT("NOK : %s not found", cPatName);
	}
	if (status1 != PRO_TK_NO_ERROR || status2 != PRO_TK_NO_ERROR) {
		return;
	}

	/*--------------------------------------------------------------------*\
		�������p�[�c�̃T�[�t�F�C�X�����擾
	\*--------------------------------------------------------------------*/
	// �������m��
	SurfaceInfoAppData* rightSurfInfoData;	// ����������t���[���̋��E��(�E)
	rightSurfInfoData = (SurfaceInfoAppData*)calloc(1, sizeof(SurfaceInfoAppData));
	if (!rightSurfInfoData) {
		// �������s��
		LOG_PRINT("NOK : Not enough memory");
		return PRO_TK_GENERAL_ERROR;
	}

	// �������m��
	SurfaceInfoAppData* leftSurfInfoData;	// ����������t���[���̋��E��(��)
	leftSurfInfoData = (SurfaceInfoAppData*)calloc(1, sizeof(SurfaceInfoAppData));
	if (!leftSurfInfoData) {
		// �������s��
		LOG_PRINT("NOK : Not enough memory");
		return PRO_TK_GENERAL_ERROR;
	}

	// �T�[�t�F�C�X���̎擾
	getSurfaceInfo(rightSurfInfoData, RIGHT_FRAME, mdlRightPart);
	getSurfaceInfo(leftSurfInfoData, LEFT_FRAME, mdlLeftPart);


	/*--------------------------------------------------------------------*\
		�����������̊J�n
	\*--------------------------------------------------------------------*/
	for (int iInputMdlCnt = 0; iInputMdlCnt < iHoleSectionMaxRows; iInputMdlCnt++) {
		// L,R,B�̐؂蕪��
		ProStringToWstring(wSide, strHole->cSide);
		status = ProWstringCompare(L"L", wSide, PRO_VALUE_UNUSED, &iResultL);
		status = ProWstringCompare(L"R", wSide, PRO_VALUE_UNUSED, &iResultR);
		status = ProWstringCompare(L"B", wSide, PRO_VALUE_UNUSED, &iResultB);

		if (iResultL == 0) {
			// L�̏���
			status = createHole(top_asm, mdlLeftPart, strHole, strHoleTable, LEFT_FRAME, iHoleTableSectionMaxRows, iHoletype, leftSurfInfoData);

		}
		else if(iResultR == 0) {
			// R�̏���
			status = createHole(top_asm, mdlRightPart, strHole, strHoleTable, RIGHT_FRAME, iHoleTableSectionMaxRows, iHoletype, rightSurfInfoData);

		}
		else if (iResultB == 0) {
			// B�̏���
			status = createHole(top_asm, mdlLeftPart, strHole, strHoleTable, BOTH_FRAME, iHoleTableSectionMaxRows, iHoletype, leftSurfInfoData);
			status = createHole(top_asm, mdlRightPart, strHole, strHoleTable, RIGHT_FRAME, iHoleTableSectionMaxRows, iHoletype, rightSurfInfoData);

		}
		else {
			// �z��O�̃G���[. �z��O�̌���������(L/R/B)�ł�
			LOG_PRINT("NOK : %s : HoleSide(L/R/B) is an abnormal value", strHole->cHoleGroupName);
		}
		strHole++;
	}

	free(rightSurfInfoData);
	free(leftSurfInfoData);

	return;
}

/*====================================================================*\
FUNCTION : makeHole
PURPOSE  : Holes/HoleTable�Z�N�V����
	ProMdl* top_asm						(in) Feature�Ώ�
	ProMdl mdlPart						(in) �������p�[�c��ProMdl
	InputFileHole* localHole			(in) �R���t�B�O���[�V�����t�@�C���̓��e
	InputFileHoleTable* localHoleTable	(in) �R���t�B�O���[�V�����t�@�C���̓��e
	int iLRFlag							(in) �������p�[�cRight/Left��� 1:Left, 2:Right 3:Both(Left)
	int iHoleTableSectionMaxRows		(in) �������ׂ��R���t�B�O���[�V�����t�@�C���̍s��(Hole_Table)
	int iHoletype						(in) 0:Hole , 1:innerlinerHole
	SurfaceInfoAppData* surfInfoData	(in) �������T�[�t�F�C�X���
*====================================================================*/
ProError  createHole(ProMdl* top_asm, ProMdl mdlPart, InputFileHole* localHole, InputFileHoleTable* localHoleTable, int iLRFlag ,int iHoleTableSectionMaxRows, int iHoletype, SurfaceInfoAppData* surfInfoData)
{
	ProError status = PRO_TK_NO_ERROR;
	ProFeature dtmPlnFeature_X;	// X���̃f�[�^������
	ProFeature dtmPlnFeature_Y;	// Y���̃f�[�^������
	ProFeature dtmPlnFeature_Z;	// Z���̃f�[�^������
	ProFeature dtmPlnFeature_YZ;	// YZ���̃f�[�^������
	ProFeature dtmPlnFeature_selSurf;	// �z�u��/�������ʂ̃f�[�^������
	ProFeature dtmAxisFeature2;	// ��2�ȍ~�̃f�[�^����
	ProFeature holeFeatureOne;	// ��1
	ProFeature holeFeature;	// ��2�ȍ~
	ProGroup group;
	ProFeature dtmPlnFeature_Z2;	// Z���̃f�[�^������

	int  n_sel;
	int iHoleTableCnt = 0;
	int iUnderUpperFlag = 0;
	ProBoolean bAxisFlag = PRO_B_FALSE;

	/*--------------------------------------------------------------------*\
		Left / Right �ɂ��O���[�v���̒�`
	\*--------------------------------------------------------------------*/
	ProMdlName wRefHoleGroupName;
	ProMdlName wHoleGroupName;
	ProMdlName wDatumGroupName;
	if (iLRFlag == LEFT_FRAME || iLRFlag == BOTH_FRAME) {
		// Left
		ProWstringCopy(REF_HOLE_LEFT_BLIND, wRefHoleGroupName, PRO_VALUE_UNUSED);
		ProWstringCopy(HOLE_LEFT_BLIND, wHoleGroupName, PRO_VALUE_UNUSED);
		ProWstringCopy(CSYS_DTM_LEFT_OUTER, wDatumGroupName, PRO_VALUE_UNUSED);

	}
	else if (iLRFlag == RIGHT_FRAME) {
		// Right 
		ProWstringCopy(REF_HOLE_RIGHT_BLIND, wRefHoleGroupName, PRO_VALUE_UNUSED);
		ProWstringCopy(HOLE_RIGHT_BLIND, wHoleGroupName, PRO_VALUE_UNUSED);
		ProWstringCopy(CSYS_DTM_RIGHT_OUTER, wDatumGroupName, PRO_VALUE_UNUSED);
	}
	else {
		// �z��O�ُ̈�.�����ɗ��邱�Ƃ͂��肦�Ȃ��̂ŃG���[���b�Z�[�W���o���Ȃ�
		
	}

	/*--------------------------------------------------------------------*\
		�������O���[�vHoleTable���擾����
	\*--------------------------------------------------------------------*/
	status = getHoleTableInfOfSameNum(localHole, localHoleTable, iHoleTableSectionMaxRows ,&iHoleTableCnt);
	if (status != PRO_TK_NO_ERROR) {
		// �G���[���b�Z�[�W��getHoleTableInfOfSameNum���ŋL��
		return status;
	}

	// �m�ۂ����������̐擪�A�h���X�i�[�p
	InputFileHoleTable* startHoleTable = staticHoleTable;

	/*--------------------------------------------------------------------*\
		�t���[���\�ʂ������ɕ�����邱�Ƃ����邽�߁A
		dXPoint �� iDatumType �ɂ���Ă��ꂼ��𔻒f����
	\*--------------------------------------------------------------------*/
	ProSurface		xDatumSurface;

	int iHoleFlag = atoi(staticHoleTable->cHoleFlag);

	if (iHoleFlag == 1 || iHoleFlag == 2) {
		// �T�[�t�F�X�̏����擾
		status = getXDatumSurfaceInfo(&surfInfoData->strFrontInfo, localHole, top_asm, &xDatumSurface);

	}
	else if (iHoleFlag == 3 || iHoleFlag == 4) {
		// �T�[�t�F�X�̏����擾�B���ʊ�Ō���������
		status = getXDatumSurfaceInfo(&surfInfoData->strBackInfo, localHole, top_asm, &xDatumSurface);
	}
	else {
		// �z��O�̃G���[. ���t���O(1�`4)���z��O�ł�
		LOG_PRINT("NOK : %s : HoleFlag(1�`4) is an abnormal value", staticHoleTable->cHoleFlag);

	}

	if (status != PRO_TK_NO_ERROR) {
		// ��ƂȂ�T�[�t�F�C�X��������Ȃ�����
		LOG_PRINT("NOK : %s %s : failed to create (errorcode:1)", localHole->cHoleGroupName, staticHoleTable->cHoleID);

		displayError(localHole, iHoleTableCnt);
		return status;
	}

	// ProModelitem�ɕϊ�����
	ProModelitem dtmplnConstrMdlItemZ;
	ProAsmcomppath compPath;
	int iSurfaceId;
	status = getAsmcompathIncludePart(*top_asm, mdlPart, &compPath);
	status = ProSurfaceIdGet(xDatumSurface, &iSurfaceId);
	status = ProModelitemInit(mdlPart, iSurfaceId, PRO_SURFACE, &dtmplnConstrMdlItemZ);
	TRAIL_PRINT("%s(%d) : ProModelitemInit = %s", __func__, __LINE__, getProErrorMessage(status));

	/*--------------------------------------------------------------------*\
		�t���[���\�ʂƕ��s�� �f�[�^�����ʂ̍쐬
	\*--------------------------------------------------------------------*/
	double dYPoint = atof(staticHoleTable->cYCord);

	if (iHoleFlag == 1 || iHoleFlag == 2) {
		dYPoint = dYPoint * -1;
	}

	if (iHoletype == INNERLINER_HOLE && iHoleFlag == 1) {
		double dThickness;
		searchParameters(PARAMETER_THICKNESS, staticMdlSideFrame, &dThickness);
		dYPoint = dYPoint + dThickness;
	}

	// �t���[���\�ʂƕ��s�ȃf�[�^�����ʂ��쐬
	status = createDatumPlaneZ(top_asm, mdlPart, dYPoint, dtmplnConstrMdlItemZ, &dtmPlnFeature_Z);
	if (status != PRO_TK_NO_ERROR) {
		// �t���[���\�ʂƕ��s�ȃf�[�^�����ʂ̍쐬�Ɏ��s
		LOG_PRINT("NOK : %s %s : failed to create (errorcode:6)", localHole->cHoleGroupName, staticHoleTable->cHoleID);
		displayError(localHole, iHoleTableCnt);

		return status;
	}

	/*--------------------------------------------------------------------*\
		�t���[�������̃T�[�t�F�X�̏����擾���AProModelitem���擾���� 
	\*--------------------------------------------------------------------*/
	
	ProSelection selSurfYLower;
	SurfaceAppData appdataSurface;

	// �e�l�̏����� double�^�̍ő�l�ŏ���������
	appdataSurface.dYPoint = DBL_MAX;

	// ��ƂȂ�T�[�t�F�C�X���擾
	status = ProSolidSurfaceVisit(ProMdlToSolid(mdlPart),
		(ProSurfaceVisitAction)getLowerSurfaceFromYpointAction,
		(ProSurfaceFilterAction)NULL,
		(ProAppData)&appdataSurface);

	if (status != PRO_TK_NO_ERROR) {
		// ��ƂȂ�T�[�t�F�C�X��������Ȃ�����
		LOG_PRINT("NOK : %s %s : failed to create (errorcode:3)", localHole->cHoleGroupName, staticHoleTable->cHoleID);

		displayError(localHole, iHoleTableCnt);
		return (PRO_TK_E_NOT_FOUND);
	}

	// ProModelitem�ɕϊ�����
	ProModelitem dtmplnConstrMdlItemYLower;
	iSurfaceId = 0;
	status = ProSurfaceIdGet(appdataSurface.surface, &iSurfaceId);
	status = ProModelitemInit(mdlPart, iSurfaceId, PRO_SURFACE, &dtmplnConstrMdlItemYLower);
	status = ProSelectionAlloc(&compPath, &dtmplnConstrMdlItemYLower, &selSurfYLower);
	TRAIL_PRINT("%s(%d) : ProSelectionAlloc = %s", __func__, __LINE__, getProErrorMessage(status));

	/*--------------------------------------------------------------------*\
		�t���[���㕔�̃T�[�t�F�X�̏����擾���AProModelitem���擾����
	\*--------------------------------------------------------------------*/

	ProSelection selSurfYUpper;

	// �e�l�̏����� double�^�̍ŏ��l�ŏ���������
	appdataSurface.dYPoint = DBL_MIN;

	// ��ƂȂ�T�[�t�F�C�X���擾
	status = ProSolidSurfaceVisit(ProMdlToSolid(mdlPart),
		(ProSurfaceVisitAction)getUpperSurfaceFromYpointAction,
		(ProSurfaceFilterAction)NULL,
		(ProAppData)&appdataSurface);

	if (status != PRO_TK_NO_ERROR) {
		// ��ƂȂ�T�[�t�F�C�X��������Ȃ�����
		LOG_PRINT("NOK : %s %s : failed to create (errorcode:4)", localHole->cHoleGroupName, staticHoleTable->cHoleID);

		displayError(localHole, iHoleTableCnt);
		return (PRO_TK_E_NOT_FOUND);
	}

	// ProModelitem�ɕϊ�����
	ProModelitem dtmplnConstrMdlItemYUpper;
	iSurfaceId = 0;
	status = ProSurfaceIdGet(appdataSurface.surface, &iSurfaceId);
	status = ProModelitemInit(mdlPart, iSurfaceId, PRO_SURFACE, &dtmplnConstrMdlItemYUpper);
	status = ProSelectionAlloc(&compPath, &dtmplnConstrMdlItemYUpper, &selSurfYUpper);
	TRAIL_PRINT("%s(%d) : ProSelectionAlloc = %s", __func__, __LINE__, getProErrorMessage(status));

	/*--------------------------------------------------------------------*\
		X���̃f�[�^�����ʂ��쐬
		XM��FRAME_END�ƕ��s�ȃf�[�^������
	\*--------------------------------------------------------------------*/
	int iDatumType = 0;
	iDatumType = atoi(localHole->cDatumType);
	double dXPoint = 0;
	dXPoint = atof(localHole->cXCord);

	status = createDatumPlaneX(top_asm, dXPoint, &dtmPlnFeature_X, iDatumType);
	TRAIL_PRINT("%s(%d) : createDatumPlaneX = %s", __func__, __LINE__, getProErrorMessage(status));

	if (status != PRO_TK_NO_ERROR) {
		// �������Ɏ��s
		LOG_PRINT("NOK : %s %s : failed to create (errorcode:5)", localHole->cHoleGroupName, staticHoleTable->cHoleID);
		displayError(localHole, iHoleTableCnt);

		return status;
	}

	/*--------------------------------------------------------------------*\
		���̑傫�����擾
	\*--------------------------------------------------------------------*/
	double dDiameter = 0;
	if (iHoletype == HOLE) {
		// sideFrame�̌������̏ꍇ
		dDiameter = atof(staticHoleTable->cFrameDiameter);
	}
	else {
		// innerliner�̌������̏ꍇ
		dDiameter = atof(staticHoleTable->cInnerLineDiameter);
	}

	/*--------------------------------------------------------------------*\
		YZ���̃f�[�^�����ʂ��쐻

		Hole_table�ɂČ�1��Z���W=0�̎��A
		1�O���[�v�����ׂĂ̌����t���[���㕔 or �����ƂȂ�B
		(2���ڂ̃f�[�^�����ʂ̌������ς��)
	\*--------------------------------------------------------------------*/
	// �����𔽓]������
	double dZPoint = atof(staticHoleTable->cZCord);
	// Y���̃f�[�^�����ʂ̍쐬
	status = createDatumPlaneY(top_asm, mdlPart, (dZPoint * -1), &dtmPlnFeature_Y);
	if (status != PRO_TK_NO_ERROR) {
		// Y���̃f�[�^�����ʂ̍쐬�Ɏ��s
		LOG_PRINT("NOK : %s %s : failed to create (errorcode:7)", localHole->cHoleGroupName, staticHoleTable->cHoleID);
		displayError(localHole, iHoleTableCnt);
		return status;
	}

	ProSelection selSurf;

	double dHeightParam = 0;
	searchParameters(PARAMETER_FRAME_HEIGHT, *top_asm, &dHeightParam);

	if (dZPoint == 0) {
		// �t���[���̏㕔or�����Ɍ���������
		iUnderUpperFlag = 1;

		if (iHoleFlag == 1 || iHoleFlag == 4) {
			// �t���[������
			selSurf = selSurfYLower;
		}
		else if (iHoleFlag == 2 || iHoleFlag == 3) {
			// �t���[���㕔
			selSurf = selSurfYUpper;
		}
		dtmPlnFeature_YZ = dtmPlnFeature_Z;

		// �������\�ʁiselSurfYLower/selSurfYUpper�j���g�p
		status = createHoleAroundPlane(*top_asm, mdlPart, dtmPlnFeature_X, dtmPlnFeature_YZ, selSurf, dtmPlnFeature_selSurf, 0, 0, dDiameter, &holeFeatureOne);
	}
	else if (dZPoint >= dHeightParam) {
		// �t���[���̏㕔�Ɍ���������
		iUnderUpperFlag = 1;

		selSurf = selSurfYUpper;
		dtmPlnFeature_YZ = dtmPlnFeature_Z;
		// �������\�ʁiselSurfYLower/selSurfYUpper�j���g�p
		status = createHoleAroundPlane(*top_asm, mdlPart, dtmPlnFeature_X, dtmPlnFeature_YZ, selSurf, dtmPlnFeature_selSurf, 0, 0, dDiameter, &holeFeatureOne);
	}
	else {
		//selSurf = selSurfZ;
		dtmPlnFeature_YZ = dtmPlnFeature_Y;

		dtmPlnFeature_selSurf = dtmPlnFeature_Z;

		// �������\�ʁiselSurfZ�j�̓��W���[���ŏ�����\��������̂ŁA�g�p�����A�f�[�^�����ʂ���Ƃ���
		status = createHoleAroundPlane(*top_asm, mdlPart, dtmPlnFeature_X, dtmPlnFeature_YZ, NULL, dtmPlnFeature_selSurf, 0, 0, dDiameter, &holeFeatureOne);
	}

	/*--------------------------------------------------------------------*\
		��1�ƃf�[�^������1�̃O���[�v�ɂ܂Ƃ߂�
	\*--------------------------------------------------------------------*/
	n_sel = 1;
	ProFeature* groupHoleOne = (ProFeature*)calloc(n_sel, sizeof(ProFeature));
	if (!groupHoleOne) {
		// �������s��
		LOG_PRINT("NOK : Not enough memory");
		return PRO_TK_GENERAL_ERROR;
	}
	groupHoleOne[0] = holeFeatureOne;
	status = createGroupFunction((ProSolid)mdlPart, wRefHoleGroupName, groupHoleOne, n_sel, &group);
	free(groupHoleOne);

	// ���O���b�Z�[�W
	if (status == PRO_TK_NO_ERROR) {
		LOG_PRINT("OK  : %s %s", localHole->cHoleGroupName, staticHoleTable->cHoleID);
	}
	else {
		LOG_PRINT("NOK : %s %s : failed to create (errorcode:8)", localHole->cHoleGroupName, staticHoleTable->cHoleID);
		displayError(localHole, iHoleTableCnt);
	}

	// ���O���[�v�̃O���[�v���̏���
	strGroupToFeature appData;
	appData.group = group;
	ProWstringCopy(wRefHoleGroupName, appData.wName, PRO_VALUE_UNUSED);
	status = ProSolidFeatVisit((ProSolid)mdlPart, convertGroupToFeature, NULL, (ProAppData)&appData);
	staticHoleGroupAll[0] = appData.feature;

	/*--------------------------------------------------------------------*\
		��2�ȍ~�̍쐬
	\*--------------------------------------------------------------------*/
	for (int iHoleCnt = 1; iHoleCnt < iHoleTableCnt; iHoleCnt++) {
		staticHoleTable++;
		double distXorg = atof(staticHoleTable->cXCord);
		double distX = atof(staticHoleTable->cXCord);
		double distYorg = atof(staticHoleTable->cYCord);
		double distY = atof(staticHoleTable->cYCord);
		double distZ = atof(staticHoleTable->cZCord);
		double dist = 0;

		// ���̒��a���擾
		if (iHoletype == HOLE) {
			// SideFrame�̏ꍇ
			dDiameter = atof(staticHoleTable->cFrameDiameter);
		}
		else {
			// innerliner�̏ꍇ
			dDiameter = atof(staticHoleTable->cInnerLineDiameter);
		}

		if (iLRFlag == LEFT_FRAME || iLRFlag == BOTH_FRAME) {
			distX = distX;
		}	else {
			distX = distX * -1;
		}

		if (iUnderUpperFlag == 1) {
			// �S UNDER or ��1��UPPER
			dist = distY;

			if (iHoleFlag == 3 || iHoleFlag == 4) {
				// �t���[������
				distX = distX * -1;

			}

			status = createHoleBaseHoleOne(*top_asm, mdlPart, holeFeatureOne, dtmPlnFeature_YZ, selSurf, dtmPlnFeature_Z, distX, dist, dDiameter, &holeFeature);
		}else {
			// ��1���\�ʕ���
			if (distY > 0 && distZ < 0) {
				// ��2�ȍ~������
				if (iLRFlag == LEFT_FRAME || iLRFlag == BOTH_FRAME) {
					distX = distX * -1;
				}
				else {
					distX = distX;
				}

				// ����
				dist = distY * -1;
				selSurf = selSurfYLower;
				dtmPlnFeature_YZ = dtmPlnFeature_Y;

				status = createHoleBaseHoleOneOther(*top_asm, mdlPart, holeFeatureOne, dtmPlnFeature_Z, selSurf, distX, dist, dDiameter, &holeFeature);
			}
			else {
				// ��2�ȍ~���\��

				/*--------------------------------------------------------------------*\
					��2�ȍ~�̖ʂ��ς�邱�Ƃ����邽�߁AX���W�Ŋm�F����
					(�\�ʂ͕\�ʂ����A�΂ߖʂɍ����|���邱�Ƃ�����)
				\*--------------------------------------------------------------------*/
				ProSurface		xDatumSurface2;
				double dXHolePoint = dXPoint + distXorg;
				status = getXDatumSurfaceInfoForHole2(&surfInfoData->strFrontInfo, localHole, top_asm, dXHolePoint, &xDatumSurface2);

				// ProModelitem�ɕϊ�����
				ProModelitem dtmplnConstrMdlItemZ2;
				ProAsmcomppath compPath2;
				int iSurfaceId2;
				status = getAsmcompathIncludePart(*top_asm, mdlPart, &compPath2);
				status = ProSurfaceIdGet(xDatumSurface2, &iSurfaceId2);
				status = ProModelitemInit(mdlPart, iSurfaceId2, PRO_SURFACE, &dtmplnConstrMdlItemZ2);
				TRAIL_PRINT("%s(%d) : ProModelitemInit = %s", __func__, __LINE__, getProErrorMessage(status));


				if (dtmplnConstrMdlItemZ.id != dtmplnConstrMdlItemZ2.id) {
					bAxisFlag = PRO_B_TRUE;
					//�\�ʂ̒��ł��ʂ��ς��(�΂ߖʂƂȂ�)�ꍇ
					dtmPlnFeature_YZ = dtmPlnFeature_Y;
					/*--------------------------------------------------------------------*\
						�t���[���\�ʂƕ��s�� �f�[�^�����ʂ̍쐬
					\*--------------------------------------------------------------------*/
					double dYPoint_2 = atof(staticHoleTable->cYCord);
					

					if (iHoleFlag == 1 || iHoleFlag == 2) {
						dYPoint_2 = dYPoint_2 * -1;
					}

					if (iHoletype == INNERLINER_HOLE && iHoleFlag == 1) {
						double dThickness;
						searchParameters(PARAMETER_THICKNESS, staticMdlSideFrame, &dThickness);
						dYPoint_2 = dYPoint_2 + dThickness;
					}

					// �t���[���\�ʂƕ��s�ȃf�[�^�����ʂ��쐬
					status = createDatumPlaneZ(top_asm, mdlPart, dYPoint_2, dtmplnConstrMdlItemZ2, &dtmPlnFeature_Z2);
					if (status != PRO_TK_NO_ERROR) {
						// �t���[���\�ʂƕ��s�ȃf�[�^�����ʂ̍쐬�Ɏ��s
						LOG_PRINT("NOK : %s %s : failed to create (errorcode:6)", localHole->cHoleGroupName, staticHoleTable->cHoleID);
						displayError(localHole, iHoleTableCnt);

						return status;
					}

					/*--------------------------------------------------------------------*\
						�{�������J���ׂ��ꏊ�̈ʒu�ɁA���ł͂Ȃ��f�[�^�������쐬����B
					\*--------------------------------------------------------------------*/
					if (iLRFlag == LEFT_FRAME || iLRFlag == BOTH_FRAME) {
						distX = distX ;
						dist = distZ;
					}
					else {
						distX = distX * -1;
						dist = distZ * -1;
					}
					createDatumAxis(*top_asm, mdlPart, dtmplnConstrMdlItemZ2, dtmPlnFeature_X, dtmPlnFeature_YZ, distX, dist, &dtmAxisFeature2);
					 
					/*--------------------------------------------------------------------*\
						�f�[�^��������Ɍ���������
					\*--------------------------------------------------------------------*/
					status = createHoleBaseAxis(*top_asm, mdlPart, dtmAxisFeature2, dtmPlnFeature_Z2, 0, 0, dDiameter, &holeFeature);


				}else{
					// ��2�ȍ~���\��
					distX = distX * -1;

					dist = distZ;
					dtmPlnFeature_YZ = dtmPlnFeature_Y;

					status = createHoleBaseHoleOne(*top_asm, mdlPart, holeFeatureOne, dtmPlnFeature_YZ, NULL, dtmPlnFeature_Z, distX, dist, dDiameter, &holeFeature);

				}
			}
		}
		

		// ��2�ȍ~�̃O���[�v���B1�O���[�v��1���t�B�[�`��
		n_sel = 1;
		ProFeature* groupHoleOther = (ProFeature*)calloc(n_sel, sizeof(ProFeature));
		if (!groupHoleOther) {
			// �������s��
			LOG_PRINT("NOK : Not enough memory");
			return PRO_TK_GENERAL_ERROR;
		}
		groupHoleOther[0] = holeFeature;
		status = createGroupFunction((ProSolid)mdlPart, wHoleGroupName, groupHoleOther, n_sel, &group);
		free(groupHoleOther);

		// ���O���b�Z�[�W
		if (status == PRO_TK_NO_ERROR) {
			LOG_PRINT("OK  : %s %s", localHole->cHoleGroupName, staticHoleTable->cHoleID);
		}
		else {
			LOG_PRINT("NOK : %s %s : failed to create", localHole->cHoleGroupName, staticHoleTable->cHoleID);
		}

		// ���O���[�v�̃O���[�v���̏���
		appData.group = group;
		ProWstringCopy(wHoleGroupName, appData.wName, PRO_VALUE_UNUSED);
		status = ProSolidFeatVisit((ProSolid)mdlPart, convertGroupToFeature, NULL, (ProAppData)&appData);
		staticHoleGroupAll[iHoleCnt] = appData.feature;

	}

	/*--------------------------------------------------------------------*\
		*MODULES �Z�N�V���� ���(H)�ŕK�v�ƂȂ�̂ŁA���P�̏ꏊ��CSYS���쐬����
	\*--------------------------------------------------------------------*/
	ProCsys csys;
	ProFeature csysFeature;

	// �O���[�v���̎擾
	ProMdlName groupName;
	ProMdlName wCsysName;
	ProMdlName wCsysNameBk;
	int iGroupNameCnt = 0;
	ProMdlName wGroupNameCnt;

	ProStringToWstring(groupName, localHole->cHoleGroupName);
	ProWstringCopy(groupName, wCsysName, PRO_VALUE_UNUSED);
	ProWstringConcatenate(L"_", wCsysName, PRO_VALUE_UNUSED);
	if (iLRFlag == BOTH_FRAME) {
		ProWstringConcatenate(L"B", wCsysName, PRO_VALUE_UNUSED);
	}else if (iLRFlag == LEFT_FRAME) {
		ProWstringConcatenate(L"L", wCsysName, PRO_VALUE_UNUSED);
	}
	else {
		ProWstringConcatenate(L"R", wCsysName, PRO_VALUE_UNUSED);
	}

	// ���O��������Ƃ��ɐ������l�ɂȂ�Ȃ����߁ACSYS�����������Ĕ��Ȃ��悤�ɂ���
	UserCsysAppData	serchCsysName;

	ProWstringCopy(wCsysName, wCsysNameBk, PRO_VALUE_UNUSED);
	
	while (TRUE) {
		ProWstringCopy(wCsysNameBk, serchCsysName.csys_name, PRO_VALUE_UNUSED);
		// ����������
		ProWstringCopy(wCsysName, wCsysNameBk, PRO_VALUE_UNUSED);

		status = ProSolidFeatVisit((ProSolid)*top_asm,
			NULL,
			CsysNameFindFilterAction,
			(ProAppData)&serchCsysName);

		if (status != PRO_TK_E_NOT_FOUND) {
			// ����CSYS������ꍇ��_(�A��)������
			iGroupNameCnt++;
			ProMdlName wGroupNameCnt;
			_itow_s(iGroupNameCnt, wGroupNameCnt, sizeof(wGroupNameCnt),10);//�ϊ��p�֐�,10�i���ŕϊ�

			ProWstringConcatenate(L"_", wCsysNameBk, PRO_VALUE_UNUSED);
			ProWstringConcatenate(wGroupNameCnt, wCsysNameBk, PRO_VALUE_UNUSED);
		}
		else {
			break;
		}
	}
	ProWstringCopy(serchCsysName.csys_name, wCsysName, PRO_VALUE_UNUSED);


	if (iLRFlag == BOTH_FRAME) {
		// Top�p�[�c�̃f�[�^������(Z_DATUM_NAME)�̃t�B�[�`��������
		DatumAppData	z_datum;
		z_datum.iFindCnt = 0;
		ProWstringCopy(Z_DATUM_NAME, z_datum.name, PRO_VALUE_UNUSED);
		ProSolidFeatVisit((ProSolid)*top_asm, getFeatureIdAction, NULL, (ProAppData)&z_datum);

		if (z_datum.iFindCnt == 0) {
			// �f�[�^������(Z_DATUM_NAME)�̃t�B�[�`����������Ȃ�����
			LOG_PRINT("NOK : %s : XZ_MAIN Datum Plane does not exist", localHole->cHoleGroupName, staticHoleTable->cHoleID);
			return (PRO_TK_E_NOT_FOUND);
		}
		else {
			// B�̏ꍇ��Z��XZ_MAIN�f�[�^�����ʂɂȂ�
			status = createCsysHoleOne(top_asm, z_datum.feature, dtmPlnFeature_X, dtmPlnFeature_Y, wCsysName, iLRFlag, &csysFeature);
		}
	}
	else {
		// L��R�̏ꍇ��Z������f�[�^�����ʂɂȂ�
		status = createCsysHoleOne(top_asm, dtmPlnFeature_Z, dtmPlnFeature_X, dtmPlnFeature_Y, wCsysName, iLRFlag, &csysFeature);

	}

	/*--------------------------------------------------------------------*\
		�f�[�^������3����1�̃O���[�v�ɂ܂Ƃ߂�
	\*--------------------------------------------------------------------*/
	if (bAxisFlag) {
		// �f�[�^���������쐬���������O���[�v�ɂ܂Ƃ߂�
		n_sel = 6;
		ProFeature* groupFeat = (ProFeature*)calloc(n_sel, sizeof(ProFeature));
		if (!groupFeat) {
			// �������s��
			LOG_PRINT("NOK : Not enough memory");
			return PRO_TK_GENERAL_ERROR;
		}
		groupFeat[0] = dtmPlnFeature_X;
		groupFeat[1] = dtmPlnFeature_Y;
		groupFeat[2] = dtmPlnFeature_Z;
		groupFeat[3] = csysFeature;
		groupFeat[4] = dtmAxisFeature2;
		groupFeat[5] = dtmPlnFeature_Z2;

		status = createGroupFunction((ProSolid)*top_asm, wDatumGroupName, groupFeat, n_sel, &group);
		free(groupFeat);
	}
	else {
		// �W��
		n_sel = 4;
		ProFeature* groupFeat = (ProFeature*)calloc(n_sel, sizeof(ProFeature));
		if (!groupFeat) {
			// �������s��
			LOG_PRINT("NOK : Not enough memory");
			return PRO_TK_GENERAL_ERROR;
		}
		groupFeat[0] = dtmPlnFeature_X;
		groupFeat[1] = dtmPlnFeature_Y;
		groupFeat[2] = dtmPlnFeature_Z;
		groupFeat[3] = csysFeature;
		status = createGroupFunction((ProSolid)*top_asm, wDatumGroupName, groupFeat, n_sel, &group);
		free(groupFeat);

	}

	/*--------------------------------------------------------------------*\
		�����̌��O���[�v��1�̃O���[�v�ɂ܂Ƃ߂�
	\*--------------------------------------------------------------------*/
	status = createGroupFunction((ProSolid)staticHoleGroupAll[0].owner, groupName, staticHoleGroupAll, iHoleTableCnt, &group);
	free(staticHoleGroupAll);

	/*--------------------------------------------------------------------*\
		�������J��
	\*--------------------------------------------------------------------*/
	free(startHoleTable);

	return PRO_TK_NO_ERROR;
}

/*====================================================================*\
	FUNCTION :	errorHole()
	PURPOSE  :	��1�̃G���[�ɔ����A��2�ȍ~�̃G���[�\�����s��
	ProSolid solid,			in
	int feature_num,		in
	ProFeature* p_feature	out

\*====================================================================*/
ProError displayError(InputFileHole* localHole, int iHoleTableCnt) {

	for (int iHoleCnt = 1; iHoleCnt < iHoleTableCnt; iHoleCnt++) {
		staticHoleTable++;

		LOG_PRINT("NOK : %s %s : failed to create hole1", localHole->cHoleGroupName, staticHoleTable->cHoleID);
	}
}

/*====================================================================*\
	FUNCTION :	ProUtilSortedIntArrayObjectAdd
	PURPOSE  :	�\�[�g���ꂽint�̔z���int�l��ǉ�/ TestFeats.c �Q�l�@(���󖢎g�p)
\*====================================================================*/
ProError ProUtilSortedIntArrayObjectAdd(
	int** p_array,
	int new_value)
{
	ProError status;
	int i, size;

	status = ProArraySizeGet((ProArray)p_array[0], &size);

	for (i = 0; i<size && new_value>p_array[0][i]; i++) {
		if (i >= size || new_value < p_array[0][i])
		{
			status = ProArrayObjectAdd((ProArray*)p_array, i, 1, &new_value);
		}
	}

	return (status);
}

/*====================================================================*\
	FUNCTION :	ProUtilFeatByNumberInit()
	PURPOSE  :	�A�N�e�B�uFeture�݂̂�p_feature�ɐݒ肷��/ TestFeats.c �Q�l
	ProSolid solid,			in		
	int feature_num,		in
	ProFeature* p_feature	out

\*====================================================================*/
ProError ProUtilFeatByNumberInit(
	ProSolid solid,
	int feature_num,
	ProFeature* p_feature)
{
	ProError err, status = PRO_TK_GENERAL_ERROR;
	int* p_feat_id_array, n_features;
	ProFeatStatus* p_status_array;
	int feature_count;
	int actual_feat_number = 0;
	ProBoolean feature_found = PRO_B_FALSE;


	/* ID�z��ƃG���[�z��̊��� */
	err = ProArrayAlloc(0, sizeof(int), 1, (ProArray*)&p_feat_id_array);
	if (err != PRO_TK_NO_ERROR) {
		return (PRO_TK_GENERAL_ERROR);
	}

	err = ProArrayAlloc(0, sizeof(ProFeatStatus), 1, (ProArray*)&p_status_array);
	if (err != PRO_TK_NO_ERROR) {
		return (PRO_TK_GENERAL_ERROR);
	}

	/* �w�肳�ꂽ�\���b�h�̃t�B�[�`���ƃt�B�[�`���̃X�e�[�^�X���X�g���擾 */
	err = ProSolidFeatstatusGet(solid, &p_feat_id_array, &p_status_array, &n_features);

	/* feature_num--; */
	if (err == PRO_TK_NO_ERROR && (n_features > (feature_num - 1)))
	{
		for (feature_count = 0; feature_count < n_features; feature_count++)
		{
			if (p_status_array[feature_count] == PRO_FEAT_ACTIVE || p_status_array[feature_count] == PRO_FEAT_SIMP_REP_SUPPRESSED) {
					actual_feat_number++;
			}
			if (actual_feat_number == feature_num)
			{
				feature_found = PRO_B_TRUE;
				break;
			}
		}

		if (feature_found)
		{
			status = ProFeatureInit(solid, p_feat_id_array[feature_count], p_feature);

			err = ProArrayFree((ProArray*)&p_feat_id_array);
			err = ProArrayFree((ProArray*)&p_status_array);
			return (status);
			/* ����ȌĂяo�� */
		}
		else
		{
			return PRO_TK_E_NOT_FOUND;

			/* ���̏ꍇ�Ɏ��{����
			1.feature_number�̓��͒l�́A���f���Ŏg�p�\�ȃA�N�e�B�u�ȋ@�\�̐������傫�����A�ő�ID����������
			*/
		}
	}
	else
	{
		return PRO_TK_BAD_CONTEXT;
		/* ���̏ꍇ�Ɏ��{����
		1.ProSolidFeatstatusGet�ւ̌Ăяo�������s
		2.�@�\�ԍ��̓��͒l�����W���ꂽID�̐������傫��
		*/
	}


}

/*====================================================================*\
FUNCTION : createGroupFunction
PURPOSE  : �O���[�v �쐬 / TestFeats.c (ProTestLocalGroupCreate)�Q�l (����m�FOK)
ProSolid* p_solid	in	�O���[�v�Ώۃt�B�[�`����������\���b�h
\*====================================================================*/
ProError  createGroupFunction(ProSolid p_solid, ProName wGroupName, ProFeature featModelitem[], int iSel, ProGroup* group)
{
	ProError status;
	int* feat_arr = NULL,  i, *p_feat_id_array = NULL, feat_num;
	ProModelitem modelitem;
	
	/* Sort features array by number */
	ProArrayAlloc(0, sizeof(int), 1, (ProArray*)&feat_arr);
	for (i = 0; i < iSel; i++)
	{
		// �I�������t�B�[�`���̃t�B�[�`���ԍ����擾
		status = ProFeatureNumberGet((ProFeature*)&featModelitem[i], &feat_num);
		if (status != PRO_TK_NO_ERROR) {
			//continue;
			return status;
		}

		// �\�[�g�����ɃI�u�W�F�N�g�ǉ�
		status = ProArrayObjectAdd((ProArray*)&feat_arr, PRO_VALUE_UNUSED, 1, &feat_num);
		if (status != PRO_TK_NO_ERROR) {
			//continue;
			return status;
		}
	}

	status = ProArraySizeGet((ProArray)feat_arr, &iSel);

	status = ProArrayAlloc(0, sizeof(int), 1, (ProArray*)&p_feat_id_array);
	if (status != PRO_TK_NO_ERROR) {
		//break;
		return status;

	}

	for (i = 0; i < iSel; i++)
	{
		// �A�N�e�B�u�t�B�[�`���݂̂�modelitem�ɐݒ�
		status = ProUtilFeatByNumberInit(p_solid, feat_arr[i], (ProFeature*)&modelitem);

		ProArrayObjectAdd((ProArray*)&p_feat_id_array, PRO_VALUE_UNUSED, 1, &modelitem.id);
	}

	// �O���[�v�̍쐬
	status = ProLocalGroupCreate((ProSolid)modelitem.owner, p_feat_id_array, iSel, wGroupName, group);

	/*-----------------------------------------------------------------*\
		�������̊J��
	\*-----------------------------------------------------------------*/
	status = ProArrayFree((ProArray*)&feat_arr);
	status = ProArrayFree((ProArray*)&p_feat_id_array);

	return PRO_TK_NO_ERROR;
}
/*====================================================================*\
FUNCTION : createCsysHoleOne
PURPOSE  : ��1��CSYS�̍쐬
 *curMdl					(in) �f�[�^�����ʂ������i/�A�Z���u��
 ProFeature dtmPlnZFeature  (in) ��1�̃f�[�^������Z
 ProFeature dtmPlnXFeature  (in) ��1�̃f�[�^������X
 ProFeature dtmPlnYFeature  (in) ��1�̃f�[�^������Y
 ProMdlName csysName		(in) csys��
 int		iLRFlag			(in) Both,Left,Right
 ProFeature* csysFeature	(out) �쐬����CSYS�̃n���h��
 ���l
 ��1�������邽�߂ɍ쐬�����R���̃f�[�^�����ʂ��g�p���āACSYS���쐬����B
 CSYS�� *MODULE �Z�N�V�����̌��(H)�̎��Ɏg�p����
\*====================================================================*/
ProError  createCsysHoleOne(ProMdl* curMdl, ProFeature dtmPlnZFeature, ProFeature dtmPlnXFeature, ProFeature dtmPlnYFeature, ProMdlName csysName, int iLRFlag ,ProFeature* csysFeature)
{
	ProError status = PRO_TK_NO_ERROR;

	ProElement featElemTree;
	status = ProElementAlloc(PRO_E_FEATURE_TREE, &featElemTree);

	// �t�B�[�`���^�C�v (CSYS) PRO_E_FEATURE_TYPE
	ProElement featureTypeElem;
	status = ProElementAlloc(PRO_E_FEATURE_TYPE, &featureTypeElem);
	status = ProElementIntegerSet(featureTypeElem, PRO_FEAT_CSYS);
	status = ProElemtreeElementAdd(featElemTree, NULL, featureTypeElem);

	// �t�B�[�`���� �̐ݒ� PRO_E_STD_FEATURE_NAME
	ProElement featureNameElem;
	status = ProElementAlloc(PRO_E_STD_FEATURE_NAME, &featureNameElem);
	status = ProElementWstringSet(featureNameElem, csysName);
	status = ProElemtreeElementAdd(featElemTree, NULL, featureNameElem);

	//PRO_E_CSYS_ORIGIN_CONSTRS
	ProElement pro_e_csys_origin_constrs;
	status = ProElementAlloc(PRO_E_CSYS_ORIGIN_CONSTRS, &pro_e_csys_origin_constrs);
	status = ProElemtreeElementAdd(featElemTree, NULL, pro_e_csys_origin_constrs);

	/*--------------------------------------------------------------------*\
		�f�[�^������Z
	\*--------------------------------------------------------------------*/
	//PRO_E_CSYS_ORIGIN_CONSTRS
	//  |--PRO_E_CSYS_ORIGIN_CONSTR
	ProElement pro_e_csys_origin_constr_z;
	status = ProElementAlloc(PRO_E_CSYS_ORIGIN_CONSTR, &pro_e_csys_origin_constr_z);
	status = ProElemtreeElementAdd(pro_e_csys_origin_constrs, NULL, pro_e_csys_origin_constr_z);

	//PRO_E_CSYS_ORIGIN_CONSTRS
	//  |--PRO_E_CSYS_ORIGIN_CONSTR
	//    |--PRO_E_CSYS_ORIGIN_CONSTR_REF�@�I���W�i���Q��
	ProValue value;
	ProValueData value_data;
	ProElement pro_e_csys_origin_constr_ref_z;
	ProModelitem mdlitemZ;
	ProSelection pSelZ;

	ProFeatureGeomitemVisit(&dtmPlnZFeature, PRO_SURFACE, UsrPointAddAction, NULL, (ProAppData)&mdlitemZ);
	status = ProSelectionAlloc(NULL, &mdlitemZ, &pSelZ);
	status = ProElementAlloc(PRO_E_CSYS_ORIGIN_CONSTR_REF, &pro_e_csys_origin_constr_ref_z);

	value_data.type = PRO_VALUE_TYPE_SELECTION;
	value_data.v.r = pSelZ;

	status = ProValueAlloc(&value);
	status = ProValueDataSet(value, &value_data);
	status = ProElementValueSet(pro_e_csys_origin_constr_ref_z, value);
	status = ProElemtreeElementAdd(pro_e_csys_origin_constr_z, NULL, pro_e_csys_origin_constr_ref_z);

	ProReference refPlnZ;
	status = ProSelectionToReference(pSelZ, &refPlnZ);

	/*--------------------------------------------------------------------*\
		�f�[�^������X
	\*--------------------------------------------------------------------*/
	//PRO_E_CSYS_ORIGIN_CONSTRS
	//  |--PRO_E_CSYS_ORIGIN_CONSTR

	ProElement pro_e_csys_origin_constr_x;
	status = ProElementAlloc(PRO_E_CSYS_ORIGIN_CONSTR, &pro_e_csys_origin_constr_x);
	status = ProElemtreeElementAdd(pro_e_csys_origin_constrs, NULL, pro_e_csys_origin_constr_x);

	//PRO_E_CSYS_ORIGIN_CONSTRS
	//  |--PRO_E_CSYS_ORIGIN_CONSTR
	//    |--PRO_E_CSYS_ORIGIN_CONSTR_REF�@�I���W�i���Q��
	ProElement pro_e_csys_origin_constr_ref_x;
	ProModelitem mdlitemX;
	ProSelection pSelX;
	ProFeatureGeomitemVisit(&dtmPlnXFeature, PRO_SURFACE, UsrPointAddAction, NULL, (ProAppData)&mdlitemX);
	status = ProSelectionAlloc(NULL, &mdlitemX, &pSelX);
	status = ProElementAlloc(PRO_E_CSYS_ORIGIN_CONSTR_REF, &pro_e_csys_origin_constr_ref_x);

	value_data.type = PRO_VALUE_TYPE_SELECTION;
	value_data.v.r = pSelX;

	status = ProValueAlloc(&value);
	status = ProValueDataSet(value, &value_data);
	status = ProElementValueSet(pro_e_csys_origin_constr_ref_x, value);
	status = ProElemtreeElementAdd(pro_e_csys_origin_constr_x, NULL, pro_e_csys_origin_constr_ref_x);

	ProReference refPlnX;
	status = ProSelectionToReference(pSelX, &refPlnX);

	/*--------------------------------------------------------------------*\
		�f�[�^������Y
	\*--------------------------------------------------------------------*/
	//PRO_E_CSYS_ORIGIN_CONSTRS
	//  |--PRO_E_CSYS_ORIGIN_CONSTR

	ProElement pro_e_csys_origin_constr_y;
	status = ProElementAlloc(PRO_E_CSYS_ORIGIN_CONSTR, &pro_e_csys_origin_constr_y);
	status = ProElemtreeElementAdd(pro_e_csys_origin_constrs, NULL, pro_e_csys_origin_constr_y);

	//PRO_E_CSYS_ORIGIN_CONSTRS
	//  |--PRO_E_CSYS_ORIGIN_CONSTR
	//    |--PRO_E_CSYS_ORIGIN_CONSTR_REF�@�I���W�i���Q��
	ProElement pro_e_csys_origin_constr_ref_y;
	ProModelitem mdlitemY;
	ProSelection pSelY;

	ProFeatureGeomitemVisit(&dtmPlnYFeature, PRO_SURFACE, UsrPointAddAction, NULL, (ProAppData)&mdlitemY);
	status = ProSelectionAlloc(NULL, &mdlitemY, &pSelY);
	status = ProElementAlloc(PRO_E_CSYS_ORIGIN_CONSTR_REF, &pro_e_csys_origin_constr_ref_y);

	value_data.type = PRO_VALUE_TYPE_SELECTION;
	value_data.v.r = pSelY;

	status = ProValueAlloc(&value);
	status = ProValueDataSet(value, &value_data);
	status = ProElementValueSet(pro_e_csys_origin_constr_ref_y, value);
	status = ProElemtreeElementAdd(pro_e_csys_origin_constr_y, NULL, pro_e_csys_origin_constr_ref_y);

	ProReference refPlnY;
	status = ProSelectionToReference(pSelY, &refPlnY);

	/*--------------------------------------------------------------------*\
		��]����/�Q�Ƃ̑I��
	\*--------------------------------------------------------------------*/
	//PRO_E_CSYS_ORIENTSELAXIS1_REF
	ProElement csysOrientSelAxis1RefElem;
	status = ProElementAlloc(PRO_E_CSYS_ORIENTSELAXIS1_REF, &csysOrientSelAxis1RefElem);
	status = ProElementReferenceSet(csysOrientSelAxis1RefElem, refPlnZ);
	status = ProElemtreeElementAdd(featElemTree, NULL, csysOrientSelAxis1RefElem);

	//PRO_E_CSYS_ORIENTSELAXIS1_REF_OPT
	ProElement csysOrientSelAxis1RefOptElem;
	status = ProElementAlloc(PRO_E_CSYS_ORIENTSELAXIS1_REF_OPT, &csysOrientSelAxis1RefOptElem);
	status = ProElementIntegerSet(csysOrientSelAxis1RefOptElem, PRO_CSYS_DIRCSYSREF_OPT_ORIGIN);
	status = ProElemtreeElementAdd(featElemTree, NULL, csysOrientSelAxis1RefOptElem);

	//PRO_E_CSYS_ORIENTSELAXIS1_OPT
	ProElement csysOrientSelAxis1OptElem;
	status = ProElementAlloc(PRO_E_CSYS_ORIENTSELAXIS1_OPT, &csysOrientSelAxis1OptElem);
	status = ProElementIntegerSet(csysOrientSelAxis1OptElem, PRO_CSYS_ORIENTMOVE_AXIS_OPT_Y);
	status = ProElemtreeElementAdd(featElemTree, NULL, csysOrientSelAxis1OptElem);

	//PRO_E_CSYS_ORIENTSELAXIS1_FLIP(���]�̗L��)
	ProElement csysOrientSelAxis1FlipElem;
	status = ProElementAlloc(PRO_E_CSYS_ORIENTSELAXIS1_FLIP, &csysOrientSelAxis1FlipElem);

	if (iLRFlag == LEFT_FRAME) {
		status = ProElementIntegerSet(csysOrientSelAxis1FlipElem, PRO_CSYS_ORIENTSELAXIS_FLIP_YES);
	}
	else {
		status = ProElementIntegerSet(csysOrientSelAxis1FlipElem, PRO_CSYS_ORIENTSELAXIS_FLIP_NO);
	}
	status = ProElemtreeElementAdd(featElemTree, NULL, csysOrientSelAxis1FlipElem);

	//PRO_E_CSYS_ORIENTSELAXIS2_REF
	ProElement csysOrientSelAxis2RefElem;
	status = ProElementAlloc(PRO_E_CSYS_ORIENTSELAXIS2_REF, &csysOrientSelAxis2RefElem);
	status = ProElementReferenceSet(csysOrientSelAxis2RefElem, refPlnX);
	status = ProElemtreeElementAdd(featElemTree, NULL, csysOrientSelAxis2RefElem);

	//PRO_E_CSYS_ORIENTSELAXIS2_REF_OPT
	ProElement csysOrientSelAxis2RefOptElem;
	status = ProElementAlloc(PRO_E_CSYS_ORIENTSELAXIS2_REF_OPT, &csysOrientSelAxis2RefOptElem);
	status = ProElementIntegerSet(csysOrientSelAxis2RefOptElem, PRO_CSYS_DIRCSYSREF_OPT_ORIGIN);
	status = ProElemtreeElementAdd(featElemTree, NULL, csysOrientSelAxis2RefOptElem);

	//PRO_E_CSYS_ORIENTSELAXIS2_OPT
	ProElement csysOrientSelAxis2OptElem;
	status = ProElementAlloc(PRO_E_CSYS_ORIENTSELAXIS2_OPT, &csysOrientSelAxis2OptElem);
	status = ProElementIntegerSet(csysOrientSelAxis2OptElem, PRO_CSYS_ORIENTMOVE_AXIS_OPT_X);
	status = ProElemtreeElementAdd(featElemTree, NULL, csysOrientSelAxis2OptElem);

	//PRO_E_CSYS_ORIENTSELAXIS2_FLIP
	ProElement csysOrientSelAxis2FlipElem;
	status = ProElementAlloc(PRO_E_CSYS_ORIENTSELAXIS2_FLIP, &csysOrientSelAxis2FlipElem);
	status = ProElementIntegerSet(csysOrientSelAxis2FlipElem, PRO_CSYS_ORIENTSELAXIS_FLIP_NO);
	status = ProElemtreeElementAdd(featElemTree, NULL, csysOrientSelAxis2FlipElem);

	// �v�f�c���[����t�B�[�`�����쐬����
	ProIdTable idTable;
	idTable[0] = -1;
	ProAsmcomppath compPath;
	status = ProAsmcomppathInit((ProSolid)*curMdl, idTable, 0, &compPath);
	ProModelitem compMdlModelItem;
	status = ProMdlToModelitem(*curMdl, &compMdlModelItem);

	ProSelection compMdlSel;
	status = ProSelectionAlloc(&compPath, &compMdlModelItem, &compMdlSel);

	ProFeatureCreateOptions* featCreationOpts = NULL;
	status = ProArrayAlloc(1, sizeof(ProFeatureCreateOptions), 1, (ProArray*)&featCreationOpts);
	featCreationOpts[0] = PRO_FEAT_CR_NO_OPTS;

	ProErrorlist errorList;
	status = ProFeatureWithoptionsCreate(
		compMdlSel,
		featElemTree,
		featCreationOpts,
		PRO_REGEN_NO_FLAGS,
		csysFeature,
		&errorList);

	TRAIL_PRINT("%s(%d) : ProFeatureWithoptionsCreate = %s", __func__, __LINE__, getProErrorMessage(status));

	return status;
}


/*====================================================================*\
FUNCTION : createDatumPlaneX
PURPOSE  : X���̃f�[�^�����ʂ̍쐬
 *curMdl		(in) �f�[�^������(XM)�������i/�A�Z���u��
 offsetValue	(in) �f�[�^�����ʂ̋���
 dtmPlnFeature	(out) �f�[�^�����ʂ̃n���h��
 dtmPlnSelection(out) �f�[�^�����ʂ̔���
 ���l
 Top�p�[�c�̃f�[�^������(XM) ����ɂ��ăf�[�^�����ʂ��쐬����
\*====================================================================*/
ProError  createDatumPlaneX(ProMdl* curMdl, double offsetValue, ProFeature* dtmPlnFeature, int iDatumType)
{
	ProError status = PRO_TK_NO_ERROR;

	ProElement featElemTree;
	status = ProElementAlloc(PRO_E_FEATURE_TREE, &featElemTree);

	// �t�B�[�`���^�C�v (�f�[�^��) PRO_E_FEATURE_TYPE
	ProElement featureTypeElem;
	status = ProElementAlloc(PRO_E_FEATURE_TYPE, &featureTypeElem);
	status = ProElementIntegerSet(featureTypeElem, PRO_FEAT_DATUM);
	status = ProElemtreeElementAdd(featElemTree, NULL, featureTypeElem);

	// �t�B�[�`���� �̐ݒ� PRO_E_STD_FEATURE_NAME ���ݒ�̂��߁A�f�t�H���g
	ProElement featureNameElem;
	wchar_t* featureName = L"ADTM";
	status = ProElementAlloc(PRO_E_STD_FEATURE_NAME, &featureNameElem);
	status = ProElementWstringSet(featureNameElem, featureName);
	status = ProElemtreeElementAdd(featElemTree, NULL, featureNameElem);

	////////////////////////////////////////////////////////////////////
	// �f�[�^���z�u�̐ݒ� 
	// 
	//PRO_E_DTMPLN_CONSTRAINTS
	//  |--PRO_E_DTMPLN_CONSTRAINT
	//    |--PRO_E_DTMPLN_CONSTR_TYPE�@���ʂɉ������I�t�Z�b�g
	//    |--PRO_E_DTMPLN_CONSTR_REF   ���ƂȂ镽�ʂ̒�`
	//    |--PRO_E_DTMPLN_CONSTR_REF_OFFSET	�I�t�Z�b�g�l

	ProElement dtmplnConstraintsElem;
	status = ProElementAlloc(PRO_E_DTMPLN_CONSTRAINTS, &dtmplnConstraintsElem);
	status = ProElemtreeElementAdd(featElemTree, NULL, dtmplnConstraintsElem);

	ProElement* dtmplnConstraints = NULL;
	status = ProArrayAlloc(0, sizeof(ProElement), 1, (ProArray*)&dtmplnConstraints);

	ProElement dtmplnConstraintElem1;
	status = ProElementAlloc(PRO_E_DTMPLN_CONSTRAINT, &dtmplnConstraintElem1);

	// �f�[�^�����ʂ̍쐬��� (���ʂɉ������I�t�Z�b�g)
	ProElement dtmplnConstrTypeElem;
	status = ProElementAlloc(PRO_E_DTMPLN_CONSTR_TYPE, &dtmplnConstrTypeElem);
	status = ProElementIntegerSet(dtmplnConstrTypeElem, PRO_DTMPLN_OFFS);
	status = ProElemtreeElementAdd(dtmplnConstraintElem1, NULL, dtmplnConstrTypeElem);

	DatumAppData	appdataFeature;

	if (iDatumType == 0) {
		// ��������f�[�^�����ʂ̖��O��ݒ�
		ProWstringCopy(X_DATUM_NAME_0, appdataFeature.name, PRO_VALUE_UNUSED);

	}
	else if (iDatumType == 1) {
		// ��������f�[�^�����ʂ̖��O��ݒ�
		ProWstringCopy(X_DATUM_NAME_1, appdataFeature.name, PRO_VALUE_UNUSED);

	}
	else if (iDatumType == 2) {
		// ��������f�[�^�����ʂ̖��O��ݒ�
		ProWstringCopy(X_DATUM_NAME_2, appdataFeature.name, PRO_VALUE_UNUSED);

	}
	else if (iDatumType == 3) {
		// ��������f�[�^�����ʂ̖��O��ݒ�
		ProWstringCopy(X_DATUM_NAME_3, appdataFeature.name, PRO_VALUE_UNUSED);

	}
	else if (iDatumType == 4) {
		// ��������f�[�^�����ʂ̖��O��ݒ�
		ProWstringCopy(X_DATUM_NAME_4, appdataFeature.name, PRO_VALUE_UNUSED);

	}
	else {
		// X���W�̊�ƂȂ�f�[�^�����ʂ��z��O�̒l
		return PRO_TK_BAD_INPUTS;
	}

	// ����������
	appdataFeature.iFindCnt = 0;

	// Top�p�[�c�̃f�[�^������(X_DATUM_NAME)�̃t�B�[�`��������
	ProSolidFeatVisit((ProSolid)*curMdl, getFeatureIdAction, NULL, (ProAppData)&appdataFeature);

	if (appdataFeature.iFindCnt == 0) {
		// �f�[�^������(X_DATUM_NAME)�̃t�B�[�`����������Ȃ�����
		return (PRO_TK_E_NOT_FOUND);
	}
	
	// �t�B�[�`���� PRO_SURFACE ��ProModelitem�ɕϊ�����
	ProModelitem dtmplnConstrMdlItem;
	ProFeatureGeomitemVisit(&appdataFeature.feature, PRO_SURFACE, UsrPointAddAction, NULL,	(ProAppData)&dtmplnConstrMdlItem);

	ProIdTable idTable;
	idTable[0] = -1;

	ProAsmcomppath compPath;
	status = ProAsmcomppathInit((ProSolid)*curMdl, idTable, 0, &compPath);

	ProSelection dtmplnConstrSel;
	status = ProSelectionAlloc(&compPath, &dtmplnConstrMdlItem, &dtmplnConstrSel);

	ProReference dtmplnConstrRef;
	status = ProSelectionToReference(dtmplnConstrSel, &dtmplnConstrRef);

	// �Q�ƒl 
	ProElement dtmplnConstrRefElem;
	status = ProElementAlloc(PRO_E_DTMPLN_CONSTR_REF, &dtmplnConstrRefElem);
	status = ProElementReferenceSet(dtmplnConstrRefElem, dtmplnConstrRef);
	status = ProElemtreeElementAdd(dtmplnConstraintElem1, NULL, dtmplnConstrRefElem);

	// �I�t�Z�b�g�l 
	ProElement dtmplnOffCsysOffsetElem;
	status = ProElementAlloc(PRO_E_DTMPLN_CONSTR_REF_OFFSET, &dtmplnOffCsysOffsetElem);
	status = ProElementDoubleSet(dtmplnOffCsysOffsetElem, offsetValue);
	status = ProElemtreeElementAdd(dtmplnConstraintElem1, NULL, dtmplnOffCsysOffsetElem);

	status = ProArrayObjectAdd((ProArray*)&dtmplnConstraints, PRO_VALUE_UNUSED, 1, &dtmplnConstraintElem1);

	//Set value for PRO_E_DTMPLN_CONSTRAINTS element.
	status = ProElementArraySet(dtmplnConstraintsElem, NULL, dtmplnConstraints);

	// ���]���� 
	ProElement dtmplnFlipDirElem;
	status = ProElementAlloc(PRO_E_DTMPLN_FLIP_DIR, &dtmplnFlipDirElem);
	status = ProElementIntegerSet(dtmplnFlipDirElem, PRO_DTMPLN_FLIP_DIR_NO);
	status = ProElemtreeElementAdd(featElemTree, NULL, dtmplnFlipDirElem);

	// �v�f�c���[����t�B�[�`�����쐬����
	ProModelitem compMdlModelItem;
	status = ProMdlToModelitem(*curMdl, &compMdlModelItem);

	ProSelection compMdlSel;
	status = ProSelectionAlloc(&compPath, &compMdlModelItem, &compMdlSel);

	ProFeatureCreateOptions* featCreationOpts = NULL;
	status = ProArrayAlloc(1, sizeof(ProFeatureCreateOptions), 1, (ProArray*)&featCreationOpts);
	featCreationOpts[0] = PRO_FEAT_CR_NO_OPTS;

	ProErrorlist errorList;
	status = ProFeatureWithoptionsCreate(
		compMdlSel,
		featElemTree,
		featCreationOpts,
		PRO_REGEN_NO_FLAGS,
		dtmPlnFeature,
		&errorList);

	TRAIL_PRINT("%s(%d) : ProFeatureWithoptionsCreate = %s", __func__, __LINE__, getProErrorMessage(status));

	return status;
}

/*====================================================================*\
FUNCTION : createDatumPlaneY
PURPOSE  : Y���̃f�[�^�����ʂ̍쐬
 *curMdl		(in) �������p�[�c(***_1/2)�������i/�A�Z���u��
 mdlPart		(in) �������p�[�cProMdl
 offsetValue	(in) �f�[�^�����ʂ̋���
 dtmPlnFeature	(out) �f�[�^�����ʂ̃n���h��
 ���l
 �w�茊�����p�[�c�̃T�[�t�F�C�X����ɂ��ăf�[�^�����ʂ��쐬����
\*====================================================================*/
ProError  createDatumPlaneY(ProMdl* curMdl, ProMdl mdlPart, double offsetValue, ProFeature* dtmPlnFeature)
{
	ProError status = PRO_TK_NO_ERROR;

	ProElement featElemTree;
	status = ProElementAlloc(PRO_E_FEATURE_TREE, &featElemTree);

	// �t�B�[�`���^�C�v (�f�[�^��) PRO_E_FEATURE_TYPE
	ProElement featureTypeElem;
	status = ProElementAlloc(PRO_E_FEATURE_TYPE, &featureTypeElem);
	status = ProElementIntegerSet(featureTypeElem, PRO_FEAT_DATUM);
	status = ProElemtreeElementAdd(featElemTree, NULL, featureTypeElem);

	// �t�B�[�`���� �̐ݒ� PRO_E_STD_FEATURE_NAME ���ݒ�̂��߁A�f�t�H���g
	ProElement featureNameElem;
	wchar_t* featureName = L"ADTM";
	status = ProElementAlloc(PRO_E_STD_FEATURE_NAME, &featureNameElem);
	status = ProElementWstringSet(featureNameElem, featureName);
	status = ProElemtreeElementAdd(featElemTree, NULL, featureNameElem);

	////////////////////////////////////////////////////////////////////
	// �f�[�^���z�u�̐ݒ� 
	// 
	//PRO_E_DTMPLN_CONSTRAINTS
	//  |--PRO_E_DTMPLN_CONSTRAINT
	//    |--PRO_E_DTMPLN_CONSTR_TYPE�@���ʂɉ������I�t�Z�b�g
	//    |--PRO_E_DTMPLN_CONSTR_REF   ���ƂȂ镽�ʂ̒�`
	//    |--PRO_E_DTMPLN_CONSTR_REF_OFFSET	�I�t�Z�b�g�l

	ProElement dtmplnConstraintsElem;
	status = ProElementAlloc(PRO_E_DTMPLN_CONSTRAINTS, &dtmplnConstraintsElem);
	status = ProElemtreeElementAdd(featElemTree, NULL, dtmplnConstraintsElem);

	ProElement* dtmplnConstraints = NULL;
	status = ProArrayAlloc(0, sizeof(ProElement), 1, (ProArray*)&dtmplnConstraints);

	ProElement dtmplnConstraintElem1;
	status = ProElementAlloc(PRO_E_DTMPLN_CONSTRAINT, &dtmplnConstraintElem1);

	// �f�[�^�����ʂ̍쐬��� (���ʂɉ������I�t�Z�b�g)
	ProElement dtmplnConstrTypeElem;
	status = ProElementAlloc(PRO_E_DTMPLN_CONSTR_TYPE, &dtmplnConstrTypeElem);
	status = ProElementIntegerSet(dtmplnConstrTypeElem, PRO_DTMPLN_OFFS);
	status = ProElemtreeElementAdd(dtmplnConstraintElem1, NULL, dtmplnConstrTypeElem);

	/*--------------------------------------------------------------------*\
		�T�[�t�F�X�̏����擾���AProModelitem���擾����
	\*--------------------------------------------------------------------*/
	SurfaceAppData appdataSurface;

	// �e�l�̏����� double�^�̍ő�l�ŏ���������
	appdataSurface.dYPoint = DBL_MAX;

	// ��ƂȂ�T�[�t�F�C�X���擾
	status = ProSolidSurfaceVisit(ProMdlToSolid(mdlPart),
		(ProSurfaceVisitAction)getLowerSurfaceFromYpointAction,
		(ProSurfaceFilterAction)NULL,
		(ProAppData)&appdataSurface);


	if (status != PRO_TK_NO_ERROR) {
		// ��ƂȂ�T�[�t�F�C�X��������Ȃ�����
		return (PRO_TK_E_NOT_FOUND);
	}

	// ProModelitem�ɕϊ�����
	ProModelitem dtmplnConstrMdlItem;
	int iSurfaceId;
	status = ProSurfaceIdGet(appdataSurface.surface, &iSurfaceId);
	status = ProModelitemInit(mdlPart , iSurfaceId, PRO_SURFACE, &dtmplnConstrMdlItem);

	/*--------------------------------------------------------------------*\
		�t�B�[�`���̏����擾���AProAsmcompath�f�[�^�\��������������
	\*--------------------------------------------------------------------*/
	ProAsmcomppath compPath;
	getAsmcompathIncludePart(*curMdl, mdlPart, &compPath);

	ProSelection dtmplnConstrSel;
	status = ProSelectionAlloc(&compPath, &dtmplnConstrMdlItem, &dtmplnConstrSel);

	ProReference dtmplnConstrRef;
	status = ProSelectionToReference(dtmplnConstrSel, &dtmplnConstrRef);

	// �Q�ƒl 
	ProElement dtmplnConstrRefElem;
	status = ProElementAlloc(PRO_E_DTMPLN_CONSTR_REF, &dtmplnConstrRefElem);
	status = ProElementReferenceSet(dtmplnConstrRefElem, dtmplnConstrRef);
	status = ProElemtreeElementAdd(dtmplnConstraintElem1, NULL, dtmplnConstrRefElem);

	// �I�t�Z�b�g�l 
	ProElement dtmplnOffCsysOffsetElem;
	status = ProElementAlloc(PRO_E_DTMPLN_CONSTR_REF_OFFSET, &dtmplnOffCsysOffsetElem);
	status = ProElementDoubleSet(dtmplnOffCsysOffsetElem, offsetValue);
	status = ProElemtreeElementAdd(dtmplnConstraintElem1, NULL, dtmplnOffCsysOffsetElem);

	status = ProArrayObjectAdd((ProArray*)&dtmplnConstraints, PRO_VALUE_UNUSED, 1, &dtmplnConstraintElem1);

	//Set value for PRO_E_DTMPLN_CONSTRAINTS element.
	status = ProElementArraySet(dtmplnConstraintsElem, NULL, dtmplnConstraints);

	// ���]���� 
	ProElement dtmplnFlipDirElem;
	status = ProElementAlloc(PRO_E_DTMPLN_FLIP_DIR, &dtmplnFlipDirElem);
	status = ProElementIntegerSet(dtmplnFlipDirElem, PRO_DTMPLN_FLIP_DIR_NO);
	status = ProElemtreeElementAdd(featElemTree, NULL, dtmplnFlipDirElem);

	// �v�f�c���[����t�B�[�`�����쐬����
	ProModelitem compMdlModelItem;
	status = ProMdlToModelitem(*curMdl, &compMdlModelItem);

	ProSelection compMdlSel;
	ProAsmcomppath compPathAssy;
	ProIdTable idTable;
	idTable[0] = -1;
	status = ProAsmcomppathInit((ProSolid)*curMdl, idTable, 0, &compPathAssy);
	status = ProSelectionAlloc(&compPathAssy, &compMdlModelItem, &compMdlSel);

	ProFeatureCreateOptions* featCreationOpts = NULL;
	status = ProArrayAlloc(1, sizeof(ProFeatureCreateOptions), 1, (ProArray*)&featCreationOpts);

	featCreationOpts[0] = PRO_FEAT_CR_NO_OPTS;

	ProErrorlist errorList;
	status = ProFeatureWithoptionsCreate(
		compMdlSel,
		featElemTree,
		featCreationOpts,
		PRO_REGEN_NO_FLAGS,
		dtmPlnFeature,
		&errorList);

	TRAIL_PRINT("%s(%d) : ProFeatureWithoptionsCreate = %s", __func__, __LINE__, getProErrorMessage(status));

	return status;
}

/*====================================================================*\
FUNCTION : createDatumPlaneZ
PURPOSE  : Z���̃f�[�^�����ʂ̍쐬
 *curMdl		(in) �������p�[�c(***_1/2)�������i/�A�Z���u��
 mdlPart		(in) �������p�[�cProMdl
 offsetValue	(in) �f�[�^�����ʂ̋���
 dtmplnConstrMdlItem(in) modelitem
 dtmPlnFeature	(out) �f�[�^�����ʂ̃n���h��
 ���l
 �w�茊�����p�[�c�̃T�[�t�F�C�X����ɂ��ăf�[�^�����ʂ��쐬����
\*====================================================================*/
ProError  createDatumPlaneZ(ProMdl* curMdl, ProMdl mdlPart, double offsetValue, ProModelitem dtmplnConstrMdlItem, ProFeature* dtmPlnFeature)
{
	ProError status = PRO_TK_NO_ERROR;

	ProElement featElemTree;
	status = ProElementAlloc(PRO_E_FEATURE_TREE, &featElemTree);

	// �t�B�[�`���^�C�v (�f�[�^��) PRO_E_FEATURE_TYPE
	ProElement featureTypeElem;
	status = ProElementAlloc(PRO_E_FEATURE_TYPE, &featureTypeElem);
	status = ProElementIntegerSet(featureTypeElem, PRO_FEAT_DATUM);
	status = ProElemtreeElementAdd(featElemTree, NULL, featureTypeElem);

	// �t�B�[�`���� �̐ݒ� PRO_E_STD_FEATURE_NAME ���ݒ�̂��߁A�f�t�H���g
	ProElement featureNameElem;
	wchar_t* featureName = L"ADTM";
	status = ProElementAlloc(PRO_E_STD_FEATURE_NAME, &featureNameElem);
	status = ProElementWstringSet(featureNameElem, featureName);
	status = ProElemtreeElementAdd(featElemTree, NULL, featureNameElem);

	////////////////////////////////////////////////////////////////////
	// �f�[�^���z�u�̐ݒ� 
	// 
	//PRO_E_DTMPLN_CONSTRAINTS
	//  |--PRO_E_DTMPLN_CONSTRAINT
	//    |--PRO_E_DTMPLN_CONSTR_TYPE�@���ʂɉ������I�t�Z�b�g
	//    |--PRO_E_DTMPLN_CONSTR_REF   ���ƂȂ镽�ʂ̒�`
	//    |--PRO_E_DTMPLN_CONSTR_REF_OFFSET	�I�t�Z�b�g�l

	ProElement dtmplnConstraintsElem;
	status = ProElementAlloc(PRO_E_DTMPLN_CONSTRAINTS, &dtmplnConstraintsElem);
	status = ProElemtreeElementAdd(featElemTree, NULL, dtmplnConstraintsElem);

	ProElement* dtmplnConstraints = NULL;
	status = ProArrayAlloc(0, sizeof(ProElement), 1, (ProArray*)&dtmplnConstraints);

	ProElement dtmplnConstraintElem1;
	status = ProElementAlloc(PRO_E_DTMPLN_CONSTRAINT, &dtmplnConstraintElem1);

	// �f�[�^�����ʂ̍쐬��� (���ʂɉ������I�t�Z�b�g)
	ProElement dtmplnConstrTypeElem;
	status = ProElementAlloc(PRO_E_DTMPLN_CONSTR_TYPE, &dtmplnConstrTypeElem);
	status = ProElementIntegerSet(dtmplnConstrTypeElem, PRO_DTMPLN_OFFS);
	status = ProElemtreeElementAdd(dtmplnConstraintElem1, NULL, dtmplnConstrTypeElem);

	/*--------------------------------------------------------------------*\
		�t�B�[�`���̏����擾���AProAsmcompath�f�[�^�\��������������
	\*--------------------------------------------------------------------*/
	ProAsmcomppath compPath;
	getAsmcompathIncludePart(*curMdl, mdlPart, &compPath);

	ProSelection dtmplnConstrSel;
	status = ProSelectionAlloc(&compPath, &dtmplnConstrMdlItem, &dtmplnConstrSel);

	ProReference dtmplnConstrRef;
	status = ProSelectionToReference(dtmplnConstrSel, &dtmplnConstrRef);

	// �Q�ƒl 
	ProElement dtmplnConstrRefElem;
	status = ProElementAlloc(PRO_E_DTMPLN_CONSTR_REF, &dtmplnConstrRefElem);
	status = ProElementReferenceSet(dtmplnConstrRefElem, dtmplnConstrRef);
	status = ProElemtreeElementAdd(dtmplnConstraintElem1, NULL, dtmplnConstrRefElem);

	// �I�t�Z�b�g�l 
	ProElement dtmplnOffCsysOffsetElem;
	status = ProElementAlloc(PRO_E_DTMPLN_CONSTR_REF_OFFSET, &dtmplnOffCsysOffsetElem);
	status = ProElementDoubleSet(dtmplnOffCsysOffsetElem, offsetValue);
	status = ProElemtreeElementAdd(dtmplnConstraintElem1, NULL, dtmplnOffCsysOffsetElem);

	status = ProArrayObjectAdd((ProArray*)&dtmplnConstraints, PRO_VALUE_UNUSED, 1, &dtmplnConstraintElem1);

	//Set value for PRO_E_DTMPLN_CONSTRAINTS element.
	status = ProElementArraySet(dtmplnConstraintsElem, NULL, dtmplnConstraints);

	// ���]���� 
	ProElement dtmplnFlipDirElem;
	status = ProElementAlloc(PRO_E_DTMPLN_FLIP_DIR, &dtmplnFlipDirElem);
	status = ProElementIntegerSet(dtmplnFlipDirElem, PRO_DTMPLN_FLIP_DIR_NO);
	status = ProElemtreeElementAdd(featElemTree, NULL, dtmplnFlipDirElem);

	// �v�f�c���[����t�B�[�`�����쐬����
	ProModelitem compMdlModelItem;
	status = ProMdlToModelitem(*curMdl, &compMdlModelItem);

	ProSelection compMdlSel;
	ProAsmcomppath compPathAssy;
	ProIdTable idTable;
	idTable[0] = -1;
	status = ProAsmcomppathInit((ProSolid)*curMdl, idTable, 0, &compPathAssy);
	status = ProSelectionAlloc(&compPathAssy, &compMdlModelItem, &compMdlSel);

	ProFeatureCreateOptions* featCreationOpts = NULL;
	status = ProArrayAlloc(1, sizeof(ProFeatureCreateOptions), 1, (ProArray*)&featCreationOpts);
	featCreationOpts[0] = PRO_FEAT_CR_NO_OPTS;

	//ProFeature dtmPlnFeature;
	ProErrorlist errorList;
	status = ProFeatureWithoptionsCreate(
		compMdlSel,
		featElemTree,
		featCreationOpts,
		PRO_REGEN_NO_FLAGS,
		dtmPlnFeature,
		&errorList);

	TRAIL_PRINT("%s(%d) : ProFeatureWithoptionsCreate = %s", __func__, __LINE__, getProErrorMessage(status));

	return status;
}

/*====================================================================*\
FUNCTION : createHoleAroundPlane
PURPOSE  : �f�[�^�����ʂ���Ɍ����쐬 (��1�̍쐬)
	ProMdl top_asm,					in top�A�Z���u��
	ProMdl part						in ����������p�[�c
	ProFeature dtmPlnFeature_X,		in �f�[�^���ʃt�B�[�`��(X��)
	ProFeature dtmPlnFeature_YZ,	in �f�[�^���ʃt�B�[�`��(Y��)
	ProSelection selSurf			in �z�u��/�����������
	double distX,					in ��1����̋���(X��)
	double distY					in ��1����̋���(Y��)
	double dDiameter				in �����a
	ProFeature* holeFeature			out ���t�B�[�`��
\*====================================================================*/
ProError createHoleAroundPlane(ProMdl top_asm, 
	ProMdl part,
	ProFeature dtmPlnFeature_X,
	ProFeature dtmPlnFeature_Y,
	ProSelection selSurf,
	ProFeature dtmPlnFeature_selSurf,
	double distX,
	double distY,
	double dDiameter,
	ProFeature* holeFeature)
{
	ProError status;
	ProElement feat_elemtree;
	ProElement elem_feattype;
	ProElement elem_featform;
	ProElement elem_hle_com;
	ProElement elem_hle_type_new;
	ProElement elem_hle_stan_type;
	ProElement elem_diameter;
	ProElement elem_hole_std_depth;
	ProElement elem_hole_depth_to;
	ProElement elem_hole_depth_to_type;
	ProElement elem_hole_depth_to_value;
	ProElement elem_hole_depth_from;
	ProElement elem_hole_depth_from_type;
	ProElement elem_hole_depth_from_value;
	ProElement elem_hle_placement;
	ProElement elem_hle_prim_ref;
	ProElement elem_hle_pl_type;
	ProElement elem_hle_dim_ref1;
	ProElement elem_hle_dim_dist1;
	ProElement elem_hle_dim_ref2;
	ProElement elem_hle_dim_dist2;
	ProValue value;
	ProValueData value_data;
	ProSelection* p_selection;
	int n_selection;
	ProFeatureCreateOptions* options = 0;
	ProFeature created_feature;
	ProErrorlist p_errors;
	//ProMdl model;
	ProModelitem model_item;
	ProSelection model_selection;
	ProReference reference;


	/***********************************************
	 �v�f�c���[�쐬�̊J�n
	*************************************************/
	/* �v�f�c���[(���[�g�v�f)�̒ǉ� */
	status = ProElementAlloc(PRO_E_FEATURE_TREE, &feat_elemtree);

	/***********************************************
	 PRO_E_FEATURE_TYPE
	*************************************************/
	/* �v�f�c���[�� �t�B�[�`���^�C�v(��) ��ǉ����� */
	status = ProElementAlloc(PRO_E_FEATURE_TYPE, &elem_feattype);
	status = ProElementIntegerSet(elem_feattype, PRO_FEAT_HOLE);
	status = ProElemtreeElementAdd(feat_elemtree, NULL, elem_feattype);

	/***********************************************
	 PRO_E_FEATURE_FORM
	*************************************************/
	/* �v�f�c���[�� �t�B�[�`���t�H�[��(�X�g���[�g�z�[��) ��ǉ����� */
	status = ProElementAlloc(PRO_E_FEATURE_FORM, &elem_featform);
	status = ProElementIntegerSet(elem_featform, PRO_HLE_TYPE_STRAIGHT);
	status = ProElemtreeElementAdd(feat_elemtree, NULL, elem_featform);

	/***********************************************
	 PRO_E_HLE_COM
	*************************************************/
	/* �v�f�c���[�� �����̋��ʗv�f �̒ǉ�  */
	status = ProElementAlloc(PRO_E_HLE_COM, &elem_hle_com);
	status = ProElemtreeElementAdd(feat_elemtree, NULL, elem_hle_com);

	/* ����� (���^�C�v:�X�g���[�g) ���w�肷�� */
	status = ProElementAlloc(PRO_E_HLE_TYPE_NEW, &elem_hle_type_new);
	status = ProElementIntegerSet(elem_hle_type_new, PRO_HLE_NEW_TYPE_STRAIGHT);
	status = ProElemtreeElementAdd(elem_hle_com, NULL, elem_hle_type_new);

	/* ����� (���̒��a) ���w�肷�� */
	status = ProElementAlloc(PRO_E_DIAMETER, &elem_diameter);
	status = ProElementDoubleSet(elem_diameter, dDiameter);
	status = ProElemtreeElementAdd(elem_hle_com, NULL, elem_diameter);

	/* �W���[�x�̗v�f��ǉ�����

			  |--PRO_E_HOLE_STD_DEPTH
			  |    |--PRO_E_HOLE_DEPTH_TO
			  |    |    |--PRO_E_HOLE_DEPTH_TO_TYPE
			  |    |    |--PRO_E_EXT_DEPTH_TO_VALUE
			  |    |--PRO_E_HOLE_DEPTH_FROM
			  |         |--PRO_E_HOLE_DEPTH_FROM_TYPE
			  |         |--PRO_E_EXT_DEPTH_FROM_VALUE
	*/

	// �[���v�f
	status = ProElementAlloc(PRO_E_HOLE_STD_DEPTH, &elem_hole_std_depth);
	status = ProElemtreeElementAdd(elem_hle_com, NULL, elem_hole_std_depth);

	// side1 ���F Blind �[��25
	status = ProElementAlloc(PRO_E_HOLE_DEPTH_TO, &elem_hole_depth_to);
	status = ProElemtreeElementAdd(elem_hole_std_depth, NULL, elem_hole_depth_to);
	status = ProElementAlloc(PRO_E_HOLE_DEPTH_TO_TYPE, &elem_hole_depth_to_type);
	status = ProElementIntegerSet(elem_hole_depth_to_type, PRO_HLE_STRGHT_BLIND_DEPTH);
	status = ProElemtreeElementAdd(elem_hole_depth_to, NULL, elem_hole_depth_to_type);
	status = ProElementAlloc(PRO_E_EXT_DEPTH_TO_VALUE, &elem_hole_depth_to_value);
	status = ProElementDoubleSet(elem_hole_depth_to_value, 25.0);
	status = ProElemtreeElementAdd(elem_hole_depth_to, NULL, elem_hole_depth_to_value);
	// side2 ���F Blind �[��25
	status = ProElementAlloc(PRO_E_HOLE_DEPTH_FROM, &elem_hole_depth_from);
	status = ProElemtreeElementAdd(elem_hole_std_depth, NULL, elem_hole_depth_from);
	status = ProElementAlloc(PRO_E_HOLE_DEPTH_FROM_TYPE, &elem_hole_depth_from_type);
	status = ProElementIntegerSet(elem_hole_depth_from_type, PRO_HLE_STRGHT_BLIND_DEPTH);
	status = ProElemtreeElementAdd(elem_hole_depth_from, NULL, elem_hole_depth_from_type);
	status = ProElementAlloc(PRO_E_EXT_DEPTH_FROM_VALUE, &elem_hole_depth_from_value);
	status = ProElementDoubleSet(elem_hole_depth_from_value, 25.0);
	status = ProElemtreeElementAdd(elem_hole_depth_from, NULL, elem_hole_depth_from_value);


	/* �z�u�̏ڍׂɊ֘A����v�f��ǉ�����
	 �������Ƃ��铯����

		 |--PRO_E_HLE_PLACEMENT
		 |    |--PRO_E_HLE_PRIM_REF
		 |    |--PRO_E_HLE_PL_TYPE
		 |    |--PRO_E_HLE_DIM_REF1
		 |    |--PRO_E_HLE_DIM_DIST1
		 |    |--PRO_E_HLE_DIM_REF2
		 |    |--PRO_E_HLE_DIM_DIST2

	*/

	status = ProElementAlloc(PRO_E_HLE_PLACEMENT, &elem_hle_placement);
	status = ProElemtreeElementAdd(feat_elemtree, NULL, elem_hle_placement);

	ProModelitem mdlitem1;
	ProSelection pSel1;
	if (selSurf == NULL) {
		ProFeatureGeomitemVisit(&dtmPlnFeature_selSurf, PRO_SURFACE, UsrPointAddAction, NULL, (ProAppData)&mdlitem1);
		status = ProSelectionAlloc(NULL, &mdlitem1, &pSel1);
		status = ProElementAlloc(PRO_E_HLE_PRIM_REF, &elem_hle_prim_ref);
		value_data.type = PRO_VALUE_TYPE_SELECTION;
		value_data.v.r = pSel1;
	}
	else {
		status = ProElementAlloc(PRO_E_HLE_PRIM_REF, &elem_hle_prim_ref);
		value_data.type = PRO_VALUE_TYPE_SELECTION;
		value_data.v.r = selSurf;
	}
	status = ProValueAlloc(&value);
	status = ProValueDataSet(value, &value_data);
	status = ProElementValueSet(elem_hle_prim_ref, value);
	status = ProElemtreeElementAdd(elem_hle_placement, NULL, elem_hle_prim_ref);

	// ���z�u�I�v�V����
	status = ProElementAlloc(PRO_E_HLE_PL_TYPE, &elem_hle_pl_type);
	status = ProElementIntegerSet(elem_hle_pl_type, PRO_HLE_PL_TYPE_LIN);
	status = ProElemtreeElementAdd(elem_hle_placement, NULL, elem_hle_pl_type);

	// �񎟑I�� (X��)
	ProModelitem mdlitem2;
	ProSelection pSel2;

	ProFeatureGeomitemVisit(&dtmPlnFeature_X, PRO_SURFACE, UsrPointAddAction, NULL, (ProAppData)&mdlitem2);
	status = ProSelectionAlloc(NULL, &mdlitem2, &pSel2);
	status = ProElementAlloc(PRO_E_HLE_DIM_REF1, &elem_hle_prim_ref);

	value_data.type = PRO_VALUE_TYPE_SELECTION;
	value_data.v.r = pSel2;

	status = ProValueAlloc(&value);
	status = ProValueDataSet(value, &value_data);
	status = ProElementValueSet(elem_hle_prim_ref, value);
	status = ProElemtreeElementAdd(elem_hle_placement, NULL, elem_hle_prim_ref);

	// ���� 1
	status = ProElementAlloc(PRO_E_HLE_DIM_DIST1, &elem_hle_dim_dist1);
	status = ProElementDoubleSet(elem_hle_dim_dist1, distX);
	status = ProElemtreeElementAdd(elem_hle_placement, NULL, elem_hle_dim_dist1);

	// �O���I�� (Y��)
	ProModelitem mdlitem3;
	ProSelection pSel3;
	ProFeatureGeomitemVisit(&dtmPlnFeature_Y, PRO_SURFACE, UsrPointAddAction, NULL, (ProAppData)&mdlitem3);
	status = ProSelectionAlloc(NULL, &mdlitem3, &pSel3);

	status = ProElementAlloc(PRO_E_HLE_DIM_REF2, &elem_hle_prim_ref);

	value_data.type = PRO_VALUE_TYPE_SELECTION;
	value_data.v.r = pSel3;
	status = ProValueAlloc(&value);
	status = ProValueDataSet(value, &value_data);
	status = ProElementValueSet(elem_hle_prim_ref, value);
	status = ProElemtreeElementAdd(elem_hle_placement, NULL, elem_hle_prim_ref);

	// ���� 2
	status = ProElementAlloc(PRO_E_HLE_DIM_DIST2, &elem_hle_dim_dist2);
	status = ProElementDoubleSet(elem_hle_dim_dist2, distY);
	status = ProElemtreeElementAdd(elem_hle_placement, NULL, elem_hle_dim_dist2);
	/* �v�f�c���[�쐬�̏I�� */


	/***********************************************
	 �t�B�[�`���[�̍쐬
	*************************************************/
	ProAsmcomppath compPath;
	status = getAsmcompathIncludePart(top_asm, part, &compPath);
	if (status != PRO_TK_NO_ERROR) {
		// �p�[�c�̃t�B�[�`����������Ȃ�����
		return (PRO_TK_E_NOT_FOUND);
	}

	status = ProMdlToModelitem(part, &model_item);
	status = ProSelectionAlloc(&compPath, &model_item, &model_selection);
	status = ProArrayAlloc(1, sizeof(ProFeatureCreateOptions), 1, (ProArray*)&options);

	options[0] = PRO_FEAT_CR_NO_OPTS;
	status = ProFeatureWithoptionsCreate(model_selection, feat_elemtree, options, PRO_REGEN_NO_FLAGS, holeFeature, &p_errors);
	TRAIL_PRINT("%s(%d) : ProFeatureWithoptionsCreate = %s", __func__, __LINE__, getProErrorMessage(status));

	/***********************************************
	 ���\�[�X�̉��
	*************************************************/
	status = ProElementFree(&feat_elemtree);
	status = ProArrayFree((ProArray*)&options);

	return (status);
}

/*====================================================================*\
FUNCTION : createHoleBaseHoleOne
PURPOSE  : ��1����ɁA��1�Ɠ����ʂɌ����쐬����(��2�ȍ~�̍쐬)
	ProMdl top_asm,					in top�A�Z���u��
	ProMdl part						in ����������p�[�c
	ProFeature holeFeatureOne,		in ��ƂȂ錊1�̃t�B�[�`��
	ProFeature dtmPlnFeature_YZ,	in ���@��]�����ƂȂ�t�B�[�`��
	ProSelection selSurf			in �z�u��/�����������
	double distX,					in ��1����̋���(X��)
	double distY					in ��1����̋���(Y��)
	double dDiameter				in �����a
	ProFeature* holeFeature			out ���t�B�[�`��
\*====================================================================*/
ProError createHoleBaseHoleOne(ProMdl top_asm,
	ProMdl part,
	ProFeature holeFeatureOne,
	ProFeature dtmPlnFeature_YZ,
	ProSelection selSurf,
	ProFeature dtmPlnFeature_selSurf,
	double distX,
	double distY,
	double dDiameter,
	ProFeature* holeFeature)
{
	ProError status;

	ProElement feat_elemtree;
	ProElement elem_feattype;
	ProElement elem_featform;
	ProElement elem_hle_com;
	ProElement elem_hle_type_new;
	ProElement elem_hle_stan_type;
	ProElement elem_diameter;
	ProElement elem_hole_std_depth;
	ProElement elem_hole_depth_to;
	ProElement elem_hole_depth_to_type;
	ProElement elem_hole_depth_to_value;
	ProElement elem_hole_depth_from;
	ProElement elem_hole_depth_from_type;
	ProElement elem_hole_depth_from_value;
	ProElement elem_hle_placement;
	ProElement elem_hle_prim_ref;
	ProElement elem_hle_pl_type;
	ProElement elem_hle_dim_ref1;
	ProElement elem_hle_dim_dist1;
	ProElement elem_hle_dim_dist2;
	ProElement lin_hole_dir_ref;
	ProValue value;
	ProValueData value_data;
	ProSelection* p_selection;
	int n_selection;

	ProFeatureCreateOptions* options = 0;
	ProFeature created_feature;
	ProErrorlist p_errors;
	//ProMdl model;
	ProModelitem model_item;
	ProSelection model_selection;
	ProReference reference;


	/***********************************************
	 �v�f�c���[�쐬�̊J�n
	*************************************************/
	/* �v�f�c���[(���[�g�v�f)�̒ǉ� */
	status = ProElementAlloc(PRO_E_FEATURE_TREE, &feat_elemtree);

	/***********************************************
	 PRO_E_FEATURE_TYPE
	*************************************************/
	/* �v�f�c���[�� �t�B�[�`���^�C�v(��) ��ǉ����� */
	status = ProElementAlloc(PRO_E_FEATURE_TYPE, &elem_feattype);
	status = ProElementIntegerSet(elem_feattype, PRO_FEAT_HOLE);
	status = ProElemtreeElementAdd(feat_elemtree, NULL, elem_feattype);

	/***********************************************
	 PRO_E_FEATURE_FORM
	*************************************************/
	/* �v�f�c���[�� �t�B�[�`���t�H�[��(�X�g���[�g�z�[��) ��ǉ����� */
	status = ProElementAlloc(PRO_E_FEATURE_FORM, &elem_featform);
	status = ProElementIntegerSet(elem_featform, PRO_HLE_TYPE_STRAIGHT);
	status = ProElemtreeElementAdd(feat_elemtree, NULL, elem_featform);

	/***********************************************
	 PRO_E_HLE_COM
	*************************************************/
	/* �v�f�c���[�� �����̋��ʗv�f �̒ǉ�  */
	status = ProElementAlloc(PRO_E_HLE_COM, &elem_hle_com);
	status = ProElemtreeElementAdd(feat_elemtree, NULL, elem_hle_com);

	/* ����� (���^�C�v:�X�g���[�g) ���w�肷�� */
	status = ProElementAlloc(PRO_E_HLE_TYPE_NEW, &elem_hle_type_new);
	status = ProElementIntegerSet(elem_hle_type_new, PRO_HLE_NEW_TYPE_STRAIGHT);
	status = ProElemtreeElementAdd(elem_hle_com, NULL, elem_hle_type_new);

	/* ����� (���̒��a) ���w�肷�� */
	status = ProElementAlloc(PRO_E_DIAMETER, &elem_diameter);
	status = ProElementDoubleSet(elem_diameter, dDiameter);
	status = ProElemtreeElementAdd(elem_hle_com, NULL, elem_diameter);

	/* �W���[�x�̗v�f��ǉ�����

			  |--PRO_E_HOLE_STD_DEPTH
			  |    |--PRO_E_HOLE_DEPTH_TO
			  |    |    |--PRO_E_HOLE_DEPTH_TO_TYPE
			  |    |    |--PRO_E_EXT_DEPTH_TO_VALUE
			  |    |--PRO_E_HOLE_DEPTH_FROM
			  |         |--PRO_E_HOLE_DEPTH_FROM_TYPE
			  |         |--PRO_E_EXT_DEPTH_FROM_VALUE
	*/

	// �[���v�f
	status = ProElementAlloc(PRO_E_HOLE_STD_DEPTH, &elem_hole_std_depth);
	status = ProElemtreeElementAdd(elem_hle_com, NULL, elem_hole_std_depth);

	// side1 ���F Blind �[��25
	status = ProElementAlloc(PRO_E_HOLE_DEPTH_TO, &elem_hole_depth_to);
	status = ProElemtreeElementAdd(elem_hole_std_depth, NULL, elem_hole_depth_to);
	status = ProElementAlloc(PRO_E_HOLE_DEPTH_TO_TYPE, &elem_hole_depth_to_type);
	status = ProElementIntegerSet(elem_hole_depth_to_type, PRO_HLE_STRGHT_BLIND_DEPTH);
	status = ProElemtreeElementAdd(elem_hole_depth_to, NULL, elem_hole_depth_to_type);
	status = ProElementAlloc(PRO_E_EXT_DEPTH_TO_VALUE, &elem_hole_depth_to_value);
	status = ProElementDoubleSet(elem_hole_depth_to_value, 25.0);
	status = ProElemtreeElementAdd(elem_hole_depth_to, NULL, elem_hole_depth_to_value);

	// side2 ���F Blind �[��25
	status = ProElementAlloc(PRO_E_HOLE_DEPTH_FROM, &elem_hole_depth_from);
	status = ProElemtreeElementAdd(elem_hole_std_depth, NULL, elem_hole_depth_from);
	status = ProElementAlloc(PRO_E_HOLE_DEPTH_FROM_TYPE, &elem_hole_depth_from_type);
	status = ProElementIntegerSet(elem_hole_depth_from_type, PRO_HLE_STRGHT_BLIND_DEPTH);
	status = ProElemtreeElementAdd(elem_hole_depth_from, NULL, elem_hole_depth_from_type);
	status = ProElementAlloc(PRO_E_EXT_DEPTH_FROM_VALUE, &elem_hole_depth_from_value);
	status = ProElementDoubleSet(elem_hole_depth_from_value, 25.0);
	status = ProElemtreeElementAdd(elem_hole_depth_from, NULL, elem_hole_depth_from_value);

	/* �z�u�̏ڍׂɊ֘A����v�f��ǉ�����
	 �������Ƃ��铯����

		 |--PRO_E_HLE_PLACEMENT
		 |    |--PRO_E_HLE_PRIM_REF
		 |    |--PRO_E_HLE_PL_TYPE
		 |    |--PRO_E_HLE_DIM_REF1
		 |    |--PRO_E_HLE_DIM_DIST1
		 |    |--PRO_E_HLE_DIM_DIST2
		 |    |--PRO_E_LIN_HOLE_DIR_REF

	*/

	ProAsmcomppath compPath;
	status = getAsmcompathIncludePart(top_asm, part, &compPath);
	if (status != PRO_TK_NO_ERROR) {
		// �p�[�c(patName)�̃t�B�[�`����������Ȃ�����
		return (PRO_TK_E_NOT_FOUND);
	}

	status = ProElementAlloc(PRO_E_HLE_PLACEMENT, &elem_hle_placement);
	status = ProElemtreeElementAdd(feat_elemtree, NULL, elem_hle_placement);

	// �ꎟ�I�� (��������)
	ProModelitem mdlitem;
	ProSelection pSel;

	if (selSurf == NULL) {
		ProFeatureGeomitemVisit(&dtmPlnFeature_selSurf, PRO_SURFACE, UsrPointAddAction, NULL, (ProAppData)&mdlitem);
		status = ProSelectionAlloc(NULL, &mdlitem, &pSel);
		status = ProElementAlloc(PRO_E_HLE_PRIM_REF, &elem_hle_prim_ref);

		value_data.type = PRO_VALUE_TYPE_SELECTION;
		value_data.v.r = pSel;
	}
	else {
		status = ProElementAlloc(PRO_E_HLE_PRIM_REF, &elem_hle_prim_ref);

		value_data.type = PRO_VALUE_TYPE_SELECTION;
		value_data.v.r = selSurf;
	}
	status = ProValueAlloc(&value);
	status = ProValueDataSet(value, &value_data);
	status = ProElementValueSet(elem_hle_prim_ref, value);
	status = ProElemtreeElementAdd(elem_hle_placement, NULL, elem_hle_prim_ref);

	// ���z�u�I�v�V����
	status = ProElementAlloc(PRO_E_HLE_PL_TYPE, &elem_hle_pl_type);
	status = ProElementIntegerSet(elem_hle_pl_type, PRO_HLE_PL_TYPE_LIN);
	status = ProElemtreeElementAdd(elem_hle_placement, NULL, elem_hle_pl_type);

	// �񎟑I�� (��1�̃f�[�^����)
	ProModelitem mdlitem2;
	ProSelection pSel2;

	ProFeatureGeomitemVisit(&holeFeatureOne, PRO_AXIS, UsrPointAddAction, NULL, (ProAppData)&mdlitem2);
	status = ProSelectionAlloc(&compPath, &mdlitem2, &pSel2);

	status = ProElementAlloc(PRO_E_HLE_DIM_REF1, &elem_hle_dim_ref1);
	value_data.type = PRO_VALUE_TYPE_SELECTION;
	value_data.v.r = pSel2;
	status = ProValueAlloc(&value);
	status = ProValueDataSet(value, &value_data);
	status = ProElementValueSet(elem_hle_dim_ref1, value);
	status = ProElemtreeElementAdd(elem_hle_placement, NULL, elem_hle_dim_ref1);

	// ���� 1
	status = ProElementAlloc(PRO_E_HLE_DIM_DIST1, &elem_hle_dim_dist1);
	status = ProElementDoubleSet(elem_hle_dim_dist1, distX);
	status = ProElemtreeElementAdd(elem_hle_placement, NULL, elem_hle_dim_dist1);

	// PRO_E_HLE_DIM_REF1 ���f�[�^�����̂��߁APRO_E_HLE_DIM_REF2 �̐ݒ�Ȃ�

	// ���� 2
	status = ProElementAlloc(PRO_E_HLE_DIM_DIST2, &elem_hle_dim_dist2);
	status = ProElementDoubleSet(elem_hle_dim_dist2, distY);
	status = ProElemtreeElementAdd(elem_hle_placement, NULL, elem_hle_dim_dist2);

	// �z�u���@�X�L�[���̕������`
	ProModelitem mdlitem3;
	ProSelection pSel3;
	ProFeatureGeomitemVisit(&dtmPlnFeature_YZ, PRO_SURFACE, UsrPointAddAction, NULL, (ProAppData)&mdlitem3);
	status = ProSelectionAlloc(NULL, &mdlitem3, &pSel3);
	status = ProElementAlloc(PRO_E_LIN_HOLE_DIR_REF, &lin_hole_dir_ref);

	value_data.type = PRO_VALUE_TYPE_SELECTION;
	value_data.v.r = pSel3;
	status = ProValueAlloc(&value);
	status = ProValueDataSet(value, &value_data);
	status = ProElementValueSet(lin_hole_dir_ref, value);
	status = ProElemtreeElementAdd(elem_hle_placement, NULL, lin_hole_dir_ref);
	/* �v�f�c���[�쐬�̏I�� */

	/***********************************************
	 �t�B�[�`���[�̍쐬
	*************************************************/
	status = ProMdlToModelitem(part, &model_item);
	status = ProSelectionAlloc(&compPath, &model_item, &model_selection);
	status = ProArrayAlloc(1, sizeof(ProFeatureCreateOptions), 1, (ProArray*)&options);
	options[0] = PRO_FEAT_CR_NO_OPTS;
	status = ProFeatureWithoptionsCreate(model_selection, feat_elemtree, options, PRO_REGEN_NO_FLAGS, holeFeature, &p_errors);
	TRAIL_PRINT("%s(%d) : ProFeatureWithoptionsCreate = %s", __func__, __LINE__, getProErrorMessage(status));

	/***********************************************
	 ���\�[�X�̉��
	*************************************************/
    status = ProElementFree(&feat_elemtree);
	status = ProSelectionFree(&model_selection);
	status = ProArrayFree((ProArray*)&options);

	return (status);
}



/*====================================================================*\
FUNCTION : createHoleBaseHoleOneOther
PURPOSE  : ��1����ɁA��1�ƈႤ�ʂɌ����쐬���� (��2�ȍ~�̍쐬)
	ProMdl top_asm,					in top�A�Z���u��
	ProMdl part						in ����������p�[�c
	ProFeature holeFeatureOne,		in ��ƂȂ錊1�̃t�B�[�`��
	ProSelection selOffsetSurf,		in �I�t�Z�b�g�t�B�[�`��
	ProSelection selSurf			in �z�u��/�����������
	double distX,					in ��1����̋���(X��)
	double distY					in ��1����̋���(Y��)
	double dDiameter				in �����a
	ProFeature* holeFeature			out ���t�B�[�`��
\*====================================================================*/
ProError createHoleBaseHoleOneOther(ProMdl top_asm,
	ProMdl part,
	ProFeature holeFeatureOne,
	ProFeature selOffsetSurf,
	ProSelection selSurf,
	double distX,
	double distY,
	double dDiameter,
	ProFeature* holeFeature)
{
	ProError status;

	ProElement feat_elemtree;
	ProElement elem_feattype;
	ProElement elem_featform;
	ProElement elem_hle_com;
	ProElement elem_hle_type_new;
	ProElement elem_hle_stan_type;
	ProElement elem_diameter;
	ProElement elem_hole_std_depth;
	ProElement elem_hole_depth_to;
	ProElement elem_hole_depth_to_type;
	ProElement elem_hole_depth_to_value;
	ProElement elem_hole_depth_from;
	ProElement elem_hole_depth_from_type;
	ProElement elem_hole_depth_from_value;
	ProElement elem_hle_placement;
	ProElement elem_hle_prim_ref;
	ProElement elem_hle_pl_type;
	ProElement elem_hle_dim_ref1;
	ProElement elem_hle_dim_dist1;
	ProElement elem_hle_dim_ref2;
	ProElement elem_hle_dim_dist2;
	ProElement lin_hole_dir_ref;
	ProValue value;
	ProValueData value_data;
	ProSelection* p_selection;
	int n_selection;

	ProFeatureCreateOptions* options = 0;
	ProFeature created_feature;
	ProErrorlist p_errors;
	//ProMdl model;
	ProModelitem model_item;
	ProSelection model_selection;
	ProReference reference;


	/***********************************************
	 �v�f�c���[�쐬�̊J�n
	*************************************************/
	/* �v�f�c���[(���[�g�v�f)�̒ǉ� */
	status = ProElementAlloc(PRO_E_FEATURE_TREE, &feat_elemtree);

	/***********************************************
	 PRO_E_FEATURE_TYPE
	*************************************************/
	/* �v�f�c���[�� �t�B�[�`���^�C�v(��) ��ǉ����� */
	status = ProElementAlloc(PRO_E_FEATURE_TYPE, &elem_feattype);
	status = ProElementIntegerSet(elem_feattype, PRO_FEAT_HOLE);
	status = ProElemtreeElementAdd(feat_elemtree, NULL, elem_feattype);

	/***********************************************
	 PRO_E_FEATURE_FORM
	*************************************************/
	/* �v�f�c���[�� �t�B�[�`���t�H�[��(�X�g���[�g�z�[��) ��ǉ����� */
	status = ProElementAlloc(PRO_E_FEATURE_FORM, &elem_featform);
	status = ProElementIntegerSet(elem_featform, PRO_HLE_TYPE_STRAIGHT);
	status = ProElemtreeElementAdd(feat_elemtree, NULL, elem_featform);

	/***********************************************
	 PRO_E_HLE_COM
	*************************************************/
	/* �v�f�c���[�� �����̋��ʗv�f �̒ǉ�  */
	status = ProElementAlloc(PRO_E_HLE_COM, &elem_hle_com);
	status = ProElemtreeElementAdd(feat_elemtree, NULL, elem_hle_com);

	/* ����� (���^�C�v:�X�g���[�g) ���w�肷�� */
	status = ProElementAlloc(PRO_E_HLE_TYPE_NEW, &elem_hle_type_new);
	status = ProElementIntegerSet(elem_hle_type_new, PRO_HLE_NEW_TYPE_STRAIGHT);
	status = ProElemtreeElementAdd(elem_hle_com, NULL, elem_hle_type_new);

	/* ����� (���̒��a) ���w�肷�� */
	status = ProElementAlloc(PRO_E_DIAMETER, &elem_diameter);
	status = ProElementDoubleSet(elem_diameter, dDiameter);
	status = ProElemtreeElementAdd(elem_hle_com, NULL, elem_diameter);

	/* �W���[�x�̗v�f��ǉ�����

			  |--PRO_E_HOLE_STD_DEPTH
			  |    |--PRO_E_HOLE_DEPTH_TO
			  |    |    |--PRO_E_HOLE_DEPTH_TO_TYPE
			  |    |    |--PRO_E_EXT_DEPTH_TO_VALUE
			  |    |--PRO_E_HOLE_DEPTH_FROM
			  |         |--PRO_E_HOLE_DEPTH_FROM_TYPE
			  |         |--PRO_E_EXT_DEPTH_FROM_VALUE
	*/

	// �[���v�f
	status = ProElementAlloc(PRO_E_HOLE_STD_DEPTH, &elem_hole_std_depth);
	status = ProElemtreeElementAdd(elem_hle_com, NULL, elem_hole_std_depth);

	// side1 ���F Blind �[��25
	status = ProElementAlloc(PRO_E_HOLE_DEPTH_TO, &elem_hole_depth_to);
	status = ProElemtreeElementAdd(elem_hole_std_depth, NULL, elem_hole_depth_to);
	status = ProElementAlloc(PRO_E_HOLE_DEPTH_TO_TYPE, &elem_hole_depth_to_type);
	status = ProElementIntegerSet(elem_hole_depth_to_type, PRO_HLE_STRGHT_BLIND_DEPTH);
	status = ProElemtreeElementAdd(elem_hole_depth_to, NULL, elem_hole_depth_to_type);
	status = ProElementAlloc(PRO_E_EXT_DEPTH_TO_VALUE, &elem_hole_depth_to_value);
	status = ProElementDoubleSet(elem_hole_depth_to_value, 25.0);
	status = ProElemtreeElementAdd(elem_hole_depth_to, NULL, elem_hole_depth_to_value);

	// side2 ���F Blind �[��25
	status = ProElementAlloc(PRO_E_HOLE_DEPTH_FROM, &elem_hole_depth_from);
	status = ProElemtreeElementAdd(elem_hole_std_depth, NULL, elem_hole_depth_from);
	status = ProElementAlloc(PRO_E_HOLE_DEPTH_FROM_TYPE, &elem_hole_depth_from_type);
	status = ProElementIntegerSet(elem_hole_depth_from_type, PRO_HLE_STRGHT_BLIND_DEPTH);
	status = ProElemtreeElementAdd(elem_hole_depth_from, NULL, elem_hole_depth_from_type);
	status = ProElementAlloc(PRO_E_EXT_DEPTH_FROM_VALUE, &elem_hole_depth_from_value);
	status = ProElementDoubleSet(elem_hole_depth_from_value, 25.0);
	status = ProElemtreeElementAdd(elem_hole_depth_from, NULL, elem_hole_depth_from_value);


	/* �z�u�̏ڍׂɊ֘A����v�f��ǉ�����

	 �������Ƃ��铯����

		 |--PRO_E_HLE_PLACEMENT
		 |    |--PRO_E_HLE_PRIM_REF
		 |    |--PRO_E_HLE_PL_TYPE
		 |    |--PRO_E_HLE_DIM_REF1
		 |    |--PRO_E_HLE_DIM_DIST1
		 |    |--PRO_E_HLE_DIM_DIST2
		 |    |--PRO_E_LIN_HOLE_DIR_REF

	*/

	ProAsmcomppath compPath;
	status = getAsmcompathIncludePart(top_asm, part, &compPath);
	if (status != PRO_TK_NO_ERROR) {
		// �p�[�c(patName)�̃t�B�[�`����������Ȃ�����
		return (PRO_TK_E_NOT_FOUND);
	}

	status = ProElementAlloc(PRO_E_HLE_PLACEMENT, &elem_hle_placement);
	status = ProElemtreeElementAdd(feat_elemtree, NULL, elem_hle_placement);

	// �ꎟ�I�� (��������)
	status = ProElementAlloc(PRO_E_HLE_PRIM_REF, &elem_hle_prim_ref);

	value_data.type = PRO_VALUE_TYPE_SELECTION;
	value_data.v.r = selSurf;

	status = ProValueAlloc(&value);
	status = ProValueDataSet(value, &value_data);
	status = ProElementValueSet(elem_hle_prim_ref, value);
	status = ProElemtreeElementAdd(elem_hle_placement, NULL, elem_hle_prim_ref);

	// ���z�u�I�v�V����
	status = ProElementAlloc(PRO_E_HLE_PL_TYPE, &elem_hle_pl_type);
	status = ProElementIntegerSet(elem_hle_pl_type, PRO_HLE_PL_TYPE_LIN);
	status = ProElemtreeElementAdd(elem_hle_placement, NULL, elem_hle_pl_type);

	// �񎟑I�� (��1�̃f�[�^����)
	ProModelitem mdlitem2;
	ProSelection pSel2;

	ProFeatureGeomitemVisit(&holeFeatureOne, PRO_AXIS, UsrPointAddAction, NULL, (ProAppData)&mdlitem2);
	status = ProSelectionAlloc(&compPath, &mdlitem2, &pSel2);
	status = ProElementAlloc(PRO_E_HLE_DIM_REF1, &elem_hle_dim_ref1);

	value_data.type = PRO_VALUE_TYPE_SELECTION;
	value_data.v.r = pSel2;

	status = ProValueAlloc(&value);
	status = ProValueDataSet(value, &value_data);
	status = ProElementValueSet(elem_hle_dim_ref1, value);
	status = ProElemtreeElementAdd(elem_hle_placement, NULL, elem_hle_dim_ref1);

	// ���� 1
	status = ProElementAlloc(PRO_E_HLE_DIM_DIST1, &elem_hle_dim_dist1);
	status = ProElementDoubleSet(elem_hle_dim_dist1, distX);
	status = ProElemtreeElementAdd(elem_hle_placement, NULL, elem_hle_dim_dist1);

	// �O���I�� (��1�̃f�[�^�����Ɠ����T�[�t�F�C�X��)
	ProModelitem mdlitemZ;
	ProSelection pSelZ;

	ProFeatureGeomitemVisit(&selOffsetSurf, PRO_SURFACE, UsrPointAddAction, NULL, (ProAppData)&mdlitemZ);

	status = ProSelectionAlloc(NULL, &mdlitemZ, &pSelZ);
	status = ProElementAlloc(PRO_E_HLE_DIM_REF2, &elem_hle_dim_ref2);

	value_data.type = PRO_VALUE_TYPE_SELECTION;
	value_data.v.r = pSelZ;

	status = ProValueAlloc(&value);
	status = ProValueDataSet(value, &value_data);
	status = ProElementValueSet(elem_hle_dim_ref2, value);
	status = ProElemtreeElementAdd(elem_hle_placement, NULL, elem_hle_dim_ref2);

	// ���� 2
	status = ProElementAlloc(PRO_E_HLE_DIM_DIST2, &elem_hle_dim_dist2);
	status = ProElementDoubleSet(elem_hle_dim_dist2, distY);
	status = ProElemtreeElementAdd(elem_hle_placement, NULL, elem_hle_dim_dist2);

	/* �v�f�c���[�쐬�̏I�� */

	/***********************************************
	 �t�B�[�`���[�̍쐬
	*************************************************/
	status = ProMdlToModelitem(part, &model_item);
	status = ProSelectionAlloc(&compPath, &model_item, &model_selection);
	status = ProArrayAlloc(1, sizeof(ProFeatureCreateOptions), 1, (ProArray*)&options);
	options[0] = PRO_FEAT_CR_NO_OPTS;
	status = ProFeatureWithoptionsCreate(model_selection, feat_elemtree, options, PRO_REGEN_NO_FLAGS, holeFeature, &p_errors);
	TRAIL_PRINT("%s(%d) : ProFeatureWithoptionsCreate = %s", __func__, __LINE__, getProErrorMessage(status));

	/***********************************************
	 ���\�[�X�̉��
	*************************************************/
	status = ProElementFree(&feat_elemtree);
	status = ProSelectionFree(&model_selection);
	status = ProArrayFree((ProArray*)&options);

	return (status);
}


/*=============================================================*\
  Function: 	getHoleTableInfOfSameNum
  Purpose:		HoleTable �̏��擾
  InputFileHole* strHole,					in	�����Ώۂ̌����
  InputFileHoleTable* strHoleTableInput		in	�R���t�B�O���[�V�����t�@�C���̒l
  int iHoleTableSectionMaxRows				in  �������ׂ��R���t�B�O���[�V�����t�@�C���̍s��(Hole_Table)
  int* iHoleTableCnt						out	�������ʂ̍ő�s��
  \*=============================================================*/
ProError getHoleTableInfOfSameNum(InputFileHole* strHole, InputFileHoleTable* strHoleTableInput, int iHoleTableSectionMaxRows, int* iHoleTableCnt)
{
	char cHoleID[INPUTFILE_MAXLINE];
	// �J�n�n�_�̃A�h���X���m��
	InputFileHoleTable* HoleTableStart = strHoleTableInput;
	InputFileHoleTable* staticHoleTableStart;
	/*--------------------------------------------------------------------*\
		���O���[�v�̐����J�E���g����
	\*--------------------------------------------------------------------*/
	for (int iInputMdlCnt = 0; iInputMdlCnt < iHoleTableSectionMaxRows; iInputMdlCnt++) {

		// Hole��HoleTable���r���A���O���[�v�̐����J�E���g
		if (strcmp(strHole->cHoleGroupName, strHoleTableInput->cHoleGroupName) == 0) {
			*iHoleTableCnt = *iHoleTableCnt + 1;
		}
		*strHoleTableInput++;
	}
	if (*iHoleTableCnt == 0) {
		// �Ώۂ�HoleTable��������܂���ł����B
		LOG_PRINT("NOK : %s : Not found HoleGroup in HoleTable", strHole->cHoleGroupName);
		return PRO_TK_E_NOT_FOUND;
	}

	/*--------------------------------------------------------------------*\
		���̐��������������m�ۂ���
	\*--------------------------------------------------------------------*/
	staticHoleTable = (InputFileHoleTable*)calloc(*iHoleTableCnt, sizeof(InputFileHoleTable));
	if (!staticHoleTable) {
		// �������s��
		LOG_PRINT("NOK : Not enough memory");
		return PRO_TK_GENERAL_ERROR;
	}

	/*--------------------------------------------------------------------*\
		���O���[�v�p�̃��������m�ۂ���
	\*--------------------------------------------------------------------*/
	staticHoleGroupAll = (ProFeature*)calloc(*iHoleTableCnt, sizeof(ProFeature));
	if (!staticHoleGroupAll) {
		// �������s��
		LOG_PRINT("NOK : Not enough memory");
		return PRO_TK_GENERAL_ERROR;
	}


	staticHoleTableStart = staticHoleTable;
	strHoleTableInput = HoleTableStart;
	int iRow = 0;
	/*--------------------------------------------------------------------*\
		HoleTable�̏����i�[����
	\*--------------------------------------------------------------------*/
	for (int iInputMdlCnt = 0; iInputMdlCnt < iHoleTableSectionMaxRows; iInputMdlCnt++) {
		// Hole��HoleTable���r���A���O���[�v�������s������
		if (strcmp(strHole->cHoleGroupName, strHoleTableInput->cHoleGroupName) == 0) {

			if(iRow>0){
				// 2��ڂ̃��[�v����J�E���g����
				*staticHoleTable++;
			}
			strncpy(staticHoleTable->cFrameDiameter, strHoleTableInput->cFrameDiameter, sizeof(strHoleTableInput->cFrameDiameter));
			strncpy(staticHoleTable->cHoleGroupName, strHoleTableInput->cHoleGroupName, sizeof(strHoleTableInput->cHoleGroupName));
			strncpy(staticHoleTable->cHoleID, strHoleTableInput->cHoleID, sizeof(strHoleTableInput->cHoleID));
			strncpy(staticHoleTable->cInnerLineDiameter, strHoleTableInput->cInnerLineDiameter, sizeof(strHoleTableInput->cInnerLineDiameter));
			strncpy(staticHoleTable->cNA1, strHoleTableInput->cNA1, sizeof(strHoleTableInput->cNA1));
			strncpy(staticHoleTable->cHoleFlag, strHoleTableInput->cHoleFlag, sizeof(strHoleTableInput->cHoleFlag));
			strncpy(staticHoleTable->cXCord, strHoleTableInput->cXCord, sizeof(strHoleTableInput->cXCord));
			strncpy(staticHoleTable->cYCord, strHoleTableInput->cYCord, sizeof(strHoleTableInput->cYCord));
			strncpy(staticHoleTable->cZCord, strHoleTableInput->cZCord, sizeof(strHoleTableInput->cZCord));
			
			iRow++;
		}
		*strHoleTableInput++;
	}
	// �������̈ʒu��擪�ɖ߂�
	staticHoleTable = staticHoleTableStart;
	strHoleTableInput = HoleTableStart;
	return PRO_TK_NO_ERROR;
}

/*=============================================================*\
  Function: 	searchParameters
  Purpose:		�p�����[�^�̒l���������擾����
  ProName wParameterName	(in)	��������p�����[�^���̈ꕔ
  ProMdl mdlCurrent		(in)	�p�����[�^�������Ă��郂�f��
  double* dParam			(out)	��������
  \*=============================================================*/
ProError searchParameters(ProName wParameterName, ProMdl mdlCurrent, double* dParam){
	
	ProError status;
	ProModelitem featureParam;
	
	strSearchParameter searchParameter;
	searchParameter.dParam = 0;
	ProWstringCopy(wParameterName, searchParameter.wParameterName, PRO_VALUE_UNUSED);

	// �p�����[�^Modelitem�̎擾
	status = ProMdlToModelitem(mdlCurrent, &featureParam);

	// �p�����[�^�̌���
	status = ProParameterVisit(&featureParam, NULL, (ProParameterAction)getParametersValue, (ProAppData)&searchParameter);

	if (status == PRO_TK_CONTINUE) {
		*dParam = searchParameter.dParam;

	}
}

/*====================================================================*\
	FUNCTION :	 getParametersValue()
	PURPOSE  :   �p�����[�^�ꗗ����p�����[�^�l�̎擾
\*====================================================================*/
ProError getParametersValue(ProParameter* param, ProError err, ProAppData app_data)
{
	ProError status;
	ProCharName param_name;
	ProCharName cParameterName;
	ProParamvalue value;
	ProUnititem units;

	// �p�����[�^�̖��O�擾
	ProWstringToString(param_name, param->id);
	ProWstringToString(cParameterName, ((strSearchParameter*)app_data)->wParameterName);

	// �p�����[�^�����܂ރp�����[�^����������
	if (strstr(param_name, cParameterName) == NULL) {
		return PRO_TK_NO_ERROR;
	}

	// �p�����[�^���擾
	status = ProParameterValueWithUnitsGet(param, &value, &units);

	if (value.type == PRO_PARAM_DOUBLE) {
		((strSearchParameter*)app_data)->dParam = value.value.d_val;
	}

	return PRO_TK_CONTINUE;
}

/*====================================================================*\
FUNCTION: getSurfaceFromYpointAction()
PURPOSE:  Y���f�[�^�����ʂ̎Q�ƕ��ʂ̌���(�t���[������)
\*====================================================================*/
ProError getLowerSurfaceFromYpointAction(
	ProSurface surface,
	ProError status,
	ProAppData app_data)
{
	ProSrftype stype;
	ProGeomitemdata* pDataPtr;

	/*--------------------------------------------------------------------*\
		�T�[�t�F�X�����ʂłȂ��ꍇ�́A�X�L�b�v
	\*--------------------------------------------------------------------*/
	status = ProSurfaceTypeGet(surface, &stype);

	if (stype != PRO_SRF_PLANE) {
		return(PRO_TK_NO_ERROR);
	}

	/*--------------------------------------------------------------------*\
		�T�[�t�F�X�̏����擾
	\*--------------------------------------------------------------------*/
	ProVector xyz_point;
	ProVector normal;

	status = ProSurfaceDataGet(surface, &pDataPtr);

	status = ProSurfaceXyzdataEval(surface, pDataPtr->data.p_surface_data->uv_max,
		xyz_point, NULL, NULL, normal);

	/*--------------------------------------------------------------------*\
		Y���W���ł��������A����	�@���̒l�� (0, -1, 0) �ł���T�[�t�F�C�X���������A�l���擾����
	\*--------------------------------------------------------------------*/
	if(normal[0] == 0 && normal[1] == -1 && normal[2] == 0)
	{
		if (((SurfaceAppData*)app_data)->dYPoint > xyz_point[1]) {

			((SurfaceAppData*)app_data)->surface = surface;
			((SurfaceAppData*)app_data)->dYPoint = xyz_point[1];
		}
	}
	ProGeomitemdataFree(&pDataPtr);

	return(PRO_TK_NO_ERROR);
}

/*====================================================================*\
FUNCTION: getSurfaceFromYpointAction()
PURPOSE:  Y���f�[�^�����ʂ̎Q�ƕ��ʂ̌���(�t���[���㕔)
\*====================================================================*/
ProError getUpperSurfaceFromYpointAction(
	ProSurface surface,
	ProError status,
	ProAppData app_data)
{
	ProSrftype stype;
	ProGeomitemdata* pDataPtr;

	/*--------------------------------------------------------------------*\
		�T�[�t�F�X�����ʂłȂ��ꍇ�́A�X�L�b�v
	\*--------------------------------------------------------------------*/
	status = ProSurfaceTypeGet(surface, &stype);
	if (stype != PRO_SRF_PLANE) {
		return(PRO_TK_NO_ERROR);
	}

	/*--------------------------------------------------------------------*\
		�T�[�t�F�X�̏����擾
	\*--------------------------------------------------------------------*/
	ProVector xyz_point;
	ProVector normal;

	status = ProSurfaceDataGet(surface, &pDataPtr);
	status = ProSurfaceXyzdataEval(surface, pDataPtr->data.p_surface_data->uv_max,
		xyz_point, NULL, NULL, normal);

	/*--------------------------------------------------------------------*\
		Y���W���ł��傫���A����	�@���̒l�� (0, -1, 0) �ł���T�[�t�F�C�X���������A�l���擾����
	\*--------------------------------------------------------------------*/
	if (normal[0] == 0 && normal[1] == -1 && normal[2] == 0)
	{
		if (((SurfaceAppData*)app_data)->dYPoint < xyz_point[1]) {

			((SurfaceAppData*)app_data)->surface = surface;
			((SurfaceAppData*)app_data)->dYPoint = xyz_point[1];
		}
	}
	ProGeomitemdataFree(&pDataPtr);

	return(PRO_TK_NO_ERROR);
}


/*====================================================================*\
FUNCTION: getSurfZpointAction()
PURPOSE:  �������ʂ̋��E���W�̌���
\*====================================================================*/
ProError getSurfZpointAction(ProSurface surface,ProError status,ProAppData app_data)
{
	ProSrftype stype;
	ProGeomitemdata* pDataPtr;

	/*--------------------------------------------------------------------*\
		�T�[�t�F�X�����ʂłȂ��ꍇ�́A�X�L�b�v
	\*--------------------------------------------------------------------*/
	status = ProSurfaceTypeGet(surface, &stype);
	if (stype != PRO_SRF_PLANE) {
		return(PRO_TK_NO_ERROR);
	}

	if (((SurfaceInfoAppData*)app_data)->strFrontInfo.iCounter == (MAX_SURFACE * 2)) {
		// �m�ۂ����l�ȏ���擾���Ȃ��悤��
		return(PRO_TK_NO_ERROR);

	}

	/*--------------------------------------------------------------------*\
		�T�[�t�F�X�̏����擾
	\*--------------------------------------------------------------------*/
	ProVector xyz_point;
	ProVector normal;

	status = ProSurfaceDataGet(surface, &pDataPtr);
	status = ProSurfaceXyzdataEval(surface, pDataPtr->data.p_surface_data->uv_max,
		xyz_point, NULL, NULL, normal);

	/*--------------------------------------------------------------------*\
	�T�[�t�F�C�X�̕\�ʐς�100�����̏ꍇ�͑ΏۊO�Ƃ���
	���ΏۃT�[�t�F�C�X�̐؂蕪�����ł��Ȃ������̂ŁA����̍�
	�@100�����c4�䕪�̃g���b�N�f�[�^���m�F���Ēl�����߂��B10.5���傫��55255.9��菬�����l�ł���Ζ��Ȃ��͂��I
	\*--------------------------------------------------------------------*/
	// �\�ʐ�
	double dArea;
	ProSurfaceAreaEval(surface, &dArea);
	if (dArea < 100) {
		return(PRO_TK_NO_ERROR);
	}

	/*--------------------------------------------------------------------*\
		Left part�̏ꍇ�A�@����X�l�� 0 ���傫��
		Right part�̏ꍇ�A�@����X�l�� 0 ��菬����
	\*--------------------------------------------------------------------*/
	char temp[512];
	sprintf(temp, "%.3f", normal[0]);
	double hosen = atof(temp);

	if ((hosen > 0 && (((SurfaceInfoAppData*)app_data)->iLRFlag == LEFT_FRAME))
		|| (hosen < 0 && (((SurfaceInfoAppData*)app_data)->iLRFlag == RIGHT_FRAME)))
	{
		int iCnt = ((SurfaceInfoAppData*)app_data)->strFrontInfo.iCounter;
		double min = pDataPtr->data.p_surface_data->xyz_min[2];
		double max = pDataPtr->data.p_surface_data->xyz_max[2];

		// min/max���}�C�i�X�̏ꍇ�̓v���X�ɂ���
		if (min < 0) {
			min = min * -1;
		}
		if (max < 0) {
			max = max * -1;
		}

		if (max < min) {
			((SurfaceInfoAppData*)app_data)->strFrontInfo.dSurfZ[iCnt] = max;
			((SurfaceInfoAppData*)app_data)->strFrontInfo.dSurfZ[iCnt + 1] = min;

		}
		else {
			((SurfaceInfoAppData*)app_data)->strFrontInfo.dSurfZ[iCnt] = min;
			((SurfaceInfoAppData*)app_data)->strFrontInfo.dSurfZ[iCnt + 1] = max;

		}

		if (((SurfaceInfoAppData*)app_data)->strFrontInfo.surface[(iCnt + 2) / 2 - 1] == NULL) {
			((SurfaceInfoAppData*)app_data)->strFrontInfo.surface[(iCnt + 2) / 2 - 1] = surface;
		}

		if (((SurfaceInfoAppData*)app_data)->dSurfY[(iCnt + 2) / 2 - 1] == 0) {
			((SurfaceInfoAppData*)app_data)->dSurfY[(iCnt + 2) / 2 - 1] = pDataPtr->data.p_surface_data->xyz_min[1];

		}

		((SurfaceInfoAppData*)app_data)->strFrontInfo.iCounter = iCnt + 2;

	}
	ProGeomitemdataFree(&pDataPtr);

	return(PRO_TK_NO_ERROR);
}

/*====================================================================*\
FUNCTION: searchBackSurfAction()
PURPOSE:  �������ʂ̗��ʂ̋��E���W�̌���
\*====================================================================*/
ProError searchBackSurfAction(ProSurface surface, ProError status, ProAppData app_data)
{
	ProSrftype stype;
	ProGeomitemdata* pDataPtr;

	/*--------------------------------------------------------------------*\
		�T�[�t�F�X�����ʂłȂ��ꍇ�́A�X�L�b�v
	\*--------------------------------------------------------------------*/
	status = ProSurfaceTypeGet(surface, &stype);
	if (stype != PRO_SRF_PLANE) {
		return(PRO_TK_NO_ERROR);
	}

	/*--------------------------------------------------------------------*\
		�T�[�t�F�X�̏����擾
	\*--------------------------------------------------------------------*/
	ProVector xyz_point;
	ProVector normal;

	status = ProSurfaceDataGet(surface, &pDataPtr);
	status = ProSurfaceXyzdataEval(surface, pDataPtr->data.p_surface_data->uv_max,
		xyz_point, NULL, NULL, normal);

	char temp[512];
	sprintf(temp, "%.3f", normal[0]);
	double hosen = atof(temp);

	if ((hosen < 0 && (((SurfaceInfoAppData*)app_data)->iLRFlag == LEFT_FRAME))
		|| (hosen > 0 && (((SurfaceInfoAppData*)app_data)->iLRFlag == RIGHT_FRAME)))
	{

		int iCnt = ((SurfaceInfoAppData*)app_data)->strBackInfo.iCounter;
		/*--------------------------------------------------------------------*\
		 �� �t���[���̕\�ʂ̃T�[�t�F�C�X�����擾
				Left part�̏ꍇ�A�@����X�l�� 0 ��菬����
				Right part�̏ꍇ�A�@����X�l�� 0 ���傫��
		\*--------------------------------------------------------------------*/
		if (iCnt == (MAX_SURFACE * 2)) {
			// �m�ۂ����l�ȏ���擾���Ȃ��悤��
			return(PRO_TK_NO_ERROR);
		}

		double dYPointMin = pDataPtr->data.p_surface_data->xyz_min[1];

		/*--------------------------------------------------------------------*\
		 �� �t���[���̕\�ʂ̃T�[�t�F�C�X�����擾
				�\�ʂƓ���Y���W������
		\*--------------------------------------------------------------------*/
		if (iCnt == 0) {
			if (((SurfaceInfoAppData*)app_data)->dSurfY[iCnt] != dYPointMin) {
				// �\�ʂ�Y���W���قȂ�ꍇ��NG
				return(PRO_TK_NO_ERROR);
			}
		}
		else {
			if (((SurfaceInfoAppData*)app_data)->dSurfY[iCnt / 2] != dYPointMin) {
				// �\�ʂ�Y���W���قȂ�ꍇ��NG
				return(PRO_TK_NO_ERROR);
			}
		}

		double min = pDataPtr->data.p_surface_data->xyz_min[2];
		double max = pDataPtr->data.p_surface_data->xyz_max[2];

		// min/max���}�C�i�X�̏ꍇ�̓v���X�ɂ���
		if (min < 0) {
			min = min * -1;
		}
		if (max < 0) {
			max = max * -1;
		}

		if (max < min) {
			((SurfaceInfoAppData*)app_data)->strBackInfo.dSurfZ[iCnt] = max;
			((SurfaceInfoAppData*)app_data)->strBackInfo.dSurfZ[iCnt + 1] = min;

		}
		else {
			((SurfaceInfoAppData*)app_data)->strBackInfo.dSurfZ[iCnt] = min;
			((SurfaceInfoAppData*)app_data)->strBackInfo.dSurfZ[iCnt + 1] = max;

		}

		((SurfaceInfoAppData*)app_data)->strBackInfo.iCounter = iCnt + 2;
		// �T�[�t�F�C�X���̎擾
		((SurfaceInfoAppData*)app_data)->strBackInfo.surface[((SurfaceInfoAppData*)app_data)->strBackInfo.iCounter / 2 - 1] = surface;
	}

	ProGeomitemdataFree(&pDataPtr);
	return(PRO_TK_NO_ERROR);
}



/*====================================================================*\
FUNCTION: getAsmcompathIncludePart()
PURPOSE:  part �����܂� ProAsmcomppath ���擾����
ProMdl top_asm					in		TOP�A�Z���u�����
ProMdl part						in		part���
ProAsmcomppath* compPath		out		�擾����ProAsmcomppath
\*====================================================================*/
ProError getAsmcompathIncludePart(ProMdl top_asm, ProMdl part, ProAsmcomppath* compPath) {
	ProError status = PRO_TK_NO_ERROR;
	ProPath patName;
	DatumAppData	appdataFeature;
	// ��������p�[�c�̖��O��ݒ�

	ProMdlMdlnameGet(part, patName);
	ProWstringCopy(patName, appdataFeature.name, PRO_VALUE_UNUSED);
	// ����������
	appdataFeature.iFindCnt = 0;

	// Top�p�[�c�̃p�[�c(patName)�̃t�B�[�`��������
	ProSolidFeatVisit((ProSolid)top_asm, getFeatureIdAction, NULL, (ProAppData)&appdataFeature);

	if (appdataFeature.iFindCnt == 0) {
		// �p�[�c(patName)�̃t�B�[�`����������Ȃ�����
		return (PRO_TK_E_NOT_FOUND);
	}

	ProIdTable idTable;
	idTable[0] = appdataFeature.feature.id;

	// ProAsmcompath�f�[�^�\���̏�����
	// top�A�Z���u�����猩�āA1�K�w���̃p�[�c�ł��邱�Ƃ��`
	status = ProAsmcomppathInit((ProSolid)top_asm, idTable, 1, compPath);

	return status;
}

/*====================================================*\
  Function : convertGroupToFeature()
  Purpose  : ProGroup��Feature�ɕϊ�����
\*====================================================*/
ProError convertGroupToFeature(ProFeature* pFeature, ProError status, ProAppData app_data)
{
	// �t�B�[�`�����O���[�v�ɕϊ�
	ProGroup		localGroup;
	ProName			wFeatureName;
	ProCharName		cFeatureName;
	ProCharName		cName;

	status = ProFeatureGroupGet(pFeature, &localGroup);

	if (localGroup.type == ((strGroupToFeature*)app_data)->group.type
		&& localGroup.id == ((strGroupToFeature*)app_data)->group.id
		&& localGroup.owner == ((strGroupToFeature*)app_data)->group.owner) {

		// Group���̂��̂܂Ńq�b�g����̂ŁA�O���[�v���Ńt�B���^�[����
		status = ProModelitemNameGet(pFeature, wFeatureName);
		ProWstringToString(cFeatureName, wFeatureName);
		ProWstringToString(cName, ((strGroupToFeature*)app_data)->wName);

		if (strcmp(cFeatureName, "") != NULL &&
			strstr(cFeatureName, cName) != NULL) {
			((strGroupToFeature*)app_data)->feature = *pFeature;
			return PRO_TK_NO_ERROR;

		}

	}

	return PRO_TK_CONTINUE;
}

/*=========================================================================*\
	Function:	CsysNameFindFilterAction()
	Purpose:	CSYS�����t�B���^�[
\*=========================================================================*/
ProError CsysNameFindFilterAction(ProFeature* p_feature, ProAppData app_data)
{
	ProError    status;
	ProFeattype ftype;
	ProName wName;
	int iResult;

	// �t�B�[�`���^�C�v���擾 (�f�[�^��, CSYS, �R���|�[�l���g ...)
	status = ProFeatureTypeGet(p_feature, &ftype);
	if (status == PRO_TK_NO_ERROR && ftype == PRO_FEAT_CSYS)
	{
		status = ProModelitemNameGet(p_feature, wName);

		ProWstringCompare(((UserCsysAppData*)app_data)->csys_name, wName, PRO_VALUE_UNUSED, &iResult);
		if (iResult == 0) {
			return(PRO_TK_NO_ERROR);
		}
	}

	return(PRO_TK_CONTINUE);
}


/*=========================================================================*\
	Function:	getSurfaceInfo()
	Purpose:	�������p�[�c�̃T�[�t�F�C�X�����擾
	SurfaceInfoAppData surfInfoData (out)	�T�[�t�F�C�X���
	int iLRFlag						(in)	�p�[�c����(���E)
	ProMdl mdlPart					(in)	�T�[�t�F�C�X���̎擾����p�[�c	
\*=========================================================================*/
ProError getSurfaceInfo(SurfaceInfoAppData* surfInfoData , int iLRFlag, ProMdl mdlPart) {
	// �l�̏�����

	surfInfoData->iLRFlag = iLRFlag;

	// �������p�[�c�̃T�[�t�F�C�X�����擾
	ProSolidSurfaceVisit(ProMdlToSolid(mdlPart),
		(ProSurfaceVisitAction)getSurfZpointAction,
		(ProSurfaceFilterAction)NULL,
		(ProAppData)surfInfoData);

	// �������p�[�c�̗��ʂ̃T�[�t�F�C�X�����擾
	ProSolidSurfaceVisit(ProMdlToSolid(mdlPart),
		(ProSurfaceVisitAction)searchBackSurfAction,
		(ProSurfaceFilterAction)NULL,
		(ProAppData)surfInfoData);
}


/*=========================================================================*\
	Function:	getXDatumSurfaceInfo()
	Purpose:	�\�ʂ������ɕ�����Ă���ꍇ������̂ŁA
				�f�[�^������X�̍��W �� ����X �̍��W����Ώۖʂ��擾����
SurfaceInfoAppData* surfInfoData	(in)�@�T�[�t�F�C�X���
InputFileHole* localHole			(in)  �R���t�B�O���[�V�����t�@�C���̏��
ProMdl* top_asm						(in)�@�g�b�v�A�Z���u��
ProSurface* dtmplnModelitem			(out)�@ProSurface
\*=========================================================================*/
ProError getXDatumSurfaceInfo(MainSurfaceInfo* surfInfoData, InputFileHole* localHole, ProMdl* top_asm, ProSurface* dtmplnModelitem) {

	ProError status;

	/*--------------------------------------------------------------------*\
		�f�[�^������X �̃T�[�t�F�C�X�Ƃ��擾
	\*--------------------------------------------------------------------*/
	DatumAppData	appdataFeature;
	int iDatumType = 0;
	iDatumType = atoi(localHole->cDatumType);

	if (iDatumType == 0) {
		// ��������f�[�^�����ʂ̖��O��ݒ�
		ProWstringCopy(X_DATUM_NAME_0, appdataFeature.name, PRO_VALUE_UNUSED);

	}
	else if (iDatumType == 1) {
		// ��������f�[�^�����ʂ̖��O��ݒ�
		ProWstringCopy(X_DATUM_NAME_1, appdataFeature.name, PRO_VALUE_UNUSED);

	}
	else if (iDatumType == 2) {
		// ��������f�[�^�����ʂ̖��O��ݒ�
		ProWstringCopy(X_DATUM_NAME_2, appdataFeature.name, PRO_VALUE_UNUSED);

	}
	else if (iDatumType == 3) {
		// ��������f�[�^�����ʂ̖��O��ݒ�
		ProWstringCopy(X_DATUM_NAME_3, appdataFeature.name, PRO_VALUE_UNUSED);

	}
	else if (iDatumType == 4) {
		// ��������f�[�^�����ʂ̖��O��ݒ�
		ProWstringCopy(X_DATUM_NAME_4, appdataFeature.name, PRO_VALUE_UNUSED);

	}
	else {
		// X���W�̊�ƂȂ�f�[�^�����ʂ��z��O�̒l
		return PRO_TK_BAD_INPUTS;
	}

	// ����������
	appdataFeature.iFindCnt = 0;

	// Top�p�[�c�̃f�[�^������(X_DATUM_NAME)�̃t�B�[�`��������
	ProSolidFeatVisit((ProSolid)*top_asm, getFeatureIdAction, NULL, (ProAppData)&appdataFeature);

	if (appdataFeature.iFindCnt == 0) {
		// �f�[�^������(X_DATUM_NAME)�̃t�B�[�`����������Ȃ�����
		return (PRO_TK_E_NOT_FOUND);
	}

	// �t�B�[�`���� PRO_SURFACE ��ProModelitem�ɕϊ�����
	ProModelitem dtmplnConstrMdlItem;
	ProFeatureGeomitemVisit(&appdataFeature.feature, PRO_SURFACE, UsrPointAddAction, NULL, (ProAppData)&dtmplnConstrMdlItem);

	// ProModelitem�� PRO_SURFACE �ɕϊ�����
	ProSurface selectedSurf;
	status = ProGeomitemToSurface(&dtmplnConstrMdlItem, &selectedSurf);

	/*--------------------------------------------------------------------*\
		�f�[�^������X �̍��W���擾
	\*--------------------------------------------------------------------*/
	ProGeomitemdata* pDataPtr;
	status = ProSurfaceDataGet(selectedSurf, &pDataPtr);
	double dMaxXDtatumPoint = pDataPtr->data.p_surface_data->xyz_max[0];
	double dMinXDtatumPoint = pDataPtr->data.p_surface_data->xyz_min[0];

	if (dMaxXDtatumPoint != dMinXDtatumPoint) {
		return (PRO_TK_E_NOT_FOUND);
	}

	/*--------------------------------------------------------------------*\
		����X �̍��W���擾
	\*--------------------------------------------------------------------*/
	double dXHolePoint = 0;
	dXHolePoint = atof(localHole->cXCord);

	/*--------------------------------------------------------------------*\
		�f�[�^������X�̍��W �� ����X �̍��W����Ώۖʂ��擾����
	\*--------------------------------------------------------------------*/
	double dXPoint = dMaxXDtatumPoint + dXHolePoint;

	ProSurface		xDatumSurface = NULL;
	int iCnt = 0;
	int iLoop = 0;
	for (int iLoop = 0; iLoop < MAX_SURFACE; iLoop++) {
		if (surfInfoData->dSurfZ[iLoop] == 0) {
			break;
		}
		if (surfInfoData->dSurfZ[iLoop*2] < dXPoint && dXPoint < surfInfoData->dSurfZ[(iLoop * 2)+1]) {
			xDatumSurface = surfInfoData->surface[iLoop];
			break;
		}
	}

	if(xDatumSurface == NULL){
		// �Ώۖʂ�������܂���ł���
		return (PRO_TK_E_NOT_FOUND);
	}

	*dtmplnModelitem = xDatumSurface;
	return PRO_TK_NO_ERROR;
}

/*=========================================================================*\
	Function:	getXDatumSurfaceInfoForHole2()
	Purpose:	�\�ʂ������ɕ�����Ă���ꍇ������̂ŁA
				�f�[�^������X�̍��W �� ����X �̍��W����Ώۖʂ��擾����
				����2�ȍ~�̒����Ɏg�p����
SurfaceInfoAppData* surfInfoData	(in)�@�T�[�t�F�C�X���
InputFileHole* localHole			(in)  �R���t�B�O���[�V�����t�@�C���̏��
ProMdl* top_asm						(in)�@�g�b�v�A�Z���u��
double dXHolePoint					(in)�@X���W
ProSurface* dtmplnModelitem			(out)�@ProSurface
\*=========================================================================*/
ProError getXDatumSurfaceInfoForHole2(MainSurfaceInfo* surfInfoData, InputFileHole* localHole, ProMdl* top_asm, double dXHolePoint, ProSurface* dtmplnModelitem) {

	ProError status;

	/*--------------------------------------------------------------------*\
		�f�[�^������X �̃T�[�t�F�C�X�Ƃ��擾
	\*--------------------------------------------------------------------*/
	DatumAppData	appdataFeature;
	int iDatumType = 0;
	iDatumType = atoi(localHole->cDatumType);

	if (iDatumType == 0) {
		// ��������f�[�^�����ʂ̖��O��ݒ�
		ProWstringCopy(X_DATUM_NAME_0, appdataFeature.name, PRO_VALUE_UNUSED);

	}
	else if (iDatumType == 1) {
		// ��������f�[�^�����ʂ̖��O��ݒ�
		ProWstringCopy(X_DATUM_NAME_1, appdataFeature.name, PRO_VALUE_UNUSED);

	}
	else if (iDatumType == 2) {
		// ��������f�[�^�����ʂ̖��O��ݒ�
		ProWstringCopy(X_DATUM_NAME_2, appdataFeature.name, PRO_VALUE_UNUSED);

	}
	else if (iDatumType == 3) {
		// ��������f�[�^�����ʂ̖��O��ݒ�
		ProWstringCopy(X_DATUM_NAME_3, appdataFeature.name, PRO_VALUE_UNUSED);

	}
	else if (iDatumType == 4) {
		// ��������f�[�^�����ʂ̖��O��ݒ�
		ProWstringCopy(X_DATUM_NAME_4, appdataFeature.name, PRO_VALUE_UNUSED);

	}
	else {
		// X���W�̊�ƂȂ�f�[�^�����ʂ��z��O�̒l
		return PRO_TK_BAD_INPUTS;
	}

	// ����������
	appdataFeature.iFindCnt = 0;

	// Top�p�[�c�̃f�[�^������(X_DATUM_NAME)�̃t�B�[�`��������
	ProSolidFeatVisit((ProSolid)*top_asm, getFeatureIdAction, NULL, (ProAppData)&appdataFeature);

	if (appdataFeature.iFindCnt == 0) {
		// �f�[�^������(X_DATUM_NAME)�̃t�B�[�`����������Ȃ�����
		return (PRO_TK_E_NOT_FOUND);
	}

	// �t�B�[�`���� PRO_SURFACE ��ProModelitem�ɕϊ�����
	ProModelitem dtmplnConstrMdlItem;
	ProFeatureGeomitemVisit(&appdataFeature.feature, PRO_SURFACE, UsrPointAddAction, NULL, (ProAppData)&dtmplnConstrMdlItem);

	// ProModelitem�� PRO_SURFACE �ɕϊ�����
	ProSurface selectedSurf;
	status = ProGeomitemToSurface(&dtmplnConstrMdlItem, &selectedSurf);

	/*--------------------------------------------------------------------*\
		�f�[�^������X �̍��W���擾
	\*--------------------------------------------------------------------*/
	ProGeomitemdata* pDataPtr;
	status = ProSurfaceDataGet(selectedSurf, &pDataPtr);
	double dMaxXDtatumPoint = pDataPtr->data.p_surface_data->xyz_max[0];
	double dMinXDtatumPoint = pDataPtr->data.p_surface_data->xyz_min[0];

	if (dMaxXDtatumPoint != dMinXDtatumPoint) {
		return (PRO_TK_E_NOT_FOUND);
	}

	/*--------------------------------------------------------------------*\
		�f�[�^������X�̍��W �� ����X �̍��W����Ώۖʂ��擾����
	\*--------------------------------------------------------------------*/
	double dXPoint = dMaxXDtatumPoint + dXHolePoint;

	ProSurface		xDatumSurface = NULL;
	int iCnt = 0;
	int iLoop = 0;
	for (int iLoop = 0; iLoop < MAX_SURFACE; iLoop++) {
		if (surfInfoData->dSurfZ[iLoop] == 0) {
			break;
		}
		if (surfInfoData->dSurfZ[iLoop * 2] < dXPoint && dXPoint < surfInfoData->dSurfZ[(iLoop * 2) + 1]) {
			xDatumSurface = surfInfoData->surface[iLoop];
			break;
		}
	}

	if (xDatumSurface == NULL) {
		// �Ώۖʂ�������܂���ł���
		return (PRO_TK_E_NOT_FOUND);
	}

	*dtmplnModelitem = xDatumSurface;
	return PRO_TK_NO_ERROR;

}

/*====================================================================*\
FUNCTION : createDatumAxis
PURPOSE  : �f�[�^�����̍쐬
 top_asm			(in) �A�Z���u��
 mdlPart			(in) ����������p�[�c
 surfaceMdlItem		(in) �f�[�^�������쐬�����
 dtmPlnFeature_X	(in) ��1�������鎞�Ɏg�p�����f�[�^������(X��)
 dtmPlnFeature_Y	(in) ��1�������鎞�Ɏg�p�����f�[�^������(Y��)
 distX				(in) ��1����̋���(X��)
 distY				(in) ��1����̋���(Y��)
 axisFeature		(out)�f�[�^�����t�B�[�`��
 ���l
 ��1���\�ʁA��2�ȍ~����(�΂�)�ʂɌ���������ꍇ�Ɏg�p����B
 �ʂɑ΂��Đ����ȃf�[�^�������쐬����

 -------------
PRO_E_FEATURE_TREE
	|--PRO_E_FEATURE_TYPE
	|--PRO_E_STD_FEATURE_NAME
	|--PRO_E_DTMAXIS_CONSTRAINTS
	|    |--PRO_E_DTMAXIS_CONSTRAINT	(�T�[�t�F�C�X)
	|         |--PRO_E_DTMAXIS_CONSTR_TYPE
	|         |--PRO_E_DTMAXIS_CONSTR_REF
	|
	|--PRO_E_DTMAXIS_DIM_CONSTRAINTS
		|--PRO_E_DTMAXIS_DIM_CONSTRAINT	(�f�[�^������1)
		|    |--PRO_E_DTMAXIS_DIM_CONSTR_REF
		|    |--PRO_E_DTMAXIS_DIM_CONSTR_VAL
		|
		|--PRO_E_DTMAXIS_DIM_CONSTRAINT	(�f�[�^������2)
			|--PRO_E_DTMAXIS_DIM_CONSTR_REF
			|--PRO_E_DTMAXIS_DIM_CONSTR_VAL
\*====================================================================*/
ProError createDatumAxis(
	ProMdl top_asm, 
	ProMdl mdlPart, 
	ProModelitem surfaceMdlItem,
	ProFeature dtmPlnFeature_X,
	ProFeature dtmPlnFeature_Y,
	double distX,
	double distY,
	ProFeature* holeFeature
)
{
	ProReference REPDEP_ref3;
	ProReference REPDEP_ref2;
	ProReference REPDEP_ref1;
	ProErrorlist            errors;
	ProModelitem            model_item;
	ProSelection            model_sel;
	ProFeature              feature;
	ProFeatureCreateOptions* opts = 0;
	ProAsmcomppath* p_comp_path = NULL;
	ProValue                value;
	ProError		    status;

	ProElement pro_e_feature_tree;
	ProElement pro_e_feature_type;
	ProElement pro_e_std_feature_name;
	ProElement pro_e_dtmaxis_constraints;
	ProElement pro_e_dtmaxis_constraint;
	ProElement pro_e_dtmaxis_constr_type;
	ProElement pro_e_dtmaxis_constr_ref;
	ProElement pro_e_dtmaxis_constr_ref2;
	ProElement pro_e_dtmaxis_constr_ref3;
	ProElement pro_e_dtmaxis_dim_constraints;
	ProElement pro_e_dtmaxis_dim_constraint;
	ProElement pro_e_dtmaxis_dim_constr_ref2;
	ProElement pro_e_dtmaxis_dim_constr_val2;
	ProElement pro_e_dtmaxis_dim_constraint2;
	ProElement pro_e_dtmaxis_dim_constr_ref3;
	ProElement pro_e_dtmaxis_dim_constr_val3;

	ProName 	wide_string;
	ProValueData 	value_data;
	ProSelection* p_select;
	int 		n_select;
	ProBoolean 	is_interactive = PRO_B_TRUE;

	/*---------------------------------------------------------------*\
	  PRO_E_FEATURE_TREE
	\*---------------------------------------------------------------*/
	status = ProElementAlloc(PRO_E_FEATURE_TREE, &pro_e_feature_tree);

	/*---------------------------------------------------------------*\
	  PRO_E_FEATURE_TREE
		|--PRO_E_FEATURE_TYPE
	\*---------------------------------------------------------------*/
	status = ProElementAlloc(PRO_E_FEATURE_TYPE, &pro_e_feature_type);
	status = ProElementIntegerSet(pro_e_feature_type, PRO_FEAT_DATUM_AXIS);
	status = ProElemtreeElementAdd(pro_e_feature_tree, NULL, pro_e_feature_type);

	/*---------------------------------------------------------------*\
		PRO_E_FEATURE_TREE
			|--PRO_E_STD_FEATURE_NAME
	\*---------------------------------------------------------------*/
	status = ProElementAlloc(PRO_E_STD_FEATURE_NAME, &pro_e_std_feature_name);
	ProStringToWstring(wide_string, "MY_A_1");
	status = ProElementWstringSet(pro_e_std_feature_name, wide_string);
	status = ProElemtreeElementAdd(pro_e_feature_tree, NULL, pro_e_std_feature_name);

	/*---------------------------------------------------------------*\
		PRO_E_FEATURE_TREE
			|--PRO_E_DTMAXIS_CONSTRAINTS
			|    |--PRO_E_DTMAXIS_CONSTRAINT	(�T�[�t�F�C�X)
			|		|--PRO_E_DTMAXIS_CONSTR_REF
			|		|--PRO_E_DTMAXIS_CONSTR_TYPE
	\*---------------------------------------------------------------*/
	// PRO_E_DTMAXIS_CONSTRAINTS
	status = ProElementAlloc(PRO_E_DTMAXIS_CONSTRAINTS, &pro_e_dtmaxis_constraints);
	status = ProElemtreeElementAdd(pro_e_feature_tree, NULL, pro_e_dtmaxis_constraints);

	// PRO_E_DTMAXIS_CONSTRAINT
	status = ProElementAlloc(PRO_E_DTMAXIS_CONSTRAINT, &pro_e_dtmaxis_constraint);
	status = ProElemtreeElementAdd(pro_e_dtmaxis_constraints, NULL, pro_e_dtmaxis_constraint);

	// PRO_E_DTMAXIS_CONSTR_REF
	ProAsmcomppath compPath;
	ProSelection dtmplnConstrSel;
	getAsmcompathIncludePart(top_asm, mdlPart, &compPath);
	status = ProSelectionAlloc(&compPath, &surfaceMdlItem, &dtmplnConstrSel);

	status = ProElementAlloc(PRO_E_DTMAXIS_CONSTR_REF, &pro_e_dtmaxis_constr_ref);
	status = ProSelectionToReference(dtmplnConstrSel, &REPDEP_ref1);
	status = ProElementReferenceSet(pro_e_dtmaxis_constr_ref, REPDEP_ref1);
	status = ProElemtreeElementAdd(pro_e_dtmaxis_constraint, NULL, pro_e_dtmaxis_constr_ref);

	// PRO_E_DTMAXIS_CONSTR_TYPE
	status = ProElementAlloc(PRO_E_DTMAXIS_CONSTR_TYPE, &pro_e_dtmaxis_constr_type);
	status = ProElementIntegerSet(pro_e_dtmaxis_constr_type, PRO_DTMAXIS_CONSTR_TYPE_NORMAL);
	status = ProElemtreeElementAdd(pro_e_dtmaxis_constraint, NULL, pro_e_dtmaxis_constr_type);

	/*---------------------------------------------------------------*\
		PRO_E_FEATURE_TREE
			|--PRO_E_DTMAXIS_DIM_CONSTRAINTS
				|--PRO_E_DTMAXIS_DIM_CONSTRAINT	(�f�[�^������1)
				|    |--PRO_E_DTMAXIS_DIM_CONSTR_REF
				|    |--PRO_E_DTMAXIS_DIM_CONSTR_VAL
				|
				|--PRO_E_DTMAXIS_DIM_CONSTRAINT	(�f�[�^������2)
					|--PRO_E_DTMAXIS_DIM_CONSTR_REF
					|--PRO_E_DTMAXIS_DIM_CONSTR_VAL
	\*---------------------------------------------------------------*/
	// PRO_E_DTMAXIS_DIM_CONSTRAINTS
	status = ProElementAlloc(PRO_E_DTMAXIS_DIM_CONSTRAINTS, &pro_e_dtmaxis_dim_constraints);
	status = ProElemtreeElementAdd(pro_e_feature_tree, NULL, pro_e_dtmaxis_dim_constraints);

	// PRO_E_DTMAXIS_DIM_CONSTRAINT 	(�f�[�^������1)
	status = ProElementAlloc(PRO_E_DTMAXIS_DIM_CONSTRAINT, &pro_e_dtmaxis_dim_constraint);
	status = ProElemtreeElementAdd(pro_e_dtmaxis_dim_constraints, NULL, pro_e_dtmaxis_dim_constraint);

	// PRO_E_DTMAXIS_CONSTR_REF
	ProModelitem mdlitem2;
	ProSelection pSel2;

	ProFeatureGeomitemVisit(&dtmPlnFeature_X, PRO_SURFACE, UsrPointAddAction, NULL, (ProAppData)&mdlitem2);
	status = ProSelectionAlloc(NULL, &mdlitem2, &pSel2);
	status = ProElementAlloc(PRO_E_DTMAXIS_DIM_CONSTR_REF, &pro_e_dtmaxis_dim_constr_ref2);
	status = ProSelectionToReference(pSel2, &REPDEP_ref2);
	status = ProElementReferenceSet(pro_e_dtmaxis_dim_constr_ref2, REPDEP_ref2);
	status = ProElemtreeElementAdd(pro_e_dtmaxis_dim_constraint, NULL, pro_e_dtmaxis_dim_constr_ref2);

	// PRO_E_DTMAXIS_DIM_CONSTR_VAL:�I�t�Z�b�g����(X)
	status = ProElementAlloc(PRO_E_DTMAXIS_DIM_CONSTR_VAL, &pro_e_dtmaxis_dim_constr_val2);
	status = ProElementDoubleSet(pro_e_dtmaxis_dim_constr_val2, distX);
	status = ProElemtreeElementAdd(pro_e_dtmaxis_dim_constraint, NULL, pro_e_dtmaxis_dim_constr_val2);

	// PRO_E_DTMAXIS_DIM_CONSTRAINT 	(�f�[�^������2)
	status = ProElementAlloc(PRO_E_DTMAXIS_DIM_CONSTRAINT, &pro_e_dtmaxis_dim_constraint);
	status = ProElemtreeElementAdd(pro_e_dtmaxis_dim_constraints, NULL, pro_e_dtmaxis_dim_constraint);

	// PRO_E_DTMAXIS_CONSTR_REF
	ProModelitem mdlitem3;
	ProSelection pSel3;
	ProFeatureGeomitemVisit(&dtmPlnFeature_Y, PRO_SURFACE, UsrPointAddAction, NULL, (ProAppData)&mdlitem3);
	status = ProSelectionAlloc(NULL, &mdlitem3, &pSel3);

	status = ProElementAlloc(PRO_E_DTMAXIS_DIM_CONSTR_REF, &pro_e_dtmaxis_dim_constr_ref3);
	status = ProSelectionToReference(pSel3, &REPDEP_ref3);
	status = ProElementReferenceSet(pro_e_dtmaxis_dim_constr_ref3, REPDEP_ref3);
	status = ProElemtreeElementAdd(pro_e_dtmaxis_dim_constraint, NULL, pro_e_dtmaxis_dim_constr_ref3);

	// PRO_E_DTMAXIS_DIM_CONSTR_VAL�I�t�Z�b�g����(Y)
	status = ProElementAlloc(PRO_E_DTMAXIS_DIM_CONSTR_VAL, &pro_e_dtmaxis_dim_constr_val3);
	status = ProElementDoubleSet(pro_e_dtmaxis_dim_constr_val3, distY);
	status = ProElemtreeElementAdd(pro_e_dtmaxis_dim_constraint, NULL, pro_e_dtmaxis_dim_constr_val3);

	/*---------------------------------------------------------------*\
	  Create the feature in the current model.
	\*---------------------------------------------------------------*/
	status = ProMdlToModelitem(top_asm, &model_item);
	status = ProSelectionAlloc(p_comp_path, &model_item, &model_sel);
	status = ProArrayAlloc(1, sizeof(ProFeatureCreateOptions), 1, (ProArray*)&opts);

	opts[0] = PRO_FEAT_CR_NO_OPTS;

	status = ProFeatureWithoptionsCreate(model_sel, pro_e_feature_tree, opts, PRO_REGEN_NO_FLAGS, holeFeature, &errors);
	TRAIL_PRINT("%s(%d) : ProFeatureWithoptionsCreate = %s", __func__, __LINE__, getProErrorMessage(status));

	status = ProArrayFree((ProArray*)&opts);
	status = ProElementFree(&pro_e_feature_tree);

	return (status);
}

/*====================================================================*\
FUNCTION : createHoleBaseAxis
PURPOSE  : �f�[�^��������Ɍ���������
	ProMdl top_asm,					in top�A�Z���u��
	ProMdl part						in ����������p�[�c
	ProFeature dtmAxisFeature2,		in ��ƂȂ�f�[�^�����̃t�B�[�`��
	ProFeature dtmPlnFeature_selSurf in �z�u��/�����������
	double distX,					in ��1����̋���(X��)
	double distY					in ��1����̋���(Y��)
	double dDiameter				in �����a
	ProFeature* holeFeature			out ���t�B�[�`��
���l
	�w�肵���f�[�^��������Ɍ���������
\*====================================================================*/
ProError createHoleBaseAxis(ProMdl top_asm,
	ProMdl part,
	ProFeature dtmAxisFeature2,
	ProFeature dtmPlnFeature_selSurf,
	double distX,
	double distY,
	double dDiameter,
	ProFeature* holeFeature)
{
	ProError status;

	ProElement feat_elemtree;
	ProElement elem_feattype;
	ProElement elem_featform;
	ProElement elem_hle_com;
	ProElement elem_hle_type_new;
	ProElement elem_hle_stan_type;
	ProElement elem_diameter;
	ProElement elem_hole_std_depth;
	ProElement elem_hole_depth_to;
	ProElement elem_hole_depth_to_type;
	ProElement elem_hole_depth_to_value;
	ProElement elem_hole_depth_from;
	ProElement elem_hole_depth_from_type;
	ProElement elem_hole_depth_from_value;
	ProElement elem_hle_placement;
	ProElement elem_hle_prim_ref;
	ProElement elem_hle_pl_type;
	ProElement elem_hle_dim_ref1;
	ProElement elem_hle_dim_dist1;
	ProElement elem_hle_dim_dist2;
	ProElement lin_hole_dir_ref;
	ProValue value;
	ProValueData value_data;
	ProSelection* p_selection;
	int n_selection;

	ProFeatureCreateOptions* options = 0;
	ProFeature created_feature;
	ProErrorlist p_errors;
	//ProMdl model;
	ProModelitem model_item;
	ProSelection model_selection;
	ProReference reference;


	/***********************************************
	 �v�f�c���[�쐬�̊J�n
	*************************************************/
	/* �v�f�c���[(���[�g�v�f)�̒ǉ� */
	status = ProElementAlloc(PRO_E_FEATURE_TREE, &feat_elemtree);

	/***********************************************
	 PRO_E_FEATURE_TYPE
	*************************************************/
	/* �v�f�c���[�� �t�B�[�`���^�C�v(��) ��ǉ����� */
	status = ProElementAlloc(PRO_E_FEATURE_TYPE, &elem_feattype);
	status = ProElementIntegerSet(elem_feattype, PRO_FEAT_HOLE);
	status = ProElemtreeElementAdd(feat_elemtree, NULL, elem_feattype);

	/***********************************************
	 PRO_E_FEATURE_FORM
	*************************************************/
	/* �v�f�c���[�� �t�B�[�`���t�H�[��(�X�g���[�g�z�[��) ��ǉ����� */
	status = ProElementAlloc(PRO_E_FEATURE_FORM, &elem_featform);
	status = ProElementIntegerSet(elem_featform, PRO_HLE_TYPE_STRAIGHT);
	status = ProElemtreeElementAdd(feat_elemtree, NULL, elem_featform);

	/***********************************************
	 PRO_E_HLE_COM
	*************************************************/
	/* �v�f�c���[�� �����̋��ʗv�f �̒ǉ�  */
	status = ProElementAlloc(PRO_E_HLE_COM, &elem_hle_com);
	status = ProElemtreeElementAdd(feat_elemtree, NULL, elem_hle_com);

	/* ����� (���^�C�v:�X�g���[�g) ���w�肷�� */
	status = ProElementAlloc(PRO_E_HLE_TYPE_NEW, &elem_hle_type_new);
	status = ProElementIntegerSet(elem_hle_type_new, PRO_HLE_NEW_TYPE_STRAIGHT);
	status = ProElemtreeElementAdd(elem_hle_com, NULL, elem_hle_type_new);

	/* ����� (���̒��a) ���w�肷�� */
	status = ProElementAlloc(PRO_E_DIAMETER, &elem_diameter);
	status = ProElementDoubleSet(elem_diameter, dDiameter);
	status = ProElemtreeElementAdd(elem_hle_com, NULL, elem_diameter);

	/* �W���[�x�̗v�f��ǉ�����

			  |--PRO_E_HOLE_STD_DEPTH
			  |    |--PRO_E_HOLE_DEPTH_TO
			  |    |    |--PRO_E_HOLE_DEPTH_TO_TYPE
			  |    |    |--PRO_E_EXT_DEPTH_TO_VALUE
			  |    |--PRO_E_HOLE_DEPTH_FROM
			  |         |--PRO_E_HOLE_DEPTH_FROM_TYPE
			  |         |--PRO_E_EXT_DEPTH_FROM_VALUE
	*/

	// �[���v�f
	status = ProElementAlloc(PRO_E_HOLE_STD_DEPTH, &elem_hole_std_depth);
	status = ProElemtreeElementAdd(elem_hle_com, NULL, elem_hole_std_depth);

	// side1 ���F Blind �[��25
	status = ProElementAlloc(PRO_E_HOLE_DEPTH_TO, &elem_hole_depth_to);
	status = ProElemtreeElementAdd(elem_hole_std_depth, NULL, elem_hole_depth_to);
	status = ProElementAlloc(PRO_E_HOLE_DEPTH_TO_TYPE, &elem_hole_depth_to_type);
	status = ProElementIntegerSet(elem_hole_depth_to_type, PRO_HLE_STRGHT_BLIND_DEPTH);
	status = ProElemtreeElementAdd(elem_hole_depth_to, NULL, elem_hole_depth_to_type);
	status = ProElementAlloc(PRO_E_EXT_DEPTH_TO_VALUE, &elem_hole_depth_to_value);
	status = ProElementDoubleSet(elem_hole_depth_to_value, 25.0);
	status = ProElemtreeElementAdd(elem_hole_depth_to, NULL, elem_hole_depth_to_value);

	// side2 ���F Blind �[��25
	status = ProElementAlloc(PRO_E_HOLE_DEPTH_FROM, &elem_hole_depth_from);
	status = ProElemtreeElementAdd(elem_hole_std_depth, NULL, elem_hole_depth_from);
	status = ProElementAlloc(PRO_E_HOLE_DEPTH_FROM_TYPE, &elem_hole_depth_from_type);
	status = ProElementIntegerSet(elem_hole_depth_from_type, PRO_HLE_STRGHT_BLIND_DEPTH);
	status = ProElemtreeElementAdd(elem_hole_depth_from, NULL, elem_hole_depth_from_type);
	status = ProElementAlloc(PRO_E_EXT_DEPTH_FROM_VALUE, &elem_hole_depth_from_value);
	status = ProElementDoubleSet(elem_hole_depth_from_value, 25.0);
	status = ProElemtreeElementAdd(elem_hole_depth_from, NULL, elem_hole_depth_from_value);

	/* �z�u�̏ڍׂɊ֘A����v�f��ǉ�����
	 �������Ƃ��铯����

		 |--PRO_E_HLE_PLACEMENT
		 |    |--PRO_E_HLE_PRIM_REF		(�ꎞ�I��:��)
		 |    |--PRO_E_HLE_PL_TYPE		(�z�u��:PRO_HLE_PL_TYPE_COAX) 
		 |    |--PRO_E_HLE_PLCMNT_PLANE (�z�u��)


	*/

	ProAsmcomppath compPath;
	status = getAsmcompathIncludePart(top_asm, part, &compPath);
	if (status != PRO_TK_NO_ERROR) {
		// �p�[�c(patName)�̃t�B�[�`����������Ȃ�����
		return (PRO_TK_E_NOT_FOUND);
	}

	status = ProElementAlloc(PRO_E_HLE_PLACEMENT, &elem_hle_placement);
	status = ProElemtreeElementAdd(feat_elemtree, NULL, elem_hle_placement);

	// PRO_E_HLE_PRIM_REF�@�ꎟ�I�� (��)
	ProModelitem mdlitem3;
	ProSelection pSel3;
	ProFeatureGeomitemVisit(&dtmAxisFeature2, PRO_AXIS, UsrPointAddAction, NULL, (ProAppData)&mdlitem3);
	status = ProSelectionAlloc(NULL, &mdlitem3, &pSel3);

	status = ProElementAlloc(PRO_E_HLE_PRIM_REF, &elem_hle_prim_ref);
	value_data.type = PRO_VALUE_TYPE_SELECTION;
	value_data.v.r = pSel3;
	status = ProValueAlloc(&value);
	status = ProValueDataSet(value, &value_data);
	status = ProElementValueSet(elem_hle_prim_ref, value);
	status = ProElemtreeElementAdd(elem_hle_placement, NULL, elem_hle_prim_ref);

	// PRO_E_HLE_PL_TYPE  ���z�u�I�v�V����
	status = ProElementAlloc(PRO_E_HLE_PL_TYPE, &elem_hle_pl_type);
	status = ProElementIntegerSet(elem_hle_pl_type, PRO_HLE_PL_TYPE_COAX);
	status = ProElemtreeElementAdd(elem_hle_placement, NULL, elem_hle_pl_type);

	// PRO_E_HLE_PLCMNT_PLANE�@�z�u��
	ProModelitem mdlitem;
	ProSelection pSel;
	ProFeatureGeomitemVisit(&dtmPlnFeature_selSurf, PRO_SURFACE, UsrPointAddAction, NULL, (ProAppData)&mdlitem);
	status = ProSelectionAlloc(NULL, &mdlitem, &pSel);

	status = ProElementAlloc(PRO_E_HLE_PLCMNT_PLANE, &elem_hle_prim_ref);
	value_data.type = PRO_VALUE_TYPE_SELECTION;
	value_data.v.r = pSel;
	status = ProValueAlloc(&value);
	status = ProValueDataSet(value, &value_data);
	status = ProElementValueSet(elem_hle_prim_ref, value);
	status = ProElemtreeElementAdd(elem_hle_placement, NULL, elem_hle_prim_ref);

	// ���� 1
	status = ProElementAlloc(PRO_E_HLE_DIM_DIST1, &elem_hle_dim_dist1);
	status = ProElementDoubleSet(elem_hle_dim_dist1, distX);
	status = ProElemtreeElementAdd(elem_hle_placement, NULL, elem_hle_dim_dist1);

	// PRO_E_HLE_DIM_REF1 ���f�[�^�����̂��߁APRO_E_HLE_DIM_REF2 �̐ݒ�Ȃ�

	// ���� 2
	status = ProElementAlloc(PRO_E_HLE_DIM_DIST2, &elem_hle_dim_dist2);
	status = ProElementDoubleSet(elem_hle_dim_dist2, distY);
	status = ProElemtreeElementAdd(elem_hle_placement, NULL, elem_hle_dim_dist2);

	/* �v�f�c���[�쐬�̏I�� */

	/***********************************************
	 �t�B�[�`���[�̍쐬
	*************************************************/
	status = ProMdlToModelitem(part, &model_item);
	status = ProSelectionAlloc(&compPath, &model_item, &model_selection);
	status = ProArrayAlloc(1, sizeof(ProFeatureCreateOptions), 1, (ProArray*)&options);

	options[0] = PRO_FEAT_CR_NO_OPTS;
	status = ProFeatureWithoptionsCreate(model_selection, feat_elemtree, options, PRO_REGEN_NO_FLAGS, holeFeature, &p_errors);
	TRAIL_PRINT("%s(%d) : ProFeatureWithoptionsCreate = %s", __func__, __LINE__, getProErrorMessage(status));

	/***********************************************
	 ���\�[�X�̉��
	*************************************************/
	status = ProElementFree(&feat_elemtree);
	status = ProSelectionFree(&model_selection);
	status = ProArrayFree((ProArray*)&options);

	return (status);
}

