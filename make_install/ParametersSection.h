/*
* ParametersSection.h
*/
#pragma once

#include "InputFile.h"

//------------------------------------------------
//  �}�N����`(Macro definition)
//------------------------------------------------

//------------------------------------------------
//  �^��`(Type definition)
//------------------------------------------------

//------------------------------------------------
//  �v���g�^�C�v�錾(Prototype declaration)
//------------------------------------------------
ProError  setParametersSection(ProMdl mdlTopAssy, ProPath layoutName, InputFileParamFeature* strFeature, InputFileParameters* strParam, int iFeatureSectionMaxRows, int iParameterSectionMaxRows);
