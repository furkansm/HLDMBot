//
// hlapi.cpp
//
// implementation of HLAI class
//
#include <cmath>
#include <cstring>
#include <vector>
#include <ctime>
#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "hudgl.h"

enum AIState {
    IDLE,
    NAVIGATE,
    SHOOT
};

bool AI_InitDone = false;
std::vector<AIWaypoint*> AI_WaypointHistory;

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
    if (gEngfuncs.Cmd_Argc() != 5 ) {
        ConsolePrint("hlai_addwaypoint <x> <y> <z> <group>\n");
        return;
    }

    float x = atof(gEngfuncs.Cmd_Argv(1));
    float y = atof(gEngfuncs.Cmd_Argv(2));
    float z = atof(gEngfuncs.Cmd_Argv(3));
    auto waypoint = new AIWaypoint(Vector(x, y, z), hlai.waypoints.size(), atoi(gEngfuncs.Cmd_Argv(4)));
    
    if (!hlai.waypoints.empty()) {
        auto lastWaypoint = hlai.waypoints.back();
        
        if (lastWaypoint->group == waypoint->group) {
            lastWaypoint->next = waypoint->id;
            ConsolePrint("Added: ");
            ConsolePrint(std::to_string(lastWaypoint->next).c_str());
            ConsolePrint("\n");
        }
    }
    hlai.waypoints.push_back(waypoint);
}

void _callback_hlai_clearwaypoints() {
    hlai.waypoints.clear();
    ConsolePrint("Waypoints cleared.\n");
}

long int _get_now() {
    return static_cast<long int>(std::time(0));
}

float _lerp(float a, float b, float f)
{
    return a + f * (b - a);
}

float _get_vector_angle(Vector vec1, Vector vec2) {
    auto diff = (vec1 - vec2);

    auto result = atan2(diff.y, diff.x) * 180 / 3.1415926535f;
    return result;
}

float _get_distance(Vector vec1) {
    return (vec1 - gEngfuncs.GetLocalPlayer()->origin).Length();
}

AIWaypoint* _ai_get_closest_waypoint( int excludeGroup = -1, int excludeLastWaypointCount = 8 ) {
    auto pos = gEngfuncs.GetLocalPlayer()->origin;
    
    float minDist = FLT_MAX;
    AIWaypoint* closestWaypoint = NULL;
    excludeLastWaypointCount = min(AI_WaypointHistory.size(), excludeLastWaypointCount);
    std::vector<AIWaypoint*> _excludedWaypoints(AI_WaypointHistory.end() - excludeLastWaypointCount, AI_WaypointHistory.end());

    for (auto &waypoint : hlai.waypoints)
    {
        bool abort = false;
        if (waypoint->group == excludeGroup) {
            continue;
        }

        for (auto &excluded : _excludedWaypoints) { // Exclude if we visited this waypoint before
            if (excluded->id == waypoint->id) {
                abort = true;
                break;
            }   
        }

        if (abort)
            continue;

        
        auto diff = ( waypoint->location - pos ).Length();
        
        if (diff < minDist) {
            minDist = diff;
            closestWaypoint = waypoint;
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
    gEngfuncs.pfnAddCommand("hlai_clearwaypoints", _callback_hlai_clearwaypoints);
}


void HLAI::ServerInit( void ) {
    _callback_hlai_clearwaypoints();
    EngineClientCmd("exec waypoints.cfg");

}

AIWaypoint* AI_TargetWaypoint = NULL;
AIState AI_State = IDLE;
long int AI_LastWaypointChangeTime = 0;
bool AI_ForceNewWaypoint = false;
int AI_IgnoreGroup = -1;

void HLAI::Update( void ) {
    if (!AI_InitDone) {
        AI_InitDone = true;
        ServerInit();
        return;
    }

    if (!IsEnabled())
        return;

    if (hlai.waypoints.size() == 0) 
        return;
    
    long int timeSpentNavigating;
    float distanceToWaypoint = 0;

    switch (AI_State)
    {
    case IDLE:
        AI_State = NAVIGATE;
        break;
    
    case NAVIGATE:
        timeSpentNavigating = _get_now() - AI_LastWaypointChangeTime;
        if (AI_TargetWaypoint != NULL)
            distanceToWaypoint = _get_distance(AI_TargetWaypoint->location);

        if (AI_TargetWaypoint != NULL && _get_distance(AI_TargetWaypoint->location) < 90.0f) { // Claim waypoint if we are close to it
            AI_TargetWaypoint = NULL;
            AI_State = IDLE;
            ConsolePrint("Waypoint done, idling.\n");
            break;
        }
        
        if (AI_TargetWaypoint == NULL) {
            AI_TargetWaypoint = _ai_get_closest_waypoint( AI_IgnoreGroup ); // Select the closest waypoint
            if (!AI_WaypointHistory.empty()) {
                auto lastWaypoint = AI_WaypointHistory.back();
                
                if (lastWaypoint->next != -1 && !AI_ForceNewWaypoint) { // Select the next waypoint of the previous waypoint
                    AI_TargetWaypoint = hlai.waypoints[lastWaypoint->next];
                }

                if (lastWaypoint->next == -1) { // If we've completed the entire waypoint group, ignore it for a while 
                    AI_IgnoreGroup = lastWaypoint->group;
                }

                AI_ForceNewWaypoint = false;
            }
            
            AI_WaypointHistory.push_back(AI_TargetWaypoint);
            AI_LastWaypointChangeTime = _get_now();

            input_jump = false;

            ConsolePrint("Waypoint Group ");
            ConsolePrint(std::to_string(AI_TargetWaypoint->group).c_str());
            ConsolePrint(" with id #");
            ConsolePrint(std::to_string(AI_TargetWaypoint->id).c_str());
            ConsolePrint("\n");
        }

        if (timeSpentNavigating >= 2 && distanceToWaypoint < 100) { // Start jumping after 2 secs.
            input_jump = true;
        }

        if (timeSpentNavigating >= 3) { // If the AI had been stuck for the last 3 seconds, remove the waypoint
            
            if (distanceToWaypoint > 800) { // Select new waypoint if we are far apart from current waypoint group
                AI_ForceNewWaypoint = true;
            }
            AI_TargetWaypoint = NULL;
            AI_IgnoreGroup = -1;
            break;
        }
        
        input_yaw = _lerp(input_yaw, _get_vector_angle(AI_TargetWaypoint->location, gEngfuncs.GetLocalPlayer()->origin), 0.08f); // Look at waypoint
        input_forwardmove = 1.0f; // Move forward

        break;
    
    default:
    // NOT POSSIBLE
        break;
    }
}

bool HLAI::IsEnabled( void ) {
    return CVAR_GET_FLOAT("hlai_enabled") != 0;
}