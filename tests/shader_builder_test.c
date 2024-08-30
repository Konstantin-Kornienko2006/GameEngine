#include <string.h>
#include <stdio.h>

#include "Objects/primitiveObject.h"

#include "Tools/e_shaders.h"
   
ShaderBuilder frag;
ShaderBuilder vert;

ShaderBuilder temp;
ShaderBuilder temp2;

PrimitiveObject po;

int main(){

    memset(&vert, 0, sizeof(ShaderBuilder));
    memset(&frag, 0, sizeof(ShaderBuilder));

    ShadersMakeDefault2DShader(&vert, &frag, false);
    
    uint32_t num = BluePrintInit(&po.go.graphObj.blueprints);

    ShaderBuilderMakeUniformsFromShader(&temp, vert.code, vert.size, &po.go.graphObj.blueprints, num, 1);
    ShaderBuilderMakeUniformsFromShader(&temp2, frag.code, frag.size, &po.go.graphObj.blueprints, num, 1);

    remove("D:\\Projects\\Temp\\frag.spv");
    remove("D:\\Projects\\Temp\\vert.spv");

    ShaderBuilderWriteToFile(&vert, "D:\\Projects\\Temp\\vert.spv");
    ShaderBuilderWriteToFile(&frag, "D:\\Projects\\Temp\\frag.spv");

    return 0;
}