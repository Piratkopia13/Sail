#pragma once

#ifdef _DEBUG
constexpr float TICKRATE = 32.0f;
#else
constexpr float TICKRATE = 64.0f;
#endif


//const float TIMESTEP = 1.0f / (5.0f*TICKRATE); // if you want slow-motion
constexpr float TIMESTEP = 1.0f / TICKRATE;