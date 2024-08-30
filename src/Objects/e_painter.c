#include "Objects/e_painter.h"

#include <vulkan/vulkan.h>

#include "Core/e_buffer.h"
#include "Core/graphicsObject.h"

#include "Data/e_resource_data.h"
#include "Data/e_resource_shapes.h"
#include "Data/e_resource_engine.h"
#include "Data/e_resource_export.h"

extern ZEngine engine;

PaintDrawFunc some_func;

vec4 brush_color = {1, 1, 1, 1};

void PainterObjectPainterBufferUpdate(EPainter *painter, BluePrintDescriptor *descriptor)
{
    some_func(painter);

    void* data;

    PainterBuffer pb = {};
    pb.offset.x = 0;
    pb.offset.y = 0;

    vec2 offset = {0, 0};

    pb.position = painter->go.transform.position;
    pb.size = painter->go.transform.scale;

    DescriptorUpdate(descriptor, (char *)&pb, sizeof(pb));
}

void PainterObjectDrawObjectsBufferUpdate(EPainter *painter, BluePrintDescriptor *descriptor)
{
    some_func(painter);

    DescriptorUpdate(descriptor, (char *)&painter->drawObjects, sizeof(painter->drawObjects));;

    memset(&painter->drawObjects, 0, sizeof(painter->drawObjects));
}

void PainterObjectInit(EPainter *painter)
{
    memset(painter, 0, sizeof(EPainter));
    memcpy(painter->go.name, "Painter", 7);

    GameObject2DInit((GameObject2D *)painter);

    GraphicsObjectSetVertex(&painter->go.graphObj, (void **)projPlaneVert, 4, sizeof(Vertex2D), (void **)projPlaneIndx, 6, sizeof(uint32_t));
}

void PainterObjectAddDefault(EPainter *painter, void *render)
{    
    uint32_t num_pack = BluePrintInit(&painter->go.graphObj.blueprints);
    
    GraphicsObjectSetSomeShader(&painter->go.graphObj, &_binary_shaders_gui_painter_vert_spv_start, (size_t)(&_binary_shaders_gui_painter_vert_spv_size), num_pack);
    GraphicsObjectSetSomeShader(&painter->go.graphObj, &_binary_shaders_gui_painter_frag_spv_start, (size_t)(&_binary_shaders_gui_painter_frag_spv_size), num_pack);

    BluePrintAddSomeUpdater(&painter->go.graphObj.blueprints, num_pack, 0, PainterObjectPainterBufferUpdate);
    BluePrintAddSomeUpdater(&painter->go.graphObj.blueprints, num_pack, 1, PainterObjectDrawObjectsBufferUpdate);
}

void PainterObjectInitDefault(EPainter *painter, DrawParam *dParam)
{
    PainterObjectInit(painter);
    PainterObjectAddDefault(painter, dParam->render);
    GameObject2DInitDraw((GameObject2D *)painter);
}

void PainterObjectSetPaintFunc(PaintDrawFunc paint_func)
{
    some_func = paint_func;
}

void PainterObjectSetColor(float r, float g, float b)
{
    brush_color.x = r;
    brush_color.y = g;
    brush_color.z = b;
}

void PainterObjectMakeRectangle(EPainter *painter, float x, float y, float width, float height)
{
    int size = painter->drawObjects.size;
    painter->drawObjects.objs[size].type = ENGINE_PAINTER_TYPE_BOX;
    painter->drawObjects.objs[size].color = brush_color;
    painter->drawObjects.objs[size].position.x = x / engine.width;
    painter->drawObjects.objs[size].position.y = y / engine.height;
    painter->drawObjects.objs[size].size.x = width / engine.width;
    painter->drawObjects.objs[size].size.y = height / engine.height;
    painter->drawObjects.objs[size].transparent = 1.0;

    painter->drawObjects.size ++;
}

void PainterObjectMakeCircle(EPainter *painter, float x, float y, float radius)
{
    int size = painter->drawObjects.size;
    painter->drawObjects.objs[size].type = ENGINE_PAINTER_TYPE_CIRCLE;
    painter->drawObjects.objs[size].color = brush_color;
    painter->drawObjects.objs[size].position.x = x / engine.width;
    painter->drawObjects.objs[size].position.y = y / engine.height;
    painter->drawObjects.objs[size].radius = radius / 1000;
    painter->drawObjects.objs[size].transparent = 1.0;

    painter->drawObjects.size ++;
}


void PainterObjectMakeOrientedBox(EPainter *painter, float x, float y, float width, float height, float angle)
{
    int size = painter->drawObjects.size;
    painter->drawObjects.objs[size].type = ENGINE_PAINTER_TYPE_ORIENTED_BOX;
    painter->drawObjects.objs[size].color = brush_color;
    painter->drawObjects.objs[size].position.x = x / engine.width;
    painter->drawObjects.objs[size].position.y = y / engine.height;
    painter->drawObjects.objs[size].size.x = width / engine.width;
    painter->drawObjects.objs[size].size.y = height / engine.height;
    painter->drawObjects.objs[size].angle = angle;
    painter->drawObjects.objs[size].transparent = 1.0;

    painter->drawObjects.size ++;
}


void PainterObjectMakeSegment(EPainter *painter, float x, float y, float x2, float y2, float radius)
{
    int size = painter->drawObjects.size;
    painter->drawObjects.objs[size].type = ENGINE_PAINTER_TYPE_SEGMENT;
    painter->drawObjects.objs[size].color = brush_color;
    painter->drawObjects.objs[size].position.x = x / engine.width;
    painter->drawObjects.objs[size].position.y = y / engine.height;
    painter->drawObjects.objs[size].size.x = x2 / engine.width;
    painter->drawObjects.objs[size].size.y = y2 / engine.height;
    painter->drawObjects.objs[size].radius = radius / 1000;

    painter->drawObjects.objs[size].transparent = 1.0;

    painter->drawObjects.size ++;
}

