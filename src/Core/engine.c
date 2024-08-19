#include "Core/engine.h"
#include "Core/e_memory.h"
#include "Core/e_device.h"
#include "Core/e_window.h"
#include "Core/pipeline.h"
#include "Core/e_buffer.h"
#include "Core/e_camera.h"
#include "Core/e_debuger.h"
#include "Core/swapchain.h"
#include "Core/e_texture.h"
#include "Core/e_texture.h"

#include <vulkan/vulkan.h>

#include "wManager/window_manager.h"
#include "wManager/manager_includes.h"

#include "Variabels/e_texture_variables.h"

#include "string.h"

#include "Objects/gameObject.h"
#include "Objects/text_object.h"
#include "Objects/lightObject.h"
#include "Objects/render_texture.h"

#include "GUI/e_widget_entry.h"
#include "GUI/e_widget_entry_area.h"

#include "Tools/e_tools.h"

#include "Data/e_resource_data.h"
#include "Data/e_resource_engine.h"

extern ZEngine engine;

void EngineCreateSilent(){

    memset(&engine, 0, sizeof(ZEngine));  
    
    engine.window = AllocateMemoryP(1, sizeof(ZWindow), &engine);
    engine.device = AllocateMemoryP(1, sizeof(ZDevice), &engine);
    engine.swapchain = AllocateMemoryP(1, sizeof(ZSwapChain), &engine);
    
    engine.cache.alloc_buffers_memory_head = calloc(1, sizeof(ChildStack));
    engine.cache.alloc_descriptor_head = calloc(1, sizeof(ChildStack));
    engine.cache.alloc_pipeline_head = calloc(1, sizeof(ChildStack));

    wManagerInit();
    createInstance();
    setupDebugMessenger();
    pickPhysicalDevice();
    createLogicalDevice();
    BuffersCreateCommandPool();
}

void EngineInitVulkan(){
    createInstance();
    setupDebugMessenger();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    SwapChainCreate();
    SwapChainCreateImageViews();
    BuffersCreateCommandPool();
    ToolsCreateDepthResources();
    BuffersCreateCommand();
    EngineCreateSyncobjects();
}

void EngineFixedCursorCenter(){
    ZWindow *window = (ZWindow *)engine.window;

    wManagerSetCursorPos(window->e_window, engine.width / 2, engine.height / 2);
}

void EngineGetCursorPos(double *xpos, double *ypos){
    ZWindow *window = (ZWindow *)engine.window;

    wManagerGetCursorPos(window->e_window, xpos, ypos);
}

void EngineSetCursorPos(float xpos, float ypos){
    ZWindow *window = (ZWindow *)engine.window;

    wManagerSetCursorPos(window->e_window, xpos, ypos);
}

void EngineHideCursor(char state){
    ZWindow *window = (ZWindow *)engine.window;

    switch(state){
        case 0 :
            wManagerSetInputMode(window->e_window, ENGINE_CURSOR, ENGINE_CURSOR_DISABLED);
            break;
        case 1 :
            wManagerSetInputMode(window->e_window, ENGINE_CURSOR, ENGINE_CURSOR_HIDDEN);
            break;
        case 2 :
            wManagerSetInputMode(window->e_window, ENGINE_CURSOR, ENGINE_CURSOR_NORMAL);
            break;
    }
}

int EngineGetMousePress(int Key){
    ZWindow *window = (ZWindow *)engine.window;

    int state = wManagerGetMouseButton(window->e_window, Key);

    return state;
}

int EngineWindowIsClosed(){
    return wManagerWindowIsClosed();
}

double EngineGetTime(){
    return wManagerGetTime();
}

const char *EngineGetClipBoardString(){
    ZWindow *window = (ZWindow *)engine.window;

    return wManagerGetClipboardString(window->e_window);
}

void EngineSetClipBoardString(const char *string){
    ZWindow *window = (ZWindow *)engine.window;

    wManagerSetClipboardString(window->e_window, string);
}

void EngineDeviceWaitIdle(){
    ZDevice *device = (ZDevice *)engine.device;

    vkDeviceWaitIdle(device->e_device);
}

int EngineGetKeyPress(int Key){
    ZWindow *window = (ZWindow *)engine.window;

    int res = wManagerGetKey(window->e_window, Key);

    return res;
}

void EngineCreateSyncobjects() {

    ZDevice *device = (ZDevice *)engine.device;

    engine.Sync.imageAvailableSemaphores = (VkSemaphore *)AllocateMemoryP(engine.MAX_FRAMES_IN_FLIGHT, sizeof(VkSemaphore), &engine);
    engine.Sync.renderFinishedSemaphores = (VkSemaphore *)AllocateMemoryP(engine.MAX_FRAMES_IN_FLIGHT, sizeof(VkSemaphore), &engine);
    engine.Sync.inFlightFences = (VkFence *)AllocateMemoryP(engine.MAX_FRAMES_IN_FLIGHT, sizeof(VkFence), &engine);

    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (int i = 0; i < engine.MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(device->e_device, &semaphoreInfo, NULL, &engine.Sync.imageAvailableSemaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(device->e_device, &semaphoreInfo, NULL, &engine.Sync.renderFinishedSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(device->e_device, &fenceInfo, NULL, &engine.Sync.inFlightFences[i]) != VK_SUCCESS) {
            printf("failed to create synchronization objects for a frame!");
            exit(1);
        }
    }
}

void EngineAcceptShadow(void *shadow, uint32_t count, uint32_t shadow_type)
{
    RenderTexture **array;

    switch(shadow_type){
        case ENGINE_SHADOW_TYPE_DIRECTIONAL :
            engine.DataR.dir_shadow_array = AllocateMemory(count, sizeof(RenderTexture *));
            array = engine.DataR.dir_shadow_array;
            engine.DataR.num_dir_shadows = count;
            break;
        case ENGINE_SHADOW_TYPE_POINT :
            engine.DataR.point_shadow_array = AllocateMemory(count, sizeof(RenderTexture *));
            array = engine.DataR.point_shadow_array;
            engine.DataR.num_point_shadows = count;
            break;
        case ENGINE_SHADOW_TYPE_SPOT :
            engine.DataR.spot_shadow_array = AllocateMemory(count, sizeof(RenderTexture *));
            array = engine.DataR.spot_shadow_array;
            engine.DataR.num_spot_shadows = count;
            break;
    }

    RenderTexture *renders = shadow;

    for(int i=0;i < count;i++)
        array[i] = &renders[i];
}

void EngineWaitEvents()
{
    wManagerWaitEvents();
}
