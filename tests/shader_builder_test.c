#include <string.h>
#include <stdio.h>
#include "Tools/e_shaders.h"
   
ShaderBuilder frag;
ShaderBuilder vert;

int main(){

    memset(&vert, 0, sizeof(ShaderBuilder));
    memset(&frag, 0, sizeof(ShaderBuilder));

    ShadersMakeDefault2DShader(&vert, &frag, true);
    
    remove("D:\\Projects\\Temp\\frag.spv");
    remove("D:\\Projects\\Temp\\vert.spv");

    ShaderBuilderWriteToFile(&vert, "D:\\Projects\\Temp\\vert.spv");
    ShaderBuilderWriteToFile(&frag, "D:\\Projects\\Temp\\frag.spv");

    return 0;
}