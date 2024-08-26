#include "GUI/e_widget_window.h"

#include <vulkan/vulkan.h>

#include "Tools/e_math.h"

#include "Core/engine.h"

#include "Data/e_resource_data.h"
#include "Data/e_resource_engine.h"
#include "Data/e_resource_shapes.h"
#include "Data/e_resource_export.h"

extern ZEngine engine;

vec2 e_var_mouse, e_var_temp, e_var_tscale ;

uint32_t button_wind_offset = 15;

int WindowWidgetSetSize(EWidgetWindow* window, float x, float y)
{

    /*Transform2DSetScale(&window->window, x, y);

    vec2 position = {};
    Transform2DSetPosition(&window->close, (x - 20) * 2 , button_wind_offset);
    Transform2DSetPosition(&window->resize, (x - 40) * 2 , button_wind_offset);
    Transform2DSetPosition(&window->hide, (x - 60) * 2 , button_wind_offset);

    vec2 botSize = window->window.go.transform.scaleOrig;
    vec2 size = window->window.go.transform.scaleOrig;
    botSize.y -= 40;
    botSize.x -= 10;

    Transform2DSetScale(&window->surface, botSize.x, botSize.y);
    Transform2DSetPosition(&window->surface, 10, 60);*/

    return 0;
}

int WindowWidgetPress(EWidget* widget, void* entry, void* args)
{

    double xpos, ypos;

    EngineGetCursorPos(&xpos, &ypos);
    e_var_mouse.x = xpos;
    e_var_mouse.y = ypos;

    e_var_temp = Transform2DGetPosition(widget);
    e_var_tscale = Transform2DGetScale(widget);

    return 0;
}

int WindowWidgetMove(EWidget* widget, void* entry, void* args)
{

    EWidgetWindow *window = (EWidgetWindow *)args;

    vec2 te;
    double xpos, ypos;

    EngineGetCursorPos(&xpos, &ypos);
    te.x = xpos;
    te.y = ypos;


    if(e_var_mouse.y > e_var_temp.y + 10 && e_var_mouse.y < e_var_temp.y && window->resizeble)
    {
        te = v2_sub(te, e_var_mouse);
        vec2 scale = v2_add(e_var_tscale, te);

        scale.x = scale.x <= 0 ? 100 : scale.x;
        scale.y = scale.y <= 0 ? 20 : scale.y;
        WindowWidgetSetSize(window, scale.x, scale.y);

        window->wasHide = false;
        window->wasResize = false;
    }
    else
    {
        te = v2_muls(v2_sub(te, e_var_mouse), 2.0);
        te = v2_add(e_var_temp, te);
        Transform2DSetPosition(widget, te.x, te.y);
    }

    return 0;
}

int WindowWidgetCloseButton(EWidget* widget, void* entry, void *arg){

    EWidgetWindow *window = (EWidgetWindow *)arg;

    window->show = false;

    WidgetConfirmTrigger(window, ENGINE_WIDGET_TRIGGER_WINDOW_CLOSE, NULL);

    return 0;
}

int WindowWidgetResizeButton(EWidget* widget, void* entry, void *arg){

    EWidgetWindow *window = (EWidgetWindow *)arg;

    if(!window->resizeble)
        return;

    if(!window->wasResize && !window->wasHide)
    {
        window->lastPos = Transform2DGetPosition(&window->window);
        window->lastSize = Transform2DGetScale(&window->window);
    }else if(window->wasResize){
        Transform2DSetPosition(&window->window, window->lastPos.x, window->lastPos.y);
        WindowWidgetSetSize(window, window->lastSize.x, window->lastSize.y);
        window->wasHide = false;
        window->wasResize = false;
        return;
    }

    window->wasResize = true;

    vec2 size = {engine.width * 2, engine.height * 2};

    WindowWidgetSetSize(window, size.x, size.y);
    Transform2DSetPosition(&window->window, 0, 0);


    return 0;
}

int WindowWidgetHideButton(EWidget* widget, void* entry, void *arg){

    EWidgetWindow *window = (EWidgetWindow *)arg;

    if(!window->wasHide && !window->wasResize)
    {
        window->lastPos = Transform2DGetPosition(&window->window);
        window->lastSize = Transform2DGetScale(&window->window);
    }else if(window->wasHide){
        Transform2DSetPosition(&window->window, window->lastPos.x, window->lastPos.y);
        WindowWidgetSetSize(window, window->lastSize.x, window->lastSize.y);
        window->wasHide = false;
        window->wasResize = false;
        return;
    }

    window->wasHide = true;

    WindowWidgetSetSize(window, 200, 40);
    Transform2DSetPosition(&widget->parent->go, 60, (engine.height * 2) - 60);

    return 0;
}

void WindowWidgetDraw(EWidgetWindow *window){

    if(!window->show)
        return; 

    WidgetUpdateScissor(&window->surface);
    WidgetUpdateScissorFromParent(&window->window, &window->name);
    
    GameObject2DDefaultUpdate(&window->window);
    GameObject2DDefaultDraw(&window->window);
    GameObjectDraw(&window->surface);
    GameObjectDraw(&window->name);
    
    GameObjectDraw(&window->close);
    GameObjectDraw(&window->resize);
    GameObjectDraw(&window->hide);
}

void WindowWidgetDestroy(EWidgetWindow *window){
    GameObjectDestroy(&window->name);

    ChildStack *child = window->window.child;
    ChildStack *lastChild;

    while(child != NULL)
    {
        GameObjectDestroy(child->node);
        lastChild = child;
        child = child->next;
        FreeMemory(lastChild);
    }

    FreeMemory(window->window.callbacks.stack);
}


void WindowWidgetInitWindow(EWidget* widget, DrawParam *dParam, vec2 size, vec2 position){

    WidgetInit(widget, NULL);
    GameObjectSetDrawFunc((GameObject *)widget, (void *)WindowWidgetDraw);
    GameObjectSetDestroyFunc((GameObject *)widget, (void *)WindowWidgetDestroy);
    WidgetAddDefault(widget, dParam->render);
    GameObject2DInitDraw(widget);

    Transform2DSetScale(widget, size.x, size.y);
    Transform2DSetPosition(widget, position.x, position.y);
    
    widget->type = ENGINE_WIDGET_TYPE_WINDOW;

    WidgetSetColor(widget, vec3_f(1, 1, 1));
    widget->color = (vec3){1, 1, 1};
    widget->transparent = 1.0f;
}

void InitName(TextObject *to, char* name, DrawParam *dParam, EWidget *parent)
{
    TextObjectInitDefault(to, 9, NULL, dParam);
    TextObjectSetText(to, name);
    Transform2DSetPosition(to, 20, 40);
}

void InitSurface(EWidget* widget, DrawParam *dParam, vec2 size, EWidget *parent){

    WidgetInit(widget, parent);
    WidgetAddDefault(widget, dParam->render);
    GameObject2DInitDraw(widget);

    vec2 botSize = size;
    botSize.y -= 40;
    botSize.x -= 10;

    WidgetSetColor(widget, vec3_f(0.5f, 0.5f, 0.5f));
    Transform2DSetScale(widget, botSize.x, botSize.y);
    Transform2DSetPosition(widget, 10, 60);
}

void InitClose(EWidget* widget, DrawParam *dParam, vec2 size, EWidget *parent){

    ButtonWidgetInit(widget, NULL, dParam, parent);

    ButtonWidgetSetColor(widget, 1.0f, 0.0f, 0.0);

    Transform2DSetScale(widget, 10, 10);
    Transform2DSetPosition(widget, (size.x - 20) * 2 , button_wind_offset);
}

void InitResize(EWidget* widget, DrawParam *dParam, vec2 size, EWidget *parent){
    
    ButtonWidgetInit(widget, NULL, dParam, parent);
    
    ButtonWidgetSetColor(widget, 0.0f, 1.0f, 0.0f);

    Transform2DSetScale(widget, 10, 10);
    Transform2DSetPosition(widget, (size.x - 40) * 2, button_wind_offset);
}

void InitHide(EWidget* widget, DrawParam *dParam, vec2 size, EWidget *parent){

    ButtonWidgetInit(widget, NULL, dParam, parent);

    ButtonWidgetSetColor(widget, 0.0f, 0.0f, 1.0f);

    Transform2DSetScale(widget, 10, 10);
    Transform2DSetPosition(widget, (size.x - 60) * 2, button_wind_offset);
}

void WindowWidgetInit(EWidgetWindow *ww, char* name, vec2 size, DrawParam *dParam, vec2 position)
{
    WindowWidgetInitWindow(&ww->window, dParam, size, position);
    InitSurface(&ww->surface, dParam, size, &ww->window);
    InitName(&ww->name, name, dParam, &ww->window);

    InitClose(&ww->close, dParam, size, &ww->window);
    InitResize(&ww->resize, dParam, size, &ww->window);
    InitHide(&ww->hide, dParam, size, &ww->window);

    WidgetConnect(&ww->window, ENGINE_WIDGET_TRIGGER_MOUSE_PRESS, WindowWidgetPress, NULL);
    WidgetConnect(&ww->window, ENGINE_WIDGET_TRIGGER_MOUSE_MOVE, WindowWidgetMove, ww);

    WidgetConnect(&ww->close, ENGINE_WIDGET_TRIGGER_MOUSE_PRESS, WindowWidgetCloseButton, ww);
    WidgetConnect(&ww->resize, ENGINE_WIDGET_TRIGGER_MOUSE_PRESS, WindowWidgetResizeButton, ww);
    WidgetConnect(&ww->hide, ENGINE_WIDGET_TRIGGER_MOUSE_PRESS, WindowWidgetHideButton, ww);

    ww->window.type = ENGINE_WIDGET_TYPE_WINDOW;
    ww->show = true;
    ww->wasHide = false;
    ww->wasResize = false;
    ww->resizeble = true;
}

void WindowWidgetShow(EWidgetWindow *ww){
    ww->show = true;

    WidgetConfirmTrigger(ww, ENGINE_WIDGET_TRIGGER_WINDOW_OPEN, NULL);
}

void WindowWidgetHide(EWidgetWindow *ww){
    ww->show = false;

    WidgetConfirmTrigger(ww, ENGINE_WIDGET_TRIGGER_WINDOW_CLOSE, NULL);
}

void WindowWidgetUpdate(EWidgetWindow *ww){

    if(!ww->show)
        return;

    WidgetEventsPipe(&ww->window);
}

EWidget *WindowWidgetGetSurface(EWidgetWindow *ww){
    return &ww->surface;
}