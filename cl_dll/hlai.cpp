//
// hlapi.cpp
//
// implementation of HLAI class
//
#include <cmath>
#include <cstring>
#include "hlai.h"
#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "hudgl.h"

void HLAI::Init( void ) {
    // Init CVARs
    CVAR_CREATE("hlai_enabled", "0", FCVAR_ARCHIVE);
}

void HLAI::ServerInit( void ) {
    
}

bool HLAI::IsEnabled( void ) {
    return CVAR_GET_FLOAT("hlai_enabled") != 0;
}