#include "Objects/gameObject2D.h"

#include <vulkan/vulkan.h>
#include <math.h>

#include "Core/e_device.h"
#include "Core/e_buffer.h"
#include "Core/pipeline.h"
#include "Core/e_camera.h"

#include "Tools/e_math.h"
#include "Tools/shader_builder.h"

#include "Variabels/e_pipeline_variables.h"

#include "Data/e_resource_data.h"
#include "Data/e_resource_engine.h"
#include "Data/e_resource_export.h"

extern ZEngine engine;

typedef void (*Update_Descriptor2D)(GameObject2D* go, BluePrintDescriptor *descriptor);

void GameObject2DTransformBufferUpdate(GameObject2D *go, BluePrintDescriptor *descriptor)
{
    TransformBuffer2D tbo;

    tbo.position = v2_subs(v2_add(go->transform.position, go->transform.scale), 1.0f);
    tbo.rotation = go->transform.rotation;
    tbo.scale = go->transform.scale;

    DescriptorUpdate(descriptor, (char *)&tbo, sizeof(tbo));
}

void GameObject2DImageBuffer(GameObject2D *go, BluePrintDescriptor *descriptor)
{
    ImageBufferObjects ibo;
    ibo.offset = go->transform.img.offset;
    ibo.rotation = go->transform.img.rotation;

    ibo.rotation.x = 0;
    ibo.rotation.y = 0;

    DescriptorUpdate(descriptor, (char *)&ibo, sizeof(ibo));
}

void GameObject2DDefaultUpdate(GameObject2D* go) {

    for(int i=0; i < go->graphObj.gItems.num_shader_packs;i++)
    {
        BluePrintPack *pack = &go->graphObj.blueprints.blue_print_packs[i];

        if(pack->render_point == engine.current_render)
        {
            for(int j=0;j < pack->num_descriptors;j++)
            {
                BluePrintDescriptor *descriptor = &pack->descriptors[j];

                if(descriptor->descrType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
                {
                    Update_Descriptor2D update = descriptor->update;
                    update(go, descriptor);
                }
            }
        }
    }
}

void GameObject2DDefaultDraw(GameObject2D* go){
    
    ZDevice *device = (ZDevice *)engine.device;
    
    VkCommandBuffer command = device->commandBuffers[engine.imageIndex];

    for(int i=0; i < go->graphObj.gItems.num_shader_packs;i++)
    {
        BluePrintPack *pack = &go->graphObj.blueprints.blue_print_packs[i];

        if(pack->render_point == engine.current_render)
        {
            ShaderPack *pack = &go->graphObj.gItems.shader_packs[i];

            for(int j=0; j < pack->num_pipelines; j++){
                vkCmdBindPipeline(command, VK_PIPELINE_BIND_POINT_GRAPHICS, pack->pipelines[j].pipeline);

                PipelineSetting *settings = &go->graphObj.blueprints.blue_print_packs[i].settings[j];

                vertexParam *vParam = &go->graphObj.shapes[settings->vert_indx].vParam;
                indexParam *iParam = &go->graphObj.shapes[settings->vert_indx].iParam;

                if(vParam->verticesSize == 0)
                    continue;

                if(settings->flags & ENGINE_PIPELINE_FLAG_DYNAMIC_VIEW){
                    vkCmdSetViewport(command, 0, 1, (const VkViewport *)&settings->viewport);
                    vkCmdSetScissor(command, 0, 1, (const VkRect2D *)&settings->scissor);
                }

                VkBuffer vertexBuffers[] = {vParam->buffer.buffer};
                VkDeviceSize offsets[] = {0};

                vkCmdBindVertexBuffers(command, 0, 1, vertexBuffers, offsets);
                vkCmdBindDescriptorSets(command, VK_PIPELINE_BIND_POINT_GRAPHICS, pack->pipelines[j].layout, 0, 1, &pack->descriptor.descr_sets[engine.imageIndex], 0, NULL);

                if(settings->flags & ENGINE_PIPELINE_FLAG_DRAW_INDEXED){

                    vkCmdBindIndexBuffer(command, iParam->buffer.buffer, 0, VK_INDEX_TYPE_UINT32);
                    vkCmdDrawIndexed(command, iParam->indexesSize, 1, 0, 0, 0);
                }else
                    vkCmdDraw(command, vParam->verticesSize, 1, 0, 0);
            }
        }
    }
}

void GameObject2DInitDraw(GameObject2D *go)
{
    GraphicsObjectCreateDrawItems(&go->graphObj);

    PipelineCreateGraphics(&go->graphObj);
}

void GameObject2DAddSettingPipeline(GameObject2D* go, uint32_t indx_pack, void *arg){

    BluePrintPack *pack = &go->graphObj.blueprints.blue_print_packs[indx_pack];

    if(pack->num_settings + 1 > MAX_UNIFORMS)
        return;

    PipelineSetting* setting = arg;


    PipelineSetting* settings = pack->settings;
    memcpy(&settings[pack->num_settings], setting, sizeof(PipelineSetting));

    pack->num_settings++;

}

void GameObject2DClean(GameObject2D* go){
    GraphicsObjectClean(&go->graphObj);
}

void GameObject2DRecreate(GameObject2D* go){

    GameObject2DClean(go);

    for(int i=0; i < go->graphObj.gItems.num_shader_packs;i++)
    {
        BluePrintPack *pack = &go->graphObj.blueprints.blue_print_packs[i];

        PipelineSetting *settings = pack->settings;

        for(int j=0; j < pack->num_settings;j++)
        {
            settings[j].scissor.offset.x = 0;
            settings[j].scissor.offset.y = 0;
            settings[j].scissor.extent.height = engine.height;
            settings[j].scissor.extent.width = engine.width;
            settings[j].viewport.x = 0;
            settings[j].viewport.y = 0;
            settings[j].viewport.height = engine.height;
            settings[j].viewport.width = engine.width;
        }
    }

    BuffersRecreateUniform(&go->graphObj.blueprints);

    GraphicsObjectCreateDrawItems(&go->graphObj);
    PipelineCreateGraphics(&go->graphObj);

    Transform2DReposition((struct GameObject2D *)go);
    Transform2DRescale((struct GameObject2D *)go);

}

void GameObject2DDestroy(GameObject2D* go){

    if(!go->self.init)
        return;

    GraphicsObjectDestroy(&go->graphObj);

    if(go->image != NULL)
    {
        FreeMemory(go->image->path);

        if(go->image->size > 0)
            FreeMemory(go->image->buffer);

        FreeMemory(go->image);
    }

    for(int i=0; i < go->graphObj.num_shapes; i++)
    {
        if(go->graphObj.shapes[i].vParam.verticesSize)
            FreeMemory(go->graphObj.shapes[i].vParam.vertices);

        if(go->graphObj.shapes[i].iParam.indexesSize)
            FreeMemory(go->graphObj.shapes[i].iParam.indices);
    }

    FreeMemory(go->self.vert);
    FreeMemory(go->self.frag);
    
    go->self.init = false;
}

void GameObject2DInit(GameObject2D* go)
{
    GameObjectSetUpdateFunc((GameObject *)go, (void *)GameObject2DDefaultUpdate);
    GameObjectSetDrawFunc((GameObject *)go, (void *)GameObject2DDefaultDraw);
    GameObjectSetCleanFunc((GameObject *)go, (void *)GameObject2DClean);
    GameObjectSetRecreateFunc((GameObject *)go, (void *)GameObject2DRecreate);
    GameObjectSetDestroyFunc((GameObject *)go, (void *)GameObject2DDestroy);

    go->self.obj_type = ENGINE_GAME_OBJECT_TYPE_2D;

    Transform2DInit(&go->transform);
    GraphicsObjectInit(&go->graphObj, ENGINE_VERTEX_TYPE_2D_OBJECT);

    go->self.vert = AllocateMemory(1, sizeof(ShaderBuilder));
    go->self.frag = AllocateMemory(1, sizeof(ShaderBuilder));

    go->self.init = true;
}

vec2 GameObject2DGetSize(GameObject2D* go)
{
    /*vec2 size;
    size.x = fabs((go->graphObj.shape.vParam.vertices[1].position.x - go->graphObj.shape.vParam.vertices[3].position.x) * (viewSize.x * diffSize.x)) * go->transform.scale.x;
    size.y = fabs((go->graphObj.shape.vParam.vertices[1].position.y - go->graphObj.shape.vParam.vertices[3].position.y) * (viewSize.y * diffSize.y)) * go->transform.scale.y;
    return size;*/
}
