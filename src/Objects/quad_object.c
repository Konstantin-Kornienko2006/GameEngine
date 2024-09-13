#include "Objects/quad_object.h"
#include "Objects/render_texture.h"

#include <vulkan/vulkan.h>

#include "Core/pipeline.h"
#include "Core/e_buffer.h"
#include "Core/e_camera.h"

#include "Tools/e_math.h"

#include "Data/e_resource_data.h"
#include "Data/e_resource_engine.h"
#include "Data/e_resource_export.h"
#include "Data/e_resource_shapes.h"

void QuadObjectUpdate(GameObject2D *go, BluePrintDescriptor *descriptor)
{
    TransformBuffer2D tbo;

    tbo.position = v2_subs(go->transform.position, 0.5f);
    tbo.rotation = go->transform.rotation;
    tbo.scale = go->transform.scale;

    DescriptorUpdate(descriptor, (char *)&tbo, sizeof(tbo));
}

void QuadObjectInit(QuadObject *qu, uint32_t type)
{
    GameObject2DInit((GameObject2D *)qu);

    GraphicsObjectSetVertex(&qu->go.graphObj, (void *)projPlaneVert, 4, sizeof(Vertex2D), (uint32_t *)projPlaneIndx, 6, sizeof(uint32_t));

    qu->type = type;

    qu->go.graphObj.num_shapes = 1;
}

void QuadObjectAddDefault(QuadObject *qu, void *render)
{
    /*uint32_t num_pack = BluePrintInit(&qu->go.graphObj.blueprints);
    
    GraphicsObjectSetSomeShader(&qu->go.graphObj, &_binary_shaders_quad_vert_spv_start, (size_t)(&_binary_shaders_quad_vert_spv_size), num_pack);

     switch(qu->type == ENGINE_QUAD_TYPE_DEPTH){
        case ENGINE_QUAD_TYPE_DEPTH:
            GraphicsObjectSetSomeShader(&qu->go.graphObj,  &_binary_shaders_quad_frag_spv_start, (size_t)(&_binary_shaders_quad_frag_spv_size), num_pack);
            break;
        case ENGINE_QUAD_TYPE_CUBE:
            GraphicsObjectSetSomeShader(&qu->go.graphObj,  &_binary_shaders_quad_frag_2_spv_start, (size_t)(&_binary_shaders_quad_frag_2_spv_size), num_pack);
            break;
        case ENGINE_QUAD_TYPE_IMAGE:
            GraphicsObjectSetSomeShader(&qu->go.graphObj,  &_binary_shaders_quad_frag_3_spv_start, (size_t)(&_binary_shaders_quad_frag_3_spv_size), num_pack);
            break;
    }

    BluePrintAddSomeUpdater(&qu->go.graphObj.blueprints, num_pack, 0, QuadObjectUpdate);*/
    
    /*setting.flags &= ~(ENGINE_PIPELINE_FLAG_ALPHA);
    setting.vert_indx = 0;*/
}
