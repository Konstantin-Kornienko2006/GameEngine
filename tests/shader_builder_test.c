#include <string.h>
#include <stdio.h>
#include "Tools/e_shaders.h"
   
ShaderBuilder frag;
ShaderBuilder vert;

ShaderBuilder temp;

int main(){

    memset(&vert, 0, sizeof(ShaderBuilder));
    memset(&frag, 0, sizeof(ShaderBuilder));

    ShadersMakeDefault2DShader(&vert, &frag, false);
    
    ShaderBuilderParcingShader(&temp, vert.code, vert.size);

    remove("D:\\Projects\\Temp\\frag.spv");
    remove("D:\\Projects\\Temp\\vert.spv");

    ShaderBuilderWriteToFile(&vert, "D:\\Projects\\Temp\\vert.spv");
    ShaderBuilderWriteToFile(&frag, "D:\\Projects\\Temp\\frag.spv");

    return 0;
}