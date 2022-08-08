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

    float x = atof(gEngfuncs.Cmd_Argv(1));
    float y = atof(gEngfuncs.Cmd_Argv(2));
    float z = atof(gEngfuncs.Cmd_Argv(3));
    AIWaypoint way(Vector(x, y, z));

    hlai.waypoints.push_back(way);
}

float _get_vector_angle(Vector vec1, Vector vec2) {
    auto diff = (vec1 - vec2);

    auto result = atan2(diff.y, diff.x) * 180 / 3.1415926535f;
    return result;

}

AIWaypoint* HLAI::ClosestWaypoint( void ) {
    auto pos = gEngfuncs.GetLocalPlayer()->origin;
    
    float minDist = FLT_MAX;
    AIWaypoint* closestWaypoint = NULL;

    for (auto &waypoint : hlai.waypoints)
    {
        auto dist = ( waypoint.location - pos ).Length();
        if (dist < minDist) {
            minDist = dist;
            closestWaypoint = &waypoint;
        }
    }

    return closestWaypoint;
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
    if (ClosestWaypoint() == NULL) {
        ConsolePrint("closest waypoint null");
        return;
    }
    input_yaw = _get_vector_angle(ClosestWaypoint()->location, gEngfuncs.GetLocalPlayer()->origin);
}

bool HLAI::IsEnabled( void ) {
    return CVAR_GET_FLOAT("hlai_enabled") != 0;
}