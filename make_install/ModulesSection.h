/*
* ModulesSection.h
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
ProError setModulesSection(ProMdl* top_asm, InputFileModules* strModules, InputFileHole* strHole, int iSectionMaxRows, int iHoleSectionMaxRows, int iSectionType);
ProError renameInModulesSection(InputFileRenameFile* strModulesRename, int iSectionMaxRows);


