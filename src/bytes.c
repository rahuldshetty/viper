#include "bytes.h"

ObjByte* createByteObject(Value item){
    if(IS_NUMBER(item)){
        return newBytes( AS_NUMBER(item) );
    } else if(IS_LIST(item)){
        ObjList* list = AS_LIST(item);

        ObjByte* bytes = newBytes(list->array.count);
        
        for(int i = 0; i < list->array.count; i++){
            if(IS_NUMBER(list->array.values[i])){
                bytes->bytes.byte[i] = (unsigned char) AS_NUMBER(list->array.values[i]);
            } else {
                bytes->bytes.byte[i] = 0;
            }
        }

        return bytes;
    }
}