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
	プロトタイプ宣言
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
	マクロ
\*-----------------------------------------------------------------*/
#define Z_DATUM_NAME L"XZ_MAIN"			// Z座標の基準となるデータム平面の名前

#define X_DATUM_NAME_0 L"XM"			// X座標の基準となるデータム平面の名前
#define X_DATUM_NAME_1 L"FRAME_FRONT"	// X座標の基準となるデータム平面の名前
#define X_DATUM_NAME_2 L"XRAP"			// X座標の基準となるデータム平面の名前
#define X_DATUM_NAME_3 L"FRAME_END"		// X座標の基準となるデータム平面の名前
#define X_DATUM_NAME_4 L"FAP"			// X座標の基準となるデータム平面の名前

// グループ名(L)
#define REF_HOLE_LEFT_BLIND		L"REF_HOLE_LEFT_BLIND"	// 穴1のグループ名
#define HOLE_LEFT_BLIND			L"HOLE_LEFT_BLIND"		// 穴2以降のグループ名
#define CSYS_DTM_LEFT_OUTER		L"CSYS_DTM_LEFT_OUTER"	// データム平面のグループ名
// グループ名(R)
#define REF_HOLE_RIGHT_BLIND		L"REF_HOLE_RIGHT_BLIND"	// 穴1のグループ名
#define HOLE_RIGHT_BLIND			L"HOLE_RIGHT_BLIND"		// 穴2以降のグループ名
#define CSYS_DTM_RIGHT_OUTER		L"CSYS_DTM_RIGHT_OUTER"	// データム平面のグループ名
// 取得パラメータ
#define PARAMETER_FRAME_HEIGHT		L"FRAME_HEIGHT"	// フレームの高さ
#define PARAMETER_THICKNESS			L"THICKNESS"	// フレームの厚み

// その他
#define BOTH_FRAME		0
#define LEFT_FRAME		1
#define RIGHT_FRAME		2

#define MAX_SURFACE		10

// サーフェイス 検索時に使用するProAppData
typedef struct {
	ProSurface		surface;		// out 検索結果
	double			dYPoint;		// out Y座標（要素数は適当な値）
}SurfaceAppData;

// フレームの表面のサーフェイス情報
// とりあえず10面分を用意したが、10という数字に意味はない。いただいたトラックデータ内は最大3面だった
typedef struct {
	double			dSurfZ[MAX_SURFACE * 2];	// out Z座標格納用:穴をあけるフレームの境界線。１表面につき2座標格納。10面分用意。
	ProSurface		surface[MAX_SURFACE];		// out 各面のサーフェイス情報
	int				iCounter;					// out counter
}MainSurfaceInfo;


// 穴あけ面関連の構造体
typedef struct {
	int				iLRFlag;			// in  1:L, 2:R
	double			dSurfY[MAX_SURFACE];	// in/out :MainSurfaceInfoの Y座標格納用/Backを調べるときに使用

	MainSurfaceInfo strFrontInfo;
	MainSurfaceInfo strBackInfo;
}SurfaceInfoAppData;


// ProGroup→ProFeatur変換
typedef struct {
	ProGroup		group;		// in 検索対象
	ProMdlName		wName;		// in 検索対象名
	ProFeature		feature;	// out 検索結果
}strGroupToFeature;

typedef struct {
	ProName wParameterName;		// in 検索対象
	double			dParam;		// Out 検索結果
}strSearchParameter;

static InputFileHoleTable* staticHoleTable;	// コンフィグレーションファイルのHoleTable情報
static ProFeature* staticHoleGroupAll;		// 複数の穴グループを1つのグループにまとめる

static ProMdl staticMdlSideFrame;
/*====================================================================*\
FUNCTION : setHoles_HoleTableSection
PURPOSE  : Holes/HoleTableセクション
	ProMdl* top_asm						(in) 穴あけパーツを含むTopアセンブリ
	ProMdl* mdlSideFrame				(in) 穴あけパーツを含むTopアセンブリ
	ProPath patName						(in) 穴あけパーツの基準名
	InputFileHole* strHole				(in) コンフィグレーションファイルの内容(Holes)
	InputFileHoleTable* strHoleTable	(in) コンフィグレーションファイルの内容(Hole_Table)
	int iHoleSectionMaxRows				(in) 処理すべきコンフィグレーションファイルの行数(Holes)
	int iHoleTableSectionMaxRows		(in) 処理すべきコンフィグレーションファイルの行数(Hole_Table)
	int iHoletype						(in) 0:Hole , 1:innerlinerHole
*====================================================================*/
ProError  setHoles_HoleTableSection(ProMdl* top_asm, ProMdl* mdlSideFrame, ProPath patName, InputFileHole* strHole, InputFileHoleTable* strHoleTable, int iHoleSectionMaxRows, int iHoleTableSectionMaxRows, int iHoletype)
{
	ProError status = PRO_TK_NO_ERROR;
	int iResultL;
	int iResultR;
	int iResultB;
	wchar_t wSide[INPUTFILE_MAXLINE];           // 穴をあける方向. L(左)/R(右)/B(両方)
	ProPath wRightPart;
	ProPath wLeftPart;

	// グローバル変数の初期化
	staticMdlSideFrame = *mdlSideFrame;

	// 左右の穴あけパーツ名を設定
	ProWstringCopy(patName, wLeftPart, PRO_VALUE_UNUSED);
	ProWstringConcatenate(L"_1", wLeftPart, PRO_VALUE_UNUSED);
	ProWstringCopy(patName, wRightPart, PRO_VALUE_UNUSED);
	ProWstringConcatenate(L"_2", wRightPart, PRO_VALUE_UNUSED);

	/*--------------------------------------------------------------------*\
		part名から穴をあけるパーツ情報の確認
	\*--------------------------------------------------------------------*/
	ProError status1 = PRO_TK_NO_ERROR;
	ProError status2 = PRO_TK_NO_ERROR;
	ProMdl mdlLeftPart = NULL;
	ProMdl mdlRightPart = NULL;

	status1 = ProMdlnameInit(wLeftPart, PRO_MDLFILE_PART, &mdlLeftPart);
	TRAIL_PRINT("%s(%d) : ProMdlnameInit = %s", __func__, __LINE__, getProErrorMessage(status1));

	if (status1 != PRO_TK_NO_ERROR) {
		// 編集するパーツが見つかりませんでした。
		ProCharPath cPatName;
		ProWstringToString(cPatName, wLeftPart);
		LOG_PRINT("NOK : %s not found", cPatName);
	}

	status2 = ProMdlnameInit(wRightPart, PRO_MDLFILE_PART, &mdlRightPart);
	TRAIL_PRINT("%s(%d) : ProMdlnameInit = %s", __func__, __LINE__, getProErrorMessage(status2));

	if (status2 != PRO_TK_NO_ERROR) {
		// 編集するパーツが見つかりませんでした。
		ProCharPath cPatName;
		ProWstringToString(cPatName, wRightPart);
		LOG_PRINT("NOK : %s not found", cPatName);
	}
	if (status1 != PRO_TK_NO_ERROR || status2 != PRO_TK_NO_ERROR) {
		return;
	}

	/*--------------------------------------------------------------------*\
		穴あけパーツのサーフェイス情報を取得
	\*--------------------------------------------------------------------*/
	// メモリ確保
	SurfaceInfoAppData* rightSurfInfoData;	// 穴をあけるフレームの境界線(右)
	rightSurfInfoData = (SurfaceInfoAppData*)calloc(1, sizeof(SurfaceInfoAppData));
	if (!rightSurfInfoData) {
		// メモリ不足
		LOG_PRINT("NOK : Not enough memory");
		return PRO_TK_GENERAL_ERROR;
	}

	// メモリ確保
	SurfaceInfoAppData* leftSurfInfoData;	// 穴をあけるフレームの境界線(左)
	leftSurfInfoData = (SurfaceInfoAppData*)calloc(1, sizeof(SurfaceInfoAppData));
	if (!leftSurfInfoData) {
		// メモリ不足
		LOG_PRINT("NOK : Not enough memory");
		return PRO_TK_GENERAL_ERROR;
	}

	// サーフェイス情報の取得
	getSurfaceInfo(rightSurfInfoData, RIGHT_FRAME, mdlRightPart);
	getSurfaceInfo(leftSurfInfoData, LEFT_FRAME, mdlLeftPart);


	/*--------------------------------------------------------------------*\
		穴あけ処理の開始
	\*--------------------------------------------------------------------*/
	for (int iInputMdlCnt = 0; iInputMdlCnt < iHoleSectionMaxRows; iInputMdlCnt++) {
		// L,R,Bの切り分け
		ProStringToWstring(wSide, strHole->cSide);
		status = ProWstringCompare(L"L", wSide, PRO_VALUE_UNUSED, &iResultL);
		status = ProWstringCompare(L"R", wSide, PRO_VALUE_UNUSED, &iResultR);
		status = ProWstringCompare(L"B", wSide, PRO_VALUE_UNUSED, &iResultB);

		if (iResultL == 0) {
			// Lの処理
			status = createHole(top_asm, mdlLeftPart, strHole, strHoleTable, LEFT_FRAME, iHoleTableSectionMaxRows, iHoletype, leftSurfInfoData);

		}
		else if(iResultR == 0) {
			// Rの処理
			status = createHole(top_asm, mdlRightPart, strHole, strHoleTable, RIGHT_FRAME, iHoleTableSectionMaxRows, iHoletype, rightSurfInfoData);

		}
		else if (iResultB == 0) {
			// Bの処理
			status = createHole(top_asm, mdlLeftPart, strHole, strHoleTable, BOTH_FRAME, iHoleTableSectionMaxRows, iHoletype, leftSurfInfoData);
			status = createHole(top_asm, mdlRightPart, strHole, strHoleTable, RIGHT_FRAME, iHoleTableSectionMaxRows, iHoletype, rightSurfInfoData);

		}
		else {
			// 想定外のエラー. 想定外の穴あけ方向(L/R/B)です
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
PURPOSE  : Holes/HoleTableセクション
	ProMdl* top_asm						(in) Feature対象
	ProMdl mdlPart						(in) 穴あけパーツのProMdl
	InputFileHole* localHole			(in) コンフィグレーションファイルの内容
	InputFileHoleTable* localHoleTable	(in) コンフィグレーションファイルの内容
	int iLRFlag							(in) 穴あけパーツRight/Left情報 1:Left, 2:Right 3:Both(Left)
	int iHoleTableSectionMaxRows		(in) 処理すべきコンフィグレーションファイルの行数(Hole_Table)
	int iHoletype						(in) 0:Hole , 1:innerlinerHole
	SurfaceInfoAppData* surfInfoData	(in) 穴あけサーフェイス情報
*====================================================================*/
ProError  createHole(ProMdl* top_asm, ProMdl mdlPart, InputFileHole* localHole, InputFileHoleTable* localHoleTable, int iLRFlag ,int iHoleTableSectionMaxRows, int iHoletype, SurfaceInfoAppData* surfInfoData)
{
	ProError status = PRO_TK_NO_ERROR;
	ProFeature dtmPlnFeature_X;	// X軸のデータム平面
	ProFeature dtmPlnFeature_Y;	// Y軸のデータム平面
	ProFeature dtmPlnFeature_Z;	// Z軸のデータム平面
	ProFeature dtmPlnFeature_YZ;	// YZ軸のデータム平面
	ProFeature dtmPlnFeature_selSurf;	// 配置面/穴あけ面のデータム平面
	ProFeature dtmAxisFeature2;	// 穴2以降のデータム軸
	ProFeature holeFeatureOne;	// 穴1
	ProFeature holeFeature;	// 穴2以降
	ProGroup group;
	ProFeature dtmPlnFeature_Z2;	// Z軸のデータム平面

	int  n_sel;
	int iHoleTableCnt = 0;
	int iUnderUpperFlag = 0;
	ProBoolean bAxisFlag = PRO_B_FALSE;

	/*--------------------------------------------------------------------*\
		Left / Right によるグループ名の定義
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
		// 想定外の異常.ここに来ることはありえないのでエラーメッセージを出さない
		
	}

	/*--------------------------------------------------------------------*\
		同じ穴グループHoleTableを取得する
	\*--------------------------------------------------------------------*/
	status = getHoleTableInfOfSameNum(localHole, localHoleTable, iHoleTableSectionMaxRows ,&iHoleTableCnt);
	if (status != PRO_TK_NO_ERROR) {
		// エラーメッセージはgetHoleTableInfOfSameNum内で記載
		return status;
	}

	// 確保したメモリの先頭アドレス格納用
	InputFileHoleTable* startHoleTable = staticHoleTable;

	/*--------------------------------------------------------------------*\
		フレーム表面が複数に分かれることもあるため、
		dXPoint と iDatumType によってそれぞれを判断する
	\*--------------------------------------------------------------------*/
	ProSurface		xDatumSurface;

	int iHoleFlag = atoi(staticHoleTable->cHoleFlag);

	if (iHoleFlag == 1 || iHoleFlag == 2) {
		// サーフェスの情報を取得
		status = getXDatumSurfaceInfo(&surfInfoData->strFrontInfo, localHole, top_asm, &xDatumSurface);

	}
	else if (iHoleFlag == 3 || iHoleFlag == 4) {
		// サーフェスの情報を取得。裏面基準で穴をあける
		status = getXDatumSurfaceInfo(&surfInfoData->strBackInfo, localHole, top_asm, &xDatumSurface);
	}
	else {
		// 想定外のエラー. 穴フラグ(1～4)が想定外です
		LOG_PRINT("NOK : %s : HoleFlag(1～4) is an abnormal value", staticHoleTable->cHoleFlag);

	}

	if (status != PRO_TK_NO_ERROR) {
		// 基準となるサーフェイスが見つからなかった
		LOG_PRINT("NOK : %s %s : failed to create (errorcode:1)", localHole->cHoleGroupName, staticHoleTable->cHoleID);

		displayError(localHole, iHoleTableCnt);
		return status;
	}

	// ProModelitemに変換する
	ProModelitem dtmplnConstrMdlItemZ;
	ProAsmcomppath compPath;
	int iSurfaceId;
	status = getAsmcompathIncludePart(*top_asm, mdlPart, &compPath);
	status = ProSurfaceIdGet(xDatumSurface, &iSurfaceId);
	status = ProModelitemInit(mdlPart, iSurfaceId, PRO_SURFACE, &dtmplnConstrMdlItemZ);
	TRAIL_PRINT("%s(%d) : ProModelitemInit = %s", __func__, __LINE__, getProErrorMessage(status));

	/*--------------------------------------------------------------------*\
		フレーム表面と平行な データム平面の作成
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

	// フレーム表面と平行なデータム平面を作成
	status = createDatumPlaneZ(top_asm, mdlPart, dYPoint, dtmplnConstrMdlItemZ, &dtmPlnFeature_Z);
	if (status != PRO_TK_NO_ERROR) {
		// フレーム表面と平行なデータム平面の作成に失敗
		LOG_PRINT("NOK : %s %s : failed to create (errorcode:6)", localHole->cHoleGroupName, staticHoleTable->cHoleID);
		displayError(localHole, iHoleTableCnt);

		return status;
	}

	/*--------------------------------------------------------------------*\
		フレーム下部のサーフェスの情報を取得し、ProModelitemを取得する 
	\*--------------------------------------------------------------------*/
	
	ProSelection selSurfYLower;
	SurfaceAppData appdataSurface;

	// 各値の初期化 double型の最大値で初期化する
	appdataSurface.dYPoint = DBL_MAX;

	// 基準となるサーフェイスを取得
	status = ProSolidSurfaceVisit(ProMdlToSolid(mdlPart),
		(ProSurfaceVisitAction)getLowerSurfaceFromYpointAction,
		(ProSurfaceFilterAction)NULL,
		(ProAppData)&appdataSurface);

	if (status != PRO_TK_NO_ERROR) {
		// 基準となるサーフェイスが見つからなかった
		LOG_PRINT("NOK : %s %s : failed to create (errorcode:3)", localHole->cHoleGroupName, staticHoleTable->cHoleID);

		displayError(localHole, iHoleTableCnt);
		return (PRO_TK_E_NOT_FOUND);
	}

	// ProModelitemに変換する
	ProModelitem dtmplnConstrMdlItemYLower;
	iSurfaceId = 0;
	status = ProSurfaceIdGet(appdataSurface.surface, &iSurfaceId);
	status = ProModelitemInit(mdlPart, iSurfaceId, PRO_SURFACE, &dtmplnConstrMdlItemYLower);
	status = ProSelectionAlloc(&compPath, &dtmplnConstrMdlItemYLower, &selSurfYLower);
	TRAIL_PRINT("%s(%d) : ProSelectionAlloc = %s", __func__, __LINE__, getProErrorMessage(status));

	/*--------------------------------------------------------------------*\
		フレーム上部のサーフェスの情報を取得し、ProModelitemを取得する
	\*--------------------------------------------------------------------*/

	ProSelection selSurfYUpper;

	// 各値の初期化 double型の最小値で初期化する
	appdataSurface.dYPoint = DBL_MIN;

	// 基準となるサーフェイスを取得
	status = ProSolidSurfaceVisit(ProMdlToSolid(mdlPart),
		(ProSurfaceVisitAction)getUpperSurfaceFromYpointAction,
		(ProSurfaceFilterAction)NULL,
		(ProAppData)&appdataSurface);

	if (status != PRO_TK_NO_ERROR) {
		// 基準となるサーフェイスが見つからなかった
		LOG_PRINT("NOK : %s %s : failed to create (errorcode:4)", localHole->cHoleGroupName, staticHoleTable->cHoleID);

		displayError(localHole, iHoleTableCnt);
		return (PRO_TK_E_NOT_FOUND);
	}

	// ProModelitemに変換する
	ProModelitem dtmplnConstrMdlItemYUpper;
	iSurfaceId = 0;
	status = ProSurfaceIdGet(appdataSurface.surface, &iSurfaceId);
	status = ProModelitemInit(mdlPart, iSurfaceId, PRO_SURFACE, &dtmplnConstrMdlItemYUpper);
	status = ProSelectionAlloc(&compPath, &dtmplnConstrMdlItemYUpper, &selSurfYUpper);
	TRAIL_PRINT("%s(%d) : ProSelectionAlloc = %s", __func__, __LINE__, getProErrorMessage(status));

	/*--------------------------------------------------------------------*\
		X軸のデータム平面を作成
		XMやFRAME_ENDと並行なデータム平面
	\*--------------------------------------------------------------------*/
	int iDatumType = 0;
	iDatumType = atoi(localHole->cDatumType);
	double dXPoint = 0;
	dXPoint = atof(localHole->cXCord);

	status = createDatumPlaneX(top_asm, dXPoint, &dtmPlnFeature_X, iDatumType);
	TRAIL_PRINT("%s(%d) : createDatumPlaneX = %s", __func__, __LINE__, getProErrorMessage(status));

	if (status != PRO_TK_NO_ERROR) {
		// 穴あけに失敗
		LOG_PRINT("NOK : %s %s : failed to create (errorcode:5)", localHole->cHoleGroupName, staticHoleTable->cHoleID);
		displayError(localHole, iHoleTableCnt);

		return status;
	}

	/*--------------------------------------------------------------------*\
		穴の大きさを取得
	\*--------------------------------------------------------------------*/
	double dDiameter = 0;
	if (iHoletype == HOLE) {
		// sideFrameの穴あけの場合
		dDiameter = atof(staticHoleTable->cFrameDiameter);
	}
	else {
		// innerlinerの穴あけの場合
		dDiameter = atof(staticHoleTable->cInnerLineDiameter);
	}

	/*--------------------------------------------------------------------*\
		YZ軸のデータム平面を作製

		Hole_tableにて穴1のZ座標=0の時、
		1グループ内すべての穴がフレーム上部 or 下部となる。
		(2枚目のデータム平面の向きが変わる)
	\*--------------------------------------------------------------------*/
	// 方向を反転させる
	double dZPoint = atof(staticHoleTable->cZCord);
	// Y軸のデータム平面の作成
	status = createDatumPlaneY(top_asm, mdlPart, (dZPoint * -1), &dtmPlnFeature_Y);
	if (status != PRO_TK_NO_ERROR) {
		// Y軸のデータム平面の作成に失敗
		LOG_PRINT("NOK : %s %s : failed to create (errorcode:7)", localHole->cHoleGroupName, staticHoleTable->cHoleID);
		displayError(localHole, iHoleTableCnt);
		return status;
	}

	ProSelection selSurf;

	double dHeightParam = 0;
	searchParameters(PARAMETER_FRAME_HEIGHT, *top_asm, &dHeightParam);

	if (dZPoint == 0) {
		// フレームの上部or下部に穴をあける
		iUnderUpperFlag = 1;

		if (iHoleFlag == 1 || iHoleFlag == 4) {
			// フレーム下部
			selSurf = selSurfYLower;
		}
		else if (iHoleFlag == 2 || iHoleFlag == 3) {
			// フレーム上部
			selSurf = selSurfYUpper;
		}
		dtmPlnFeature_YZ = dtmPlnFeature_Z;

		// 穴あけ表面（selSurfYLower/selSurfYUpper）を使用
		status = createHoleAroundPlane(*top_asm, mdlPart, dtmPlnFeature_X, dtmPlnFeature_YZ, selSurf, dtmPlnFeature_selSurf, 0, 0, dDiameter, &holeFeatureOne);
	}
	else if (dZPoint >= dHeightParam) {
		// フレームの上部に穴をあける
		iUnderUpperFlag = 1;

		selSurf = selSurfYUpper;
		dtmPlnFeature_YZ = dtmPlnFeature_Z;
		// 穴あけ表面（selSurfYLower/selSurfYUpper）を使用
		status = createHoleAroundPlane(*top_asm, mdlPart, dtmPlnFeature_X, dtmPlnFeature_YZ, selSurf, dtmPlnFeature_selSurf, 0, 0, dDiameter, &holeFeatureOne);
	}
	else {
		//selSurf = selSurfZ;
		dtmPlnFeature_YZ = dtmPlnFeature_Y;

		dtmPlnFeature_selSurf = dtmPlnFeature_Z;

		// 穴あけ表面（selSurfZ）はレジュームで消える可能性があるので、使用せず、データム平面を基準とする
		status = createHoleAroundPlane(*top_asm, mdlPart, dtmPlnFeature_X, dtmPlnFeature_YZ, NULL, dtmPlnFeature_selSurf, 0, 0, dDiameter, &holeFeatureOne);
	}

	/*--------------------------------------------------------------------*\
		穴1とデータム軸を1つのグループにまとめる
	\*--------------------------------------------------------------------*/
	n_sel = 1;
	ProFeature* groupHoleOne = (ProFeature*)calloc(n_sel, sizeof(ProFeature));
	if (!groupHoleOne) {
		// メモリ不足
		LOG_PRINT("NOK : Not enough memory");
		return PRO_TK_GENERAL_ERROR;
	}
	groupHoleOne[0] = holeFeatureOne;
	status = createGroupFunction((ProSolid)mdlPart, wRefHoleGroupName, groupHoleOne, n_sel, &group);
	free(groupHoleOne);

	// ログメッセージ
	if (status == PRO_TK_NO_ERROR) {
		LOG_PRINT("OK  : %s %s", localHole->cHoleGroupName, staticHoleTable->cHoleID);
	}
	else {
		LOG_PRINT("NOK : %s %s : failed to create (errorcode:8)", localHole->cHoleGroupName, staticHoleTable->cHoleID);
		displayError(localHole, iHoleTableCnt);
	}

	// 穴グループのグループ化の準備
	strGroupToFeature appData;
	appData.group = group;
	ProWstringCopy(wRefHoleGroupName, appData.wName, PRO_VALUE_UNUSED);
	status = ProSolidFeatVisit((ProSolid)mdlPart, convertGroupToFeature, NULL, (ProAppData)&appData);
	staticHoleGroupAll[0] = appData.feature;

	/*--------------------------------------------------------------------*\
		穴2以降の作成
	\*--------------------------------------------------------------------*/
	for (int iHoleCnt = 1; iHoleCnt < iHoleTableCnt; iHoleCnt++) {
		staticHoleTable++;
		double distXorg = atof(staticHoleTable->cXCord);
		double distX = atof(staticHoleTable->cXCord);
		double distYorg = atof(staticHoleTable->cYCord);
		double distY = atof(staticHoleTable->cYCord);
		double distZ = atof(staticHoleTable->cZCord);
		double dist = 0;

		// 穴の直径を取得
		if (iHoletype == HOLE) {
			// SideFrameの場合
			dDiameter = atof(staticHoleTable->cFrameDiameter);
		}
		else {
			// innerlinerの場合
			dDiameter = atof(staticHoleTable->cInnerLineDiameter);
		}

		if (iLRFlag == LEFT_FRAME || iLRFlag == BOTH_FRAME) {
			distX = distX;
		}	else {
			distX = distX * -1;
		}

		if (iUnderUpperFlag == 1) {
			// 全 UNDER or 穴1がUPPER
			dist = distY;

			if (iHoleFlag == 3 || iHoleFlag == 4) {
				// フレーム下部
				distX = distX * -1;

			}

			status = createHoleBaseHoleOne(*top_asm, mdlPart, holeFeatureOne, dtmPlnFeature_YZ, selSurf, dtmPlnFeature_Z, distX, dist, dDiameter, &holeFeature);
		}else {
			// 穴1が表面方向
			if (distY > 0 && distZ < 0) {
				// 穴2以降が下面
				if (iLRFlag == LEFT_FRAME || iLRFlag == BOTH_FRAME) {
					distX = distX * -1;
				}
				else {
					distX = distX;
				}

				// 下面
				dist = distY * -1;
				selSurf = selSurfYLower;
				dtmPlnFeature_YZ = dtmPlnFeature_Y;

				status = createHoleBaseHoleOneOther(*top_asm, mdlPart, holeFeatureOne, dtmPlnFeature_Z, selSurf, distX, dist, dDiameter, &holeFeature);
			}
			else {
				// 穴2以降も表面

				/*--------------------------------------------------------------------*\
					穴2以降の面が変わることがあるため、X座標で確認する
					(表面は表面だが、斜め面に差し掛かることがある)
				\*--------------------------------------------------------------------*/
				ProSurface		xDatumSurface2;
				double dXHolePoint = dXPoint + distXorg;
				status = getXDatumSurfaceInfoForHole2(&surfInfoData->strFrontInfo, localHole, top_asm, dXHolePoint, &xDatumSurface2);

				// ProModelitemに変換する
				ProModelitem dtmplnConstrMdlItemZ2;
				ProAsmcomppath compPath2;
				int iSurfaceId2;
				status = getAsmcompathIncludePart(*top_asm, mdlPart, &compPath2);
				status = ProSurfaceIdGet(xDatumSurface2, &iSurfaceId2);
				status = ProModelitemInit(mdlPart, iSurfaceId2, PRO_SURFACE, &dtmplnConstrMdlItemZ2);
				TRAIL_PRINT("%s(%d) : ProModelitemInit = %s", __func__, __LINE__, getProErrorMessage(status));


				if (dtmplnConstrMdlItemZ.id != dtmplnConstrMdlItemZ2.id) {
					bAxisFlag = PRO_B_TRUE;
					//表面の中でも面が変わる(斜め面となる)場合
					dtmPlnFeature_YZ = dtmPlnFeature_Y;
					/*--------------------------------------------------------------------*\
						フレーム表面と平行な データム平面の作成
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

					// フレーム表面と平行なデータム平面を作成
					status = createDatumPlaneZ(top_asm, mdlPart, dYPoint_2, dtmplnConstrMdlItemZ2, &dtmPlnFeature_Z2);
					if (status != PRO_TK_NO_ERROR) {
						// フレーム表面と平行なデータム平面の作成に失敗
						LOG_PRINT("NOK : %s %s : failed to create (errorcode:6)", localHole->cHoleGroupName, staticHoleTable->cHoleID);
						displayError(localHole, iHoleTableCnt);

						return status;
					}

					/*--------------------------------------------------------------------*\
						本来穴を開くべき場所の位置に、穴ではなくデータム軸を作成する。
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
						データム軸を基準に穴をあける
					\*--------------------------------------------------------------------*/
					status = createHoleBaseAxis(*top_asm, mdlPart, dtmAxisFeature2, dtmPlnFeature_Z2, 0, 0, dDiameter, &holeFeature);


				}else{
					// 穴2以降も表面
					distX = distX * -1;

					dist = distZ;
					dtmPlnFeature_YZ = dtmPlnFeature_Y;

					status = createHoleBaseHoleOne(*top_asm, mdlPart, holeFeatureOne, dtmPlnFeature_YZ, NULL, dtmPlnFeature_Z, distX, dist, dDiameter, &holeFeature);

				}
			}
		}
		

		// 穴2以降のグループ化。1グループに1穴フィーチャ
		n_sel = 1;
		ProFeature* groupHoleOther = (ProFeature*)calloc(n_sel, sizeof(ProFeature));
		if (!groupHoleOther) {
			// メモリ不足
			LOG_PRINT("NOK : Not enough memory");
			return PRO_TK_GENERAL_ERROR;
		}
		groupHoleOther[0] = holeFeature;
		status = createGroupFunction((ProSolid)mdlPart, wHoleGroupName, groupHoleOther, n_sel, &group);
		free(groupHoleOther);

		// ログメッセージ
		if (status == PRO_TK_NO_ERROR) {
			LOG_PRINT("OK  : %s %s", localHole->cHoleGroupName, staticHoleTable->cHoleID);
		}
		else {
			LOG_PRINT("NOK : %s %s : failed to create", localHole->cHoleGroupName, staticHoleTable->cHoleID);
		}

		// 穴グループのグループ化の準備
		appData.group = group;
		ProWstringCopy(wHoleGroupName, appData.wName, PRO_VALUE_UNUSED);
		status = ProSolidFeatVisit((ProSolid)mdlPart, convertGroupToFeature, NULL, (ProAppData)&appData);
		staticHoleGroupAll[iHoleCnt] = appData.feature;

	}

	/*--------------------------------------------------------------------*\
		*MODULES セクション 穴基準(H)で必要となるので、穴１の場所にCSYSを作成する
	\*--------------------------------------------------------------------*/
	ProCsys csys;
	ProFeature csysFeature;

	// グループ名の取得
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

	// 名前が被ったときに正しい値にならないため、CSYS名を検索して被らないようにする
	UserCsysAppData	serchCsysName;

	ProWstringCopy(wCsysName, wCsysNameBk, PRO_VALUE_UNUSED);
	
	while (TRUE) {
		ProWstringCopy(wCsysNameBk, serchCsysName.csys_name, PRO_VALUE_UNUSED);
		// 初期化する
		ProWstringCopy(wCsysName, wCsysNameBk, PRO_VALUE_UNUSED);

		status = ProSolidFeatVisit((ProSolid)*top_asm,
			NULL,
			CsysNameFindFilterAction,
			(ProAppData)&serchCsysName);

		if (status != PRO_TK_E_NOT_FOUND) {
			// 同名CSYSがある場合は_(連番)をつける
			iGroupNameCnt++;
			ProMdlName wGroupNameCnt;
			_itow_s(iGroupNameCnt, wGroupNameCnt, sizeof(wGroupNameCnt),10);//変換用関数,10進数で変換

			ProWstringConcatenate(L"_", wCsysNameBk, PRO_VALUE_UNUSED);
			ProWstringConcatenate(wGroupNameCnt, wCsysNameBk, PRO_VALUE_UNUSED);
		}
		else {
			break;
		}
	}
	ProWstringCopy(serchCsysName.csys_name, wCsysName, PRO_VALUE_UNUSED);


	if (iLRFlag == BOTH_FRAME) {
		// Topパーツのデータム平面(Z_DATUM_NAME)のフィーチャを検索
		DatumAppData	z_datum;
		z_datum.iFindCnt = 0;
		ProWstringCopy(Z_DATUM_NAME, z_datum.name, PRO_VALUE_UNUSED);
		ProSolidFeatVisit((ProSolid)*top_asm, getFeatureIdAction, NULL, (ProAppData)&z_datum);

		if (z_datum.iFindCnt == 0) {
			// データム平面(Z_DATUM_NAME)のフィーチャが見つからなかった
			LOG_PRINT("NOK : %s : XZ_MAIN Datum Plane does not exist", localHole->cHoleGroupName, staticHoleTable->cHoleID);
			return (PRO_TK_E_NOT_FOUND);
		}
		else {
			// Bの場合はZがXZ_MAINデータム平面になる
			status = createCsysHoleOne(top_asm, z_datum.feature, dtmPlnFeature_X, dtmPlnFeature_Y, wCsysName, iLRFlag, &csysFeature);
		}
	}
	else {
		// LとRの場合はZが自作データム平面になる
		status = createCsysHoleOne(top_asm, dtmPlnFeature_Z, dtmPlnFeature_X, dtmPlnFeature_Y, wCsysName, iLRFlag, &csysFeature);

	}

	/*--------------------------------------------------------------------*\
		データム平面3枚を1つのグループにまとめる
	\*--------------------------------------------------------------------*/
	if (bAxisFlag) {
		// データム軸軸を作成した分もグループにまとめる
		n_sel = 6;
		ProFeature* groupFeat = (ProFeature*)calloc(n_sel, sizeof(ProFeature));
		if (!groupFeat) {
			// メモリ不足
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
		// 標準
		n_sel = 4;
		ProFeature* groupFeat = (ProFeature*)calloc(n_sel, sizeof(ProFeature));
		if (!groupFeat) {
			// メモリ不足
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
		複数の穴グループを1つのグループにまとめる
	\*--------------------------------------------------------------------*/
	status = createGroupFunction((ProSolid)staticHoleGroupAll[0].owner, groupName, staticHoleGroupAll, iHoleTableCnt, &group);
	free(staticHoleGroupAll);

	/*--------------------------------------------------------------------*\
		メモリ開放
	\*--------------------------------------------------------------------*/
	free(startHoleTable);

	return PRO_TK_NO_ERROR;
}

/*====================================================================*\
	FUNCTION :	errorHole()
	PURPOSE  :	穴1のエラーに伴い、穴2以降のエラー表示を行う
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
	PURPOSE  :	ソートされたintの配列にint値を追加/ TestFeats.c 参考　(現状未使用)
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
	PURPOSE  :	アクティブFetureのみをp_featureに設定する/ TestFeats.c 参考
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


	/* ID配列とエラー配列の割当 */
	err = ProArrayAlloc(0, sizeof(int), 1, (ProArray*)&p_feat_id_array);
	if (err != PRO_TK_NO_ERROR) {
		return (PRO_TK_GENERAL_ERROR);
	}

	err = ProArrayAlloc(0, sizeof(ProFeatStatus), 1, (ProArray*)&p_status_array);
	if (err != PRO_TK_NO_ERROR) {
		return (PRO_TK_GENERAL_ERROR);
	}

	/* 指定されたソリッドのフィーチャとフィーチャのステータスリストを取得 */
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
			/* 正常な呼び出し */
		}
		else
		{
			return PRO_TK_E_NOT_FOUND;

			/* 次の場合に実施する
			1.feature_numberの入力値は、モデルで使用可能なアクティブな機能の数よりも大きいが、最大IDよりも小さい
			*/
		}
	}
	else
	{
		return PRO_TK_BAD_CONTEXT;
		/* 次の場合に実施する
		1.ProSolidFeatstatusGetへの呼び出しが失敗
		2.機能番号の入力値が収集されたIDの数よりも大きい
		*/
	}


}

/*====================================================================*\
FUNCTION : createGroupFunction
PURPOSE  : グループ 作成 / TestFeats.c (ProTestLocalGroupCreate)参考 (動作確認OK)
ProSolid* p_solid	in	グループ対象フィーチャが属するソリッド
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
		// 選択したフィーチャのフィーチャ番号を取得
		status = ProFeatureNumberGet((ProFeature*)&featModelitem[i], &feat_num);
		if (status != PRO_TK_NO_ERROR) {
			//continue;
			return status;
		}

		// ソートせずにオブジェクト追加
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
		// アクティブフィーチャのみをmodelitemに設定
		status = ProUtilFeatByNumberInit(p_solid, feat_arr[i], (ProFeature*)&modelitem);

		ProArrayObjectAdd((ProArray*)&p_feat_id_array, PRO_VALUE_UNUSED, 1, &modelitem.id);
	}

	// グループの作成
	status = ProLocalGroupCreate((ProSolid)modelitem.owner, p_feat_id_array, iSel, wGroupName, group);

	/*-----------------------------------------------------------------*\
		メモリの開放
	\*-----------------------------------------------------------------*/
	status = ProArrayFree((ProArray*)&feat_arr);
	status = ProArrayFree((ProArray*)&p_feat_id_array);

	return PRO_TK_NO_ERROR;
}
/*====================================================================*\
FUNCTION : createCsysHoleOne
PURPOSE  : 穴1のCSYSの作成
 *curMdl					(in) データム平面をもつ部品/アセンブリ
 ProFeature dtmPlnZFeature  (in) 穴1のデータム平面Z
 ProFeature dtmPlnXFeature  (in) 穴1のデータム平面X
 ProFeature dtmPlnYFeature  (in) 穴1のデータム平面Y
 ProMdlName csysName		(in) csys名
 int		iLRFlag			(in) Both,Left,Right
 ProFeature* csysFeature	(out) 作成したCSYSのハンドル
 備考
 穴1をあけるために作成した３枚のデータム平面を使用して、CSYSを作成する。
 CSYSは *MODULE セクションの穴基準(H)の時に使用する
\*====================================================================*/
ProError  createCsysHoleOne(ProMdl* curMdl, ProFeature dtmPlnZFeature, ProFeature dtmPlnXFeature, ProFeature dtmPlnYFeature, ProMdlName csysName, int iLRFlag ,ProFeature* csysFeature)
{
	ProError status = PRO_TK_NO_ERROR;

	ProElement featElemTree;
	status = ProElementAlloc(PRO_E_FEATURE_TREE, &featElemTree);

	// フィーチャタイプ (CSYS) PRO_E_FEATURE_TYPE
	ProElement featureTypeElem;
	status = ProElementAlloc(PRO_E_FEATURE_TYPE, &featureTypeElem);
	status = ProElementIntegerSet(featureTypeElem, PRO_FEAT_CSYS);
	status = ProElemtreeElementAdd(featElemTree, NULL, featureTypeElem);

	// フィーチャ名 の設定 PRO_E_STD_FEATURE_NAME
	ProElement featureNameElem;
	status = ProElementAlloc(PRO_E_STD_FEATURE_NAME, &featureNameElem);
	status = ProElementWstringSet(featureNameElem, csysName);
	status = ProElemtreeElementAdd(featElemTree, NULL, featureNameElem);

	//PRO_E_CSYS_ORIGIN_CONSTRS
	ProElement pro_e_csys_origin_constrs;
	status = ProElementAlloc(PRO_E_CSYS_ORIGIN_CONSTRS, &pro_e_csys_origin_constrs);
	status = ProElemtreeElementAdd(featElemTree, NULL, pro_e_csys_origin_constrs);

	/*--------------------------------------------------------------------*\
		データム平面Z
	\*--------------------------------------------------------------------*/
	//PRO_E_CSYS_ORIGIN_CONSTRS
	//  |--PRO_E_CSYS_ORIGIN_CONSTR
	ProElement pro_e_csys_origin_constr_z;
	status = ProElementAlloc(PRO_E_CSYS_ORIGIN_CONSTR, &pro_e_csys_origin_constr_z);
	status = ProElemtreeElementAdd(pro_e_csys_origin_constrs, NULL, pro_e_csys_origin_constr_z);

	//PRO_E_CSYS_ORIGIN_CONSTRS
	//  |--PRO_E_CSYS_ORIGIN_CONSTR
	//    |--PRO_E_CSYS_ORIGIN_CONSTR_REF　オリジナル参照
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
		データム平面X
	\*--------------------------------------------------------------------*/
	//PRO_E_CSYS_ORIGIN_CONSTRS
	//  |--PRO_E_CSYS_ORIGIN_CONSTR

	ProElement pro_e_csys_origin_constr_x;
	status = ProElementAlloc(PRO_E_CSYS_ORIGIN_CONSTR, &pro_e_csys_origin_constr_x);
	status = ProElemtreeElementAdd(pro_e_csys_origin_constrs, NULL, pro_e_csys_origin_constr_x);

	//PRO_E_CSYS_ORIGIN_CONSTRS
	//  |--PRO_E_CSYS_ORIGIN_CONSTR
	//    |--PRO_E_CSYS_ORIGIN_CONSTR_REF　オリジナル参照
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
		データム平面Y
	\*--------------------------------------------------------------------*/
	//PRO_E_CSYS_ORIGIN_CONSTRS
	//  |--PRO_E_CSYS_ORIGIN_CONSTR

	ProElement pro_e_csys_origin_constr_y;
	status = ProElementAlloc(PRO_E_CSYS_ORIGIN_CONSTR, &pro_e_csys_origin_constr_y);
	status = ProElemtreeElementAdd(pro_e_csys_origin_constrs, NULL, pro_e_csys_origin_constr_y);

	//PRO_E_CSYS_ORIGIN_CONSTRS
	//  |--PRO_E_CSYS_ORIGIN_CONSTR
	//    |--PRO_E_CSYS_ORIGIN_CONSTR_REF　オリジナル参照
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
		回転方向/参照の選択
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

	//PRO_E_CSYS_ORIENTSELAXIS1_FLIP(反転の有無)
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

	// 要素ツリーからフィーチャを作成する
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
PURPOSE  : X軸のデータム平面の作成
 *curMdl		(in) データム平面(XM)をもつ部品/アセンブリ
 offsetValue	(in) データム平面の距離
 dtmPlnFeature	(out) データム平面のハンドル
 dtmPlnSelection(out) データム平面の判別
 備考
 Topパーツのデータム平面(XM) を基準にしてデータム平面を作成する
\*====================================================================*/
ProError  createDatumPlaneX(ProMdl* curMdl, double offsetValue, ProFeature* dtmPlnFeature, int iDatumType)
{
	ProError status = PRO_TK_NO_ERROR;

	ProElement featElemTree;
	status = ProElementAlloc(PRO_E_FEATURE_TREE, &featElemTree);

	// フィーチャタイプ (データム) PRO_E_FEATURE_TYPE
	ProElement featureTypeElem;
	status = ProElementAlloc(PRO_E_FEATURE_TYPE, &featureTypeElem);
	status = ProElementIntegerSet(featureTypeElem, PRO_FEAT_DATUM);
	status = ProElemtreeElementAdd(featElemTree, NULL, featureTypeElem);

	// フィーチャ名 の設定 PRO_E_STD_FEATURE_NAME 未設定のため、デフォルト
	ProElement featureNameElem;
	wchar_t* featureName = L"ADTM";
	status = ProElementAlloc(PRO_E_STD_FEATURE_NAME, &featureNameElem);
	status = ProElementWstringSet(featureNameElem, featureName);
	status = ProElemtreeElementAdd(featElemTree, NULL, featureNameElem);

	////////////////////////////////////////////////////////////////////
	// データム配置の設定 
	// 
	//PRO_E_DTMPLN_CONSTRAINTS
	//  |--PRO_E_DTMPLN_CONSTRAINT
	//    |--PRO_E_DTMPLN_CONSTR_TYPE　平面に沿ったオフセット
	//    |--PRO_E_DTMPLN_CONSTR_REF   軸となる平面の定義
	//    |--PRO_E_DTMPLN_CONSTR_REF_OFFSET	オフセット値

	ProElement dtmplnConstraintsElem;
	status = ProElementAlloc(PRO_E_DTMPLN_CONSTRAINTS, &dtmplnConstraintsElem);
	status = ProElemtreeElementAdd(featElemTree, NULL, dtmplnConstraintsElem);

	ProElement* dtmplnConstraints = NULL;
	status = ProArrayAlloc(0, sizeof(ProElement), 1, (ProArray*)&dtmplnConstraints);

	ProElement dtmplnConstraintElem1;
	status = ProElementAlloc(PRO_E_DTMPLN_CONSTRAINT, &dtmplnConstraintElem1);

	// データム平面の作成種類 (平面に沿ったオフセット)
	ProElement dtmplnConstrTypeElem;
	status = ProElementAlloc(PRO_E_DTMPLN_CONSTR_TYPE, &dtmplnConstrTypeElem);
	status = ProElementIntegerSet(dtmplnConstrTypeElem, PRO_DTMPLN_OFFS);
	status = ProElemtreeElementAdd(dtmplnConstraintElem1, NULL, dtmplnConstrTypeElem);

	DatumAppData	appdataFeature;

	if (iDatumType == 0) {
		// 検索するデータム平面の名前を設定
		ProWstringCopy(X_DATUM_NAME_0, appdataFeature.name, PRO_VALUE_UNUSED);

	}
	else if (iDatumType == 1) {
		// 検索するデータム平面の名前を設定
		ProWstringCopy(X_DATUM_NAME_1, appdataFeature.name, PRO_VALUE_UNUSED);

	}
	else if (iDatumType == 2) {
		// 検索するデータム平面の名前を設定
		ProWstringCopy(X_DATUM_NAME_2, appdataFeature.name, PRO_VALUE_UNUSED);

	}
	else if (iDatumType == 3) {
		// 検索するデータム平面の名前を設定
		ProWstringCopy(X_DATUM_NAME_3, appdataFeature.name, PRO_VALUE_UNUSED);

	}
	else if (iDatumType == 4) {
		// 検索するデータム平面の名前を設定
		ProWstringCopy(X_DATUM_NAME_4, appdataFeature.name, PRO_VALUE_UNUSED);

	}
	else {
		// X座標の基準となるデータム平面が想定外の値
		return PRO_TK_BAD_INPUTS;
	}

	// 初期化する
	appdataFeature.iFindCnt = 0;

	// Topパーツのデータム平面(X_DATUM_NAME)のフィーチャを検索
	ProSolidFeatVisit((ProSolid)*curMdl, getFeatureIdAction, NULL, (ProAppData)&appdataFeature);

	if (appdataFeature.iFindCnt == 0) {
		// データム平面(X_DATUM_NAME)のフィーチャが見つからなかった
		return (PRO_TK_E_NOT_FOUND);
	}
	
	// フィーチャを PRO_SURFACE のProModelitemに変換する
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

	// 参照値 
	ProElement dtmplnConstrRefElem;
	status = ProElementAlloc(PRO_E_DTMPLN_CONSTR_REF, &dtmplnConstrRefElem);
	status = ProElementReferenceSet(dtmplnConstrRefElem, dtmplnConstrRef);
	status = ProElemtreeElementAdd(dtmplnConstraintElem1, NULL, dtmplnConstrRefElem);

	// オフセット値 
	ProElement dtmplnOffCsysOffsetElem;
	status = ProElementAlloc(PRO_E_DTMPLN_CONSTR_REF_OFFSET, &dtmplnOffCsysOffsetElem);
	status = ProElementDoubleSet(dtmplnOffCsysOffsetElem, offsetValue);
	status = ProElemtreeElementAdd(dtmplnConstraintElem1, NULL, dtmplnOffCsysOffsetElem);

	status = ProArrayObjectAdd((ProArray*)&dtmplnConstraints, PRO_VALUE_UNUSED, 1, &dtmplnConstraintElem1);

	//Set value for PRO_E_DTMPLN_CONSTRAINTS element.
	status = ProElementArraySet(dtmplnConstraintsElem, NULL, dtmplnConstraints);

	// 反転方向 
	ProElement dtmplnFlipDirElem;
	status = ProElementAlloc(PRO_E_DTMPLN_FLIP_DIR, &dtmplnFlipDirElem);
	status = ProElementIntegerSet(dtmplnFlipDirElem, PRO_DTMPLN_FLIP_DIR_NO);
	status = ProElemtreeElementAdd(featElemTree, NULL, dtmplnFlipDirElem);

	// 要素ツリーからフィーチャを作成する
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
PURPOSE  : Y軸のデータム平面の作成
 *curMdl		(in) 穴あけパーツ(***_1/2)をもつ部品/アセンブリ
 mdlPart		(in) 穴あけパーツProMdl
 offsetValue	(in) データム平面の距離
 dtmPlnFeature	(out) データム平面のハンドル
 備考
 指定穴あけパーツのサーフェイスを基準にしてデータム平面を作成する
\*====================================================================*/
ProError  createDatumPlaneY(ProMdl* curMdl, ProMdl mdlPart, double offsetValue, ProFeature* dtmPlnFeature)
{
	ProError status = PRO_TK_NO_ERROR;

	ProElement featElemTree;
	status = ProElementAlloc(PRO_E_FEATURE_TREE, &featElemTree);

	// フィーチャタイプ (データム) PRO_E_FEATURE_TYPE
	ProElement featureTypeElem;
	status = ProElementAlloc(PRO_E_FEATURE_TYPE, &featureTypeElem);
	status = ProElementIntegerSet(featureTypeElem, PRO_FEAT_DATUM);
	status = ProElemtreeElementAdd(featElemTree, NULL, featureTypeElem);

	// フィーチャ名 の設定 PRO_E_STD_FEATURE_NAME 未設定のため、デフォルト
	ProElement featureNameElem;
	wchar_t* featureName = L"ADTM";
	status = ProElementAlloc(PRO_E_STD_FEATURE_NAME, &featureNameElem);
	status = ProElementWstringSet(featureNameElem, featureName);
	status = ProElemtreeElementAdd(featElemTree, NULL, featureNameElem);

	////////////////////////////////////////////////////////////////////
	// データム配置の設定 
	// 
	//PRO_E_DTMPLN_CONSTRAINTS
	//  |--PRO_E_DTMPLN_CONSTRAINT
	//    |--PRO_E_DTMPLN_CONSTR_TYPE　平面に沿ったオフセット
	//    |--PRO_E_DTMPLN_CONSTR_REF   軸となる平面の定義
	//    |--PRO_E_DTMPLN_CONSTR_REF_OFFSET	オフセット値

	ProElement dtmplnConstraintsElem;
	status = ProElementAlloc(PRO_E_DTMPLN_CONSTRAINTS, &dtmplnConstraintsElem);
	status = ProElemtreeElementAdd(featElemTree, NULL, dtmplnConstraintsElem);

	ProElement* dtmplnConstraints = NULL;
	status = ProArrayAlloc(0, sizeof(ProElement), 1, (ProArray*)&dtmplnConstraints);

	ProElement dtmplnConstraintElem1;
	status = ProElementAlloc(PRO_E_DTMPLN_CONSTRAINT, &dtmplnConstraintElem1);

	// データム平面の作成種類 (平面に沿ったオフセット)
	ProElement dtmplnConstrTypeElem;
	status = ProElementAlloc(PRO_E_DTMPLN_CONSTR_TYPE, &dtmplnConstrTypeElem);
	status = ProElementIntegerSet(dtmplnConstrTypeElem, PRO_DTMPLN_OFFS);
	status = ProElemtreeElementAdd(dtmplnConstraintElem1, NULL, dtmplnConstrTypeElem);

	/*--------------------------------------------------------------------*\
		サーフェスの情報を取得し、ProModelitemを取得する
	\*--------------------------------------------------------------------*/
	SurfaceAppData appdataSurface;

	// 各値の初期化 double型の最大値で初期化する
	appdataSurface.dYPoint = DBL_MAX;

	// 基準となるサーフェイスを取得
	status = ProSolidSurfaceVisit(ProMdlToSolid(mdlPart),
		(ProSurfaceVisitAction)getLowerSurfaceFromYpointAction,
		(ProSurfaceFilterAction)NULL,
		(ProAppData)&appdataSurface);


	if (status != PRO_TK_NO_ERROR) {
		// 基準となるサーフェイスが見つからなかった
		return (PRO_TK_E_NOT_FOUND);
	}

	// ProModelitemに変換する
	ProModelitem dtmplnConstrMdlItem;
	int iSurfaceId;
	status = ProSurfaceIdGet(appdataSurface.surface, &iSurfaceId);
	status = ProModelitemInit(mdlPart , iSurfaceId, PRO_SURFACE, &dtmplnConstrMdlItem);

	/*--------------------------------------------------------------------*\
		フィーチャの情報を取得し、ProAsmcompathデータ構造を初期化する
	\*--------------------------------------------------------------------*/
	ProAsmcomppath compPath;
	getAsmcompathIncludePart(*curMdl, mdlPart, &compPath);

	ProSelection dtmplnConstrSel;
	status = ProSelectionAlloc(&compPath, &dtmplnConstrMdlItem, &dtmplnConstrSel);

	ProReference dtmplnConstrRef;
	status = ProSelectionToReference(dtmplnConstrSel, &dtmplnConstrRef);

	// 参照値 
	ProElement dtmplnConstrRefElem;
	status = ProElementAlloc(PRO_E_DTMPLN_CONSTR_REF, &dtmplnConstrRefElem);
	status = ProElementReferenceSet(dtmplnConstrRefElem, dtmplnConstrRef);
	status = ProElemtreeElementAdd(dtmplnConstraintElem1, NULL, dtmplnConstrRefElem);

	// オフセット値 
	ProElement dtmplnOffCsysOffsetElem;
	status = ProElementAlloc(PRO_E_DTMPLN_CONSTR_REF_OFFSET, &dtmplnOffCsysOffsetElem);
	status = ProElementDoubleSet(dtmplnOffCsysOffsetElem, offsetValue);
	status = ProElemtreeElementAdd(dtmplnConstraintElem1, NULL, dtmplnOffCsysOffsetElem);

	status = ProArrayObjectAdd((ProArray*)&dtmplnConstraints, PRO_VALUE_UNUSED, 1, &dtmplnConstraintElem1);

	//Set value for PRO_E_DTMPLN_CONSTRAINTS element.
	status = ProElementArraySet(dtmplnConstraintsElem, NULL, dtmplnConstraints);

	// 反転方向 
	ProElement dtmplnFlipDirElem;
	status = ProElementAlloc(PRO_E_DTMPLN_FLIP_DIR, &dtmplnFlipDirElem);
	status = ProElementIntegerSet(dtmplnFlipDirElem, PRO_DTMPLN_FLIP_DIR_NO);
	status = ProElemtreeElementAdd(featElemTree, NULL, dtmplnFlipDirElem);

	// 要素ツリーからフィーチャを作成する
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
PURPOSE  : Z軸のデータム平面の作成
 *curMdl		(in) 穴あけパーツ(***_1/2)をもつ部品/アセンブリ
 mdlPart		(in) 穴あけパーツProMdl
 offsetValue	(in) データム平面の距離
 dtmplnConstrMdlItem(in) modelitem
 dtmPlnFeature	(out) データム平面のハンドル
 備考
 指定穴あけパーツのサーフェイスを基準にしてデータム平面を作成する
\*====================================================================*/
ProError  createDatumPlaneZ(ProMdl* curMdl, ProMdl mdlPart, double offsetValue, ProModelitem dtmplnConstrMdlItem, ProFeature* dtmPlnFeature)
{
	ProError status = PRO_TK_NO_ERROR;

	ProElement featElemTree;
	status = ProElementAlloc(PRO_E_FEATURE_TREE, &featElemTree);

	// フィーチャタイプ (データム) PRO_E_FEATURE_TYPE
	ProElement featureTypeElem;
	status = ProElementAlloc(PRO_E_FEATURE_TYPE, &featureTypeElem);
	status = ProElementIntegerSet(featureTypeElem, PRO_FEAT_DATUM);
	status = ProElemtreeElementAdd(featElemTree, NULL, featureTypeElem);

	// フィーチャ名 の設定 PRO_E_STD_FEATURE_NAME 未設定のため、デフォルト
	ProElement featureNameElem;
	wchar_t* featureName = L"ADTM";
	status = ProElementAlloc(PRO_E_STD_FEATURE_NAME, &featureNameElem);
	status = ProElementWstringSet(featureNameElem, featureName);
	status = ProElemtreeElementAdd(featElemTree, NULL, featureNameElem);

	////////////////////////////////////////////////////////////////////
	// データム配置の設定 
	// 
	//PRO_E_DTMPLN_CONSTRAINTS
	//  |--PRO_E_DTMPLN_CONSTRAINT
	//    |--PRO_E_DTMPLN_CONSTR_TYPE　平面に沿ったオフセット
	//    |--PRO_E_DTMPLN_CONSTR_REF   軸となる平面の定義
	//    |--PRO_E_DTMPLN_CONSTR_REF_OFFSET	オフセット値

	ProElement dtmplnConstraintsElem;
	status = ProElementAlloc(PRO_E_DTMPLN_CONSTRAINTS, &dtmplnConstraintsElem);
	status = ProElemtreeElementAdd(featElemTree, NULL, dtmplnConstraintsElem);

	ProElement* dtmplnConstraints = NULL;
	status = ProArrayAlloc(0, sizeof(ProElement), 1, (ProArray*)&dtmplnConstraints);

	ProElement dtmplnConstraintElem1;
	status = ProElementAlloc(PRO_E_DTMPLN_CONSTRAINT, &dtmplnConstraintElem1);

	// データム平面の作成種類 (平面に沿ったオフセット)
	ProElement dtmplnConstrTypeElem;
	status = ProElementAlloc(PRO_E_DTMPLN_CONSTR_TYPE, &dtmplnConstrTypeElem);
	status = ProElementIntegerSet(dtmplnConstrTypeElem, PRO_DTMPLN_OFFS);
	status = ProElemtreeElementAdd(dtmplnConstraintElem1, NULL, dtmplnConstrTypeElem);

	/*--------------------------------------------------------------------*\
		フィーチャの情報を取得し、ProAsmcompathデータ構造を初期化する
	\*--------------------------------------------------------------------*/
	ProAsmcomppath compPath;
	getAsmcompathIncludePart(*curMdl, mdlPart, &compPath);

	ProSelection dtmplnConstrSel;
	status = ProSelectionAlloc(&compPath, &dtmplnConstrMdlItem, &dtmplnConstrSel);

	ProReference dtmplnConstrRef;
	status = ProSelectionToReference(dtmplnConstrSel, &dtmplnConstrRef);

	// 参照値 
	ProElement dtmplnConstrRefElem;
	status = ProElementAlloc(PRO_E_DTMPLN_CONSTR_REF, &dtmplnConstrRefElem);
	status = ProElementReferenceSet(dtmplnConstrRefElem, dtmplnConstrRef);
	status = ProElemtreeElementAdd(dtmplnConstraintElem1, NULL, dtmplnConstrRefElem);

	// オフセット値 
	ProElement dtmplnOffCsysOffsetElem;
	status = ProElementAlloc(PRO_E_DTMPLN_CONSTR_REF_OFFSET, &dtmplnOffCsysOffsetElem);
	status = ProElementDoubleSet(dtmplnOffCsysOffsetElem, offsetValue);
	status = ProElemtreeElementAdd(dtmplnConstraintElem1, NULL, dtmplnOffCsysOffsetElem);

	status = ProArrayObjectAdd((ProArray*)&dtmplnConstraints, PRO_VALUE_UNUSED, 1, &dtmplnConstraintElem1);

	//Set value for PRO_E_DTMPLN_CONSTRAINTS element.
	status = ProElementArraySet(dtmplnConstraintsElem, NULL, dtmplnConstraints);

	// 反転方向 
	ProElement dtmplnFlipDirElem;
	status = ProElementAlloc(PRO_E_DTMPLN_FLIP_DIR, &dtmplnFlipDirElem);
	status = ProElementIntegerSet(dtmplnFlipDirElem, PRO_DTMPLN_FLIP_DIR_NO);
	status = ProElemtreeElementAdd(featElemTree, NULL, dtmplnFlipDirElem);

	// 要素ツリーからフィーチャを作成する
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
PURPOSE  : データム平面を基準に穴を作成 (穴1の作成)
	ProMdl top_asm,					in topアセンブリ
	ProMdl part						in 穴をあけるパーツ
	ProFeature dtmPlnFeature_X,		in データム面フィーチャ(X軸)
	ProFeature dtmPlnFeature_YZ,	in データム面フィーチャ(Y軸)
	ProSelection selSurf			in 配置面/穴をあける面
	double distX,					in 穴1からの距離(X軸)
	double distY					in 穴1からの距離(Y軸)
	double dDiameter				in 穴直径
	ProFeature* holeFeature			out 穴フィーチャ
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
	 要素ツリー作成の開始
	*************************************************/
	/* 要素ツリー(ルート要素)の追加 */
	status = ProElementAlloc(PRO_E_FEATURE_TREE, &feat_elemtree);

	/***********************************************
	 PRO_E_FEATURE_TYPE
	*************************************************/
	/* 要素ツリーに フィーチャタイプ(穴) を追加する */
	status = ProElementAlloc(PRO_E_FEATURE_TYPE, &elem_feattype);
	status = ProElementIntegerSet(elem_feattype, PRO_FEAT_HOLE);
	status = ProElemtreeElementAdd(feat_elemtree, NULL, elem_feattype);

	/***********************************************
	 PRO_E_FEATURE_FORM
	*************************************************/
	/* 要素ツリーに フィーチャフォーム(ストレートホール) を追加する */
	status = ProElementAlloc(PRO_E_FEATURE_FORM, &elem_featform);
	status = ProElementIntegerSet(elem_featform, PRO_HLE_TYPE_STRAIGHT);
	status = ProElemtreeElementAdd(feat_elemtree, NULL, elem_featform);

	/***********************************************
	 PRO_E_HLE_COM
	*************************************************/
	/* 要素ツリーに 穴情報の共通要素 の追加  */
	status = ProElementAlloc(PRO_E_HLE_COM, &elem_hle_com);
	status = ProElemtreeElementAdd(feat_elemtree, NULL, elem_hle_com);

	/* 穴情報 (穴タイプ:ストレート) を指定する */
	status = ProElementAlloc(PRO_E_HLE_TYPE_NEW, &elem_hle_type_new);
	status = ProElementIntegerSet(elem_hle_type_new, PRO_HLE_NEW_TYPE_STRAIGHT);
	status = ProElemtreeElementAdd(elem_hle_com, NULL, elem_hle_type_new);

	/* 穴情報 (穴の直径) を指定する */
	status = ProElementAlloc(PRO_E_DIAMETER, &elem_diameter);
	status = ProElementDoubleSet(elem_diameter, dDiameter);
	status = ProElemtreeElementAdd(elem_hle_com, NULL, elem_diameter);

	/* 標準深度の要素を追加する

			  |--PRO_E_HOLE_STD_DEPTH
			  |    |--PRO_E_HOLE_DEPTH_TO
			  |    |    |--PRO_E_HOLE_DEPTH_TO_TYPE
			  |    |    |--PRO_E_EXT_DEPTH_TO_VALUE
			  |    |--PRO_E_HOLE_DEPTH_FROM
			  |         |--PRO_E_HOLE_DEPTH_FROM_TYPE
			  |         |--PRO_E_EXT_DEPTH_FROM_VALUE
	*/

	// 深さ要素
	status = ProElementAlloc(PRO_E_HOLE_STD_DEPTH, &elem_hole_std_depth);
	status = ProElemtreeElementAdd(elem_hle_com, NULL, elem_hole_std_depth);

	// side1 情報： Blind 深さ25
	status = ProElementAlloc(PRO_E_HOLE_DEPTH_TO, &elem_hole_depth_to);
	status = ProElemtreeElementAdd(elem_hole_std_depth, NULL, elem_hole_depth_to);
	status = ProElementAlloc(PRO_E_HOLE_DEPTH_TO_TYPE, &elem_hole_depth_to_type);
	status = ProElementIntegerSet(elem_hole_depth_to_type, PRO_HLE_STRGHT_BLIND_DEPTH);
	status = ProElemtreeElementAdd(elem_hole_depth_to, NULL, elem_hole_depth_to_type);
	status = ProElementAlloc(PRO_E_EXT_DEPTH_TO_VALUE, &elem_hole_depth_to_value);
	status = ProElementDoubleSet(elem_hole_depth_to_value, 25.0);
	status = ProElemtreeElementAdd(elem_hole_depth_to, NULL, elem_hole_depth_to_value);
	// side2 情報： Blind 深さ25
	status = ProElementAlloc(PRO_E_HOLE_DEPTH_FROM, &elem_hole_depth_from);
	status = ProElemtreeElementAdd(elem_hole_std_depth, NULL, elem_hole_depth_from);
	status = ProElementAlloc(PRO_E_HOLE_DEPTH_FROM_TYPE, &elem_hole_depth_from_type);
	status = ProElementIntegerSet(elem_hole_depth_from_type, PRO_HLE_STRGHT_BLIND_DEPTH);
	status = ProElemtreeElementAdd(elem_hole_depth_from, NULL, elem_hole_depth_from_type);
	status = ProElementAlloc(PRO_E_EXT_DEPTH_FROM_VALUE, &elem_hole_depth_from_value);
	status = ProElementDoubleSet(elem_hole_depth_from_value, 25.0);
	status = ProElemtreeElementAdd(elem_hole_depth_from, NULL, elem_hole_depth_from_value);


	/* 配置の詳細に関連する要素を追加する
	 軸を主基準とする同軸穴

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

	// 穴配置オプション
	status = ProElementAlloc(PRO_E_HLE_PL_TYPE, &elem_hle_pl_type);
	status = ProElementIntegerSet(elem_hle_pl_type, PRO_HLE_PL_TYPE_LIN);
	status = ProElemtreeElementAdd(elem_hle_placement, NULL, elem_hle_pl_type);

	// 二次選択 (X軸)
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

	// 距離 1
	status = ProElementAlloc(PRO_E_HLE_DIM_DIST1, &elem_hle_dim_dist1);
	status = ProElementDoubleSet(elem_hle_dim_dist1, distX);
	status = ProElemtreeElementAdd(elem_hle_placement, NULL, elem_hle_dim_dist1);

	// 三次選択 (Y軸)
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

	// 距離 2
	status = ProElementAlloc(PRO_E_HLE_DIM_DIST2, &elem_hle_dim_dist2);
	status = ProElementDoubleSet(elem_hle_dim_dist2, distY);
	status = ProElemtreeElementAdd(elem_hle_placement, NULL, elem_hle_dim_dist2);
	/* 要素ツリー作成の終了 */


	/***********************************************
	 フィーチャーの作成
	*************************************************/
	ProAsmcomppath compPath;
	status = getAsmcompathIncludePart(top_asm, part, &compPath);
	if (status != PRO_TK_NO_ERROR) {
		// パーツのフィーチャが見つからなかった
		return (PRO_TK_E_NOT_FOUND);
	}

	status = ProMdlToModelitem(part, &model_item);
	status = ProSelectionAlloc(&compPath, &model_item, &model_selection);
	status = ProArrayAlloc(1, sizeof(ProFeatureCreateOptions), 1, (ProArray*)&options);

	options[0] = PRO_FEAT_CR_NO_OPTS;
	status = ProFeatureWithoptionsCreate(model_selection, feat_elemtree, options, PRO_REGEN_NO_FLAGS, holeFeature, &p_errors);
	TRAIL_PRINT("%s(%d) : ProFeatureWithoptionsCreate = %s", __func__, __LINE__, getProErrorMessage(status));

	/***********************************************
	 リソースの解放
	*************************************************/
	status = ProElementFree(&feat_elemtree);
	status = ProArrayFree((ProArray*)&options);

	return (status);
}

/*====================================================================*\
FUNCTION : createHoleBaseHoleOne
PURPOSE  : 穴1を基準に、穴1と同じ面に穴を作成する(穴2以降の作成)
	ProMdl top_asm,					in topアセンブリ
	ProMdl part						in 穴をあけるパーツ
	ProFeature holeFeatureOne,		in 基準となる穴1のフィーチャ
	ProFeature dtmPlnFeature_YZ,	in 寸法回転方向となるフィーチャ
	ProSelection selSurf			in 配置面/穴をあける面
	double distX,					in 穴1からの距離(X軸)
	double distY					in 穴1からの距離(Y軸)
	double dDiameter				in 穴直径
	ProFeature* holeFeature			out 穴フィーチャ
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
	 要素ツリー作成の開始
	*************************************************/
	/* 要素ツリー(ルート要素)の追加 */
	status = ProElementAlloc(PRO_E_FEATURE_TREE, &feat_elemtree);

	/***********************************************
	 PRO_E_FEATURE_TYPE
	*************************************************/
	/* 要素ツリーに フィーチャタイプ(穴) を追加する */
	status = ProElementAlloc(PRO_E_FEATURE_TYPE, &elem_feattype);
	status = ProElementIntegerSet(elem_feattype, PRO_FEAT_HOLE);
	status = ProElemtreeElementAdd(feat_elemtree, NULL, elem_feattype);

	/***********************************************
	 PRO_E_FEATURE_FORM
	*************************************************/
	/* 要素ツリーに フィーチャフォーム(ストレートホール) を追加する */
	status = ProElementAlloc(PRO_E_FEATURE_FORM, &elem_featform);
	status = ProElementIntegerSet(elem_featform, PRO_HLE_TYPE_STRAIGHT);
	status = ProElemtreeElementAdd(feat_elemtree, NULL, elem_featform);

	/***********************************************
	 PRO_E_HLE_COM
	*************************************************/
	/* 要素ツリーに 穴情報の共通要素 の追加  */
	status = ProElementAlloc(PRO_E_HLE_COM, &elem_hle_com);
	status = ProElemtreeElementAdd(feat_elemtree, NULL, elem_hle_com);

	/* 穴情報 (穴タイプ:ストレート) を指定する */
	status = ProElementAlloc(PRO_E_HLE_TYPE_NEW, &elem_hle_type_new);
	status = ProElementIntegerSet(elem_hle_type_new, PRO_HLE_NEW_TYPE_STRAIGHT);
	status = ProElemtreeElementAdd(elem_hle_com, NULL, elem_hle_type_new);

	/* 穴情報 (穴の直径) を指定する */
	status = ProElementAlloc(PRO_E_DIAMETER, &elem_diameter);
	status = ProElementDoubleSet(elem_diameter, dDiameter);
	status = ProElemtreeElementAdd(elem_hle_com, NULL, elem_diameter);

	/* 標準深度の要素を追加する

			  |--PRO_E_HOLE_STD_DEPTH
			  |    |--PRO_E_HOLE_DEPTH_TO
			  |    |    |--PRO_E_HOLE_DEPTH_TO_TYPE
			  |    |    |--PRO_E_EXT_DEPTH_TO_VALUE
			  |    |--PRO_E_HOLE_DEPTH_FROM
			  |         |--PRO_E_HOLE_DEPTH_FROM_TYPE
			  |         |--PRO_E_EXT_DEPTH_FROM_VALUE
	*/

	// 深さ要素
	status = ProElementAlloc(PRO_E_HOLE_STD_DEPTH, &elem_hole_std_depth);
	status = ProElemtreeElementAdd(elem_hle_com, NULL, elem_hole_std_depth);

	// side1 情報： Blind 深さ25
	status = ProElementAlloc(PRO_E_HOLE_DEPTH_TO, &elem_hole_depth_to);
	status = ProElemtreeElementAdd(elem_hole_std_depth, NULL, elem_hole_depth_to);
	status = ProElementAlloc(PRO_E_HOLE_DEPTH_TO_TYPE, &elem_hole_depth_to_type);
	status = ProElementIntegerSet(elem_hole_depth_to_type, PRO_HLE_STRGHT_BLIND_DEPTH);
	status = ProElemtreeElementAdd(elem_hole_depth_to, NULL, elem_hole_depth_to_type);
	status = ProElementAlloc(PRO_E_EXT_DEPTH_TO_VALUE, &elem_hole_depth_to_value);
	status = ProElementDoubleSet(elem_hole_depth_to_value, 25.0);
	status = ProElemtreeElementAdd(elem_hole_depth_to, NULL, elem_hole_depth_to_value);

	// side2 情報： Blind 深さ25
	status = ProElementAlloc(PRO_E_HOLE_DEPTH_FROM, &elem_hole_depth_from);
	status = ProElemtreeElementAdd(elem_hole_std_depth, NULL, elem_hole_depth_from);
	status = ProElementAlloc(PRO_E_HOLE_DEPTH_FROM_TYPE, &elem_hole_depth_from_type);
	status = ProElementIntegerSet(elem_hole_depth_from_type, PRO_HLE_STRGHT_BLIND_DEPTH);
	status = ProElemtreeElementAdd(elem_hole_depth_from, NULL, elem_hole_depth_from_type);
	status = ProElementAlloc(PRO_E_EXT_DEPTH_FROM_VALUE, &elem_hole_depth_from_value);
	status = ProElementDoubleSet(elem_hole_depth_from_value, 25.0);
	status = ProElemtreeElementAdd(elem_hole_depth_from, NULL, elem_hole_depth_from_value);

	/* 配置の詳細に関連する要素を追加する
	 軸を主基準とする同軸穴

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
		// パーツ(patName)のフィーチャが見つからなかった
		return (PRO_TK_E_NOT_FOUND);
	}

	status = ProElementAlloc(PRO_E_HLE_PLACEMENT, &elem_hle_placement);
	status = ProElemtreeElementAdd(feat_elemtree, NULL, elem_hle_placement);

	// 一次選択 (穴あけ面)
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

	// 穴配置オプション
	status = ProElementAlloc(PRO_E_HLE_PL_TYPE, &elem_hle_pl_type);
	status = ProElementIntegerSet(elem_hle_pl_type, PRO_HLE_PL_TYPE_LIN);
	status = ProElemtreeElementAdd(elem_hle_placement, NULL, elem_hle_pl_type);

	// 二次選択 (穴1のデータム軸)
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

	// 距離 1
	status = ProElementAlloc(PRO_E_HLE_DIM_DIST1, &elem_hle_dim_dist1);
	status = ProElementDoubleSet(elem_hle_dim_dist1, distX);
	status = ProElemtreeElementAdd(elem_hle_placement, NULL, elem_hle_dim_dist1);

	// PRO_E_HLE_DIM_REF1 がデータム軸のため、PRO_E_HLE_DIM_REF2 の設定なし

	// 距離 2
	status = ProElementAlloc(PRO_E_HLE_DIM_DIST2, &elem_hle_dim_dist2);
	status = ProElementDoubleSet(elem_hle_dim_dist2, distY);
	status = ProElemtreeElementAdd(elem_hle_placement, NULL, elem_hle_dim_dist2);

	// 配置寸法スキームの方向を定義
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
	/* 要素ツリー作成の終了 */

	/***********************************************
	 フィーチャーの作成
	*************************************************/
	status = ProMdlToModelitem(part, &model_item);
	status = ProSelectionAlloc(&compPath, &model_item, &model_selection);
	status = ProArrayAlloc(1, sizeof(ProFeatureCreateOptions), 1, (ProArray*)&options);
	options[0] = PRO_FEAT_CR_NO_OPTS;
	status = ProFeatureWithoptionsCreate(model_selection, feat_elemtree, options, PRO_REGEN_NO_FLAGS, holeFeature, &p_errors);
	TRAIL_PRINT("%s(%d) : ProFeatureWithoptionsCreate = %s", __func__, __LINE__, getProErrorMessage(status));

	/***********************************************
	 リソースの解放
	*************************************************/
    status = ProElementFree(&feat_elemtree);
	status = ProSelectionFree(&model_selection);
	status = ProArrayFree((ProArray*)&options);

	return (status);
}



/*====================================================================*\
FUNCTION : createHoleBaseHoleOneOther
PURPOSE  : 穴1を基準に、穴1と違う面に穴を作成する (穴2以降の作成)
	ProMdl top_asm,					in topアセンブリ
	ProMdl part						in 穴をあけるパーツ
	ProFeature holeFeatureOne,		in 基準となる穴1のフィーチャ
	ProSelection selOffsetSurf,		in オフセットフィーチャ
	ProSelection selSurf			in 配置面/穴をあける面
	double distX,					in 穴1からの距離(X軸)
	double distY					in 穴1からの距離(Y軸)
	double dDiameter				in 穴直径
	ProFeature* holeFeature			out 穴フィーチャ
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
	 要素ツリー作成の開始
	*************************************************/
	/* 要素ツリー(ルート要素)の追加 */
	status = ProElementAlloc(PRO_E_FEATURE_TREE, &feat_elemtree);

	/***********************************************
	 PRO_E_FEATURE_TYPE
	*************************************************/
	/* 要素ツリーに フィーチャタイプ(穴) を追加する */
	status = ProElementAlloc(PRO_E_FEATURE_TYPE, &elem_feattype);
	status = ProElementIntegerSet(elem_feattype, PRO_FEAT_HOLE);
	status = ProElemtreeElementAdd(feat_elemtree, NULL, elem_feattype);

	/***********************************************
	 PRO_E_FEATURE_FORM
	*************************************************/
	/* 要素ツリーに フィーチャフォーム(ストレートホール) を追加する */
	status = ProElementAlloc(PRO_E_FEATURE_FORM, &elem_featform);
	status = ProElementIntegerSet(elem_featform, PRO_HLE_TYPE_STRAIGHT);
	status = ProElemtreeElementAdd(feat_elemtree, NULL, elem_featform);

	/***********************************************
	 PRO_E_HLE_COM
	*************************************************/
	/* 要素ツリーに 穴情報の共通要素 の追加  */
	status = ProElementAlloc(PRO_E_HLE_COM, &elem_hle_com);
	status = ProElemtreeElementAdd(feat_elemtree, NULL, elem_hle_com);

	/* 穴情報 (穴タイプ:ストレート) を指定する */
	status = ProElementAlloc(PRO_E_HLE_TYPE_NEW, &elem_hle_type_new);
	status = ProElementIntegerSet(elem_hle_type_new, PRO_HLE_NEW_TYPE_STRAIGHT);
	status = ProElemtreeElementAdd(elem_hle_com, NULL, elem_hle_type_new);

	/* 穴情報 (穴の直径) を指定する */
	status = ProElementAlloc(PRO_E_DIAMETER, &elem_diameter);
	status = ProElementDoubleSet(elem_diameter, dDiameter);
	status = ProElemtreeElementAdd(elem_hle_com, NULL, elem_diameter);

	/* 標準深度の要素を追加する

			  |--PRO_E_HOLE_STD_DEPTH
			  |    |--PRO_E_HOLE_DEPTH_TO
			  |    |    |--PRO_E_HOLE_DEPTH_TO_TYPE
			  |    |    |--PRO_E_EXT_DEPTH_TO_VALUE
			  |    |--PRO_E_HOLE_DEPTH_FROM
			  |         |--PRO_E_HOLE_DEPTH_FROM_TYPE
			  |         |--PRO_E_EXT_DEPTH_FROM_VALUE
	*/

	// 深さ要素
	status = ProElementAlloc(PRO_E_HOLE_STD_DEPTH, &elem_hole_std_depth);
	status = ProElemtreeElementAdd(elem_hle_com, NULL, elem_hole_std_depth);

	// side1 情報： Blind 深さ25
	status = ProElementAlloc(PRO_E_HOLE_DEPTH_TO, &elem_hole_depth_to);
	status = ProElemtreeElementAdd(elem_hole_std_depth, NULL, elem_hole_depth_to);
	status = ProElementAlloc(PRO_E_HOLE_DEPTH_TO_TYPE, &elem_hole_depth_to_type);
	status = ProElementIntegerSet(elem_hole_depth_to_type, PRO_HLE_STRGHT_BLIND_DEPTH);
	status = ProElemtreeElementAdd(elem_hole_depth_to, NULL, elem_hole_depth_to_type);
	status = ProElementAlloc(PRO_E_EXT_DEPTH_TO_VALUE, &elem_hole_depth_to_value);
	status = ProElementDoubleSet(elem_hole_depth_to_value, 25.0);
	status = ProElemtreeElementAdd(elem_hole_depth_to, NULL, elem_hole_depth_to_value);

	// side2 情報： Blind 深さ25
	status = ProElementAlloc(PRO_E_HOLE_DEPTH_FROM, &elem_hole_depth_from);
	status = ProElemtreeElementAdd(elem_hole_std_depth, NULL, elem_hole_depth_from);
	status = ProElementAlloc(PRO_E_HOLE_DEPTH_FROM_TYPE, &elem_hole_depth_from_type);
	status = ProElementIntegerSet(elem_hole_depth_from_type, PRO_HLE_STRGHT_BLIND_DEPTH);
	status = ProElemtreeElementAdd(elem_hole_depth_from, NULL, elem_hole_depth_from_type);
	status = ProElementAlloc(PRO_E_EXT_DEPTH_FROM_VALUE, &elem_hole_depth_from_value);
	status = ProElementDoubleSet(elem_hole_depth_from_value, 25.0);
	status = ProElemtreeElementAdd(elem_hole_depth_from, NULL, elem_hole_depth_from_value);


	/* 配置の詳細に関連する要素を追加する

	 軸を主基準とする同軸穴

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
		// パーツ(patName)のフィーチャが見つからなかった
		return (PRO_TK_E_NOT_FOUND);
	}

	status = ProElementAlloc(PRO_E_HLE_PLACEMENT, &elem_hle_placement);
	status = ProElemtreeElementAdd(feat_elemtree, NULL, elem_hle_placement);

	// 一次選択 (穴あけ面)
	status = ProElementAlloc(PRO_E_HLE_PRIM_REF, &elem_hle_prim_ref);

	value_data.type = PRO_VALUE_TYPE_SELECTION;
	value_data.v.r = selSurf;

	status = ProValueAlloc(&value);
	status = ProValueDataSet(value, &value_data);
	status = ProElementValueSet(elem_hle_prim_ref, value);
	status = ProElemtreeElementAdd(elem_hle_placement, NULL, elem_hle_prim_ref);

	// 穴配置オプション
	status = ProElementAlloc(PRO_E_HLE_PL_TYPE, &elem_hle_pl_type);
	status = ProElementIntegerSet(elem_hle_pl_type, PRO_HLE_PL_TYPE_LIN);
	status = ProElemtreeElementAdd(elem_hle_placement, NULL, elem_hle_pl_type);

	// 二次選択 (穴1のデータム軸)
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

	// 距離 1
	status = ProElementAlloc(PRO_E_HLE_DIM_DIST1, &elem_hle_dim_dist1);
	status = ProElementDoubleSet(elem_hle_dim_dist1, distX);
	status = ProElemtreeElementAdd(elem_hle_placement, NULL, elem_hle_dim_dist1);

	// 三次選択 (穴1のデータム軸と同じサーフェイス面)
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

	// 距離 2
	status = ProElementAlloc(PRO_E_HLE_DIM_DIST2, &elem_hle_dim_dist2);
	status = ProElementDoubleSet(elem_hle_dim_dist2, distY);
	status = ProElemtreeElementAdd(elem_hle_placement, NULL, elem_hle_dim_dist2);

	/* 要素ツリー作成の終了 */

	/***********************************************
	 フィーチャーの作成
	*************************************************/
	status = ProMdlToModelitem(part, &model_item);
	status = ProSelectionAlloc(&compPath, &model_item, &model_selection);
	status = ProArrayAlloc(1, sizeof(ProFeatureCreateOptions), 1, (ProArray*)&options);
	options[0] = PRO_FEAT_CR_NO_OPTS;
	status = ProFeatureWithoptionsCreate(model_selection, feat_elemtree, options, PRO_REGEN_NO_FLAGS, holeFeature, &p_errors);
	TRAIL_PRINT("%s(%d) : ProFeatureWithoptionsCreate = %s", __func__, __LINE__, getProErrorMessage(status));

	/***********************************************
	 リソースの解放
	*************************************************/
	status = ProElementFree(&feat_elemtree);
	status = ProSelectionFree(&model_selection);
	status = ProArrayFree((ProArray*)&options);

	return (status);
}


/*=============================================================*\
  Function: 	getHoleTableInfOfSameNum
  Purpose:		HoleTable の情報取得
  InputFileHole* strHole,					in	検索対象の穴情報
  InputFileHoleTable* strHoleTableInput		in	コンフィグレーションファイルの値
  int iHoleTableSectionMaxRows				in  処理すべきコンフィグレーションファイルの行数(Hole_Table)
  int* iHoleTableCnt						out	検索結果の最大行数
  \*=============================================================*/
ProError getHoleTableInfOfSameNum(InputFileHole* strHole, InputFileHoleTable* strHoleTableInput, int iHoleTableSectionMaxRows, int* iHoleTableCnt)
{
	char cHoleID[INPUTFILE_MAXLINE];
	// 開始地点のアドレスを確保
	InputFileHoleTable* HoleTableStart = strHoleTableInput;
	InputFileHoleTable* staticHoleTableStart;
	/*--------------------------------------------------------------------*\
		穴グループの数をカウントする
	\*--------------------------------------------------------------------*/
	for (int iInputMdlCnt = 0; iInputMdlCnt < iHoleTableSectionMaxRows; iInputMdlCnt++) {

		// HoleとHoleTableを比較し、穴グループの数をカウント
		if (strcmp(strHole->cHoleGroupName, strHoleTableInput->cHoleGroupName) == 0) {
			*iHoleTableCnt = *iHoleTableCnt + 1;
		}
		*strHoleTableInput++;
	}
	if (*iHoleTableCnt == 0) {
		// 対象のHoleTableが見つかりませんでした。
		LOG_PRINT("NOK : %s : Not found HoleGroup in HoleTable", strHole->cHoleGroupName);
		return PRO_TK_E_NOT_FOUND;
	}

	/*--------------------------------------------------------------------*\
		穴の数だけメモリを確保する
	\*--------------------------------------------------------------------*/
	staticHoleTable = (InputFileHoleTable*)calloc(*iHoleTableCnt, sizeof(InputFileHoleTable));
	if (!staticHoleTable) {
		// メモリ不足
		LOG_PRINT("NOK : Not enough memory");
		return PRO_TK_GENERAL_ERROR;
	}

	/*--------------------------------------------------------------------*\
		穴グループ用のメモリを確保する
	\*--------------------------------------------------------------------*/
	staticHoleGroupAll = (ProFeature*)calloc(*iHoleTableCnt, sizeof(ProFeature));
	if (!staticHoleGroupAll) {
		// メモリ不足
		LOG_PRINT("NOK : Not enough memory");
		return PRO_TK_GENERAL_ERROR;
	}


	staticHoleTableStart = staticHoleTable;
	strHoleTableInput = HoleTableStart;
	int iRow = 0;
	/*--------------------------------------------------------------------*\
		HoleTableの情報を格納する
	\*--------------------------------------------------------------------*/
	for (int iInputMdlCnt = 0; iInputMdlCnt < iHoleTableSectionMaxRows; iInputMdlCnt++) {
		// HoleとHoleTableを比較し、穴グループが同じ行を検索
		if (strcmp(strHole->cHoleGroupName, strHoleTableInput->cHoleGroupName) == 0) {

			if(iRow>0){
				// 2回目のループからカウントする
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
	// メモリの位置を先頭に戻す
	staticHoleTable = staticHoleTableStart;
	strHoleTableInput = HoleTableStart;
	return PRO_TK_NO_ERROR;
}

/*=============================================================*\
  Function: 	searchParameters
  Purpose:		パラメータの値を検索し取得する
  ProName wParameterName	(in)	検索するパラメータ名の一部
  ProMdl mdlCurrent		(in)	パラメータを持っているモデル
  double* dParam			(out)	検索結果
  \*=============================================================*/
ProError searchParameters(ProName wParameterName, ProMdl mdlCurrent, double* dParam){
	
	ProError status;
	ProModelitem featureParam;
	
	strSearchParameter searchParameter;
	searchParameter.dParam = 0;
	ProWstringCopy(wParameterName, searchParameter.wParameterName, PRO_VALUE_UNUSED);

	// パラメータModelitemの取得
	status = ProMdlToModelitem(mdlCurrent, &featureParam);

	// パラメータの検索
	status = ProParameterVisit(&featureParam, NULL, (ProParameterAction)getParametersValue, (ProAppData)&searchParameter);

	if (status == PRO_TK_CONTINUE) {
		*dParam = searchParameter.dParam;

	}
}

/*====================================================================*\
	FUNCTION :	 getParametersValue()
	PURPOSE  :   パラメータ一覧からパラメータ値の取得
\*====================================================================*/
ProError getParametersValue(ProParameter* param, ProError err, ProAppData app_data)
{
	ProError status;
	ProCharName param_name;
	ProCharName cParameterName;
	ProParamvalue value;
	ProUnititem units;

	// パラメータの名前取得
	ProWstringToString(param_name, param->id);
	ProWstringToString(cParameterName, ((strSearchParameter*)app_data)->wParameterName);

	// パラメータ名を含むパラメータを検索する
	if (strstr(param_name, cParameterName) == NULL) {
		return PRO_TK_NO_ERROR;
	}

	// パラメータを取得
	status = ProParameterValueWithUnitsGet(param, &value, &units);

	if (value.type == PRO_PARAM_DOUBLE) {
		((strSearchParameter*)app_data)->dParam = value.value.d_val;
	}

	return PRO_TK_CONTINUE;
}

/*====================================================================*\
FUNCTION: getSurfaceFromYpointAction()
PURPOSE:  Y軸データム平面の参照平面の検索(フレーム下部)
\*====================================================================*/
ProError getLowerSurfaceFromYpointAction(
	ProSurface surface,
	ProError status,
	ProAppData app_data)
{
	ProSrftype stype;
	ProGeomitemdata* pDataPtr;

	/*--------------------------------------------------------------------*\
		サーフェスが平面でない場合は、スキップ
	\*--------------------------------------------------------------------*/
	status = ProSurfaceTypeGet(surface, &stype);

	if (stype != PRO_SRF_PLANE) {
		return(PRO_TK_NO_ERROR);
	}

	/*--------------------------------------------------------------------*\
		サーフェスの情報を取得
	\*--------------------------------------------------------------------*/
	ProVector xyz_point;
	ProVector normal;

	status = ProSurfaceDataGet(surface, &pDataPtr);

	status = ProSurfaceXyzdataEval(surface, pDataPtr->data.p_surface_data->uv_max,
		xyz_point, NULL, NULL, normal);

	/*--------------------------------------------------------------------*\
		Y座標が最も小さく、かつ	法線の値が (0, -1, 0) であるサーフェイスを検索し、値を取得する
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
PURPOSE:  Y軸データム平面の参照平面の検索(フレーム上部)
\*====================================================================*/
ProError getUpperSurfaceFromYpointAction(
	ProSurface surface,
	ProError status,
	ProAppData app_data)
{
	ProSrftype stype;
	ProGeomitemdata* pDataPtr;

	/*--------------------------------------------------------------------*\
		サーフェスが平面でない場合は、スキップ
	\*--------------------------------------------------------------------*/
	status = ProSurfaceTypeGet(surface, &stype);
	if (stype != PRO_SRF_PLANE) {
		return(PRO_TK_NO_ERROR);
	}

	/*--------------------------------------------------------------------*\
		サーフェスの情報を取得
	\*--------------------------------------------------------------------*/
	ProVector xyz_point;
	ProVector normal;

	status = ProSurfaceDataGet(surface, &pDataPtr);
	status = ProSurfaceXyzdataEval(surface, pDataPtr->data.p_surface_data->uv_max,
		xyz_point, NULL, NULL, normal);

	/*--------------------------------------------------------------------*\
		Y座標が最も大きく、かつ	法線の値が (0, -1, 0) であるサーフェイスを検索し、値を取得する
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
PURPOSE:  穴あけ面の境界座標の検索
\*====================================================================*/
ProError getSurfZpointAction(ProSurface surface,ProError status,ProAppData app_data)
{
	ProSrftype stype;
	ProGeomitemdata* pDataPtr;

	/*--------------------------------------------------------------------*\
		サーフェスが平面でない場合は、スキップ
	\*--------------------------------------------------------------------*/
	status = ProSurfaceTypeGet(surface, &stype);
	if (stype != PRO_SRF_PLANE) {
		return(PRO_TK_NO_ERROR);
	}

	if (((SurfaceInfoAppData*)app_data)->strFrontInfo.iCounter == (MAX_SURFACE * 2)) {
		// 確保した値以上を取得しないように
		return(PRO_TK_NO_ERROR);

	}

	/*--------------------------------------------------------------------*\
		サーフェスの情報を取得
	\*--------------------------------------------------------------------*/
	ProVector xyz_point;
	ProVector normal;

	status = ProSurfaceDataGet(surface, &pDataPtr);
	status = ProSurfaceXyzdataEval(surface, pDataPtr->data.p_surface_data->uv_max,
		xyz_point, NULL, NULL, normal);

	/*--------------------------------------------------------------------*\
	サーフェイスの表面積が100未満の場合は対象外とする
	※対象サーフェイスの切り分けができなかったので、苦肉の策
	　100未満…4台分のトラックデータを確認して値を決めた。10.5より大きく55255.9より小さい値であれば問題ないはず！
	\*--------------------------------------------------------------------*/
	// 表面積
	double dArea;
	ProSurfaceAreaEval(surface, &dArea);
	if (dArea < 100) {
		return(PRO_TK_NO_ERROR);
	}

	/*--------------------------------------------------------------------*\
		Left partの場合、法線のX値が 0 より大きい
		Right partの場合、法線のX値が 0 より小さい
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

		// min/maxがマイナスの場合はプラスにする
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
PURPOSE:  穴あけ面の裏面の境界座標の検索
\*====================================================================*/
ProError searchBackSurfAction(ProSurface surface, ProError status, ProAppData app_data)
{
	ProSrftype stype;
	ProGeomitemdata* pDataPtr;

	/*--------------------------------------------------------------------*\
		サーフェスが平面でない場合は、スキップ
	\*--------------------------------------------------------------------*/
	status = ProSurfaceTypeGet(surface, &stype);
	if (stype != PRO_SRF_PLANE) {
		return(PRO_TK_NO_ERROR);
	}

	/*--------------------------------------------------------------------*\
		サーフェスの情報を取得
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
		 ★ フレームの表面のサーフェイス情報を取得
				Left partの場合、法線のX値が 0 より小さい
				Right partの場合、法線のX値が 0 より大きい
		\*--------------------------------------------------------------------*/
		if (iCnt == (MAX_SURFACE * 2)) {
			// 確保した値以上を取得しないように
			return(PRO_TK_NO_ERROR);
		}

		double dYPointMin = pDataPtr->data.p_surface_data->xyz_min[1];

		/*--------------------------------------------------------------------*\
		 ★ フレームの表面のサーフェイス情報を取得
				表面と同じY座標を持つ
		\*--------------------------------------------------------------------*/
		if (iCnt == 0) {
			if (((SurfaceInfoAppData*)app_data)->dSurfY[iCnt] != dYPointMin) {
				// 表面とY座標が異なる場合はNG
				return(PRO_TK_NO_ERROR);
			}
		}
		else {
			if (((SurfaceInfoAppData*)app_data)->dSurfY[iCnt / 2] != dYPointMin) {
				// 表面とY座標が異なる場合はNG
				return(PRO_TK_NO_ERROR);
			}
		}

		double min = pDataPtr->data.p_surface_data->xyz_min[2];
		double max = pDataPtr->data.p_surface_data->xyz_max[2];

		// min/maxがマイナスの場合はプラスにする
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
		// サーフェイス情報の取得
		((SurfaceInfoAppData*)app_data)->strBackInfo.surface[((SurfaceInfoAppData*)app_data)->strBackInfo.iCounter / 2 - 1] = surface;
	}

	ProGeomitemdataFree(&pDataPtr);
	return(PRO_TK_NO_ERROR);
}



/*====================================================================*\
FUNCTION: getAsmcompathIncludePart()
PURPOSE:  part 情報を含む ProAsmcomppath を取得する
ProMdl top_asm					in		TOPアセンブリ情報
ProMdl part						in		part情報
ProAsmcomppath* compPath		out		取得するProAsmcomppath
\*====================================================================*/
ProError getAsmcompathIncludePart(ProMdl top_asm, ProMdl part, ProAsmcomppath* compPath) {
	ProError status = PRO_TK_NO_ERROR;
	ProPath patName;
	DatumAppData	appdataFeature;
	// 検索するパーツの名前を設定

	ProMdlMdlnameGet(part, patName);
	ProWstringCopy(patName, appdataFeature.name, PRO_VALUE_UNUSED);
	// 初期化する
	appdataFeature.iFindCnt = 0;

	// Topパーツのパーツ(patName)のフィーチャを検索
	ProSolidFeatVisit((ProSolid)top_asm, getFeatureIdAction, NULL, (ProAppData)&appdataFeature);

	if (appdataFeature.iFindCnt == 0) {
		// パーツ(patName)のフィーチャが見つからなかった
		return (PRO_TK_E_NOT_FOUND);
	}

	ProIdTable idTable;
	idTable[0] = appdataFeature.feature.id;

	// ProAsmcompathデータ構造の初期化
	// topアセンブリから見て、1階層下のパーツであることを定義
	status = ProAsmcomppathInit((ProSolid)top_asm, idTable, 1, compPath);

	return status;
}

/*====================================================*\
  Function : convertGroupToFeature()
  Purpose  : ProGroupをFeatureに変換する
\*====================================================*/
ProError convertGroupToFeature(ProFeature* pFeature, ProError status, ProAppData app_data)
{
	// フィーチャをグループに変換
	ProGroup		localGroup;
	ProName			wFeatureName;
	ProCharName		cFeatureName;
	ProCharName		cName;

	status = ProFeatureGroupGet(pFeature, &localGroup);

	if (localGroup.type == ((strGroupToFeature*)app_data)->group.type
		&& localGroup.id == ((strGroupToFeature*)app_data)->group.id
		&& localGroup.owner == ((strGroupToFeature*)app_data)->group.owner) {

		// Group内のものまでヒットするので、グループ名でフィルターする
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
	Purpose:	CSYS検索フィルター
\*=========================================================================*/
ProError CsysNameFindFilterAction(ProFeature* p_feature, ProAppData app_data)
{
	ProError    status;
	ProFeattype ftype;
	ProName wName;
	int iResult;

	// フィーチャタイプを取得 (データム, CSYS, コンポーネント ...)
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
	Purpose:	穴あけパーツのサーフェイス情報を取得
	SurfaceInfoAppData surfInfoData (out)	サーフェイス情報
	int iLRFlag						(in)	パーツ向き(左右)
	ProMdl mdlPart					(in)	サーフェイス情報の取得するパーツ	
\*=========================================================================*/
ProError getSurfaceInfo(SurfaceInfoAppData* surfInfoData , int iLRFlag, ProMdl mdlPart) {
	// 値の初期化

	surfInfoData->iLRFlag = iLRFlag;

	// 穴あけパーツのサーフェイス情報を取得
	ProSolidSurfaceVisit(ProMdlToSolid(mdlPart),
		(ProSurfaceVisitAction)getSurfZpointAction,
		(ProSurfaceFilterAction)NULL,
		(ProAppData)surfInfoData);

	// 穴あけパーツの裏面のサーフェイス情報を取得
	ProSolidSurfaceVisit(ProMdlToSolid(mdlPart),
		(ProSurfaceVisitAction)searchBackSurfAction,
		(ProSurfaceFilterAction)NULL,
		(ProAppData)surfInfoData);
}


/*=========================================================================*\
	Function:	getXDatumSurfaceInfo()
	Purpose:	表面が複数に分かれている場合があるので、
				データム平面Xの座標 と 穴のX の座標から対象面を取得する
SurfaceInfoAppData* surfInfoData	(in)　サーフェイス情報
InputFileHole* localHole			(in)  コンフィグレーションファイルの情報
ProMdl* top_asm						(in)　トップアセンブリ
ProSurface* dtmplnModelitem			(out)　ProSurface
\*=========================================================================*/
ProError getXDatumSurfaceInfo(MainSurfaceInfo* surfInfoData, InputFileHole* localHole, ProMdl* top_asm, ProSurface* dtmplnModelitem) {

	ProError status;

	/*--------------------------------------------------------------------*\
		データム平面X のサーフェイスとを取得
	\*--------------------------------------------------------------------*/
	DatumAppData	appdataFeature;
	int iDatumType = 0;
	iDatumType = atoi(localHole->cDatumType);

	if (iDatumType == 0) {
		// 検索するデータム平面の名前を設定
		ProWstringCopy(X_DATUM_NAME_0, appdataFeature.name, PRO_VALUE_UNUSED);

	}
	else if (iDatumType == 1) {
		// 検索するデータム平面の名前を設定
		ProWstringCopy(X_DATUM_NAME_1, appdataFeature.name, PRO_VALUE_UNUSED);

	}
	else if (iDatumType == 2) {
		// 検索するデータム平面の名前を設定
		ProWstringCopy(X_DATUM_NAME_2, appdataFeature.name, PRO_VALUE_UNUSED);

	}
	else if (iDatumType == 3) {
		// 検索するデータム平面の名前を設定
		ProWstringCopy(X_DATUM_NAME_3, appdataFeature.name, PRO_VALUE_UNUSED);

	}
	else if (iDatumType == 4) {
		// 検索するデータム平面の名前を設定
		ProWstringCopy(X_DATUM_NAME_4, appdataFeature.name, PRO_VALUE_UNUSED);

	}
	else {
		// X座標の基準となるデータム平面が想定外の値
		return PRO_TK_BAD_INPUTS;
	}

	// 初期化する
	appdataFeature.iFindCnt = 0;

	// Topパーツのデータム平面(X_DATUM_NAME)のフィーチャを検索
	ProSolidFeatVisit((ProSolid)*top_asm, getFeatureIdAction, NULL, (ProAppData)&appdataFeature);

	if (appdataFeature.iFindCnt == 0) {
		// データム平面(X_DATUM_NAME)のフィーチャが見つからなかった
		return (PRO_TK_E_NOT_FOUND);
	}

	// フィーチャを PRO_SURFACE のProModelitemに変換する
	ProModelitem dtmplnConstrMdlItem;
	ProFeatureGeomitemVisit(&appdataFeature.feature, PRO_SURFACE, UsrPointAddAction, NULL, (ProAppData)&dtmplnConstrMdlItem);

	// ProModelitemを PRO_SURFACE に変換する
	ProSurface selectedSurf;
	status = ProGeomitemToSurface(&dtmplnConstrMdlItem, &selectedSurf);

	/*--------------------------------------------------------------------*\
		データム平面X の座標を取得
	\*--------------------------------------------------------------------*/
	ProGeomitemdata* pDataPtr;
	status = ProSurfaceDataGet(selectedSurf, &pDataPtr);
	double dMaxXDtatumPoint = pDataPtr->data.p_surface_data->xyz_max[0];
	double dMinXDtatumPoint = pDataPtr->data.p_surface_data->xyz_min[0];

	if (dMaxXDtatumPoint != dMinXDtatumPoint) {
		return (PRO_TK_E_NOT_FOUND);
	}

	/*--------------------------------------------------------------------*\
		穴のX の座標を取得
	\*--------------------------------------------------------------------*/
	double dXHolePoint = 0;
	dXHolePoint = atof(localHole->cXCord);

	/*--------------------------------------------------------------------*\
		データム平面Xの座標 と 穴のX の座標から対象面を取得する
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
		// 対象面が見つかりませんでした
		return (PRO_TK_E_NOT_FOUND);
	}

	*dtmplnModelitem = xDatumSurface;
	return PRO_TK_NO_ERROR;
}

/*=========================================================================*\
	Function:	getXDatumSurfaceInfoForHole2()
	Purpose:	表面が複数に分かれている場合があるので、
				データム平面Xの座標 と 穴のX の座標から対象面を取得する
				※穴2以降の調査に使用する
SurfaceInfoAppData* surfInfoData	(in)　サーフェイス情報
InputFileHole* localHole			(in)  コンフィグレーションファイルの情報
ProMdl* top_asm						(in)　トップアセンブリ
double dXHolePoint					(in)　X座標
ProSurface* dtmplnModelitem			(out)　ProSurface
\*=========================================================================*/
ProError getXDatumSurfaceInfoForHole2(MainSurfaceInfo* surfInfoData, InputFileHole* localHole, ProMdl* top_asm, double dXHolePoint, ProSurface* dtmplnModelitem) {

	ProError status;

	/*--------------------------------------------------------------------*\
		データム平面X のサーフェイスとを取得
	\*--------------------------------------------------------------------*/
	DatumAppData	appdataFeature;
	int iDatumType = 0;
	iDatumType = atoi(localHole->cDatumType);

	if (iDatumType == 0) {
		// 検索するデータム平面の名前を設定
		ProWstringCopy(X_DATUM_NAME_0, appdataFeature.name, PRO_VALUE_UNUSED);

	}
	else if (iDatumType == 1) {
		// 検索するデータム平面の名前を設定
		ProWstringCopy(X_DATUM_NAME_1, appdataFeature.name, PRO_VALUE_UNUSED);

	}
	else if (iDatumType == 2) {
		// 検索するデータム平面の名前を設定
		ProWstringCopy(X_DATUM_NAME_2, appdataFeature.name, PRO_VALUE_UNUSED);

	}
	else if (iDatumType == 3) {
		// 検索するデータム平面の名前を設定
		ProWstringCopy(X_DATUM_NAME_3, appdataFeature.name, PRO_VALUE_UNUSED);

	}
	else if (iDatumType == 4) {
		// 検索するデータム平面の名前を設定
		ProWstringCopy(X_DATUM_NAME_4, appdataFeature.name, PRO_VALUE_UNUSED);

	}
	else {
		// X座標の基準となるデータム平面が想定外の値
		return PRO_TK_BAD_INPUTS;
	}

	// 初期化する
	appdataFeature.iFindCnt = 0;

	// Topパーツのデータム平面(X_DATUM_NAME)のフィーチャを検索
	ProSolidFeatVisit((ProSolid)*top_asm, getFeatureIdAction, NULL, (ProAppData)&appdataFeature);

	if (appdataFeature.iFindCnt == 0) {
		// データム平面(X_DATUM_NAME)のフィーチャが見つからなかった
		return (PRO_TK_E_NOT_FOUND);
	}

	// フィーチャを PRO_SURFACE のProModelitemに変換する
	ProModelitem dtmplnConstrMdlItem;
	ProFeatureGeomitemVisit(&appdataFeature.feature, PRO_SURFACE, UsrPointAddAction, NULL, (ProAppData)&dtmplnConstrMdlItem);

	// ProModelitemを PRO_SURFACE に変換する
	ProSurface selectedSurf;
	status = ProGeomitemToSurface(&dtmplnConstrMdlItem, &selectedSurf);

	/*--------------------------------------------------------------------*\
		データム平面X の座標を取得
	\*--------------------------------------------------------------------*/
	ProGeomitemdata* pDataPtr;
	status = ProSurfaceDataGet(selectedSurf, &pDataPtr);
	double dMaxXDtatumPoint = pDataPtr->data.p_surface_data->xyz_max[0];
	double dMinXDtatumPoint = pDataPtr->data.p_surface_data->xyz_min[0];

	if (dMaxXDtatumPoint != dMinXDtatumPoint) {
		return (PRO_TK_E_NOT_FOUND);
	}

	/*--------------------------------------------------------------------*\
		データム平面Xの座標 と 穴のX の座標から対象面を取得する
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
		// 対象面が見つかりませんでした
		return (PRO_TK_E_NOT_FOUND);
	}

	*dtmplnModelitem = xDatumSurface;
	return PRO_TK_NO_ERROR;

}

/*====================================================================*\
FUNCTION : createDatumAxis
PURPOSE  : データム軸の作成
 top_asm			(in) アセンブリ
 mdlPart			(in) 穴をあけるパーツ
 surfaceMdlItem		(in) データム軸を作成する面
 dtmPlnFeature_X	(in) 穴1をあける時に使用したデータム平面(X軸)
 dtmPlnFeature_Y	(in) 穴1をあける時に使用したデータム平面(Y軸)
 distX				(in) 穴1からの距離(X軸)
 distY				(in) 穴1からの距離(Y軸)
 axisFeature		(out)データム軸フィーチャ
 備考
 穴1が表面、穴2以降が他(斜め)面に穴をあける場合に使用する。
 面に対して垂直なデータム軸を作成する

 -------------
PRO_E_FEATURE_TREE
	|--PRO_E_FEATURE_TYPE
	|--PRO_E_STD_FEATURE_NAME
	|--PRO_E_DTMAXIS_CONSTRAINTS
	|    |--PRO_E_DTMAXIS_CONSTRAINT	(サーフェイス)
	|         |--PRO_E_DTMAXIS_CONSTR_TYPE
	|         |--PRO_E_DTMAXIS_CONSTR_REF
	|
	|--PRO_E_DTMAXIS_DIM_CONSTRAINTS
		|--PRO_E_DTMAXIS_DIM_CONSTRAINT	(データム平面1)
		|    |--PRO_E_DTMAXIS_DIM_CONSTR_REF
		|    |--PRO_E_DTMAXIS_DIM_CONSTR_VAL
		|
		|--PRO_E_DTMAXIS_DIM_CONSTRAINT	(データム平面2)
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
			|    |--PRO_E_DTMAXIS_CONSTRAINT	(サーフェイス)
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
				|--PRO_E_DTMAXIS_DIM_CONSTRAINT	(データム平面1)
				|    |--PRO_E_DTMAXIS_DIM_CONSTR_REF
				|    |--PRO_E_DTMAXIS_DIM_CONSTR_VAL
				|
				|--PRO_E_DTMAXIS_DIM_CONSTRAINT	(データム平面2)
					|--PRO_E_DTMAXIS_DIM_CONSTR_REF
					|--PRO_E_DTMAXIS_DIM_CONSTR_VAL
	\*---------------------------------------------------------------*/
	// PRO_E_DTMAXIS_DIM_CONSTRAINTS
	status = ProElementAlloc(PRO_E_DTMAXIS_DIM_CONSTRAINTS, &pro_e_dtmaxis_dim_constraints);
	status = ProElemtreeElementAdd(pro_e_feature_tree, NULL, pro_e_dtmaxis_dim_constraints);

	// PRO_E_DTMAXIS_DIM_CONSTRAINT 	(データム平面1)
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

	// PRO_E_DTMAXIS_DIM_CONSTR_VAL:オフセット距離(X)
	status = ProElementAlloc(PRO_E_DTMAXIS_DIM_CONSTR_VAL, &pro_e_dtmaxis_dim_constr_val2);
	status = ProElementDoubleSet(pro_e_dtmaxis_dim_constr_val2, distX);
	status = ProElemtreeElementAdd(pro_e_dtmaxis_dim_constraint, NULL, pro_e_dtmaxis_dim_constr_val2);

	// PRO_E_DTMAXIS_DIM_CONSTRAINT 	(データム平面2)
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

	// PRO_E_DTMAXIS_DIM_CONSTR_VALオフセット距離(Y)
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
PURPOSE  : データム軸を基準に穴をあける
	ProMdl top_asm,					in topアセンブリ
	ProMdl part						in 穴をあけるパーツ
	ProFeature dtmAxisFeature2,		in 基準となるデータム軸のフィーチャ
	ProFeature dtmPlnFeature_selSurf in 配置面/穴をあける面
	double distX,					in 穴1からの距離(X軸)
	double distY					in 穴1からの距離(Y軸)
	double dDiameter				in 穴直径
	ProFeature* holeFeature			out 穴フィーチャ
備考
	指定したデータム軸を基準に穴をあける
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
	 要素ツリー作成の開始
	*************************************************/
	/* 要素ツリー(ルート要素)の追加 */
	status = ProElementAlloc(PRO_E_FEATURE_TREE, &feat_elemtree);

	/***********************************************
	 PRO_E_FEATURE_TYPE
	*************************************************/
	/* 要素ツリーに フィーチャタイプ(穴) を追加する */
	status = ProElementAlloc(PRO_E_FEATURE_TYPE, &elem_feattype);
	status = ProElementIntegerSet(elem_feattype, PRO_FEAT_HOLE);
	status = ProElemtreeElementAdd(feat_elemtree, NULL, elem_feattype);

	/***********************************************
	 PRO_E_FEATURE_FORM
	*************************************************/
	/* 要素ツリーに フィーチャフォーム(ストレートホール) を追加する */
	status = ProElementAlloc(PRO_E_FEATURE_FORM, &elem_featform);
	status = ProElementIntegerSet(elem_featform, PRO_HLE_TYPE_STRAIGHT);
	status = ProElemtreeElementAdd(feat_elemtree, NULL, elem_featform);

	/***********************************************
	 PRO_E_HLE_COM
	*************************************************/
	/* 要素ツリーに 穴情報の共通要素 の追加  */
	status = ProElementAlloc(PRO_E_HLE_COM, &elem_hle_com);
	status = ProElemtreeElementAdd(feat_elemtree, NULL, elem_hle_com);

	/* 穴情報 (穴タイプ:ストレート) を指定する */
	status = ProElementAlloc(PRO_E_HLE_TYPE_NEW, &elem_hle_type_new);
	status = ProElementIntegerSet(elem_hle_type_new, PRO_HLE_NEW_TYPE_STRAIGHT);
	status = ProElemtreeElementAdd(elem_hle_com, NULL, elem_hle_type_new);

	/* 穴情報 (穴の直径) を指定する */
	status = ProElementAlloc(PRO_E_DIAMETER, &elem_diameter);
	status = ProElementDoubleSet(elem_diameter, dDiameter);
	status = ProElemtreeElementAdd(elem_hle_com, NULL, elem_diameter);

	/* 標準深度の要素を追加する

			  |--PRO_E_HOLE_STD_DEPTH
			  |    |--PRO_E_HOLE_DEPTH_TO
			  |    |    |--PRO_E_HOLE_DEPTH_TO_TYPE
			  |    |    |--PRO_E_EXT_DEPTH_TO_VALUE
			  |    |--PRO_E_HOLE_DEPTH_FROM
			  |         |--PRO_E_HOLE_DEPTH_FROM_TYPE
			  |         |--PRO_E_EXT_DEPTH_FROM_VALUE
	*/

	// 深さ要素
	status = ProElementAlloc(PRO_E_HOLE_STD_DEPTH, &elem_hole_std_depth);
	status = ProElemtreeElementAdd(elem_hle_com, NULL, elem_hole_std_depth);

	// side1 情報： Blind 深さ25
	status = ProElementAlloc(PRO_E_HOLE_DEPTH_TO, &elem_hole_depth_to);
	status = ProElemtreeElementAdd(elem_hole_std_depth, NULL, elem_hole_depth_to);
	status = ProElementAlloc(PRO_E_HOLE_DEPTH_TO_TYPE, &elem_hole_depth_to_type);
	status = ProElementIntegerSet(elem_hole_depth_to_type, PRO_HLE_STRGHT_BLIND_DEPTH);
	status = ProElemtreeElementAdd(elem_hole_depth_to, NULL, elem_hole_depth_to_type);
	status = ProElementAlloc(PRO_E_EXT_DEPTH_TO_VALUE, &elem_hole_depth_to_value);
	status = ProElementDoubleSet(elem_hole_depth_to_value, 25.0);
	status = ProElemtreeElementAdd(elem_hole_depth_to, NULL, elem_hole_depth_to_value);

	// side2 情報： Blind 深さ25
	status = ProElementAlloc(PRO_E_HOLE_DEPTH_FROM, &elem_hole_depth_from);
	status = ProElemtreeElementAdd(elem_hole_std_depth, NULL, elem_hole_depth_from);
	status = ProElementAlloc(PRO_E_HOLE_DEPTH_FROM_TYPE, &elem_hole_depth_from_type);
	status = ProElementIntegerSet(elem_hole_depth_from_type, PRO_HLE_STRGHT_BLIND_DEPTH);
	status = ProElemtreeElementAdd(elem_hole_depth_from, NULL, elem_hole_depth_from_type);
	status = ProElementAlloc(PRO_E_EXT_DEPTH_FROM_VALUE, &elem_hole_depth_from_value);
	status = ProElementDoubleSet(elem_hole_depth_from_value, 25.0);
	status = ProElemtreeElementAdd(elem_hole_depth_from, NULL, elem_hole_depth_from_value);

	/* 配置の詳細に関連する要素を追加する
	 軸を主基準とする同軸穴

		 |--PRO_E_HLE_PLACEMENT
		 |    |--PRO_E_HLE_PRIM_REF		(一時選択:軸)
		 |    |--PRO_E_HLE_PL_TYPE		(配置面:PRO_HLE_PL_TYPE_COAX) 
		 |    |--PRO_E_HLE_PLCMNT_PLANE (配置面)


	*/

	ProAsmcomppath compPath;
	status = getAsmcompathIncludePart(top_asm, part, &compPath);
	if (status != PRO_TK_NO_ERROR) {
		// パーツ(patName)のフィーチャが見つからなかった
		return (PRO_TK_E_NOT_FOUND);
	}

	status = ProElementAlloc(PRO_E_HLE_PLACEMENT, &elem_hle_placement);
	status = ProElemtreeElementAdd(feat_elemtree, NULL, elem_hle_placement);

	// PRO_E_HLE_PRIM_REF　一次選択 (軸)
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

	// PRO_E_HLE_PL_TYPE  穴配置オプション
	status = ProElementAlloc(PRO_E_HLE_PL_TYPE, &elem_hle_pl_type);
	status = ProElementIntegerSet(elem_hle_pl_type, PRO_HLE_PL_TYPE_COAX);
	status = ProElemtreeElementAdd(elem_hle_placement, NULL, elem_hle_pl_type);

	// PRO_E_HLE_PLCMNT_PLANE　配置面
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

	// 距離 1
	status = ProElementAlloc(PRO_E_HLE_DIM_DIST1, &elem_hle_dim_dist1);
	status = ProElementDoubleSet(elem_hle_dim_dist1, distX);
	status = ProElemtreeElementAdd(elem_hle_placement, NULL, elem_hle_dim_dist1);

	// PRO_E_HLE_DIM_REF1 がデータム軸のため、PRO_E_HLE_DIM_REF2 の設定なし

	// 距離 2
	status = ProElementAlloc(PRO_E_HLE_DIM_DIST2, &elem_hle_dim_dist2);
	status = ProElementDoubleSet(elem_hle_dim_dist2, distY);
	status = ProElemtreeElementAdd(elem_hle_placement, NULL, elem_hle_dim_dist2);

	/* 要素ツリー作成の終了 */

	/***********************************************
	 フィーチャーの作成
	*************************************************/
	status = ProMdlToModelitem(part, &model_item);
	status = ProSelectionAlloc(&compPath, &model_item, &model_selection);
	status = ProArrayAlloc(1, sizeof(ProFeatureCreateOptions), 1, (ProArray*)&options);

	options[0] = PRO_FEAT_CR_NO_OPTS;
	status = ProFeatureWithoptionsCreate(model_selection, feat_elemtree, options, PRO_REGEN_NO_FLAGS, holeFeature, &p_errors);
	TRAIL_PRINT("%s(%d) : ProFeatureWithoptionsCreate = %s", __func__, __LINE__, getProErrorMessage(status));

	/***********************************************
	 リソースの解放
	*************************************************/
	status = ProElementFree(&feat_elemtree);
	status = ProSelectionFree(&model_selection);
	status = ProArrayFree((ProArray*)&options);

	return (status);
}

