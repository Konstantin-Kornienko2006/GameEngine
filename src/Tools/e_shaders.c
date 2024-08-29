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

    ShaderStructConstr uniform_arr_2[] = {
        {SHADER_VARIABLE_TYPE_VECTOR, 2, 0, "offset"},
        {SHADER_VARIABLE_TYPE_VECTOR, 2, 0, "scale"},
        {SHADER_VARIABLE_TYPE_VECTOR, 2, 0, "rotation"},
    };

    uint32_t uniform2 = ShaderBuilderAddUniform(frag, uniform_arr_2, 3, "ImageBufferObjects", 0, 2);

    uint32_t fragColor = ShaderBuilderAddIOData(frag, SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 3, "fragColor", 0, 0);
    uint32_t fragTexCoord = ShaderBuilderAddIOData(frag, SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 2, "fragTexCoord", 1, 0);
    uint32_t outColor = ShaderBuilderAddIOData(frag, SHADER_VARIABLE_TYPE_VECTOR, SHADER_DATA_FLAG_OUTPUT, NULL, 4, "outColor", 0, 0);


    uint32_t texture = ShaderBuilderAddIOData(frag, SHADER_VARIABLE_TYPE_IMAGE, SHADER_DATA_FLAG_UNIFORM_CONSTANT, NULL, 0, "Texture2D", 0, 3);

    uint32_t res = ShaderBuilderAddFuncMult(frag, &frag->main_point_index->labels[0], uniform2, 1, SHADER_VARIABLE_TYPE_VECTOR, 2, fragTexCoord, 0, SHADER_VARIABLE_TYPE_VECTOR, 2, 2);

    ShaderBuilderAddFuncSetTexure(frag, &frag->main_point_index->labels[0], texture, res, outColor, 4);

    ShaderBuilderMake(vert);
    ShaderBuilderMake(frag);
}

void ShadersMakeDefault3DShader(ShaderBuilder *vert, ShaderBuilder *frag, bool hasTexture){

    memset(vert, 0, sizeof(ShaderBuilder));
    memset(frag, 0, sizeof(ShaderBuilder));

    ShaderBuilderInit(vert, SHADER_TYPE_VERTEX);
    ShaderBuilderInit(frag, SHADER_TYPE_FRAGMENT);

//------------------------------------------------------

    ShaderStructConstr uniform_arr[] = {
        {SHADER_VARIABLE_TYPE_MATRIX, 4, 0, "model"},
        {SHADER_VARIABLE_TYPE_MATRIX, 4, 0, "view"},
        {SHADER_VARIABLE_TYPE_MATRIX, 4, 0, "porl"},
    };

    uint32_t uniform = ShaderBuilderAddUniform(vert, uniform_arr, 3, "ModelBufferObjects", 0, 1);

    uint32_t posit = ShaderBuilderAddIOData(vert, SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 3, "position", 0, 0);
    uint32_t clr_indx = ShaderBuilderAddIOData(vert, SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 3, "color", 1, 0);
    uint32_t txt_indx = ShaderBuilderAddIOData(vert, SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 2, "inTexCoord", 2, 0);

    uint32_t clr_dst = ShaderBuilderAddIOData(vert, SHADER_VARIABLE_TYPE_VECTOR, SHADER_DATA_FLAG_OUTPUT, NULL, 3, "fragColor", 0, 0);
    uint32_t txt_dst = ShaderBuilderAddIOData(vert, SHADER_VARIABLE_TYPE_VECTOR, SHADER_DATA_FLAG_OUTPUT, NULL, 2, "fragTexCoord", 1, 0);

    uint32_t res = ShaderBuilderAddFuncMult(vert, &vert->main_point_index->labels[0], uniform, 1, SHADER_VARIABLE_TYPE_MATRIX, 4, uniform, 0, SHADER_VARIABLE_TYPE_MATRIX, 4, 4);
    res = ShaderBuilderAddFuncMult(vert, &vert->main_point_index->labels[0], uniform, 2,  SHADER_VARIABLE_TYPE_MATRIX, 4, res, 0, SHADER_VARIABLE_TYPE_MATRIX, 4, 4);
    res = ShaderBuilderAddFuncMult(vert, &vert->main_point_index->labels[0], res, 0,  SHADER_VARIABLE_TYPE_MATRIX, 4, posit, 0, SHADER_VARIABLE_TYPE_VECTOR, 3, 4);
    ShaderBuilderAddFuncMoveToGL(vert, &vert->main_point_index->labels[0], res, 4, vert->gl_struct_indx);
    
    ShaderBuilderAddFuncMove(vert, &vert->main_point_index->labels[0], clr_indx, 3, clr_dst, 3);
    ShaderBuilderAddFuncMove(vert, &vert->main_point_index->labels[0], txt_indx, 2, txt_dst, 2);

//----------------------------------------

    uint32_t fragColor = ShaderBuilderAddIOData(frag, SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 3, "fragColor", 0, 0);
    uint32_t fragTexCoord = ShaderBuilderAddIOData(frag, SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 2, "fragTexCoord", 1, 0);
    uint32_t outColor = ShaderBuilderAddIOData(frag, SHADER_VARIABLE_TYPE_VECTOR, SHADER_DATA_FLAG_OUTPUT, NULL, 4, "outColor", 0, 0);

    uint32_t texture = ShaderBuilderAddIOData(frag, SHADER_VARIABLE_TYPE_IMAGE, SHADER_DATA_FLAG_UNIFORM_CONSTANT, NULL, 0, "Texture2D", 0, 2);

    if(hasTexture){
        ShaderBuilderAddFuncSetTexure(frag, &frag->main_point_index->labels[0], texture, fragTexCoord, outColor, 4);
    }else{

        uint32_t res = ShaderBuilderAcceptLoadL(frag, &frag->main_point_index->labels[0], fragColor, 0);

        VectorExtract extr = ShaderBuilderGetElemenets(frag, &frag->main_point_index->labels[0], res, 0, 3);

        uint32_t vec_type = ShaderBuilderAddVector(frag, 4, NULL);

        float v_f = 1.0f;
        uint32_t v_u = 0;
        memcpy(&v_u, &v_f, sizeof(uint32_t));        
        uint32_t cnst = ShaderBuilderAddConstant(frag, SHADER_VARIABLE_TYPE_FLOAT, 0, v_u, 1);

        uint32_t arr[] = { vec_type, extr.elems[0], extr.elems[1], extr.elems[2], cnst };
        res = ShaderBuilderCompositeConstruct(frag, &frag->main_point_index->labels[0], arr, 5);
        
        uint32_t arr2[] = {outColor, res};
        
        ShaderBuilderStoreValue(frag, &frag->main_point_index->labels[0], arr2, sizeof(arr2));
    }

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

    ShaderBuilderAddFuncMoveToGL(vert, &vert->main_point_index->labels[0], posit, 2, vert->gl_struct_indx);
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