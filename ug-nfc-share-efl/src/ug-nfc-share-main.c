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

#include "ug-nfc-share-main.h"
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

#include <assert.h>
#include <glib.h>


#ifndef UG_MODULE_API
#define UG_MODULE_API __attribute__ ((visibility("default")))
#endif

 /* nfc_handover */
#define NET_NFC_SHARE_SEPERATOR "?"
#define NET_NFC_SHARE_REQUEST_TYPE_BUFFER "data_buffer"
#define NET_NFC_SHARE_REQUEST_TYPE_PATH "file_path"

#define NFC_SETTING_KEY_NAME	"db/nfc/enable"

#define DISPLAY_STRING_MAX_SIZE		90
#define ABBREVIATION_TEXT			"..."

void ug_nfc_share_create_base_view(void *user_data);
void ug_nfc_share_create_nfc_share_view(void *user_data);
bool ug_nfc_share_check_nfc_isAvailable(void *user_data);
void ug_nfc_share_create_data(struct ui_gadget *ug, bundle *bd, ugdata_t* ug_data);

static Evas_Object *_subwindow = NULL;

/******************* UI Functions *************/
static Evas_Object *_create_button(Evas_Object *parent, void(*func)(void *data, Evas_Object *obj, void *event_info), const char *label, const char *swallow, const char *style, void *data)
{
	Evas_Object *button;

	retv_if(parent == NULL, NULL);

	button = elm_button_add(parent);
	if (style != NULL)
	{
		elm_object_style_set(button, style);
	}

	if (label != NULL)
	{
		elm_object_text_set(button, label);
	}

	evas_object_smart_callback_add(button, "clicked", func, (void*)data);
	evas_object_show(button);
	elm_object_part_content_set(parent, swallow, button);

	return button;
}

static Evas_Object *_create_image(Evas_Object *parent, const char *file_path, const char *swallow, void *data)
{
	Evas_Object *image;


	retv_if(parent == NULL, NULL);
	retv_if(file_path == NULL, NULL);
	retv_if(swallow == NULL, NULL);

	image = elm_image_add(parent);
	retv_if(image == NULL, NULL);

	elm_image_file_set(image, file_path, NULL);
	elm_image_no_scale_set(image, 1);
	elm_image_resizable_set(image, 0, 0);
	elm_object_part_content_set(parent, swallow, image);


	return image;
}

static Evas_Object *_create_label(Evas_Object *parent, const char *text, const char *swallow, void *data)
{
	Evas_Object *label;

	retv_if(parent == NULL, NULL);

	label = elm_label_add(parent);
	elm_object_part_content_set(parent, swallow, label);
	elm_object_text_set(label, text);

	return label;
}

static Evas_Object *_create_box(Evas_Object *parent, bool is_hori)
{
	Evas_Object *box;

	retv_if(parent == NULL, NULL);

	box = elm_box_add(parent);
	evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(box, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(box);

	return box;
}

static Evas_Object *_create_scroller(Evas_Object *parent)
{
	Evas_Object *scroller;

	retv_if(parent == NULL, NULL);

	scroller = elm_scroller_add(parent);

	elm_scroller_bounce_set(scroller, EINA_FALSE, EINA_FALSE);
	elm_scroller_policy_set(scroller, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);
	evas_object_show(scroller);
	elm_object_part_content_set(parent, "elm.swallow.content", scroller);

	return scroller;
}

static Evas_Object *_create_layout(Evas_Object *parent, const char *clas, const char *group, const char *style, bool signal)
{
	Evas_Object *ly;


	retv_if(parent == NULL, NULL);

	ly = elm_layout_add(parent);

	elm_layout_theme_set(ly, clas, group, style);
	evas_object_size_hint_weight_set(ly, EVAS_HINT_EXPAND, 0.0);
	evas_object_size_hint_align_set(ly, EVAS_HINT_FILL, 0.0);

	evas_object_show(ly);


	return ly;
}

static Evas_Object *_create_bg(Evas_Object *win)
{
	Evas_Object *bg = elm_bg_add(win);

	evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(bg, EVAS_HINT_FILL, EVAS_HINT_FILL);

	elm_win_resize_object_add(win, bg);

	evas_object_show(bg);

	return bg;
}

static Evas_Object *_create_main_layout(Evas_Object* parent)
{
	Evas_Object *layout;

	if (parent == NULL)
		return NULL;

	layout = elm_layout_add(parent);

	elm_layout_theme_set(layout, "layout", "application", "default");

	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(parent, layout);

	evas_object_show(layout);

	return layout;
}

static void _win_del(void *data, Evas_Object *obj, void *event_info)
{
	LOGD("[%s(): %d] BEGIN >>>>", __FUNCTION__, __LINE__);

	LOGD("[%s(): %d] END >>>>", __FUNCTION__, __LINE__);
}

static Evas_Object* _create_win(Evas_Object * parent)
{
	Evas_Object *eo;
	int w, h;

	eo = elm_win_add(parent, "NFCShareSubWindow", ELM_WIN_BASIC);
	if (eo)
	{
		elm_win_title_set(eo, "NFCShareSubWindow");
		elm_win_borderless_set(eo, EINA_TRUE);
		evas_object_smart_callback_add(eo, "delete,request", _win_del, NULL);
		ecore_x_window_size_get(ecore_x_window_root_first_get(), &w, &h);
		evas_object_resize(eo, w, h);
		elm_win_indicator_mode_set(eo, EINA_TRUE);
	}

	return eo;
}

/* nfc_handover required to multi file name */
static bool _get_display_text(ug_nfc_share_tag_type type, nfc_ndef_message_h ndef_msg, char *text)
{
	char *temp_text = NULL;

	LOGD("[%s(): %d] BEGIN >>>>", __FUNCTION__, __LINE__);

	retv_if(ndef_msg == NULL, FALSE);

	if (type == UG_NFC_SHARE_TAG_HANDOVER)
	{
		char *file_name = NULL;
		nfc_ndef_record_h record = NULL;
		uint8_t *payload_buffer = NULL;
		uint32_t payload_size = 0;
		int result = 0;
		int count = 0;


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

		result = nfc_ndef_record_get_payload(record, &payload_buffer, &payload_size);
		if (result != NFC_ERROR_NONE)
		{
			LOGD("nfc_ndef_record_get_payload failed[%d]\n", result);
			goto error;
		}

		if (payload_buffer != NULL && strlen(payload_buffer) > 0)
		{
			/* write file name into id */
			file_name = strrchr(payload_buffer, '/');
			if (file_name == NULL)
			{
				file_name = (char *)payload_buffer;
			}
			else
			{
				file_name++;
			}

			UG_NFC_SHARE_MEM_STRDUP(temp_text, file_name);
		}
	}
	else
	{
		LOGD("->>>>>>>>>>>>>>>>>>>. type is--  %d  ",type);
		temp_text = ug_nfc_share_get_display_text_from_ndef(type, ndef_msg);
	}

	if (temp_text == NULL)
	{
		UG_NFC_SHARE_MEM_STRDUP(temp_text, IDS_UNKNOWN);
	}

	ug_nfc_share_set_display_str(temp_text);

	char display_text[100];

	memset(display_text, 0x00, sizeof(display_text));

	if (strlen(temp_text) > DISPLAY_STRING_MAX_SIZE)
	{
		memcpy(display_text, temp_text, DISPLAY_STRING_MAX_SIZE);
		memcpy(display_text+DISPLAY_STRING_MAX_SIZE, ABBREVIATION_TEXT, strlen(ABBREVIATION_TEXT));
	}
	else
	{
		memcpy(display_text, temp_text, strlen(temp_text));
	}


	snprintf(text, LABEL_TEXT_SIZE, IDS_PS_SELECTED, display_text);

	UG_NFC_SHARE_MEM_FREE(temp_text);

error:
	LOGD("[%s(): %d] END >>>>", __FUNCTION__, __LINE__);

	return TRUE;
}

static void _handover_popup_response_cb(void *data, Evas_Object *obj, void *event_info)
{
	LOGD("[%s(): %d] BEGIN>>>>", __FUNCTION__, __LINE__);

	ugdata_t *ug_data = (ugdata_t *)data;
	ret_if(ug_data == NULL);

	nfc_ndef_message_h msg = NULL;
	nfc_ndef_record_h record = NULL;

	int result = (int) event_info;

	/* remove subwindow */
	if (_subwindow)
		evas_object_del(_subwindow);

	switch (result)
	{
	case UG_NFC_POPUP_RESP_OK:
		{
			char temp_data[1024] = { 0, };

			LOGD("ELM_POPUP_RESPONSE_OK");

			snprintf(temp_data, sizeof(temp_data), "%s", ug_data->uri);
			LOGD("new uri : %s\n", temp_data);

			/* create record */
			result = nfc_ndef_record_create_mime(&record, "file", temp_data, strlen(temp_data));
			if (result != NFC_ERROR_NONE)
			{
				LOGD("nfc_ndef_record_create failed (%d)\n", result);
				return;
			}

			/* create ndef msg*/
			result = nfc_ndef_message_create(&msg);
			if (result != NFC_ERROR_NONE)
			{
				LOGD("nfc_ndef_message_create failed (%d)\n", result);
				return;
			}

			/* append record to ndef msg */
			result = nfc_ndef_message_append_record(msg, record);
			if (result != NFC_ERROR_NONE)
			{
				LOGD("nfc_ndef_message_create failed (%d)\n", result);
				return;
			}
		}
		break;

	case UG_NFC_POPUP_RESP_CANCEL:
		{
			LOGD("ELM_POPUP_RESPONSE_CANCEL");

#if 0
			/* destroy UG */
			LOGD("ug_destroy_me >>>>", __FUNCTION__, __LINE__);
			ug_destroy_me(ug_data->nfc_share_ug);
#endif
			result = ug_nfc_share_make_ndef_message_from_file(&msg, ug_data->uri);
			if (result != UG_NFC_SHARE_OK)
			{
				LOGD("nfc_app_make_ndef_message_from_file failed (%d)\n", result);
				return;
			}

			/* change type : UG_NFC_SHARE_TAG_HANDOVER -> UG_NFC_SHARE_TAG_FILE */
			ug_nfc_share_set_tag_type(UG_NFC_SHARE_TAG_FILE);

			if (ug_data->uri != NULL)
			{
				UG_NFC_SHARE_MEM_FREE(ug_data->uri);
				ug_data->uri = NULL;
			}
		}
		break;
	default:
		return;
	}

	if (!ug_nfc_share_set_current_ndef(ug_data, msg))
	{
		LOGD("ug_nfc_share_set_current_ndef success\n");
	}
	else
	{
		LOGD("ug_nfc_share_set_current_ndef failed (%d)\n", result);
		return;
	}

	ug_nfc_share_create_nfc_share_view(ug_data);

	LOGD("[%s(): %d] END>>>>", __FUNCTION__, __LINE__);
}


static void _show_handover_popup(void *data)
{
	LOGD("[%s(): %d] BEGIN>>>>", __FUNCTION__, __LINE__);

	ugdata_t *ug_data = (ugdata_t *)data;
	ret_if(ug_data == NULL);

	char popup_str[POPUP_TEXT_SIZE] = {0,};

	_subwindow = _create_win(ug_data->ug_win_main);
	ret_if(_subwindow == NULL);

	memcpy(popup_str, IDS_UNABLE_TO_SHARE_FILES_MAXIMUM_FILE_SIZE_EXCEEDED_SHARE_FILES_VIA_BLUETOOTH_PAIRED_DEVICES_MAY_BE_DISCONNECTED_CONTINUE_Q, \
		strlen(IDS_UNABLE_TO_SHARE_FILES_MAXIMUM_FILE_SIZE_EXCEEDED_SHARE_FILES_VIA_BLUETOOTH_PAIRED_DEVICES_MAY_BE_DISCONNECTED_CONTINUE_Q));

	ug_nfc_share_create_popup(ug_data, _subwindow, popup_str, IDS_YES, UG_NFC_POPUP_RESP_OK, IDS_NO, UG_NFC_POPUP_RESP_CANCEL, NULL, 0, false, false, _handover_popup_response_cb);

	evas_object_show(_subwindow);

	LOGD("[%s(): %d] END>>>>", __FUNCTION__, __LINE__);
}

static void _setting_on_YesNo_popup_response_cb(void *data, Evas_Object *obj, void *event_info)
{
	LOGD("[%s(): %d] BEGIN>>>>", __FUNCTION__, __LINE__);

	ugdata_t *ug_data = (ugdata_t *)data;
	ret_if(ug_data == NULL);

	int result = (int) event_info;

	/* remove subwindow */
	if (_subwindow)
		evas_object_del(_subwindow);

	switch (result)
	{
		case UG_NFC_POPUP_RESP_OK:
			/* setting is on */
			LOGD("setting is on >>>>", __FUNCTION__, __LINE__);
			nfc_manager_set_activation(TRUE, NULL,NULL);

			/* check if handover is needed */
			if (ug_nfc_share_get_tag_type() == UG_NFC_SHARE_TAG_HANDOVER) {
				_show_handover_popup(ug_data);
				return;
			}

			/* create share view */
			ug_nfc_share_create_nfc_share_view(ug_data);

			break;

		case UG_NFC_POPUP_RESP_CANCEL:
			/* destroy UG */
			LOGD("ug_destroy_me >>>>", __FUNCTION__, __LINE__);
			ug_destroy_me(ug_data->nfc_share_ug);

			break;

		default:
			break;
	}

	LOGD("[%s(): %d] END>>>>", __FUNCTION__, __LINE__);
}

static void _setting_on_YesNo_popup(void *data)
{
	LOGD("[%s(): %d] BEGIN>>>>", __FUNCTION__, __LINE__);

	ugdata_t *ug_data = (ugdata_t *)data;
	ret_if(ug_data == NULL);

	char popup_str[POPUP_TEXT_SIZE] = {0,};

	_subwindow = _create_win(ug_data->ug_win_main);
	ret_if(_subwindow == NULL);

	memcpy(popup_str, IDS_SERVICE_NOT_AVAILABLE_NFC_TURNED_OFF_TURN_ON_NFC_Q, strlen(IDS_SERVICE_NOT_AVAILABLE_NFC_TURNED_OFF_TURN_ON_NFC_Q));

	ug_nfc_share_create_popup(ug_data, _subwindow, popup_str, IDS_YES, UG_NFC_POPUP_RESP_OK, IDS_NO, UG_NFC_POPUP_RESP_CANCEL, NULL, 0, false, false, _setting_on_YesNo_popup_response_cb);

	evas_object_show(_subwindow);

	LOGD("[%s(): %d] END>>>>", __FUNCTION__, __LINE__);
}

static void _back_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	LOGD("[%s(): %d] BEGIN >>>>", __FUNCTION__, __LINE__);

	/* nfc deactivate */
	if(nfc_manager_deinitialize () != NFC_ERROR_NONE)
	{
		LOGD("nfc_manager_deinitialize failed", __FUNCTION__, __LINE__);
	}

	/* destroy UG */
	ug_destroy_me(data);

	LOGD("[%s(): %d] END>>>>", __FUNCTION__, __LINE__);
}

static Evas_Object *ug_nfc_share_create_layout(void *data)
{
	LOGD("[%s(): %d] BEGIN >>>>", __FUNCTION__, __LINE__);

	ugdata_t* ug_data = (ugdata_t*)data;

	Evas_Object *layout_base = NULL;
	Evas_Object *layout = NULL;
	Evas_Object *scroller = NULL;
	Evas_Object *box_base = NULL;
	nfc_ndef_message_h msg = NULL;
	ug_nfc_share_tag_type type = UG_NFC_SHARE_TAG_MAX;
	char label_string[LABEL_TEXT_SIZE] = {0,};
	char *file_path = NULL;


	/* create base layout */
	layout_base = elm_layout_add(ug_data->base_naviframe);
	retv_if(layout_base == NULL, NULL);
	elm_layout_theme_set(layout_base, "layout", "application", "noindicator");
	evas_object_size_hint_weight_set(layout_base, EVAS_HINT_EXPAND, 0.0);
	evas_object_size_hint_align_set(layout_base, EVAS_HINT_FILL, 0.0);
	evas_object_show(layout_base);
	retv_if(layout_base == NULL, NULL);


	/* Add scroller */
	scroller = _create_scroller(layout_base);
	retv_if(scroller == NULL, NULL);


	/* Add box_base */
	box_base = _create_box(layout_base, FALSE);
	retv_if(box_base == NULL, NULL);


	/* create layout for edc */
	layout = elm_layout_add(layout_base);
	elm_layout_file_set(layout, EDJ_FILE, "share_via_nfc");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(layout);


	/* get data */
	msg = ug_nfc_share_get_current_ndef(ug_data);
	type = ug_nfc_share_get_tag_type();


	/* create label name */
	if(!_get_display_text(type, msg, label_string))
	{
		return NULL;
	}

	Evas_Object *label_name = NULL;
	label_name = _create_label(layout, label_string, "label_name", NULL);
	retv_if(label_name == NULL, NULL);
	elm_label_line_wrap_set(label_name, ELM_WRAP_WORD);
	evas_object_show(label_name);


	/* create message label */
	Evas_Object *label_message = NULL;
	label_message = _create_label(layout, IDS_GENTLY_TOUCH_PHONES_TOGETHER_TO_SHARE, "label_message", NULL);
	retv_if(label_message == NULL, NULL);
	elm_label_line_wrap_set(label_message, ELM_WRAP_WORD);
	evas_object_show(label_message);


	file_path = IMAGES_PATH"/U05_AfterSelect_Share_help.png";


	/* create image */
	Evas_Object *image = NULL;
	image = _create_image(layout, file_path, "touch_image", ug_data);
	retv_if(image == NULL, NULL);
	evas_object_show(image);


	/* create cancel message label */
	Evas_Object *label_cancel_message = NULL;
	label_cancel_message = _create_label(layout, IDS_TO_CANCEL_SHARING_TAP_CANCEL, "label_cancel_message", NULL);
	retv_if(label_cancel_message == NULL, NULL);
	elm_label_line_wrap_set(label_cancel_message, ELM_WRAP_WORD);
	evas_object_show(label_cancel_message);


	/* create cancel button */
	Evas_Object *cancel_button = NULL;
	cancel_button = _create_button(layout, _back_clicked_cb, IDS_CANCEL, "cancel_button", "style1", ug_data->nfc_share_ug);
	retv_if(cancel_button == NULL, NULL);
	evas_object_show(cancel_button);

	/* push layout to box and add scroller to box */
	elm_box_pack_end(box_base, layout);
	elm_object_content_set(scroller, box_base);

	if(NFC_ERROR_NONE == nfc_manager_initialize(NULL,NULL))
	{
		LOGD("nfc_manager_initialize success\n");
		ug_nfc_set_nfc_callback(ug_data);
	}
	else
	{
		LOGD("nfc_manager_initialize FAIL!!!!\n");
	}

	return layout_base;
}

/******************* UI Functions ENDZZZ*************/

static char *_get_file_path_from_uri(char *uri)
{
	char *path = NULL;

	if (uri != NULL)
	{
		if (strlen(uri) > 8 && strncmp(uri, "file:///", 8) == 0)
		{
			path = uri + 7;
		}
		else if(strlen(uri) > 6 && strncmp(uri, "file:/", 6) == 0)
		{
			path = uri + 5;
		}
		else if(uri[0] == '/')
		{
			path = uri;
		}
	}

	return path;
}

void ug_nfc_share_create_base_view(void *user_data)
{
	LOGD("[%s(): %d] BEGIN >>>>", __FUNCTION__, __LINE__);

	ugdata_t* ug_data = (ugdata_t*)user_data;

	/* create base layout */
	ug_data->base_layout = _create_main_layout(ug_data->ug_win_main);
	ug_data->bg = _create_bg(ug_data->base_layout);
	elm_object_part_content_set(ug_data->base_layout, "elm.swallow.bg", ug_data->bg);
	evas_object_show(ug_data->base_layout);

	LOGD("[%s(): %d] END >>>>", __FUNCTION__, __LINE__);

}

void ug_nfc_share_create_nfc_share_view(void *user_data)
{
	LOGD("[%s(): %d] BEGIN >>>>", __FUNCTION__, __LINE__);

	ugdata_t* ug_data = (ugdata_t*)user_data;
	Evas_Object* nfc_share_layout = NULL;
	Evas_Object* l_button = NULL;

	ug_data->base_naviframe = elm_naviframe_add(ug_data->base_layout);
	elm_object_part_content_set(ug_data->base_layout, "elm.swallow.content", ug_data->base_naviframe);

	evas_object_show(ug_data->base_layout);
	evas_object_show(ug_data->base_naviframe);

	/* create share layout */
	nfc_share_layout = ug_nfc_share_create_layout(ug_data);

	if(nfc_share_layout == NULL) {
		LOGD("nfc_share_layout is NULL\n");
		return;
	}

	/* push naviframe */
	l_button = elm_button_add(ug_data->base_naviframe);
	evas_object_smart_callback_add(l_button, "clicked", _back_clicked_cb, ug_data->nfc_share_ug);
	ug_data->base_navi_it = elm_naviframe_item_push(ug_data->base_naviframe, IDS_SHARE_VIA_NFC, l_button, NULL, nfc_share_layout, "1line");
	elm_object_style_set(l_button, "naviframe/back_btn/default");

	LOGD("[%s(): %d] END >>>>", __FUNCTION__, __LINE__);

}

bool ug_nfc_share_check_nfc_isAvailable(void *user_data)
{
	LOGD("[%s(): %d] BEGIN >>>>", __FUNCTION__, __LINE__);

	ugdata_t *ug_data = (ugdata_t *)user_data;
	ret_if(ug_data == NULL);

	int result = EINA_FALSE;
	int on;

	/* check if nfc is on */
	result = vconf_get_bool(NFC_SETTING_KEY_NAME, &on);

	if (!on) {
		/* show nfc on/off popup */
		_setting_on_YesNo_popup(ug_data);
		return FALSE;
	}

	/* check if handover is needed */
	if (ug_nfc_share_get_tag_type() == UG_NFC_SHARE_TAG_HANDOVER) {
		_show_handover_popup(ug_data);
		return FALSE;
	}

	LOGD("[%s(): %d] END >>>>", __FUNCTION__, __LINE__);

	return TRUE;
}

 /* nfc_handover */
static int ug_get_share_data_list( char *bundle_txt, char *datalist[], int number_of_files)
{
	char *token = NULL;
	char *param = NULL;
	int i = 0;

	param = (char *)bundle_txt;
	while (((token = strstr(param, NET_NFC_SHARE_SEPERATOR)) != NULL) && i < number_of_files)
	{
		*token = '\0';
		datalist[i] = param;
		LOGD("ug_get_share_data_list [%d] [%s]\n", i, datalist[i]);
		param = token + 1;
		i++;
	}
	if (i == (number_of_files - 1))
	{
		datalist[i] = param;
		LOGD("data [%d] [%s]\n", i, datalist[i]);
	}
	else
	{
		LOGD("Not match : [%d] / [%d]\n", number_of_files, i);
		return -1;
	}

	return 0;
}

void ug_nfc_share_create_data(struct ui_gadget *ug, bundle *bd, ugdata_t* ug_data)
{

	LOGD("[%s(): %d] BEGIN >>>>", __FUNCTION__, __LINE__);

	char *key[] = {"count","request_type","request_data"};

	char *count = (char *)bundle_get_val(bd,key[0]);
	char *request_type = (char *)bundle_get_val(bd,key[1]);
	char *request_data = (char *)bundle_get_val(bd,key[2]);
	char *extra = NULL;

	int number_of_data = 0;
	char *request_data_temp = NULL;

	int result = UG_NFC_SHARE_ERROR;
	nfc_ndef_message_h msg = NULL;


	number_of_data = atoi(count);

	LOGD("count : %s, request_type : %s, request_data : %s request_data strlen %d\n", GET_SAFE_STRING(count), GET_SAFE_STRING(request_type), GET_SAFE_STRING(request_data), strlen(request_data));

	if (request_data == NULL || strlen(request_data) == 0 || number_of_data < 1)
	{
		LOGD("request_data IS NULL\n");
		/* exit */
		return;
	}

	request_data_temp = malloc (strlen (request_data) + 1);
	memset(request_data_temp,0x00,strlen(request_data)+1);
	memcpy(request_data_temp,request_data,strlen(request_data));
	request_data_temp[strlen(request_data)] = '\0';

	/* process data and store specific space for sharing data */
	if (strncmp(request_type, NET_NFC_SHARE_REQUEST_TYPE_PATH, strlen(NET_NFC_SHARE_REQUEST_TYPE_PATH)) == 0)
	{
		struct stat st;
		char *path = NULL;
		char *pathlist[number_of_data];
		int sum_filesize = 0;
		int i = 0;


		LOGD("data is file\n");

		ug_get_share_data_list(request_data_temp, pathlist , number_of_data);

		while (i< number_of_data)
		{
			path = _get_file_path_from_uri(pathlist[i]);
			stat(path, &st);
			sum_filesize += st.st_size;
			LOGD("ug_nfc_share_create_data #[%d] filename(%s)  size =  (%d) sum_filesize = (%d) \n", i, path, st.st_size, sum_filesize);
			i++ ;
		}

		if (sum_filesize < UG_NFC_SHARE_HANDOVER_FILE_SIZE)
		{
			LOGD("file path for NFC P2P : request_data [%s]\n", request_data);
		    	 /* nfc_handover required to seperator depends on AC later*/
			result = ug_nfc_share_make_ndef_message_from_multi_file(&msg, (const char **)pathlist, number_of_data);
			if (result != UG_NFC_SHARE_OK)
			{
				LOGD("ug_nfc_share_make_ndef_message_from_file failed (%d)\n", result);
				return ;
			}

			ug_nfc_share_set_tag_type(UG_NFC_SHARE_TAG_FILE);
		}
		else
		{
			LOGD("file path for AC : [%s]\n", request_data);

			if (ug_data->uri != NULL)
			{
				UG_NFC_SHARE_MEM_FREE(ug_data->uri);
				ug_data->uri = NULL;
			}

			if (number_of_data > 1)
			{
				UG_NFC_SHARE_MEM_STRDUP(ug_data->uri, request_data);
			}
			else
			{
				UG_NFC_SHARE_MEM_STRDUP(ug_data->uri, path);
			}

			ug_nfc_share_set_tag_type(UG_NFC_SHARE_TAG_HANDOVER);
		}
	}
	else if (strncmp(request_type, NET_NFC_SHARE_REQUEST_TYPE_BUFFER, strlen(NET_NFC_SHARE_REQUEST_TYPE_BUFFER)) == 0)
	{
		/* nfc_handover now no user case of multi recored for buffered string*/
		nfc_ndef_record_h record = NULL;
		/* make memo NDEF message */
		LOGD("data is string\n");
		request_data[(strlen (request_data) -1)] = '\0';   /* nfc_handover todo, this may be memory issue of MEMO app */

		result = nfc_ndef_message_create(&msg);
		if(result != NFC_ERROR_NONE)
		{
			LOGD("nfc_ndef_message_create failed\n");

			return ;
		}
		if( (strncmp(request_data, "http://", strlen("http://")) == 0) ||(strncmp(request_data, "https://", strlen("https://")) == 0))
		{
			LOGD("data is URI\n");

			result = nfc_ndef_record_create_uri(&record, request_data);
			ug_nfc_share_set_tag_type(UG_NFC_SHARE_TAG_URL);
		}
		else
		{
			LOGD("data is MEMO\n");

			result = nfc_ndef_record_create_text(&record,request_data, "en-US" ,NFC_ENCODE_UTF_8);
			ug_nfc_share_set_tag_type(UG_NFC_SHARE_TAG_MEMO);
		}

		result = nfc_ndef_message_append_record(msg, record);

	}

	result = ug_nfc_share_set_current_ndef(ug_data, msg);
	if (result != UG_NFC_SHARE_OK)
	{
		LOGD("ug_nfc_share_set_current_ndef failed (%d)\n", result);
		return ;

	}

	/* set UG data */
	ug_data->ug_win_main = ug_get_parent_layout(ug);
	ug_data->nfc_share_ug = ug;

	LOGD("[%s(): %d] END >>>>", __FUNCTION__, __LINE__);
}



static void *__ug_nfc_share_create(struct ui_gadget *ug, enum ug_mode mode, bundle *bd, void *user_data)
{
	LOGD("[%s(): %d] BEGIN >>>>", __FUNCTION__, __LINE__);
	ugdata_t* ug_data = (ugdata_t*)user_data;

	/* set text domain */
	bindtextdomain(NFCUG_TEXT_DOMAIN, NFCUG_LOCALEDIR);

	/* parsing pameter to make nfc data */
	ug_nfc_share_create_data(ug, bd, ug_data);

	/* create base layout */
	ug_nfc_share_create_base_view(ug_data);

	/* check whether nfc is available */
	if (ug_nfc_share_check_nfc_isAvailable(ug_data)) {
		LOGD("Normal NFC", __FUNCTION__, __LINE__);

		/* create share view */
		ug_nfc_share_create_nfc_share_view(ug_data);
	}

	LOGD("[%s(): %d] END >>>>", __FUNCTION__, __LINE__);

	return ug_data->base_layout;
}

static void __ug_nfc_share_destroy(struct ui_gadget *ug, bundle *bd, void *user_data)
{
	LOGD("[%s(): %d] BEGIN >>>>", __FUNCTION__, __LINE__);

	ugdata_t *ug_data = (ugdata_t *)user_data;

	if (ug_data == NULL)
		return;

	evas_object_del(ug_get_layout(ug));

	LOGD("[%s(): %d] END >>>>", __FUNCTION__, __LINE__);
}

static void __ug_nfc_share_start(struct ui_gadget *ug, bundle *bd, void *user_data)
{
}

static void __ug_nfc_share_pause(struct ui_gadget *ug, bundle *bd, void *user_data)
{
}

static void __ug_nfc_share_resume(struct ui_gadget *ug, bundle *bd, void *user_data)
{
}

static void __ug_nfc_share_message(struct ui_gadget *ug, bundle *msg, bundle *bd, void *user_data)
{
}

static void __ug_nfc_share_event(struct ui_gadget *ug, enum ug_event event, bundle *bd, void *user_data)
{
}

UG_MODULE_API int UG_MODULE_INIT(struct ug_module_ops *ops)
{
	LOGD("[%s(): %d] UG_MODULE_INIT!!\n", __FUNCTION__, __LINE__);

	ugdata_t *ug_data; // User defined private data
	if (!ops)
		return -1;

	ug_data = (ugdata_t *)malloc(sizeof(ugdata_t));
	if (!ug_data)
		return -1;

	memset(ug_data, 0x00, sizeof(ugdata_t));

	ops->create = __ug_nfc_share_create;
	ops->start = __ug_nfc_share_start;
	ops->pause = __ug_nfc_share_pause;
	ops->resume = __ug_nfc_share_resume;
	ops->destroy = __ug_nfc_share_destroy;
	ops->message = __ug_nfc_share_message;
	ops->event = __ug_nfc_share_event;
	ops->priv = ug_data;
	ops->opt = UG_OPT_INDICATOR_ENABLE;

	return 0;
}

UG_MODULE_API void UG_MODULE_EXIT(struct ug_module_ops *ops)
{
	ugdata_t* ug_data;

	if (!ops)
		return;

	ug_data = ops->priv;

	if (ug_data)
		free(ug_data);

	ops->priv = NULL;
}

