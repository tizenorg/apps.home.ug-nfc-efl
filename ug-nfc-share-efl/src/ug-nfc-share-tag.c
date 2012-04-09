/*
 * Copyright (c) 2000 - 2012 Samsung Electronics Co., Ltd All Rights Reserved
 *
 * This file is part of the ug-nfc-efl
 * Written by Junyong Sim <junyong.sim@samsung.com>
 *
 * PROPRIETARY/CONFIDENTIAL
 *
 * This software is the confidential and proprietary information of SAMSUNG ELECTRONICS ("Confidential Information").
 * You shall not disclose such Confidential Information and shall use it only in accordance
 * with the terms of the license agreement you entered into with SAMSUNG ELECTRONICS.
 * SAMSUNG make no representations or warranties about the suitability of the software,
 * either express or implied, including but not limited to the implied warranties of merchantability,
 * fitness for a particular purpose, or non-infringement.
 * SAMSUNG shall not be liable for any damages suffered by licensee as a result of using,
 * modifying or distributing this software or its derivatives.
 *
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
#include <appcore-efl.h>

/* external library header */
#include "xdgmime.h"
#include <contacts-svc.h>


#define NFC_POPUP_TIMEOUT               3.0

static ug_nfc_share_tag_type ug_nfc_share_tagType;
static char *ug_nfc_share_displayText = NULL;


int _bt_ipc_send_obex_message(uint8_t *address, const uint8_t *files, uint32_t length);

/*-----------------------------------------------------------------------------------------------*/

static void _failure_popup_response_cb(void *data, Evas_Object *obj, void *event_info)
{
	LOGD("[%s(): %d] BEGIN>>>>", __FUNCTION__, __LINE__);

	int result = (int) event_info;

	// to do : action will be determined
	LOGD("[%s(): %d] END>>>>", __FUNCTION__, __LINE__);
}

static void _show_failure_popup(void *data, char *str)
{
	LOGD("[%s(): %d] BEGIN>>>>", __FUNCTION__, __LINE__);

	char popup_str[POPUP_TEXT_SIZE] = { 0, };

	ugdata_t* ug_data = (ugdata_t*)data;
	ret_if(ug_data == NULL);

	ug_nfc_share_create_popup(ug_data, ug_data->base_layout, popup_str, NULL, 0, NULL, 0, NULL, 0, true, false, _failure_popup_response_cb);

	LOGD("[%s(): %d] END>>>>", __FUNCTION__, __LINE__);
}

static void _success_popup_response_cb(void *data, Evas_Object *obj, void *event_info)
{
	LOGD("[%s(): %d] BEGIN>>>>", __FUNCTION__, __LINE__);

	int result = (int) event_info;

	ugdata_t* ug_data = (ugdata_t*)data;
	ret_if(ug_data == NULL);

	if (result == UG_NFC_POPUP_RESP_OK)
	{
		LOGD("UG_NFC_POPUP_RESP_OK");
	}
	else
	{
		LOGD("UG_NFC_POPUP_RESP_OK NOT!!");
	}

	LOGD("[%s(): %d] END>>>>", __FUNCTION__, __LINE__);
}

static void _show_success_popup(void *data)
{
	LOGD("[%s(): %d] BEGIN>>>>", __FUNCTION__, __LINE__);

	char popup_str[POPUP_TEXT_SIZE] = { 0, };
	char *display_str = NULL;

	ugdata_t* ug_data = (ugdata_t*)data;
	ret_if(ug_data == NULL);

	UG_NFC_SHARE_MEM_STRDUP(display_str, ug_nfc_share_get_display_str());
	if (display_str == NULL)
	{
		UG_NFC_SHARE_MEM_STRDUP(display_str, IDS_NONE);
	}

	snprintf(popup_str, POPUP_TEXT_SIZE, IDS_PS_TAG_SHARED, display_str);

	ug_nfc_share_create_popup(data, ug_data->base_layout, popup_str, IDS_OK, UG_NFC_POPUP_RESP_OK, NULL, 0, NULL, 0, false, false, _success_popup_response_cb);

	UG_NFC_SHARE_MEM_FREE(display_str);

	LOGD("[%s(): %d] END>>>>", __FUNCTION__, __LINE__);
}

char *ug_nfc_share_get_display_str(void)
{
	return ug_nfc_share_displayText;
}

void ug_nfc_share_set_display_str(const char* text)
{
	if (ug_nfc_share_displayText != NULL)
	{
		UG_NFC_SHARE_MEM_FREE(ug_nfc_share_displayText);
	}

	UG_NFC_SHARE_MEM_STRDUP(ug_nfc_share_displayText, text);
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

	LOGD("[%s(): %d] BEGIN >>>>", __FUNCTION__, __LINE__);

	retv_if(ug_data == NULL, NULL);

	LOGD("[%s(): %d] END >>>>", __FUNCTION__, __LINE__);

	return ug_data->current_ndef;
}

ug_nfc_share_result_e ug_nfc_share_set_current_ndef(void *data, nfc_ndef_message_h ndef_msg)
{
	LOGD("[%s(): %d] BEGIN >>>>", __FUNCTION__, __LINE__);

	ugdata_t *ug_data = (ugdata_t *)data;
	if (ug_data == NULL)
	{
		LOGD("[%s(): %d] ug_data is null", __FUNCTION__, __LINE__);
		return UG_NFC_SHARE_ERROR;
	}

	if (ug_data->current_ndef != NULL)
	{
		if (nfc_ndef_message_destroy(ug_data->current_ndef) != NFC_ERROR_NONE)
		{
			LOGD("nfc_ndef_message_destroy failed");
		}
	}

	ug_data->current_ndef = ndef_msg;
	LOGD("[%s(): %d] END >>>>", __FUNCTION__, __LINE__);

	return UG_NFC_SHARE_OK;

}

static ug_nfc_share_result_e ug_nfc_share_get_extension_from_mime_type(const char *mime_type, char *extension, size_t ext_size)
{
	ug_nfc_share_result_e result = UG_NFC_SHARE_ERROR;
#if 1
	size_t length = 0;
	const char *temp_ext = NULL;
#else
	const char **temp_name;
#endif

	LOGD("[%s(): %d] BEGIN >>>>", __FUNCTION__, __LINE__);

	retv_if(mime_type == NULL, result);
	retv_if((extension == NULL || ext_size == 0), result);

	memset(extension, 0, ext_size);

#if 1
	length = strlen(mime_type);

	if (strncmp(mime_type, "image/png", length) == 0)
	{
		temp_ext = ".png";
	}
	else if (strncmp(mime_type, "image/gif", length) == 0)
	{
		temp_ext = ".gif";
	}
	else if (strncmp(mime_type, "image/bmp", length) == 0)
	{
		temp_ext = ".bmp";
	}
	else if (strncmp(mime_type, "image/jpeg", length) == 0)
	{
		temp_ext = ".jpg";
	}
	else if (strncmp(mime_type, "audio/mpeg", length) == 0)
	{
		temp_ext = ".mp3";
	}
	else if (strncmp(mime_type, "audio/midi", length) == 0)
	{
		temp_ext = ".mid";
	}
	else if (strncmp(mime_type, "audio/x-wav", length) == 0)
	{
		temp_ext = ".wav";
	}
	else if (strncmp(mime_type, "video/mplength", length) == 0)
	{
		temp_ext = ".mplength";
	}
	else if (strncmp(mime_type, "video/mpeg", length) == 0)
	{
		temp_ext = ".mpg";
	}
	else if (strncmp(mime_type, "video/quicktime", length) == 0)
	{
		temp_ext = ".mov";
	}
	else if (strncmp(mime_type, "video/x-flv", length) == 0)
	{
		temp_ext = ".flv";
	}
	else if (strncmp(mime_type, "video/x-msvideo", length) == 0)
	{
		temp_ext = ".avi";
	}
	else if (strncmp(mime_type, "video/x-ms-asf", length) == 0)
	{
		temp_ext = ".asf";
	}
	else if (strncmp(mime_type, "video/x-msvideo", length) == 0)
	{
		temp_ext = ".avi";
	}
	else
	{
		temp_ext = "";
	}

	strncpy(extension, temp_ext, ext_size - 1);
	extension[ext_size - 1] = '\0';

#else
	temp_name = xdg_mime_get_file_names_from_mime_type(mime_type);
	LOGD("xdg_mime_get_file_names_from_mime_type(%s)", mime_type);
	if (temp_name != NULL)
	{
		LOGD("temp_name (%p)", temp_name);
		if (temp_name[0] != NULL)
		{
			LOGD("temp_name[0] (%p)", temp_name[0]);
			LOGD("temp_name[0] : %s", temp_name[0]);

			strncpy(extension, temp_name[0], ext_size - 1);
			extension[ext_size - 1] = '\0';
		}
	}
#endif

	LOGD("extension : %s", extension);

	result = UG_NFC_SHARE_OK;

	LOGD("[%s(): %d] END >>>>", __FUNCTION__, __LINE__);

	return result;
}

static ug_nfc_share_result_e ug_nfc_share_get_file_name_from_record(nfc_ndef_record_h record, char *file_name, size_t name_size)
{
	ug_nfc_share_result_e result = UG_NFC_SHARE_ERROR;
	uint8_t *id_buffer = NULL;
	uint32_t id_size = 0;

	LOGD("[%s(): %d] END >>>>", __FUNCTION__, __LINE__);

	retv_if(record == NULL, result);
	retv_if((file_name == NULL || name_size == 0), result);

	result = nfc_ndef_record_get_id(record, &id_buffer, &id_size);
	if (result != NFC_ERROR_NONE)
	{
		LOGD("nfc_ndef_record_get_id failed [%d]", result);
		goto ERROR;
	}

	if (id_buffer == NULL || id_size == 0)
	{
		int last_file_num = 0;
		uint8_t *type_buffer = NULL;
		uint32_t type_size = 0;
		char extension[10] = { 0, };

		/* generate file name */
		/* get record type name - maybe mime type string */
		result = nfc_ndef_record_get_type(record, &type_buffer, &type_size);
		if (result != NFC_ERROR_NONE)
		{
			LOGD("nfc_ndef_record_get_type failed (%d)", result);
			goto ERROR;
		}

		LOGD("type_buffer : %s", type_buffer);

		ug_nfc_share_get_extension_from_mime_type(type_buffer, extension, sizeof(extension));

		/* get current file number */
		vconf_get_int(UG_NFC_SHARE_LAST_FILE_NUMBER_KEY, &last_file_num);
		last_file_num++;
		vconf_set_int(UG_NFC_SHARE_LAST_FILE_NUMBER_KEY, last_file_num);

		/* make file name */
		snprintf(file_name, name_size, "%s%04d%s", UG_NFC_SHARE_FILE_NAME_PREFIX, last_file_num, extension);
	}
	else
	{
		strncpy(file_name, id_buffer, name_size - 1);
		file_name[name_size - 1] = '\0';
	}

	LOGD("file name : %s", file_name);

	result = UG_NFC_SHARE_OK;

ERROR :

	LOGD("[%s(): %d] END >>>>", __FUNCTION__, __LINE__);

	return result;
}

static char *ug_nfc_share_get_display_text_from_record(ug_nfc_share_tag_type record_type, nfc_ndef_record_h record)
{
	int result = UG_NFC_SHARE_ERROR;
	char *disp_text = NULL;
	uint8_t *payload_buffer = NULL;
	uint32_t payload_size = 0;

	LOGD("[%s(): %d] END >>>>", __FUNCTION__, __LINE__);

	switch (record_type)
	{
	case UG_NFC_SHARE_TAG_CONTACT :
		{
			CTSstruct *pCts = NULL;
			CTSvalue *value = NULL;
			char *temp = NULL;

			LOGD("record_type : UG_NFC_SHARE_TAG_CONTACT");

			result = nfc_ndef_record_get_payload(record, &payload_buffer, &payload_size);
			if (result != NFC_ERROR_NONE)
			{
				LOGD("nfc_ndef_record_get_payload failed[%d]", result);
				break;
			}

			if ((result = contacts_svc_get_contact_from_vcard(payload_buffer, &pCts)) == CTS_SUCCESS)
			{
				contacts_svc_struct_get_value(pCts, CTS_CF_NAME_VALUE, &value);
				temp = (char *)contacts_svc_value_get_str(value, CTS_NAME_VAL_FIRST_STR);
				if (temp != NULL)
				{
					UG_NFC_SHARE_MEM_STRDUP(disp_text, temp);
				}
				else
				{
					UG_NFC_SHARE_MEM_STRDUP(disp_text, "NULL");
				}
				LOGD("display_name:%s", disp_text);
			}
			else
			{
				LOGD("contacts_svc_get_contact is failed [result : %d]", result);
			}
		}
		break;

	case UG_NFC_SHARE_TAG_URL :
		LOGD("record_type : UG_NFC_SHARE_TAG_URL");

		result = nfc_ndef_record_get_uri(record, &disp_text);
		if (result != NFC_ERROR_NONE)
		{
			LOGD("nfc_ndef_record_get_uri failed[%d]", result);
			break;
		}
		break;

	case UG_NFC_SHARE_TAG_MPLAYER :
		LOGD("record_type : UG_NFC_SHARE_TAG_MPLAYER");
		break;

	case UG_NFC_SHARE_TAG_MEMO :
		LOGD("record_type : UG_NFC_SHARE_TAG_MEMO");

		result = nfc_ndef_record_get_text(record, &disp_text);
		if (result != NFC_ERROR_NONE)
		{
			LOGD("nfc_ndef_record_get_text failed[%d]", result);
			break;
		}
		break;

	case UG_NFC_SHARE_TAG_BT :
		{
			int name_len = 0;

			LOGD("record_type : UG_NFC_SHARE_TAG_BT");

			result = nfc_ndef_record_get_payload(record, &payload_buffer, &payload_size);
			if (result != NFC_ERROR_NONE)
			{
				LOGD("nfc_ndef_record_get_payload failed[%d]", result);
				break;
			}

			LOGD("buffer:%s", payload_buffer);
			name_len = ((int)(payload_buffer[0]) * 256 + (int)(payload_buffer[1])) - 9;

			UG_NFC_SHARE_MEM_MALLOC(disp_text, name_len, char);
			if (disp_text != NULL)
				memcpy(disp_text, payload_buffer + 9, name_len);
		}
		break;

	case UG_NFC_SHARE_TAG_WIFI :
		LOGD("record_type : UG_NFC_SHARE_TAG_WIFI");
		break;

	case UG_NFC_SHARE_TAG_PROFILE :
		LOGD("record_type : UG_NFC_SHARE_TAG_PROFILE");
		break;

	case UG_NFC_SHARE_TAG_ALLSHARE :
		LOGD("record_type : UG_NFC_SHARE_TAG_ALLSHARE");
		break;

	case UG_NFC_SHARE_TAG_FILE :
		{
			char file_name[1024] = { 0, };

			LOGD("record_type : UG_NFC_SHARE_TAG_FILE");

			/* get file name */
			ug_nfc_share_get_file_name_from_record(record, file_name, sizeof(file_name));
			UG_NFC_SHARE_MEM_STRDUP(disp_text, file_name);
		}
		break;

	case UG_NFC_SHARE_TAG_HANDOVER :
		{
			LOGD("record_type : UG_NFC_SHARE_TAG_HANDOVER");

			/* get file name */
			UG_NFC_SHARE_MEM_STRDUP(disp_text, IDS_HANDOVER);
		}
		break;

	case UG_NFC_SHARE_TAG_UNKNOWN :
		LOGD("record_type : UG_NFC_SHARE_TAG_UNKNOWN");
		break;

	default :
		break;
	}

	LOGD("[%s(): %d] END >>>>", __FUNCTION__, __LINE__);

	return disp_text;
}

char *ug_nfc_share_get_display_text_from_ndef(ug_nfc_share_tag_type record_type, nfc_ndef_message_h ndef_msg)
{
	int result = UG_NFC_SHARE_ERROR;
	char *disp_text = NULL;
	nfc_ndef_record_h record = NULL;
	int count = 0;

	LOGD("[%s(): %d] BEGIN >>>>", __FUNCTION__, __LINE__);

	result = nfc_ndef_message_get_record_count(ndef_msg, &count);
	if (result != NFC_ERROR_NONE)
	{
		LOGD("nfc_ndef_message_get_record_count failed[%d]", result);
		goto error;
	}
	LOGD("count = [%d]", count);

	//If count>2, we should implement for this case.
	result = nfc_ndef_message_get_record(ndef_msg, 0, &record);
	if (result != NFC_ERROR_NONE)
	{
		LOGD("nfc_ndef_message_get_record failed[%d]", result);
		goto error;
	}

	disp_text = ug_nfc_share_get_display_text_from_record(record_type, record);

error :

	LOGD("[%s(): %d] END >>>>", __FUNCTION__, __LINE__);

	return disp_text;
}

static ug_nfc_share_result_e ug_nfc_share_make_mime_type_data_from_file_path(const char *path, uint8_t **type_data, uint32_t *type_size)
{
	ug_nfc_share_result_e result = UG_NFC_SHARE_ERROR;
#if 0
	char *extension = NULL;
	char *mime_str = NULL;
	int length = 0;
#else
	struct stat st;
	const char *mime_type;
#endif

	LOGD("[%s(): %d] BEGIN >>>>", __FUNCTION__, __LINE__);

	retv_if(path == NULL, result);
	retv_if(type_data == NULL, result);

#if 0
	length = strlen(path);

	if (length > 5 && strncmp(mime_type, ".jpeg", 5) == 0)
	{
		memcpy(*type_data, STRING_AND_SIZE("image/jpeg"));
		*type_size = strlen("image/jpeg");
	}
	else if (length > 5 && strncmp(mime_type, ".mpeg", 5) == 0)
	{
		memcpy(*type_data, STRING_AND_SIZE("image/mpeg"));
		*type_size = strlen("image/mpeg");
	}
	else if (length > 4 && strncmp(mime_type, ".png", 4) == 0)
	{
		memcpy(*type_data, STRING_AND_SIZE("image/png"));
		*type_size = strlen("image/png");
	}
	else if (length > 4 && strncmp(mime_type, ".gif", 4) == 0)
	{
		memcpy(*type_data, STRING_AND_SIZE("image/gif"));
		*type_size = strlen("image/gif");
	}
	else if (length > 4 && strncmp(mime_type, ".bmp", 4) == 0)
	{
		memcpy(*type_data, STRING_AND_SIZE("image/bmp"));
		*type_size = strlen("image/bmp");
	}
	else if (length > 4 && ((strncmp(mime_type, ".jpg", 4) == 0) || (strncmp(mime_type, ".jpe", 4) == 0)))
	{
		memcpy(*type_data, STRING_AND_SIZE("image/jpeg"));
		*type_size = strlen("image/jpeg");
	}
	else if (length > 4 && strncmp(mime_type, ".mp3", 4) == 0)
	{
		memcpy(*type_data, STRING_AND_SIZE("audio/mpeg"));
		*type_size = strlen("audio/mpeg");
	}
	else if (length > 4 && strncmp(mime_type, ".mid", 4) == 0)
	{
		memcpy(*type_data, STRING_AND_SIZE("audio/midi"));
		*type_size = strlen("audio/midi");
	}
	else if (length > 4 && strncmp(mime_type, ".wav", 4) == 0)
	{
		memcpy(*type_data, STRING_AND_SIZE("audio/x-wav"));
		*type_size = strlen("audio/x-wav");
	}
	else if (length > 4 && strncmp(mime_type, ".mp4", 4) == 0)
	{
		memcpy(*type_data, STRING_AND_SIZE("video/mp4"));
		*type_size = strlen("video/mp4");
	}
	else if (length > 4 && ((strncmp(mime_type, ".mpe", 4) == 0) || (strncmp(mime_type, ".mpg", 4) == 0)))
	{
		memcpy(*type_data, STRING_AND_SIZE("video/mpeg"));
		*type_size = strlen("video/mpeg");
	}
	else if (length > 4 && strncmp(mime_type, ".mov", 4) == 0)
	{
		memcpy(*type_data, STRING_AND_SIZE("video/quicktime"));
		*type_size = strlen("video/quicktime");
	}
	else if (length > 4 && strncmp(mime_type, ".flv", 4) == 0)
	{
		memcpy(*type_data, STRING_AND_SIZE("video/x-flv"));
		*type_size = strlen("video/x-flv");
	}
	else if (length > 4 && strncmp(mime_type, ".avi", 4) == 0)
	{
		memcpy(*type_data, STRING_AND_SIZE("video/x-msvideo"));
		*type_size = strlen("video/x-msvideo");
	}
	else if (length > 4 && strncmp(mime_type, ".asf", 4) == 0)
	{
		memcpy(*type_data, STRING_AND_SIZE("video/x-ms-asf"));
		*type_size = strlen("video/x-ms-asf");
	}
	else if (length > 4 && strncmp(mime_type, ".avi", 4) == 0)
	{
		memcpy(*type_data, STRING_AND_SIZE("video/x-msvideo"));
		*type_size = strlen("video/x-msvideo");
	}
	else
	{
		memcpy(*type_data, STRING_AND_SIZE("application/octet-stream"));
		*type_size = strlen("application/octet-stream");
	}
#else
//	mime_type = xdg_mime_get_mime_type_for_file(path, &st);
	mime_type = xdg_mime_get_mime_type_from_file_name(path);
	UG_NFC_SHARE_MEM_MALLOC(*type_data, strlen(mime_type), uint8_t);
	memcpy(*type_data, mime_type, strlen(mime_type));
	*type_size = strlen(mime_type);
#endif

	LOGD("mime type : %s", GET_SAFE_STRING((char *)*type_data));

	result = UG_NFC_SHARE_OK;

	LOGD("[%s(): %d] END >>>>", __FUNCTION__, __LINE__);

	return result;
}

ug_nfc_share_result_e ug_nfc_share_make_ndef_message_from_file(nfc_ndef_message_h *msg, const char *path)
{
	ug_nfc_share_result_e result = UG_NFC_SHARE_ERROR;
	struct stat st;
	uint8_t *type_buffer = NULL;
	uint32_t type_size = 0;
	nfc_ndef_record_h record = NULL;
	FILE *file = NULL;
	char *file_name = NULL;
	uint8_t *file_data = NULL;
	long int file_len = 0;


	LOGD("[%s(): %d] BEGIN >>>>", __FUNCTION__, __LINE__);

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
			LOGD("ERROR :: UG_NFC_SHARE_MEM_MALLOC failed");

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

		LOGD("fread(%s) success, size %ld\n", path, file_len);
	}
	else
	{
		LOGD("fopen(%s) error\n");

		return result;
	}

	/* get type data */
	result = ug_nfc_share_make_mime_type_data_from_file_path(path, &type_buffer, &type_size);
	if (result != UG_NFC_SHARE_OK)
	{
		LOGD("ERROR :: _make_mime_type_data_from_file_path failed [%d]", result);

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

	LOGD("file name : %s", file_name);

	/* create record */
	result = nfc_ndef_record_create(&record, NFC_RECORD_TNF_MIME_MEDIA, type_buffer, type_size, file_name, strlen(file_name), file_data, file_len);
	if (result != NFC_ERROR_NONE)
	{
		LOGD("nfc_ndef_record_create failed (%d)", result);

		return result;
	}

	/* make file NDEF message */
	result = nfc_ndef_message_create(msg);
	if (result != NFC_ERROR_NONE)
	{
		LOGD("nfc_ndef_message_create failed [%d]\n", result);

		nfc_ndef_record_destroy(record);

		return result;
	}

	/* append record to ndef message */
	result = nfc_ndef_message_append_record(*msg, record);
	if (result != NFC_ERROR_NONE)
	{
		LOGD("nfc_ndef_message_append_record failed (%d)", result);

		return result;
	}

	LOGD("ug_nfc_share_make_ndef_message_from_file success");

	LOGD("[%s(): %d] END>>>>", __FUNCTION__, __LINE__);

	return result;
}

/* nfc_handover */
ug_nfc_share_result_e ug_nfc_share_make_ndef_message_from_multi_file(nfc_ndef_message_h *msg, const char *path[], int record_count)
{
	ug_nfc_share_result_e result = UG_NFC_SHARE_ERROR;
	struct stat st;
	uint8_t *type_buffer = NULL;
	uint32_t type_size = 0;
	nfc_ndef_record_h record = NULL;
	FILE *file = NULL;
	char *file_name = NULL;
	uint8_t *file_data = NULL;
	long int file_len = 0;
	int index;


	LOGD("[%s(): %d] BEGIN >>>>", __FUNCTION__, __LINE__);

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
				LOGD("ERROR :: UG_NFC_SHARE_MEM_MALLOC failed");

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

			LOGD("fread(%s) success, size %ld\n", path[index], file_len);
		}
		else
		{
			LOGD("fopen(%s) error\n");

			return result;
		}

		/* get type data */
		result = ug_nfc_share_make_mime_type_data_from_file_path(path[index], &type_buffer, &type_size);
		if (result != UG_NFC_SHARE_OK)
		{
			LOGD("ERROR :: _make_mime_type_data_from_file_path failed [%d]", result);

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

		LOGD("file name : %s", file_name);

		/* create record */
		result = nfc_ndef_record_create(&record, NFC_RECORD_TNF_MIME_MEDIA, type_buffer, type_size, file_name, strlen(file_name), file_data, file_len);
		if (result != NFC_ERROR_NONE)
		{
			LOGD("nfc_ndef_record_create failed (%d)", result);

			return result;
		}

		/* make file NDEF message */
		result = nfc_ndef_message_create(msg);
		if (result != NFC_ERROR_NONE)
		{
			LOGD("nfc_ndef_message_create failed [%d]\n", result);

			nfc_ndef_record_destroy(record);

			return result;
		}

		/* append record to ndef message */
		result = nfc_ndef_message_append_record(*msg, record);
		if (result != NFC_ERROR_NONE)
		{
			LOGD("nfc_ndef_message_append_record failed (%d)", result);

			return result;
		}

		LOGD("ug_nfc_share_make_ndef_message_from_file success");

		LOGD("[%s(): %d] END>>>>", __FUNCTION__, __LINE__);
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

	LOGD("string : %s", addr_string);

	UG_NFC_SHARE_MEM_STRNDUP(temp, addr_string, strlen(addr_string));
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
		while (token = strtok(NULL, ":"));
	}
}

static void _p2p_connection_handover_completed_cb(nfc_error_e result, nfc_ac_type_e carrior, void *ac_data, int ac_data_size, void *user_data)
{
	LOGD("[%s(): %d] BEGIN>>>>", __FUNCTION__, __LINE__);

	ugdata_t* ug_data = (ugdata_t*)user_data;
	uint8_t *data = (uint8_t *)ac_data;

	if(result == NFC_ERROR_NONE)
	{
		uint8_t address[6] = { 0, };

		LOGD("p2p_connection_handover is completed");
		LOGD("uri[%d] = %s", strlen(ug_data->uri), ug_data->uri);

		/* To prevent write event during showing popup, unset response callback */
		ug_nfc_unset_nfc_callback();

		_ug_nfc_share_get_bt_addr_from_string(address, (char *)ac_data);

		LOGD("address[%d] = { 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X }", sizeof(address), address[0], address[1], address[2], address[3], address[4], address[5]);

		if (_bt_ipc_send_obex_message(address, ug_data->uri, strlen(ug_data->uri)) == 0)
		{
			/* show success popup */
			_show_success_popup(ug_data);
		}
		else
		{
			LOGD("_bt_ipc_send_obex_message failed");

			/* show failure popup */
			_show_failure_popup(ug_data, IDS_FAILED_TO_SHARE_TAG);
		}
	}
	else
	{
		LOGD("p2p_connection_handover failed");

		/* show failure popup */
		_show_failure_popup(ug_data, IDS_FAILED_TO_SHARE_TAG);
	}

	LOGD("[%s(): %d] END>>>>", __FUNCTION__, __LINE__);
}

static void _p2p_send_completed_cb(nfc_error_e result, void *user_data)
{
	LOGD("[%s(): %d] BEGIN>>>>", __FUNCTION__, __LINE__);

	ugdata_t* ug_data = (ugdata_t*)user_data;

	if(result == NFC_ERROR_NONE)
	{
		LOGD("_p2p_send_completed_cb is completed");

		/* To prevent write event during showing popup, unset response callback */
		ug_nfc_unset_nfc_callback();

		/* show success popup */
		_show_success_popup(ug_data);
	}
	else
	{
		LOGD("_p2p_send_completed_cb failed");

		/* show failure popup */
		_show_failure_popup(ug_data, IDS_FAILED_TO_SHARE_TAG);
	}

	LOGD("[%s(): %d] END>>>>", __FUNCTION__, __LINE__);
}

static void _p2p_target_discovered_cb(nfc_discovered_type_e type, nfc_p2p_target_h target, void *user_data)
{
	LOGD("[%s(): %d] BEGIN>>>>", __FUNCTION__, __LINE__);

	ugdata_t* ug_data = (ugdata_t*)user_data;

	if(type == NFC_DISCOVERED_TYPE_ATTACHED)
	{
		int result = NFC_ERROR_NONE;

		LOGD("NFC_DISCOVERED_TYPE_ATTACHED");

		if (ug_nfc_share_get_tag_type() == UG_NFC_SHARE_TAG_HANDOVER)
		{
			LOGD("UG_NFC_SHARE_TAG_HANDOVER\n");

			/* The code below will be changed after capi is completed */
			if ((result = nfc_p2p_connection_handover(target, NFC_AC_TYPE_UNKNOWN, _p2p_connection_handover_completed_cb, ug_data)) != NFC_ERROR_NONE)
			{
				LOGD("nfc_p2p_connection_handover failed [%d]", result);
			}

			return;
		}
		else
		{
			nfc_ndef_message_h msg = NULL;

			msg = ug_nfc_share_get_current_ndef(ug_data);
			if(msg == NULL)
			{
				LOGD("nfc_ndef_message_h is NULL!!\n");
				return;
			}

			result = nfc_p2p_send(target, msg, _p2p_send_completed_cb, ug_data);
			if(result != NFC_ERROR_NONE)
			{
				LOGD("nfc_p2p_send failed[%d]\n", result);
				return;
			}
		}
	}
	else
	{
		LOGD("NFC_DISCOVERED_TYPE_DETACHED");
	}

	LOGD("[%s(): %d] END>>>>", __FUNCTION__, __LINE__);
}

void ug_nfc_set_nfc_callback(void *user_data)
{
	LOGD("[%s(): %d] BEGIN>>>>", __FUNCTION__, __LINE__);

	nfc_manager_set_p2p_target_discovered_cb(_p2p_target_discovered_cb, user_data);

	LOGD("[%s(): %d] END>>>>", __FUNCTION__, __LINE__);
}

void ug_nfc_unset_nfc_callback(void)
{
	LOGD("[%s(): %d] BEGIN>>>>", __FUNCTION__, __LINE__);

	nfc_manager_unset_p2p_target_discovered_cb();

	LOGD("[%s(): %d] END>>>>", __FUNCTION__, __LINE__);
}

int _bt_ipc_send_obex_message(uint8_t *address, const uint8_t *files, uint32_t length)
{
	int result = 0;
	uint32_t i, count = 1;
	E_DBus_Connection *conn = NULL;

	LOGD("[%s(): %d] BEGIN>>>>", __FUNCTION__, __LINE__);

	if (address == NULL || files == NULL)
	{
		LOGD("invalid param [%p] [%p]", address, files);
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

					LOGD("msg [%p], reserved [%d], address [%02X:%02X:%02X:%02X:%02X:%02X], count [%d], files [%s]", msg, reserved, address[0], address[1], address[2], address[3], address[4], address[5], count, files);

					if (dbus_message_append_args(msg,
												DBUS_TYPE_INT32, &reserved,
												DBUS_TYPE_ARRAY, DBUS_TYPE_BYTE, &address, 6,
												DBUS_TYPE_INT32, &count,
												DBUS_TYPE_STRING, &files,
												DBUS_TYPE_STRING, &type,
												DBUS_TYPE_INVALID))
					{
						e_dbus_message_send(conn, msg, NULL, -1, NULL);

						LOGD("Send success");
					}
					else
					{
						LOGE("Connect sending failed");

						result = -1;
					}

					dbus_message_unref(msg);
				}
				else
				{
					LOGE("dbus_message_new_signal failed");

					result = -1;
				}
			}
			else
			{
				LOGE("e_dbus_request_name failed");

				result = -1;
			}

			e_dbus_connection_close(conn);
		}
		else
		{
			LOGE("e_dbus_bus_get failed");

			result = -1;
		}

		e_dbus_shutdown();
	}
	else
	{
		LOGE("e_dbus_init failed");

		result = -1;
	}

	LOGD("[%s(): %d] END>>>>", __FUNCTION__, __LINE__);

	return result;
}
