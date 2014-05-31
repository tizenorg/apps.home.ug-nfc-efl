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

#include "ug-nfc-setting-popup.h"


#define NFC_POPUP_AUTO_TIMEOUT_SEC 3.0


static Evas_Object *_popup = NULL;
static UG_NFC_POPUP_USER_RESP_CB _user_response_cb;
static void *_user_data = NULL;


static void _ug_nfc_setting_popup_response_cb(void *data, Evas_Object *obj, void *event_info)
{
	LOGD("BEGIN >>>>");

	int btn_type = (int)data;
	UG_NFC_POPUP_USER_RESP_CB temp_cb = _user_response_cb;
	void *temp_data = _user_data;

	LOGD("btn_type: %d", (int)btn_type);
	LOGD("Popup is removed: [%p]", obj);
	evas_object_del(_popup);

	_popup = NULL;
	_user_response_cb = NULL;
	_user_data = NULL;

	if (temp_cb)
		temp_cb(temp_data, obj, (void*)btn_type);

	LOGD("END>>>>");
}

static Eina_Bool _ug_nfc_setting_popup_show_cb(void *data)
{
	LOGD("BEGIN >>>>");

	if (_popup)
		evas_object_show(_popup);

	LOGD("END>>>>");

	return ECORE_CALLBACK_CANCEL;
}

static void _ug_nfc_setting_popup_block_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	LOGD("BEGIN >>>>");

	_ug_nfc_setting_popup_response_cb((void*)UG_NFC_POPUP_RESP_CLOSE, _popup, NULL);

	LOGD("END>>>>");
}

Evas_Object *ug_nfc_setting_create_popup(void *data,
                                    Evas_Object *parent_layout,
                                    const char *description,
                                    const char *btn1_text,
                                    int btn1_type,
                                    const char *btn2_text,
                                    int btn2_type,
                                    const char *btn3_text,
                                    int btn3_type,
                                    bool is_alert_type,
                                    bool enable_timeout,
                                    UG_NFC_POPUP_USER_RESP_CB response_cb)
{
	LOGD("BEGIN >>>>");

	Evas_Object *btn = NULL;

	if (data == NULL)
		return NULL;

	if (_popup) {
		ug_nfc_setting_close_popup(_popup);
		_popup = NULL;
	}

	_popup = elm_popup_add(parent_layout);
	LOGD("Popup is created: [%p]", _popup);
	evas_object_size_hint_weight_set(_popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	elm_object_text_set(_popup, description);

	if(btn1_text) {
		btn = elm_button_add(_popup);
		elm_object_style_set (btn, "popup_button/default");
		elm_object_text_set(btn, btn1_text);
		elm_object_part_content_set(_popup, "button1", btn);
		evas_object_smart_callback_add(btn, "clicked", _ug_nfc_setting_popup_response_cb, (void*)btn1_type);
	}
	if (btn2_text) {
		btn = elm_button_add(_popup);
		elm_object_style_set (btn, "popup_button/default");
		elm_object_text_set(btn, btn2_text);
		elm_object_part_content_set(_popup, "button2", btn);
		evas_object_smart_callback_add(btn, "clicked", _ug_nfc_setting_popup_response_cb, (void*)btn2_type);
	}
	if (btn3_text) {
		btn = elm_button_add(_popup);
		elm_object_style_set (btn, "popup_button/default");
		elm_object_text_set(btn, btn3_text);
		elm_object_part_content_set(_popup, "button3", btn);
		evas_object_smart_callback_add(btn, "clicked", _ug_nfc_setting_popup_response_cb, (void*)btn3_type);
	}

	_user_response_cb = response_cb;
	_user_data = data;

	if (is_alert_type) {
		evas_object_smart_callback_add(_popup, "block,clicked", _ug_nfc_setting_popup_block_clicked_cb, NULL);
	}

	if (enable_timeout)
	{
		elm_popup_timeout_set(_popup, NFC_POPUP_AUTO_TIMEOUT_SEC);
		evas_object_smart_callback_add(_popup, "timeout", _ug_nfc_setting_popup_response_cb, NULL);
	}

	_ug_nfc_setting_popup_show_cb(NULL);

	LOGD("END>>>>");

	return _popup;
}


void ug_nfc_setting_close_popup(Evas_Object* popup)
{
	LOGD("BEGIN >>>>");

	if (NULL == popup || NULL == _popup || _popup != popup) {
		LOGD("NULL == popup || NULL == _popup || _popup != popup");
	}

	_ug_nfc_setting_popup_response_cb((void*)UG_NFC_POPUP_RESP_CLOSE, _popup, NULL);

	LOGD("END>>>>");
}


