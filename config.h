#ifndef HEADER_RAY_CONFIG
#define HEADER_RAY_CONFIG

#ifndef RUN_BENCHMARK
//Normal settings
#define TRACE_DEPTH 5
#define AMBIENT_COLOUR (colour(32, 32, 32))
#define SPECULAR_POWER 20
#define LIGHT_FALLOFF 500
#define COL_MAX 255
#define SCREEN_RESOLUTION_WIDTH 1920
#define SCREEN_RESOLUTION_HEIGHT 1080
#define SCREEN_DISTANCE 5
#define SCREEN_SIZE 5.0
#define EYE_POSITION (vector3(0, 0, 0)) 
#define EYE_DIRECTION (vector3(0, 0, 1)) 
#define SUPER_SAMPLE 2
#define BACKGROUND_COLOUR (colour(32, 32, 32))
#define SHADOWS_ENABLE
#define REFLECTION_ENABLE


//Benchmark settings
#else
#define TRACE_DEPTH 3
#define AMBIENT_COLOUR (colour(25, 25, 25))
#define SPECULAR_POWER 20
#define LIGHT_FALLOFF 500
#define COL_MAX 255
#define SCREEN_RESOLUTION_WIDTH 500
#define SCREEN_RESOLUTION_HEIGHT 500
#define SCREEN_DISTANCE 5
#define SCREEN_SIZE 5.0
#define EYE_POSITION (vector3(0, 0, 4)) 
#define EYE_DIRECTION (vector3(-0.1, -0.2, 1)) 
//#define SUPER_SAMPLE 2
#define BACKGROUND_COLOUR (colour(0, 0, 0))
#define SHADOWS_ENABLE
#define REFLECTION_ENABLE
#endif

#endif
