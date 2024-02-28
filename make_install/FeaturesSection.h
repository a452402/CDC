/*
* FeaturesSection.h
*/
#pragma once
#include "InputFile.h"

//------------------------------------------------
//  �}�N����`(Macro definition)
//------------------------------------------------
#define MAX_WORDS_IN_CELL 32767
#define COMMA_SEPARATION    ","


//------------------------------------------------
//  �^��`(Type definition)
//------------------------------------------------

/*-----------------------------------------------------------------*\
    �\����
\*-----------------------------------------------------------------*/
// FeatureList�̒��g�i�[�p
typedef struct
{
    wchar_t wFeatureNumber[MAX_WORDS_IN_CELL];   // Feature number
    wchar_t wFeatureName[MAX_WORDS_IN_CELL];     // Fetaure name
    wchar_t wTargetX[MAX_WORDS_IN_CELL];         // Feature �� Frame Template �ɖ����ꍇ�ł����O��NOK�ƕ\�������Ȃ�
    wchar_t wMemo[MAX_WORDS_IN_CELL];            // Memo
}FeatureList;


//------------------------------------------------
//  �v���g�^�C�v�錾(Prototype declaration)
//------------------------------------------------
ProError setFeaturesSection(ProMdl top_asm, InputFileFeature* strFeat, int iSectionMaxRows, FeatureList* FeatureList);
ProError  loadFeatureList(ProCharPath strFileName);

extern FeatureList* gstrFeatureList;
