/*
* AssemblyChange.h
*/
#pragma once

//------------------------------------------------
//  �}�N����`(Macro definition)
//------------------------------------------------

//------------------------------------------------
//  �^��`(Type definition)
//------------------------------------------------
enum ProcessEndMessage {
    ENUM_MESSAGE_INITIAL_VALUE,        // 0
    ENUM_MESSAGE_SUCCESS,              // 1
    ENUM_MESSAGE_CHECKIN_FAILED,       // 2
    ENUM_MESSAGE_SETTING_FAILED        // 3
};


//------------------------------------------------
//  �v���g�^�C�v�錾(Prototype declaration)
//------------------------------------------------
ProError mainCustom();

