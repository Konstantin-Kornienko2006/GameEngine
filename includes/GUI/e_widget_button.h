#ifndef E_WIDGET_BUTTON_H
#define E_WIDGET_BUTTON_H

#include "Variabels/engine_includes.h"

#include "e_widget.h"
#include "e_widget_image.h"
#include "Objects/text_object.h"

#include <wchar.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct{
    EWidget widget;    
    TextObject to;
    EWidgetImage image;
    vec3 selfColor;
} EWidgetButton;

void ButtonWidgetInit(EWidgetButton *button, const char *text, DrawParam *dParam, EWidget *parent);
void ButtonWidgetSetText(EWidgetButton *button, const char *text);
void ButtonWidgetSetColor(EWidgetButton *button, float r, float g, float b);
void ButtonWidgetSetImage(EWidgetButton *button, char *path, DrawParam *dParam);

#ifdef __cplusplus
}
#endif

#endif // E_WIDGET_BUTTON_H
