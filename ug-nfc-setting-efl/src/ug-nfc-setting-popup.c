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

#include "ug-nfc-setting-popup.h"

#include <utilX.h>

#define NFC_POPUP_AUTO_TIMEOUT_SEC 3.0
#define MOUSE_RIGHT_BTN_UP  3

static Evas_Object *_popup = NULL;
static UG_NFC_POPUP_USER_RESP_CB _user_response_cb;
static void *_user_data = NULL;

static void _remove_key_event_handler(void);
static void _add_key_event_handler(void);

static void _ug_nfc_setting_popup_response_cb(void *data, Evas_Object *obj, void *event_info)
{
	int btn_type = (int)data;

	if (_user_response_cb)
		_user_response_cb(_user_data, obj, (void*)btn_type);

	LOGD("btn_type: %d", (int)btn_type);
	LOGD("Popup is removed: [%p]", obj);
	evas_object_del(_popup);

	_popup = NULL;
	_user_response_cb = NULL;
	_user_data = NULL;
}

static Eina_Bool _ug_nfc_setting_popup_show_cb(void *data)
{
	if (_popup)
		evas_object_show(_popup);

	return ECORE_CALLBACK_CANCEL;
}

static void _popup_back_click_cb(void)
{
	if (!_popup)
		return;

	_remove_key_event_handler();

	_ug_nfc_setting_popup_response_cb((void*)UG_NFC_POPUP_RESP_CANCEL, _popup, NULL);
}

static void _mouseup_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	Evas_Event_Mouse_Up *ev = event_info;

	if (!ev)
		return;

	if (ev->button == MOUSE_RIGHT_BTN_UP)
		_popup_back_click_cb();
}

static void _keydown_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	Evas_Event_Key_Down *ev = event_info;

	if (!ev)
		return;

	if (!strcmp(ev->keyname, KEY_BACK))
		_popup_back_click_cb();
}

static void _remove_key_event_handler(void)
{
	if (!_popup)
		return;

	evas_object_event_callback_del(_popup, EVAS_CALLBACK_MOUSE_UP, _mouseup_cb);
	evas_object_event_callback_del(_popup, EVAS_CALLBACK_KEY_DOWN, _keydown_cb);
}

static void _add_key_event_handler(void)
{
	if (!_popup)
		return;

	evas_object_event_callback_add(_popup, EVAS_CALLBACK_MOUSE_UP, _mouseup_cb, NULL);
	evas_object_event_callback_add(_popup, EVAS_CALLBACK_KEY_DOWN, _keydown_cb, NULL);
}

static void _ug_nfc_setting_popup_block_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	_ug_nfc_setting_popup_response_cb((void*)UG_NFC_POPUP_RESP_CANCEL, _popup, NULL);
}

Evas_Object *ug_nfc_setting_create_popup(void *data,
                                    Evas_Object *parent_layout,
                                    const char *title,
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

	if (title)
		elm_object_part_text_set(_popup, "title,text", title);

	elm_object_text_set(_popup, description);

	if(btn1_text) {
		btn = elm_button_add(_popup);
		elm_object_style_set (btn, "popup");
		elm_object_text_set(btn, btn1_text);
		elm_object_part_content_set(_popup, "button1", btn);
		evas_object_smart_callback_add(btn, "clicked",
			_ug_nfc_setting_popup_response_cb, (void*)btn1_type);
	}
	if (btn2_text) {
		btn = elm_button_add(_popup);
		elm_object_style_set (btn, "popup");
		elm_object_text_set(btn, btn2_text);
		elm_object_part_content_set(_popup, "button2", btn);
		evas_object_smart_callback_add(btn, "clicked",
			_ug_nfc_setting_popup_response_cb, (void*)btn2_type);
	}
	if (btn3_text) {
		btn = elm_button_add(_popup);
		elm_object_style_set (btn, "popup");
		elm_object_text_set(btn, btn3_text);
		elm_object_part_content_set(_popup, "button3", btn);
		evas_object_smart_callback_add(btn, "clicked",
			_ug_nfc_setting_popup_response_cb, (void*)btn3_type);
	}

	_user_response_cb = response_cb;
	_user_data = data;

	if (is_alert_type) {
		evas_object_smart_callback_add(_popup, "block,clicked",
			_ug_nfc_setting_popup_block_clicked_cb, NULL);
	}

	if (enable_timeout) {
		elm_popup_timeout_set(_popup, NFC_POPUP_AUTO_TIMEOUT_SEC);
		evas_object_smart_callback_add(_popup, "timeout",
			_ug_nfc_setting_popup_response_cb, NULL);
	}

	_add_key_event_handler();

	_ug_nfc_setting_popup_show_cb(NULL);

	return _popup;
}


void ug_nfc_setting_close_popup(Evas_Object* popup)
{
	if (NULL == popup || NULL == _popup || _popup != popup) {
		LOGD("NULL == popup || NULL == _popup || _popup != popup");
	}

	_ug_nfc_setting_popup_response_cb((void*)UG_NFC_POPUP_RESP_CANCEL, _popup, NULL);
}


