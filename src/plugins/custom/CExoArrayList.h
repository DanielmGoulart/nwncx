#pragma once

//Allocates size bytes and returns a pointer to its start location
void * (__cdecl *nwncx_malloc)( unsigned int size ) = (void *(__cdecl *)(unsigned int))0x005BEEA0;
//Frees memory blocks allocated by nwnx_malloc/calloc/realloc
void (__cdecl *nwncx_free)( void * ptr  ) = (void (__cdecl *)(void *))0x005BEEB0;

template <class T> 
struct CExoArrayList{
	T* data;
	int len;
	int alloc;

	inline int Add(T val, int reserve = 10){
		if (len == alloc) {                                                                       
            T *re;    
			re = (T *) nwncx_malloc(sizeof(T) * (alloc + reserve));
			if(!re)
				return -1;
			memcpy(re, data, len);
			re[len] = 0;

			nwncx_free(data);
            //if ((re = (T *)realloc(data, sizeof(T) * (alloc + reserve))) == NULL)     
            //    return -1;                                                                        
            data = re;                                                                            
            alloc +=reserve;                                                                      
        }                                                                                         
        data[len++] = val;                                                                        
        return len; 
	}

	inline int Contains(T val){
		for (int i = 0; i < len; i++){
			if(data[i] == val)
				return i + 1;
		}

		return 0;
	}
};