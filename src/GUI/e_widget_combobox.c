#include "GUI/e_widget_combobox.h"

int ComboboxWidgetPressMain(EWidget* widget, void* entry, void *arg){

    EWidgetCombobox *combo = widget;

    WidgetConfirmTrigger(combo, ENGINE_WIDGET_TRIGGER_COMBOBOX_PRESS, NULL);

    combo->show = !combo->show;

    return 0;
}

int ComboboxWidgetPressSub(EWidget* widget, int id, void *arg){

    EWidgetList *list = widget;

    EWidgetCombobox *parent = widget->parent;

    EWidgetButton *butt = WidgetFindChild(list, id)->node;

    if(parent == NULL)
        return;

    ButtonWidgetSetText(parent, butt->text);

    parent->currId = id;

    WidgetConfirmTrigger(parent, ENGINE_WIDGET_TRIGGER_COMBOBOX_CHANGE_SELLECTED_ITEM, id);

    return 0;
}

extern void ButtonWidgetDraw(EWidgetButton *button);

void ComboboxWidgetDraw(EWidgetCombobox *combobox){
    
    if(combobox->button.widget.widget_flags & ENGINE_FLAG_WIDGET_VISIBLE){
        ButtonWidgetDraw(&combobox->button);

        WidgetSetPosition(&combobox->list, combobox->button.widget.position.x , combobox->button.widget.position.x + combobox->button.widget.scale.y);
    }

}

void ComboboxWidgetInit(EWidgetCombobox *combobox, vec2 scale, EWidget *parent){

    ButtonWidgetInit(combobox, scale," ", parent);
    ButtonWidgetSetColor(combobox, 0.4, 0.4, 0.4);

    GameObjectSetDrawFunc(combobox, ComboboxWidgetDraw);

    combobox->button.widget.type = ENGINE_WIDGET_TYPE_COMBOBOX;
    combobox->button.widget.rounding = 0.f;

    combobox->size_x = scale.x;
    combobox->size_y = scale.y;
    combobox->currId = -1;
    combobox->show = false;

    ListWidgetInit(&combobox->list, scale, combobox);
    WidgetConnect(combobox, ENGINE_WIDGET_TRIGGER_BUTTON_PRESS, ComboboxWidgetPressMain,  NULL);
    WidgetConnect(&combobox->list, ENGINE_WIDGET_TRIGGER_LIST_PRESS_ITEM, ComboboxWidgetPressSub,  NULL);

    combobox->show = false;

}

void ComboboxWidgetAddItem(EWidgetCombobox *combobox, const char* text){
    EWidgetButton *butt = ListWidgetAddItem(&combobox->list, text);

    butt->widget.widget_flags |= ENGINE_FLAG_WIDGET_ALLOCATED;
    butt->widget.rounding = 0.f;
}
