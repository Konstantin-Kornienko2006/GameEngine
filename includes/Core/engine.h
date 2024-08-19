/// @file Core/engine.h
#ifndef GAMENGINE_H
#define GAMENGINE_H

#include "Variabels/engine_includes.h"

#ifdef __cplusplus
extern "C"
{
#endif

void EngineCreateSilent();
/// @brief Инициализация вулкана
void EngineInitVulkan();
/// @brief Фиксация указателя в центре экрана
void EngineFixedCursorCenter();
/// @brief Получение положения указателя
/// @param xpos - положение по Х
/// @param ypos - положение по У
void EngineGetCursorPos(double *xpos, double *ypos);
/// @brief Установка положения указателя
/// @param xpos - положение по Х
/// @param ypos - положение по У
void EngineSetCursorPos(float xpos, float ypos);
/// @brief Состояние указателя
/// @param state - Состояние 1 : Отключен и не видим. Состояние 2 : Не видим и активен. Состояние 3 : Видим и активен
void EngineHideCursor(char state);
/// @brief Нажата ли клавиша мыши
/// @param Key - Идентификатор мыши
/// @return Возвращет 0 (false) или 1 (true)
int EngineGetMousePress(int Key);
/// @brief Нажата ли кнопка клавиатуры
/// @param Key - Идентификатор кнопки клавиатуры
/// @return Возвращет 0 (false) или 1 (true)
int EngineGetKeyPress(int Key);
/// @brief Проверка закрыто ли окно
/// @return Возвращает 0 (false) и 1 (true)
int EngineWindowIsClosed();
/// @brief Получение текущего времени с момента старта программы
/// @return Возвращает время
double EngineGetTime();
/// @brief Получение строки сохраненной в буфере окна
/// @return Возвращает указатель на строку
const char *EngineGetClipBoard();
/// @brief Установка набора символов в буфере окна
/// @param string - Строка содежащая символы
void EngineSetClipBoardString(const char *string);
/// @brief Функция ожидания какого-либо события окна
void EngineWaitEvents();
/// @brief Функция ожидания окончания работы устройства
void EngineDeviceWaitIdle();
/// @brief Функция очистки сменяемы буферов
void EngineCleanupSwapChain();
/// @brief Функция пересоздания сменяемых буферов
void EnginereRecreateSwapChain();
/// @brief Функция создания объектов для синхронизации кадров
void EngineCreateSyncobjects();
/// @brief Функция установки рендеров теней
/// @param shadow - объект рендера типа RenderTexture
/// @param count - количество объектов
/// @param shadow_type - тип тени
void EngineAcceptShadow(void *shadow, uint32_t count, uint32_t shadow_type);

#ifdef __cplusplus
}
#endif

#endif
