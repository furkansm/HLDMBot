#pragma once
#include <cstdint>

class HLAI {
    public:
        void Init( void );
        void ServerInit( void );
        void Update( void );
        bool IsEnabled( void );

        /* From 0 to 1 */
        float input_sidemove = 0.0f;
        float input_forwardmove = 0.0f;
        float input_upmove = 0.0f;

        bool input_jump = false;
        bool input_attack1 = false;
        bool input_attack2 = false;
        bool input_reload = false;
        bool input_duck = false;

        /* Unit is degrees */
        float input_yaw = 0.0f;  
        float input_pitch = 0.0f;
};

extern HLAI hlai;