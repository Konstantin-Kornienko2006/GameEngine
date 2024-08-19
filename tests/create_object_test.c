#include <ZamEngine.h>

#include <Core/engine.h>
#include <Objects/render_texture.h>
#include <Objects/primitiveObject.h>
#include <wManager/window_manager.h>

PrimitiveObject po;

int main(){

    EngineCreateSilent();
        
    DrawParam dParam;
    memset(&dParam, 0, sizeof(DrawParam));

    PrimitiveObjectInit(&po, &dParam, ENGINE_PRIMITIVE3D_CUBE, NULL);
   
    printf("Allocated count : %i\n", GetAllocatedMemoryCount());  
    GameObjectDestroy((GameObject *)&po);
    
    printf("Allocated count : %i\n", GetAllocatedMemoryCount()); 
    EngineDeviceWaitIdle();
    
    ZEngineCleanUp();

    return 0;
}