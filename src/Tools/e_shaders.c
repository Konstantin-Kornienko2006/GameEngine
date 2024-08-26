#include "Tools/e_shaders.h"

void ShadersMakeDefault2DShader(ShaderBuilder *vert, ShaderBuilder *frag, bool hasTexture){

    memset(vert, 0, sizeof(ShaderBuilder));
    memset(frag, 0, sizeof(ShaderBuilder));

    ShaderBuilderInit(vert, SHADER_TYPE_VERTEX);
    ShaderBuilderInit(frag, SHADER_TYPE_FRAGMENT);

//------------------------------------------------------

    ShaderStructConstr uniform_arr[] = {
        {SHADER_VARIABLE_TYPE_VECTOR, 2, 0, "position"},
        {SHADER_VARIABLE_TYPE_VECTOR, 2, 0, "rotation"},
        {SHADER_VARIABLE_TYPE_VECTOR, 2, 0, "scale"},
    };

    uint32_t uniform = ShaderBuilderAddUniform(vert, uniform_arr, 3, "TransformBufferObjects", 0, 1);

    uint32_t posit = ShaderBuilderAddIOData(vert, SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 2, "position", 0, 0);
    uint32_t clr_indx = ShaderBuilderAddIOData(vert, SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 3, "color", 1, 0);
    uint32_t txt_indx = ShaderBuilderAddIOData(vert, SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 2, "inTexCoord", 2, 0);

    uint32_t clr_dst = ShaderBuilderAddIOData(vert, SHADER_VARIABLE_TYPE_VECTOR, SHADER_DATA_FLAG_OUTPUT, NULL, 3, "fragColor", 0, 0);
    uint32_t txt_dst = ShaderBuilderAddIOData(vert, SHADER_VARIABLE_TYPE_VECTOR, SHADER_DATA_FLAG_OUTPUT, NULL, 2, "fragTexCoord", 1, 0);

    ShaderBuilderAddFuncAdd(vert, &vert->main_point_index->labels[0], uniform, posit, 2, vert->gl_struct_indx);
    ShaderBuilderAddFuncMove(vert, &vert->main_point_index->labels[0], clr_indx, 3, clr_dst, 3);
    ShaderBuilderAddFuncMove(vert, &vert->main_point_index->labels[0], txt_indx, 2, txt_dst, 2);

//----------------------------------------

    uint32_t fragColor = ShaderBuilderAddIOData(frag, SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 3, "fragColor", 0, 0);
    uint32_t fragTexCoord = ShaderBuilderAddIOData(frag, SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 2, "fragTexCoord", 1, 0);
    uint32_t outColor = ShaderBuilderAddIOData(frag, SHADER_VARIABLE_TYPE_VECTOR, SHADER_DATA_FLAG_OUTPUT, NULL, 4, "outColor", 0, 0);


    uint32_t texture = ShaderBuilderAddIOData(frag, SHADER_VARIABLE_TYPE_IMAGE, SHADER_DATA_FLAG_UNIFORM_CONSTANT, NULL, 0, "Texture2D", 0, 2);

    if(hasTexture)
        ShaderBuilderAddFuncSetTexure(frag, &frag->main_point_index->labels[0], texture, fragTexCoord, outColor, 4);
    else
        ShaderBuilderAddFuncMove(frag, &frag->main_point_index->labels[0], fragColor, 3, outColor, 4);

    ShaderBuilderMake(vert);
    ShaderBuilderMake(frag);
}

void ShadersMakeClear2DShader(ShaderBuilder *vert, ShaderBuilder *frag){

    memset(vert, 0, sizeof(ShaderBuilder));
    memset(frag, 0, sizeof(ShaderBuilder));

    ShaderBuilderInit(vert, SHADER_TYPE_VERTEX);
    ShaderBuilderInit(frag, SHADER_TYPE_FRAGMENT);

//------------------------------------------------------

    ShaderStructConstr uniform_arr[] = {
        {SHADER_VARIABLE_TYPE_VECTOR, 2, 0, "position"},
        {SHADER_VARIABLE_TYPE_VECTOR, 2, 0, "rotation"},
        {SHADER_VARIABLE_TYPE_VECTOR, 2, 0, "scale"},
    };
    
    uint32_t posit = ShaderBuilderAddIOData(vert, SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 2, "position", 0, 0);
    uint32_t clr_indx = ShaderBuilderAddIOData(vert, SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 3, "color", 1, 0);
    uint32_t txt_indx = ShaderBuilderAddIOData(vert, SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 2, "inTexCoord", 2, 0);

    uint32_t clr_dst = ShaderBuilderAddIOData(vert, SHADER_VARIABLE_TYPE_VECTOR, SHADER_DATA_FLAG_OUTPUT, NULL, 3, "fragColor", 0, 0);
    uint32_t txt_dst = ShaderBuilderAddIOData(vert, SHADER_VARIABLE_TYPE_VECTOR, SHADER_DATA_FLAG_OUTPUT, NULL, 2, "fragTexCoord", 1, 0);

    ShaderBuilderAddFuncMoveToGL(vert, &vert->main_point_index->labels[0], posit, vert->gl_struct_indx);
    ShaderBuilderAddFuncMove(vert, &vert->main_point_index->labels[0], clr_indx, 3, clr_dst, 3);
    ShaderBuilderAddFuncMove(vert, &vert->main_point_index->labels[0], txt_indx, 2, txt_dst, 2);

    
//----------------------------------------

    uint32_t fragColor = ShaderBuilderAddIOData(frag, SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 3, "fragColor", 0, 0);
    uint32_t fragTexCoord = ShaderBuilderAddIOData(frag, SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 2, "fragTexCoord", 1, 0);
    uint32_t outColor = ShaderBuilderAddIOData(frag, SHADER_VARIABLE_TYPE_VECTOR, SHADER_DATA_FLAG_OUTPUT, NULL, 4, "outColor", 0, 0);

    uint32_t texture = ShaderBuilderAddIOData(frag, SHADER_VARIABLE_TYPE_IMAGE, SHADER_DATA_FLAG_UNIFORM_CONSTANT, NULL, 0, "Texture2D", 0, 1);

    uint32_t tExtract = ShaderBuilderGetTexture(frag, &frag->main_point_index->labels[0], texture, fragTexCoord);

    uint32_t res = ShaderBuilderAddFuncSetColor4(frag, &frag->main_point_index->labels[0], fragColor, tExtract, 1);

    uint32_t arr[] = { outColor, res} ;
    ShaderBuilderStoreValue(frag, &frag->main_point_index->labels[0], arr, 2);

    ShaderBuilderMake(vert);
    ShaderBuilderMake(frag);
}

void ShadersMakeDefault2DTextShader(ShaderBuilder *vert, ShaderBuilder *frag){

    memset(vert, 0, sizeof(ShaderBuilder));
    memset(frag, 0, sizeof(ShaderBuilder));

    ShaderBuilderInit(vert, SHADER_TYPE_VERTEX);
    ShaderBuilderInit(frag, SHADER_TYPE_FRAGMENT);

//------------------------------------------------------

    ShaderStructConstr uniform_arr[] = {
        {SHADER_VARIABLE_TYPE_VECTOR, 2, 0, "position"},
        {SHADER_VARIABLE_TYPE_VECTOR, 2, 0, "rotation"},
        {SHADER_VARIABLE_TYPE_VECTOR, 2, 0, "scale"},
    };

    uint32_t uniform = ShaderBuilderAddUniform(vert, uniform_arr, 3, "TransformBufferObjects", 0, 1);

    uint32_t posit = ShaderBuilderAddIOData(vert, SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 2, "position", 0, 0);
    uint32_t clr_indx = ShaderBuilderAddIOData(vert, SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 3, "color", 1, 0);
    uint32_t txt_indx = ShaderBuilderAddIOData(vert, SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 2, "inTexCoord", 2, 0);

    uint32_t clr_dst = ShaderBuilderAddIOData(vert, SHADER_VARIABLE_TYPE_VECTOR, SHADER_DATA_FLAG_OUTPUT, NULL, 3, "fragColor", 0, 0);
    uint32_t txt_dst = ShaderBuilderAddIOData(vert, SHADER_VARIABLE_TYPE_VECTOR, SHADER_DATA_FLAG_OUTPUT, NULL, 2, "fragTexCoord", 1, 0);

    ShaderBuilderAddFuncAdd(vert, &vert->main_point_index->labels[0], uniform, posit, 2, vert->gl_struct_indx);
    ShaderBuilderAddFuncMove(vert, &vert->main_point_index->labels[0], clr_indx, 3, clr_dst, 3);
    ShaderBuilderAddFuncMove(vert, &vert->main_point_index->labels[0], txt_indx, 2, txt_dst, 2);

//----------------------------------------

    uint32_t fragColor = ShaderBuilderAddIOData(frag, SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 3, "fragColor", 0, 0);
    uint32_t fragTexCoord = ShaderBuilderAddIOData(frag, SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 2, "fragTexCoord", 1, 0);
    uint32_t outColor = ShaderBuilderAddIOData(frag, SHADER_VARIABLE_TYPE_VECTOR, SHADER_DATA_FLAG_OUTPUT, NULL, 4, "outColor", 0, 0);


    uint32_t texture = ShaderBuilderAddIOData(frag, SHADER_VARIABLE_TYPE_IMAGE, SHADER_DATA_FLAG_UNIFORM_CONSTANT, NULL, 0, "Texture2D", 0, 2);

    uint32_t tExtract = ShaderBuilderGetTexture(frag, &frag->main_point_index->labels[0], texture, fragTexCoord);

    uint32_t res = ShaderBuilderAddFuncSetColor4(frag, &frag->main_point_index->labels[0], fragColor, tExtract, 1);

    uint32_t arr[] = { outColor, res} ;
    ShaderBuilderStoreValue(frag, &frag->main_point_index->labels[0], arr, 2);

    ShaderBuilderMake(vert);
    ShaderBuilderMake(frag);
}