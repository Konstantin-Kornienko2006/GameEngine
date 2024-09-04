#include "Objects/primitiveObject.h"

#include <vulkan/vulkan.h>

#include "Tools/e_math.h"

#include "Core/e_memory.h"
#include "Core/e_camera.h"
#include "Core/e_buffer.h"
#include "Core/pipeline.h"
#include "Core/e_texture.h"
#include "Core/e_blue_print.h"

#include "Objects/light_object.h"
#include "Objects/render_texture.h"

#include "Tools/e_shaders.h"

#include "Data/e_resource_data.h"
#include "Data/e_resource_engine.h"
#include "Data/e_resource_shapes.h"
#include "Data/e_resource_descriptors.h"
#include "Data/e_resource_export.h"

extern ZEngine engine;

void PrimitiveObjectDestroy(PrimitiveObject *po)
{
    GameObject3DDestroy(po);

    FreeMemory(po->params);
    po->params = NULL;
}

int PrimitiveObjectInit(PrimitiveObject *po, DrawParam *dParam, char type, void *params){

    GameObject3DInit((GameObject3D *)po);

    GameObjectSetDestroyFunc((GameObject *)po, (void *)PrimitiveObjectDestroy);

    po->type = type;

    PlaneParam *pParam = (PlaneParam *)params;
    SphereParam *sParam = (SphereParam *)params;
    CubeSphereParam *csParam = (CubeSphereParam *)params;
    ConeParam *cParam = (ConeParam *)params;

    int builded = false;

    vertexParam vParam;
    indexParam iParam;

    po->params = NULL;
    switch(type)
    {
        case ENGINE_PRIMITIVE3D_LINE :
            GraphicsObjectSetVertex(&po->go.graphObj, (void *)lineVert, 2, sizeof(Vertex3D), NULL, 0, sizeof(uint32_t));
            break;
        case ENGINE_PRIMITIVE3D_TRIANGLE :
            GraphicsObjectSetVertex(&po->go.graphObj, (void *)triVert, 3, sizeof(Vertex3D), triIndx, 3, sizeof(uint32_t));
            break;
        case ENGINE_PRIMITIVE3D_QUAD :
            GraphicsObjectSetVertex(&po->go.graphObj, (void *)quadVert, 4, sizeof(Vertex3D), quadIndx, 6, sizeof(uint32_t));
            break;
        case ENGINE_PRIMITIVE3D_PLANE :
            InitPlane3D(&vParam, &iParam, pParam->sectorCount, pParam->stackCount);
            po->params = AllocateMemory(1, sizeof(PlaneParam));
            memcpy(po->params, params, sizeof(PlaneParam));
            builded = true;
            break;
        case ENGINE_PRIMITIVE3D_CUBE :
            GraphicsObjectSetVertex(&po->go.graphObj, (void *)cubeVert, 24, sizeof(Vertex3D), cubeIndx, 36, sizeof(uint32_t));
            break;
        case ENGINE_PRIMITIVE3D_CUBESPHERE :
            Cubesphere(&vParam, &iParam, csParam->radius, csParam->verperrow);
            po->params = AllocateMemory(1, sizeof(CubeSphereParam));
            memcpy(po->params, params, sizeof(CubeSphereParam));
            builded = true;
            break;
        case ENGINE_PRIMITIVE3D_ICOSPHERE :
            IcoSphereGenerator(&vParam, &iParam, sParam->radius);
            po->params = AllocateMemory(1, sizeof(SphereParam));
            memcpy(po->params, params, sizeof(SphereParam));
            break;
        case ENGINE_PRIMITIVE3D_SPHERE :
            SphereGenerator3D(&vParam, &iParam, sParam->radius, sParam->sectorCount, sParam->stackCount);
            po->params = AllocateMemory(1, sizeof(SphereParam));
            memcpy(po->params, params, sizeof(SphereParam));
            builded = true;
            break;
        case ENGINE_PRIMITIVE3D_CONE :
            ConeGenerator(&vParam, &iParam, cParam->height, cParam->sectorCount, cParam->stackCount);
            po->params = AllocateMemory(1, sizeof(ConeParam));
            memcpy(po->params, params, sizeof(ConeParam));
            builded = true;
            break;
        case ENGINE_PRIMITIVE3D_SKYBOX:
            SphereGenerator3D(&vParam, &iParam, sParam->radius, sParam->sectorCount, sParam->stackCount);
            po->params = AllocateMemory(1, sizeof(SphereParam));
            memcpy(po->params, params, sizeof(SphereParam));
            builded = true;
            break;
    }

    if(builded)
    {
        GraphicsObjectSetVertex(&po->go.graphObj, vParam.vertices, vParam.verticesSize, sizeof(Vertex3D), iParam.indices, iParam.indexesSize, sizeof(uint32_t));
        FreeMemory(vParam.vertices);
        FreeMemory(iParam.indices);
    }

    GameObject3DInitTextures((GameObject3D *)po, dParam);

    if(type == ENGINE_PRIMITIVE3D_SKYBOX)
        Transform3DSetScale((GameObject3D *)po, -500, -500, -500);

    return 1;
}

void PrimitiveObjectSetShadowDefaultDescriptor(PrimitiveObject *po)
{
    /*BluePrintAddUniformObject(&po->go.graphObj.blueprints, nums, sizeof(ModelBuffer3D), VK_SHADER_STAGE_VERTEX_BIT, (void *)GameObject3DDescriptorModelUpdate, 0);
    BluePrintAddUniformObject(&po->go.graphObj.blueprints, nums, sizeof(DirLightBuffer), VK_SHADER_STAGE_FRAGMENT_BIT, (void *)GameObject3DDescriptorDirLightsUpdate, 0);
    BluePrintAddUniformObject(&po->go.graphObj.blueprints, nums, sizeof(PointLightBuffer), VK_SHADER_STAGE_FRAGMENT_BIT, (void *)GameObject3DDescriptorPointLightsUpdate, 0);
    BluePrintAddUniformObject(&po->go.graphObj.blueprints, nums, sizeof(SpotLightBuffer), VK_SHADER_STAGE_FRAGMENT_BIT, (void *)GameObject3DDescriptorSpotLightsUpdate, 0);
    BluePrintAddUniformObject(&po->go.graphObj.blueprints, nums, sizeof(LightStatusBuffer), VK_SHADER_STAGE_FRAGMENT_BIT, (void *)GameObject3DLigtStatusBufferUpdate, 0);

    RenderTexture **renders = engine.DataR.dir_shadow_array;

    if(engine.DataR.num_dir_shadows > 1)
        BluePrintAddRenderImageArray(&po->go.graphObj.blueprints, nums, renders, engine.DataR.num_dir_shadows);
    else
        BluePrintAddRenderImage(&po->go.graphObj.blueprints, nums, renders[0]);

    renders = engine.DataR.point_shadow_array;

    if(engine.DataR.num_point_shadows > 1)
        BluePrintAddRenderImageArray(&po->go.graphObj.blueprints, nums, renders, engine.DataR.num_point_shadows);
    else
        BluePrintAddRenderImage(&po->go.graphObj.blueprints, nums, renders[0]);

    renders = engine.DataR.spot_shadow_array;

    if(engine.DataR.num_spot_shadows > 1)
        BluePrintAddRenderImageArray(&po->go.graphObj.blueprints, nums, renders, engine.DataR.num_spot_shadows);
    else
        BluePrintAddRenderImage(&po->go.graphObj.blueprints, nums, renders[0]);

    BluePrintAddTextureImage(&po->go.graphObj.blueprints, nums, &po->go.images[0], VK_SHADER_STAGE_FRAGMENT_BIT);
    BluePrintAddTextureImage(&po->go.graphObj.blueprints, nums, &po->go.images[1], VK_SHADER_STAGE_FRAGMENT_BIT);*/


    /*uint32_t num_pack = BluePrintInit(&po->go.graphObj.blueprints);
    
    GraphicsObjectSetSomeShader(&po->go.graphObj, &_binary_shaders_3d_object_shadow_vert_spv_start, (size_t)(&_binary_shaders_3d_object_shadow_vert_spv_size), num_pack);
    GraphicsObjectSetSomeShader(&po->go.graphObj, &_binary_shaders_3d_object_shadow_frag_spv_start, (size_t)(&_binary_shaders_3d_object_shadow_frag_spv_size), num_pack);

    BluePrintAddSomeUpdater(&po->go.graphObj.blueprints, num_pack, 0, GameObject3DDescriptorModelUpdate);
    BluePrintSetTextureImageCreate(&po->go.graphObj.blueprints, num_pack, &po->go.images[0], 0);*/

}

void PrimitiveObjectSetBigDesriptor(PrimitiveObject *po){

    uint32_t num_pack = BluePrintInit(&po->go.graphObj.blueprints);

    /*BluePrintAddUniformObject(&po->go.graphObj.blueprints, num_pack, sizeof(ModelBuffer3D), VK_SHADER_STAGE_VERTEX_BIT, (void *)GameObject3DDescriptorModelUpdate, 0);
    BluePrintAddUniformObject(&po->go.graphObj.blueprints, num_pack, sizeof(DirLightBuffer), VK_SHADER_STAGE_FRAGMENT_BIT, (void *)GameObject3DDescriptorDirLightsUpdate, 0);
    BluePrintAddUniformObject(&po->go.graphObj.blueprints, num_pack, sizeof(PointLightBuffer), VK_SHADER_STAGE_FRAGMENT_BIT, (void *)GameObject3DDescriptorPointLightsUpdate, 0);
    BluePrintAddUniformObject(&po->go.graphObj.blueprints, num_pack, sizeof(SpotLightBuffer), VK_SHADER_STAGE_FRAGMENT_BIT, (void *)GameObject3DDescriptorSpotLightsUpdate, 0);
    BluePrintAddUniformObject(&po->go.graphObj.blueprints, num_pack, sizeof(LightStatusBuffer), VK_SHADER_STAGE_FRAGMENT_BIT, (void *)GameObject3DLigtStatusBufferUpdate, 0);
    BluePrintAddTextureImage(&po->go.graphObj.blueprints, num_pack, &po->go.images[0], VK_SHADER_STAGE_FRAGMENT_BIT);
    BluePrintAddTextureImage(&po->go.graphObj.blueprints, num_pack, &po->go.images[1], VK_SHADER_STAGE_FRAGMENT_BIT);
    
    PipelineSetting* setting = (PipelineSetting *)&po->go.graphObj.blueprints.blue_print_packs[num_pack].setting;

    PipelineSettingSetShader(setting, &_binary_shaders_3d_object_vert_spv_start, (size_t)(&_binary_shaders_3d_object_vert_spv_size), VK_SHADER_STAGE_VERTEX_BIT);
    PipelineSettingSetShader(setting, &_binary_shaders_3d_object_frag_spv_start, (size_t)(&_binary_shaders_3d_object_frag_spv_size), VK_SHADER_STAGE_FRAGMENT_BIT);*/
        
    /*ShaderObject vert_shader, frag_shader;

    vert_shader.code = &_binary_shaders_3d_object_vert_spv_start;
    vert_shader.size = (size_t)(&_binary_shaders_3d_object_vert_spv_size);
    
    frag_shader.code = &_binary_shaders_3d_object_frag_spv_start;
    frag_shader.size = (size_t)(&_binary_shaders_3d_object_frag_spv_size);

    GraphicsObjectSetSomeShader(&po->go.graphObj, &vert_shader, num_pack);
    GraphicsObjectSetSomeShader(&po->go.graphObj, &frag_shader, num_pack);
    
    GameObject3DSetDescriptorUpdate(po, num_pack, 0, GameObject3DDescriptorModelUpdate);
    GameObject3DSetDescriptorUpdate(po, num_pack, 1, GameObject3DDescriptorDirLightsUpdate);
    GameObject3DSetDescriptorUpdate(po, num_pack, 2, GameObject3DDescriptorPointLightsUpdate);
    GameObject3DSetDescriptorUpdate(po, num_pack, 3, GameObject3DDescriptorSpotLightsUpdate);
    GameObject3DSetDescriptorUpdate(po, num_pack, 4, GameObject3DLigtStatusBufferUpdate);
    GameObject3DSetDescriptorTextureCreate(po, num_pack, 5, &po->go.images[0]);
    GameObject3DSetDescriptorTextureCreate(po, num_pack, 6, &po->go.images[1]);

    po->go.self.flags |= ENGINE_GAME_OBJECT_FLAG_SHADED;*/
}

void PrimitiveObjectSetInstanceDescriptor(PrimitiveObject *po)
{
    /*uint32_t num_pack = BluePrintInit(&po->go.graphObj.blueprints);
    
    ShaderObject vert_shader, frag_shader;

    vert_shader.code = &_binary_shaders_3d_object_instance_vert_spv_start;
    vert_shader.size = (size_t)(&_binary_shaders_3d_object_instance_vert_spv_size);
    
    frag_shader.code = &_binary_shaders_3d_object_instance_frag_spv_start;
    frag_shader.size = (size_t)(&_binary_shaders_3d_object_instance_frag_spv_size);

    GraphicsObjectSetSomeShader(&po->go.graphObj, &vert_shader, num_pack);
    GraphicsObjectSetSomeShader(&po->go.graphObj, &frag_shader, num_pack);

    BluePrintAddSomeUpdater(&po->go.graphObj.blueprints, num_pack, 0, GameObject3DDescriptorModelUpdate);
    BluePrintSetTextureImageCreate(&po->go.graphObj.blueprints, num_pack, &po->go.images[0], 0);
    
    po->go.self.flags |= ENGINE_GAME_OBJECT_FLAG_SHADED;*/
}

void *PrimitiveObjectGetVertex(PrimitiveObject *po)
{
    return &po->go.graphObj.shapes[0].vParam;
}

void PrimitiveObjectAddShadow(PrimitiveObject *po, DrawParam *dParam)
{
    /*RenderTexture **renders = engine.DataR.dir_shadow_array;

    for(int i=0;i < engine.DataR.num_dir_shadows;i++)
        GameObject3DAddShadowDescriptor((GameObject3D *)po, ENGINE_LIGHT_TYPE_DIRECTIONAL, renders[i], i);

    renders = engine.DataR.point_shadow_array;

    for(int i=0;i < engine.DataR.num_point_shadows;i++)
        GameObject3DAddOmiShadow((GameObject3D *)po, renders[i], i);

    renders = engine.DataR.spot_shadow_array;

    for(int i=0;i < engine.DataR.num_spot_shadows;i++)
        GameObject3DAddShadowDescriptor((GameObject3D *)po, ENGINE_LIGHT_TYPE_SPOT, renders[i], i);*/

    PrimitiveObjectSetShadowDefaultDescriptor(po);
}

//Не корректно
void PrimitiveObjectDiffuseTextureSetData(PrimitiveObject *po, void *data, uint32_t size_data, uint32_t offset)
{
    /*ShaderDescriptor *descriprots = po->go.graphObj.blueprints.descriptors;

    TextureUpdate(&descriprots[2], data, size_data, offset);*/
}

//Не корректно
void PrimitiveObjectDiffuseSetTexture(PrimitiveObject *po, const char *path)
{
    /*ShaderDescriptor *descriprots = po->go.graphObj.blueprints.descriptors;

    TextureSetTexture(&descriprots[2], path);*/
}
