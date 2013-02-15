/*
  * Copyright (c) 2012, 2013 Samsung Electronics Co., Ltd.
  *
  * Licensed under the Flora License, Version 1.0 (the "License");
  * you may not use this file except in compliance with the License.
  * You may obtain a copy of the License at

  *     http://floralicense.org/license/
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  */


#ifndef __UG_NFC_SHARE_COMMON_UTIL_H__
#define __UG_NFC_SHARE_COMMON_UTIL_H__

#include <glib.h>
#include <assert.h>
#include <dlog.h>

#define _STRNCPY(DEST, SRC, LEN) _strncpy(DEST, SRC, LEN , __FILE__, __LINE__)
#define _STRCAT(DEST, SRC, DEST_SIZE) _strcat(DEST, SRC, DEST_SIZE, __FILE__, __LINE__)
#define _STRCMP(SRC1, SRC2) _strcmp(SRC1, SRC2, __FILE__, __LINE__)

char*	_strncpy(char* dest, const char* src, int len, char* file, int line);
char*	_strcat(char* dest, const char* src, int dest_size, char* file, int line);
int		_strcmp(const char* src1, const char* src2, char* file, int line);


#define UG_NFC_SHARE_MAX_HEAP_SIZE							5*1024*1024

#ifdef _UG_NFC_SHARE_MEM_TRACE_

typedef struct _ug_nfc_share_mem_unit_t	ug_nfc_share_mem_unit_t;
struct _ug_nfc_share_mem_unit_t
{
	void* address;
	gint size;
	gchar *func_name;
	gint line;
	ug_nfc_share_mem_unit_t *next;
	ug_nfc_share_mem_unit_t *prev;
};

void ug_nfc_share_memory_unit_add(gchar *func_name, gint line_num, void* pMemory, gint size);
void ug_nfc_share_memory_unit_delete(void* pMemory);
void ug_nfc_share_memory_print_list(void);


#define _EDJ(obj) elm_layout_edje_get(obj)

#define UG_NFC_SHARE_MEM_MALLOC(ptr,no_elements,type)	\
		do{\
			if((gint)(no_elements)<=0)\
			{\
				ptr = NULL;\
			}\
			else if(UG_NFC_SHARE_MAX_HEAP_SIZE<(gint)(no_elements)*sizeof(type))\
			{\
				assert(0);\
			}\
			else\
			{\
				ptr=(type*)g_malloc0((gint)(no_elements)*sizeof(type));\
				assert(ptr);\
				UG_NFC_SHARE_DEBUG("Allocate memory. pointer:%p\n", ptr);\
				gchar *func_name = g_malloc0(sizeof(__FUNCTION__));\
				snprintf(func_name, sizeof(__FUNCTION__), __FUNCTION__);\
				ug_nfc_share_memory_unit_add(func_name, __LINE__, ptr, (no_elements)*sizeof(type));\
			}\
		}while(0)

#define UG_NFC_SHARE_MEM_STRDUP(ptr,str)	\
		do {\
			if((str) != NULL)\
			{\
				ptr = g_strdup((const char *)(str));\
				assert(ptr);\
				UG_NFC_SHARE_DEBUG("Allocate memory. pointer:%p\n", (ptr));\
				gchar *func_name = g_malloc0(sizeof(__FUNCTION__));\
				snprintf(func_name, sizeof(__FUNCTION__), __FUNCTION__);\
				ug_nfc_share_memory_unit_add(func_name, __LINE__, (ptr), (strlen(ptr)+1)*sizeof(char));\
			}\
			else\
			{\
				(ptr) = NULL;\
			}\
		}while(0)

#define UG_NFC_SHARE_MEM_STRNDUP(ptr,str,buf_size)\
		do {\
			if((str) != NULL && (buf_size) >= 0 )\
			{\
				ptr = g_strndup((const char *)(str),(buf_size));\
				assert(ptr);\
				UG_NFC_SHARE_DEBUG("Allocate memory. pointer:%p\n", (ptr));\
				gchar *func_name = g_malloc0(sizeof(__FUNCTION__));\
				snprintf(func_name, sizeof(__FUNCTION__), __FUNCTION__);\
				ug_nfc_share_memory_unit_add(func_name, __LINE__, (ptr), (strlen(ptr)+1)*sizeof(char));\
			}\
			else\
			{\
				(ptr) = NULL;\
			}\
		}while(0)

#define UG_NFC_SHARE_MEM_MEMDUP(ptr,src,buf_size)\
		do {\
			if((src) != NULL && (buf_size) >= 0 )\
			{\
				ptr = g_malloc0((buf_size));\
				assert(ptr);\
				memcpy((ptr), (void *)(src),(buf_size));\
				UG_NFC_SHARE_DEBUG("Allocate memory. pointer:%p\n", (ptr));\
				gchar *func_name = g_malloc0(sizeof(__FUNCTION__));\
				snprintf(func_name, sizeof(__FUNCTION__), __FUNCTION__);\
				ug_nfc_share_memory_unit_add(func_name, __LINE__, (ptr), (strlen(ptr)+1)*sizeof(char));\
			}\
			else\
			{\
				(ptr) = NULL;\
			}\
		}while(0)

#define UG_NFC_SHARE_MEM_FREE(ptr)	\
		do {\
			if(ptr != NULL)\
			{\
				UG_NFC_SHARE_DEBUG("Free memory. pointer:%p\n", ptr);\
				ug_nfc_share_memory_unit_delete( ptr );\
				g_free(ptr);\
				ptr = NULL;\
			}\
		}while(0)

#else	/* _UG_NFC_SHARE_MEM_TRACE_ */

#define UG_NFC_SHARE_MEM_MALLOC(ptr,no_elements,type)	\
		do{\
			if((gint)(no_elements)<=0)\
			{\
				ptr=NULL;\
			}\
			else if(UG_NFC_SHARE_MAX_HEAP_SIZE<(gint)(no_elements)*sizeof(type))\
			{\
				assert(0);\
			}\
			else\
			{\
				ptr=(type*)g_malloc0((gint)(no_elements)*sizeof(type));\
				assert(ptr);\
			}\
		}while(0)

#define UG_NFC_SHARE_MEM_STRDUP(ptr,str)	\
			do {\
				if((str) != NULL)\
				{\
					ptr = g_strdup((const char *)(str));\
					assert(ptr);\
				}\
				else\
				{\
					(ptr) = NULL;\
				}\
			}while(0)

#define UG_NFC_SHARE_MEM_STRNDUP(ptr,str,buf_size)\
		do {\
			if((str) != NULL && (buf_size) >= 0 )\
			{\
				ptr = g_strndup((const char *)(str),(buf_size));\
				assert(ptr);\
			}\
			else\
			{\
				(ptr) = NULL;\
			}\
		}while(0)

#define UG_NFC_SHARE_MEM_MEMDUP(ptr,src,buf_size)\
		do {\
			if((src) != NULL && (buf_size) >= 0 )\
			{\
				ptr = g_malloc0((buf_size));\
				assert(ptr);\
				memcpy((ptr), (void *)(src),(buf_size));\
			}\
			else\
			{\
				(ptr) = NULL;\
			}\
		}while(0)

#define UG_NFC_SHARE_MEM_FREE(ptr)	\
		do {\
			if(ptr != NULL)\
			{\
				g_free(ptr);\
				ptr = NULL;\
			}\
		}while(0)

#endif /* _UG_NFC_SHARE_MEM_TRACE_ */

#endif	/*__UG_NFC_SHARE_COMMON_UTIL_H__*/

