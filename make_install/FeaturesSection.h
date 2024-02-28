/*
* FeaturesSection.h
*/
#pragma once
#include "InputFile.h"

//------------------------------------------------
//  マクロ定義(Macro definition)
//------------------------------------------------
#define MAX_WORDS_IN_CELL 32767
#define COMMA_SEPARATION    ","


//------------------------------------------------
//  型定義(Type definition)
//------------------------------------------------

/*-----------------------------------------------------------------*\
    構造体
\*-----------------------------------------------------------------*/
// FeatureListの中身格納用
typedef struct
{
    wchar_t wFeatureNumber[MAX_WORDS_IN_CELL];   // Feature number
    wchar_t wFeatureName[MAX_WORDS_IN_CELL];     // Fetaure name
    wchar_t wTargetX[MAX_WORDS_IN_CELL];         // Feature が Frame Template に無い場合でもログにNOKと表示させない
    wchar_t wMemo[MAX_WORDS_IN_CELL];            // Memo
}FeatureList;


//------------------------------------------------
//  プロトタイプ宣言(Prototype declaration)
//------------------------------------------------
ProError setFeaturesSection(ProMdl top_asm, InputFileFeature* strFeat, int iSectionMaxRows, FeatureList* FeatureList);
ProError  loadFeatureList(ProCharPath strFileName);

extern FeatureList* gstrFeatureList;
