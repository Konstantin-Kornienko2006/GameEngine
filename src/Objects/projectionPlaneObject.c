#include "Objects/projectionPlaneObject.h"

#include <vulkan/vulkan.h>

#include "wManager/window_manager.h"

#include "Tools/e_math.h"
#include "Tools/e_tools.h"

#include "Core/e_camera.h"
#include "Core/e_blue_print.h"

#include "Data/e_resource_data.h"
#include "Data/e_resource_engine.h"
#include "Data/e_resource_shapes.h"
#include "Data/e_resource_export.h"

extern ZEngine engine;

void ProjectionPlaneUpdate(GameObject2D *go, BluePrintDescriptor *descriptor){

    Camera3D* cam = (Camera3D*) engine.cam3D;

    ProjDataBuffer pdf = {};
    pdf.time = wManagerGetTime();
    pdf.camRot = cam->rotation;
    pdf.camPos = v3_divs(cam->position, 10);

    DescriptorUpdate(descriptor, (char *)&pdf, sizeof(pdf));
}

void ProjectionPlaneInit(GameObject2D *go, DrawParam dParam){

    GameObject2DInit(go);

    GraphicsObjectSetVertex(&go->graphObj, (void *)projPlaneVert, 4, sizeof(Vertex2D), projPlaneIndx, 6, sizeof(uint32_t));

    GraphicsObjectSetShadersPath(&go->graphObj, dParam.vertShader, dParam.fragShader);

}

void ProjectionPlaneAddDefault(GameObject2D *go, void *render)
{    
    uint32_t num_pack = BluePrintInit(&go->graphObj.blueprints);
    
    GraphicsObjectSetSomeShader(&go->graphObj, &_binary_shaders_sprite_vert_spv_start, (size_t)(&_binary_shaders_sprite_vert_spv_size), num_pack);
    GraphicsObjectSetSomeShader(&go->graphObj, &_binary_shaders_sprite_frag_spv_start, (size_t)(&_binary_shaders_sprite_frag_spv_size), num_pack);

    BluePrintAddSomeUpdater(&go->graphObj.blueprints, num_pack, 0, ProjectionPlaneUpdate);
}
