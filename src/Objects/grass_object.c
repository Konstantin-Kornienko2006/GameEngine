#include "Objects/grass_object.h"

#include <vulkan/vulkan.h>

#include "Core/e_buffer.h"
#include "Core/pipeline.h"

#include "Tools/e_tools.h"
#include "Tools/e_shaders.h"

#include "Data/e_resource_shapes.h"
#include "Data/e_resource_export.h"

void GrassObjectSetDefaultDescriptor(GrassObject *grass, DrawParam *dParam)
{   
    uint32_t num_pack = BluePrintInit(&grass->go.graphObj.blueprints);
    
    ShaderBuilder *vert = grass->go.self.vert;
    ShaderBuilder *frag = grass->go.self.frag;

    ShadersMakeDefault3DShader(vert, frag, grass->go.num_images > 0);

    GraphicsObjectSetSomeShader(&grass->go.graphObj, vert->code, vert->size, num_pack);
    GraphicsObjectSetSomeShader(&grass->go.graphObj, frag->code, frag->size, num_pack);

    BluePrintAddSomeUpdater(&grass->go.graphObj.blueprints, num_pack, 0, GameObject3DDescriptorModelUpdate);
    BluePrintSetTextureImageCreate(&grass->go.graphObj.blueprints, num_pack, &grass->go.images[0], 0);
}

void GrassObjectSetInstanceDescriptor(GrassObject *grass, DrawParam *dParam)
{
    uint32_t num_pack = BluePrintInit(&grass->go.graphObj.blueprints);

    GraphicsObjectSetSomeShader(&grass->go.graphObj, &_binary_shaders_3d_object_instance_vert_spv_start, (size_t)(&_binary_shaders_3d_object_instance_vert_spv_size), num_pack);
    GraphicsObjectSetSomeShader(&grass->go.graphObj, &_binary_shaders_3d_object_instance_frag_spv_start, (size_t)(&_binary_shaders_3d_object_instance_frag_spv_size), num_pack);

    BluePrintAddSomeUpdater(&grass->go.graphObj.blueprints, num_pack, 0, GameObject3DDescriptorModelUpdate);
    BluePrintSetTextureImageCreate(&grass->go.graphObj.blueprints, num_pack, &grass->go.images[0], 0);
    
    /*setting.vert_indx = 0;
    setting.cull_mode = VK_CULL_MODE_NONE;*/
}

void GrassObjectInit(GrassObject *grass, DrawParam *dParam)
{
    GameObject3DInit((GameObject3D *)grass);

    vertexParam vParam;
    indexParam iParam;

    InitGrass3D(&vParam, &iParam);

    GraphicsObjectSetVertex(&grass->go.graphObj, vParam.vertices, vParam.verticesSize, sizeof(Vertex3D), iParam.indices, iParam.indexesSize, sizeof(uint32_t));

    FreeMemory(vParam.vertices);
    FreeMemory(iParam.indices);

    GameObject3DInitTextures((GameObject3D *)grass, dParam);
}
