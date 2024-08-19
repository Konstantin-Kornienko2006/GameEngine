#ifndef ZAMENGINE_H
#define ZAMENGINE_H

#include <stdint.h>

#include <Objects/gameObject.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef void (*DrawFunc_T)();
typedef void (*RecreateFunc_T)();

/// @brief Инициализация Движка
/// @param width - Ширина экрана
/// @param height - Высота экрана
/// @param name - Имя программы
void ZEngineInitSystem(int width, int height, const char* name);
/// @brief Функция установки рендера
/// @param obj - объект типа RenderTexture
/// @param count - количество объектов
void ZEngineSetRender(void *obj, uint32_t count);
/// @brief Установка функции вызываемая при изменении окна
/// @param func - вызываемая функция
void ZEngineSetRecreateFunc(RecreateFunc_T func);
void ZEngineDraw(GameObject *go);
/// @brief Вызов цикла отрисовки оъектов
///     Будет вызвана отрисовка указанных через EngineDraw объектов на все указанные через EngineSetRender рендеры
void ZEngineRender();
/// @brief Функция обработки событий окна
void ZEnginePoolEvents();
/// @brief Установка обратного вызова событий кнопок клавиатуры
/// @param callback - Функция, которая будет вызвана при событии нажатия кнопок клавиатуры
void ZEngineSetKeyCallback(void *callback);
/// @brief Установка обратного вызова ввода с клавиатуры
/// @param callback - Функция, которая будет вызвана при событии нажатия кнопок клавиатуры
void ZEngineSetCharCallback(void *callback);
/// @brief Установка обратного вызова событий кнопок мыши
/// @param callback - Функция, которая будет вызвана при событии нажатия кнопок мыши
void ZEngineSetMouseKeyCallback(void *callback);
/// @brief Установка обратного вызова при изменении позиции мыши на экране
/// @param callback - Функция вызываемая при изменении положения мыши
void ZEngineSetCursorPoscallback(void * callback);
/// @brief Очистка движка перед закрытием 
void ZEngineCleanUp();

#ifdef __cplusplus
}
#endif


#endif