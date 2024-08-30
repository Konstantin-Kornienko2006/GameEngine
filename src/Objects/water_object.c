#include "Objects/water_object.h"

#include <vulkan/vulkan.h>

#include "Core/pipeline.h"

#include "Tools/e_tools.h"

#include "wManager/window_manager.h"

#include "Data/e_resource_export.h"

void WaterObjectUpdateBuffer(GameObject3D* go, BluePrintDescriptor *descriptor)
{

    double time = wManagerGetTime();

    WaterBuffer wb;

    wb.time = time;

    DescriptorUpdate(descriptor, (char *)&wb, sizeof(wb));
}


void WaterObjectSetDefaultDescriptor(WaterObject *water, DrawParam *dParam)
{    
    uint32_t num_pack = BluePrintInit(&water->go.graphObj.blueprints);

    GraphicsObjectSetSomeShader(&water->go.graphObj, &_binary_shaders_water_vert_spv_start, (size_t)(&_binary_shaders_water_vert_spv_size), num_pack);
    GraphicsObjectSetSomeShader(&water->go.graphObj, &_binary_shaders_water_frag_spv_start, (size_t)(&_binary_shaders_water_frag_spv_size), num_pack);

    BluePrintAddSomeUpdater(&water->go.graphObj.blueprints, num_pack, 0, GameObject3DDescriptorModelUpdate);
    BluePrintAddSomeUpdater(&water->go.graphObj.blueprints, num_pack, 1, WaterObjectUpdateBuffer);
    BluePrintSetTextureImageCreate(&water->go.graphObj.blueprints, num_pack, &water->go.images[0], 0);
}

void WaterObjectInit(WaterObject *water, DrawParam *dParam, uint32_t size){

    GameObject3DInit((GameObject3D *)water);

    vertexParam vParam;
    indexParam iParam;

    InitPlane3D(&vParam, &iParam, size, size);

    GraphicsObjectSetVertex(&water->go.graphObj, vParam.vertices, vParam.verticesSize, sizeof(Vertex3D), iParam.indices, iParam.indexesSize, sizeof(uint32_t));

    FreeMemory(vParam.vertices);
    FreeMemory(iParam.indices);

    GameObject3DInitTextures((GameObject3D *)water, dParam);
}

void WaterObjectInitDefault(WaterObject *water, DrawParam *dParam, uint32_t size){
    WaterObjectInit(water, dParam, size);
    WaterObjectSetDefaultDescriptor(water, dParam);
    GameObject3DInitDraw((GameObject3D *)water);
}
