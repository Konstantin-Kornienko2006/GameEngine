#include "Variabels/engine_includes.h"

#include "Core/e_buffer.h"

#include "Objects/gameObject2D.h"

#ifndef IM_DRAWLIST_TEX_LINES_WIDTH_MAX
#define IM_DRAWLIST_TEX_LINES_WIDTH_MAX     (63)
#endif

typedef enum DrawListFlags
{
    GUIDrawListFlags_None                    = 0,
    GUIDrawListFlags_AntiAliasedLines        = 1 << 0,  // Enable anti-aliased lines/borders (*2 the number of triangles for 1.0f wide line or lines thin enough to be drawn using textures, otherwise *3 the number of triangles)
    GUIDrawListFlags_AntiAliasedLinesUseTex  = 1 << 1,  // Enable anti-aliased lines/borders using textures when possible. Require backend to render with bilinear filtering (NOT point/nearest filtering).
    GUIDrawListFlags_AntiAliasedFill         = 1 << 2,  // Enable anti-aliased edge around filled shapes (rounded rectangles, circles).
    GUIDrawListFlags_AllowVtxOffset          = 1 << 3,  // Can emit 'VtxOffset > 0' to allow large meshes. Set when 'ImGuiBackendFlags_RendererHasVtxOffset' is enabled.
} DrawListFlags;

typedef enum DrawFlags
{
    GUIDrawFlags_None                        = 0,
    GUIDrawFlags_Closed                      = 1 << 0, // PathStroke(), AddPolyline(): specify that shape should be closed (Important: this is always == 1 for legacy reason)
    GUIDrawFlags_RoundCornersTopLeft         = 1 << 4, // AddRect(), AddRectFilled(), PathRect(): enable rounding top-left corner only (when rounding > 0.0f, we default to all corners). Was 0x01.
    GUIDrawFlags_RoundCornersTopRight        = 1 << 5, // AddRect(), AddRectFilled(), PathRect(): enable rounding top-right corner only (when rounding > 0.0f, we default to all corners). Was 0x02.
    GUIDrawFlags_RoundCornersBottomLeft      = 1 << 6, // AddRect(), AddRectFilled(), PathRect(): enable rounding bottom-left corner only (when rounding > 0.0f, we default to all corners). Was 0x04.
    GUIDrawFlags_RoundCornersBottomRight     = 1 << 7, // AddRect(), AddRectFilled(), PathRect(): enable rounding bottom-right corner only (when rounding > 0.0f, we default to all corners). Wax 0x08.
    GUIDrawFlags_RoundCornersNone            = 1 << 8, // AddRect(), AddRectFilled(), PathRect(): disable rounding on all corners (when rounding > 0.0f). This is NOT zero, NOT an implicit flag!
    GUIDrawFlags_RoundCornersTop             = GUIDrawFlags_RoundCornersTopLeft | GUIDrawFlags_RoundCornersTopRight,
    GUIDrawFlags_RoundCornersBottom          = GUIDrawFlags_RoundCornersBottomLeft | GUIDrawFlags_RoundCornersBottomRight,
    GUIDrawFlags_RoundCornersLeft            = GUIDrawFlags_RoundCornersBottomLeft | GUIDrawFlags_RoundCornersTopLeft,
    GUIDrawFlags_RoundCornersRight           = GUIDrawFlags_RoundCornersBottomRight | GUIDrawFlags_RoundCornersTopRight,
    GUIDrawFlags_RoundCornersAll             = GUIDrawFlags_RoundCornersTopLeft | GUIDrawFlags_RoundCornersTopRight | GUIDrawFlags_RoundCornersBottomLeft | GUIDrawFlags_RoundCornersBottomRight,
    GUIDrawFlags_RoundCornersDefault_        = GUIDrawFlags_RoundCornersAll, // Default to ALL corners if none of the _RoundCornersXX flags are specified.
    GUIDrawFlags_RoundCornersMask_           = GUIDrawFlags_RoundCornersAll | GUIDrawFlags_RoundCornersNone,
}DrawFlags;

// Flags for ImFontAtlas build
typedef enum FontAtlasFlags
{
    GUIFontAtlasFlags_None               = 0,
    GUIFontAtlasFlags_NoPowerOfTwoHeight = 1 << 0,   // Don't round the height to next power of two
    GUIFontAtlasFlags_NoMouseCursors     = 1 << 1,   // Don't build software mouse cursors into the atlas (save a little texture memory)
    GUIFontAtlasFlags_NoBakedLines       = 1 << 2,   // Don't build thick line textures into the atlas (save a little texture memory, allow support for point/nearest filtering). The AntiAliasedLinesUseTex features uses them, otherwise they will be rendered using polygons (more expensive for CPU/GPU).
} FontAtlasFlags;

typedef struct{
    Vertex2D *points;
    uint32_t vert_count;
    uint32_t *indeces;
    uint32_t indx_count;
} GUIObj;

typedef struct{
    GameObject2D go;
    ChildStack *draw_list;
    BufferObject vertBuffer;
    BufferObject indxBuffer;
    vec2 _Path[256];
    uint32_t _Path_Size;
    float _FringeScale;
    uint32_t currIndx;
    uint32_t Flags;
    bool sellected;

    struct ChildStack* first_widget;
    struct ChildStack* last_widget;

    struct{
        uint32_t fontWidth;
        uint32_t fontHeight;
        void *info;
        void *cdata;
        void *texture;
        float fontSize;
    } font;
} GUIManager;

extern GUIManager gui;
extern ZEngine engine;

void GUIManagerDrawRect(vec2 a, vec2 c, vec3 color);

void GUIAddLine(const vec2 p1, const vec2 p2, vec3 col, float thickness);
void GUIAddRect(const vec2 p_min, const vec2 p_max, vec3 col, float rounding, uint32_t flags, float thickness);
void GUIAddRectFilled(const vec2 p_min, const vec2 p_max, vec3 col, float rounding, uint32_t flags);
void GUIAddQuad(const vec2 p1, const vec2 p2, const vec2 p3, const vec2 p4, vec3 col, float thickness);
void GUIAddQuadFilled(const vec2 p1, const vec2 p2, const vec2 p3, const vec2 p4, vec3 col);
void GUIAddTriangle(const vec2 p1, const vec2 p2, const vec2 p3, vec3 col, float thickness);
void GUIAddTriangleFilled(const vec2 p1, const vec2 p2, const vec2 p3, vec3 col);
void GUIAddCircle(vec2 center, float radius, vec3 col, int num_segments, float thickness);
void GUIAddCircleFilled(vec2 center, float radius, vec3 col, int num_segments);
void GUIAddNgon(vec2 center, float radius, vec3 col, int num_segments, float thickness);
void GUIAddNgonFilled(vec2 center, float radius, vec3 col, int num_segments);
void GUIAddEllipse(vec2 center, const vec2 radius, vec3 col, float rot, int num_segments, float thickness);
void GUIAddEllipseFilled(vec2 center, const vec2 radius, vec3 col, float rot, int num_segments);

void GUIAddTextU8(float xpos, float ypos, vec3 color, float font_size, char *text);
void GUIAddTextU32(float xpos, float ypos, vec3 color, float font_size, uint32_t *text);

void GUIManagerAddConvexPolyFilled(const vec2 *points, const int points_count, vec3 col);
void GUIManagerAddPolyline(const vec2* points, int num_points, vec3 color, uint32_t flags, float thickness);

void PathLineTo(vec2 pos);
void PathFillConvex(vec3 col);
void PathRect(vec2 a, vec2 b, float rounding, uint32_t flags);
void PathStroke(vec3 color, uint32_t flags, float thickness);

void GUIManagerInit();
void GUIManagerDraw();
void GUIManagerClear();
void GUIManagerRecreate();
void GUIManagerDestroy();

#define GUIAddText(xpos, ypos, color, font_size, text)\
    _Generic((text),\
    char *: GUIAddTextU8,\
    const char *: GUIAddTextU8,\
    unsigned int *: GUIAddTextU32\
    )(xpos, ypos, color, font_size, text)
