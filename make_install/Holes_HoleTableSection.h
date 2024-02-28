/*
* Holes_HoleTableSection.h
*/
#pragma once
#include "InputFile.h"

//------------------------------------------------
//  マクロ定義(Macro definition)
//------------------------------------------------
#define HOLE 0
#define INNERLINER_HOLE 1

//------------------------------------------------
//  型定義(Type definition)
//------------------------------------------------

//------------------------------------------------
//  プロトタイプ宣言(Prototype declaration)
//------------------------------------------------
ProError  setHoles_HoleTableSection(ProMdl* top_asm, ProMdl* mdlSideFrame, ProPath patName, InputFileHole* strHole, InputFileHoleTable* strHoleTable, int iHoleSectionMaxRows, int iHoleTableSectionMaxRows, int iHoletype);

