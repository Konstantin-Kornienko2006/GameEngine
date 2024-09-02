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

    uint32_t num_pack = BluePrintInit(&img->image.graphObj.blueprints);
    
    ShaderBuilder *vert = img->image.self.vert;
    ShaderBuilder *frag = img->image.self.frag;

    ShadersMakeDefault2DShader(vert, frag, img->image.num_images > 0);

    GraphicsObjectSetSomeShader(&img->image.graphObj, vert->code, vert->size, num_pack);
    GraphicsObjectSetSomeShader(&img->image.graphObj, frag->code, frag->size, num_pack);

    BluePrintAddSomeUpdater(&img->image.graphObj.blueprints, num_pack, 0, GameObject2DTransformBufferUpdate);
    BluePrintAddSomeUpdater(&img->image.graphObj.blueprints, num_pack, 1, GameObject2DImageBuffer);
    BluePrintSetTextureImageCreate(&img->image.graphObj.blueprints, num_pack, img->image.image, 0);

    uint32_t flags = BluePrintGetSettingsValue(&img->image.graphObj.blueprints, num_pack, 3);
    BluePrintSetSettingsValue(&img->image.graphObj.blueprints, num_pack, 3, flags | ENGINE_PIPELINE_FLAG_FACE_CLOCKWISE);

    //----------------------------------------------------

    GameObject2DInitDraw(&img->image);

}