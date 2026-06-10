/**
 * c_API.cpp
 *
 */

#include "SSystem/SComponent/c_API.h"

extern void mDoGph_BlankingON();
extern void mDoGph_BlankingOFF();
extern int mDoGph_BeforeOfDraw();
extern int mDoGph_AfterOfDraw();
extern int mDoGph_Painter();
extern int mDoGph_Create();

// These return int but the interface expects void(*)(void). Calling through a
// cast pointer with a different return type is UB and traps on wasm
// (call_indirect validates signatures), so wrap instead of casting.
static void cAPI_Create() { mDoGph_Create(); }
static void cAPI_BeforeOfDraw() { mDoGph_BeforeOfDraw(); }
static void cAPI_AfterOfDraw() { mDoGph_AfterOfDraw(); }
static void cAPI_Painter() { mDoGph_Painter(); }

cAPI_Interface g_cAPI_Interface = {
    cAPI_Create,
    cAPI_BeforeOfDraw,
    cAPI_AfterOfDraw,
    cAPI_Painter,
    mDoGph_BlankingON,
    mDoGph_BlankingOFF,
};
