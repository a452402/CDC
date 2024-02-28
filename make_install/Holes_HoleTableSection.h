/*
* Holes_HoleTableSection.h
*/
#pragma once
#include "InputFile.h"

//------------------------------------------------
//  �}�N����`(Macro definition)
//------------------------------------------------
#define HOLE 0
#define INNERLINER_HOLE 1

//------------------------------------------------
//  �^��`(Type definition)
//------------------------------------------------

//------------------------------------------------
//  �v���g�^�C�v�錾(Prototype declaration)
//------------------------------------------------
ProError  setHoles_HoleTableSection(ProMdl* top_asm, ProMdl* mdlSideFrame, ProPath patName, InputFileHole* strHole, InputFileHoleTable* strHoleTable, int iHoleSectionMaxRows, int iHoleTableSectionMaxRows, int iHoletype);

