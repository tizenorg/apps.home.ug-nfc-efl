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

/* external library header */
#include <assert.h>
#include <glib.h>

#ifndef UG_MODULE_API
#define UG_MODULE_API __attribute__ ((visibility("default")))
#endif

/* nfc_handover */
#define NET_NFC_SHARE_SEPERATOR "?"
#define NET_NFC_SHARE_REQUEST_TYPE_BUFFER "data_buffer"
#define NET_NFC_SHARE_REQUEST_TYPE_PATH "file_path"

#define DISPLAY_STRING_MAX_SIZE		90
#define ABBREVIATION_TEXT			"..."

void ug_nfc_share_create_base_view(void *user_data);
void ug_nfc_share_create_nfc_share_view(void *user_data);
bool ug_nfc_share_check_nfc_isAvailable(void *user_data);
static void ug_nfc_share_create_data(ugdata_t* ug_data);

static Ecore_Timer *mdmPopup_timer = NULL;

int _get_theme_type()
{
	/* TODO : will be added */
	return 0;
}

const char *_get_font_name(int font)
{
	const char *name = NULL;

	/* TODO : will be improved */
	if (_get_theme_type() == 0)
	{
		switch (font)
		{
		case UG_FONT_LIST :
			name = "SLP";
			break;

		case UG_FONT_SBEAM_TITLE :
			name = "SLP";
			break;

		case UG_FONT_LIVEBOX :
			name = "SLP";
			break;

		case UG_FONT_HELP :
			name = "SLP";
			break;

		default :
			LOGD("[%s(): %d] unknown font type [%d]", __FUNCTION__, __LINE__, font);
			break;
		}
	}

	return name;
}

uint32_t _get_font_color(int font)
{
	uint32_t color = 0x00808080;

	/* TODO : will be improved */
	if (_get_theme_type() == 0)
	{
		switch (font)
		{
		case UG_FONT_LIST :
			color = 0x00000000;
			break;

		case UG_FONT_SBEAM_TITLE :
			color = 0x00000000;
			break;

		case UG_FONT_LIVEBOX :
			color = 0x00FFFFFF;
			break;

		case UG_FONT_HELP :
			color = 0x00000000;
			break;

		default :
			LOGD("[%s(): %d] unknown font type [%d]", __FUNCTION__, __LINE__, font);
			break;
		}
	}

	return color;
}

int _get_font_size(int font)
{
	int size = 0;

	/* TODO : will be improved */
	if (_get_theme_type() == 0)
	{
		switch (font)
		{
		case UG_FONT_LIST :
			size = 28;
			break;

		case UG_FONT_SBEAM_TITLE :
			size = 38;
			break;

		case UG_FONT_LIVEBOX :
			size = 30;
			break;

		case UG_FONT_HELP :
			size = 32;
			break;

		default :
			LOGD("[%s(): %d] unknown font type [%d]", __FUNCTION__, __LINE__, font);
			break;
		}
	}

	return size;
}

const char *_get_font_style(int font)
{
	const char *style = NULL;

	/* TODO : will be improved */
	if (_get_theme_type() == 0)
	{
		switch (font)
		{
		case UG_FONT_LIST :
			style = "bold";
			break;

		case UG_FONT_SBEAM_TITLE :
			style = "";
			break;

		case UG_FONT_LIVEBOX :
			style = "";
			break;

		case UG_FONT_HELP :
			style = "bold";
			break;

		default :
			LOGD("[%s(): %d] unknown font type [%d]", __FUNCTION__, __LINE__, font);
			break;
		}
	}

	return style;
}

bool _get_label_text(int font, const char *text, int align, char *output, int len)
{
	bool result = false;
	char *align_begin = NULL;
	char *align_end = NULL;

	if (text == NULL)
		return result;

	switch (align)
	{
	case UG_ALIGN_CENTER :
		align_begin = "<align=center>";
		align_end = "</align>";
		break;

	case UG_ALIGN_RIGHT :
		align_begin = "<align=right>";
		align_end = "</align>";
		break;

	case UG_ALIGN_LEFT :
		default :
		align_begin = "";
		align_end = "";
		break;
	}

	/* TODO : will be improved */
	if (_get_font_style(font) != NULL)
	{
		snprintf(output, len, "%s<font_size=%d><color=#%06X><b>%s</b></color></font_size>%s", align_begin, _get_font_size(font), _get_font_color(font), text, align_end);
	}
	else
	{
		snprintf(output, len, "%s<font_size=%d><color=#%06X>%s</color></font_size>%s", align_begin, _get_font_size(font), _get_font_color(font), text, align_end);
	}

	return result;
}

void _get_image_path(const char *image, char *path, int len)
{
	const char *prefix = NULL;

	switch (_get_theme_type())
	{
	/* TODO : will be improved */
	case 0 :
		prefix = IMAGES_PATH"/white";
		break;

	default :
		prefix = IMAGES_PATH"/black";
		break;
	}

	if (image != NULL)
	{
		snprintf(path, len, "%s/%s", prefix, image);
	}
	else
	{
		snprintf(path, len, "%s", prefix);
	}
}

/******************* UI Functions *************/
static Evas_Object *_create_button(Evas_Object *parent, void (*func)(void *data, Evas_Object *obj, void *event_info), const char *label, const char *swallow, const char *style, void *data)
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

#if 0
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

static Evas_Object* _create_win_for_popup(Evas_Object * parent)
{
	Evas_Object *eo;
	int w, h;

	eo = elm_win_add(parent, "NFCShareSubWindow", ELM_WIN_BASIC);
	if (eo)
	{
		elm_win_title_set(eo, "NFCShareSubWindow");
		elm_win_borderless_set(eo, EINA_TRUE);
		elm_win_alpha_set(eo, EINA_TRUE);
		evas_object_smart_callback_add(eo, "delete,request", _win_del, NULL);
		ecore_x_window_size_get(ecore_x_window_root_first_get(), &w, &h);
		evas_object_resize(eo, w, h);
		elm_win_indicator_mode_set(eo, EINA_TRUE);
	}

	return eo;
}
#endif

static Evas_Object *_create_bg(Evas_Object *win)
{
	Evas_Object *bg = elm_bg_add(win);

	evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
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

static void _mdm_restricted_popup_response_cb(void *data, Evas_Object *obj, void *event_info)
{
	ugdata_t *ug_data = (ugdata_t *)data;
	ret_if(ug_data == NULL);

	LOGD("[%s(): %d] BEGIN >>>>", __FUNCTION__, __LINE__);
	LOGD("[%s(): %d] END >>>>", __FUNCTION__, __LINE__);

	ug_destroy_me(ug_data->nfc_share_ug);
}

static Eina_Bool _mdm_restricted_popup(void *data)
{
	LOGD("[%s(): %d] BEGIN >>>>", __FUNCTION__, __LINE__);

	char popup_str[POPUP_TEXT_SIZE] = { 0, };
	ugdata_t *ug_data = (ugdata_t *)data;
	retv_if(ug_data == NULL, ECORE_CALLBACK_CANCEL);

	snprintf(popup_str, POPUP_TEXT_SIZE, IDS_SECURITY_POLICY_RESTRICTS_USE_OF_PS, IDS_NFC_NFC);
	ug_nfc_share_create_popup(ug_data, ug_data->base_layout, popup_str, NULL, 0, NULL, 0, NULL, 0, true, true, _mdm_restricted_popup_response_cb);

	LOGD("[%s(): %d] END >>>>", __FUNCTION__, __LINE__);

	return ECORE_CALLBACK_CANCEL;
}

static void _activation_completed_cb(nfc_error_e error, void *user_data)
{
	ugdata_t *ug_data = (ugdata_t *)user_data;

	LOGD("[%s(): %d] BEGIN >>>>", __FUNCTION__, __LINE__);

	ret_if(ug_data == NULL);

	/* create share view */
	ug_nfc_share_create_nfc_share_view(ug_data);

	LOGD("[%s(): %d] END <<<<", __FUNCTION__, __LINE__);
}

static void _setting_on_YesNo_popup_response_cb(void *data, Evas_Object *obj, void *event_info)
{
	ugdata_t *ug_data = (ugdata_t *)data;
	int result = (int)event_info;

	LOGD("[%s(): %d] BEGIN>>>>", __FUNCTION__, __LINE__);

	ret_if(ug_data == NULL);

	switch (result)
	{
	case UG_NFC_POPUP_RESP_OK :
		/* setting is on */
		LOGD("setting is on >>>>", __FUNCTION__, __LINE__);

		result = nfc_manager_set_activation(TRUE, _activation_completed_cb, ug_data);
		if (result == NFC_ERROR_SECURITY_RESTRICTED)
		{
			LOGD("mdmPopup_timer START ");
			mdmPopup_timer = ecore_timer_add(0.25, _mdm_restricted_popup, ug_data);
		}
		break;

	case UG_NFC_POPUP_RESP_CANCEL :
		/* destroy UG */
		LOGD("ug_destroy_me >>>>", __FUNCTION__, __LINE__);
		ug_destroy_me(ug_data->nfc_share_ug);
		break;

	default :
		break;
	}

	LOGD("[%s(): %d] END>>>>", __FUNCTION__, __LINE__);
}

static void _setting_on_YesNo_popup(void *data)
{
	ugdata_t *ug_data = (ugdata_t *)data;
	char popup_str[POPUP_TEXT_SIZE] = { 0, };

	LOGD("[%s(): %d] BEGIN>>>>", __FUNCTION__, __LINE__);

	ret_if(ug_data == NULL);

	memcpy(popup_str, IDS_SERVICE_NOT_AVAILABLE_NFC_TURNED_OFF_TURN_ON_NFC_Q, strlen(IDS_SERVICE_NOT_AVAILABLE_NFC_TURNED_OFF_TURN_ON_NFC_Q));

	ug_nfc_share_create_popup(ug_data, ug_data->base_layout, popup_str, IDS_YES, UG_NFC_POPUP_RESP_OK, IDS_NO, UG_NFC_POPUP_RESP_CANCEL, NULL, 0, false, false, _setting_on_YesNo_popup_response_cb);

	LOGD("[%s(): %d] END>>>>", __FUNCTION__, __LINE__);
}

static void _app_error_popup_response_cb(void *data, Evas_Object *obj, void *event_info)
{
	ugdata_t *ug_data = (ugdata_t *)data;
	int result = (int)event_info;

	LOGD("[%s(): %d] BEGIN >>>>", __FUNCTION__, __LINE__);

	if (ug_data == NULL)
		return;

	switch (result)
	{
	case UG_NFC_POPUP_RESP_CLOSE :
		/* destroy UG */
		LOGD("ug_destroy_me >>>>", __FUNCTION__, __LINE__);
		ug_destroy_me(ug_data->nfc_share_ug);
		break;

	default :
		break;
	}

	LOGD("[%s(): %d] END <<<<", __FUNCTION__, __LINE__);
}

static void _show_app_error_popup(void *data)
{
	ugdata_t *ug_data = (ugdata_t *)data;
	char popup_str[POPUP_TEXT_SIZE] = { 0, };

	LOGD("[%s(): %d] BEGIN >>>>", __FUNCTION__, __LINE__);

	if (ug_data == NULL)
		return;

	memcpy(popup_str, IDS_NFC_SERVICE_IS_NOT_SUPPORTED, strlen(IDS_NFC_SERVICE_IS_NOT_SUPPORTED));

	ug_nfc_share_create_popup(ug_data, ug_data->base_layout, popup_str, IDS_CLOSE, UG_NFC_POPUP_RESP_CLOSE, NULL, 0, NULL, 0, false, false, _app_error_popup_response_cb);

	LOGD("[%s(): %d] END <<<<", __FUNCTION__, __LINE__);
}

static void _back_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	LOGD("[%s(): %d] BEGIN >>>>", __FUNCTION__, __LINE__);

	/* destroy UG */
	ug_destroy_me(data);

	LOGD("[%s(): %d] END>>>>", __FUNCTION__, __LINE__);
}

static Evas_Object *ug_nfc_share_create_layout(void *data)
{
	ugdata_t* ug_data = (ugdata_t*)data;

	Evas_Object *layout = NULL;
	Evas_Object *cancel_btn = NULL;
	nfc_ndef_message_h msg = NULL;
	ug_nfc_share_tag_type type = UG_NFC_SHARE_TAG_MAX;
	int width, height;

	LOGD("[%s(): %d] BEGIN >>>>", __FUNCTION__, __LINE__);

	ecore_x_window_size_get(ecore_x_window_root_first_get(), &width, &height);
	LOGD("width[%d] / height[%d]\n", width, height);

	/* create base layout */
	layout = elm_layout_add(ug_data->base_naviframe);
	retv_if(layout == NULL, NULL);
	elm_layout_file_set(layout, EDJ_FILE, "share_via_nfc");
	if (width == 480) // WVGA
	{
		elm_layout_file_set(layout, EDJ_FILE, "share_via_nfc_wvga");
	}
	else // HD
	{
		elm_layout_file_set(layout, EDJ_FILE, "share_via_nfc");
	}
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(layout);

	/* get data */
	msg = ug_nfc_share_get_current_ndef(ug_data);
	type = ug_nfc_share_get_tag_type();

	char path[1024] = { 0, };
	Evas_Object *label = NULL;

	_get_label_text(UG_FONT_HELP, IDS_GENTLY_TOUCH_PHONES_TOGETHER_TO_SHARE, UG_ALIGN_LEFT, path, sizeof(path));
	label = elm_label_add(layout);
	elm_object_part_content_set(layout, "label_name", label);
	elm_label_line_wrap_set(label, ELM_WRAP_WORD);
	elm_object_text_set(label, path);

	/* create image */
	Evas_Object *image = NULL;
	if (width == 480) // WVGA
	{
		_get_image_path("U05_popup_Share_help.png", path, sizeof(path));
	}
	else // HD
	{
		_get_image_path("U05_AfterSelect_Share_help.png", path, sizeof(path));
	}
	image = _create_image(layout, path, "touch_image", ug_data);
	retv_if(image == NULL, NULL);
	evas_object_show(image);

#if 0
	_get_label_text(UG_FONT_HELP, IDS_TO_CANCEL_SHARING_TAP_CANCEL, UG_ALIGN_LEFT, path, sizeof(path));
	label = elm_label_add(layout);
	elm_object_part_content_set(layout, "label_cancel_message", label);
	elm_label_line_wrap_set(label, ELM_WRAP_WORD);
	elm_object_text_set(label, path);
#endif

	/* create cancel button */
	cancel_btn = _create_button(layout, _back_clicked_cb, IDS_CANCEL, "cancel_button", "style1", ug_data->nfc_share_ug);
	retv_if(cancel_btn == NULL, NULL);
	evas_object_show(cancel_btn);

	if (NFC_ERROR_NONE == nfc_manager_initialize(NULL, NULL))
	{
		LOGD("nfc_manager_initialize success\n");
		ug_nfc_set_nfc_callback(ug_data);
	}
	else
	{
		LOGD("nfc_manager_initialize FAIL!!!!\n");
	}

	return layout;
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
		else if (strlen(uri) > 6 && strncmp(uri, "file:/", 6) == 0)
		{
			path = uri + 5;
		}
		else if (uri[0] == '/')
		{
			path = uri;
		}
	}

	return path;
}

void ug_nfc_share_create_base_view(void *user_data)
{
	ugdata_t* ug_data = (ugdata_t*)user_data;

	LOGD("[%s(): %d] BEGIN >>>>", __FUNCTION__, __LINE__);

	/* create base layout */
	ug_data->base_layout = _create_main_layout(ug_data->ug_win_main);
	ug_data->bg = _create_bg(ug_data->ug_win_main);
	elm_object_part_content_set(ug_data->base_layout, "elm.swallow.bg", ug_data->bg);
	evas_object_show(ug_data->base_layout);

	LOGD("[%s(): %d] END >>>>", __FUNCTION__, __LINE__);
}

void ug_nfc_share_create_nfc_share_view(void *user_data)
{
	ugdata_t* ug_data = (ugdata_t*)user_data;
	Evas_Object* nfc_share_layout = NULL;

	LOGD("[%s(): %d] BEGIN >>>>", __FUNCTION__, __LINE__);

	ug_data->base_naviframe = elm_naviframe_add(ug_data->base_layout);
	elm_object_part_content_set(ug_data->base_layout, "elm.swallow.content", ug_data->base_naviframe);
	evas_object_show(ug_data->base_naviframe);

	/* create data */
	ug_nfc_share_create_data(ug_data);

	/* create share layout */
	nfc_share_layout = ug_nfc_share_create_layout(ug_data);

	if (nfc_share_layout == NULL)
	{
		LOGD("nfc_share_layout is NULL\n");
		return;
	}

	ug_data->base_navi_it = elm_naviframe_item_push(ug_data->base_naviframe, IDS_SHARE_VIA_NFC, NULL, NULL, nfc_share_layout, NULL);
	elm_object_item_signal_emit(ug_data->base_navi_it, "elm,state,controlbar,close", "");

	LOGD("[%s(): %d] END >>>>", __FUNCTION__, __LINE__);
}

bool ug_nfc_share_check_nfc_isAvailable(void *user_data)
{
	ugdata_t *ug_data = (ugdata_t *)user_data;
	int result = EINA_FALSE;
	int on;

	LOGD("[%s(): %d] BEGIN >>>>", __FUNCTION__, __LINE__);

	retv_if(ug_data == NULL, FALSE);

	/* check if nfc is on */
	result = vconf_get_bool(VCONFKEY_NFC_STATE, &on);
	if (!on)
	{
		/* show nfc on/off popup */
		_setting_on_YesNo_popup(ug_data);
		return FALSE;
	}

	LOGD("[%s(): %d] END >>>>", __FUNCTION__, __LINE__);

	return TRUE;
}

/* nfc_handover */
static int ug_get_share_data_list(char *bundle_txt, char *datalist[], int number_of_files)
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

static void ug_nfc_share_create_data(ugdata_t* ug_data)
{
	bundle *bd = ug_data->bd;
	int type;

	LOGD("[%s(): %d] BEGIN >>>>", __FUNCTION__, __LINE__);

	char *key[] = { "count", "request_type", "request_data" };

	char *count = (char *)bundle_get_val(bd, key[0]);
	char *request_data = (char *)bundle_get_val(bd, key[2]);
	int number_of_data = 0;

	int result = UG_NFC_SHARE_ERROR;
	nfc_ndef_message_h msg = NULL;

	type = ug_nfc_share_get_tag_type();
	number_of_data = atoi(count);

	LOGD("count : %s, request_data : %s request_data strlen %d\n", GET_SAFE_STRING(count), GET_SAFE_STRING(request_data), strlen(request_data));

	if (request_data == NULL || strlen(request_data) == 0 || number_of_data < 1)
	{
		LOGD("request_data IS NULL\n");
		/* exit */
		return;
	}

	switch (type)
	{
	case UG_NFC_SHARE_TAG_FILE :
		{
			char *pathlist[number_of_data];
			char *request_data_temp = NULL;

			LOGD("file path for NFC P2P : request_data [%s]\n", request_data);

			UG_NFC_SHARE_MEM_STRDUP(request_data_temp, request_data);

			ug_get_share_data_list(request_data_temp, pathlist, number_of_data);

			/* nfc_handover required to seperator depends on AC later*/
			result = ug_nfc_share_make_ndef_message_from_multi_file(&msg, (const char **)pathlist, number_of_data);
			if (result != UG_NFC_SHARE_OK)
			{
				LOGD("ug_nfc_share_make_ndef_message_from_file failed (%d)\n", result);
			}

			UG_NFC_SHARE_MEM_FREE(request_data_temp);
		}
		break;

	case UG_NFC_SHARE_TAG_HANDOVER :
		{
			nfc_ndef_record_h record = NULL;

			LOGD("file path for AC : [%s]\n", request_data);

			/* create record */
			result = nfc_ndef_record_create_mime(&record, "file", (uint8_t *)request_data, strlen(request_data));
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
				nfc_ndef_record_destroy(record);
				return;
			}

			/* append record to ndef msg */
			result = nfc_ndef_message_append_record(msg, record);
			if (result != NFC_ERROR_NONE)
			{
				LOGD("nfc_ndef_message_create failed (%d)\n", result);
				nfc_ndef_message_destroy(msg);
				nfc_ndef_record_destroy(record);
				return;
			}
		}
		break;

	case UG_NFC_SHARE_TAG_URL :
		{
			/* nfc_handover now no user case of multi recored for buffered string*/
			nfc_ndef_record_h record = NULL;

			/* make memo NDEF message */
			LOGD("data is string\n");

			result = nfc_ndef_record_create_uri(&record, request_data);
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
				nfc_ndef_record_destroy(record);
				return;
			}

			/* append record to ndef msg */
			result = nfc_ndef_message_append_record(msg, record);
			if (result != NFC_ERROR_NONE)
			{
				LOGD("nfc_ndef_message_create failed (%d)\n", result);
				nfc_ndef_message_destroy(msg);
				nfc_ndef_record_destroy(record);
				return;
			}
		}
		break;

	case UG_NFC_SHARE_TAG_MEMO :
		{
			/* nfc_handover now no user case of multi recored for buffered string*/
			nfc_ndef_record_h record = NULL;

			/* make memo NDEF message */
			LOGD("UG_NFC_SHARE_TAG_MEMO\n");

			result = nfc_ndef_record_create_text(&record, request_data, "en-US", NFC_ENCODE_UTF_8);
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
				nfc_ndef_record_destroy(record);
				return;
			}

			/* append record to ndef msg */
			result = nfc_ndef_message_append_record(msg, record);
			if (result != NFC_ERROR_NONE)
			{
				LOGD("nfc_ndef_message_create failed (%d)\n", result);
				nfc_ndef_message_destroy(msg);
				nfc_ndef_record_destroy(record);
				return;
			}
		}
		break;

	default :
		break;
	}

	result = ug_nfc_share_set_current_ndef(ug_data, msg);
	if (result != UG_NFC_SHARE_OK)
	{
		LOGD("ug_nfc_share_set_current_ndef failed (%d)\n", result);
	}

	LOGD("[%s(): %d] END >>>>", __FUNCTION__, __LINE__);
}

static void ug_nfc_share_set_tag_type_from_bundle(bundle *bd)
{
	int count = 0;

	char *temp_count = (char *)bundle_get_val(bd, "count");
	char *request_type = (char *)bundle_get_val(bd, "request_type");
	char *request_data = (char *)bundle_get_val(bd, "request_data");

	LOGD("[%s(): %d] BEGIN >>>>", __FUNCTION__, __LINE__);

	count = atoi(temp_count);

	LOGD("count : %d, request_type : %s, request_data : %s request_data strlen %d\n", count,
			GET_SAFE_STRING(request_type), GET_SAFE_STRING(request_data), strlen(request_data));

	if (request_data == NULL || strlen(request_data) == 0 || count < 1)
	{
		LOGD("request_data IS NULL\n");
		/* exit */
		return;
	}

	/* process data and store specific space for sharing data */
	if (strncmp(request_type, NET_NFC_SHARE_REQUEST_TYPE_PATH, strlen(NET_NFC_SHARE_REQUEST_TYPE_PATH)) == 0)
	{
		struct stat st;
		size_t sum_filesize = 0;
		char *temp_data = NULL;
		char *token = NULL;

		LOGD("data is file(s)\n");

		UG_NFC_SHARE_MEM_STRDUP(temp_data, request_data);
		if (temp_data != NULL)
		{
			/* get files size */
			token = strtok(temp_data, NET_NFC_SHARE_SEPERATOR);
			for (; token != NULL; token = strtok(NULL, NET_NFC_SHARE_SEPERATOR))
			{
				LOGD("token [%s]\n", token);
				if (stat(token, &st) == 0)
				{
					sum_filesize += st.st_size;
				}
				else
				{
					LOGE("file not found : %s", token);
				}
			}

			/* set share type */
			if (sum_filesize < UG_NFC_SHARE_HANDOVER_FILE_SIZE)
			{
				LOGD("file path for NFC P2P : request_data [%s]\n", request_data);

				ug_nfc_share_set_tag_type(UG_NFC_SHARE_TAG_FILE);
			}
			else
			{
				LOGD("file path for Handover : [%s]\n", request_data);

				ug_nfc_share_set_tag_type(UG_NFC_SHARE_TAG_HANDOVER);
			}

			UG_NFC_SHARE_MEM_FREE(temp_data);
		}
		else
		{
			LOGE("alloc failed");
		}
	}
	else if (strncmp(request_type, NET_NFC_SHARE_REQUEST_TYPE_BUFFER, strlen(NET_NFC_SHARE_REQUEST_TYPE_BUFFER)) == 0)
	{
		if ((strncmp(request_data, "http://", strlen("http://")) == 0) || (strncmp(request_data, "https://", strlen("https://")) == 0))
		{
			LOGD("data is URI\n");

			ug_nfc_share_set_tag_type(UG_NFC_SHARE_TAG_URL);
		}
		else
		{
			LOGD("data is MEMO\n");

			ug_nfc_share_set_tag_type(UG_NFC_SHARE_TAG_MEMO);
		}
	}

	LOGD("[%s(): %d] END >>>>", __FUNCTION__, __LINE__);
}

static void *__ug_nfc_share_create(ui_gadget_h ug, enum ug_mode mode, service_h service, void *priv)
{
	ugdata_t *ug_data = (ugdata_t *)priv;

	LOGD("[%s(): %d] BEGIN >>>>", __FUNCTION__, __LINE__);

	/* set text domain */
	bindtextdomain(NFCUG_TEXT_DOMAIN, NFCUG_LOCALEDIR);

	/* set UG data */
	ug_data->ug_win_main = ug_get_parent_layout(ug);
	ug_data->nfc_share_ug = ug;
#if 0
	if (SERVICE_ERROR_NONE != service_export_as_bundle(service, &ug_data->bd))
	return NULL;
#else
	bundle *bd = NULL;
	char *temp_count = NULL;
	char *request_type = NULL;
	char *request_data = NULL;
	char *mime_type = NULL;

	bd = bundle_create();
	if (bd == NULL)
		return NULL;

	service_get_extra_data(service, "count", &temp_count);
	service_get_extra_data(service, "request_type", &request_type);
	service_get_extra_data(service, "request_data", &request_data);
	service_get_extra_data(service, "mime_type", &mime_type);

	bundle_add(bd, "count", temp_count);
	bundle_add(bd, "request_type", request_type);
	bundle_add(bd, "request_data", request_data);
	bundle_add(bd, "mime_type", mime_type);

	ug_data->bd = bd;
#endif

	/* parse pameter, and them set request type */
	ug_nfc_share_set_tag_type_from_bundle(ug_data->bd);

	/* create base layout */
	ug_nfc_share_create_base_view(ug_data);

	/* create share view */
	ug_nfc_share_create_nfc_share_view(ug_data);

	LOGD("[%s(): %d] END >>>>", __FUNCTION__, __LINE__);

	return ug_data->base_layout;
}

static void __ug_nfc_share_destroy(ui_gadget_h ug, service_h service, void *priv)
{
	ugdata_t *ug_data = (ugdata_t *)priv;

	LOGD("[%s(): %d] BEGIN >>>>", __FUNCTION__, __LINE__);

	if (ug_data == NULL)
		return;

	/* unset callback */
	ug_nfc_unset_nfc_callback();

	/* nfc deactivate */
	if (nfc_manager_deinitialize() != NFC_ERROR_NONE)
	{
		LOGD("nfc_manager_deinitialize failed", __FUNCTION__, __LINE__);
	}

	if (ug_data->bd != NULL)
	{
		bundle_free(ug_data->bd);
		ug_data->bd = NULL;
	}

	evas_object_del(ug_get_layout(ug));

	LOGD("[%s(): %d] END >>>>", __FUNCTION__, __LINE__);
}

static void __ug_nfc_share_start(ui_gadget_h ug, service_h service, void *priv)
{
	LOGD("[%s(): %d] BEGIN >>>>", __FUNCTION__, __LINE__);

	ugdata_t *ug_data = (ugdata_t *)priv;

	/* check nfc-device*/
	if (!nfc_manager_is_supported())
	{
		LOGD("It is not nfc device >>>>");
		_show_app_error_popup(ug_data);
		return;
	}

	ug_nfc_share_check_nfc_isAvailable(ug_data);

	LOGD("[%s(): %d] END >>>>", __FUNCTION__, __LINE__);
}

static void __ug_nfc_share_pause(ui_gadget_h ug, service_h service, void *priv)
{
	LOGD("[%s(): %d] BEGIN >>>>", __FUNCTION__, __LINE__);

	LOGD("[%s(): %d] END >>>>", __FUNCTION__, __LINE__);
}

static void __ug_nfc_share_resume(ui_gadget_h ug, service_h service, void *priv)
{
	LOGD("[%s(): %d] BEGIN >>>>", __FUNCTION__, __LINE__);

	LOGD("[%s(): %d] END >>>>", __FUNCTION__, __LINE__);
}

static void __ug_nfc_share_message(ui_gadget_h ug, service_h msg, service_h service, void *priv)
{
	LOGD("[%s(): %d] BEGIN >>>>", __FUNCTION__, __LINE__);

	LOGD("[%s(): %d] END >>>>", __FUNCTION__, __LINE__);
}

static void __ug_nfc_share_event(ui_gadget_h ug, enum ug_event event, service_h service, void *priv)
{
	LOGD("[%s(): %d] BEGIN >>>>", __FUNCTION__, __LINE__);

	ugdata_t *ug_data = (ugdata_t *)priv;

	LOGD("event[%d]", event);

	switch (event) {
	case UG_EVENT_LOW_MEMORY:
		LOGD("UG_EVENT_LOW_MEMORY");
		break;
	case UG_EVENT_LOW_BATTERY:
		LOGD("UG_EVENT_LOW_BATTERY");
		break;
	case UG_EVENT_LANG_CHANGE:
		LOGD("UG_EVENT_LANG_CHANGE");
		break;
	case UG_EVENT_ROTATE_PORTRAIT:
		elm_win_rotation_with_resize_set(ug_data->ug_win_main, 0);
		LOGD("UG_EVENT_ROTATE_PORTRAIT");
		break;
	case UG_EVENT_ROTATE_PORTRAIT_UPSIDEDOWN:
		elm_win_rotation_with_resize_set(ug_data->ug_win_main, 0);
		LOGD("UG_EVENT_ROTATE_PORTRAIT_UPSIDEDOWN");
		break;
	case UG_EVENT_ROTATE_LANDSCAPE:
		LOGD("UG_EVENT_ROTATE_LANDSCAPE");
		elm_win_rotation_with_resize_set(ug_data->ug_win_main, 0);
		break;
	case UG_EVENT_ROTATE_LANDSCAPE_UPSIDEDOWN:
		LOGD("UG_EVENT_ROTATE_LANDSCAPE_UPSIDEDOWN");
		elm_win_rotation_with_resize_set(ug_data->ug_win_main, 0);
		break;
	case UG_EVENT_REGION_CHANGE:
		LOGD("UG_EVENT_REGION_CHANGE");
		break;
	default:
		break;
	}

	LOGD("[%s(): %d] END >>>>", __FUNCTION__, __LINE__);
}

UG_MODULE_API int UG_MODULE_INIT(struct ug_module_ops *ops)
{
	ugdata_t *ug_data; // User defined private data

	LOGD("[%s(): %d] UG_MODULE_INIT!!\n", __FUNCTION__, __LINE__);

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
	ugdata_t *ug_data;

	if (!ops)
		return;

	ug_data = ops->priv;

	if (ug_data)
		free(ug_data);

	ops->priv = NULL;
}
