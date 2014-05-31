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


#include "ug-nfc-share-tag.h"
#include "ug-nfc-share-popup.h"

#include <stdio.h>
#include <Elementary.h>
#include <Ecore.h>
#include <bundle.h>
#include <Ecore_X.h>
#include <vconf.h>
#include <sys/types.h>
#include <sys/wait.h>

/* external library header */
#include <mime_type.h>
#include <notification.h>

#define NFC_POPUP_TIMEOUT               3.0

static ug_nfc_share_tag_type ug_nfc_share_tagType;


int _bt_ipc_send_obex_message(char *address, const uint8_t *files, uint32_t length);

/*-----------------------------------------------------------------------------------------------*/

static void _show_status_text(void *data, char *text)
{
	ugdata_t *ug_data = (ugdata_t *)data;

	UG_NFC_SHARE_BEGIN();

	ret_if(ug_data == NULL);
	ret_if(text == NULL);

	notification_status_message_post(text);

	ug_destroy_me(ug_data->nfc_share_ug);

	UG_NFC_SHARE_END();
}

ug_nfc_share_tag_type ug_nfc_share_get_tag_type(void)
{
	return ug_nfc_share_tagType;
}

void ug_nfc_share_set_tag_type(ug_nfc_share_tag_type tag_type)
{
	if (tag_type < 0 || tag_type >= UG_NFC_SHARE_TAG_MAX)
		return;

	ug_nfc_share_tagType = tag_type;
}

nfc_ndef_message_h ug_nfc_share_get_current_ndef(void *data)
{
	ugdata_t *ug_data = (ugdata_t *)data;

	UG_NFC_SHARE_BEGIN();

	retv_if(ug_data == NULL, NULL);

	UG_NFC_SHARE_END();

	return ug_data->current_ndef;
}

ug_nfc_share_result_e ug_nfc_share_set_current_ndef(void *data, nfc_ndef_message_h ndef_msg)
{
	UG_NFC_SHARE_BEGIN();

	ugdata_t *ug_data = (ugdata_t *)data;
	if (ug_data == NULL)
	{
		UG_NFC_SHARE_DEBUG_ERR("ug_data is null");
		return UG_NFC_SHARE_ERROR;
	}

	if (ug_data->current_ndef != NULL)
	{
		if (nfc_ndef_message_destroy(ug_data->current_ndef) != NFC_ERROR_NONE)
		{
			UG_NFC_SHARE_DEBUG_ERR("nfc_ndef_message_destroy failed");
		}
	}

	ug_data->current_ndef = ndef_msg;

	UG_NFC_SHARE_END();

	return UG_NFC_SHARE_OK;

}

static ug_nfc_share_result_e ug_nfc_share_make_mime_type_data_from_file_path(const char *path, uint8_t *type_data, uint32_t *type_size)
{
	ug_nfc_share_result_e result = UG_NFC_SHARE_ERROR;
	char *extension = NULL;

	UG_NFC_SHARE_BEGIN();

	retv_if(path == NULL, result);
	retv_if(type_data == NULL, result);
	retv_if(type_size == NULL, result);

	UG_NFC_SHARE_DEBUG("typedata = %p, typesize = %d", type_data, *type_size);

	memset(type_data, 0, *type_size);
	*type_size = 0;

	extension = strrchr(path, '.');
	UG_NFC_SHARE_DEBUG("extension = %s", GET_SAFE_STRING(extension));

	if (extension != NULL)
	{
		char *mime_str = NULL;

		if (mime_type_get_mime_type(extension+1, &mime_str) == MIME_TYPE_ERROR_NONE)
		{
			UG_NFC_SHARE_DEBUG("mime_str[%s]", mime_str);

			*type_size = strlen(mime_str);
			memcpy(type_data, mime_str, *type_size);
			result = UG_NFC_SHARE_OK;
		}
		else
		{
			UG_NFC_SHARE_DEBUG_ERR("ERROR :: mime_type_get_mime_type failed");
			result = UG_NFC_SHARE_ERROR;
		}
	}

	UG_NFC_SHARE_DEBUG("mime type : %s", GET_SAFE_STRING((char *)type_data));

	UG_NFC_SHARE_END();

	return result;
}

ug_nfc_share_result_e ug_nfc_share_make_ndef_message_from_file(nfc_ndef_message_h *msg, const char *path)
{
	int result = UG_NFC_SHARE_ERROR;
	struct stat st;
	uint8_t type_buffer[50] = { 0, };
	int type_size = sizeof(type_buffer);
	nfc_ndef_record_h record = NULL;
	FILE *file = NULL;
	char *file_name = NULL;
	uint8_t *file_data = NULL;
	long int file_len = 0;


	UG_NFC_SHARE_BEGIN();

	retv_if(msg == NULL, result);
	retv_if(path == NULL, result);
	/*Cause of Svace warning */
	/*Name : TOCTTOU_SEQUENCE
	 Mitigation :
	 1.use fstat inplace of stat
	 2.or using check-Use-Check Pattern
	 */
	retv_if((stat(path, &st) == -1 && errno == ENOENT), result);

	/* read file and make payload*/
	file = fopen(path, "r");
	if (file != NULL)
	{
		long int read_count = 0, read_total = 0;

		fseek(file, 0, SEEK_END);
		file_len = ftell(file);
		fseek(file, 0, SEEK_SET);

		UG_NFC_SHARE_MEM_MALLOC(file_data, file_len, uint8_t);
		if (file_data == NULL)
		{
			UG_NFC_SHARE_DEBUG_ERR("ERROR :: UG_NFC_SHARE_MEM_MALLOC failed");

			fclose(file);

			return result;
		}

		do
		{
			read_count = fread(file_data + read_total, 1, file_len - read_total, file);
			read_total += read_count;
		}
		while (read_count != 0 && read_total < file_len);

		fclose(file);

		UG_NFC_SHARE_DEBUG("fread(%s) success, size %ld", path, file_len);
	}
	else
	{
		UG_NFC_SHARE_DEBUG_ERR("fopen(%s) error");

		return result;
	}

	/* get type data */
	result = ug_nfc_share_make_mime_type_data_from_file_path(path, type_buffer, (uint32_t *)&type_size);
	if (result != UG_NFC_SHARE_OK)
	{
		UG_NFC_SHARE_DEBUG_ERR("ERROR :: _make_mime_type_data_from_file_path failed [%d]", result);

		return result;
	}

	/* get file name for id */
	file_name = strrchr(path, '/');
	if (file_name == NULL)
	{
		file_name = (char *)path;
	}
	else
	{
		file_name++;
	}

	UG_NFC_SHARE_DEBUG("file name : %s", file_name);

	/* create record */
	result = nfc_ndef_record_create(&record, NFC_RECORD_TNF_MIME_MEDIA, type_buffer, type_size, (uint8_t *)file_name, strlen(file_name), file_data, file_len);
	if (result != NFC_ERROR_NONE)
	{
		UG_NFC_SHARE_DEBUG_ERR("nfc_ndef_record_create failed (%d)", result);

		return result;
	}

	/* make file NDEF message */
	result = nfc_ndef_message_create(msg);
	if (result != NFC_ERROR_NONE)
	{
		UG_NFC_SHARE_DEBUG_ERR("nfc_ndef_message_create failed [%d]", result);

		nfc_ndef_record_destroy(record);

		return result;
	}

	/* append record to ndef message */
	result = nfc_ndef_message_append_record(*msg, record);
	if (result != NFC_ERROR_NONE)
	{
		UG_NFC_SHARE_DEBUG_ERR("nfc_ndef_message_append_record failed (%d)", result);

		return result;
	}

	UG_NFC_SHARE_DEBUG("ug_nfc_share_make_ndef_message_from_file success");

	UG_NFC_SHARE_END();

	return result;
}

/* nfc_handover */
ug_nfc_share_result_e ug_nfc_share_make_ndef_message_from_multi_file(nfc_ndef_message_h *msg, const char *path[], int record_count)
{
	int result = UG_NFC_SHARE_ERROR;
	struct stat st;
	uint8_t type_buffer[50] = { 0, };
	int type_size = sizeof(type_buffer);
	nfc_ndef_record_h record = NULL;
	FILE *file = NULL;
	char *file_name = NULL;
	uint8_t *file_data = NULL;
	long int file_len = 0;
	int index;


	UG_NFC_SHARE_BEGIN();

	retv_if(msg == NULL, result);
	retv_if(path == NULL, result);
	/*Cause of Svace warning */
	/*Name : TOCTTOU_SEQUENCE
	 Mitigation :
	 1.use fstat inplace of stat
	 2.or using check-Use-Check Pattern
	 */

	for (index = 0; index < record_count; index++)
	{
		retv_if((stat(path[index], &st) == -1 && errno == ENOENT), result);

		/* read file and make payload*/
		file = fopen(path[index], "r");

		if (file != NULL)
		{
			long int read_count = 0, read_total = 0;

			fseek(file, 0, SEEK_END);
			file_len = ftell(file);
			fseek(file, 0, SEEK_SET);

			UG_NFC_SHARE_MEM_MALLOC(file_data, file_len, uint8_t);
			if (file_data == NULL)
			{
				UG_NFC_SHARE_DEBUG_ERR("ERROR :: UG_NFC_SHARE_MEM_MALLOC failed");

				fclose(file);

				return result;
			}

			do
			{
				read_count = fread(file_data + read_total, 1, file_len - read_total, file);
				read_total += read_count;
			}
			while (read_count != 0 && read_total < file_len);

			fclose(file);

			UG_NFC_SHARE_DEBUG("fread(%s) success, size %ld", path[index], file_len);
		}
		else
		{
			UG_NFC_SHARE_DEBUG_ERR("fopen(%s) error");

			return result;
		}

		/* get type data */
		result = ug_nfc_share_make_mime_type_data_from_file_path(path[index], type_buffer, (uint32_t *)&type_size);
		if (result != UG_NFC_SHARE_OK)
		{
			UG_NFC_SHARE_DEBUG_ERR("ERROR :: _make_mime_type_data_from_file_path failed [%d]", result);

			return result;
		}

		/* get file name for id */
		file_name = strrchr(path[index], '/');
		if (file_name == NULL)
		{
			file_name = (char *)path[index];
		}
		else
		{
			file_name++;
		}

		UG_NFC_SHARE_DEBUG("file name : %s", file_name);

		/* create record */
		result = nfc_ndef_record_create(&record, NFC_RECORD_TNF_MIME_MEDIA, type_buffer, type_size, (uint8_t *)file_name, strlen(file_name), file_data, file_len);
		if (result != NFC_ERROR_NONE)
		{
			UG_NFC_SHARE_DEBUG_ERR("nfc_ndef_record_create failed (%d)", result);

			return result;
		}

		/* make file NDEF message */
		result = nfc_ndef_message_create(msg);
		if (result != NFC_ERROR_NONE)
		{
			UG_NFC_SHARE_DEBUG_ERR("nfc_ndef_message_create failed [%d]", result);

			nfc_ndef_record_destroy(record);

			return result;
		}

		/* append record to ndef message */
		result = nfc_ndef_message_append_record(*msg, record);
		if (result != NFC_ERROR_NONE)
		{
			UG_NFC_SHARE_DEBUG_ERR("nfc_ndef_message_append_record failed (%d)", result);

			return result;
		}

		UG_NFC_SHARE_DEBUG("ug_nfc_share_make_ndef_message_from_file success");

		UG_NFC_SHARE_END();
	}
	return result;
}

void _ug_nfc_share_get_bt_addr_from_string(uint8_t *addr, char *addr_string)
{
	char *temp = NULL;

	if (addr == NULL || addr_string == NULL)
	{
		return;
	}

	UG_NFC_SHARE_DEBUG("string : %s", addr_string);

	UG_NFC_SHARE_MEM_STRDUP(temp, addr_string);
	if (temp != NULL)
	{
		char *token = NULL;
		long value;
		int count = 0;

		token = strtok(temp, ":");

		do
		{
			value = strtol(token, NULL, 16);

			addr[count++] = (uint8_t)value;
		}
		while ((token = strtok(NULL, ":")) != NULL);
	}
	UG_NFC_SHARE_MEM_FREE(temp);
}

static void _p2p_connection_handover_completed_cb(nfc_error_e result, nfc_ac_type_e carrior, void *ac_data, int ac_data_size, void *user_data)
{
	UG_NFC_SHARE_BEGIN();

	ugdata_t* ug_data = (ugdata_t*)user_data;

	/* To prevent write event during showing popup, unset response callback */
	ug_nfc_unset_nfc_callback();

	/* nfc deactivate */
	if(nfc_manager_deinitialize () != NFC_ERROR_NONE)
	{
		UG_NFC_SHARE_DEBUG_ERR("nfc_manager_deinitialize failed");
	}

	if(result == NFC_ERROR_NONE)
	{
		char *data = NULL;

		UG_NFC_SHARE_DEBUG("p2p_connection_handover is completed");

		data = (char *)bundle_get_val(ug_data->bd, "request_data");

		UG_NFC_SHARE_DEBUG("uri[%d] = %s", strlen(data), data);

		if (_bt_ipc_send_obex_message((char *)ac_data, (uint8_t *)data, strlen(data) + 1) == 0)
		{
			_show_status_text(ug_data, IDS_SHARED);
		}
		else
		{
			UG_NFC_SHARE_DEBUG_ERR("_bt_ipc_send_obex_message failed");

			_show_status_text(ug_data, IDS_UNABLE_TO_SHARE);
		}
	}
	else
	{
		UG_NFC_SHARE_DEBUG_ERR("p2p_connection_handover failed");

		_show_status_text(ug_data, IDS_UNABLE_TO_SHARE);
	}

	UG_NFC_SHARE_END();
}

static void _p2p_send_completed_cb(nfc_error_e result, void *user_data)
{
	UG_NFC_SHARE_BEGIN();

	ugdata_t* ug_data = (ugdata_t*)user_data;

	/* To prevent write event during showing popup, unset response callback */
	ug_nfc_unset_nfc_callback();

	/* nfc deactivate */
	if(nfc_manager_deinitialize () != NFC_ERROR_NONE)
	{
		UG_NFC_SHARE_DEBUG_ERR("nfc_manager_deinitialize failed");
	}

	if(result == NFC_ERROR_NONE)
	{
		UG_NFC_SHARE_DEBUG("_p2p_send_completed_cb is completed");

		_show_status_text(ug_data, IDS_SHARED);
	}
	else
	{
		UG_NFC_SHARE_DEBUG_ERR("_p2p_send_completed_cb failed");

		_show_status_text(ug_data, IDS_UNABLE_TO_SHARE);
	}

	UG_NFC_SHARE_END();
}

static void _p2p_target_discovered_cb(nfc_discovered_type_e type, nfc_p2p_target_h target, void *user_data)
{
	UG_NFC_SHARE_BEGIN();

	ugdata_t* ug_data = (ugdata_t*)user_data;

	if(type == NFC_DISCOVERED_TYPE_ATTACHED)
	{
		int result = NFC_ERROR_NONE;

		UG_NFC_SHARE_DEBUG("NFC_DISCOVERED_TYPE_ATTACHED");

		if (ug_nfc_share_get_tag_type() == UG_NFC_SHARE_TAG_HANDOVER)
		{
			UG_NFC_SHARE_DEBUG("UG_NFC_SHARE_TAG_HANDOVER");

			/* The code below will be changed after capi is completed */
			if ((result = nfc_p2p_connection_handover(target, NFC_AC_TYPE_UNKNOWN, _p2p_connection_handover_completed_cb, ug_data)) != NFC_ERROR_NONE)
			{
				UG_NFC_SHARE_DEBUG_ERR("nfc_p2p_connection_handover failed [%d]", result);
			}

			return;
		}
		else
		{
			nfc_ndef_message_h msg = NULL;

			msg = ug_nfc_share_get_current_ndef(ug_data);
			if(msg == NULL)
			{
				UG_NFC_SHARE_DEBUG_ERR("nfc_ndef_message_h is NULL!!");
				return;
			}

			result = nfc_p2p_send(target, msg, _p2p_send_completed_cb, ug_data);
			if(result != NFC_ERROR_NONE)
			{
				UG_NFC_SHARE_DEBUG_ERR("nfc_p2p_send failed[%d]", result);
				return;
			}
		}
	}
	else
	{
		UG_NFC_SHARE_DEBUG("NFC_DISCOVERED_TYPE_DETACHED");
	}

	UG_NFC_SHARE_END();
}

void ug_nfc_set_nfc_callback(void *user_data)
{
	UG_NFC_SHARE_BEGIN();

	nfc_manager_set_p2p_target_discovered_cb(_p2p_target_discovered_cb, user_data);

	UG_NFC_SHARE_END();
}

void ug_nfc_unset_nfc_callback(void)
{
	UG_NFC_SHARE_BEGIN();

	nfc_manager_unset_p2p_target_discovered_cb();

	UG_NFC_SHARE_END();
}

int _bt_ipc_send_obex_message(char *address, const uint8_t *files, uint32_t length)
{
	int result = 0;
	uint32_t i, count = 1;
	E_DBus_Connection *conn = NULL;

	UG_NFC_SHARE_BEGIN();

	if (address == NULL || files == NULL)
	{
		UG_NFC_SHARE_DEBUG_ERR("invalid param [%p] [%p]", address, files);
		return 0;
	}

	/* count files */
	for (i = 0; i < length; i++)
	{
		if (files[i] == '?')
		{
			count++;
		}
	}

	if (e_dbus_init() > 0)
	{
		if ((conn = e_dbus_bus_get(DBUS_BUS_SYSTEM)) != NULL)
		{
			DBusMessage *msg = NULL;

			if (e_dbus_request_name(conn, "User.Bluetooth.UG", 0, NULL, NULL) != NULL)
			{
				if ((msg = dbus_message_new_signal("/org/projectx/connect_device", "User.Bluetooth.UG", "Send")) != NULL)
				{
					int reserved = 0;
					char *type = "nfc";
					char *name = address;
					uint8_t temp[6] = { 0, };
					uint8_t *addr = temp;

					_ug_nfc_share_get_bt_addr_from_string(temp, address);

					UG_NFC_SHARE_DEBUG("msg [%p], reserved [%d], address [%02X:%02X:%02X:%02X:%02X:%02X], count [%d], files [%s]", msg, reserved, addr[0], addr[1], addr[2], addr[3], addr[4], addr[5], count, files);

					if (dbus_message_append_args(msg,
												DBUS_TYPE_INT32, &reserved,
												DBUS_TYPE_ARRAY, DBUS_TYPE_BYTE, &addr, 6,
												DBUS_TYPE_INT32, &count,
												DBUS_TYPE_STRING, &files,
												DBUS_TYPE_STRING, &type,
												DBUS_TYPE_STRING, &name,
												DBUS_TYPE_INVALID))
					{
						e_dbus_message_send(conn, msg, NULL, -1, NULL);

						UG_NFC_SHARE_DEBUG("Send success");
					}
					else
					{
						UG_NFC_SHARE_DEBUG_ERR("Connect sending failed");

						result = -1;
					}

					dbus_message_unref(msg);
				}
				else
				{
					UG_NFC_SHARE_DEBUG_ERR("dbus_message_new_signal failed");

					result = -1;
				}
			}
			else
			{
				UG_NFC_SHARE_DEBUG_ERR("e_dbus_request_name failed");

				result = -1;
			}
		}
		else
		{
			UG_NFC_SHARE_DEBUG_ERR("e_dbus_bus_get failed");

			result = -1;
		}

		e_dbus_shutdown();
	}
	else
	{
		UG_NFC_SHARE_DEBUG_ERR("e_dbus_init failed");

		result = -1;
	}

	UG_NFC_SHARE_END();

	return result;
}
