//
// hlapi.cpp
//
// implementation of HLAI class
//
#include <cmath>
#include <cstring>
#include <vector>
#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "hudgl.h"

void _callback_hlai_addwaypoint_cmdgen() {
    auto pos = gEngfuncs.GetLocalPlayer()->origin;
    ConsolePrint("hlai_addwaypoint ");
    ConsolePrint(std::to_string(pos.x).c_str());
    ConsolePrint(" ");
    ConsolePrint(std::to_string(pos.y).c_str());
    ConsolePrint(" ");
    ConsolePrint(std::to_string(pos.z).c_str());
    ConsolePrint("\n");

    PlaySound("common/wpn_denyselect.wav", 1);
}

void _callback_hlai_addwaypoint() {
    if (gEngfuncs.Cmd_Argc() != 4 ) {
        gEngfuncs.Con_Printf("hlai_addwaypoint <x> <y> <z>\n");
        return;
    }

    int x = atoi(gEngfuncs.Cmd_Argv(1));
    int y = atoi(gEngfuncs.Cmd_Argv(2));
    int z = atoi(gEngfuncs.Cmd_Argv(3));
    AIWaypoint way(Vector(x, y, z));

    hlai.waypoints.push_back(way);

    ConsolePrint(std::to_string(x).c_str());
    ConsolePrint("\n");
}

void HLAI::Init( void ) {
    // Init CVARs
   
    CVAR_CREATE("hlai_enabled", "0", FCVAR_ARCHIVE);

    // Init Commands
    gEngfuncs.pfnAddCommand("hlai_addwaypoint_cmdgen", _callback_hlai_addwaypoint_cmdgen);
    gEngfuncs.pfnAddCommand("hlai_addwaypoint", _callback_hlai_addwaypoint);
}


void HLAI::ServerInit( void ) {
    
}

void HLAI::Update( void ) {
    if (!IsEnabled())
        return;

    ConsolePrint("Update\n");
}

bool HLAI::IsEnabled( void ) {
    return CVAR_GET_FLOAT("hlai_enabled") != 0;
}