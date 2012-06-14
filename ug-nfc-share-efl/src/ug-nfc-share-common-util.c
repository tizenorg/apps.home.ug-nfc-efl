/*
  * Copyright 2012  Samsung Electronics Co., Ltd
  *
  * Licensed under the Flora License, Version 1.0 (the "License");
  * you may not use this file except in compliance with the License.
  * You may obtain a copy of the License at

  *     http://www.tizenopensource.org/license
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  */

#include <string.h>

#include "appsvc.h"
#include <aul.h>

#include "ug-nfc-share-common-util.h"
#include "ug-nfc-share-common-debug.h"

char* _strncpy(char* dest, const char* src, int len, char* file, int line)
{
	if(dest == NULL || src == NULL || len <= 0)
	{
		UG_NFC_SHARE_DEBUG_ERR("STRNCPY ERROR at %s(%d)", file, line);
		return dest;
	}

	strncpy(dest, src, len);
	dest[len] = '\0';

	return dest;
}

char* _strcat(char* dest, const char* src, int dest_size, char* file, int line)
{
	if(dest == NULL || src == NULL || dest_size <= 0)
	{
		UG_NFC_SHARE_DEBUG_ERR("STRNCAT ERROR at %s(%d)", file, line);
		return dest;
	}

	int ret_len = 0;

	ret_len = g_strlcat(dest, src, dest_size);
	if(ret_len >= dest_size)
	{
		UG_NFC_SHARE_DEBUG_ERR("STRNCAT SIZE ERROR at %s(%d)", file, line);
		//return dest;
	}
	return dest;
}

int _strcmp(const char* src1, const char* src2, char* file, int line)
{
	if(src1 == NULL || src2 == NULL)
	{
		UG_NFC_SHARE_DEBUG_ERR("STRCMP ERROR at %s(%d)", file, line);
		return -1;
	}

	return strcmp(src1, src2);
}


#ifdef _UG_NFC_SHARE_MEM_TRACE_
static gint g_ug_nfc_share_total_memory_size = 0;
static gint g_ug_nfc_share_peak_memory_size = 0;
static ug_nfc_share_mem_unit_t *g_memory_unit_first = NULL;
static ug_nfc_share_mem_unit_t *g_memory_unit_last = NULL;

void ug_nfc_share_memory_unit_add(gchar *func_name, gint line_num, void* pMemory, gint size )
{

	ug_nfc_share_mem_unit_t * unit_mem;

	if( g_memory_unit_first == NULL )
	{
		unit_mem = g_malloc0(sizeof(ug_nfc_share_mem_unit_t));
		if( unit_mem == NULL )
		{
			UG_NFC_SHARE_DEBUG_ERR("\n\n ## [%s - %d] ## out of memory!\n\n", __FUNCTION__, __LINE__);
			return ;
		}
		else
		{
			unit_mem->size = size;
			unit_mem->address = pMemory;
			unit_mem->func_name = func_name;
			unit_mem->line = line_num;
			unit_mem->next = NULL;
			unit_mem->prev = NULL;
		}
		g_memory_unit_first = unit_mem;
		g_memory_unit_last = unit_mem;
	}
	else
	{
		unit_mem = g_malloc0(sizeof(ug_nfc_share_mem_unit_t));
		if( unit_mem == NULL )
		{
			UG_NFC_SHARE_DEBUG_ERR("\n\n ## [%s - %d] ## out of memory!\n\n", __FUNCTION__, __LINE__);
			return ;
		}
		else
		{
			unit_mem->size = size;
			unit_mem->address = pMemory;
			unit_mem->func_name = func_name;
			unit_mem->line = line_num;
			unit_mem->next = NULL;
			unit_mem->prev = g_memory_unit_last;
		}
		g_memory_unit_last->next = unit_mem;
		g_memory_unit_last = unit_mem;

	}
	g_ug_nfc_share_total_memory_size = g_ug_nfc_share_total_memory_size + size;
	if( g_ug_nfc_share_peak_memory_size < g_ug_nfc_share_total_memory_size )
		g_ug_nfc_share_peak_memory_size = g_ug_nfc_share_total_memory_size;

}
void ug_nfc_share_memory_unit_delete(void* pMemory)
{
	ug_nfc_share_mem_unit_t *tmp;
	if( pMemory == g_memory_unit_first->address )
	{
		tmp = g_memory_unit_first;
		g_memory_unit_first = tmp->next;
		if(g_memory_unit_first != NULL)
		g_memory_unit_first->prev = NULL;

		g_ug_nfc_share_total_memory_size = g_ug_nfc_share_total_memory_size - tmp->size;
		g_free(tmp->func_name);
		tmp->func_name = NULL;
		g_free(tmp);
		tmp = NULL;
	}
	else if( pMemory == g_memory_unit_last->address )
	{
		tmp = g_memory_unit_last;

		g_memory_unit_last = tmp->prev;
		g_memory_unit_last->next = NULL;

		g_ug_nfc_share_total_memory_size = g_ug_nfc_share_total_memory_size - tmp->size;
		g_free(tmp->func_name);
		tmp->func_name = NULL;

		g_free(tmp);
		tmp = NULL;

	}
	else
	{
		tmp = g_memory_unit_first;
		for( ; tmp != g_memory_unit_last && tmp != NULL; )
		{
			if( tmp->address == pMemory )
			{
				tmp->prev->next = tmp->next;
				tmp->next->prev = tmp->prev;

				g_ug_nfc_share_total_memory_size = g_ug_nfc_share_total_memory_size - tmp->size;
				g_free(tmp->func_name);
				tmp->func_name = NULL;
				g_free( tmp );
				tmp = NULL;
				break;
			}
			tmp = tmp->next;
		}
	}

}

void
ug_nfc_share_memory_print_list()
{
	ug_nfc_share_mem_unit_t *tmp = g_memory_unit_first;
	while(tmp != NULL)
	{
		UG_NFC_SHARE_DEBUG_ERR("func_name:%s(%d) <address:%p, size:%d>", tmp->func_name, tmp->line, tmp->address, tmp->size);
		tmp = tmp->next;
	}
	UG_NFC_SHARE_DEBUG("total memory size :%d", g_ug_nfc_share_total_memory_size);
	UG_NFC_SHARE_DEBUG("peak memory size :%d", g_ug_nfc_share_peak_memory_size);


}
#endif /*_UG_NFC_SHARE_MEM_TRACE_*/

int ug_nfc_share_launch_service(const char *operation, const char *uri, const char *mime)
{
	int result = -1;

	bundle *bd = NULL;

	bd = bundle_create();
	retv_if(bd == NULL, result);

	if (operation != NULL)
	{
		appsvc_set_operation(bd, operation);
	}

	if (uri != NULL)
	{
		appsvc_set_uri(bd, uri);
	}

	if (mime != NULL)
	{
		appsvc_set_mime(bd, mime);
	}

	result = appsvc_run_service(bd, 0, NULL, NULL);

	bundle_free(bd);

	return result;
}
