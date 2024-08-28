#include <GUI/e_widget_image.h>

#include "Objects/gameObject2D.h"

#include <vulkan/vulkan.h>

#include "Data/e_resource_data.h"
#include "Data/e_resource_engine.h"
#include "Data/e_resource_export.h"

#include "Tools/e_math.h"
#include "Tools/e_shaders.h"

extern ZEngine engine;


void ImageWidgetCreateQuad(GameObject2D *go)
{
    Vertex2D *verts = AllocateMemory(4, sizeof(Vertex2D));

    verts[0].position.x = -1;
    verts[0].position.y = -1;
    verts[0].texCoord.x = 0;
    verts[0].texCoord.y = 0;

    verts[1].position.x = 1;
    verts[1].position.y = -1;
    verts[1].texCoord.x = 1.0f;
    verts[1].texCoord.y = 0;

    verts[2].position.x = 1;
    verts[2].position.y = 1;
    verts[2].texCoord.x = 1.0f;
    verts[2].texCoord.y = 1.0f;

    verts[3].position.x = -1;
    verts[3].position.y = 1;
    verts[3].texCoord.x = 0;
    verts[3].texCoord.y = 1.0f;

    for(int i=0;i < 4;i++)
    {
        verts[i].color = vec3_f(1, 1, 1);
    }

    uint32_t *tIndx = AllocateMemory(6, sizeof(uint32_t));

    uint32_t indx[] = {
      0, 1, 2, 2, 3, 0
    };

    memcpy(tIndx, indx, 6 * sizeof(uint32_t));

    GraphicsObjectSetVertex(&go->graphObj, verts, 4, sizeof(Vertex2D), tIndx, 6, sizeof(uint32_t));

    FreeMemory(verts);
    FreeMemory(tIndx);
}


void ImageWidgetDraw(EWidgetImage *img){

    Transform2DSetScale(&img->image, img->widget.scale.x, img->widget.scale.y);
    Transform2DSetPosition(&img->image, img->widget.position.x + img->widget.base.x, img->widget.position.y + img->widget.base.y);

    if(img->widget.widget_flags & ENGINE_FLAG_WIDGET_VISIBLE){
        GameObjectDraw(&img->image);
    }
}

extern void WidgetDestroy(EWidget *widget);

void ImageWidgetDestroy(EWidgetImage *img){

    if(!img->widget.go.init)
        return;

    WidgetDestroy(img);

    GameObjectDestroy(&img->image);

    img->widget.go.init = false;
}


void ImageWidgetInit(EWidgetImage *img, char *image_path, EWidget *parent){

    if(strlen(image_path) == 0)
        return;

    WidgetInit(img, parent);

    GameObjectSetDrawFunc(img, ImageWidgetDraw);
    GameObjectSetDestroyFunc(img, ImageWidgetDestroy);

    img->widget.type = ENGINE_WIDGET_TYPE_IMAGE;
    ///--------------------------------------------------

    GameObject2DInit(&img->image);

    ImageWidgetCreateQuad(&img->image);

    img->image.image = AllocateMemory(1, sizeof(GameObjectImage));

    int len = strlen(image_path);
    img->image.image->path = AllocateMemory(len + 1, sizeof(char));
    memcpy(img->image.image->path, image_path, len);
    img->image.image->path[len] = '\0';;
    img->image.num_images ++;

    BluePrintAddUniformObject(&img->image.graphObj.blueprints, 0, sizeof(TransformBuffer2D), VK_SHADER_STAGE_VERTEX_BIT, (void *)GameObject2DTransformBufferUpdate, 0);

    BluePrintAddTextureImage(&img->image.graphObj.blueprints, 0, img->image.image, VK_SHADER_STAGE_FRAGMENT_BIT);

    PipelineSetting setting;

    PipelineSettingSetDefault(&img->image.graphObj, &setting);
    
    ShaderBuilder *vert = img->image.self.vert;
    ShaderBuilder *frag = img->image.self.frag;

    ShadersMakeDefault2DShader(vert, frag, true);

    PipelineSettingSetShader(&setting, (char *)vert->code, vert->size * sizeof(uint32_t), VK_SHADER_STAGE_VERTEX_BIT);
    PipelineSettingSetShader(&setting, (char *)frag->code, frag->size * sizeof(uint32_t), VK_SHADER_STAGE_FRAGMENT_BIT);

    setting.fromFile = 0;
    setting.flags |= ENGINE_PIPELINE_FLAG_FACE_CLOCKWISE;

    GameObject2DAddSettingPipeline((GameObject2D *)&img->image, 0, &setting);

    img->image.graphObj.blueprints.num_blue_print_packs ++;

    GameObject2DInitDraw(&img->image);

}