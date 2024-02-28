/*
* ParametersSection.h
*/
#pragma once

#include "InputFile.h"

//------------------------------------------------
//  マクロ定義(Macro definition)
//------------------------------------------------

//------------------------------------------------
//  型定義(Type definition)
//------------------------------------------------

//------------------------------------------------
//  プロトタイプ宣言(Prototype declaration)
//------------------------------------------------
ProError  setParametersSection(ProMdl mdlTopAssy, ProPath layoutName, InputFileParamFeature* strFeature, InputFileParameters* strParam, int iFeatureSectionMaxRows, int iParameterSectionMaxRows);
