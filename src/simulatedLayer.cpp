#include "simulatedLayer.h"
#include <openxr/openxr_platform.h>
#include <GL/GL.h>
#include <common/gfxwrapper_opengl.h>

#include <chrono>
using namespace std::chrono;

#include "common.h"
#include "compatibility.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

static bool useInterception = true;

//#define VERBOSE
const char* _simulatedLayer = "XR_aardworx_streamout";
#define log std::cout << _simulatedLayer << ":: "


