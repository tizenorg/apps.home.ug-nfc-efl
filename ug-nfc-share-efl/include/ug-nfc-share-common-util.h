/*
  * Copyright (c) 2012, 2013 Samsung Electronics Co., Ltd.
  *
  * Licensed under the Flora License, Version 1.1 (the "License");
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


#define UG_NFC_SHARE_MAX_HEAP_SIZE							5*1024*1024

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

#define UG_NFC_SHARE_MEM_FREE(ptr)	\
		do {\
			if(ptr != NULL)\
			{\
				g_free(ptr);\
				ptr = NULL;\
			}\
		}while(0)

#endif	/*__UG_NFC_SHARE_COMMON_UTIL_H__*/

