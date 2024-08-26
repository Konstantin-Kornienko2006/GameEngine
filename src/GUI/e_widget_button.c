#include "GUI/e_widget_button.h"

#include "Tools/e_math.h"

#include "ZamEngine.h"

extern ZEngine engine;

int ButtonWidgetPress(EWidget *widget, void* entry, void *arg){
    EWidgetButton *button = (EWidgetButton *)widget;

    button->widget.color.x = button->selfColor.x - 0.2f;
    button->widget.color.y = button->selfColor.y - 0.2f;
    button->widget.color.z = button->selfColor.z - 0.2f;
    
    WidgetSetColor(&button->widget, v3_subs(button->selfColor, 0.3f));

    return 0;
}

int ButtonWidgetRelease(EWidget *widget, void* entry, void *arg){

    EWidgetButton *button = (EWidgetButton *)widget;
    
    WidgetSetColor(&button->widget, button->selfColor);

    WidgetConfirmTrigger(widget, ENGINE_WIDGET_TRIGGER_BUTTON_PRESS, NULL);

    return 0;
}

void ButtonWidgetDraw(EWidgetButton *button){
            
            
}


void ButtonWidgetDestroy(EWidgetButton *button){
    GameObjectDestroy(&button->to);
    GameObject2DDestroy(&button->widget);
        
    FreeMemory(button->widget.callbacks.stack);
}

void ButtonWidgetInit(EWidgetButton *button, const char *text, DrawParam *dParam, EWidget *parent){

    WidgetInit(button, parent);

    GameObjectSetDrawFunc((GameObject *)button, (void *)ButtonWidgetDraw);
    GameObjectSetDestroyFunc((GameObject *)button, (void *)ButtonWidgetDestroy);

    button->widget.type = ENGINE_WIDGET_TYPE_BUTTON;

    button->selfColor = (vec3){ 1, 1, 1};

    WidgetSetColor(&button->widget, button->selfColor);

    TextObjectInitDefault(&button->to, 9, NULL, dParam);

    if(text != NULL)
        TextObjectSetText(&button->to, text);

    Transform2DSetPosition(&button->to, 0, 0);

    WidgetConnect(&button->widget, ENGINE_WIDGET_TRIGGER_MOUSE_PRESS, ButtonWidgetPress, NULL);
    WidgetConnect(&button->widget, ENGINE_WIDGET_TRIGGER_MOUSE_RELEASE, ButtonWidgetRelease, NULL);

}

void ButtonWidgetSetImage(EWidgetButton *button, char *path, DrawParam *dParam)
{
    ImageWidgetInitDefault(&button->image, path, dParam, button);

    Transform2DSetScale(&button->image, 64, 64);
    Transform2DSetPosition(&button->image, 0, 0);

    Transform2DSetPosition(&button->to, 9 * 6.0f, 9 * 4.0f);
}

void ButtonWidgetSetText(EWidgetButton *button, const char *text){
    TextObjectSetText(&button->to, text);
}

void ButtonWidgetSetColor(EWidgetButton *button, float r, float g, float b){
    button->selfColor.x = button->widget.color.x = r;
    button->selfColor.y = button->widget.color.y = g;
    button->selfColor.z = button->widget.color.z = b;
    
    WidgetSetColor(&button->widget, button->selfColor);
}
