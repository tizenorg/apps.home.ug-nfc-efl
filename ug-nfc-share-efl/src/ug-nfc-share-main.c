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
#define NET_NFC_SHARE_REQUEST_TYPE_BUFFER "data_buffer"

#define DISPLAY_STRING_MAX_SIZE		90
#define ABBREVIATION_TEXT			"..."

void ug_nfc_share_create_base_view(void *user_data);
void ug_nfc_share_create_nfc_share_view(void *user_data);
bool ug_nfc_share_check_nfc_isAvailable(void *user_data);
static void ug_nfc_share_create_data(ugdata_t* ug_data);


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
			name = "Tizen";
			break;

		case UG_FONT_SBEAM_TITLE :
			name = "Tizen";
			break;

		case UG_FONT_LIVEBOX :
			name = "Tizen";
			break;

		case UG_FONT_HELP :
			name = "Tizen";
			break;

		default :
			UG_NFC_SHARE_DEBUG_ERR("unknown font type [%d]", font);
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
			UG_NFC_SHARE_DEBUG_ERR("unknown font type [%d]", font);
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
			UG_NFC_SHARE_DEBUG_ERR("unknown font type [%d]", font);
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
			UG_NFC_SHARE_DEBUG_ERR("unknown font type [%d]", font);
			break;
		}
	}

	return style;
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

	evas_object_show(layout);

	return layout;
}

static void _activation_completed_cb(nfc_error_e error, void *user_data)
{
	ugdata_t *ug_data = (ugdata_t *)user_data;

	UG_NFC_SHARE_BEGIN();

	ret_if(ug_data == NULL);

	/* create share view */
	ug_nfc_share_create_nfc_share_view(ug_data);

	UG_NFC_SHARE_END();
}

static void _setting_on_YesNo_popup_response_cb(void *data, Evas_Object *obj, void *event_info)
{
	ugdata_t *ug_data = (ugdata_t *)data;
	int result = (int)event_info;

	UG_NFC_SHARE_BEGIN();

	ret_if(ug_data == NULL);

	switch (result)
	{
	case UG_NFC_POPUP_RESP_OK :
		/* setting is on */
		UG_NFC_SHARE_DEBUG("setting is on >>>>");

		result = nfc_manager_set_activation(true, _activation_completed_cb, ug_data);
		if (result != NFC_ERROR_NONE)
		{
			UG_NFC_SHARE_DEBUG_ERR("nfc_manager_set_activation failed");
		}
		break;

	case UG_NFC_POPUP_RESP_CANCEL :
		/* destroy UG */
		UG_NFC_SHARE_DEBUG("ug_destroy_me >>>>");
		ug_destroy_me(ug_data->nfc_share_ug);
		break;

	default :
		break;
	}

	UG_NFC_SHARE_END();
}

static void _setting_on_YesNo_popup(void *data)
{
	ugdata_t *ug_data = (ugdata_t *)data;
	char popup_str[POPUP_TEXT_SIZE] = { 0, };

	UG_NFC_SHARE_BEGIN();

	ret_if(ug_data == NULL);

	memcpy(popup_str, IDS_SERVICE_NOT_AVAILABLE_NFC_TURNED_OFF_TURN_ON_NFC_Q, strlen(IDS_SERVICE_NOT_AVAILABLE_NFC_TURNED_OFF_TURN_ON_NFC_Q));

	ug_nfc_share_create_popup(ug_data, ug_data->base_layout, popup_str, IDS_NO, UG_NFC_POPUP_RESP_CANCEL, IDS_YES, UG_NFC_POPUP_RESP_OK, NULL, 0, false, false, _setting_on_YesNo_popup_response_cb);

	UG_NFC_SHARE_END();
}

static void _app_error_popup_response_cb(void *data, Evas_Object *obj, void *event_info)
{
	ugdata_t *ug_data = (ugdata_t *)data;
	int result = (int)event_info;

	UG_NFC_SHARE_BEGIN();

	if (ug_data == NULL)
		return;

	switch (result)
	{
	case UG_NFC_POPUP_RESP_CLOSE :
		/* destroy UG */
		UG_NFC_SHARE_DEBUG("ug_destroy_me >>>>");
		ug_destroy_me(ug_data->nfc_share_ug);
		break;

	default :
		break;
	}

	UG_NFC_SHARE_END();
}

static void _show_app_error_popup(void *data)
{
	ugdata_t *ug_data = (ugdata_t *)data;
	char popup_str[POPUP_TEXT_SIZE] = { 0, };

	UG_NFC_SHARE_BEGIN();

	if (ug_data == NULL)
		return;

	memcpy(popup_str, IDS_NFC_SERVICE_IS_NOT_SUPPORTED, strlen(IDS_NFC_SERVICE_IS_NOT_SUPPORTED));

	ug_nfc_share_create_popup(ug_data, ug_data->base_layout, popup_str, IDS_CLOSE, UG_NFC_POPUP_RESP_CLOSE, NULL, 0, NULL, 0, false, false, _app_error_popup_response_cb);

	UG_NFC_SHARE_END();
}

static void _back_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	UG_NFC_SHARE_BEGIN();

	/* destroy UG */
	ug_destroy_me(data);

	UG_NFC_SHARE_END();
}

static void _lang_changed(void *data)
{
	UG_NFC_SHARE_BEGIN();

	ugdata_t* ug_data = (ugdata_t*)data;
	ret_if(ug_data == NULL);

	edje_object_part_text_set(elm_layout_edje_get(ug_data->layout), "title", IDS_GENTLY_TOUCH_PHONES_TOGETHER_TO_SHARE);

	elm_object_text_set(ug_data->cancel_btn, IDS_CANCEL);

	UG_NFC_SHARE_END();
}

static Evas_Object *ug_nfc_share_create_layout(void *data)
{
	ugdata_t* ug_data = (ugdata_t*)data;

	Evas_Object *layout = NULL;
	Evas_Object *cancel_btn = NULL;
	Evas_Object *access_layout = NULL;
	Evas_Object *access_object = NULL;
	int width, height;

	UG_NFC_SHARE_BEGIN();

	ecore_x_window_size_get(ecore_x_window_root_first_get(), &width, &height);
	UG_NFC_SHARE_DEBUG("width[%d] / height[%d]", width, height);

	/* create base layout */
	layout = elm_layout_add(ug_data->base_layout);
	retv_if(layout == NULL, NULL);
	ug_data->layout = layout;

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

	/* create message label */
	edje_object_part_text_set(elm_layout_edje_get(layout), "title", IDS_GENTLY_TOUCH_PHONES_TOGETHER_TO_SHARE);

	/* create image */
	Evas_Object *image = NULL;
	char path[1024] = { 0, };
	if (width == 480) // WVGA
	{
		_get_image_path("U05_AfterSelect_Share_help_popup.png", path, sizeof(path));
	}
	else // HD
	{
		_get_image_path("U05_AfterSelect_Share_help.png", path, sizeof(path));
	}
	image = _create_image(layout, path, "image", ug_data);
	retv_if(image == NULL, NULL);
	evas_object_show(image);

	/* create cancel button */
	cancel_btn = _create_button(layout, _back_clicked_cb, IDS_CANCEL, "cancel_btn", "style1", ug_data->nfc_share_ug);
	retv_if(cancel_btn == NULL, NULL);
	ug_data->cancel_btn = cancel_btn;
	evas_object_show(cancel_btn);

	/* accessibility */
	access_layout = (Evas_Object *)edje_object_part_object_get(elm_layout_edje_get(layout), "access");
	retv_if(access_layout == NULL, NULL);
	access_object = elm_access_object_register(access_layout, elm_layout_edje_get(layout));
	retv_if(access_object == NULL, NULL);
	elm_access_info_set(access_object, ELM_ACCESS_INFO, IDS_GENTLY_TOUCH_PHONES_TOGETHER_TO_SHARE);

	if (NFC_ERROR_NONE == nfc_manager_initialize(NULL, NULL))
	{
		UG_NFC_SHARE_DEBUG("nfc_manager_initialize success");
		ug_nfc_set_nfc_callback(ug_data);
	}
	else
	{
		UG_NFC_SHARE_DEBUG_ERR("nfc_manager_initialize FAIL!!!!");
	}

	return layout;
}

/******************* UI Functions ENDZZZ*************/

void ug_nfc_share_create_base_view(void *user_data)
{
	ugdata_t* ug_data = (ugdata_t*)user_data;

	UG_NFC_SHARE_BEGIN();

	/* create base layout */
	ug_data->base_layout = _create_main_layout(ug_data->ug_win_main);
	ug_data->bg = _create_bg(ug_data->ug_win_main);
	elm_object_part_content_set(ug_data->base_layout, "elm.swallow.bg", ug_data->bg);
	evas_object_show(ug_data->base_layout);

	UG_NFC_SHARE_END();
}

void ug_nfc_share_create_nfc_share_view(void *user_data)
{
	ugdata_t* ug_data = (ugdata_t*)user_data;
	Evas_Object* nfc_share_layout = NULL;

	UG_NFC_SHARE_BEGIN();


	/* create data */
	ug_nfc_share_create_data(ug_data);

	/* create share layout */
	nfc_share_layout = ug_nfc_share_create_layout(ug_data);

	if (nfc_share_layout == NULL)
	{
		UG_NFC_SHARE_DEBUG_ERR("nfc_share_layout is NULL");
		return;
	}

	elm_object_part_content_set(ug_data->base_layout, "elm.swallow.content", nfc_share_layout);

	UG_NFC_SHARE_END();
}

bool ug_nfc_share_check_nfc_isAvailable(void *user_data)
{
	ugdata_t *ug_data = (ugdata_t *)user_data;
	int result, on;

	UG_NFC_SHARE_BEGIN();

	retv_if(ug_data == NULL, false);

	/* check if nfc is on */
	if ((result = vconf_get_bool(VCONFKEY_NFC_STATE, &on)) == 0)
	{
		UG_NFC_SHARE_DEBUG("vconf_get_bool status [%d]", on);
		if (!on)
		{
			/* show nfc on/off popup */
			_setting_on_YesNo_popup(ug_data);
			return false;
		}
	}
	else
	{
		UG_NFC_SHARE_DEBUG_ERR("vconf_get_bool failed");
		return false;
	}

	UG_NFC_SHARE_END();

	return true;
}

static void ug_nfc_share_create_data(ugdata_t* ug_data)
{
	bundle *bd = ug_data->bd;
	int type;

	UG_NFC_SHARE_BEGIN();

	int result = UG_NFC_SHARE_ERROR;
	nfc_ndef_message_h msg = NULL;
	char *request_data = (char *)bundle_get_val(bd, "request_data");


	if (request_data == NULL)
	{
		UG_NFC_SHARE_DEBUG_ERR("request_data IS NULL");
		/* exit */
		return;
	}

	UG_NFC_SHARE_DEBUG("request_data : %s request_data strlen %d", GET_SAFE_STRING(request_data), strlen(request_data));

	if (strlen(request_data) == 0)
	{
		UG_NFC_SHARE_DEBUG_ERR("bundle data is wrong ");
		/* exit */
		return;
	}

	type = ug_nfc_share_get_tag_type();

	switch (type)
	{
	case UG_NFC_SHARE_TAG_URL :
		{
			/* nfc_handover now no user case of multi recored for buffered string*/
			nfc_ndef_record_h record = NULL;

			/* make memo NDEF message */
			UG_NFC_SHARE_DEBUG("data is string");

			result = nfc_ndef_record_create_uri(&record, request_data);
			if (result != NFC_ERROR_NONE)
			{
				UG_NFC_SHARE_DEBUG_ERR("nfc_ndef_record_create failed (%d)", result);
				return;
			}

			/* create ndef msg*/
			result = nfc_ndef_message_create(&msg);
			if (result != NFC_ERROR_NONE)
			{
				UG_NFC_SHARE_DEBUG_ERR("nfc_ndef_message_create failed (%d)", result);
				nfc_ndef_record_destroy(record);
				return;
			}

			/* append record to ndef msg */
			result = nfc_ndef_message_append_record(msg, record);
			if (result != NFC_ERROR_NONE)
			{
				UG_NFC_SHARE_DEBUG_ERR("nfc_ndef_message_create failed (%d)", result);
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
			UG_NFC_SHARE_DEBUG("UG_NFC_SHARE_TAG_MEMO");

			result = nfc_ndef_record_create_text(&record, request_data, "en-US", NFC_ENCODE_UTF_8);
			if (result != NFC_ERROR_NONE)
			{
				UG_NFC_SHARE_DEBUG_ERR("nfc_ndef_record_create failed (%d)", result);
				return;
			}

			/* create ndef msg*/
			result = nfc_ndef_message_create(&msg);
			if (result != NFC_ERROR_NONE)
			{
				UG_NFC_SHARE_DEBUG_ERR("nfc_ndef_message_create failed (%d)", result);
				nfc_ndef_record_destroy(record);
				return;
			}

			/* append record to ndef msg */
			result = nfc_ndef_message_append_record(msg, record);
			if (result != NFC_ERROR_NONE)
			{
				UG_NFC_SHARE_DEBUG_ERR("nfc_ndef_message_create failed (%d)", result);
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
		UG_NFC_SHARE_DEBUG_ERR("ug_nfc_share_set_current_ndef failed (%d)", result);
		nfc_ndef_message_destroy(msg);
	}

	UG_NFC_SHARE_END();
}

static bool ug_nfc_share_parse_bundle(bundle *bd)
{
	char *request_type = (char *)bundle_get_val(bd, "request_type");
	char *request_data = (char *)bundle_get_val(bd, "request_data");

	UG_NFC_SHARE_BEGIN();

	if (request_type == NULL || request_data == NULL)
	{
		UG_NFC_SHARE_DEBUG_ERR("request_data or request_data IS NULL");
		/* exit */
		return false;
	}

	UG_NFC_SHARE_DEBUG("request_type : %s, request_data : %s request_data strlen %d", GET_SAFE_STRING(request_type), GET_SAFE_STRING(request_data), strlen(request_data));

	if (strlen(request_data) == 0)
	{
		UG_NFC_SHARE_DEBUG_ERR("bundle data is wrong ");
		/* exit */
		return false;
	}

	/* process data and store specific space for sharing data */
	if (strncmp(request_type, NET_NFC_SHARE_REQUEST_TYPE_BUFFER, strlen(NET_NFC_SHARE_REQUEST_TYPE_BUFFER)) == 0)
	{
		if ((strncmp(request_data, "http://", strlen("http://")) == 0) || (strncmp(request_data, "https://", strlen("https://")) == 0))
		{
			UG_NFC_SHARE_DEBUG("data is URI");

			ug_nfc_share_set_tag_type(UG_NFC_SHARE_TAG_URL);
		}
		else
		{
			UG_NFC_SHARE_DEBUG("data is MEMO");

			ug_nfc_share_set_tag_type(UG_NFC_SHARE_TAG_MEMO);
		}
	}
	else
	{
		UG_NFC_SHARE_DEBUG_ERR("request_type is wrong ");

		return false;
	}

	UG_NFC_SHARE_END();

	return true;
}

static void *__ug_nfc_share_create(ui_gadget_h ug, enum ug_mode mode, service_h service, void *priv)
{
	ugdata_t *ug_data = (ugdata_t *)priv;

	UG_NFC_SHARE_BEGIN();

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
	char *request_type = NULL;
	char *request_data = NULL;

	bd = bundle_create();
	if (bd == NULL)
		return NULL;

	service_get_extra_data(service, "request_type", &request_type);
	service_get_extra_data(service, "request_data", &request_data);

	bundle_add(bd, "request_type", request_type);
	bundle_add(bd, "request_data", request_data);

	ug_data->bd = bd;
#endif

	/* parse pameter, and them set request type */
	if (!ug_nfc_share_parse_bundle(ug_data->bd))
		return NULL;

	/* create base layout */
	ug_nfc_share_create_base_view(ug_data);

	/* create share view */
	ug_nfc_share_create_nfc_share_view(ug_data);

	UG_NFC_SHARE_END();

	return ug_data->base_layout;
}

static void __ug_nfc_share_destroy(ui_gadget_h ug, service_h service, void *priv)
{
	ugdata_t *ug_data = (ugdata_t *)priv;

	UG_NFC_SHARE_BEGIN();

	if (ug_data == NULL)
		return;

	if (ug_data->current_ndef)
	{
		if (nfc_ndef_message_destroy(ug_data->current_ndef) != NFC_ERROR_NONE)
			UG_NFC_SHARE_DEBUG_ERR("nfc_ndef_message_destroy failed");
		ug_data->current_ndef = NULL;
	}

	/* unset callback */
	ug_nfc_unset_nfc_callback();

	/* nfc deactivate */
	if (nfc_manager_deinitialize() != NFC_ERROR_NONE)
	{
		UG_NFC_SHARE_DEBUG_ERR("nfc_manager_deinitialize failed");
	}

	if (ug_data->bd != NULL)
	{
		bundle_free(ug_data->bd);
		ug_data->bd = NULL;
	}

	evas_object_del(ug_get_layout(ug));

	UG_NFC_SHARE_END();
}

static void __ug_nfc_share_start(ui_gadget_h ug, service_h service, void *priv)
{
	UG_NFC_SHARE_BEGIN();

	ugdata_t *ug_data = (ugdata_t *)priv;

	/* check nfc-device*/
	if (!nfc_manager_is_supported())
	{
		UG_NFC_SHARE_DEBUG_ERR("It is not nfc device >>>>");
		_show_app_error_popup(ug_data);
		return;
	}

	ug_nfc_share_check_nfc_isAvailable(ug_data);

	UG_NFC_SHARE_END();
}

static void __ug_nfc_share_pause(ui_gadget_h ug, service_h service, void *priv)
{
	UG_NFC_SHARE_BEGIN();

	UG_NFC_SHARE_END();
}

static void __ug_nfc_share_resume(ui_gadget_h ug, service_h service, void *priv)
{
	UG_NFC_SHARE_BEGIN();

	UG_NFC_SHARE_END();
}

static void __ug_nfc_share_message(ui_gadget_h ug, service_h msg, service_h service, void *priv)
{
	UG_NFC_SHARE_BEGIN();

	UG_NFC_SHARE_END();
}

static void __ug_nfc_share_event(ui_gadget_h ug, enum ug_event event, service_h service, void *priv)
{
	UG_NFC_SHARE_BEGIN();

	ugdata_t *ug_data = (ugdata_t *)priv;

	UG_NFC_SHARE_DEBUG("event[%d]", event);

	switch (event) {
	case UG_EVENT_LOW_MEMORY:
		UG_NFC_SHARE_DEBUG("UG_EVENT_LOW_MEMORY");
		break;
	case UG_EVENT_LOW_BATTERY:
		UG_NFC_SHARE_DEBUG("UG_EVENT_LOW_BATTERY");
		break;
	case UG_EVENT_LANG_CHANGE:
		UG_NFC_SHARE_DEBUG("UG_EVENT_LANG_CHANGE");
		_lang_changed(ug_data);
		break;
	case UG_EVENT_ROTATE_PORTRAIT:
		elm_win_rotation_with_resize_set(ug_data->ug_win_main, 0);
		UG_NFC_SHARE_DEBUG("UG_EVENT_ROTATE_PORTRAIT");
		break;
	case UG_EVENT_ROTATE_PORTRAIT_UPSIDEDOWN:
		elm_win_rotation_with_resize_set(ug_data->ug_win_main, 0);
		UG_NFC_SHARE_DEBUG("UG_EVENT_ROTATE_PORTRAIT_UPSIDEDOWN");
		break;
	case UG_EVENT_ROTATE_LANDSCAPE:
		UG_NFC_SHARE_DEBUG("UG_EVENT_ROTATE_LANDSCAPE");
		elm_win_rotation_with_resize_set(ug_data->ug_win_main, 0);
		break;
	case UG_EVENT_ROTATE_LANDSCAPE_UPSIDEDOWN:
		UG_NFC_SHARE_DEBUG("UG_EVENT_ROTATE_LANDSCAPE_UPSIDEDOWN");
		elm_win_rotation_with_resize_set(ug_data->ug_win_main, 0);
		break;
	case UG_EVENT_REGION_CHANGE:
		UG_NFC_SHARE_DEBUG("UG_EVENT_REGION_CHANGE");
		break;
	default:
		break;
	}

	UG_NFC_SHARE_END();
}

UG_MODULE_API int UG_MODULE_INIT(struct ug_module_ops *ops)
{
	ugdata_t *ug_data; // User defined private data

	UG_NFC_SHARE_BEGIN();

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

	UG_NFC_SHARE_END();

	return 0;
}

UG_MODULE_API void UG_MODULE_EXIT(struct ug_module_ops *ops)
{
	ugdata_t *ug_data;

	UG_NFC_SHARE_BEGIN();

	if (!ops)
		return;

	ug_data = ops->priv;

	if (ug_data)
		free(ug_data);

	ops->priv = NULL;

	UG_NFC_SHARE_END();
}
