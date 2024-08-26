#include <ZamEngine.h>

#include <Core/engine.h>
#include <Core/e_camera.h>

#include <GUI/GUIManager.h>
#include <GUI/e_widget.h>

#include <Tools/e_math.h>

#define WIDGET_COUNT 10

Camera2D cam2D;
Camera3D cam3D;

EWidget widget[WIDGET_COUNT];

void MousePress(EWidget *widget, void* entry, void *arg){
    printf("Mouse pressed!\n");
    widget->color = vec3_f(0.6, 0.2, 0.2);
}

void MouseRelease(EWidget *widget, void* entry, void *arg){
    printf("Mouse released!\n");
    widget->color = vec3_f(0.6, 0.1, 0.1);
}

int main(){

    ZEngineInitSystem(800, 600, "Test");

    Camera2DInit(&cam2D);
    Camera3DInit(&cam3D);

    Camera2DSetActive(&cam2D);
    Camera3DSetActive(&cam3D);


    for(int i = 0;i < WIDGET_COUNT;i++){
        WidgetInit(&widget[i], NULL);
        WidgetSetPosition(&widget[i], i * 20, i * 20);
        WidgetSetScale(&widget[i], 100, 100);
        WidgetConnect(&widget[i], ENGINE_WIDGET_TRIGGER_MOUSE_PRESS, MousePress, NULL);
        WidgetConnect(&widget[i], ENGINE_WIDGET_TRIGGER_MOUSE_RELEASE, MouseRelease, NULL);
    }
    
    widget[0].widget_flags &= ~(ENGINE_FLAG_WIDGET_VISIBLE);

    float rot = 0;
        
    uint32_t curr = 0;
    while (!EngineWindowIsClosed())
    {
        ZEnginePoolEvents();
        
        for(int i = 0;i < WIDGET_COUNT;i++)
            ZEngineDraw(&widget[i]);

        rot +=0.001f;

        ZEngineRender();
    }
    
    for(int i = 0;i < WIDGET_COUNT;i++)
        GameObjectDestroy(&widget[i]);
    
    EngineDeviceWaitIdle();
    
    ZEngineCleanUp();

    return 0;
}