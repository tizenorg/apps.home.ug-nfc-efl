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

#include "ug-nfc-setting-main.h"

#include <stdio.h>
#include <Elementary.h>
#include <Ecore.h>
#include <bundle.h>
#include <Ecore_X.h>
#include <vconf.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <appcore-efl.h>


#define _EDJ(obj) elm_layout_edje_get(obj)
#ifndef UG_MODULE_API
#define UG_MODULE_API __attribute__ ((visibility("default")))
#endif

#define NET_NFC_VCONF_KEY_PROGRESS "memory/nfc/progress"

#define NFC_MANAGER_ENABLE 1
#define NFC_MANAGER_DISABLE 0

static Elm_Genlist_Item_Class itc_seperator;
static Elm_Genlist_Item_Class itc_onoff;
static Elm_Object_Item *on_off_item = NULL;

static void _change_nfc_setting(void)
{
	int status;
	int result;

	LOGD("[%s(): %d] BEGIN >>>>", __FUNCTION__, __LINE__);

	if ((result = vconf_get_bool(VCONFKEY_NFC_STATE, &status)) == 0)
	{
		LOGD("[%s(): %d] vconf_get_bool status [%d]\n", __FUNCTION__, __LINE__, status);

		nfc_manager_set_activation(!status, NULL, NULL);
	}
	else
	{
		LOGD("[%s(): %d] vconf_get_bool failed\n", __FUNCTION__, __LINE__);
	}

	LOGD("[%s(): %d] END <<<<", __FUNCTION__, __LINE__);
}

static void _vconf_key_cb(keynode_t *node, void *user_data)
{
	LOGD("[%s(): %d] BEGIN >>>>", __FUNCTION__, __LINE__);

	if (node == NULL || user_data == NULL)
	{
		LOGE("[%s(): %d] abnormal parameter", __FUNCTION__, __LINE__);
		return;
	}

	if (on_off_item != NULL)
		elm_genlist_item_update(on_off_item);

	LOGD("[%s(): %d] END <<<<", __FUNCTION__, __LINE__);
}

static Evas_Object *_create_bg(Evas_Object *win)
{
	Evas_Object *bg = elm_bg_add(win);

	evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(bg, EVAS_HINT_FILL, EVAS_HINT_FILL);

	elm_object_style_set(bg, "transparent");

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

// get the state of item
static Eina_Bool _gl_state_get(void *data, Evas_Object *obj, const char *part)
{
	Eina_Bool result = EINA_FALSE;
	int enable = 0;

	LOGD("[%s(): %d] BEGIN >>>>", __FUNCTION__, __LINE__);

	if (!vconf_get_bool(VCONFKEY_NFC_STATE, &enable))
	{
		if (enable != 0)
		{
			LOGD("[%s(): %d] vconf_get_bool true", __FUNCTION__, __LINE__);
			result = EINA_TRUE;
		}
		else
		{
			LOGD("[%s(): %d] vconf_get_bool false", __FUNCTION__, __LINE__);
		}
	}
	else
	{
		LOGD("[%s(): %d] vconf_get_bool error [%d]", __FUNCTION__, __LINE__, result);
	}

	LOGD("[%s(): %d] END <<<<", __FUNCTION__, __LINE__);

	return result;
}

// callback for 'deletion'
static void _gl_del(void *data, Evas_Object *obj)
{
	LOGD("[%s(): %d] BEGIN >>>>", __FUNCTION__, __LINE__);
	LOGD("[%s(): %d] END <<<<", __FUNCTION__, __LINE__);

	return;
}

static void _gl_sel_activation(void *data, Evas_Object *obj, void *event_info)
{
	Elm_Object_Item *item = (Elm_Object_Item *)event_info;

	LOGD("[%s(): %d] BEGIN >>>>", __FUNCTION__, __LINE__);

	if (item == NULL)
	{
		LOGD("[%s(): %d] item is null\n", __FUNCTION__, __LINE__);
		return;
	}

	ugdata_t *ug_data = (ugdata_t *)data;
	if (ug_data == NULL)
	{
		LOGD("[%s(): %d] ug_data is null\n", __FUNCTION__, __LINE__);
		goto ERROR;
	}

	elm_genlist_item_selected_set(item, 0);

	_change_nfc_setting();

ERROR :
	elm_genlist_item_update(item);

	LOGD("[%s(): %d] END <<<<", __FUNCTION__, __LINE__);
}

static void _nfc_on_off_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	ugdata_t *ug_data = (ugdata_t *)data;
	int state = 0;
	int result = EINA_FALSE;

	LOGD("[%s(): %d] BEGIN >>>>", __FUNCTION__, __LINE__);

	_change_nfc_setting();

	if (((result = vconf_get_bool(VCONFKEY_NFC_STATE, &state)) == 0) && (state != 0))
	{
		LOGD("[%s(): %d] vconf_get_bool true", __FUNCTION__, __LINE__);
		elm_check_state_set(ug_data->check, EINA_TRUE);
	}
	else
	{
		LOGD("[%s(): %d] vconf_get_bool false or failed", __FUNCTION__, __LINE__);
		elm_check_state_set(ug_data->check, EINA_FALSE);
	}

	LOGD("[%s(): %d] END <<<<", __FUNCTION__, __LINE__);
}

static Evas_Object *_gl_content_get(void *data, Evas_Object *obj, const char *part)
{
	LOGD("[%s(): %d] BEGIN >>>>", __FUNCTION__, __LINE__);
	Evas_Object *content = NULL;
	int on;
	int result = EINA_FALSE;

	ugdata_t *ug_data = (ugdata_t *)data;

	if (ug_data == NULL)
	{
		LOGD("[%s(): %d] ug data is null", __FUNCTION__, __LINE__);
		return NULL;
	}

	ug_data->check = content = elm_check_add(obj);
	evas_object_propagate_events_set(ug_data->check, EINA_FALSE);
	evas_object_smart_callback_add(ug_data->check, "changed", _nfc_on_off_changed_cb, ug_data);

	if (((result = vconf_get_bool(VCONFKEY_NFC_STATE, &on)) == 0) && (on != 0))
	{
		LOGD("[%s(): %d] vconf_get_bool true", __FUNCTION__, __LINE__);
		elm_check_state_set(ug_data->check, EINA_TRUE);
	}
	else
	{
		LOGD("[%s(): %d] vconf_get_bool false or failed", __FUNCTION__, __LINE__);
		elm_check_state_set(ug_data->check, EINA_FALSE);
	}

	elm_object_style_set(content, "on&off");

	LOGD("[%s(): %d] END <<<<", __FUNCTION__, __LINE__);

	return content;
}

static char *_gl_text_get_onoff(void *data, Evas_Object *obj, const char *part)
{
	LOGD("[%s(): %d] BEGIN >>>>", __FUNCTION__, __LINE__);
	char *text = NULL;

	text = strdup(IDS_ACTIVATION);

	LOGD("[%s(): %d] END <<<<", __FUNCTION__, __LINE__);
	return text;
}

static Evas_Object *_ug_nfc_create_setting_layout(void* data)
{
	ugdata_t* ug_data = (ugdata_t*)data;
	Evas_Object* layout = NULL;
	Evas_Object *genlist = NULL;

	layout = elm_layout_add(ug_data->base_naviframe);
	elm_layout_theme_set(layout, "layout", "application", "noindicator");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

	elm_object_style_set(ug_data->bg, "group_list");

	evas_object_show(layout);

	genlist = elm_genlist_add(layout);
	if (genlist == NULL)
	{
		LOGD("[%s(): %d] genlist is null", __FUNCTION__, __LINE__);
		return NULL;
	}
	elm_genlist_bounce_set(genlist, EINA_FALSE, EINA_FALSE);

	itc_seperator.item_style = "grouptitle.dialogue.seperator";
	itc_seperator.func.text_get = NULL;
	itc_seperator.func.content_get = NULL;
	itc_seperator.func.state_get = NULL;
	itc_seperator.func.del = NULL;

	itc_onoff.item_style = "dialogue/1text.1icon";
	itc_onoff.func.text_get = _gl_text_get_onoff;
	itc_onoff.func.content_get = _gl_content_get;
	itc_onoff.func.state_get = _gl_state_get;
	itc_onoff.func.del = NULL;

	// seperator
	Elm_Object_Item* dialoguegroup = elm_genlist_item_append(genlist, &itc_seperator, NULL, NULL, ELM_GENLIST_ITEM_GROUP, NULL, NULL);
	elm_genlist_item_select_mode_set(dialoguegroup, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

	//activation
	on_off_item = elm_genlist_item_append(genlist, &itc_onoff, (void *)data, NULL, ELM_GENLIST_ITEM_NONE, _gl_sel_activation, (void *)data);

	vconf_notify_key_changed(VCONFKEY_NFC_STATE, _vconf_key_cb, data);
	vconf_notify_key_changed(NET_NFC_VCONF_KEY_PROGRESS, _vconf_key_cb, data);

	evas_object_show(genlist);
	elm_object_part_content_set(layout, "elm.swallow.content", genlist);

	return layout;
}

static void _back_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	ug_destroy_me(data);
}

static void *__ug_nfc_setting_create(struct ui_gadget *ug, enum ug_mode mode, bundle *bd, void *user_data)
{
	ugdata_t* ug_data = (ugdata_t*)user_data;
	Evas_Object* parent = NULL;
	Evas_Object* nfc_setting_layout = NULL;
	Evas_Object* l_button = NULL;

	parent = ug_get_parent_layout(ug);
	if (!parent)
		return NULL;

	/* set text domain */
	bindtextdomain(NFCUG_TEXT_DOMAIN, NFCUG_LOCALEDIR);

	ug_data->ug_win_main = parent;
	ug_data->nfc_setting_ug = ug;

	ug_data->base_layout = _create_main_layout(ug_data->ug_win_main);
	ug_data->bg = _create_bg(ug_data->base_layout);
	elm_object_part_content_set(ug_data->base_layout, "elm.swallow.bg", ug_data->bg);

	ug_data->base_naviframe = elm_naviframe_add(ug_data->base_layout);
	elm_object_part_content_set(ug_data->base_layout, "elm.swallow.content", ug_data->base_naviframe);

	evas_object_show(ug_data->base_layout);
	evas_object_show(ug_data->base_naviframe);

	nfc_setting_layout = _ug_nfc_create_setting_layout(ug_data);
	if (nfc_setting_layout == NULL)
		return NULL;

	l_button = elm_button_add(ug_data->base_naviframe);
	evas_object_smart_callback_add(l_button, "clicked", _back_clicked_cb, ug_data->nfc_setting_ug);
	ug_data->base_navi_it = elm_naviframe_item_push(ug_data->base_naviframe, IDS_NFC, l_button, NULL, nfc_setting_layout, "1line");
	elm_object_style_set(l_button, "naviframe/back_btn/default");

	return ug_data->base_layout;
}

static void __ug_nfc_setting_destroy(struct ui_gadget *ug, bundle *bd, void *user_data)
{
	ugdata_t *ug_data = (ugdata_t *)user_data;

	if (ug_data == NULL)
		return;

	vconf_ignore_key_changed(VCONFKEY_NFC_STATE, _vconf_key_cb);
	vconf_ignore_key_changed(NET_NFC_VCONF_KEY_PROGRESS, _vconf_key_cb);

	evas_object_del(ug_get_layout(ug));
}

static void __ug_nfc_setting_start(struct ui_gadget *ug, bundle *bd, void *user_data)
{
}

static void __ug_nfc_setting_pause(struct ui_gadget *ug, bundle *bd, void *user_data)
{
}

static void __ug_nfc_setting_resume(struct ui_gadget *ug, bundle *bd, void *user_data)
{
}

static void __ug_nfc_setting_message(struct ui_gadget *ug, bundle *msg, bundle *bd, void *user_data)
{
}

static void __ug_nfc_setting_event(struct ui_gadget *ug, enum ug_event event, bundle *bd, void *user_data)
{
}

UG_MODULE_API int UG_MODULE_INIT(struct ug_module_ops *ops)
{
	LOGD("[%s(): %d] UG_MODULE_INIT!!\n", __FUNCTION__, __LINE__);

	ugdata_t *ug_data; // User defined private data
	if (!ops)
		return -1;

	//app_data = calloc(1, sizeof(struct ugdata));
	ug_data = (ugdata_t *)malloc(sizeof(ugdata_t));
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
	ugdata_t* ug_data;

	if (!ops)
		return;

	ug_data = ops->priv;

	if (ug_data)
		free(ug_data);

	ops->priv = NULL;
}
