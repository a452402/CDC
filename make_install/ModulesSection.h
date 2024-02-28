/*
* ModulesSection.h
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
ProError setModulesSection(ProMdl* top_asm, InputFileModules* strModules, InputFileHole* strHole, int iSectionMaxRows, int iHoleSectionMaxRows, int iSectionType);
ProError renameInModulesSection(InputFileRenameFile* strModulesRename, int iSectionMaxRows);


