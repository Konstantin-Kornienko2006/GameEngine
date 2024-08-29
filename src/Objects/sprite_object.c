#include "Objects/sprite_object.h"

#include <vulkan/vulkan.h>

#include "Variabels/engine_includes.h"

#include "Core/pipeline.h"
#include "Core/e_buffer.h"
#include "Core/e_blue_print.h"

#include "Tools/e_shaders.h"

#include "Data/e_resource_data.h"
#include "Data/e_resource_engine.h"
#include "Data/e_resource_export.h"

void SpriteObjectCreateQuad(SpriteObject *so)
{
    Vertex2D *verts = AllocateMemory(4, sizeof(Vertex2D));

    float size = 1.0f;

    verts[0].position.x = -size;
    verts[0].position.y = -size;
    verts[0].texCoord.x = 0;
    verts[0].texCoord.y = 0;

    verts[1].position.x = size;
    verts[1].position.y = -size;
    verts[1].texCoord.x = 1.0f;
    verts[1].texCoord.y = 0;

    verts[2].position.x = size;
    verts[2].position.y = size;
    verts[2].texCoord.x = 1.0f;
    verts[2].texCoord.y = 1.0f;

    verts[3].position.x = -size;
    verts[3].position.y = size;
    verts[3].texCoord.x = 0;
    verts[3].texCoord.y = 1.0f;

    for(int i=0;i < 4;i++)
    {
        verts[i].color = (vec3){ 1, 1, 1};
    }

    uint32_t *tIndx = AllocateMemory(6, sizeof(uint32_t));

    uint32_t indx[] = {
      0, 1, 2, 2, 3, 0
    };

    memcpy(tIndx, indx, 6 * sizeof(uint32_t));

    GraphicsObjectSetVertex(&so->go.graphObj, verts, 4, sizeof(Vertex2D), tIndx, 6, sizeof(uint32_t));

    FreeMemory(verts);
}

int SpriteObjectInit(SpriteObject *so, DrawParam *dParam){

    GameObject2DInit((GameObject2D *)so);

    SpriteObjectCreateQuad(so);

    so->go.image = AllocateMemory(1, sizeof(GameObjectImage));

    char *currPath = DirectGetCurrectFilePath();
    int len = strlen(currPath);
    currPath[len] = '\\';

    if(strlen(dParam->diffuse) != 0)
    {
        char *full_path = ToolsMakeString(currPath, dParam->normal);
        
        if(!DirectIsFileExist(full_path)){
            GameObjectDestroy(so);
            FreeMemory(full_path);            
            FreeMemory(currPath);
            return 0;
        }

        int len = strlen(full_path);
        so->go.image->path = AllocateMemory(len + 1, sizeof(char));
        memcpy(so->go.image->path, full_path, len);
        so->go.image->path[len] = '\0';
        //go->image->buffer = ToolsLoadImageFromFile(&go->image->size, dParam.filePath);

        so->go.num_images ++;

        FreeMemory(full_path);   
    }

    FreeMemory(currPath);

    so->go.graphObj.num_shapes = 1;
    
    return 1;
}

void SpriteObjectAddDefault(SpriteObject *so, void *render)
{

    uint32_t nums = so->go.graphObj.blueprints.num_blue_print_packs;
    so->go.graphObj.blueprints.blue_print_packs[nums].render_point = render;

    BluePrintAddUniformObject(&so->go.graphObj.blueprints, 0, sizeof(TransformBuffer2D), VK_SHADER_STAGE_VERTEX_BIT, (void *)GameObject2DTransformBufferUpdate, 0);
    BluePrintAddUniformObject(&so->go.graphObj.blueprints, 0, sizeof(ImageBufferObjects), VK_SHADER_STAGE_FRAGMENT_BIT, (void *)GameObject2DImageBuffer, 0);

    BluePrintAddTextureImage(&so->go.graphObj.blueprints, 0, so->go.image, VK_SHADER_STAGE_FRAGMENT_BIT);

    PipelineSetting setting;

    PipelineSettingSetDefault(&so->go.graphObj, &setting);

    ShaderBuilder *vert = so->go.self.vert;
    ShaderBuilder *frag = so->go.self.frag;

    ShadersMakeDefault2DShader(vert, frag, so->go.num_images > 0);

    PipelineSettingSetShader(&setting, (char *)vert->code, vert->size * sizeof(uint32_t), VK_SHADER_STAGE_VERTEX_BIT);
    PipelineSettingSetShader(&setting, (char *)frag->code, frag->size * sizeof(uint32_t), VK_SHADER_STAGE_FRAGMENT_BIT);

    setting.fromFile = 0;
    setting.flags |= ENGINE_PIPELINE_FLAG_FACE_CLOCKWISE;

    GameObject2DAddSettingPipeline((GameObject2D *)so, nums, &setting);

    so->go.graphObj.blueprints.num_blue_print_packs ++;
}

void SpriteObjectInitDefault(SpriteObject *so, DrawParam *dParam)
{
    int res = SpriteObjectInit(so, dParam);

    if(!res)
        return;

    SpriteObjectAddDefault(so, dParam->render);
    GameObject2DInitDraw((GameObject2D *)so);
}

void SpriteObjectSetOffsetRect(SpriteObject *so, float x, float y, float width, float height)
{
    Vertex2D *verts = so->go.graphObj.shapes[0].vParam.vertices;

    float temp_x = x / so->go.image->imgWidth;
    float temp_y = y / so->go.image->imgHeight;

    verts[0].texCoord.x = temp_x;
    verts[0].texCoord.y = temp_y;

    verts[1].texCoord.x = temp_x + width / so->go.image->imgWidth;
    verts[1].texCoord.y = temp_y;

    verts[2].texCoord.x = temp_x + width / so->go.image->imgWidth;
    verts[2].texCoord.y = temp_y + height / so->go.image->imgHeight;

    verts[3].texCoord.x = temp_x;
    verts[3].texCoord.y = temp_y + height / so->go.image->imgHeight;

    BuffersUpdateVertex(&so->go.graphObj.shapes[0].vParam);

}
