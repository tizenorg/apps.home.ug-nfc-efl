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

#include <efl_assist.h>
#include <notification.h>

#include "ug-nfc-setting-main.h"
#include "ug-nfc-setting-popup.h"
#include "ug-nfc-setting-db.h"

#ifndef UG_MODULE_API
#define UG_MODULE_API __attribute__ ((visibility("default")))
#endif

#define SETTING_IMG_NFC	\
	"/usr/apps/com.samsung.setting/res/icons/settings_nfc.png"

static Elm_Genlist_Item_Class itc_sep;
static Elm_Genlist_Item_Class itc_sep_help;
static Elm_Genlist_Item_Class itc_helptext;
static Elm_Genlist_Item_Class itc_2text1con;
static Elm_Genlist_Item_Class itc_2text;
static Elm_Genlist_Item_Class itc_onoff;

static Elm_Object_Item *pd_item;
static Elm_Object_Item *ss_item;

static bool pending_status = false;
static Eina_Bool rotate_flag = EINA_FALSE;

static void __nfc_activation_completed_cb(nfc_error_e error, void *user_data);
static void __nfc_activation_changed_cb(bool activated , void *user_data);

static void __show_app_error_popup_response_cb(void *data,
	Evas_Object *obj, void *event_info)
{
	ugdata_t *ug_data = (ugdata_t *)data;
	int result = (int)event_info;

	if (!ug_data) {
		LOGE("invalid parameter");
		return;
	}

	switch (result) {
	case UG_NFC_POPUP_RESP_CANCEL :
		/* destroy UG */
		LOGD("ug_destroy_me >>>>", __FUNCTION__, __LINE__);
		ug_destroy_me(ug_data->nfc_setting_ug);
		break;

	default :
		break;
	}
}

static void __show_app_error_popup(void *data)
{
	ugdata_t *ug_data = (ugdata_t *)data;
	char popup_str[POPUP_TEXT_SIZE] = { 0, };

	if (ug_data == NULL)
		return;

	memcpy(popup_str, IDS_NFC_SERVICE_IS_NOT_SUPPORTED,
		strlen(IDS_NFC_SERVICE_IS_NOT_SUPPORTED));

	/* To do: popup_title */
	ug_nfc_setting_create_popup(ug_data,
		ug_data->base_layout,
		NULL,
		popup_str,
		IDS_CLOSE, UG_NFC_POPUP_RESP_CANCEL,
		NULL, 0,
		NULL, 0,
		false, false,
		__show_app_error_popup_response_cb);
}

static void __nfc_activation_failed_popup_res_cb(void *data,
	Evas_Object *obj, void *event_info)
{

}

static void __nfc_activation_failed_popup_lang_changed_cb(void *data,
	Evas_Object *obj, void *event_info)
{
	Evas_Object *button;

	if (obj == NULL)
		return;

	elm_object_part_text_set(obj, "title,text", IDS_FAILED_TO_TURN_ON_NFC);
	elm_object_text_set(obj, IDS_AN_ERROR_OCCURRED_WHILE_TURNING_ON_NFC_TRY_AGAIN);

	button = elm_object_part_content_get(obj, "button1");
	elm_object_text_set(button, IDS_OK);
}

static void __nfc_activation_failed_popup(void *data)
{
	ugdata_t *ug_data = (ugdata_t *)data;
	Evas_Object *popup;

	g_assert(ug_data != NULL);

	popup = ug_nfc_setting_create_popup(ug_data,
		ug_data->base_layout,
		IDS_FAILED_TO_TURN_ON_NFC,
		IDS_AN_ERROR_OCCURRED_WHILE_TURNING_ON_NFC_TRY_AGAIN,
		IDS_OK, UG_NFC_POPUP_RESP_OK,
		NULL, 0,
		NULL, 0,
		false, false, __nfc_activation_failed_popup_res_cb);

	evas_object_smart_callback_add(popup, "language,changed",
		__nfc_activation_failed_popup_lang_changed_cb, NULL);
}

static char * __get_text_turned_on_popup(void *data)
{
	ugdata_t *ug_data = (ugdata_t *)data;
	char *text = NULL;
	int boolval = 0;

	if (!ug_data) {
		LOGE("invalid parameter");
		return NULL;
	}

	if (ug_data->menu_type == MENU_NFC) {
		if (!vconf_get_bool(VCONFKEY_NFC_STATE, &boolval)) {
			if (boolval)
				text = strdup(IDS_NFC_TURNED_ON);
		}
	}

	return text;
}

static bool __get_pending_status(void)
{
	return pending_status;
}

static void __set_pending_status(bool status)
{
	pending_status = status;
}

static bool __reply_to_launch_request(app_control_h service, app_control_result_e result)
{
	app_control_h reply;
	char *operation = NULL;
	bool ret = false;
	LOGD("BEGIN >>>>");

	if(service != NULL)
	{
		app_control_create(&reply);
		app_control_get_operation(service, &operation);

		if (operation != NULL)
		{
			LOGD("reply to launch request : operation %s", operation);
			app_control_reply_to_launch_request(reply, service, result);
			ret = true;
		}

		app_control_destroy(reply);
	}

	LOGD("END >>>>");

	return ret;
}

static Eina_Bool __back_clicked_cb(void *data, Elm_Object_Item *it)
{
	ugdata_t *ug_data = (ugdata_t *)data;

	LOGD("BEGIN >>>>");

	if(!ug_data) {
		LOGE("data is null");
		return EINA_FALSE;
	}

	__reply_to_launch_request(ug_data->service, APP_CONTROL_RESULT_FAILED);

	_ug_nfc_setting_db_close();
	ug_destroy_me(ug_data->nfc_setting_ug);

	LOGD("END >>>>");

	return EINA_FALSE;
}

static void __update_title_onoff_obj(void *data)
{
	ugdata_t *ug_data = (ugdata_t *)data;
	int boolval;

	if (!ug_data)
		return;

	if (__get_pending_status()) {
		elm_object_disabled_set(ug_data->ns_on_off, EINA_TRUE);
		return;
	}

	elm_object_disabled_set(ug_data->ns_on_off, EINA_FALSE);
	if (ug_data->menu_type == MENU_NFC) {
		if (!vconf_get_bool(VCONFKEY_NFC_STATE, &boolval) &&
			boolval) {
			elm_check_state_set(ug_data->ns_on_off, EINA_TRUE);
		} else {
			elm_check_state_set(ug_data->ns_on_off, EINA_FALSE);
		}
	}
}

static void __change_nfc_onoff_setting(void *data)
{
	ugdata_t *ug_data = (ugdata_t *)data;
	int result, boolval;

	if (!ug_data)
		return;

	if (!vconf_get_bool(VCONFKEY_NFC_STATE, &boolval)) {
		LOGD("vconf_get_bool status [%d]", boolval);

		if (NFC_ERROR_NONE == nfc_manager_initialize()) {

			/* Register activation changed callback */
			nfc_manager_set_activation_changed_cb(
				__nfc_activation_changed_cb, ug_data);

			result = nfc_manager_set_activation(!boolval,
				__nfc_activation_completed_cb, ug_data);
			if (result != NFC_ERROR_NONE) {
				LOGE("nfc_manager_set_activation failed");
				return;
			}

			__set_pending_status(true);
		} else {
			LOGE("nfc_manager_initialize FAIL!!!!");
		}


	} else {
		LOGE("vconf_get_bool failed");
	}

	__update_title_onoff_obj(ug_data);
}

void __change_predefined_item_onoff_setting(void *data)
{
	ugdata_t *ug_data = (ugdata_t *)data;
	int boolval;

	if (!ug_data)
		return;

	if (!vconf_get_bool(VCONFKEY_NFC_PREDEFINED_ITEM_STATE, &boolval)) {
		LOGD("vconf_get_bool status [%d]", boolval);

		if (boolval) {
			if (vconf_set_bool(
				VCONFKEY_NFC_PREDEFINED_ITEM_STATE,
				VCONFKEY_NFC_PREDEFINED_ITEM_OFF))
				LOGE("vconf_set_bool failed");
		} else {
			if (vconf_set_bool(
				VCONFKEY_NFC_PREDEFINED_ITEM_STATE,
				VCONFKEY_NFC_PREDEFINED_ITEM_ON))
				LOGE("vconf_set_bool failed");
		}
	} else {
		LOGE("vconf_get_bool failed");
	}


}

static void __nfc_activation_completed_cb(nfc_error_e error,
	void *user_data)
{
	ugdata_t *ug_data = (ugdata_t *)user_data;

	g_assert(ug_data != NULL);

	if (error != NFC_ERROR_NONE) {
		LOGE("__nfc_activation_completed_cb failed");

		/* show failure popup */
		__nfc_activation_failed_popup(ug_data);
	}
}

static void __nfc_activation_changed_cb(bool activated , void *user_data)
{
	ugdata_t *ug_data = (ugdata_t *)user_data;

	LOGD("nfc mode %s ", activated ? "ON" : "OFF");

	nfc_manager_unset_activation_changed_cb();

	/* nfc setting ui updated */
	__set_pending_status(false);

	__update_title_onoff_obj(ug_data);

	if(__reply_to_launch_request(ug_data->service, APP_CONTROL_RESULT_SUCCEEDED) == true)
		ug_destroy_me(ug_data->nfc_setting_ug);
}

static void __title_ns_on_off_clicked_cb(void *data, Evas_Object *obj,
	void *event_info)
{
	gl_item_data *item_data = (gl_item_data *)data;
	ugdata_t *ug_data = item_data->data;

	if (ug_data == NULL) {
		LOGE("data is null");
		return;
	}

	if (__get_pending_status())
		return;

	if (ug_data->menu_type == MENU_NFC) {
		__change_nfc_onoff_setting(ug_data);
	}
}

static void __ug_layout_cb(ui_gadget_h ug, enum ug_mode mode, void *priv)
{
	Evas_Object *base;

	if (ug == NULL) {
		LOGE("data is null");
		return;
	}

	base = ug_get_layout(ug);
	if (!base) {
		LOGE("ug_get_layout() return NULL");
		ug_destroy(ug);
		return;
	}

	evas_object_size_hint_weight_set(base, EVAS_HINT_EXPAND,
		EVAS_HINT_EXPAND);
	evas_object_show(base);
}

static void __ug_destroy_cb(ui_gadget_h ug, void *data)
{
	if (ug == NULL) {
		LOGE("data is null");
		return;
	}
	ug_destroy(ug);

	ea_theme_style_set(EA_THEME_STYLE_DARK);
}

static void __popup_del_cb(void *data, Evas *e, Evas_Object *obj,
	void *event_info)
{
	evas_object_event_callback_del(obj, EVAS_CALLBACK_DEL, __popup_del_cb);
}

static void __popup_back_cb(void *data, Evas_Object *obj,
	void *event_info)
{
	evas_object_del(obj);
}

static void __popup_check_changed_cb(void *data, Evas_Object *obj,
	void *event_info)
{
	Evas_Object *check = obj;
	Eina_Bool boolval;

	boolval = elm_check_state_get(check);

	if(vconf_set_bool(SECURE_STORAGE_FIRST_TIME_POPUP_SHOW_KEY,
		(const int)!boolval))
		LOGE("vconf_set_bool failed");
}

static void __ok_btn_clicked_cb(void *data, Evas_Object *obj,
	void *event_info)
{
	ugdata_t *ug_data = (ugdata_t *)data;

	if(!ug_data)
		return;

	if (ug_data->popup)
		evas_object_del(ug_data->popup);

	_ug_nfc_secure_storage_create(ug_data);
}

static void __create_secure_storage_comfirm_popup(void *data)
{
	ugdata_t *ug_data = (ugdata_t *)data;
	Evas_Object *layout, *popup, *check, *label, *btn1;


	if (!ug_data)
		return;

	ug_data->popup = popup = elm_popup_add(ug_data->base_layout);
	evas_object_event_callback_add(popup, EVAS_CALLBACK_DEL,
		__popup_del_cb, NULL);
	ea_object_event_callback_add(popup, EA_CALLBACK_BACK,
		__popup_back_cb, NULL);

	elm_object_part_text_set(popup, "title,text",
		IDS_NFC_SECURE_STORAGE_BODY);

	label = elm_label_add(popup);
	elm_label_line_wrap_set(label, ELM_WRAP_MIXED);
	elm_object_text_set(label, IDS_NFC_SECURE_STORAGE_TIPS);
	evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, 0.0);
	evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(label);

	layout = elm_layout_add(ug_data->base_layout);
	if(!layout) {
		LOGE("layout is NULL");
		return;
	}

	elm_layout_file_set(layout, EDJ_FILE, "popup_with_check");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND,
		EVAS_HINT_EXPAND);

	check = elm_check_add(popup);
//	elm_object_style_set(check, "multiline");
	elm_object_text_set(check, IDS_NFC_DO_NOT_SHOW_AGAIN);
	elm_check_state_set(check, EINA_FALSE);
	evas_object_size_hint_align_set(check, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(check, EVAS_HINT_EXPAND,
		EVAS_HINT_EXPAND);
	evas_object_smart_callback_add(check, "changed",
		__popup_check_changed_cb, NULL);
	evas_object_show(check);

	elm_object_part_content_set(layout, "elm.swallow.content", label);
	elm_object_part_content_set(layout, "elm.swallow.end", check);

	evas_object_show(layout);
	elm_object_content_set(popup, layout);
	btn1 = elm_button_add(popup);
	elm_object_style_set(btn1, "popup");
	elm_object_text_set(btn1, IDS_OK);
	elm_object_part_content_set(popup, "button1", btn1);
	evas_object_smart_callback_add(btn1, "clicked", __ok_btn_clicked_cb,
		ug_data);

	evas_object_show(popup);
}

static void __pd_onoff_changed_cb(void *data, Evas_Object *obj,
	void *event_info)
{
	gl_item_data *item_data = (gl_item_data *)data;

	if (item_data->type == NFC_PREDEFINED_ITEM) {
		__change_predefined_item_onoff_setting(item_data->data);

		if (pd_item != NULL)
			elm_genlist_item_update(pd_item);
	} else {
		LOGE("item_data->type error");
	}
}

// get the state of item
static Eina_Bool __gl_state_get(void *data, Evas_Object *obj, const char *part)
{
	Eina_Bool result = EINA_FALSE;

	gl_item_data *item_data = (gl_item_data *)data;

	if (!item_data) {
		LOGE("invalid parameter");
		return EINA_FALSE;
	}

	return result;
}

// callback for 'deletion'
static void __gl_del(void *data, Evas_Object *obj)
{
	gl_item_data *item_data = (gl_item_data *)data;

	if (item_data != NULL)
		free(item_data);
}

static void __gl_sel_activation(void *data, Evas_Object *obj,
	void *event_info)
{
	Elm_Object_Item *item = (Elm_Object_Item *)event_info;
	gl_item_data *item_data = (gl_item_data *)data;
	ugdata_t *ug_data;


	elm_genlist_item_selected_set(item, 0);

	if (!item || !item_data) {
		LOGE("invalid parameter");
		return;
	}

	ug_data = item_data->data;
	if (!ug_data) {
		LOGE("invalid parameter");
		return;
	}

	if (item_data->type == NFC_PREDEFINED_ITEM) {
		_ug_nfc_predefined_item_create(ug_data);
	} else if (item_data->type == NFC_SECURE_STORAGE) {
		int boolval;

		if (!vconf_get_bool(SECURE_STORAGE_FIRST_TIME_POPUP_SHOW_KEY,
			&boolval)) {
			if (boolval)
				__create_secure_storage_comfirm_popup(ug_data);
			else
				_ug_nfc_secure_storage_create(ug_data);
		}
	}
}

static Evas_Object *__gl_content_get(void *data, Evas_Object *obj,
	const char *part)
{
	gl_item_data *item_data = (gl_item_data *)data;
	ugdata_t *ug_data;
	Evas_Object *content = NULL;
	int boolval;

	//LOGD("part:%s", part);

	if (!item_data) {
		LOGE("invalid parameter");
		return NULL;
	}

	ug_data = item_data->data;
	if (!ug_data) {
		LOGE("invalid parameter");
		return NULL;
	}

	if (!strcmp(part, "elm.icon")) {
		if (item_data->type == NFC_PREDEFINED_ITEM) {
			content = elm_check_add(obj);
			evas_object_propagate_events_set(content, EINA_FALSE);
			evas_object_smart_callback_add(content, "changed",
				__pd_onoff_changed_cb, item_data);

			if (!vconf_get_bool(VCONFKEY_NFC_PREDEFINED_ITEM_STATE,
				&boolval) && boolval) {
				elm_check_state_set(content, EINA_TRUE);
			} else {
				elm_check_state_set(content, EINA_FALSE);
			}

			elm_object_style_set(content, "on&off");
			return content;
		}
	}
	else if (!strncmp(part, "elm.icon.2", strlen(part))) {
		Evas_Object *tg = NULL;

		content = elm_layout_add(obj);
		elm_layout_theme_set(content, "layout", "list/C/type.3", "default");
		tg = elm_check_add(content);

		int boolval = false;
		int ret = vconf_get_bool(VCONFKEY_NFC_STATE, &boolval);

		if (boolval == false){
			elm_check_state_set(tg, EINA_FALSE);
		} else {
			elm_check_state_set(tg, EINA_TRUE);
		}

		elm_object_style_set(tg, "on&off");
		evas_object_propagate_events_set(tg, EINA_FALSE);
		evas_object_smart_callback_add(tg, "changed", __title_ns_on_off_clicked_cb, item_data);

		evas_object_show(tg);

		elm_layout_content_set(content, "elm.swallow.content", tg);
		ug_data->ns_on_off = tg;

		__update_title_onoff_obj(ug_data);

		return content;
	}

	return content;
}

static char *__gl_text_get(void *data, Evas_Object *obj,
	const char *part)
{
	gl_item_data *item_data = (gl_item_data *)data;
	char *text = NULL;
	int intval;

	//LOGD("part:%s", part);

	// label for 'elm.text' part
	if (!item_data) {
		LOGE("invalid parameter");
		return NULL;
	}

	if (!strcmp(part, "elm.text.1")) {
		if (item_data->type == NFC_PREDEFINED_ITEM) {
			text = strdup(IDS_USE_NFC_IN_HOME);
		} else if (item_data->type == NFC_SECURE_STORAGE) {
			text = strdup(IDS_NFC_SECURE_STORAGE_BODY);
		}
	} else if (!strcmp(part, "elm.text.2")) {
		if (item_data->type == NFC_PREDEFINED_ITEM) {
			app_info_h app_info = NULL;
			char *app_id = NULL;

			app_id = vconf_get_str(VCONFKEY_NFC_PREDEFINED_ITEM);
			if (!app_id) {
				return NULL;
			}

			if (APP_MANAGER_ERROR_NONE == app_info_create(app_id,
				&app_info)) {
				if (APP_MANAGER_ERROR_NONE !=
					app_info_get_label(app_info, &text))
					_ug_nfc_setting_db_get_pkgName(app_id, &text);
			} else {
				_ug_nfc_setting_db_get_pkgName(app_id, &text);
			}

			if (app_info)
				app_info_destroy(app_info);
			free(app_id);
		} else if (item_data->type == NFC_SECURE_STORAGE) {
			if (! vconf_get_int(VCONFKEY_NFC_WALLET_MODE, &intval)){
				if (intval == VCONFKEY_NFC_WALLET_MODE_MANUAL)
					text = strdup(IDS_NFC_SECURE_STORAGE_ITEM_MAN);
				else
					text = strdup(IDS_NFC_SECURE_STORAGE_ITEM_AUTO);
			} else {
				LOGE("vconf_get_int failed");
				text = strdup(IDS_NFC_SECURE_STORAGE_ITEM_MAN);
			}
		}
	}
	else if (!strncmp(part, "elm.text.main.left", strlen(part))) {
		 text = strdup(IDS_NFC_NFC);
	}

	return text;
}

static char *__gl_text_get_des(void *data, Evas_Object *obj,
	const char *part)
{
	int index = (int) data;
	char *text = NULL;

	//LOGD("index:%d", index);

	if (index == 0) {
		text = strdup(IDS_NFC_DESCRIPTION_MSG);
	} else if (index == 1) {
		text = strdup(IDS_PREDEFINED_ITEM_DESCRIPTION_MSG);
	} else if (index == 2) {
		text = strdup(IDS_NFC_S_BEAM_DESCRIPTION_MSG);
	}

	return text;
}
static void __nfc_sel(void *data, Evas_Object *obj, void *event_info)
{
	__title_ns_on_off_clicked_cb(data, obj, event_info);

	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);
}

static Evas_Object *__create_nfc_setting_list(void *data)
{
	ugdata_t *ug_data = (ugdata_t *)data;
	Evas_Object *genlist = NULL;
	Elm_Object_Item* separator = NULL;
	int boolval;


	/* make genlist */
	genlist = elm_genlist_add(ug_data->base_naviframe);
	if (genlist == NULL) {
		LOGE("genlist is null");
		return NULL;
	}
	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);

	itc_sep.item_style = "dialogue/separator";
	itc_sep.func.text_get = NULL;
	itc_sep.func.content_get = NULL;
	itc_sep.func.state_get = NULL;
	itc_sep.func.del = NULL;

	itc_sep_help.item_style = "dialogue/separator.2";
	itc_sep_help.func.text_get = NULL;
	itc_sep_help.func.content_get = NULL;
	itc_sep_help.func.state_get = NULL;
	itc_sep_help.func.del = NULL;

	itc_helptext.item_style = "multiline_sub";
	itc_helptext.func.text_get = __gl_text_get_des;
	itc_helptext.func.content_get = NULL;
	itc_helptext.func.state_get = NULL;
	itc_helptext.func.del = NULL;

	itc_2text1con.item_style = "dialogue/2text.1icon.10";
	itc_2text1con.func.text_get = __gl_text_get;
	itc_2text1con.func.content_get = __gl_content_get;
	itc_2text1con.func.state_get = __gl_state_get;
	itc_2text1con.func.del = __gl_del;

	itc_2text.item_style = "dialogue/2text";
	itc_2text.func.text_get = __gl_text_get;
	itc_2text.func.content_get = NULL;
	itc_2text.func.state_get = NULL;
	itc_2text.func.del = __gl_del;

	itc_onoff.item_style = "1line";
	itc_onoff.func.text_get = __gl_text_get;
	itc_onoff.func.content_get = __gl_content_get;
	itc_onoff.func.state_get = NULL;
	itc_onoff.func.del = __gl_del;

	gl_item_data *item_data = NULL;
	static Elm_Object_Item *onoff_item;

	item_data = (gl_item_data *)g_malloc0((gint)sizeof(gl_item_data));

	if (!item_data)
		return NULL;

	item_data->data = ug_data;
	onoff_item = elm_genlist_item_append(genlist, &itc_onoff,
				(void *) item_data, NULL, ELM_GENLIST_ITEM_NONE,
				__nfc_sel, (void *) item_data);
	elm_object_item_signal_emit(onoff_item, "elm,state,top", "");


	/* NFC Help Text + SEPARATOR */
	static Elm_Object_Item *help_item;
	help_item = elm_genlist_item_append(genlist, &itc_helptext, (void *)0,
		NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	elm_genlist_item_select_mode_set(help_item,
		ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
	elm_object_item_access_unregister(help_item);

	/* NFC Secure storage */
	/*
	if (csc_feature_get_bool(CSC_FEATURE_DEF_BOOL_NFC_CARD_ESE_DISABLE)
		!= CSC_FEATURE_BOOL_TRUE) {
		gl_item_data *item_data = NULL;

		LOGD("NFC Secure storage added");
		item_data = (gl_item_data *)g_malloc0((gint)sizeof(gl_item_data));
		if (!item_data)
			return NULL;
		item_data->data = ug_data;
		item_data->type = NFC_SECURE_STORAGE;
		ss_item = elm_genlist_item_append(genlist, &itc_2text,
			(void *) item_data, NULL, ELM_GENLIST_ITEM_NONE,
			__gl_sel_activation, (void *) item_data);
	}
	*/

	/* Predefined item setting + SEPARATOR */
	if (_ug_nfc_check_predefined_item_available()) {
		gl_item_data *item_data = NULL;

		LOGD("NFC predefined item added");
		item_data = (gl_item_data *)g_malloc0((gint)sizeof(gl_item_data));
		if (!item_data)
			return NULL;
		item_data->type = NFC_PREDEFINED_ITEM;
		item_data->data = ug_data;
		pd_item = elm_genlist_item_append(genlist,
			&itc_2text1con,
			(void *)item_data,
			NULL, ELM_GENLIST_ITEM_NONE,
			__gl_sel_activation, (void *)item_data);

		/* SEPARATOR.2 */
		separator = elm_genlist_item_append(genlist, &itc_sep_help,
			NULL, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
		elm_genlist_item_select_mode_set(separator,
			ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
		elm_object_item_access_unregister(separator);

		/* Help Text*/
		elm_genlist_item_append(genlist, &itc_helptext, (void *)1,
			NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	}

	/* set the visibility of predefine and storage item */
	if (!vconf_get_bool(VCONFKEY_NFC_STATE, &boolval) && !boolval) {
		if (vconf_set_bool(VCONFKEY_NFC_PREDEFINED_ITEM_STATE,
			VCONFKEY_NFC_PREDEFINED_ITEM_OFF)) {
			LOGE("vconf_set_bool failed");
		}

		if (pd_item != NULL)
			elm_object_item_disabled_set(pd_item, EINA_TRUE);
		if (ss_item != NULL)
			elm_object_item_disabled_set(ss_item, EINA_TRUE);
	}

	evas_object_show(genlist);

	return genlist;
}

static Evas_Object *__create_bg(Evas_Object *parent, char *style)
{
	Evas_Object *bg = elm_bg_add(parent);

	evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	if (style)
		elm_object_style_set(bg, style);

	elm_win_resize_object_add(parent, bg);

	evas_object_show(bg);

	return bg;
}

static Evas_Object *__create_main_layout(Evas_Object *parent)
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

static void *__ug_nfc_setting_create(ui_gadget_h ug, enum ug_mode mode,
	app_control_h service, void *priv)
{
	ugdata_t *ug_data = (ugdata_t *)priv;
	Evas_Object *parent = NULL;
	Evas_Object *nfc_setting_list = NULL;
	Evas_Object *l_button = NULL;
	char *type = NULL;
	char *keyword = NULL;

	LOGD("BEGIN >>>>");

	/* set text domain */
	bindtextdomain(NFCUG_TEXT_DOMAIN, NFCUG_LOCALEDIR);

	/* create base view */
	parent = ug_get_parent_layout(ug);
	if (!parent)
		return NULL;
	ug_data->ug_win_main = parent;
	evas_object_show(ug_data->ug_win_main);
	ug_data->nfc_setting_ug = ug;

	ug_data->base_layout = __create_main_layout(ug_data->ug_win_main);
	ug_data->bg = __create_bg(ug_data->ug_win_main, "group_list");
	elm_object_part_content_set(ug_data->base_layout, "elm.swallow.bg",
		ug_data->bg);

	ug_data->base_naviframe = elm_naviframe_add(ug_data->base_layout);
	elm_object_part_content_set(ug_data->base_layout, "elm.swallow.content",
		ug_data->base_naviframe);
	evas_object_show(ug_data->base_naviframe);

	ug_data->service = service;

	/* parse parameter */
	app_control_get_extra_data(service, "type", &type);
	app_control_get_extra_data(service, "keyword", &keyword);

	if (type) {
		LOGD("type [%s]", type);

		if (strncmp(type, "nfc", strlen("nfc")) == 0) {
			_ug_nfc_setting_db_open();
			nfc_setting_list = __create_nfc_setting_list(ug_data);
			ug_data->menu_type = MENU_NFC;
		}
	} else if (keyword) {
		LOGD("keyword [%s]", keyword);

		if (strncmp(keyword, "IDS_NFC_BODY_NFC", strlen("IDS_NFC_BODY_NFC")) == 0 ||
		    strncmp(keyword, "IDS_NFC_MBODY_NFC_SECURE_STORAGE",
				strlen("IDS_NFC_MBODY_NFC_SECURE_STORAGE"))){
			_ug_nfc_setting_db_open();
			nfc_setting_list = __create_nfc_setting_list(ug_data);
			ug_data->menu_type = MENU_NFC;
		}
	}

	if (nfc_setting_list == NULL) {
		LOGE("wrong type");
		return NULL;
	}

	ug_data->ns_genlist = nfc_setting_list;

	/* Push navifreme */
	l_button = elm_button_add(ug_data->base_naviframe);
	elm_object_style_set(l_button, "naviframe/back_btn/default");
	ea_object_event_callback_add(ug_data->base_naviframe, EA_CALLBACK_BACK,
		ea_naviframe_back_cb, NULL);

	if (ug_data->menu_type == MENU_NFC) {
		ug_data->base_navi_it = elm_naviframe_item_push(
			ug_data->base_naviframe,
			IDS_NFC_NFC,
			l_button,
			NULL,
			nfc_setting_list,
			NULL);
	}
	elm_naviframe_item_pop_cb_set(ug_data->base_navi_it, __back_clicked_cb,
		ug_data);

	LOGD("END >>>>");

	return ug_data->base_layout;
}

static void __ug_nfc_setting_destroy(ui_gadget_h ug, app_control_h service,
	void *priv)
{
	ugdata_t *ug_data = (ugdata_t *)priv;

	LOGD("BEGIN >>>>");

	if ((ug_data == NULL) || (ug == NULL))
		return;

	if (nfc_manager_deinitialize() != NFC_ERROR_NONE)
		LOGE("nfc_manager_deinitialize failed");

	evas_object_del(ug_get_layout(ug));

	LOGD("END >>>>");
}

static void __ug_nfc_setting_start(ui_gadget_h ug, app_control_h service,
	void *priv)
{
	ugdata_t *ug_data = (ugdata_t *)priv;

	LOGD("BEGIN >>>>");

	/* check nfc-device*/
	if (!nfc_manager_is_supported()) {
		LOGE("It is not nfc device >>>>");
		__show_app_error_popup(ug_data);
	}

	if (NFC_ERROR_NONE != nfc_manager_initialize())
		LOGE("nfc_manager_initialize FAIL!!!!");

	LOGD("END >>>>");
}

static void __ug_nfc_setting_pause(ui_gadget_h ug, app_control_h service,
	void *priv)
{
	LOGD("BEGIN >>>>");
	LOGD("END >>>>");
}

static void __ug_nfc_setting_resume(ui_gadget_h ug, app_control_h service,
	void *priv)
{
	LOGD("BEGIN >>>>");
	LOGD("END >>>>");
}


static void __ug_nfc_setting_message(ui_gadget_h ug, app_control_h msg,
	app_control_h service, void *priv)
{
	LOGD("BEGIN >>>>");
	LOGD("END >>>>");
}

static void __ug_nfc_setting_event(ui_gadget_h ug, enum ug_event event,
	app_control_h service, void *priv)
{
	LOGD("BEGIN >>>>");
	LOGD("END >>>>");
}

UG_MODULE_API int UG_MODULE_INIT(struct ug_module_ops *ops)
{
	ugdata_t *ug_data; // User defined private data

	LOGD("UG_MODULE_INIT!!");

	if (!ops)
		return -1;

	ug_data = (ugdata_t *)g_malloc0((gint)sizeof(ugdata_t));
	if (!ug_data)
		return -1;

	ops->create = __ug_nfc_setting_create;
	ops->start = __ug_nfc_setting_start;
	ops->pause = __ug_nfc_setting_pause;
	ops->resume = __ug_nfc_setting_resume;
	ops->destroy = __ug_nfc_setting_destroy;
	ops->message = __ug_nfc_setting_message;
	ops->event = __ug_nfc_setting_event;
	ops->priv = ug_data;
	ops->opt = UG_OPT_INDICATOR_ENABLE;

	return 0;
}

UG_MODULE_API void UG_MODULE_EXIT(struct ug_module_ops *ops)
{
	ugdata_t *ug_data;

	LOGD("UG_MODULE_EXIT!!");

	if (!ops)
		return;

	ug_data = ops->priv;

	if (ug_data)
		free(ug_data);

	ops->priv = NULL;
}
