#pragma once
#include <cstdint>

class HLAI {
    public:
        void Init( void );
        void ServerInit( void );
        bool IsEnabled( void );
};

extern HLAI hlai;