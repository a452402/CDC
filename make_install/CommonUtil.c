/*--------------------------------------------------------------------*\
    C includes
\*--------------------------------------------------------------------*/
#include <time.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <wtypes.h>
#include <mbstring.h>
/*--------------------------------------------------------------------*\
    Pro/Toolkit includes
\*--------------------------------------------------------------------*/
#include <ProWTUtils.h>
#include <ProMdl.h>
#include <ProFaminstance.h>
#include <ProUtil.h>
#include <ProWstring.h>
#include <ProFeature.h>
#include <ProFeatType.h>
#include <ProCsys.h>
#include <ProModelitem.h>
#include <ProWindows.h>
#include <ProSolid.h>
#include <ProSimprep.h>
#include <ProBrowser.h>

/*--------------------------------------------------------------------*\
    Application includes
\*--------------------------------------------------------------------*/
#include "CommonUtil.h"


static ProError  downloadAllObjectAction(ProFeature* pFeature, ProError status, ProAppData app_data);
static ProError  downloadObject(ProPath objectname, int loadtype);

/*====================================================================*\
FUNCTION : c_trim
PURPOSE  : 文字列の前後の空白、改行を削除する
    cpStr  (in)    変換する文字列
戻り値 : 変換後の値
\*====================================================================*/
char* c_trim(char* cpStr)
{
	int i;
	char* cpStart; /* 文字列の先頭位置 */

    if (cpStr == NULL) {
        return cpStr;
    }

	/* 後ろのスペース, または改行を削除 */
	for (i = strlen(cpStr) - 1; i >= 0; i--)
	{
		if ((cpStr[i] != ' ')&&( cpStr[i] != '\n'))
		{
			break;
		}
		cpStr[i] = '\0';
	}

	/* 前スペースを除いた先頭を探す */
	cpStart = cpStr;
	while (*cpStart == ' ')
	{
		cpStart++;
	}

	/* 前スペース分をシフト */
	memmove(cpStr, cpStart, strlen(cpStart) + 1);

	return cpStr;
}

/*====================================================================*\
FUNCTION : searchAssypathFromWindchill
PURPOSE  : Windchillへ接続してファイルを検索、チェックアウトを行う
    ProPath objectname          (in)    ロードするアセンブリ名
    int assytype                (in)    ロードするアセンブリ種類 TOP/SUB
    ProMdlfileType loadtype     (in)    ロードする種類 アセンブリ/notebook
    ProMdl* assy                (out)   ロードしたアセンブリのProMdl

\*====================================================================*/
ProError  searchAssypathFromWindchill(ProPath objectname, int assytype, int loadtype, ProMdl* assy)
{
    ProError status = PRO_TK_NO_ERROR;
    wchar_t* windchillServer;
    wchar_t* actWorkspace;
    ProPath wimpdirFile;

    /***********************************************
     Windchillからチェックアウトせずに、ワークスペースへダウンロードする
    *************************************************/
    status = downloadObject( objectname, loadtype);
    if (status != PRO_TK_NO_ERROR)
    {
        // 失敗 
        return status;
    }

    /***********************************************
     ワークスペース上から、Creo上にロードする
    *************************************************/
    // windchillのアクティブサーバ取得
    status = ProServerActiveGet(&windchillServer);

    // Windchillのワークスペース取得
    status = ProServerWorkspaceGet(windchillServer, &actWorkspace);

    ProStringToWstring(wimpdirFile, "wtws://");
    ProWstringConcatenate(windchillServer, wimpdirFile, PRO_VALUE_UNUSED);
    ProWstringConcatenate(L"/", wimpdirFile, PRO_VALUE_UNUSED);
    ProWstringConcatenate(actWorkspace, wimpdirFile, PRO_VALUE_UNUSED);
    ProWstringConcatenate(L"/", wimpdirFile, PRO_VALUE_UNUSED);
    ProWstringConcatenate(objectname, wimpdirFile, PRO_VALUE_UNUSED);

    if (loadtype == PRO_MDLFILE_ASSEMBLY) {
        // アセンブリの場合はEmptyでロードする
        status = ProAssemblySimprepMdlnameRetrieve(wimpdirFile, (ProMdlfileType)loadtype, L"EMPTY", NULL, (ProAssembly*)assy);
    }
    else {
        // ロードする
        status = ProMdlnameRetrieve(wimpdirFile, (ProMdlfileType)loadtype, assy);
    }


   return(PRO_TK_NO_ERROR);
}

/*====================================================================*\
FUNCTION : downloadObject
PURPOSE  : チェックアウトせず、ダウンロードのみ行う
    ProPath wObjectname             (in)    ロードするアセンブリ名
    ProMdlfileType loadtype         (in)    ロードする種類 アセンブリ/notebook

\*====================================================================*/
static ProError  downloadObject(ProPath wObjectname, int loadtype)
{
    ProError status;
    ProError status2;
    ProServerCheckoutOptions co_options;
    ProMdl		 inst_model;
    ProSolid generic = NULL;
    ProServerCheckinConflicts conflicts;
    wchar_t* model_url = NULL;

    // チェックアウトオプションの設定
    // PRO_SERVER_INCLUDE_ALL：選択したオブジェクトのすべてのインスタンスがチェックアウトされます。
    status = ProServercheckoutoptsAlloc(&co_options);
    status = ProServercheckoutoptsIncludeinstancesSet(co_options, PRO_SERVER_INCLUDE_ALL, NULL);
    // 読み取り専用としてダウンロード
    status = ProServercheckoutoptsReadonlySet(co_options, PRO_B_TRUE);

    if (loadtype == PRO_MDLFILE_ASSEMBLY) {
        // アセンブリの場合はEmptyでロードする
        status = ProMdlRepresentationFiletypeLoad(wObjectname, (ProMdlfileType)loadtype, L"EMPTY", NULL, &inst_model);

        if (status == PRO_TK_E_NOT_FOUND) {
            /***********************************************
             対象アセンブリにEmptyが存在しなかった場合、ワークスペース内に対象ファイルが存在するかを確認する
            *************************************************/
            ProPath* file_list, * dir_list;
            ProPath wCommonSpacePath;
            int n_files;
            bool mdlFound = FALSE;
            wchar_t* windchillServer;
            wchar_t* actWorkspace;

            // windchillのアクティブサーバ取得
            status = ProServerActiveGet(&windchillServer);

            // Windchillのワークスペース取得
            status = ProServerWorkspaceGet(windchillServer, &actWorkspace);

            ProStringToWstring(wCommonSpacePath, "wtws://");
            ProWstringConcatenate(windchillServer, wCommonSpacePath, PRO_VALUE_UNUSED);
            ProWstringConcatenate(L"/", wCommonSpacePath, PRO_VALUE_UNUSED);
            ProWstringConcatenate(actWorkspace, wCommonSpacePath, PRO_VALUE_UNUSED);

            ProArrayAlloc(0, sizeof(ProPath), 1, (ProArray*)&file_list);
            ProArrayAlloc(0, sizeof(ProPath), 1, (ProArray*)&dir_list);
            // ワークスペース内のファイルの最新バージョンのみを一覧で取得する
            status = ProFilesList(wCommonSpacePath, L"*.asm", PRO_FILE_LIST_LATEST, &file_list, &dir_list);

            status = ProArraySizeGet((ProArray)file_list, &n_files);

            for (int iLoop = 0; iLoop < n_files; iLoop++)
            {
                int iResult;
                ProMdlName wname;
                ProCharPath cObjectname;
                ProPath wObjectname_split;
                char* find;

                // フォルダパスからファイル名のみを抽出
                ProFileMdlnameParse(file_list[iLoop], NULL, wname, NULL, NULL);

                //wObjectnameに拡張子が付いている場合は、拡張子を削除してから比較する
                ProWstringToString(cObjectname, wObjectname);
                find = strstr(cObjectname, ".");
                if (find != NULL) {
                    Split(cObjectname, L"", wObjectname_split);
                    ProWstringCompare(wObjectname_split, wname, PRO_VALUE_UNUSED, &iResult);
                }
                else {

                    ProWstringCompare(wObjectname, wname, PRO_VALUE_UNUSED, &iResult);
                }

                if (iResult == 0) {
                    mdlFound = TRUE;
                    break;
                }
            }

            if (mdlFound == FALSE) {
                // windchillのコモンスペース内に見つけられなかった場合
                LOG_PRINT("NOK : %w : There is no data, so, Failed to load from windchill", wObjectname);

            }
            else {
                // ファイル自体は存在するが、Emptyが存在しない。
                LOG_PRINT("NOK : %w : Empty load failed, so, Failed to load from windchill", wObjectname);
            }

            return PRO_TK_E_NOT_FOUND;

        }
        else if (status != PRO_TK_NO_ERROR) {
            LOG_PRINT("NOK : %w : Failed to load from windchill(%s)", wObjectname, getProErrorMessage(status));
        }
    }
    else {
        // モデルをメモリに取得しますが、表示したり、現在のモデルにしたりすることはありません
        status = ProMdlnameRetrieve(wObjectname, (ProMdlfileType)loadtype, &inst_model);
    }

    // ハンドルを指定してファミリーテーブルのジェネリックモデルを取得
    status2 = ProFaminstanceGenericGet(inst_model, PRO_B_FALSE, (ProMdl*)&generic);

    // チェックアウトせず、ダウンロードのみ行う
    // オブジェクトがインスタンスの場合は、代わりにインスタンスを含む最上位のジェネリックをチェックアウトします。
    status = ProServerObjectsCheckout(status2 == PRO_TK_NO_ERROR ? generic : inst_model,
        NULL, PRO_B_FALSE, co_options, &model_url, &conflicts);

    ProWstringFree(model_url);
    ProServercheckoutoptsFree(co_options);

    if (status == PRO_TK_CHECKOUT_CONFLICT)
    {
        // コンフリクトをした時の処理
        ProServerconflictsFree(conflicts);
    }

    return status;
}

/*=========================================================================*\
    Function:	CsysFindVisitAction()
    Purpose:	CSYS検索アクション
\*=========================================================================*/
ProError CsysFindVisitAction(
    ProCsys		this_csys,
    ProError	filter_return,
    ProAppData      app_data)
{
    int 		status;

    if (filter_return != PRO_TK_NO_ERROR) {
        return (filter_return);
    }

    ((UserCsysAppData*)app_data)->p_csys = this_csys;

    return (PRO_TK_USER_ABORT);
}
/*=========================================================================*\
    Function:	CsysFindFilterAction()
    Purpose:	CSYS検索フィルター
\*=========================================================================*/
ProError CsysFindFilterAction(
    ProCsys		this_csys,
    ProAppData	app_data)
{
    ProError 		status;
    ProName	        wThisCsysName;
    ProCharName	    cThisCsysName;
    ProCharName	    cCsys_name;
    ProModelitem mdlitem;
    int id;
    int result;

    status = ProCsysIdGet(this_csys, &id);
    status = ProModelitemInit(((UserCsysAppData*)app_data)->model, id, PRO_CSYS, &mdlitem);
    status = ProModelitemNameGet(&mdlitem, wThisCsysName);
    if (status != PRO_TK_NO_ERROR) {
        return(status);

    }
     //部分一致だと複数ヒットするので、完全一致とする
    ProWstringCompare(((UserCsysAppData*)app_data)->csys_name, wThisCsysName, PRO_VALUE_UNUSED, &result);
    if (result == 0) {
        return(PRO_TK_NO_ERROR);
    }

    return(PRO_TK_CONTINUE);
}
/*=========================================================================*\
    Function:	CsysFindFilterAction()
    Purpose:	CSYS検索フィルター
\*=========================================================================*/
ProError CsysFindAllFilterAction(
    ProCsys		this_csys,
    ProAppData	app_data)
{
    ProError 		status;
    ProName	        wThisCsysName;
    ProCharName	    cThisCsysName;
    ProCharName	    cCsys_name;
    ProModelitem mdlitem;
    int id;
    int result;

    status = ProCsysIdGet(this_csys, &id);
    status = ProModelitemInit(((UserCsysAppData*)app_data)->model, id, PRO_CSYS, &mdlitem);
    status = ProModelitemNameGet(&mdlitem, wThisCsysName);
    if (status != PRO_TK_NO_ERROR) {
        return(status);

    }
    // 完全一致で検索する
    //ProWstringToString(cThisCsysName, wThisCsysName);
    //ProWstringToString(cCsys_name, ((UserCsysAppData*)app_data)->csys_name);

    ProWstringCompare(((UserCsysAppData*)app_data)->csys_name, wThisCsysName, PRO_VALUE_UNUSED, &result);
    if (result == NULL) {
        return(PRO_TK_NO_ERROR);

    }

    return(PRO_TK_CONTINUE);
}

/*====================================================================*\
FUNCTION : UsrPointAddAction()
PURPOSE  : FetureIDを変換するアクション関数
\*====================================================================*/
ProError UsrPointAddAction(ProGeomitem* p_handle, ProError status, ProAppData app_data)
{
    *(ProGeomitem*)app_data = *p_handle;

    return (PRO_TK_CONTINUE);
}


/*====================================================================*\
FUNCTION : substring()
PURPOSE  :文字列の一部分を抜き出した文字列を作成する
    引数
        target:      対象の文字列。
        begin:       抜き出す最初の文字は、target の先頭から何文字目か。
                     target の長さ以上を指定した場合の結果は、空文字列。
        length:      抜き出す文字数（末尾のヌル文字を含まない）
                     target の末尾に到達しても構わない。
        result:      結果を格納する配列のメモリアドレス。
                     ヌルポインタは不可。
                     末尾にヌル文字が付加される。
        result_size: result の要素数。length + 1 以上必要。
    戻り値
        result を返す。
\*====================================================================*/
char* substring(const char* target, size_t begin, size_t length, char* result, size_t result_size)
{
    if (begin < strlen(target)) {

        // target[begin] を起点に、length の文字数分だけコピー
        strncpy(result, target + begin, length);

        // 末尾のヌル文字を付加
        // strncpy() によってすでに付加されている可能性はある。
        // また、result[length] よりも手前にすでにヌル文字があるかもしれない。
        result[length] = '\0';
    }
    else {
        // begin が target の末尾以降の位置にあるときは、
        // 空文字列を返す
        result[0] = '\0';
    }

    return result;
}

/*====================================================================*\
FUNCTION : str_replace()
PURPOSE  : 文字列の中から指定された文字列を探し、置換する
    引数
        src:      (in)  対象の文字列。
        target:   (in)  置き換えられる文字列、検索対象の文字列
        replace:  (in)  置き換える文字列
        result    (out) 置き換え後の文字列  
\*====================================================================*/
void str_replace(const char* src, const char* target, const char* replace, char** result) {

    char* temp = (char*)malloc(sizeof(char) * 1000);
    if (temp == NULL) {
        printf("Cannot allocate memory.\n");
        return;
    }

    char* top = temp;

    // 操作できるようにコピーする
    char* work = (char*)malloc(sizeof(char) * strlen(src));
    strcpy(work, src);

    char* p;
    while ((p = strstr(work, target)) != NULL) {
        // 検知した位置に文字列終端文字を挿入
        *p = '\0';

        // 検知した位置＋対象文字数分の位置にポインタを移動
        p += strlen(target);

        // 該当文字列以降をいったんtempに退避
        strcpy(temp, p);

        // 前半文字列と置き換え文字列を連結する
        strcat(work, replace);
        strcat(work, temp);
    }

    free(temp);

    *result = work;
}


/*====================================================================*\
FUNCTION : LOG_PRINT()
PURPOSE  : ログファイルに文字列を出力する
    引数
        const char* log_txt     (in)  対象の文字列。        
\*====================================================================*/
void LOG_PRINT( const char* log_txt, ...)
{
    char str[1024];
    FILE* fp;        /* 通常ログ */


    // 引数が複数存在するときの値取得
    bool bReplaceFlag = false;
    char cArgument[1024];
    va_list args = NULL;
    // 可変長引数を1つにまとめる
    va_start(args, log_txt);

    char log_txt_arg[1024] = "";

    for (const char* p = log_txt; *p != '\0'; ++p) {

        if (*p == '%') {
            switch (*(p + 1)) {
            case 'd':
                // 可変引数にアクセスし,変数を取り出す
                sprintf_s(cArgument, sizeof(cArgument), "%d", va_arg(args, int));
                // 取り出した変数を文字列に結合
                strcat_s(log_txt_arg, sizeof(log_txt_arg), cArgument);
                bReplaceFlag = true;
                break;
            case 'f':
                // 可変引数にアクセスし,変数を取り出す
                sprintf_s(cArgument, sizeof(cArgument), "%f", va_arg(args, double));
                // 取り出した変数を文字列に結合
                strcat_s(log_txt_arg, sizeof(log_txt_arg), cArgument);
                bReplaceFlag = true;
                break;
            case 'c':
                // 可変引数にアクセスし,変数を取り出す
                sprintf_s(cArgument, sizeof(cArgument), "%c", va_arg(args, char));
                // 取り出した変数を文字列に結合
                strcat_s(log_txt_arg, sizeof(log_txt_arg), cArgument);
                bReplaceFlag = true;
                break;
            case 's':
                // 可変引数にアクセスし,変数を取り出す
                sprintf_s(cArgument, sizeof(cArgument), "%s", va_arg(args, const char*));
                // 取り出した変数を文字列に結合
                strcat_s(log_txt_arg, sizeof(log_txt_arg), cArgument);
                bReplaceFlag = true;
                break;
            case 'w':
                // 可変引数にアクセスし,変数を取り出す
                sprintf_s(cArgument, sizeof(cArgument), "%ls", va_arg(args, const wchar_t*));
                // 取り出した変数を文字列に結合
                strcat_s(log_txt_arg, sizeof(log_txt_arg), cArgument);
                bReplaceFlag = true;
            break;
            default:
                // 未対応のため、そのまま値を出力する
                sprintf_s(cArgument, sizeof(cArgument), "%c", *p);
                strcat_s(log_txt_arg, sizeof(log_txt_arg), cArgument);
                break;
            }

        }
        else if (bReplaceFlag == true) {
            // 置換後、(p+1)はスキップする
            bReplaceFlag = false;
        }
        else {
            // 何もなければ、
            sprintf_s(cArgument, sizeof(cArgument), "%c", *p);
            strcat_s(log_txt_arg, sizeof(log_txt_arg), cArgument);

        }
    }
    va_end(args);
    
    // ファイルオープン
    if ((fopen_s(&fp, gcLogFilePath, "a")) != NULL) {
        exit(EXIT_FAILURE);        /* エラーの場合は通常、異常終了する */
    }

    // 書き込み内容の作成
    time_t tTime = time(NULL);
    strftime(str, sizeof(str), "[%Y/%m/%d %H:%M:%S] ", localtime(&tTime));
    strcat_s(str, sizeof(str), log_txt_arg);
    strcat_s(str, sizeof(str), " \n");

    // ファイルに書き込む
    fputs(str, fp);
    // ファイルクローズ
    fclose(fp);

    TRAIL_PRINT(log_txt_arg);


    return;
}

/*=========================================================================*\
    Function:	loadAssembly
    Purpose:	Moduleセクション (rename部)に記載のアセンブリ(*.asm)をロードする
    InputFileRenameFile* strModulesRename    (in)    コンフィグレーションファイルの値
    int iSectionMaxRows                      (in)    処理すべきコンフィグレーションファイルの行数
    ProMdl* mdlLoadAssy                      (out)    ロード対象
    wchar_t** wBeforeName[INPUTFILE_MAXLINE]  (out)  Rename前の名前
    wchar_t** wAfterNamebak[INPUTFILE_MAXLINE]  (out)  Rename後の名前
    \*=========================================================================*/
ProError loadAssembly(InputFileRenameFile* strModulesRename, int iSectionMaxRows, ProMdl* mdlLoadAssy, wchar_t* wBeforeNamebak, wchar_t* wAfterNamebak) {
    ProError status;
    // 操作できるようにコピーする
    wchar_t* wBeforeName = (wchar_t*)calloc(INPUTFILE_MAXLINE, sizeof(wchar_t));
    wchar_t* wAfterName = (wchar_t*)calloc(INPUTFILE_MAXLINE, sizeof(wchar_t));

    for (int iInputMdlCnt = 0; iInputMdlCnt < iSectionMaxRows; iInputMdlCnt++) {
        ProStringToWstring(wBeforeName, strModulesRename->cBeforeName);
        ProStringToWstring(wAfterName, strModulesRename->cAfterName);

        if (strstr(strModulesRename->cBeforeName, ".asm") != NULL) {
            // Windchillからアセンブリをロードする
            status = searchAssypathFromWindchill(wBeforeName, SUB_ASSY, PRO_MDLFILE_ASSEMBLY, mdlLoadAssy);

            if (status != PRO_TK_NO_ERROR)
            {
                // チェックアウト失敗 
                return status;
            }

            // 拡張子抜きをwBeforeNameに格納する
            Split(strModulesRename->cBeforeName, L"", wBeforeName);

            // 拡張子抜きをwAfterNameに格納する
            Split(strModulesRename->cAfterName, L"", wAfterName);

            ProWstringCopy(wBeforeName, wBeforeNamebak, PRO_VALUE_UNUSED);
            ProWstringCopy(wAfterName, wAfterNamebak, PRO_VALUE_UNUSED);

            free(wBeforeName);
            free(wAfterName);
            wBeforeName = NULL;
            wAfterName = NULL;
            return PRO_TK_NO_ERROR;
        }

        strModulesRename++;
    }

    free(wBeforeName);
    free(wAfterName);
    wBeforeName = NULL;
    wAfterName = NULL;
    return  PRO_TK_GENERAL_ERROR;
}

/*=========================================================================*\
    Function:	Split 
    Purpose:	指定ファイル名の拡張子を抜く。拡張子(wType)をつける。拡張子(wType)の指定がない場合は抜くだけ
    char* cFilenameBefore           (in)  拡張子付きファイル名
    wchar_t* wType                       (in)  拡張子の指定
    wchar_t** wFilenameAfter        (out)  拡張子付きファイル名
    \*=========================================================================*/
ProError Split(char* cFilenameBefore, wchar_t* wType, wchar_t* wFilenameAfter) {
    char cBeforeNameTemp[INPUTFILE_MAXLINE];
    char cFilename[INPUTFILE_MAXLINE];
    wchar_t wFilename[INPUTFILE_MAXLINE];

    memcpy(cBeforeNameTemp, cFilenameBefore, sizeof(cBeforeNameTemp));
    memset(cFilename, 0, sizeof(cFilename));
    // (.)ドットの位置を検索
    char* p = strrchr(cBeforeNameTemp, '.');
    //  (.)ドットの前までの文字列を取得
    strncpy(cFilename, cBeforeNameTemp, p - cBeforeNameTemp);

    // 拡張子を付ける
    ProStringToWstring(wFilename, cFilename);
    ProWstringCopy(wFilename, wFilenameAfter, PRO_VALUE_UNUSED);
    ProWstringConcatenate(wType, wFilenameAfter, PRO_VALUE_UNUSED);

    return PRO_TK_NO_ERROR;
}

/*====================================================*\
  Function : getFeatureIdAction()
  Purpose  : 引数で指定したFeatureのFeatureIdを取得する
\*====================================================*/
ProError  getFeatureIdAction(ProFeature* pFeature, ProError status, ProAppData app_data)
{
    ProFamilyMdlName        wName;
    int iResult;
    ProFeattype ftype;

    if (((DatumAppData*)app_data)->iFindCnt != 0) {
        return PRO_TK_CONTINUE;

    }
    
    // フィーチャタイプを取得 (データム, CSYS, コンポーネント ...)
    status = ProFeatureTypeGet(pFeature, &ftype);
    if (ftype == PRO_FEAT_DATUM)
    {
        status = ProModelitemNameGet(pFeature, wName);
    }
    else if (ftype == PRO_FEAT_COMPONENT)
    {
        ProMdlfileType     mdltype;
        status = ProAsmcompMdlMdlnameGet((ProAsmcomp*)pFeature, &mdltype, wName);
    }
    else {
        return PRO_TK_CONTINUE;

    }

    ProWstringCompare(wName, ((DatumAppData*)app_data)->name, PRO_VALUE_UNUSED, &iResult);

    if (iResult == 0) {
        // app_dataに値を格納する
        ((DatumAppData*)app_data)->feature = *pFeature;
        // カウントをインクリメントする
        ((DatumAppData*)app_data)->iFindCnt = ((DatumAppData*)app_data)->iFindCnt + 1;
        return PRO_TK_NO_ERROR;

    }
    return PRO_TK_CONTINUE;
}

/*====================================================================*\
FUNCTION : renameObject
PURPOSE  : オブジェクトの名前を変更する
    ProPath wOldObject              in  旧オブジェクト名(拡張子抜き)
    ProPath wNewObject              in  新オブジェクト名(拡張子抜き)
    ProMdlfileType fileType         in  オブジェクトタイプ
\*====================================================================*/
ProError  renameObject(ProPath wOldObject, ProPath wNewObject, ProMdlfileType fileType)
{
    ProError status;
    ProMdl mdlObject = NULL;
    int iLen = 0;

    // 旧パーツ名のハンドル取得
    status = ProMdlnameInit(wOldObject, fileType, &mdlObject);
    if (status == PRO_TK_E_NOT_FOUND) {
        // リネームオブジェクトが見つかりませんでした
        LOG_PRINT("NOK : %w -> %w : Feature not found", wOldObject, wNewObject);
        return PRO_TK_GENERAL_ERROR;
    }

    // Creoオブジェクト名は拡張子を含まないで最大31文字のため、rename後が32文字以上の場合はエラーとする
    ProWstringLengthGet(wNewObject, &iLen);

    if (iLen > 31) {
        LOG_PRINT("NOK : %w -> %w : The rename is too long", wOldObject, wNewObject);
        return PRO_TK_GENERAL_ERROR;
    }
    else {
        // rename処理
        status = ProMdlnameRename(mdlObject, wNewObject);
        if (status == PRO_TK_NO_ERROR) {
            LOG_PRINT("OK  : %w -> %w", wOldObject, wNewObject);
            return PRO_TK_NO_ERROR;
        }
        else {
            LOG_PRINT("NOK : %w -> %w : Rename fails", wOldObject, wNewObject);
            return PRO_TK_GENERAL_ERROR;
        }
    }
}


/*=========================================================================*\
    Function:	getEnvCustomWithLog
    Purpose:	環境変数を取得する(成否をログに出力する)
    char env_value[]            (in)    環境変数の名前
    wchar_t* env_output_value   (out)   環境変数の値
    int* iErrorCnt              (out)    エラーの場合インクリメントする
\*=========================================================================*/
ProError getEnvCustomWithLog( ProCharPath env_value, ProPath env_output_value, int* iErrorCnt) {
    char* env;

    if ((env = getenv(env_value)) != NULL) {

        LOG_PRINT("OK  : %s=%s", env_value, env);
        ProStringToWstring(env_output_value, env);

    }
    else {
        // フレーム名の取得できません.
        LOG_PRINT("NOK : Could not get to %s", env_value);
        *iErrorCnt = *iErrorCnt + 1;
    }
}

/*=========================================================================*\
    Function:	getEnvCustom
    Purpose:	環境変数を取得する(ログファイル取得前のため、成否をログに出力できない)
                
    char env_value[]            (in)    環境変数の名前
    wchar_t* env_output_value   (out)   環境変数の値
    int* iErrorCnt              (out)    エラーの場合インクリメントする
\*=========================================================================*/
ProError getEnvCustom(ProCharPath env_value, ProPath env_output_value, int* iErrorCnt) {
    char* env;

    if ((env = getenv(env_value)) != NULL) {

        ProStringToWstring(env_output_value, env);

        TRAIL_PRINT("%s(%d) : OK : %s = %w", __func__, __LINE__, env_value, env_output_value);
    }
    else {
        // フレーム名の取得できません.
        *iErrorCnt = *iErrorCnt + 1;

        TRAIL_PRINT("%s(%d) : NG : %s = %w", __func__, __LINE__, env_value, env_output_value);
    }
}

/*====================================================================*\
FUNCTION : TRAIL_PRINT()
PURPOSE  : Trailファイルに文字列を出力する
    引数
        const char* log_txt     (in)  対象の文字列。
\*====================================================================*/
void TRAIL_PRINT(const char* log_txt, ...)
{
    char str[1024];
    ProComment wStr;
    FILE* fp;        /* 通常ログ */

    // 引数が複数存在するときの値取得
    bool bReplaceFlag = false;
    char cArgument[1024];
    va_list args = NULL;
    // 可変長引数を1つにまとめる
    va_start(args, log_txt);

    char log_txt_arg[1024] = "";

    for (const char* p = log_txt; *p != '\0'; ++p) {

        if (*p == '%') {
            switch (*(p + 1)) {
            case 'd':
                // 可変引数にアクセスし,変数を取り出す
                sprintf_s(cArgument, sizeof(cArgument), "%d", va_arg(args, int));
                // 取り出した変数を文字列に結合
                strcat_s(log_txt_arg, sizeof(log_txt_arg), cArgument);
                bReplaceFlag = true;
                break;
            case 'f':
                // 可変引数にアクセスし,変数を取り出す
                sprintf_s(cArgument, sizeof(cArgument), "%f", va_arg(args, double));
                // 取り出した変数を文字列に結合
                strcat_s(log_txt_arg, sizeof(log_txt_arg), cArgument);
                bReplaceFlag = true;
                break;
            case 'c':
                // 可変引数にアクセスし,変数を取り出す
                sprintf_s(cArgument, sizeof(cArgument), "%c", va_arg(args, char));
                // 取り出した変数を文字列に結合
                strcat_s(log_txt_arg, sizeof(log_txt_arg), cArgument);
                bReplaceFlag = true;
                break;
            case 's':
                // 可変引数にアクセスし,変数を取り出す
                sprintf_s(cArgument, sizeof(cArgument), "%s", va_arg(args, const char*));
                // 取り出した変数を文字列に結合
                strcat_s(log_txt_arg, sizeof(log_txt_arg), cArgument);
                bReplaceFlag = true;
                break;
            case 'w':
                // 可変引数にアクセスし,変数を取り出す
                sprintf_s(cArgument, sizeof(cArgument), "%ls", va_arg(args, const wchar_t*));
                // 取り出した変数を文字列に結合
                strcat_s(log_txt_arg, sizeof(log_txt_arg), cArgument);
                bReplaceFlag = true;
                break;
            default:
                // 未対応のため、そのまま値を出力する
                sprintf_s(cArgument, sizeof(cArgument), "%c", *p);
                strcat_s(log_txt_arg, sizeof(log_txt_arg), cArgument);
                break;
            }

        }
        else if (bReplaceFlag == true) {
            // 置換後、(p+1)はスキップする
            bReplaceFlag = false;
        }
        else {
            // 何もなければ、
            sprintf_s(cArgument, sizeof(cArgument), "%c", *p);
            strcat_s(log_txt_arg, sizeof(log_txt_arg), cArgument);

        }
    }
    va_end(args);

    // 書き込み内容の作成
    time_t tTime = time(NULL);
    strftime(str, sizeof(str), "[%Y/%m/%d %H:%M:%S] ", localtime(&tTime));
    strcat_s(str, sizeof(str), "CUSTOM_LOG ");
    strcat_s(str, sizeof(str), log_txt_arg);
    strcat_s(str, sizeof(str), " \n");

    ProStringToWstring(wStr, str);
    ProError status = ProTrailfileCommentWrite(wStr);

    if (status != PRO_TK_NO_ERROR) {
        ProTrailfileCommentWrite(L"ProTrailfileCommentWrite Error");
    }
    return;
}

/*====================================================================*\
FUNCTION : getProErrorMessage()
PURPOSE  : ProErrorのエラー種別を文字列で取得する
    引数
        ProError status     (in)  対象の文字列。
    戻り値
        引数に対応した文字
\*====================================================================*/
char* getProErrorMessage(ProError status)
{
    switch (status)
    {
    case PRO_TK_NO_ERROR:
        return "PRO_TK_NO_ERROR";
    case PRO_TK_GENERAL_ERROR:
        return "PRO_TK_GENERAL_ERROR";
    case PRO_TK_BAD_INPUTS:
        return "PRO_TK_BAD_INPUTS";
    case PRO_TK_USER_ABORT:
        return "PRO_TK_USER_ABORT";
    case PRO_TK_E_NOT_FOUND:
        return "PRO_TK_E_NOT_FOUND";
    case PRO_TK_E_FOUND:
        return "PRO_TK_E_FOUND";
    case PRO_TK_LINE_TOO_LONG:
        return "PRO_TK_LINE_TOO_LONG";
    case PRO_TK_CONTINUE:
        return "PRO_TK_CONTINUE";
    case PRO_TK_BAD_CONTEXT:
        return "PRO_TK_BAD_CONTEXT";
    case PRO_TK_NOT_IMPLEMENTED:
        return "PRO_TK_NOT_IMPLEMENTED";
    case PRO_TK_OUT_OF_MEMORY:
        return "PRO_TK_OUT_OF_MEMORY";
    case PRO_TK_COMM_ERROR:
        return "PRO_TK_COMM_ERROR";
    case PRO_TK_NO_CHANGE:
        return "PRO_TK_NO_CHANGE";
    case PRO_TK_SUPP_PARENTS:
        return "PRO_TK_SUPP_PARENTS";
    case PRO_TK_PICK_ABOVE:
        return "PRO_TK_PICK_ABOVE";
    case PRO_TK_INVALID_DIR:
        return "PRO_TK_INVALID_DIR";
    case PRO_TK_INVALID_FILE:
        return "PRO_TK_INVALID_FILE";
    case PRO_TK_CANT_WRITE:
        return "PRO_TK_CANT_WRITE";
    case PRO_TK_INVALID_TYPE:
        return "PRO_TK_INVALID_TYPE";
    case PRO_TK_INVALID_PTR:
        return "PRO_TK_INVALID_PTR";
    case PRO_TK_UNAV_SEC:
        return "PRO_TK_UNAV_SEC";
    case PRO_TK_INVALID_MATRIX:
        return "PRO_TK_INVALID_MATRIX";
    case PRO_TK_INVALID_NAME:
        return "PRO_TK_INVALID_NAME";
    case PRO_TK_NOT_EXIST:
        return "PRO_TK_NOT_EXIST";
    case PRO_TK_CANT_OPEN:
        return "PRO_TK_CANT_OPEN";
    case PRO_TK_ABORT:
        return "PRO_TK_ABORT";
    case PRO_TK_NOT_VALID:
        return "PRO_TK_NOT_VALID";
    case PRO_TK_INVALID_ITEM:
        return "PRO_TK_INVALID_ITEM";
    case PRO_TK_MSG_NOT_FOUND:
        return "PRO_TK_MSG_NOT_FOUND";
    case PRO_TK_MSG_NO_TRANS:
        return "PRO_TK_MSG_NO_TRANS";
    case PRO_TK_MSG_FMT_ERROR:
        return "PRO_TK_MSG_FMT_ERROR";
    case PRO_TK_MSG_USER_QUIT:
        return "PRO_TK_MSG_USER_QUIT";
    case PRO_TK_MSG_TOO_LONG:
        return "PRO_TK_MSG_TOO_LONG";
    case PRO_TK_CANT_ACCESS:
        return "PRO_TK_CANT_ACCESS";
    case PRO_TK_OBSOLETE_FUNC:
        return "PRO_TK_OBSOLETE_FUNC";
    case PRO_TK_NO_COORD_SYSTEM:
        return "PRO_TK_NO_COORD_SYSTEM";
    case PRO_TK_E_AMBIGUOUS:
        return "PRO_TK_E_AMBIGUOUS";
    case PRO_TK_E_DEADLOCK:
        return "PRO_TK_E_DEADLOCK";
    case PRO_TK_E_BUSY:
        return "PRO_TK_E_BUSY";
    case PRO_TK_E_IN_USE:
        return "PRO_TK_E_IN_USE";
    case PRO_TK_NO_LICENSE:
        return "PRO_TK_NO_LICENSE";
    case PRO_TK_BSPL_UNSUITABLE_DEGREE:
        return "PRO_TK_BSPL_UNSUITABLE_DEGREE";
    case PRO_TK_BSPL_NON_STD_END_KNOTS:
        return "PRO_TK_BSPL_NON_STD_END_KNOTS";
    case PRO_TK_BSPL_MULTI_INNER_KNOTS:
        return "PRO_TK_BSPL_MULTI_INNER_KNOTS";
    case PRO_TK_BAD_SRF_CRV:
        return "PRO_TK_BAD_SRF_CRV";
    case PRO_TK_EMPTY:
        return "PRO_TK_EMPTY";
    case PRO_TK_BAD_DIM_ATTACH:
        return "PRO_TK_BAD_DIM_ATTACH";
    case PRO_TK_NOT_DISPLAYED:
        return "PRO_TK_NOT_DISPLAYED";
    case PRO_TK_CANT_MODIFY:
        return "PRO_TK_CANT_MODIFY";
    case PRO_TK_CHECKOUT_CONFLICT:
        return "PRO_TK_CHECKOUT_CONFLICT";
    case PRO_TK_CRE_VIEW_BAD_SHEET:
        return "PRO_TK_CRE_VIEW_BAD_SHEET";
    case PRO_TK_CRE_VIEW_BAD_MODEL:
        return "PRO_TK_CRE_VIEW_BAD_MODEL";
    case PRO_TK_CRE_VIEW_BAD_PARENT:
        return "PRO_TK_CRE_VIEW_BAD_PARENT";
    case PRO_TK_CRE_VIEW_BAD_TYPE:
        return "PRO_TK_CRE_VIEW_BAD_TYPE";
    case PRO_TK_CRE_VIEW_BAD_EXPLODE:
        return "PRO_TK_CRE_VIEW_BAD_EXPLODE";
    case PRO_TK_UNATTACHED_FEATS:
        return "PRO_TK_UNATTACHED_FEATS";
    case PRO_TK_REGEN_AGAIN:
        return "PRO_TK_REGEN_AGAIN";
    case PRO_TK_DWGCREATE_ERRORS:
        return "PRO_TK_DWGCREATE_ERRORS";
    case PRO_TK_UNSUPPORTED:
        return "PRO_TK_UNSUPPORTED";
    case PRO_TK_NO_PERMISSION:
        return "PRO_TK_NO_PERMISSION";
    case PRO_TK_AUTHENTICATION_FAILURE:
        return "PRO_TK_AUTHENTICATION_FAILURE";
    case PRO_TK_OUTDATED:
        return "PRO_TK_OUTDATED";
    case PRO_TK_INCOMPLETE:
        return "PRO_TK_INCOMPLETE";
    case PRO_TK_CHECK_OMITTED:
        return "PRO_TK_CHECK_OMITTED";
    case PRO_TK_MAX_LIMIT_REACHED:
        return "PRO_TK_MAX_LIMIT_REACHED";
    case PRO_TK_OUT_OF_RANGE:
        return "PRO_TK_OUT_OF_RANGE";
    case PRO_TK_CHECK_LAST_ERROR:
        return "PRO_TK_CHECK_LAST_ERROR";
    case PRO_TK_NO_PLM_LICENSE:
        return "PRO_TK_NO_PLM_LICENSE";
    case PRO_TK_INCOMPLETE_TESS:
        return "PRO_TK_INCOMPLETE_TESS";
    case PRO_TK_MULTIBODY_UNSUPPORTED:
        return "PRO_TK_MULTIBODY_UNSUPPORTED";
    case PRO_TK_APP_CREO_BARRED:
        return "PRO_TK_APP_CREO_BARRED";
    case PRO_TK_APP_TOO_OLD:
        return "PRO_TK_APP_TOO_OLD";
    case PRO_TK_APP_BAD_DATAPATH:
        return "PRO_TK_APP_BAD_DATAPATH";
    case PRO_TK_APP_BAD_ENCODING:
        return "PRO_TK_APP_BAD_ENCODING";
    case PRO_TK_APP_NO_LICENSE:
        return "PRO_TK_APP_NO_LICENSE";
    case PRO_TK_APP_XS_CALLBACKS:
        return "PRO_TK_APP_XS_CALLBACKS";
    case PRO_TK_APP_STARTUP_FAIL:
        return "PRO_TK_APP_STARTUP_FAIL";
    case PRO_TK_APP_INIT_FAIL:
        return "PRO_TK_APP_INIT_FAIL";
    case PRO_TK_APP_VERSION_MISMATCH:
        return "PRO_TK_APP_VERSION_MISMATCH";
    case PRO_TK_APP_COMM_FAILURE:
        return "PRO_TK_APP_COMM_FAILURE";
    case PRO_TK_APP_NEW_VERSION:
        return "PRO_TK_APP_NEW_VERSION";
    case PRO_TK_APP_UNLOCK:
        return "PRO_TK_APP_UNLOCK";
    case PRO_TK_APP_JLINK_NOT_ALLOWED:
        return "PRO_TK_APP_JLINK_NOT_ALLOWED";
    default:
        return "";
    }
}

/*====================================================================*\
FUNCTION : toLowerCase
PURPOSE  : 引数で指定した文字列に大文字が含まれている場合、小文字にする
    ProMdlName* wName
 備考
\*====================================================================*/
ProError  toLowerCase(ProMdlName* wWords)
{
    ProCharPath cWords;
    ProCharPath cTemp;

    memcpy(cTemp, "", sizeof(cTemp));
    // string 型に変更
    ProWstringToString(cWords, *wWords);

    // 小文字->大文字に変更
    for (int iLoop = 0; iLoop < strlen(cWords); iLoop++) {
        cTemp[iLoop] = tolower(cWords[iLoop]);
    }

    // wstring 型に変更
    ProStringToWstring(*wWords, cTemp);
}


/*=========================================================================*\
    Function:	setWindchillOnline
    Purpose:	WindchillのOnline状況を確認し、オフラインの場合にオンラインとする
    WindchillInfo windchillInfo

\*=========================================================================*/
ProError setWindchillOnline(WindchillInfo windchillInfo) {
    ProBoolean is_online = PRO_B_FALSE;
    ProError status = PRO_TK_NO_ERROR;
    ProError status2 = PRO_TK_NO_ERROR;

    // Creoのオンライン状況を確認する
    status = ProServerIsOnline(windchillInfo.settingIniServer, &is_online);
    TRAIL_PRINT("%s(%d) : ProServerIsOnline = %s / is_online = %d", __func__, __LINE__, getProErrorMessage(status), is_online);

    if (!is_online) {
        // オフラインの場合
        wchar_t* aliased_url;

        // windchillの ログインID/パスワード取得
        status = ProBrowserAuthenticate(windchillInfo.settingIniUserID, windchillInfo.settingIniPasswords);
        TRAIL_PRINT("%s(%d) : ProBrowserAuthenticate = %s", __func__, __LINE__, getProErrorMessage(status));

        // サーバーをセッションに登録する
        status = ProServerRegister(windchillInfo.settingIniServer, windchillInfo.settingIniUrl, windchillInfo.settingIniWorkspace, &aliased_url);
        TRAIL_PRINT("%s(%d) : ProServerRegister = %s / server=%w , URL=%w , workspace=%w", __func__, __LINE__, getProErrorMessage(status),
            windchillInfo.settingIniServer, windchillInfo.settingIniUrl, windchillInfo.settingIniWorkspace);

        if (status != PRO_TK_NO_ERROR) {
            // セッション登録に失敗した場合は、一度接続を削除してから再接続する
            status = ProServerUnregister(windchillInfo.settingIniServer);
            TRAIL_PRINT("%s(%d) : ProServerUnregister = %s", __func__, __LINE__, getProErrorMessage(status));

            if (status == PRO_TK_CHECKOUT_CONFLICT) {
                // 内容にチェックアウトされたオブジェクトがあるため、ワークスペースを削除できません

                /***********************************************
                 Windchill ワークスペースの削除
                *************************************************/
                ProServerRemoveConflicts conflicts = NULL;

                // ワークスペースからモデルを削除する
                status = ProServerObjectsRemove(NULL, &conflicts);
                TRAIL_PRINT("%s(%d) : ProServerObjectsRemove = %s", __func__, __LINE__, getProErrorMessage(status));
            }


            // windchillの ログインID/パスワード取得
            status = ProBrowserAuthenticate(windchillInfo.settingIniUserID, windchillInfo.settingIniPasswords);
            TRAIL_PRINT("%s(%d) : ProBrowserAuthenticate = %s", __func__, __LINE__, getProErrorMessage(status));

            // サーバーをセッションに登録する
            status = ProServerRegister(windchillInfo.settingIniServer, windchillInfo.settingIniUrl, windchillInfo.settingIniWorkspace, &aliased_url);
            TRAIL_PRINT("%s(%d) : ProServerRegister = %s / server=%w , URL=%w , workspace=%w", __func__, __LINE__, getProErrorMessage(status),
                windchillInfo.settingIniServer, windchillInfo.settingIniUrl, windchillInfo.settingIniWorkspace);
        }
    }
    else {
        // windchillの ログインID/パスワード取得
        status = ProBrowserAuthenticate(windchillInfo.settingIniUserID, windchillInfo.settingIniPasswords);
        TRAIL_PRINT("%s(%d) : ProBrowserAuthenticate = %s", __func__, __LINE__, getProErrorMessage(status));

    }
    // 接続先をアクティブにする
    status2 = ProServerActivate(windchillInfo.settingIniServer);
    TRAIL_PRINT("%s(%d) : ProServerActivate = %s / wServer = %w", __func__, __LINE__, getProErrorMessage(status), windchillInfo.settingIniServer);

    // Creoのオンライン状況を確認する
    status = ProServerIsOnline(windchillInfo.settingIniServer, &is_online);
    TRAIL_PRINT("%s(%d) : ProServerIsOnline = %s / is_online = %d", __func__, __LINE__, getProErrorMessage(status), is_online);

    if (is_online && status2 == PRO_TK_NO_ERROR) {
        // オンライン
        return PRO_TK_NO_ERROR;
    }
    else {
        // オフライン
        return PRO_TK_GENERAL_ERROR;
    }

}