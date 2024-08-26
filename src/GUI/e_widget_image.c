#include <GUI/e_widget_image.h>

#include <vulkan/vulkan.h>

#include "Data/e_resource_data.h"
#include "Data/e_resource_engine.h"
#include "Data/e_resource_export.h"

#include "Tools/e_math.h"

extern ZEngine engine;

void ImageWidgetUpdate(EWidgetImage* img, BluePrintDescriptor *descriptor) {

}

void ImageWidgetCreateQuad(EWidgetImage *wi)
{

}


void ImageWidgetInit(EWidgetImage *img, char *image_path, EWidget *parent){

    img->widget.type = ENGINE_WIDGET_TYPE_IMAGE;


    img->widget.color = (vec3){0.4, 0.1, 0.1};

    img->widget.offset.x = 0;
    img->widget.offset.y = 0;
    img->widget.transparent = 1.0f;

    WidgetSetParent(&img->widget, parent);

    img->widget.widget_flags = ENGINE_FLAG_WIDGET_VISIBLE;

    img->widget.callbacks.stack = (CallbackStruct *) AllocateMemory(MAX_GUI_CALLBACKS, sizeof(CallbackStruct));
    img->widget.callbacks.size = 0;

}


void ImageWidgetInitDefault(EWidgetImage *img, char *image_path, DrawParam *dParam, EWidget *parent)
{
    ImageWidgetInit(img, image_path, parent);
}
