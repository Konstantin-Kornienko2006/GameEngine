#include "Objects/gameObject.h"

#include "Core/engine.h"
#include "Core/e_device.h"

extern ZEngine engine;

void GameObjectSetInitFunc(GameObject *go, void *func){
    go->InitPoint = func;
}

void GameObjectSetUpdateFunc(GameObject *go, void *func){
    go->UpdatePoint = func;
}

void GameObjectSetDrawFunc(GameObject *go, void *func){
    go->DrawPoint = func;
}

void GameObjectSetCleanFunc(GameObject *go, void *func){
    go->CleanPoint = func;
}

void GameObjectSetRecreateFunc(GameObject *go, void *func){
    go->RecreatePoint = func;
}

void GameObjectSetDestroyFunc(GameObject *go, void *func){
    go->DestroyPoint = func;
}


void GameObjectInit(GameObject* go){

    if(go == NULL)
        return;

    go->InitPoint(go);
}

void GameObjectUpdate(GameObject* go) {

    if(go == NULL)
        return;

    if(go->UpdatePoint == NULL)
        return;

    go->UpdatePoint(go);
}

void GameObjectDraw(GameObject* go) {

    ZDevice *device = (ZDevice *)engine.device;

    if(go == NULL)
        return;

    GameObjectUpdate(go);

    go->DrawPoint(go, device->commandBuffers[engine.imageIndex]);
}

void GameObjectClean(GameObject* go){

    if(go == NULL)
        return;

    go->CleanPoint(go);
}

void GameObjectRecreate(GameObject* go){

    if(go == NULL)
        return;

    go->RecreatePoint(go);
}

void GameObjectDestroy(GameObject* go){

    EngineDeviceWaitIdle();

    if(go == NULL)
        return;

    void (*destroy)(GameObject* go) = go->DestroyPoint;

    destroy(go);
}
