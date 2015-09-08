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


#include <glib.h>

#include "ug-nfc-setting-main.h"

static Elm_Genlist_Item_Class itc_sep;
static Elm_Genlist_Item_Class itc_sep_help;
static Elm_Genlist_Item_Class itc_helptext;
static Elm_Genlist_Item_Class itc_1text1icon;


static void __radio_changed_cb(void *data, Evas_Object *obj,
	void *event_info)
{
	int value;

	value = elm_radio_value_get(obj);

	if (value == NFC_SECURE_STORAGE_MAN) {
		if (vconf_set_int(VCONFKEY_NFC_WALLET_MODE,
			VCONFKEY_NFC_WALLET_MODE_MANUAL)) {
			LOGE("vconf_set_int failed");
		}
	} else if (value == NFC_SECURE_STORAGE_AUTO) {
		if (vconf_set_int(VCONFKEY_NFC_WALLET_MODE,
			VCONFKEY_NFC_WALLET_MODE_AUTOMATIC)) {
			LOGE("vconf_set_int failed");
		}
	}
}

static char *__gl_text_get(void *data, Evas_Object *obj, const char *part)
{
	int index = (int) data;
	char *text = NULL;

	//LOGD("part:%s", part);
	//LOGD("index:%d", index);

	if (index == 0) {
		text = strdup(IDS_NFC_SECURE_STORAGE_ITEM_MAN);
	} else if (index == 1) {
		text = strdup(IDS_NFC_SECURE_STORAGE_ITEM_AUTO);
	}

	return text;
}

static char *__gl_text_get_des(void *data, Evas_Object *obj,
	const char *part)
{
	int index = (int) data;
	char *text = NULL;

	//LOGD("part:%s", part);
	//LOGD("index:%d", index);

	if (index == 0) {
		text = strdup(IDS_NFC_SECURE_STORAGE_ITEM_MAN_DSC);
	} else if (index == 1) {
		text = strdup(IDS_NFC_SECURE_STORAGE_ITEM_AUTO_DSC);
	}

	return text;
}

static Evas_Object *__gl_content_get(void *data, Evas_Object *obj,
	const char *part)
{
	int index = (int) data;
	Evas_Object *content = NULL;
	Evas_Object *radio_group;
	int intval;


	//LOGD("part:%s", part);
	//LOGD("index:%d", index);

	if (!strcmp(part, "elm.icon")) {
		if (index == 0) {
			radio_group = evas_object_data_get(obj, "radio");

			content = elm_radio_add(obj);
			elm_radio_state_value_set(content,
				NFC_SECURE_STORAGE_MAN);
			elm_radio_group_add(content, radio_group);
			evas_object_propagate_events_set(content, EINA_FALSE);
			evas_object_smart_callback_add(content, "changed",
				__radio_changed_cb, NULL);

			if ((!vconf_get_int(VCONFKEY_NFC_WALLET_MODE, &intval)) &&
				(intval == VCONFKEY_NFC_WALLET_MODE_MANUAL)) {
				elm_radio_value_set(content,
					NFC_SECURE_STORAGE_MAN);
			}
		} else if (index == 1) {
			radio_group = evas_object_data_get(obj, "radio");

			content = elm_radio_add(obj);
			elm_radio_state_value_set(content,
				NFC_SECURE_STORAGE_AUTO);
			elm_radio_group_add(content, radio_group);
			evas_object_propagate_events_set(content, EINA_FALSE);
			evas_object_smart_callback_add(content, "changed",
				__radio_changed_cb, NULL);

			if ((!vconf_get_int(VCONFKEY_NFC_WALLET_MODE, &intval)) &&
				(intval == VCONFKEY_NFC_WALLET_MODE_AUTOMATIC)) {
				elm_radio_value_set(content,
					NFC_SECURE_STORAGE_AUTO);
			}
		}
	}

	return content;
}

static void __gl_sel(void *data, Evas_Object *obj, void *event_info)
{
	Elm_Object_Item *item = (Elm_Object_Item *) event_info;
	Evas_Object *radio;
	int index;

	if (!item)
		return;

	elm_genlist_item_selected_set(item, 0);

	index = (int) elm_object_item_data_get(item);
	LOGD("index:%d", index);

	if (index == 0) {
		radio = elm_object_item_part_content_get(item, "elm.icon");

		elm_radio_value_set(radio, NFC_SECURE_STORAGE_MAN);
		if (!vconf_set_int(VCONFKEY_NFC_WALLET_MODE,
			VCONFKEY_NFC_WALLET_MODE_MANUAL)) {
			LOGD("vconf_set_int success");
		}
	} else if (index == 1) {
		radio = elm_object_item_part_content_get(item,"elm.icon");

		elm_radio_value_set(radio, NFC_SECURE_STORAGE_AUTO);
		if (!vconf_set_int(VCONFKEY_NFC_WALLET_MODE,
			VCONFKEY_NFC_WALLET_MODE_AUTOMATIC)) {
			LOGD("vconf_set_int success");
		}
	}
	elm_genlist_item_update(item);
}

static void __genlist_free_cb(void *data, Evas *e, Evas_Object *obj,
	void *ei)
{
	Evas_Object *radio_group = evas_object_data_get(obj, "radio");

	if (radio_group)
		evas_object_del(radio_group);
}

static Evas_Object *__create_secure_storage_list(void *data)
{
	ugdata_t *ug_data = (ugdata_t *) data;
	Elm_Object_Item* separator;
	Evas_Object *genlist, *radio_group;

	if (!ug_data)
		return NULL;

	genlist = elm_genlist_add(ug_data->base_naviframe);
	if (!genlist) {
		LOGE("elm_genlist_add failed");
		return NULL;
	}

	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);
	evas_object_event_callback_add(genlist, EVAS_CALLBACK_FREE,
		__genlist_free_cb, NULL);

	radio_group = elm_radio_add(genlist);
	elm_radio_state_value_set(radio_group, 0);
	elm_radio_value_set(radio_group, 0);
	evas_object_data_set(genlist, "radio", radio_group);

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

	itc_1text1icon.item_style = "dialogue/1text.1icon.3";
	itc_1text1icon.func.text_get = __gl_text_get;
	itc_1text1icon.func.content_get = __gl_content_get;
	itc_1text1icon.func.state_get = NULL;
	itc_1text1icon.func.del = NULL;

	itc_helptext.item_style = "multiline/1text";
	itc_helptext.func.text_get = __gl_text_get_des;
	itc_helptext.func.content_get = NULL;
	itc_helptext.func.state_get = NULL;
	itc_helptext.func.del = NULL;

	/* SEPARATOR */
	separator = elm_genlist_item_append(genlist, &itc_sep, NULL, NULL,
		ELM_GENLIST_ITEM_NONE, NULL, NULL);
	elm_genlist_item_select_mode_set(separator,
		ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
	elm_object_item_access_unregister(separator);

	/* Manual item : index 0 */
	elm_genlist_item_append(genlist, &itc_1text1icon, (void *)0, NULL,
		ELM_GENLIST_ITEM_NONE, __gl_sel, NULL);
	separator = elm_genlist_item_append(genlist, &itc_sep_help,
		NULL, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	elm_genlist_item_select_mode_set(separator,
		ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
	elm_object_item_access_unregister(separator);

	// Help text
	elm_genlist_item_append(genlist, &itc_helptext, (void *) 0,
		NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	separator = elm_genlist_item_append(genlist, &itc_sep, NULL, NULL,
		ELM_GENLIST_ITEM_NONE, NULL, NULL);
	elm_genlist_item_select_mode_set(separator,
		ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
	elm_object_item_access_unregister(separator);

	/* Automatic item + separator */
	elm_genlist_item_append(genlist, &itc_1text1icon, (void *)1, NULL,
		ELM_GENLIST_ITEM_NONE, __gl_sel, NULL);
	separator = elm_genlist_item_append(genlist, &itc_sep_help,
		NULL, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	elm_genlist_item_select_mode_set(separator,
		ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
	elm_object_item_access_unregister(separator);

	// Help text
	elm_genlist_item_append(genlist, &itc_helptext, (void *) 1,
		NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	separator = elm_genlist_item_append(genlist, &itc_sep, NULL, NULL,
		ELM_GENLIST_ITEM_NONE, NULL, NULL);
	elm_genlist_item_select_mode_set(separator,
		ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
	elm_object_item_access_unregister(separator);

	return genlist;
}

static Eina_Bool __destroy_secure_storage_list(void *data,
	Elm_Object_Item *it)
{
	ugdata_t *ug_data = (ugdata_t *)data;

	if (ug_data && ug_data->ns_genlist) {
		elm_genlist_realized_items_update(ug_data->ns_genlist);
	}

	return EINA_TRUE;
}

void _ug_nfc_secure_storage_create(void *data)
{
	ugdata_t *ug_data = (ugdata_t *)data;
	Evas_Object *genlist;

	if (!ug_data) {
		LOGE("invalid paramater");
		return;
	}

	/* create setting view */
	ug_data->ss_genlist = genlist = __create_secure_storage_list(ug_data);
	if (!genlist)
		return;

	/* Push navifreme */
	ug_data->base_navi_it = elm_naviframe_item_push(ug_data->base_naviframe,
		IDS_NFC_SECURE_STORAGE_HEADER, NULL, NULL, genlist, NULL);
	elm_naviframe_item_pop_cb_set(ug_data->base_navi_it,
		__destroy_secure_storage_list, ug_data);
}


