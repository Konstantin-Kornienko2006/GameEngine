#include "Objects/particleSystem3D.h"

#include "ZamEngine.h"

#include <vulkan/vulkan.h>

#include "Tools/e_tools.h"

#include "Core/e_memory.h"
#include "Core/e_camera.h"
#include "Core/e_device.h"
#include "Core/pipeline.h"
#include "Core/e_buffer.h"
#include "Core/e_texture.h"
#include "Core/e_transform.h"
#include "Core/graphicsObject.h"

#include "Tools/e_math.h"

#include "Data/e_resource_data.h"
#include "Data/e_resource_engine.h"
#include "Data/e_resource_export.h"

extern ZEngine engine;

void Particle3DFind(ParticleObject3D *particle){

    for(int i=0; i < particle->num_parts; i++)
    {
        if(particle->particles[i].life <= 0){

            for(int j=i; j < particle->num_parts; j++)
            {
                if(j != particle->num_parts - 1)
                    particle->particles[j] = particle->particles[j + 1];
            }

            particle->num_parts--;

            particle->particles = (Particle3D *) realloc(particle->particles, particle->num_parts * sizeof(Particle3D));

            i--;
        }
    }

}

void Particle3DDefaultUpdate(ParticleObject3D* particle, BluePrintDescriptor *descriptor){

    if(particle->num_parts == 0)
        return;

    for(int i=0; i < particle->num_parts; i++)
    {
        particle->particles[i].life -= 0.01;
        particle->particles[i].direction.y -= 0.1f * particle->particles[i].gravity * 10;
        particle->particles[i].position = v3_add(particle->particles[i].position, v3_muls(v3_divs(particle->particles[i].direction, 10), particle->particles[i].speed)) ;
        particle->particles[i].scale -= 0.001f * particle->particles[i].speed;
    }

    Particle3DFind(particle);

    if(particle->num_parts == 0)
        return;

    vertexParam *vParam = &particle->go.graphObj.shapes[0].vParam;

    if(vParam->vertices != NULL)
        FreeMemory(vParam->vertices);

    vParam->vertices = AllocateMemory(particle->num_parts, sizeof(ParticleVertex3D));

    ParticleVertex3D *verts = vParam->vertices;

    for(int i=0; i < particle->num_parts; i++)
    {
        verts[i].color = particle->particles[i].color;
        verts[i].position = v3_divs(particle->particles[i].position, 10);
        verts[i].size = particle->particles[i].scale;
    }

    BuffersUpdateVertex((struct VertexParam_T *)vParam);


    GameObject3DDescriptorModelUpdate((GameObject3D *)particle, descriptor);

}

void Particle3DDefaultDraw(GameObject3D* go){
    ZDevice *device = (ZDevice *)engine.device;

    if(go->graphObj.shapes[0].vParam.verticesSize == 0)
        return;

    for(int i=0; i < go->graphObj.gItems.num_shader_packs;i++)
    {
        BluePrintPack *pack = &go->graphObj.blueprints.blue_print_packs[i];

        if(pack->render_point == engine.current_render)
        {
            ShaderPack *pack = &go->graphObj.gItems.shader_packs[i];

            vkCmdBindPipeline(device->commandBuffers[engine.imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, pack->pipeline.pipeline);

            PipelineSetting *settings = &go->graphObj.blueprints.blue_print_packs[i].setting;

            vkCmdSetViewport(device->commandBuffers[engine.imageIndex], 0, 1, (const VkViewport *)&settings->viewport);
            vkCmdSetScissor(device->commandBuffers[engine.imageIndex], 0, 1, (const VkRect2D *)&settings->scissor);

            VkBuffer vertexBuffers[] = {go->graphObj.shapes[settings->vert_indx].vParam.buffer.buffer};
            VkDeviceSize offsets[] = {0};

            vkCmdBindVertexBuffers(device->commandBuffers[engine.imageIndex], 0, 1, vertexBuffers, offsets);

            vkCmdBindDescriptorSets(device->commandBuffers[engine.imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, pack->pipeline.layout, 0, 1, &pack->descriptor.descr_sets[engine.imageIndex], 0, NULL);

            if(settings->flags & ENGINE_PIPELINE_FLAG_DRAW_INDEXED){
                vkCmdBindIndexBuffer(device->commandBuffers[engine.imageIndex], go->graphObj.shapes[settings->vert_indx].iParam.buffer.buffer, 0, VK_INDEX_TYPE_UINT32);
                vkCmdDrawIndexed(device->commandBuffers[engine.imageIndex], go->graphObj.shapes[settings->vert_indx].iParam.indexesSize, 1, 0, 0, 0);
            }else
                vkCmdDraw(device->commandBuffers[engine.imageIndex], go->graphObj.shapes[settings->vert_indx].vParam.verticesSize, 1, 0, 0);
        }
    }
}

void Particle3DInit(ParticleObject3D* particle, DrawParam dParam){

    GameObjectSetUpdateFunc((GameObject *)particle, (void *)GameObject3DDefaultUpdate);
    GameObjectSetDrawFunc((GameObject *)particle, (void *)Particle3DDefaultDraw);
    GameObjectSetCleanFunc((GameObject *)particle, (void *)GameObject3DClean);
    GameObjectSetRecreateFunc((GameObject *)particle, (void *)GameObject3DRecreate);
    GameObjectSetDestroyFunc((GameObject *)particle, (void *)GameObject3DDestroy);

    Transform3DInit(&particle->go.transform);
    GraphicsObjectInit(&particle->go.graphObj, ENGINE_VERTEX_TYPE_3D_PARTICLE);

    GraphicsObjectSetVertex(&particle->go.graphObj, NULL, 0, sizeof(ParticleVertex3D), NULL, 0, sizeof(uint32_t));

    particle->go.graphObj.gItems.perspective = true;
    particle->go.self.flags = 0;

    particle->go.graphObj.shapes[0].vParam.vertices = AllocateMemory(particle->num_parts, sizeof(ParticleVertex3D));
    particle->go.graphObj.num_shapes = 1;
    particle->particles = (Particle3D*) AllocateMemory(particle->num_parts, sizeof(Particle3D));

    particle->go.images = AllocateMemory(1, sizeof(GameObjectImage));

    if(strlen(dParam.diffuse) != 0)
    {
        int len = strlen(dParam.diffuse);
        particle->go.images->path = AllocateMemory(len + 1, sizeof(char));
        memcpy(particle->go.images->path, dParam.diffuse, len);
        particle->go.images->path[len] = '\0';
        //go->image->buffer = ToolsLoadImageFromFile(&go->image->size, dParam.filePath);
    }

    particle->num_parts = 0;
}

void Particle3DAddDefault(ParticleObject3D* particle, void *render)
{
    /*uint32_t num_pack = BluePrintInit(&particle->go.graphObj.blueprints);

    GraphicsObjectSetSomeShader(&particle->go.graphObj, &_binary_shaders_particle_vert3D_spv_start, (size_t)(&_binary_shaders_particle_vert3D_spv_size), num_pack);
    GraphicsObjectSetSomeShader(&particle->go.graphObj, &_binary_shaders_particle_frag3D_spv_start, (size_t)(&_binary_shaders_particle_frag3D_spv_size), num_pack);

    BluePrintAddSomeUpdater(&particle->go.graphObj.blueprints, num_pack, 0, GameObject3DDescriptorModelUpdate);
    BluePrintSetTextureImageCreate(&particle->go.graphObj.blueprints, num_pack, &particle->go.images[0], 0);*/
    
    /*setting.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
    setting.flags &= ~(ENGINE_PIPELINE_FLAG_DRAW_INDEXED);*/
}

void Particle3DAdd(ParticleObject3D* particle, vec3 position, vec3 direction, float speed, float gravity, float life){

    double time = ZEngineGetTime();

    srand((uint32_t)(time * 1000));

    particle->num_parts ++;

    particle->particles = (Particle3D*) realloc(particle->particles, particle->num_parts * sizeof(Particle3D));

    Particle3D part;
    part.position = position;
    part.direction = direction;
    part.life = life;
    part.speed = speed;
    part.gravity = gravity;

    float min_size = 0.3f;
    float max_size = 1.0f;

    part.scale = min_size + (((float)rand() / (float)RAND_MAX) * (max_size - min_size));

    part.color.x = min_size + (((float)rand() / (float)RAND_MAX) * (max_size - min_size));
    part.color.y = min_size + (((float)rand() / (float)RAND_MAX) * (max_size - min_size));
    part.color.z = min_size + (((float)rand() / (float)RAND_MAX) * (max_size - min_size));


    particle->particles[particle->num_parts - 1] = part;

}
