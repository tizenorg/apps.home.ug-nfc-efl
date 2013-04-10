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


#define MAX_PREDEFINED_ITEM_COUNT	100

typedef struct _predefined_item_data_t
{
	char *app_id;
	char *pkgName;
	char *iconPath;
	char *msgDes;
}predefined_item_data;


int dbtable_h = 0;
int predefined_item_count = 0;
GArray *predefined_item_list;
ugdata_t *ud = NULL;

static Elm_Object_Item *on_off_item = NULL;

static Elm_Genlist_Item_Class itc_sep;
static Elm_Genlist_Item_Class itc_2text;
static Elm_Genlist_Item_Class itc_radio;
Evas_Object *radio_main = NULL;
static int state_index = 0; //selected radio index


static void _check_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	LOGD("BEGIN >>>>");

	_change_nfc_predefined_item_setting(ud);
	if (on_off_item)
		elm_genlist_item_update(on_off_item);

	LOGD("END <<<<");
}

static char *_gl_text_get(void *data, Evas_Object *obj, const char *part)
{
	int index = (int) data;
	char *text = NULL;

	LOGD("[Genlist] index : %d", index);
	LOGD("[Genlist] part : %s", part);

	if (!strcmp(part, "elm.text") || !strcmp(part, "elm.text.1"))
	{
		if (index == -1)
		{
			text = strdup(IDS_PREDEFINED_ITEM);
		}
		else
		{
			predefined_item_data *item = NULL;

			item = g_array_index(predefined_item_list, predefined_item_data *, index);
			if ((item != NULL) && (item->msgDes != NULL))
			{
				text = strdup(item->msgDes);
				LOGD("pkgName : %s", item->msgDes);
			}
		}
	}
	else if (!strcmp(part, "elm.text.2"))
	{
		if (index == -1)
		{
			char *app_id = NULL;

			app_id = vconf_get_str(VCONFKEY_NFC_PREDEFINED_ITEM);
			if (app_id != NULL)
			{
				LOGD("app_id [%s]", app_id);
				_ug_nfc_setting_db_get_pkgName(app_id, &text);

				free(app_id);
			}
		}
	}

	return text;
}

static Evas_Object *_gl_content_get(void *data, Evas_Object *obj, const char *part)
{
	int index = (int) data;
	Evas_Object *content = NULL;

	LOGD("[Genlist] index : %d", index);
	LOGD("[Genlist] part : %s", part);

	if (!strcmp(part, "elm.icon") || !strcmp(part, "elm.icon.1"))
	{
		if (index == -1)
		{
			int result;
			int on;

			content = elm_check_add(obj);
			evas_object_propagate_events_set(content, EINA_FALSE);
			evas_object_smart_callback_add(content, "changed", _check_changed_cb, NULL);

			if (((result = vconf_get_bool(VCONFKEY_NFC_PREDEFINED_ITEM_STATE, &on)) == 0) && (on != 0))
			{
				LOGD("vconf_get_bool true");
				elm_check_state_set(content, EINA_TRUE);
			}
			else
			{
				LOGD("vconf_get_bool false");
				elm_check_state_set(content, EINA_FALSE);
			}
			elm_object_style_set(content, "on&off");
		}
		else
		{
			predefined_item_data *item = NULL;

			item = g_array_index(predefined_item_list, predefined_item_data *, index);

			content = elm_radio_add(obj);
			elm_radio_state_value_set(content, index);
			elm_radio_group_add(content, radio_main);

			if ((item != NULL) && (item->app_id != NULL))
			{
				char *app_id = NULL;

				app_id = vconf_get_str(VCONFKEY_NFC_PREDEFINED_ITEM);
				if (app_id != NULL)
				{
					if (!strcmp(app_id, item->app_id))
					{
						state_index = index;
						elm_radio_value_set(content, state_index);
					}
					free(app_id);
				}
			}

			evas_object_size_hint_weight_set(content, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
			evas_object_size_hint_align_set(content, EVAS_HINT_FILL, EVAS_HINT_FILL);
		}
	}
	else
	{
		if (index >= 0)
		{
			predefined_item_data *item = NULL;

			item = g_array_index(predefined_item_list, predefined_item_data *, index);

			if ((item != NULL) && (item->iconPath != NULL))
			{
				LOGD("iconPath [%s]", item->iconPath);

				content = elm_image_add(obj);
				elm_image_file_set(content, item->iconPath, NULL);
				elm_image_resizable_set(content, EINA_TRUE, EINA_TRUE);
			}
		}
	}

	return content;
}

static Eina_Bool _gl_state_get(void *data, Evas_Object *obj, const char *part)
{
	return EINA_FALSE;
}

static void _gl_sel(void *data, Evas_Object *obj, void *event_info)
{
	Elm_Object_Item *ui_item = (Elm_Object_Item *) event_info;
	int result = -1;
	int index = 0;

	elm_genlist_item_selected_set(ui_item, 0);

	if (ui_item)
	{
		index = (int) elm_object_item_data_get(ui_item);

		if (index == -1)
		{
			_change_nfc_predefined_item_setting(ud);
			elm_genlist_item_update(ui_item);
		}
		else
		{
			predefined_item_data *item = NULL;

			item = g_array_index(predefined_item_list, predefined_item_data *, index);

			state_index = index;

			if ((item != NULL) && (item->app_id != NULL))
			{
				LOGD("Selected app_id[%s]", item->app_id);
				result = vconf_set_str(VCONFKEY_NFC_PREDEFINED_ITEM, item->app_id);
				if (!result )
				{
					LOGD("vconf_set_str success");
				}
				else
				{
					LOGD("vconf_set_str failed");
				}
			}
			elm_genlist_item_update(ui_item);

			if (on_off_item)
				elm_genlist_item_update(on_off_item);
		}
	}
}

#if 0
static void _gl_realized(void *data, Evas_Object *obj, void *event_info)
{
	LOGD("BEGIN >>>>");

	Elm_Object_Item *item = (Elm_Object_Item *) event_info;
	int index = 0;

	if (item)
	{
		index = (int) elm_object_item_data_get(item);

		if (index == -1)
		{
			elm_object_item_signal_emit(event_info, "elm,state,top", "");
		}
		else if (index == predefined_item_count-1)
		{
			elm_object_item_signal_emit(event_info, "elm,state,bottom", "");
		}
		else
		{
			elm_object_item_signal_emit(event_info, "elm,state,center", "");
		}
	}

	LOGD("END <<<<");
}
#endif

bool _ug_nfc_check_predefined_item_available(void)
{
	LOGD("BEGIN >>>>");

	if (!_ug_nfc_setting_db_open_table(&dbtable_h))
	{
		_ug_nfc_setting_db_close_table(dbtable_h);
		return true;
	}
	else
	{
		return false;
	}

	LOGD("END <<<<");
}

static void __create_predefined_item_data(void)
{
	LOGD("BEGIN >>>>");

	predefined_item_data *item = NULL;
	int index;

	if (_ug_nfc_setting_db_open_table(&dbtable_h))
	{
		LOGD("_ug_nfc_setting_db_open_table is failed!!!!!");
		return;
	}

	/* get item count */
	_ug_nfc_setting_db_get_count(&predefined_item_count);
	LOGD("predefined_item_count [%d]", predefined_item_count);

	/* create item list */
	predefined_item_list = g_array_new(FALSE, FALSE, sizeof(predefined_item_data *));

	for (index = 0; index < predefined_item_count; index++)
	{
		item = (predefined_item_data *) malloc(sizeof(predefined_item_data));
		if (!item)
		{
			LOGD("failed to allocation");
			continue;
		}

		if (!_ug_nfc_setting_db_get_next_record(dbtable_h, &item->app_id, &item->pkgName, &item->iconPath, &item->msgDes))
		{
			LOGD("app_id:%s, pkgName:%s, iconPath:%s, msgDes:%s", item->app_id, item->pkgName, item->iconPath, item->msgDes);

			g_array_append_val(predefined_item_list, item);
		}
		else
		{
			free(item);
		}
	}

	_ug_nfc_setting_db_close_table(dbtable_h);

	LOGD("END <<<<");
}

static void __destroy_predefined_item_data(void)
{
	LOGD("BEGIN >>>>");

	if (predefined_item_list != NULL)
	{
		predefined_item_data *old_info = NULL;
		int index;

		for (index = 0; index < predefined_item_count; index++)
		{
			old_info = g_array_index(predefined_item_list, predefined_item_data *, index);

			if (old_info != NULL)
			{
				free(old_info->app_id);
				free(old_info->pkgName);
				free(old_info->iconPath);
				free(old_info->msgDes);

				free(old_info);
			}
		}
		g_array_free(predefined_item_list, FALSE);
	}

	LOGD("END <<<<");
}

Evas_Object *_ug_nfc_create_predefined_item_list(void *data)
{
	LOGD("BEGIN >>>>");

	ugdata_t *ug_data = (ugdata_t *)data;
	Evas_Object *genlist = NULL;
	Elm_Object_Item* separator = NULL;
	int index = 0;


	/* make a list */
	genlist = elm_genlist_add(ug_data->base_naviframe);
	if (genlist == NULL)
	{
		LOGD("genlist is null");
		return NULL;
	}
#if 0
	evas_object_smart_callback_add(genlist, "realized", _gl_realized, NULL);
#endif
	itc_sep.item_style = "dialogue/separator";
	itc_sep.func.text_get = NULL;
	itc_sep.func.content_get = NULL;
	itc_sep.func.state_get = NULL;
	itc_sep.func.del = NULL;

	itc_2text.item_style = "dialogue/2text.1icon.10";
	itc_2text.func.text_get = _gl_text_get;
	itc_2text.func.content_get = _gl_content_get;
	itc_2text.func.state_get = _gl_state_get;
	itc_2text.func.del = NULL;

	itc_radio.item_style = "1text.2icon.2";
	itc_radio.func.text_get = _gl_text_get;
	itc_radio.func.content_get = _gl_content_get;
	itc_radio.func.state_get = _gl_state_get;
	itc_radio.func.del = NULL;


	if (!radio_main)
	{
		radio_main = elm_radio_add(genlist);
		elm_radio_state_value_set(radio_main, 0);
		elm_radio_value_set(radio_main, 0);
	}

	/* create item data */
	__create_predefined_item_data();

	/* separator */
	separator = elm_genlist_item_append(genlist, &itc_sep, NULL, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	elm_genlist_item_select_mode_set(separator, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
	elm_object_item_access_unregister(separator);

	/* add check button */
	on_off_item = elm_genlist_item_append(genlist, &itc_2text, (void *)-1, NULL, ELM_GENLIST_ITEM_NONE, _gl_sel, NULL);

	/* add radio list */
	for (index = 0; index < predefined_item_count; index++)
	{
		elm_genlist_item_append(genlist, &itc_radio, (void *)index, NULL, ELM_GENLIST_ITEM_NONE, _gl_sel, NULL);
	}

	evas_object_show(genlist);

	ud = ug_data;

	LOGD("END <<<<");

	return genlist;
}

void _ug_nfc_destroy_predefined_item_list(void *data, Evas_Object *obj, void *event_info)
{
	LOGD("BEGIN >>>>");

	ugdata_t *ug_data = data;

	elm_naviframe_item_pop(ug_data->base_naviframe);

	__destroy_predefined_item_data();

	predefined_item_count = 0;

	if (radio_main)
	{
		evas_object_del(radio_main);
		radio_main = NULL;
	}

	LOGD("END <<<<");
}



