/*
* AssemblyChange.h
*/
#pragma once

//------------------------------------------------
//  マクロ定義(Macro definition)
//------------------------------------------------

//------------------------------------------------
//  型定義(Type definition)
//------------------------------------------------
enum ProcessEndMessage {
    ENUM_MESSAGE_INITIAL_VALUE,        // 0
    ENUM_MESSAGE_SUCCESS,              // 1
    ENUM_MESSAGE_CHECKIN_FAILED,       // 2
    ENUM_MESSAGE_SETTING_FAILED        // 3
};


//------------------------------------------------
//  プロトタイプ宣言(Prototype declaration)
//------------------------------------------------
ProError mainCustom();

