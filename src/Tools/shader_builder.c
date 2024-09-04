#include "Tools/shader_builder.h"

#include "Core/e_blue_print.h"

#include "spirv.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>

ShaderVariable *ShaderBuilderAllocateVariabel(ShaderBuilder *builder){

    ShaderVariable *variable = calloc(1, sizeof(ShaderVariable));

     if(builder->alloc_head->node == NULL){
        builder->alloc_head->next = calloc(1, sizeof(ChildStack));
        builder->alloc_head->node = variable;
    }
    else{

        ChildStack *child = builder->alloc_head;

        while(child->next != NULL)
        {
            child = child->next;
        }

        child->next = calloc(1, sizeof(ChildStack));
        child->node = variable;
    }

    return variable;
}

uint32_t ShaderBuilderGetVariablesCount(ShaderBuilder *builder){
    ChildStack *child = builder->alloc_head;

    uint32_t counter = 0;    
    if(child == NULL)
        return counter;
           
    while(child != NULL){    
        
        if(child->node != NULL){
            counter ++;
        }

        child = child->next;
    }

    return counter;
}

void ShaderBuilderDeleteVariabel(ShaderBuilder *builder, ShaderVariable *variable){
    ShaderVariable *curr = NULL;

    ChildStack *child = builder->alloc_head;
    ChildStack *before = NULL;

    while(child != NULL)
    {
        curr = child->node;

        if(curr == variable)
            break;

        before = child;
        child = child->next;
    }

    if(curr == NULL){
        printf("Can't find this memory 0x%x\n", variable);
        return;
    }

    if(child->next != NULL){

        if(child->node != NULL)
            free(child->node);

        if(before != NULL)
            before->next = child->next;
        else
            builder->alloc_head = child->next;

        free(child);
        child = NULL;

    }else{
        
        if(before != NULL){
            free(child);
            child = NULL;  
        } 
    }
}

void ShaderBuilderClear(ShaderBuilder *builder){
    ChildStack *child = builder->alloc_head;
    
    if(child == NULL)
        return;

    ChildStack *next = NULL;
    ChildStack *before = NULL;
           
    while(child != NULL){    
        
        if(child->node != NULL){
            free(child->node);
        }

        before = child;  
        child = child->next;

        free(before);
        before = NULL;
    }
    
    builder->alloc_head = NULL;
}

uint32_t ShaderBuilderCheckVariable(ShaderBuilder *builder, ShaderVariableType variable_type, uint32_t *vals,  uint32_t size){

    ShaderVariable *variable = NULL;
    
    ChildStack *child = builder->alloc_head;

    int find = 0;
    while(child != NULL){
        variable = child->node;

        if(variable != NULL){
            find = 1;

            if(variable->type == variable_type){
                for(int j=0;j < size;j++){
                    if(variable->values[j] != vals[j]){
                        find = 0;
                        break;
                    }
                }

                if(find)
                    return variable->indx;
            }
        }

        child = child->next;
    }

    return 0;
}

ShaderVariable *ShaderBuilderFindVar(ShaderBuilder *builder, uint32_t indx){

    ShaderVariable *variable = NULL;
    
    ChildStack *child = builder->alloc_head;

    int find = 0;
    while(child != NULL){
        variable = child->node;

        if(variable != NULL){
            if(variable->indx == indx){
                return variable;
            }
        }

        child = child->next;
    }

    return NULL;
}

void ShaderBuilderAddOp(ShaderBuilder *builder, uint32_t code, uint32_t value){

    builder->code[builder->size] = (value << 16) | code;

    builder->size++;
}

void ShaderBuilderAddValue(ShaderBuilder *builder, uint32_t value){

    builder->code[builder->size] = value;

    builder->size++;
}

void ShaderBuilderAddString(ShaderBuilder *builder, char* string){
    uint32_t len = strlen(string);

    char *point = &builder->code[builder->size];

    memcpy(point, string, len);


    while(len % sizeof(uint32_t)){
        builder->infos[builder->num_debug_infos].name[len] = 0;
        len ++;
    }

    builder->size += len / sizeof(uint32_t);

    /*if(string[len + 1] != 0)
        ShaderBuilderAddValue(builder, 0x0 );*/
}

uint32_t ShaderBuilderAddVariableF(ShaderBuilder *builder, ShaderVariableType type, uint32_t flags, uint32_t *args, uint32_t num_args, uint32_t *vals,  uint32_t num_vals){

    ShaderVariable *variable = ShaderBuilderAllocateVariabel(builder);

    variable->type = type;
    variable->indx = builder->current_index + 1;

    if(num_args > 0)
    {
        if(args == NULL)
            return;

        for(int i=0; i  < num_args;i++)
            variable->args[i] = args[i];

    }

    variable->num_args = num_args;

    if(num_vals > 0)
    {
        if(vals == NULL)
            return;

        for(int i=0; i  < num_vals;i++)
            variable->values[i] = vals[i];

    }

    variable->num_values = num_vals;
    variable->flags = flags;

    builder->current_index++;
    
    return builder->current_index;
}

uint32_t ShaderBuilderAddVariable(ShaderBuilder *builder, ShaderVariableType type, uint32_t flags, uint32_t *args, uint32_t num_args, uint32_t *vals,  uint32_t num_vals){

    ShaderVariable *variable = NULL;

    ChildStack *child = builder->alloc_head;

    int find = 0;
    while(child != NULL){
        variable = child->node;

        if(variable != NULL){
            if(flags == variable->flags && type == variable->type)
                find = 1;
            else{
                child = child->next;
                continue;
            }

            if(variable->num_args != num_args || variable->num_values != num_vals){
                find = 0;
                child = child->next;
                continue;
            }

            for(int j=0; j < num_args;j++){
                if(variable->args[j] != args[j])
                    find = 0;
            }

            for(int j=0; j < num_vals;j++){
                if(variable->values[j] != vals[j])
                    find = 0;
            }

            if(find)
                break;
        }

        child = child->next;
    }

    if(find){
        return variable->indx;
    }
    
    return ShaderBuilderAddVariableF(builder, type, flags, args, num_args, vals,  num_vals);
}

uint32_t ShaderBuilderAddFloat(ShaderBuilder *builder){

    uint32_t arr[] = { 32, 1 };
    uint32_t res = ShaderBuilderCheckVariable(builder, SHADER_VARIABLE_TYPE_FLOAT, arr, 2);

    if(!res){
        res = ShaderBuilderAddVariable(builder, SHADER_VARIABLE_TYPE_FLOAT, 0, NULL, 0, arr, 2);
    }

    return res;
}

uint32_t ShaderBuilderAddInt(ShaderBuilder *builder, uint32_t sign){

    uint32_t arr[] = { 32, sign };
    uint32_t res = ShaderBuilderCheckVariable(builder, SHADER_VARIABLE_TYPE_INT, arr, 2);

    if(!res){
        res = ShaderBuilderAddVariable(builder, SHADER_VARIABLE_TYPE_INT, 0, NULL, 0, arr, 2);
    }

    return res;
}

uint32_t ShaderBuilderCheckConstans(ShaderBuilder *builder, uint32_t type_indx, uint32_t valu){

    ShaderVariable *variable = NULL;

    ChildStack *child = builder->alloc_head;

    int find = 0;
    while(child != NULL){
        variable = child->node;

        if(variable != NULL){
            if(variable->type == SHADER_VARIABLE_TYPE_CONSTANT && variable->args[0] == type_indx && variable->values[0] == valu)
                return variable->indx;

        }

        child = child->next;
    }

    return 0;
}

int ShaderBuilderAddConstant(ShaderBuilder *builder, ShaderVariableType var_type, ShaderDataFlags flags,  uint32_t valu, uint32_t sign){

    uint32_t arr[] = { 32, sign };
    uint32_t type_indx = ShaderBuilderCheckVariable(builder, var_type, arr, 2);

    if(!type_indx){
        switch (var_type) {
            case SHADER_VARIABLE_TYPE_INT:
                type_indx = ShaderBuilderAddInt(builder, sign);
                break;
            case SHADER_VARIABLE_TYPE_FLOAT:
                type_indx = ShaderBuilderAddFloat(builder);
                break;
        }
    }

    uint32_t res = ShaderBuilderCheckConstans(builder, type_indx, valu);

    if(!res){
        ShaderVariable *variable = ShaderBuilderAllocateVariabel(builder);

        variable->type = SHADER_VARIABLE_TYPE_CONSTANT;
        variable->indx = builder->current_index + 1;
        variable->args[0] = type_indx;
        variable->values[0] = valu;

        variable->num_args = 1;
        variable->num_values = 1;
        variable->flags = flags;

        builder->current_index++;

        return builder->current_index;
    }

    return res;
}

uint32_t ShaderBuilderCheckArray(ShaderBuilder *builder, uint32_t type_indx, uint32_t const_indx){

    ShaderVariable *variable = NULL;

    ChildStack *child = builder->alloc_head;

    int find = 0;
    while(child != NULL){
        variable = child->node;

        if(variable != NULL){

            if(variable->type == SHADER_VARIABLE_TYPE_ARRAY)
                if(variable->args[0] == type_indx && variable->args[1] == const_indx)
                    return variable->indx;
        }

        child = child->next;
    }

    return 0;
}

uint32_t ShaderBuilderAddArray(ShaderBuilder *builder, ShaderVariableType var_type, uint32_t count, char *name){

    uint32_t res = 0, type_indx = 0;

    switch(var_type){
        case SHADER_VARIABLE_TYPE_FLOAT:
            type_indx = ShaderBuilderAddFloat(builder);
            break;
        case SHADER_VARIABLE_TYPE_INT:
             type_indx = ShaderBuilderAddInt(builder, 0);
            break;
    }

    uint32_t cnst = ShaderBuilderAddConstant(builder, SHADER_VARIABLE_TYPE_INT, 0, count, 0);

    uint32_t check = ShaderBuilderCheckArray(builder, type_indx, cnst);

    if(!check){
        uint32_t arr[] = { type_indx, cnst };
        res = ShaderBuilderAddVariable(builder, SHADER_VARIABLE_TYPE_ARRAY, 0, arr, 2, NULL, 0 );
    }else
        res = check;

    if(name != NULL){
        uint32_t len = strlen(name);

        memcpy(builder->infos[builder->num_debug_infos].name, name, len);
        builder->infos[builder->num_debug_infos].indx = res;

        builder->num_debug_infos++;
    }

    return res;

}

uint32_t ShaderBuilderAddVector(ShaderBuilder *builder, uint32_t size, char *name){

    uint32_t arr[] = { size };
    uint32_t res = ShaderBuilderCheckVariable(builder, SHADER_VARIABLE_TYPE_VECTOR, arr, 1);

    if(!res){
        res = ShaderBuilderAddFloat(builder);

        uint32_t arr[] = { res };
        uint32_t arr2[] = { size };
        res = ShaderBuilderAddVariable(builder, SHADER_VARIABLE_TYPE_VECTOR, 0, arr, 1, arr2, 1);
    }

    if(name != NULL){
        uint32_t len = strlen(name);

        memcpy(builder->infos[builder->num_debug_infos].name, name, len);
        builder->infos[builder->num_debug_infos].indx = res;

        builder->num_debug_infos++;
    }

    return res;
}

uint32_t ShaderBuilderAddMatrix(ShaderBuilder *builder, uint32_t size, char *name){

    uint32_t arr[] = { size };
    uint32_t res = ShaderBuilderCheckVariable(builder, SHADER_VARIABLE_TYPE_MATRIX, arr, 1);

    if(!res){
        uint32_t vec_type = ShaderBuilderAddVector(builder, size, NULL);

        uint32_t arr[] = { vec_type };
        uint32_t arr2[] = { size };
        res = ShaderBuilderAddVariable(builder, SHADER_VARIABLE_TYPE_MATRIX, 0, arr, 1, arr2, 1);
    }

    if(name != NULL){
        uint32_t len = strlen(name);

        memcpy(builder->infos[builder->num_debug_infos].name, name, len);
        builder->infos[builder->num_debug_infos].indx = res;

        builder->num_debug_infos++;
    }

    return res;
}

uint32_t ShaderBuilderAddImage(ShaderBuilder *builder){
    
    uint32_t arr[] = { 1, 0, 0, 0, 1, 0};
    uint32_t res = ShaderBuilderCheckVariable(builder, SHADER_VARIABLE_TYPE_IMAGE, arr, 6);

    if(!res){
        res = ShaderBuilderAddFloat(builder);
        
        uint32_t arr[] = { res };
        uint32_t arr2[] = { 1, 0, 0, 0, 1, 0};
        res = ShaderBuilderAddVariable(builder, SHADER_VARIABLE_TYPE_IMAGE, 0, arr, 1, arr2, 6);

    }
    
    uint32_t arr2[] = { res };
    res = ShaderBuilderAddVariable(builder, SHADER_VARIABLE_TYPE_SAMPLED_IMAGE, 0, arr2, 1, NULL, 0);

    return res;
}

uint32_t ShaderBuilderAddStruct(ShaderBuilder *builder, ShaderStructConstr *struct_arr, uint32_t count, char *name ){

    uint32_t elem_indx[count];

    for(int i = 0;i < count;i++){

         switch(struct_arr[i].var_type){
             case SHADER_VARIABLE_TYPE_INT:
                 elem_indx[i] = ShaderBuilderAddInt(builder, 1);
                 break;
             case SHADER_VARIABLE_TYPE_FLOAT:
                 elem_indx[i] = ShaderBuilderAddFloat(builder);
                 break;
             case SHADER_VARIABLE_TYPE_VECTOR:
                 elem_indx[i] = ShaderBuilderAddVector(builder, struct_arr[i].size, NULL);
                 break;
             case SHADER_VARIABLE_TYPE_MATRIX:
                 elem_indx[i] = ShaderBuilderAddMatrix(builder, struct_arr[i].size, NULL);
                 break;
             case SHADER_VARIABLE_TYPE_ARRAY:
                 elem_indx[i] = ShaderBuilderAddArray(builder, struct_arr[i].type, struct_arr[i].size, NULL);
                 break;
         }
    }

    ShaderVariable *variable = ShaderBuilderAllocateVariabel(builder);

    memcpy(variable->args, elem_indx, sizeof(uint32_t) * count);

    variable->type = SHADER_VARIABLE_TYPE_STRUCT;
    variable->indx = builder->current_index + 1;
    variable->num_args = count;
    variable->num_values = 0;
    variable->flags = 0;

    uint32_t len = strlen(name);

    memcpy(builder->infos[builder->num_debug_infos].name, name, len);
    builder->infos[builder->num_debug_infos].indx = builder->current_index + 1;

    for(int i=0;i < count;i++){
        len = strlen(struct_arr[i].name);
        memcpy(builder->infos[builder->num_debug_infos].child_name[i], struct_arr[i].name, len);
    }

    builder->infos[builder->num_debug_infos].num_childs = count;

    builder->num_debug_infos++;
    builder->current_index++;

    return builder->current_index;
}

uint32_t ShaderBuilderAddUniform(ShaderBuilder *builder, ShaderStructConstr *struct_arr, uint32_t count, char *name, uint32_t location, uint32_t binding){

    uint32_t res = ShaderBuilderAddStruct(builder, struct_arr, count, name);

    builder->decors[builder->num_decorations].indx = res;
    builder->decors[builder->num_decorations].str_point = struct_arr;
    builder->decors[builder->num_decorations].str_size = count;
    builder->num_decorations++;

    uint32_t orig_indx = res;
    {
        uint32_t arr[] = { res };
        res = ShaderBuilderAddVariableF(builder, SHADER_VARIABLE_TYPE_POINTER, SHADER_DATA_FLAG_UNIFORM, arr, 1, NULL, 0);
    }

    {
        uint32_t arr[] = { res };
        res = ShaderBuilderAddVariableF(builder, SHADER_VARIABLE_TYPE_VARIABLE, SHADER_DATA_FLAG_UNIFORM, arr, 1, NULL, 0);
    }


    builder->ioData[builder->num_io_data].type = SHADER_VARIABLE_TYPE_UNIFORM;
    builder->ioData[builder->num_io_data].indx = res;
    builder->ioData[builder->num_io_data].orig_indx = orig_indx;
    builder->ioData[builder->num_io_data].location = location;
    builder->ioData[builder->num_io_data].binding = binding;
    builder->num_io_data ++;

    builder->decors[builder->num_decorations].indx = res;
    builder->num_decorations++;

    return res;
}

uint32_t ShaderBuilderCheckPointer(ShaderBuilder *builder, uint32_t type_indx, uint32_t flags){

    ShaderVariable *variable = NULL;

    ChildStack *child = builder->alloc_head;

    int find = 0;
    while(child != NULL){
        variable = child->node;

        if(variable != NULL){
            if(variable->type == SHADER_VARIABLE_TYPE_POINTER && variable->args[0] == type_indx && (variable->flags & flags))
                return variable->indx;
        }

        child = child->next;
    }
    
    return 0;
}

uint32_t ShaderBuilderAddPointer(ShaderBuilder *builder, ShaderVariableType point_type, uint32_t size, ShaderDataFlags flags){

    uint32_t arr[] = { size };
    uint32_t type_indx = ShaderBuilderCheckVariable(builder, point_type, arr, 1);

    switch(point_type){
        case SHADER_VARIABLE_TYPE_INT:
            type_indx = ShaderBuilderAddInt(builder, 1);
            break;
        case SHADER_VARIABLE_TYPE_FLOAT:
            type_indx = ShaderBuilderAddFloat(builder);
            break;
        case SHADER_VARIABLE_TYPE_VECTOR:
            type_indx = ShaderBuilderAddVector(builder, size, NULL);
            break;
        case SHADER_VARIABLE_TYPE_IMAGE:
            type_indx = ShaderBuilderAddImage(builder);
            break;
    }

    uint32_t res = ShaderBuilderCheckPointer(builder, type_indx, flags);

    if(!res){
        uint32_t arr[] = { type_indx };
        res = ShaderBuilderAddVariable(builder, SHADER_VARIABLE_TYPE_POINTER, flags, arr, 1, NULL, 0);
    }

    return res;
}

uint32_t ShaderBuilderAddIOData(ShaderBuilder *builder, ShaderVariableType type, ShaderDataFlags flags, ShaderStructConstr *struct_arr, uint32_t size, char *name, uint32_t location, uint32_t binding){

    uint32_t res = 0;

    switch(type){
        case SHADER_VARIABLE_TYPE_VECTOR:
            res = ShaderBuilderAddVector(builder, size, NULL );
            break;
        case SHADER_VARIABLE_TYPE_STRUCT:
            res = ShaderBuilderAddStruct(builder, struct_arr, size, name);

            builder->decors[builder->num_decorations].indx = res;
            builder->decors[builder->num_decorations].str_point = struct_arr;
            builder->decors[builder->num_decorations].str_size = size;
            builder->num_decorations++;
            break;
        case SHADER_VARIABLE_TYPE_IMAGE:
            res = ShaderBuilderAddImage(builder);
            break;
    }

    uint32_t orig_indx = res;

    {
        uint32_t arr[] = { res };
        res = ShaderBuilderAddVariableF(builder, SHADER_VARIABLE_TYPE_POINTER, flags, arr, 1, NULL, 0);
    }

    {
        uint32_t arr[] = { res };
        res = ShaderBuilderAddVariableF(builder, SHADER_VARIABLE_TYPE_VARIABLE, flags, arr, 1, NULL, 0);
    }


    if(type != SHADER_VARIABLE_TYPE_IMAGE){
        if(type != SHADER_VARIABLE_TYPE_STRUCT){
            uint32_t len = strlen(name);

            memcpy(builder->infos[builder->num_debug_infos].name, name, len);
            builder->infos[builder->num_debug_infos].indx = res;

            builder->num_debug_infos++;
        }else{

            memset(builder->infos[builder->num_debug_infos].name, 0, 64);
            builder->infos[builder->num_debug_infos].indx = res;

            builder->num_debug_infos++;
        }
    }

    ShaderDataFlags tflags = 0;

    builder->ioData[builder->num_io_data].type = type == SHADER_VARIABLE_TYPE_IMAGE ? SHADER_VARIABLE_TYPE_UNIFORM_CONSTANT : type;
    builder->ioData[builder->num_io_data].indx = res;
    builder->ioData[builder->num_io_data].orig_indx = orig_indx;
    builder->ioData[builder->num_io_data].location = location;
    builder->ioData[builder->num_io_data].binding = binding;
    builder->ioData[builder->num_io_data].flags =  flags & SHADER_DATA_FLAG_SYSTEM ? SHADER_DATA_FLAG_SYSTEM : 0;

    if(type == SHADER_VARIABLE_TYPE_IMAGE){
        builder->ioData[builder->num_io_data].descr_set = builder->curr_descr_set;
        builder->curr_descr_set ++;
    }
    builder->num_io_data ++;

    return res;

}

InputOutputData *ShaderBuilderFindIOData(ShaderBuilder *builder, uint32_t indx){

    InputOutputData *res = NULL;

    for(int i=0;i < builder->num_io_data;i++){
        if(builder->ioData[i].indx == indx)
        {
            res = &builder->ioData[i];
            break;
        }
    }

    return res;

}

uint32_t ShaderBuilderAddOperand(ShaderBuilder *builder, ShaderLabel *label, uint32_t *indexes, uint32_t count, ShaderOperandType operand){

    ShaderOperand *oper = &label->operands[label->num_operands];

    memcpy(oper->var_indx, indexes, sizeof(uint32_t) * count);

    oper->num_vars = count;
    oper->op_type = operand;
    oper->indx = builder->current_index + 1;

    label->num_operands++;
    builder->current_index++;

    return builder->current_index;
}

int ShaderBuilderFindOperand( ShaderLabel *label, ShaderOperandType op_type){

    for(int i=0;i < label->num_operands;i++){
        if(label->operands[i].op_type == op_type){
            return i;
        }
    }

    return -1;
}

void ShaderBuilderStoreValue(ShaderBuilder *builder, ShaderLabel *label, uint32_t *arr, uint32_t size){

    ShaderOperand *oper = &label->operands[label->num_operands];

    memcpy(oper->var_indx, arr, size * sizeof(uint32_t));

    oper->num_vars = size;
    oper->op_type = SHADER_OPERAND_TYPE_STORE;

    label->num_operands++;

}

uint32_t ShaderBuilderAcceptAccess(ShaderBuilder *builder, ShaderLabel *label, uint32_t val_indx){

    uint32_t res = 0;

    for(int i=0;i < label->num_operands;i++){
        if(label->operands[i].op_type == SHADER_OPERAND_TYPE_ACCESS && label->operands[i].var_indx[0] == val_indx)
            res =label->operands[i].indx ;
            break;
    }

    if(!res)
        ShaderBuilderAddOperand(builder, label, &val_indx, 1,  SHADER_OPERAND_TYPE_ACCESS);

    return res;
}

ShaderVariable *ShaderBuilderCheckType(ShaderBuilder *builder, uint32_t val, uint32_t indx){
    ShaderVariable *variable = ShaderBuilderFindVar(builder, val);
    
    InputOutputData *data = ShaderBuilderFindIOData(builder, val);
    if(data != NULL){
        ShaderVariable *var = ShaderBuilderFindVar(builder, data->orig_indx);

        
        if(var->type == SHADER_VARIABLE_TYPE_STRUCT){
            ShaderVariable *res_var = ShaderBuilderFindVar(builder, &var->args[indx]);

            return res_var;
        }else{
            return var;
        }

    }else{
        return variable;
    }
}

uint32_t ShaderBuilderAcceptLoadL(ShaderBuilder *builder, ShaderLabel *label, uint32_t val_indx, uint32_t struct_indx){
    
    uint32_t res = 0;
    
    ShaderVariable *variable = ShaderBuilderFindVar(builder, val_indx);
    
    InputOutputData *data = ShaderBuilderFindIOData(builder, val_indx);
    if(data != NULL){

        ShaderVariable *var = ShaderBuilderFindVar(builder, data->orig_indx);


        if(var->type == SHADER_VARIABLE_TYPE_STRUCT){
            res = ShaderBuilderAddVariable(builder, SHADER_VARIABLE_TYPE_POINTER, SHADER_DATA_FLAG_UNIFORM, &var->args[struct_indx], 1, 0, 0);

            uint32_t cnst = ShaderBuilderAddConstant(builder, SHADER_VARIABLE_TYPE_INT, 0, struct_indx, 1);

            uint32_t arr[] = {res, data->indx, cnst};
            res = ShaderBuilderAddOperand(builder, label, arr, 3, SHADER_OPERAND_TYPE_ACCESS);

            uint32_t arr2[] = {var->args[struct_indx], res};
            res = ShaderBuilderAddOperand(builder, label, arr2, 2, SHADER_OPERAND_TYPE_LOAD);


        }else{
            uint32_t arr[] = {data->orig_indx, val_indx};
            res = ShaderBuilderAddOperand(builder, label, arr, 2, SHADER_OPERAND_TYPE_LOAD);
        }
        
    }else{

        if(variable == NULL)
            return val_indx;

        uint32_t res_type = 0;

        switch (variable->type)
        {
            case  SHADER_VARIABLE_TYPE_VECTOR:
                res_type = ShaderBuilderAddVector(builder, variable->values[0], NULL );
                break;
            
            default:
                break;
        }
        
        uint32_t arr[] = {res_type, val_indx};
        res = ShaderBuilderAddOperand(builder, label, arr, 2, SHADER_OPERAND_TYPE_LOAD);

    }

    return res;
}

uint32_t ShaderBuilderGetType(ShaderBuilder *builder, ShaderVariableType type, uint32_t size){

    uint32_t res_type = 0;

    switch (type) {
        case SHADER_VARIABLE_TYPE_INT:
            res_type = ShaderBuilderAddInt(builder, 1);
            break;
        case SHADER_VARIABLE_TYPE_FLOAT:
            res_type = ShaderBuilderAddFloat(builder);
            break;
        case SHADER_VARIABLE_TYPE_VECTOR:
            res_type = ShaderBuilderAddVector(builder, size, NULL);
            break;
    }

    return res_type;
}

uint32_t ShaderBuilderAcceptLoad(ShaderBuilder *builder, ShaderLabel *label, ShaderVariableType load_type, uint32_t val_indx, uint32_t size, uint32_t iter){

    uint32_t res_type= 0, res = 0;

    switch (load_type) {
        case SHADER_VARIABLE_TYPE_INT:
            res_type = ShaderBuilderAddInt(builder, 1);
            break;
        case SHADER_VARIABLE_TYPE_FLOAT:
            res_type = ShaderBuilderAddFloat(builder);
            break;
        case SHADER_VARIABLE_TYPE_VECTOR:
            res_type = ShaderBuilderAddVector(builder, size, NULL);
            break;
    }

    ShaderVariable * variable = ShaderBuilderFindVar(builder, val_indx);

    if(variable == NULL)
        return 0;

    if(!(variable->flags & SHADER_DATA_FLAG_UNIFORM)){
        InputOutputData *data = ShaderBuilderFindIOData(builder, val_indx);


        ShaderVariable *var_orig = ShaderBuilderFindVar(builder, data->orig_indx);

        if(var_orig->type == SHADER_VARIABLE_TYPE_STRUCT){

            uint32_t cnst = ShaderBuilderAddConstant(builder, SHADER_VARIABLE_TYPE_INT, 0, iter, 1);

            uint32_t arr[] = {res_type, val_indx, cnst};
            ShaderBuilderAddOperand(builder, label, arr, 3,  SHADER_OPERAND_TYPE_ACCESS);
        }
    }else{
        uint32_t cnst = ShaderBuilderAddConstant(builder, SHADER_VARIABLE_TYPE_INT, 0, iter, 1);
        uint32_t point = ShaderBuilderAddPointer(builder, SHADER_VARIABLE_TYPE_VECTOR, 2, SHADER_DATA_FLAG_UNIFORM);

        uint32_t arr[] = {point, val_indx, cnst};
        res = ShaderBuilderAddOperand(builder, label, arr, 3,  SHADER_OPERAND_TYPE_ACCESS);
    }


    uint32_t arr[] = {res_type, variable->flags & SHADER_DATA_FLAG_UNIFORM ? res : val_indx};
    res = ShaderBuilderAddOperand(builder, label, arr, 2, SHADER_OPERAND_TYPE_LOAD);

    return res;
}

uint32_t ShaderBuilderLoad(ShaderBuilder *builder, ShaderLabel *label, uint32_t type, uint32_t val_indx){

    uint32_t res = 0;

    InputOutputData *data = ShaderBuilderFindIOData(builder, val_indx);

    if(data != NULL){
        ShaderVariable *variable = ShaderBuilderFindVar(builder, data->orig_indx);
        
        uint32_t var_type = ShaderBuilderGetType(builder, variable->type, variable->values[0]);
        
        if(!(variable->flags & SHADER_DATA_FLAG_UNIFORM)){
            uint32_t arr[] = {var_type, val_indx};
            res = ShaderBuilderAddOperand(builder, label, arr, 2, SHADER_OPERAND_TYPE_LOAD);
        }

    }else{

        uint32_t arr[] = {type, val_indx};
        res = ShaderBuilderAddOperand(builder, label, arr, 2, SHADER_OPERAND_TYPE_LOAD);
    }

    return res;
}

uint32_t ShaderBuilderAddFuncMove(ShaderBuilder *builder, ShaderLabel *label, uint32_t src_indx, uint32_t src_size, uint32_t dest_indx, uint32_t dest_size){

    uint32_t res = ShaderBuilderAcceptLoadL(builder, label, src_indx, 0);

    if(src_size != dest_size)
    {       
        uint32_t res_type = ShaderBuilderAddVector(builder, 4, NULL);

        uint32_t arr_extract[src_size]; 

        uint32_t float_point = ShaderBuilderAddFloat(builder);

        for(int i=0;i < src_size;i++){
            uint32_t arr[] = {float_point, res,  i};
            arr_extract[i] = ShaderBuilderAddOperand(builder, label, arr, sizeof(arr), SHADER_OPERAND_TYPE_COMPOSITE_EXTRACT);
        }
        
        float v_f = 1.0f;
        uint32_t v_u = 0;
        memcpy(&v_u, &v_f, sizeof(uint32_t));        
        uint32_t cnst2 = ShaderBuilderAddConstant(builder, SHADER_VARIABLE_TYPE_FLOAT, 0, v_u, 1);
        uint32_t arr[] = {res_type, arr_extract[0], arr_extract[1], arr_extract[2], cnst2};
        res = ShaderBuilderAddOperand(builder, label, arr, 5, SHADER_OPERAND_TYPE_COMPOSITE_CONSTRUCT);
    }

    uint32_t arr[] = {dest_indx, res};
    
    ShaderBuilderStoreValue(builder, label, arr, sizeof(arr));

    return res;
}

uint32_t ShaderBuilderGetTexture(ShaderBuilder *builder, ShaderLabel *label, uint32_t texture_indx, uint32_t uv_indx){

    uint32_t res = ShaderBuilderAcceptLoadL(builder, label, texture_indx, 0);
    
    uint32_t res2 = ShaderBuilderAcceptLoadL(builder, label, uv_indx, 0);

    uint32_t res_type = ShaderBuilderAddVector(builder, 4, NULL);

    uint32_t arr[] = {res_type, res, res2};

    res = ShaderBuilderAddOperand(builder, label, arr, sizeof(arr), SHADER_OPERAND_TYPE_IMAGE_SAMLE_IMPLICIT_LOD);

    return res;
}

VectorExtract ShaderBuilderGetElemenets(ShaderBuilder *builder, ShaderLabel *label, uint32_t src_index, uint32_t start_indx, uint32_t size){

    uint32_t res = 0;   

    VectorExtract extract;
    memset(&extract, 0, sizeof(VectorExtract)); 

    uint32_t float_point = ShaderBuilderAddFloat(builder);  

    for(int i=0;i < size;i++){
        uint32_t arr[] = {float_point, src_index,  i + start_indx};
        extract.elems[i] = ShaderBuilderAddOperand(builder, label, arr, 3, SHADER_OPERAND_TYPE_COMPOSITE_EXTRACT);
    }   

    extract.size = size;

    return extract;
}

uint32_t ShaderBuilderCompositeConstruct(ShaderBuilder *builder, ShaderLabel *label, uint32_t *arr_arg, uint32_t size_arr){
        
    uint32_t res = ShaderBuilderAddOperand(builder, label, arr_arg, size_arr, SHADER_OPERAND_TYPE_COMPOSITE_CONSTRUCT);

    return res;
}

uint32_t ShaderBuilderAddFuncSetTexure(ShaderBuilder *builder, ShaderLabel *label, uint32_t texture_indx, uint32_t uv_indx, uint32_t dest_indx, uint32_t dest_size){
    
    uint32_t res = ShaderBuilderAcceptLoadL(builder, label, texture_indx, 0);
    
    uint32_t res2 = ShaderBuilderAcceptLoadL(builder, label, uv_indx, 0);

    uint32_t res_type = ShaderBuilderAddVector(builder, 4, NULL);

    uint32_t arr[] = {res_type, res, res2};

    res = ShaderBuilderAddOperand(builder, label, arr, sizeof(arr), SHADER_OPERAND_TYPE_IMAGE_SAMLE_IMPLICIT_LOD);;
        
    uint32_t arr2[] = {dest_indx, res};

    ShaderBuilderStoreValue(builder, label, arr2, sizeof(arr2));

    return res;
}

uint32_t ShaderBuilderAddFuncMult(ShaderBuilder *builder, ShaderLabel *label, uint32_t val_1, uint32_t indx_1, uint32_t type_1, uint32_t size_1,  uint32_t val_2, uint32_t indx_2, uint32_t type_2, uint32_t size_2, uint32_t res_size){

    uint32_t res = 0;

    uint32_t acc_1 = ShaderBuilderAcceptLoadL(builder, label, val_1, indx_1);
    uint32_t acc_2 = ShaderBuilderAcceptLoadL(builder, label, val_2, indx_2);

    if(type_1 == SHADER_VARIABLE_TYPE_MATRIX && type_2 == SHADER_VARIABLE_TYPE_MATRIX){
        uint32_t mat_type = ShaderBuilderAddMatrix(builder, size_1, NULL);
        uint32_t vec_type = ShaderBuilderAddVector(builder, size_2, NULL);

        uint32_t arr[] = { mat_type, acc_1, acc_2 };
        res = ShaderBuilderAddOperand(builder, label, arr, 3, SHADER_OPERAND_TYPE_MATTIMEMAT);
    }else if(type_1 == SHADER_VARIABLE_TYPE_MATRIX && type_2 == SHADER_VARIABLE_TYPE_VECTOR){
        uint32_t mat_type = ShaderBuilderAddMatrix(builder, size_1, NULL);
        uint32_t vec_type = ShaderBuilderAddVector(builder, size_2, NULL);
        uint32_t res_vec_type = ShaderBuilderAddVector(builder, res_size, NULL);

        float v_f = 1.0f;
        uint32_t v_u = 0;
        memcpy(&v_u, &v_f, sizeof(uint32_t));        
        uint32_t cnst = ShaderBuilderAddConstant(builder, SHADER_VARIABLE_TYPE_FLOAT, 0, v_u, 1);
        uint32_t cnst2 = ShaderBuilderAddConstant(builder, SHADER_VARIABLE_TYPE_FLOAT, 0, 0, 1);

        if(size_2 == 2){
                        
            VectorExtract extr = ShaderBuilderGetElemenets(builder, label, acc_2, 0, size_2);

            uint32_t arr[] = { res_vec_type, extr.elems[0], extr.elems[1], cnst2, cnst};
            res = ShaderBuilderCompositeConstruct(builder, label, arr, 5);

        }else if(size_2 == 3){

            VectorExtract extr = ShaderBuilderGetElemenets(builder, label, acc_2, 0, size_2);

            uint32_t arr[] = { res_vec_type, extr.elems[0], extr.elems[1], extr.elems[2], cnst};
            res = ShaderBuilderCompositeConstruct(builder, label, arr, 5);
        }else{
            uint32_t arr[] = { res_vec_type, acc_1, acc_2 };
            res = ShaderBuilderAddOperand(builder, label, arr, 3, SHADER_OPERAND_TYPE_MATTIMEVEC);

            return res;
        }
        
        
        uint32_t arr[] = { res_vec_type, acc_1, res };
        res = ShaderBuilderAddOperand(builder, label, arr, 3, SHADER_OPERAND_TYPE_MATTIMEVEC);

    }else if(type_1 == SHADER_VARIABLE_TYPE_VECTOR && type_2 == SHADER_VARIABLE_TYPE_VECTOR){

        uint32_t res_vec_type = ShaderBuilderAddVector(builder, res_size, NULL);

        uint32_t arr[] = { res_vec_type, acc_1, acc_2 };
        res = ShaderBuilderAddOperand(builder, label, arr, 3, SHADER_OPERAND_TYPE_MUL);
    }


    return res;
}

uint32_t ShaderBuilderAddFuncMultS(ShaderBuilder *builder, ShaderLabel *label, uint32_t val_indx, uint32_t single_indx, uint32_t size){

    uint32_t res = 0;

    uint32_t func_indx = ShaderBuilderAddPointer(builder, SHADER_VARIABLE_TYPE_FLOAT, 0, SHADER_DATA_FLAG_FUNCTION);

    uint32_t vec3_indx = ShaderBuilderAddVector(builder, 3, NULL);

    uint32_t arr[] = {func_indx, 7 /*Function*/};
    uint32_t variable = ShaderBuilderAddOperand(builder, label, arr, 2, SHADER_OPERAND_TYPE_VARIABLE);

    VectorExtract vector = ShaderBuilderGetElemenets(builder, label, single_indx, 0, 1);

    uint32_t arr2[] = { variable, vector.elems[0] };
    ShaderBuilderStoreValue(builder, label, arr2, 2);

    uint32_t var_type = ShaderBuilderGetType(builder, SHADER_VARIABLE_TYPE_FLOAT, 0);

    uint32_t val1 = ShaderBuilderLoad(builder, label, vec3_indx, val_indx);
    uint32_t val2 = ShaderBuilderLoad(builder, label, var_type, variable);

    uint32_t arr3[] = { vec3_indx, val1, val2};
    res = ShaderBuilderAddOperand(builder, label, arr3, 3, SHADER_OPERAND_TYPE_VECTIMES);
    
    return res;
}

uint32_t ShaderBuilderAddFuncSetColor4(ShaderBuilder *builder, ShaderLabel *label, uint32_t val_indx, uint32_t single_indx, uint32_t size){

    uint32_t res = 0;

    uint32_t func_indx = ShaderBuilderAddPointer(builder, SHADER_VARIABLE_TYPE_FLOAT, 0, SHADER_DATA_FLAG_FUNCTION);

    uint32_t arr[] = {func_indx, 7 /*Function*/};
    uint32_t variable = ShaderBuilderAddOperand(builder, label, arr, 2, SHADER_OPERAND_TYPE_VARIABLE);

    VectorExtract vector = ShaderBuilderGetElemenets(builder, label, single_indx, 0, 1);

    uint32_t arr2[] = { variable, vector.elems[0] };
    ShaderBuilderStoreValue(builder, label, arr2, 2);

    uint32_t var_type = ShaderBuilderGetType(builder, SHADER_VARIABLE_TYPE_FLOAT, 0);

    uint32_t vec3_indx = ShaderBuilderAddVector(builder, 3, NULL);
    uint32_t vec4_indx = ShaderBuilderAddVector(builder, 4, NULL);
    
    uint32_t val1 = ShaderBuilderLoad(builder, label, vec3_indx, val_indx);
    uint32_t val2 = ShaderBuilderLoad(builder, label, var_type, variable);
    
    vector = ShaderBuilderGetElemenets(builder, label, val1, 0, 3);
    
    uint32_t arr5[] = { vec4_indx, vector.elems[0], vector.elems[1], vector.elems[2], val2};
    res = ShaderBuilderCompositeConstruct(builder, label, arr5, 5);

    return res;
}

int ShaderBuilderAddFuncMoveToGL(ShaderBuilder *builder, ShaderLabel *label, uint32_t vec_val, uint32_t vec_size, uint32_t res_store){
    
    if(vec_size != 4){
        uint32_t vec2_indx = ShaderBuilderAddVector(builder, vec_size, NULL);
        uint32_t vec4_indx = ShaderBuilderAddVector(builder, 4, NULL);

        uint32_t val1 = ShaderBuilderLoad(builder, label, vec2_indx, vec_val);

        VectorExtract vector = ShaderBuilderGetElemenets(builder, label, val1, 0, 2);
        
        float v_f = 1.0f;
        uint32_t v_u = 0;
        memcpy(&v_u, &v_f, sizeof(uint32_t));
        uint32_t cnst1 = ShaderBuilderAddConstant(builder, SHADER_VARIABLE_TYPE_FLOAT, 0, v_u, 1);    
        uint32_t cnst2 = ShaderBuilderAddConstant(builder, SHADER_VARIABLE_TYPE_FLOAT, 0, 0, 1);

        uint32_t arr5[] = { vec4_indx, vector.elems[0], vector.elems[1], cnst2, cnst1};
        uint32_t res = ShaderBuilderCompositeConstruct(builder, label, arr5, 5);
        
        uint32_t n_res;
        {
            uint32_t cnst = ShaderBuilderAddConstant(builder, SHADER_VARIABLE_TYPE_INT, 0, 0, 1);
            uint32_t point = ShaderBuilderAddPointer(builder, SHADER_VARIABLE_TYPE_VECTOR, 4, SHADER_DATA_FLAG_OUTPUT);
            uint32_t arr[] = {point, res_store, cnst};
            n_res = ShaderBuilderAddOperand(builder, label, arr, 3, SHADER_OPERAND_TYPE_ACCESS);
        }

        uint32_t arr[] = {n_res, res};
        
        ShaderBuilderStoreValue(builder, label, arr, sizeof(arr));
    }else{

        uint32_t n_res;
        {
            uint32_t cnst = ShaderBuilderAddConstant(builder, SHADER_VARIABLE_TYPE_INT, 0, 0, 1);
            uint32_t point = ShaderBuilderAddPointer(builder, SHADER_VARIABLE_TYPE_VECTOR, 4, SHADER_DATA_FLAG_OUTPUT);
            uint32_t arr[] = {point, res_store, cnst};
            n_res = ShaderBuilderAddOperand(builder, label, arr, 3, SHADER_OPERAND_TYPE_ACCESS);
        }

        uint32_t arr[] = {n_res, vec_val};
        
        ShaderBuilderStoreValue(builder, label, arr, sizeof(arr));
    }
}

int ShaderBuilderAddFuncAdd(ShaderBuilder *builder, ShaderLabel *label, uint32_t val_1, uint32_t val_2, uint32_t size, uint32_t res_store){

    uint32_t val_indx_1 = ShaderBuilderAcceptLoad(builder, label, SHADER_VARIABLE_TYPE_VECTOR, val_1, size, 0);
    uint32_t val_indx_2 = ShaderBuilderAcceptLoad(builder, label, SHADER_VARIABLE_TYPE_VECTOR, val_2, size, 0);
    uint32_t val_indx_3 = ShaderBuilderAcceptLoad(builder, label, SHADER_VARIABLE_TYPE_VECTOR, val_1, size, 2);

    uint32_t type = ShaderBuilderAddVector(builder, size, NULL);

    uint32_t res = 0;
    {
        uint32_t arr[] = {type, val_indx_2, val_indx_3};
        res = ShaderBuilderAddOperand(builder, label, arr, 3, SHADER_OPERAND_TYPE_MUL);
    }

    {
        uint32_t arr[] = {type, val_indx_1, res};
        res = ShaderBuilderAddOperand(builder, label, arr, 3, SHADER_OPERAND_TYPE_ADD);
    }

    uint32_t res_type = ShaderBuilderAddVector(builder, 4, NULL);

    uint32_t arr_extract[size];
    {
        uint32_t float_point = ShaderBuilderAddFloat(builder);

        for(int i=0;i < size;i++){
            uint32_t arr[] = {float_point, res,  i};
            arr_extract[i] = ShaderBuilderAddOperand(builder, label, arr, 3, SHADER_OPERAND_TYPE_COMPOSITE_EXTRACT);
        }
    }

    {
        uint32_t cnst = ShaderBuilderAddConstant(builder, SHADER_VARIABLE_TYPE_FLOAT, 0, 0, 1);
        float v_f = 1.0f;
        uint32_t v_u = 0;
        memcpy(&v_u, &v_f, sizeof(uint32_t));
        uint32_t cnst2 = ShaderBuilderAddConstant(builder, SHADER_VARIABLE_TYPE_FLOAT, 0, v_u, 1);
        uint32_t arr[] = {res_type, arr_extract[0], arr_extract[1], cnst, cnst2};
        res = ShaderBuilderAddOperand(builder, label, arr, 5, SHADER_OPERAND_TYPE_COMPOSITE_CONSTRUCT);
    }

    uint32_t n_res;
    {
        uint32_t cnst = ShaderBuilderAddConstant(builder, SHADER_VARIABLE_TYPE_INT, 0, 0, 1);
        uint32_t point = ShaderBuilderAddPointer(builder, SHADER_VARIABLE_TYPE_VECTOR, 4, SHADER_DATA_FLAG_OUTPUT);
        uint32_t arr[] = {point, res_store, cnst};
        n_res = ShaderBuilderAddOperand(builder, label, arr, 3, SHADER_OPERAND_TYPE_ACCESS);
    }

    uint32_t arr[] = {n_res, res};
    
    ShaderBuilderStoreValue(builder, label, arr, sizeof(arr));

    return res;
}

ShaderLabel *ShaderBuilderAddLabel(ShaderBuilder *builder, uint32_t func_indx){

    uint32_t num_labels = builder->functions[func_indx].num_labels;

    builder->functions[func_indx].labels[num_labels].index =  builder->current_index + 1;

    builder->functions[func_indx].num_labels++;
    builder->current_index++;

    return &builder->functions[func_indx].labels[num_labels - 1];
}

ShaderFunc *ShaderBuilderAddFunction(ShaderBuilder *builder, ShaderVariableType output, char *name){

    uint32_t void_type, func_type;

    int res = ShaderBuilderCheckVariable(builder, SHADER_VARIABLE_TYPE_FUNCTION, NULL, 0);

    if(!res){

        res = ShaderBuilderCheckVariable(builder, output,  NULL, 0);

        if(!res)
            res = ShaderBuilderAddVariable(builder, output, 0, NULL, 0, NULL, 0);

        void_type = res;

        uint32_t arr[] = {res};

        res = ShaderBuilderAddVariable(builder, SHADER_VARIABLE_TYPE_FUNCTION, 0, arr, 1, NULL, 0);
    }

    func_type = res;

    builder->functions[builder->num_functions].result_type_indx = void_type;
    builder->functions[builder->num_functions].func_type_indx = func_type;
    builder->functions[builder->num_functions].indx = builder->current_index + 1;

    uint32_t len = strlen(name);

    memcpy(builder->infos[builder->num_debug_infos].name, name, len);
    builder->infos[builder->num_debug_infos].indx = builder->current_index + 1;

    builder->num_debug_infos++;
    builder->num_functions++;
    builder->current_index++;

    ShaderBuilderAddLabel(builder, builder->num_functions - 1 );

    return &builder->functions[builder->num_functions - 1];

}

void ShaderBuilderInit(ShaderBuilder *builder, ShaderType type){

    memset(builder, 0, sizeof(ShaderBuilder));

    builder->alloc_head = calloc(1, sizeof(ChildStack));

    ShaderBuilderAddValue(builder, SpvMagicNumber);// Magic number
    ShaderBuilderAddValue(builder, 0x00010000);//SpvVersion);
    ShaderBuilderAddValue(builder, 0x000D000B);//Generator version
    ShaderBuilderAddValue(builder, 0);//Lines code
    ShaderBuilderAddValue(builder, 0);

    ShaderBuilderAddOp(builder, SpvOpCapability, 2);
    ShaderBuilderAddValue(builder, SpvCapabilityShader);

    builder->type = type;

    ShaderBuilderAddVariable(builder, SHADER_VARIABLE_TYPE_EXTENDED_IMPORT, 0, NULL, 0, NULL, 0);

    builder->main_point_index = ShaderBuilderAddFunction(builder, SHADER_VARIABLE_TYPE_VOID, "main");

    if(type == SHADER_TYPE_VERTEX){
        ShaderStructConstr struct_arr[] = {
            {SHADER_VARIABLE_TYPE_VECTOR, 4, 0, "gl_Position"},
            {SHADER_VARIABLE_TYPE_FLOAT, 32, 0, "gl_PointSize"},
            {SHADER_VARIABLE_TYPE_ARRAY, 1,  SHADER_VARIABLE_TYPE_FLOAT, "gl_ClipDistance"},
            {SHADER_VARIABLE_TYPE_ARRAY, 1,  SHADER_VARIABLE_TYPE_FLOAT, "gl_CullDistance"}
        };

        builder->gl_struct_indx = ShaderBuilderAddIOData(builder, SHADER_VARIABLE_TYPE_STRUCT, SHADER_DATA_FLAG_OUTPUT | SHADER_DATA_FLAG_SYSTEM, struct_arr, 4, "gl_PerVertex", 0, 0);
    }

}


void ShaderBuilderWriteFuncType(ShaderBuilder *builder, ShaderVariable *variable){

    ShaderBuilderAddOp(builder, SpvOpTypeFunction, 3);

    ShaderBuilderAddValue(builder, variable->indx);

    for(int i=0;i < variable->num_args;i++)
        ShaderBuilderAddValue(builder, variable->args[i]);

}

void ShaderBuilderMake(ShaderBuilder *builder){

    ShaderBuilderAddOp(builder, SpvOpExtInstImport, 6);
    ShaderBuilderAddValue(builder, ((ShaderVariable *)builder->alloc_head->node)->indx);
    ShaderBuilderAddString(builder, "GLSL.std.450");
    ShaderBuilderAddValue(builder, 0x0);

    ShaderBuilderAddOp(builder, SpvOpMemoryModel, 3);
    ShaderBuilderAddValue(builder, SpvAddressingModelLogical);
    ShaderBuilderAddValue(builder, SpvMemoryModelGLSL450);

    uint32_t count = 0;

    for(int i=0;i < builder->num_io_data;i++)
        if(builder->ioData[i].type != SHADER_VARIABLE_TYPE_UNIFORM && builder->ioData[i].type != SHADER_VARIABLE_TYPE_UNIFORM_CONSTANT)
            count++;

    {
        ShaderBuilderAddOp(builder, SpvOpEntryPoint, 4 + count + 1);

        switch(builder->type)
        {
            case SHADER_TYPE_FRAGMENT:
                ShaderBuilderAddValue(builder, SpvExecutionModelFragment);
                break;
            case SHADER_TYPE_VERTEX:
                ShaderBuilderAddValue(builder, SpvExecutionModelVertex);
                break;
            case SHADER_TYPE_COMPUTED:
                ShaderBuilderAddValue(builder, SpvExecutionModelGLCompute);
                break;
            case SHADER_TYPE_GEOMETRY:
                ShaderBuilderAddValue(builder, SpvExecutionModelGeometry);
                break;
            case SHADER_TYPE_TESELLATION_CONTROL:
                ShaderBuilderAddValue(builder, SpvExecutionModelTessellationControl);
                break;
            case SHADER_TYPE_TESELLATION_EVALUATION:
                ShaderBuilderAddValue(builder, SpvExecutionModelTessellationEvaluation);
                break;
        }

        ShaderBuilderAddValue(builder, builder->main_point_index->indx);

        ShaderBuilderAddString(builder, builder->infos[0].name);
        ShaderBuilderAddValue(builder, 0x0);

        for(int i=0;i < builder->num_io_data;i++)
            if(builder->ioData[i].type != SHADER_VARIABLE_TYPE_UNIFORM && builder->ioData[i].type != SHADER_VARIABLE_TYPE_UNIFORM_CONSTANT)
                ShaderBuilderAddValue(builder, builder->ioData[i].indx);

    }

    if(builder->type == SHADER_TYPE_FRAGMENT){
        ShaderBuilderAddOp(builder, SpvOpExecutionMode, 3);
        ShaderBuilderAddValue(builder, builder->main_point_index->indx );
        ShaderBuilderAddValue(builder, SpvExecutionModeOriginUpperLeft );
    }

    //Debug informations
    {
        ShaderBuilderAddOp(builder, SpvOpSource, 3);
        ShaderBuilderAddValue(builder, SpvSourceLanguageGLSL );
        ShaderBuilderAddValue(builder, 450 );

        ShaderBuilderAddOp(builder, SpvOpSourceExtension, 10);
        ShaderBuilderAddString(builder, "GL_GOOGLE_cpp_style_line_directive");
        ShaderBuilderAddOp(builder, SpvOpSourceExtension, 8);
        ShaderBuilderAddString(builder, "GL_GOOGLE_include_directive");

        uint32_t orig_len = 0, len =  0, temp = 0;
        ShaderDebugInfo *info;
        for(int i=0;i < builder->num_debug_infos;i++){
            info = &builder->infos[i];

            orig_len = strlen( info->name);

            temp = orig_len;
            while(temp % sizeof(uint32_t))
                temp ++;

            len = temp / sizeof(uint32_t);

            if(orig_len % sizeof(uint32_t))
                len --;

            ShaderBuilderAddOp(builder, SpvOpName, 3 + len);
            ShaderBuilderAddValue(builder, info->indx );
            ShaderBuilderAddString(builder, info->name);

            if(!(orig_len % sizeof(uint32_t)))
                ShaderBuilderAddValue(builder, 0x0);

            for(int j=0;j < info->num_childs;j++){
                orig_len = strlen(info->child_name[j]);

                temp = orig_len;
                while(temp % sizeof(uint32_t))
                    temp ++;

                len = temp / sizeof(uint32_t);

                if(orig_len % sizeof(uint32_t))
                    len --;

                ShaderBuilderAddOp(builder, SpvOpMemberName, 4 + len);
                ShaderBuilderAddValue(builder, info->indx );
                ShaderBuilderAddValue(builder, j);
                ShaderBuilderAddString(builder, info->child_name[j]);

                if(!(orig_len % sizeof(uint32_t)))
                    ShaderBuilderAddValue(builder, 0x0);
            }
        }
    }

    //Decorations
    {

        if(builder->num_decorations > 0){
            uint32_t iter = 0;
            
            uint32_t i_descr = 0;
            if(builder->type == SHADER_TYPE_VERTEX){
                ShaderBuilderAddOp(builder, SpvOpDecorate, 3);
                ShaderBuilderAddValue(builder, builder->decors[i_descr].indx );
                ShaderBuilderAddValue(builder, SpvDecorationBlock );

                for(int i=0;i < 4;i++){
                    ShaderBuilderAddOp(builder, SpvOpMemberDecorate, 5);
                    ShaderBuilderAddValue(builder, builder->decors[i_descr].indx );
                    ShaderBuilderAddValue(builder, i );
                    ShaderBuilderAddValue(builder, SpvDecorationBuiltIn);
                    ShaderBuilderAddValue(builder, iter );

                    iter++;

                    if(iter == 2)
                        iter ++;
                }

                i_descr++;
            }

            for(int i=0;i < builder->num_io_data;i++){

                if(builder->ioData[i].type == SHADER_VARIABLE_TYPE_UNIFORM){
                    
                    ShaderBuilderAddOp(builder, SpvOpDecorate, 3);
                    ShaderBuilderAddValue(builder, builder->decors[i_descr].indx );
                    ShaderBuilderAddValue(builder, SpvDecorationBlock );

                    ShaderStructConstr *str = builder->decors[i_descr].str_point;

                    for(int j =0;j < builder->decors[i_descr].str_size;j++){

                        if(str[j].var_type == SHADER_VARIABLE_TYPE_MATRIX){
                        
                            ShaderBuilderAddOp(builder, SpvOpMemberDecorate, 4);
                            ShaderBuilderAddValue(builder, builder->decors[i_descr].indx );
                            ShaderBuilderAddValue(builder, j );
                            ShaderBuilderAddValue(builder, SpvDecorationColMajor );

                            ShaderBuilderAddOp(builder, SpvOpMemberDecorate, 5);
                            ShaderBuilderAddValue(builder, builder->decors[i_descr].indx );
                            ShaderBuilderAddValue(builder, j );
                            ShaderBuilderAddValue(builder, SpvDecorationOffset );
                            ShaderBuilderAddValue(builder, j * 64 );
                            
                            ShaderBuilderAddOp(builder, SpvOpMemberDecorate, 5);
                            ShaderBuilderAddValue(builder, builder->decors[i_descr].indx );
                            ShaderBuilderAddValue(builder, j );
                            ShaderBuilderAddValue(builder, SpvDecorationMatrixStride );
                            ShaderBuilderAddValue(builder, 16 );
                        }
                        else
                        {
                            ShaderBuilderAddOp(builder, SpvOpMemberDecorate, 5);
                            ShaderBuilderAddValue(builder, builder->decors[i_descr].indx );
                            ShaderBuilderAddValue(builder, j );
                            ShaderBuilderAddValue(builder, SpvDecorationOffset );
                            ShaderBuilderAddValue(builder, j * 8 );
                        }
                        
                    }
                }               

            }

            /*ShaderBuilderAddOp(builder, SpvOpDecorate, 4);
            ShaderBuilderAddValue(builder, builder->decors[2].indx );
            ShaderBuilderAddValue(builder, SpvDecorationDescriptorSet );
            ShaderBuilderAddValue(builder, 0 );
            ShaderBuilderAddOp(builder, SpvOpDecorate, 4);
            ShaderBuilderAddValue(builder, builder->decors[2].indx );
            ShaderBuilderAddValue(builder, SpvDecorationBinding );
            ShaderBuilderAddValue(builder, 0 );*/
        }

        for(int i=0;i < builder->num_io_data;i++){

            if(builder->ioData[i].binding != 0){                    
                ShaderBuilderAddOp(builder, SpvOpDecorate, 4);
                ShaderBuilderAddValue(builder, builder->ioData[i].indx );
                ShaderBuilderAddValue(builder, SpvDecorationBinding );
                ShaderBuilderAddValue(builder, builder->ioData[i].binding - 1 );
                ShaderBuilderAddOp(builder, SpvOpDecorate, 4);
                ShaderBuilderAddValue(builder, builder->ioData[i].indx );
                ShaderBuilderAddValue(builder, SpvDecorationDescriptorSet );
                ShaderBuilderAddValue(builder, builder->ioData[i].descr_set);   
            }else{         

                if(builder->ioData[i].flags & SHADER_DATA_FLAG_SYSTEM)
                    continue;

                ShaderBuilderAddOp(builder, SpvOpDecorate, 4);
                ShaderBuilderAddValue(builder, builder->ioData[i].indx );
                ShaderBuilderAddValue(builder, SpvDecorationLocation );
                ShaderBuilderAddValue(builder, builder->ioData[i].location );
            }
        }
    }

    //Variables
    {

        ShaderVariable *variable;
        ChildStack *child = builder->alloc_head;

        int j=0;
        while(child != NULL)
        {
            variable = child->node;
            j = 0;

            if(variable == NULL){
                child = child->next;
                continue;
            }

            if(variable->indx == 0)
                continue;

            switch(variable->type){
                case SHADER_VARIABLE_TYPE_VOID:
                    ShaderBuilderAddOp(builder, SpvOpTypeVoid, 2);
                    ShaderBuilderAddValue(builder, variable->indx);
                    break;
                case SHADER_VARIABLE_TYPE_FUNCTION:
                    ShaderBuilderWriteFuncType(builder, variable);
                    break;
                case SHADER_VARIABLE_TYPE_INT:
                    ShaderBuilderAddOp(builder, SpvOpTypeInt, 4);
                    ShaderBuilderAddValue(builder, variable->indx);
                    ShaderBuilderAddValue(builder, variable->values[0]);
                    ShaderBuilderAddValue(builder, variable->values[1]);
                    break;
                case SHADER_VARIABLE_TYPE_FLOAT:
                    ShaderBuilderAddOp(builder, SpvOpTypeFloat, 3);
                    ShaderBuilderAddValue(builder, variable->indx);
                    ShaderBuilderAddValue(builder, variable->values[0]);
                    break;
                case SHADER_VARIABLE_TYPE_ARRAY:
                    ShaderBuilderAddOp(builder, SpvOpTypeArray, 4);
                    ShaderBuilderAddValue(builder, variable->indx);
                    ShaderBuilderAddValue(builder, variable->args[0]);
                    ShaderBuilderAddValue(builder, variable->args[1]);
                    break;
                case SHADER_VARIABLE_TYPE_IMAGE:
                    ShaderBuilderAddOp(builder, SpvOpTypeImage, 9);
                    ShaderBuilderAddValue(builder, variable->indx);
                    ShaderBuilderAddValue(builder, variable->args[0]);
                    ShaderBuilderAddValue(builder, variable->values[0]);
                    ShaderBuilderAddValue(builder, variable->values[1]);
                    ShaderBuilderAddValue(builder, variable->values[2]);
                    ShaderBuilderAddValue(builder, variable->values[3]);
                    ShaderBuilderAddValue(builder, variable->values[4]);
                    ShaderBuilderAddValue(builder, variable->values[5]);
                    break;
                case SHADER_VARIABLE_TYPE_SAMPLED_IMAGE:
                    ShaderBuilderAddOp(builder, SpvOpTypeSampledImage, 3);
                    ShaderBuilderAddValue(builder, variable->indx);
                    ShaderBuilderAddValue(builder, variable->args[0]);
                    break;
                case SHADER_VARIABLE_TYPE_CONSTANT:
                    ShaderBuilderAddOp(builder, SpvOpConstant, 4);
                    ShaderBuilderAddValue(builder, variable->args[0]);
                    ShaderBuilderAddValue(builder, variable->indx);
                    ShaderBuilderAddValue(builder, variable->values[0]);
                    break;
                case SHADER_VARIABLE_TYPE_VECTOR:
                    ShaderBuilderAddOp(builder, SpvOpTypeVector, 4);
                    ShaderBuilderAddValue(builder, variable->indx);
                    ShaderBuilderAddValue(builder, variable->args[0]);
                    ShaderBuilderAddValue(builder, variable->values[0]);
                    break;
                case SHADER_VARIABLE_TYPE_MATRIX:
                    ShaderBuilderAddOp(builder, SpvOpTypeMatrix, 4);
                    ShaderBuilderAddValue(builder, variable->indx);
                    ShaderBuilderAddValue(builder, variable->args[0]);
                    ShaderBuilderAddValue(builder, variable->values[0]);
                    break;
                case SHADER_VARIABLE_TYPE_STRUCT:
                    ShaderBuilderAddOp(builder, SpvOpTypeStruct, 2 + variable->num_args);
                    ShaderBuilderAddValue(builder, variable->indx);
                    for(j=0;j < variable->num_args;j++)
                        ShaderBuilderAddValue(builder, variable->args[j]);
                    break;
                case SHADER_VARIABLE_TYPE_POINTER:
                    ShaderBuilderAddOp(builder, SpvOpTypePointer, 4);
                    ShaderBuilderAddValue(builder, variable->indx);
                    ShaderBuilderAddValue(builder, variable->flags & SHADER_DATA_FLAG_UNIFORM ? SpvStorageClassUniform : (variable->flags & SHADER_DATA_FLAG_OUTPUT ? SpvStorageClassOutput : (variable->flags & SHADER_DATA_FLAG_UNIFORM_CONSTANT ? SpvStorageClassUniformConstant : (variable->flags & SHADER_DATA_FLAG_FUNCTION ? SpvStorageClassFunction : SpvStorageClassInput))));
                    ShaderBuilderAddValue(builder, variable->args[0]);
                    break;
                case SHADER_VARIABLE_TYPE_VARIABLE:
                    ShaderBuilderAddOp(builder, SpvOpVariable, 4);
                    ShaderBuilderAddValue(builder, variable->args[0]);
                    ShaderBuilderAddValue(builder, variable->indx);
                    ShaderBuilderAddValue(builder, variable->flags & SHADER_DATA_FLAG_UNIFORM ? SpvStorageClassUniform : (variable->flags & SHADER_DATA_FLAG_OUTPUT ? SpvStorageClassOutput : (variable->flags & SHADER_DATA_FLAG_UNIFORM_CONSTANT ? SpvStorageClassUniformConstant : (variable->flags & SHADER_DATA_FLAG_FUNCTION ? SpvStorageClassFunction : SpvStorageClassInput))));
                    break;
            }

            child = child->next;
        }
    }

    //Functions
    {
        ShaderOperand * operand;
                    
        uint32_t iter = 0;

        for(int i=0;i < builder->num_functions;i++)
        {
            ShaderBuilderAddOp(builder, SpvOpFunction, 5);
            ShaderBuilderAddValue(builder, builder->functions[i].result_type_indx);
            ShaderBuilderAddValue(builder, builder->functions[i].indx);
            ShaderBuilderAddValue(builder, builder->functions[i].function_control);
            ShaderBuilderAddValue(builder, builder->functions[i].func_type_indx);

            for(int j=0;j < builder->functions[i].num_labels;j++)
             {
                ShaderBuilderAddOp(builder, SpvOpLabel, 2);
                ShaderBuilderAddValue(builder, builder->functions[i].labels[j].index);

                for(int l=0;l < builder->functions[i].labels[j].num_operands;l++){
                    operand = &builder->functions[i].labels[j].operands[l];

                    
                    switch(operand->op_type){                         
                        case SHADER_OPERAND_TYPE_VARIABLE:
                            ShaderBuilderAddOp(builder, SpvOpVariable, 4);
                            ShaderBuilderAddValue(builder, operand->var_indx[0]);
                            ShaderBuilderAddValue(builder, operand->indx);
                            ShaderBuilderAddValue(builder, operand->var_indx[1]);
                            break;
                    }
                }

                for(int l=0;l < builder->functions[i].labels[j].num_operands;l++){
                    operand = &builder->functions[i].labels[j].operands[l];

                    switch(operand->op_type){
                        case SHADER_OPERAND_TYPE_ACCESS:
                            ShaderBuilderAddOp(builder, SpvOpAccessChain, 4 + 1);
                            ShaderBuilderAddValue(builder, operand->var_indx[0]);
                            ShaderBuilderAddValue(builder, operand->indx);
                            ShaderBuilderAddValue(builder, operand->var_indx[1]);
                            ShaderBuilderAddValue(builder, operand->var_indx[2]);
                            break;
                        case SHADER_OPERAND_TYPE_COMPOSITE_EXTRACT:
                            ShaderBuilderAddOp(builder, SpvOpCompositeExtract, 4 + 1);
                            ShaderBuilderAddValue(builder, operand->var_indx[0]);
                            ShaderBuilderAddValue(builder, operand->indx);
                            ShaderBuilderAddValue(builder, operand->var_indx[1]);
                            ShaderBuilderAddValue(builder, operand->var_indx[2]);
                            break;
                        case SHADER_OPERAND_TYPE_COMPOSITE_CONSTRUCT:
                            ShaderBuilderAddOp(builder, SpvOpCompositeConstruct, 3 + operand->num_vars - 1);
                            ShaderBuilderAddValue(builder, operand->var_indx[0]);
                            ShaderBuilderAddValue(builder, operand->indx);

                            for(iter = 1;iter < operand->num_vars;iter++)
                                ShaderBuilderAddValue(builder, operand->var_indx[iter]);

                            break;
                        case SHADER_OPERAND_TYPE_IMAGE_SAMLE_IMPLICIT_LOD:
                            ShaderBuilderAddOp(builder, SpvOpImageSampleImplicitLod, 5);
                            ShaderBuilderAddValue(builder, operand->var_indx[0]);
                            ShaderBuilderAddValue(builder, operand->indx);
                            ShaderBuilderAddValue(builder, operand->var_indx[1]);
                            ShaderBuilderAddValue(builder, operand->var_indx[2]);
                            break;
                        case SHADER_OPERAND_TYPE_LOAD:
                            ShaderBuilderAddOp(builder, SpvOpLoad, 4);
                            ShaderBuilderAddValue(builder, operand->var_indx[0]);
                            ShaderBuilderAddValue(builder, operand->indx);
                            ShaderBuilderAddValue(builder, operand->var_indx[1]);
                            break;
                        case SHADER_OPERAND_TYPE_STORE:
                            ShaderBuilderAddOp(builder, SpvOpStore, 3);
                            ShaderBuilderAddValue(builder, operand->var_indx[0]);
                            ShaderBuilderAddValue(builder, operand->var_indx[1]);
                            break;
                        case SHADER_OPERAND_TYPE_ADD:
                            ShaderBuilderAddOp(builder, SpvOpFAdd, 5);
                            ShaderBuilderAddValue(builder, operand->var_indx[0]);
                            ShaderBuilderAddValue(builder, operand->indx);
                            ShaderBuilderAddValue(builder, operand->var_indx[1]);
                            ShaderBuilderAddValue(builder, operand->var_indx[2]);
                            break;
                        case SHADER_OPERAND_TYPE_MUL:
                            ShaderBuilderAddOp(builder, SpvOpFMul, 5);
                            ShaderBuilderAddValue(builder, operand->var_indx[0]);
                            ShaderBuilderAddValue(builder, operand->indx);
                            ShaderBuilderAddValue(builder, operand->var_indx[1]);
                            ShaderBuilderAddValue(builder, operand->var_indx[2]);
                            break; 
                        case SHADER_OPERAND_TYPE_MATTIMEVEC:
                            ShaderBuilderAddOp(builder, SpvOpMatrixTimesVector, 5);
                            ShaderBuilderAddValue(builder, operand->var_indx[0]);
                            ShaderBuilderAddValue(builder, operand->indx);
                            ShaderBuilderAddValue(builder, operand->var_indx[1]);
                            ShaderBuilderAddValue(builder, operand->var_indx[2]);
                            break; 
                        case SHADER_OPERAND_TYPE_MATTIMEMAT:
                            ShaderBuilderAddOp(builder, SpvOpMatrixTimesMatrix, 5);
                            ShaderBuilderAddValue(builder, operand->var_indx[0]);
                            ShaderBuilderAddValue(builder, operand->indx);
                            ShaderBuilderAddValue(builder, operand->var_indx[1]);
                            ShaderBuilderAddValue(builder, operand->var_indx[2]);
                            break; 
                        case SHADER_OPERAND_TYPE_VECTIMES:
                            ShaderBuilderAddOp(builder, SpvOpVectorTimesScalar, 5);
                            ShaderBuilderAddValue(builder, operand->var_indx[0]);
                            ShaderBuilderAddValue(builder, operand->indx);
                            ShaderBuilderAddValue(builder, operand->var_indx[1]);
                            ShaderBuilderAddValue(builder, operand->var_indx[2]);
                            break;   
                    }
                }

                ShaderBuilderAddOp(builder, SpvOpReturn, 1);
            }

            ShaderBuilderAddOp(builder, SpvOpFunctionEnd, 1);
        }
    }

    builder->code[3] = builder->current_index + 1;

    ShaderBuilderClear(builder);
}

int iter = 0;
uint32_t val = 0, op = 0, left_val = 0;
void StartReading(){
    left_val = 0;
    iter = 15;
    val = 0;
    op = 0;
}

void NextCode(uint32_t *ptr, uint32_t offset){

    iter += offset;
    val = ptr[iter];
    left_val = val >> 16;
    op = val & 0x0000FFFF;
}

ShaderDataFlags FindDataFlags(uint32_t val){

    switch(val){
        case 0:
            return SHADER_DATA_FLAG_UNIFORM_CONSTANT;
        case 2:
            return SHADER_DATA_FLAG_UNIFORM;
        case 3:
            return SHADER_DATA_FLAG_OUTPUT;
        case 7:
            return SHADER_DATA_FLAG_FUNCTION;
    }

    return 0;
}

void ShaderBuilderParcingShader(ShaderBuilder *builder, uint32_t *shader, uint32_t size){

    uint32_t *ptr = shader;

    int entry = 0;

    StartReading();
    NextCode(ptr, 1);

    entry = val >> 16;

    NextCode(ptr, 1);

    switch(val){
        case SpvExecutionModelFragment: 
            builder->type = SHADER_TYPE_FRAGMENT;
            NextCode(ptr, 1);
            break;
        case SpvExecutionModelVertex: 
            builder->type = SHADER_TYPE_VERTEX;
            NextCode(ptr, 1);
            break;
        case SpvExecutionModelGLCompute: 
            builder->type = SHADER_TYPE_COMPUTED;
            NextCode(ptr, 1);
            break;
        case SpvExecutionModelGeometry: 
            builder->type = SHADER_TYPE_GEOMETRY;
            NextCode(ptr, 1);
            break;
        case SpvExecutionModelTessellationControl: 
            builder->type = SHADER_TYPE_TESELLATION_CONTROL;
            NextCode(ptr, 1);
            break;
        case SpvExecutionModelTessellationEvaluation: 
            builder->type = SHADER_TYPE_TESELLATION_EVALUATION;
            NextCode(ptr, 1);
            break;
    }

    builder->main_point_index = val;
    
    int j = 0;

    while(val != 0x00040047){
        NextCode(ptr, 1);
    }

    ShaderVariable *variable = NULL;

    while(iter < size){

        switch(op){
                case SpvOpDecorate:
                    NextCode(ptr, 1);
                    builder->decors[builder->num_decorations].indx = val;
                    NextCode(ptr, 1);
                    builder->decors[builder->num_decorations].type = val;
                    if(val == 2){
                        NextCode(ptr, 1);
                        continue;
                    }
                    NextCode(ptr, 1);
                    builder->decors[builder->num_decorations].val = val;
                    builder->num_decorations++;
                    NextCode(ptr, 1);
                    continue;
                case SpvOpMemberDecorate:
                    NextCode(ptr, 4);
                    break;
                case SpvOpDecorationGroup:
                    NextCode(ptr, 2);
                    break;
                case SpvOpGroupDecorate:
                    NextCode(ptr, 2);
                    break;
                case SpvOpGroupMemberDecorate:
                    NextCode(ptr, 2);
                    break;
                case SpvOpDecorateId:
                    NextCode(ptr, 3);
                    break;
                case SpvOpDecorateString:
                    NextCode(ptr, 4);
                    break;
                case SpvOpMemberDecorateString:
                    NextCode(ptr, 4);
                    break;
        }  

        if(val < SpvOpGroupLogicalXorKHR)
        {
            NextCode(ptr, 1);
            continue;
        }
        
        switch(op){
            case SpvOpTypeVoid:
                variable = ShaderBuilderAllocateVariabel(builder);
                variable->type = SHADER_VARIABLE_TYPE_VOID;
                NextCode(ptr, 1);
                variable->indx = val;
                NextCode(ptr, 1);
                break;
            case SpvOpTypeFunction:
                variable = ShaderBuilderAllocateVariabel(builder);
                variable->type = SHADER_VARIABLE_TYPE_FUNCTION;
                NextCode(ptr, 1);
                variable->indx = val;
                NextCode(ptr, 1);
                variable->values[0] = val;
                NextCode(ptr, 1);
                variable->num_values = 1;
                break;
            case SpvOpTypeInt:
                variable = ShaderBuilderAllocateVariabel(builder);
                variable->type = SHADER_VARIABLE_TYPE_INT;
                NextCode(ptr, 1);
                variable->indx = val;
                NextCode(ptr, 1);
                variable->values[0] = val;
                NextCode(ptr, 1);
                variable->values[1] = val;
                NextCode(ptr, 1);
                variable->num_values = 2;
                break;
            case SpvOpTypeFloat:
                variable = ShaderBuilderAllocateVariabel(builder);
                variable->type = SHADER_VARIABLE_TYPE_FLOAT;
                NextCode(ptr, 1);
                variable->indx = val;
                NextCode(ptr, 1);
                variable->values[0] = val;
                NextCode(ptr, 1);
                variable->num_values = 1;
                break;
            case SpvOpTypeArray:
                variable = ShaderBuilderAllocateVariabel(builder);
                variable->type = SHADER_VARIABLE_TYPE_ARRAY;
                NextCode(ptr, 1);
                variable->indx = val;
                NextCode(ptr, 1);
                variable->args[0] = val;
                NextCode(ptr, 1);
                variable->args[1] = val;
                NextCode(ptr, 1);
                variable->num_args = 2;
                break;
            case SpvOpTypeImage:
                variable = ShaderBuilderAllocateVariabel(builder);
                variable->type = SHADER_VARIABLE_TYPE_IMAGE;
                NextCode(ptr, 1);
                variable->indx = val;
                NextCode(ptr, 1);
                variable->args[0] = val;
                NextCode(ptr, 1);
                variable->values[0] = val;
                NextCode(ptr, 1);
                variable->values[1] = val;
                NextCode(ptr, 1);
                variable->values[2] = val;
                NextCode(ptr, 1);
                variable->values[3] = val;
                NextCode(ptr, 1);
                variable->values[4] = val;
                NextCode(ptr, 1);
                variable->values[5] = val;
                NextCode(ptr, 1);
                variable->num_values = 6;
                variable->num_args = 1;
                break;
            case SpvOpTypeSampledImage:
                variable = ShaderBuilderAllocateVariabel(builder);
                variable->type = SHADER_VARIABLE_TYPE_SAMPLED_IMAGE;
                NextCode(ptr, 1);
                variable->indx = val;
                NextCode(ptr, 1);
                variable->args[0] = val;
                NextCode(ptr, 1);
                variable->num_args = 1;
                break;
            case SpvOpConstant:
                variable = ShaderBuilderAllocateVariabel(builder);
                variable->type = SHADER_VARIABLE_TYPE_CONSTANT;
                NextCode(ptr, 1);
                variable->args[0] = val;
                NextCode(ptr, 1);
                variable->indx = val;
                NextCode(ptr, 1);
                variable->values[0] = val;
                NextCode(ptr, 1);
                variable->num_args = 1;
                variable->num_values = 1;
                break;
            case SpvOpTypeVector:
                variable = ShaderBuilderAllocateVariabel(builder);
                variable->type = SHADER_VARIABLE_TYPE_VECTOR;
                NextCode(ptr, 1);
                variable->indx = val;
                NextCode(ptr, 1);
                variable->args[0] = val;
                NextCode(ptr, 1);
                variable->values[0] = val;
                NextCode(ptr, 1);
                variable->num_args = 1;
                variable->num_values = 1;
                break;
            case SpvOpTypeMatrix:
                variable = ShaderBuilderAllocateVariabel(builder);
                variable->type = SHADER_VARIABLE_TYPE_MATRIX;
                NextCode(ptr, 1);
                variable->indx = val;
                NextCode(ptr, 1);
                variable->args[0] = val;
                NextCode(ptr, 1);
                variable->values[0] = val;
                NextCode(ptr, 1);
                variable->num_args = 1;
                variable->num_values = 1;
                break;
            case SpvOpTypeStruct:   
                variable = ShaderBuilderAllocateVariabel(builder);         
                variable->type = SHADER_VARIABLE_TYPE_STRUCT;
                variable->num_args = left_val - 2;
                NextCode(ptr, 1);
                variable->indx = val;
                NextCode(ptr, 1);
                for(j=0;j < variable->num_args;j++){                    
                    variable->args[j] = val;
                    NextCode(ptr, 1);
                }
                break;
            case SpvOpTypePointer:
                variable = ShaderBuilderAllocateVariabel(builder);
                variable->type = SHADER_VARIABLE_TYPE_POINTER;
                NextCode(ptr, 1);
                variable->indx = val;
                NextCode(ptr, 1);
                variable->flags = FindDataFlags(val); // Flags
                NextCode(ptr, 1);
                variable->args[0] = val;
                NextCode(ptr, 1);
                variable->num_args = 1;
                break;
            case SpvOpVariable:
                variable = ShaderBuilderAllocateVariabel(builder);
                variable->type = SHADER_VARIABLE_TYPE_VARIABLE;
                NextCode(ptr, 1);
                variable->args[0] = val;
                NextCode(ptr, 1);
                variable->indx = val;
                NextCode(ptr, 1);
                variable->flags = FindDataFlags(val); //Flags
                NextCode(ptr, 1);
                variable->num_args = 1;
                break;
            case SpvOpFunction:
                return;
            default:        
                NextCode(ptr, 1);
                break;
        }
    }
}

uint32_t ReturnSizeVector(ShaderBuilder *builder, ShaderVariable *var_elm){

    ShaderVariable *elm_type = ShaderBuilderFindVar(builder, var_elm->args[0]);

    uint32_t temp_size = 0;
    if(elm_type->type == SHADER_VARIABLE_TYPE_FLOAT || elm_type->type == SHADER_VARIABLE_TYPE_INT)
        temp_size = 4 * var_elm->values[0] /*count elem*/;

    if(temp_size == 12)
        temp_size = 16;

    return temp_size;
}

uint32_t ReturnSizeMatrix(ShaderBuilder *builder, ShaderVariable *mat_elm){

    uint32_t size = 0;

    ShaderVariable *elm_type = ShaderBuilderFindVar(builder, mat_elm->args[0]); 

    if(elm_type->type == SHADER_VARIABLE_TYPE_FLOAT || elm_type->type == SHADER_VARIABLE_TYPE_INT)
        size += 4 * mat_elm->values[0] /*count elem*/;
    else if(elm_type->type == SHADER_VARIABLE_TYPE_VECTOR){
        size += ReturnSizeVector(builder, elm_type) * mat_elm->values[0];
    }

    return size;
}

uint32_t ReturnSizeStruct(ShaderBuilder *builder, ShaderVariable *str_elm){

    uint32_t size = 0;
    for(int j=0;j < str_elm->num_args;j++){
        ShaderVariable *var_elm = ShaderBuilderFindVar(builder, str_elm->args[j]);
        
        if(var_elm->type == SHADER_VARIABLE_TYPE_FLOAT || var_elm->type == SHADER_VARIABLE_TYPE_INT){
            size += 4;
        }else if(var_elm->type == SHADER_VARIABLE_TYPE_VECTOR){
            size += ReturnSizeVector(builder, var_elm);
        }else if(var_elm->type == SHADER_VARIABLE_TYPE_MATRIX){
            size += ReturnSizeMatrix(builder, var_elm);
        }else if(var_elm->type == SHADER_VARIABLE_TYPE_ARRAY){
            ShaderVariable *elm_type = ShaderBuilderFindVar(builder, var_elm->args[0]);
            ShaderVariable *elm_const = ShaderBuilderFindVar(builder, var_elm->args[1]);        
            if(elm_type->type == SHADER_VARIABLE_TYPE_FLOAT || elm_type->type == SHADER_VARIABLE_TYPE_INT){
                size += 4 * elm_const->values[0]/*count elem*/;
            }else if(elm_type->type == SHADER_VARIABLE_TYPE_MATRIX){
                size += ReturnSizeMatrix(builder, elm_type) * elm_const->values[0]/*count elem*/;                                
            }else if(elm_type->type == SHADER_VARIABLE_TYPE_STRUCT){
                uint32_t str_size = ReturnSizeStruct(builder, elm_type);
                size += str_size * elm_const->values[0]/*count elem*/;
            }            
            
            while(size % 16)
                size ++;

        }else if(var_elm->type == SHADER_VARIABLE_TYPE_STRUCT){
                size += ReturnSizeStruct(builder, var_elm);
        }
    }

    return size;
}

void ShaderBuilderMakeUniformsFromShader(ShaderBuilder *builder, uint32_t *code, uint32_t size, void *blueprints, uint32_t indx_pack){

    size /= sizeof(uint32_t);

    if(builder->alloc_head == NULL)
        builder->alloc_head = calloc(1, sizeof(ChildStack));

    ShaderBuilderParcingShader(builder, code, size);

    printf("Shader builder : Variables count %i \n", ShaderBuilderGetVariablesCount(builder));

    ShaderVariable *currVar = NULL;
    uint32_t size_buffer = 0, flags = 0;
    
    ChildStack *child = builder->alloc_head;

    int find = 0;
    while(child != NULL){
        currVar = child->node;

        if(currVar != NULL){
            if(currVar->type == SHADER_VARIABLE_TYPE_VARIABLE){
                flags = currVar->flags;
                if(flags & SHADER_DATA_FLAG_UNIFORM){
                    size_buffer = 0;

                    ShaderVariable *var_point = ShaderBuilderFindVar(builder, currVar->args[0]);

                    ShaderVariable *var_orig = ShaderBuilderFindVar(builder, var_point->args[0]);

                    if(var_orig->type == SHADER_VARIABLE_TYPE_STRUCT){
                        size_buffer += ReturnSizeStruct(builder, var_orig);
                        

                        uint32_t binding = 0;
                        for(int i=0;i < builder->num_decorations;i++){
                            if(builder->decors[i].type == SpvDecorationBinding && builder->decors[i].indx == currVar->indx)
                                binding = builder->decors[i].val;
                        }
                        
                        BluePrintAddUniformObjectC(blueprints, indx_pack, size_buffer, builder->type == SHADER_TYPE_VERTEX ? VK_SHADER_STAGE_VERTEX_BIT : VK_SHADER_STAGE_FRAGMENT_BIT, binding);

                        printf("Shader builder : Uniform buffer size is %i\n", size_buffer);
                    }


                }else if(flags & SHADER_DATA_FLAG_UNIFORM_CONSTANT){

                    ShaderVariable *var_point = ShaderBuilderFindVar(builder, currVar->args[0]);

                    ShaderVariable *var_orig = ShaderBuilderFindVar(builder, var_point->args[0]);
                    
                    if(var_orig->type == SHADER_VARIABLE_TYPE_SAMPLED_IMAGE){

                        
                        uint32_t binding = 0;
                        for(int i=0;i < builder->num_decorations;i++){
                            if(builder->decors[i].type == SpvDecorationBinding && builder->decors[i].indx == currVar->indx)
                                binding = builder->decors[i].val;
                        }
                    
                        BluePrintAddTextureC(blueprints, indx_pack, builder->type == SHADER_TYPE_VERTEX ? VK_SHADER_STAGE_VERTEX_BIT : VK_SHADER_STAGE_FRAGMENT_BIT, binding);

                        printf("Shader builder : Image added to blueprint\n", size_buffer);
                    }
                }
            }
        }

        child = child->next;
    }

    ShaderBuilderClear(builder);
}

void ShaderBuilderWriteToFile(ShaderBuilder *builder, const char *path){

    FILE *somefile = open(path, O_WRONLY | O_CREAT | O_BINARY);

    write(somefile, builder->code, builder->size * sizeof(uint32_t));

    close(somefile);

}
