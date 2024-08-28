#ifndef PARTICLESYSTEM2D_H
#define PARTICLESYSTEM2D_H

#include "Variabels/engine_includes.h"

#include "gameObject2D.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct{
    vec2 position;
    vec2 direction;
    vec3 color;
    float scale;
    float life;
    float speed;
    float gravity;
} Particle2D;

typedef struct{
    GameObject2D go;
    Particle2D* particles;
    int num_parts;
} ParticleObject2D;
/// @brief Инициализация объекта частиц
/// @param particle - объект частиц
/// @param dParam - параметры для отрисовки
void Particle2DInit(ParticleObject2D * particle, DrawParam dParam);
/// @brief Установка стандартного чертежа для объекта частиц
///     Используется только после инициализации объекта
/// @param particle - объект частиц
/// @param render - рендер для привязки объекта к нему. Может быть NULL.
void Particle2DAddDefault(ParticleObject2D* particle, void *render);
/// @brief Функция для добавления новой частицы
/// @param particle - объект частиц
/// @param position - положение частицы в пространстве
/// @param direction - направление движения частицы
/// @param speed - скорость движения частицы
/// @param gravity - гравитация цастицы
/// @param life - срок жизни частицы
void Particle2DAdd(ParticleObject2D* particle, vec2 position, vec2 direction, float speed, float gravity, float life);

#ifdef __cplusplus
}
#endif

#endif // PARTICLESYSTEM2D_H
