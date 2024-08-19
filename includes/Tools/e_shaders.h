#ifndef SHADERS_H
#define SHADERS_H

#include "Variabels/engine_includes.h"
#include "shader_builder.h"

void ShadersMakeDefault2DShader(ShaderBuilder *vert, ShaderBuilder *frag, bool hasTexture);
void ShadersMakeDefault2DTextShader(ShaderBuilder *vert, ShaderBuilder *frag);

#endif // SHADERS_H
