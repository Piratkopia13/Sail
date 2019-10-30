#pragma once

#ifdef _DEBUG
const float TICKRATE = 32.0f;
#else
const float TICKRATE = 64.0f;
#endif


//const float TIMESTEP = 1.0f / (5.0f*TICKRATE); // if you want slow-motion
const float TIMESTEP = 1.0f / TICKRATE;