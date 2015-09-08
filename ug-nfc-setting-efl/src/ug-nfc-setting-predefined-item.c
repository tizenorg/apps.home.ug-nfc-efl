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
#include "ug-nfc-setting-db.h"


typedef struct {
	char *app_id;
	char *pkgName;
	char *iconPath;
	char *msgDes;
} pd_data;


static int dbtable_h;
static int pd_item_cnt;
static GArray *pd_item_l;

static Elm_Genlist_Item_Class itc_sep;
static Elm_Genlist_Item_Class itc_1text2icon;

static void __nfc_vconf_changed_cb(keynode_t *key, void *data)
{
	ugdata_t *ug_data = (ugdata_t *)data;
	Elm_Object_Item *it, *next;
	int boolval;

	if (!ug_data || !key->keyname)
		return;

	LOGD("key->keyname [%s]", key->keyname);

	if (strncmp(key->keyname, VCONFKEY_NFC_STATE,
			strlen(VCONFKEY_NFC_STATE)) != 0) {
		return;
	}

	if (!vconf_get_bool(key->keyname, &boolval) && !boolval) {
		if (vconf_set_bool(VCONFKEY_NFC_PREDEFINED_ITEM_STATE,
			VCONFKEY_NFC_PREDEFINED_ITEM_OFF)) {
				LOGE("vconf_set_bool failed");
		}
		elm_check_state_set(ug_data->pd_on_off, EINA_FALSE);
	}

	if (!vconf_get_bool(VCONFKEY_NFC_PREDEFINED_ITEM_STATE, &boolval)) {
		it = elm_genlist_first_item_get(ug_data->pd_genlist);
		while (it != NULL) {
			next = elm_genlist_item_next_get(it);

			if (boolval)
				elm_object_item_disabled_set(it, EINA_FALSE);
			else
				elm_object_item_disabled_set(it, EINA_TRUE);

			it = next;
		}
	}
}

static void __title_pd_on_off_clicked_cb(void *data, Evas_Object *obj,
	void *event_info)
{
	ugdata_t *ug_data = (ugdata_t *)data;
	Elm_Object_Item *it, *next;
	int boolval;

	if (ug_data == NULL) {
		LOGE("data is null");
		return;
	}

	__change_predefined_item_onoff_setting(ug_data);

	if (!vconf_get_bool(VCONFKEY_NFC_PREDEFINED_ITEM_STATE, &boolval)) {
		it = elm_genlist_first_item_get(ug_data->pd_genlist);
		while (it != NULL) {
			next = elm_genlist_item_next_get(it);

			if (boolval)
				elm_object_item_disabled_set(it, EINA_FALSE);
			else
				elm_object_item_disabled_set(it, EINA_TRUE);

			it = next;
		}
	}
}

static char *__gl_text_get(void *data, Evas_Object *obj, const char *part)
{
	int index = (int) data;
	pd_data *item = NULL;
	char *text = NULL;

	//LOGD("index : %d", index);
	//LOGD("[Genlist] part : %s", part);

	if (!strcmp(part, "elm.text") || !strcmp(part, "elm.text.1")) {
		item = g_array_index(pd_item_l, pd_data *, index);
		if ((item != NULL) && (item->msgDes != NULL)) {
			text = strdup(item->msgDes);
			//LOGD("msgDes : %s", item->msgDes);
		}

	}

	return text;
}

static Evas_Object *__gl_content_get(void *data, Evas_Object *obj,
	const char *part)
{
	int index = (int) data;
	pd_data *item = NULL;
	Evas_Object *content = NULL;
	Evas_Object *radio_group;

	//LOGD("[Genlist] index : %d", index);
	//LOGD("[Genlist] part : %s", part);

	if (!strcmp(part, "elm.icon") || !strcmp(part, "elm.icon.1")) {
		item = g_array_index(pd_item_l, pd_data *, index);

		radio_group = evas_object_data_get(obj, "radio");
		content = elm_radio_add(obj);
		elm_radio_state_value_set(content, index);
		elm_radio_group_add(content, radio_group);

		if ((item != NULL) && (item->app_id != NULL)) {
			char *app_id;

			app_id = vconf_get_str(VCONFKEY_NFC_PREDEFINED_ITEM);
			if (app_id != NULL) {
				if (!strcmp(app_id, item->app_id)) {
					elm_radio_value_set(content, index);
				}
				free(app_id);
			}
		}

		evas_object_size_hint_weight_set(content, EVAS_HINT_EXPAND,
			EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(content, EVAS_HINT_FILL,
			EVAS_HINT_FILL);

	} else {
		item = g_array_index(pd_item_l, pd_data *, index);

		if ((item != NULL) && (item->iconPath != NULL)) {
			content = elm_image_add(obj);
			elm_image_file_set(content, item->iconPath, NULL);
			elm_image_resizable_set(content, EINA_TRUE, EINA_TRUE);
		}
	}

	return content;
}

static Eina_Bool __gl_state_get(void *data, Evas_Object *obj,
	const char *part)
{
	return EINA_FALSE;
}

static void __gl_sel(void *data, Evas_Object *obj, void *event_info)
{
	Elm_Object_Item *item = (Elm_Object_Item *) event_info;
	int index;
	pd_data *pd_item = NULL;


	if (!item)
		return;

	elm_genlist_item_selected_set(item, 0);

	index = (int) elm_object_item_data_get(item);

	pd_item = g_array_index(pd_item_l, pd_data *, index);

	if ((pd_item != NULL) && (pd_item->app_id != NULL)) {
		if (vconf_set_str(VCONFKEY_NFC_PREDEFINED_ITEM,
			pd_item->app_id)) {
			LOGE("vconf_set_str failed");
		}
	}
	elm_genlist_item_update(item);
}

static void __create_pd_data(void)
{
	pd_data *item = NULL;
	int index;

	if (_ug_nfc_setting_db_open_table(&dbtable_h)) {
		LOGE("_ug_nfc_setting_db_open_table is failed!!!!!");
		return;
	}

	/* get item count */
	_ug_nfc_setting_db_get_count(&pd_item_cnt);
	LOGD("pd_item_cnt [%d]", pd_item_cnt);

	/* create item list */
	pd_item_l = g_array_new(FALSE, FALSE,
		sizeof(pd_data *));

	for (index = 0; index < pd_item_cnt; index++) {
		item = (pd_data *) malloc(sizeof(pd_data));
		if (!item) {
			LOGE("failed to allocation");
			continue;
		}

		if (!_ug_nfc_setting_db_get_next_record(dbtable_h,
			&item->app_id,
			&item->pkgName,
			&item->iconPath,
			&item->msgDes)) {

			//LOGD("app_id:%s, pkgName:%s, iconPath:%s, msgDes:%s", item->app_id, item->pkgName, item->iconPath, item->msgDes);

			g_array_append_val(pd_item_l, item);
		} else {
			free(item);
		}
	}

	_ug_nfc_setting_db_close_table(dbtable_h);
}

static void __destroy_pd_data(void)
{
	if (pd_item_l != NULL) {
		pd_data *old_info = NULL;
		int index;

		for (index = 0; index < pd_item_cnt; index++) {
			old_info = g_array_index(pd_item_l, pd_data *, index);

			if (old_info != NULL) {
				free(old_info->app_id);
				free(old_info->pkgName);
				free(old_info->iconPath);
				free(old_info->msgDes);

				free(old_info);
			}
		}
		g_array_free(pd_item_l, FALSE);
	}
}

static void __genlist_free_cb(void *data, Evas *e, Evas_Object *obj,
	void *ei)
{
	Evas_Object *radio_group = evas_object_data_get(obj, "radio");

	if (radio_group)
		evas_object_del(radio_group);
}

Evas_Object *__create_predefined_item_list(void *data)
{
	ugdata_t *ug_data = (ugdata_t *)data;
	Evas_Object *genlist, *radio_group;
	Elm_Object_Item* separator;
	int index, boolval;

	/* create item data */
	__create_pd_data();

	/* make a list */
	genlist = elm_genlist_add(ug_data->base_naviframe);
	if (!genlist) {
		LOGE("genlist is null");
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

	itc_1text2icon.item_style = "1text.2icon.2";
	itc_1text2icon.func.text_get = __gl_text_get;
	itc_1text2icon.func.content_get = __gl_content_get;
	itc_1text2icon.func.state_get = __gl_state_get;
	itc_1text2icon.func.del = NULL;

	/* separator */
	separator = elm_genlist_item_append(genlist, &itc_sep, NULL, NULL,
		ELM_GENLIST_ITEM_NONE, NULL, NULL);
	elm_genlist_item_select_mode_set(separator,
		ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
	elm_object_item_access_unregister(separator);

	/* add radio list */
	if (!vconf_get_bool(VCONFKEY_NFC_PREDEFINED_ITEM_STATE, &boolval)) {
		for (index = 0; index < pd_item_cnt; index++) {
			Elm_Object_Item *it;

			it = elm_genlist_item_append(genlist,
				&itc_1text2icon, (void *)index,
				NULL, ELM_GENLIST_ITEM_NONE,
				__gl_sel, NULL);
			if (boolval)
				elm_object_item_disabled_set(it, EINA_FALSE);
			else
				elm_object_item_disabled_set(it, EINA_TRUE);

			/* SEPARATOR */
			separator = elm_genlist_item_append(genlist, &itc_sep,
				NULL, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
			elm_genlist_item_select_mode_set(separator,
				ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
			elm_object_item_access_unregister(separator);
		}
	}

	evas_object_show(genlist);

	vconf_notify_key_changed(VCONFKEY_NFC_STATE ,
			__nfc_vconf_changed_cb, ug_data);

	return genlist;
}

static Eina_Bool __destroy_predefined_item_list(void *data,
	Elm_Object_Item *it)
{
	ugdata_t *ug_data = (ugdata_t *)data;

	__destroy_pd_data();

	pd_item_cnt = 0;

	if (ug_data && ug_data->ns_genlist) {
		elm_genlist_realized_items_update(ug_data->ns_genlist);
	}

	vconf_ignore_key_changed(VCONFKEY_NFC_STATE,
		__nfc_vconf_changed_cb);

	return EINA_TRUE;
}

bool _ug_nfc_check_predefined_item_available(void)
{
	if (!_ug_nfc_setting_db_open_table(&dbtable_h)) {
		_ug_nfc_setting_db_close_table(dbtable_h);
		return true;
	} else {
		return false;
	}
}

void _ug_nfc_predefined_item_create(void *data)
{
	ugdata_t *ug_data = (ugdata_t *)data;
	Evas_Object *genlist;
	int boolval;

	if (!ug_data) {
		LOGE("invalid paramater");
		return;
	}

	/* create setting view */
	ug_data->pd_genlist = genlist = __create_predefined_item_list(ug_data);
	if (!genlist)
		return;

	/* Push navifreme */
	ug_data->base_navi_it = elm_naviframe_item_push(ug_data->base_naviframe,
		IDS_USE_NFC_IN_HOME, NULL, NULL, genlist, NULL);
	elm_naviframe_item_pop_cb_set(ug_data->base_navi_it,
		__destroy_predefined_item_list, ug_data);

	ug_data->pd_on_off = elm_check_add(ug_data->base_naviframe);
	elm_object_style_set(ug_data->pd_on_off, "naviframe/title_on&off");
	elm_object_item_part_content_set(ug_data->base_navi_it,
		"title_right_btn", ug_data->pd_on_off);
	evas_object_smart_callback_add(ug_data->pd_on_off, "changed",
		__title_pd_on_off_clicked_cb, ug_data);

	if (!vconf_get_bool(VCONFKEY_NFC_PREDEFINED_ITEM_STATE, &boolval) &&
		boolval) {
		elm_check_state_set(ug_data->pd_on_off, EINA_TRUE);
	} else {
		elm_check_state_set(ug_data->pd_on_off, EINA_FALSE);
	}
}


