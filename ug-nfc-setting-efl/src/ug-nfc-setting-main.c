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
#include "ug-nfc-setting-popup.h"

#include <stdio.h>
#include <Elementary.h>
#include <Ecore.h>
#include <bundle.h>
#include <Ecore_X.h>
#include <vconf.h>
#include <sys/types.h>
#include <sys/wait.h>

#define _EDJ(obj) elm_layout_edje_get(obj)
#ifndef UG_MODULE_API
#define UG_MODULE_API __attribute__ ((visibility("default")))
#endif

#define NFC_MANAGER_ENABLE 1
#define NFC_MANAGER_DISABLE 0

#ifdef _SBEAM_SUPPORT_
#define S_BEAM_DESCRIPTION_MSG_WRAP_WIDTH	640
#endif

#define MAX_PREDEFINED_ITEM_COUNT	100

static Elm_Genlist_Item_Class itc_check;
static Elm_Genlist_Item_Class itc_helptext;
static Elm_Genlist_Item_Class itc_2text;
static Elm_Genlist_Item_Class itc_radio;

static Elm_Object_Item *on_off_item = NULL;
#ifdef _SBEAM_SUPPORT_
static Elm_Object_Item *sbeam_item = NULL;
static Elm_Object_Item *sbeam_item_help = NULL;
#endif
static Elm_Object_Item *predefined_item = NULL;
static Elm_Object_Item *predefined_item_help = NULL;

Evas_Object *radio_main = NULL;
char **predefined_item_list = NULL;
int predefined_item_count = 0;
static int state_index = 0; //selected radio index

static bool pending_status = FALSE;


static void _mdm_restricted_popup_response_cb(void *data, Evas_Object *obj, void *event_info)
{
	ugdata_t *ug_data = (ugdata_t *)data;
	if (!ug_data)
		return;

	LOGD("[%s(): %d] BEGIN >>>>", __FUNCTION__, __LINE__);
	LOGD("[%s(): %d] END >>>>", __FUNCTION__, __LINE__);
}

static void _mdm_restricted_popup(void *data)
{
	LOGD("[%s(): %d] BEGIN >>>>", __FUNCTION__, __LINE__);

	char popup_str[POPUP_TEXT_SIZE] = { 0, };
	ugdata_t *ug_data = (ugdata_t *)data;
	if (!ug_data)
		return;

	snprintf(popup_str, POPUP_TEXT_SIZE, IDS_SECURITY_POLICY_RESTRICTS_USE_OF_PS, IDS_NFC_NFC);
	ug_nfc_setting_create_popup(ug_data, ug_data->base_layout, popup_str, NULL, 0, NULL, 0, NULL, 0, true, true, _mdm_restricted_popup_response_cb);

	LOGD("[%s(): %d] END >>>>", __FUNCTION__, __LINE__);
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
		ug_destroy_me(ug_data->nfc_setting_ug);
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

	ug_nfc_setting_create_popup(ug_data, ug_data->base_layout, popup_str, IDS_CLOSE, UG_NFC_POPUP_RESP_CLOSE, NULL, 0, NULL, 0, false, false, _app_error_popup_response_cb);

	LOGD("[%s(): %d] END <<<<", __FUNCTION__, __LINE__);
}

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
			color = 0x007C7C7C;
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
			style = "";
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

static bool _get_pending_status(void)
{
	return pending_status;
}

static void _set_pending_status(bool status)
{
	pending_status = status;
}

static void _change_nfc_onoff_setting(void *data)
{
	int status;
	int result;

	LOGD("[%s(): %d] BEGIN >>>>", __FUNCTION__, __LINE__);

	ugdata_t *ug_data = (ugdata_t *)data;
	if (!ug_data)
		return;

	if ((result = vconf_get_bool(VCONFKEY_NFC_STATE, &status)) == 0)
	{
		LOGD("[%s(): %d] vconf_get_bool status [%d]\n", __FUNCTION__, __LINE__, status);

		result = nfc_manager_set_activation(!status, NULL, NULL);
		if (result == NFC_ERROR_SECURITY_RESTRICTED)
		{
			_mdm_restricted_popup(ug_data);
			return;
		}
		_set_pending_status(TRUE);
	}
	else
	{
		LOGD("[%s(): %d] vconf_get_bool failed\n", __FUNCTION__, __LINE__);
	}

	LOGD("[%s(): %d] END <<<<", __FUNCTION__, __LINE__);
}

#ifdef _SBEAM_SUPPORT_
static void _change_nfc_sbeam_setting(void)
{
	int status = 0;
	int result = -1;

	LOGD("[%s(): %d] BEGIN >>>>", __FUNCTION__, __LINE__);

	if ((result = vconf_get_bool(VCONFKEY_NFC_SBEAM, &status)) == 0)
	{
		LOGD("[%s(): %d] vconf_get_bool status [%d]\n", __FUNCTION__, __LINE__, status);

		if (status)
		{
			result = vconf_set_bool(VCONFKEY_NFC_SBEAM, FALSE);
			if (!result )
			{
				LOGD("[%s(): %d] vconf_set_bool success\n", __FUNCTION__, __LINE__);
			}
			else
			{
				LOGD("[%s(): %d] vconf_set_bool failed\n", __FUNCTION__, __LINE__);
			}
		}
		else
		{
			result = vconf_set_bool(VCONFKEY_NFC_SBEAM, TRUE);
			if (!result )
			{
				LOGD("[%s(): %d] vconf_set_bool success\n", __FUNCTION__, __LINE__);
			}
			else
			{
				LOGD("[%s(): %d] vconf_set_bool failed\n", __FUNCTION__, __LINE__);
			}
		}
	}
	else
	{
		LOGD("[%s(): %d] vconf_get_bool failed\n", __FUNCTION__, __LINE__);
	}

	LOGD("[%s(): %d] END <<<<", __FUNCTION__, __LINE__);
}
#endif

void _nfc_activation_changed_cb(bool activated , void *user_data)
{
	LOGD("[%s(): %d] BEGIN >>>>", __FUNCTION__, __LINE__);

#ifdef _SBEAM_SUPPORT_
	int sbeam_state = 0;
	static int sbeam_off_by_nfcOnOff = EINA_FALSE;
#endif
	int result;

	LOGD("nfc mode %s \n", activated ? "ON" : "OFF");

#ifdef _SBEAM_SUPPORT_
	if ((result = vconf_get_bool(VCONFKEY_NFC_SBEAM, &sbeam_state)) == 0)
	{
		LOGD("[%s(): %d] vconf_get_bool status [%d]\n", __FUNCTION__, __LINE__, sbeam_state);
	}
	else
	{
		LOGD("[%s(): %d] vconf_get_bool failed\n", __FUNCTION__, __LINE__);
		return;
	}

	LOGD("S BEAM state %d, NFC was off by nfcOnOff %s \n",
			sbeam_state, sbeam_off_by_nfcOnOff == EINA_TRUE ? "Yes" : "No");
#endif

	/* nfc setting ui updated */
	_set_pending_status(FALSE);

	if (on_off_item != NULL)
		elm_genlist_item_update(on_off_item);

	if (activated == true)
	{
		/* predefined item setting enabled */
		elm_object_item_disabled_set(predefined_item, EINA_FALSE);
		elm_object_item_disabled_set(predefined_item_help, EINA_FALSE);

#ifdef _SBEAM_SUPPORT_
		/* sbeam setting enabled */
		elm_object_item_disabled_set(sbeam_item, EINA_FALSE);
		elm_object_item_disabled_set(sbeam_item_help, EINA_FALSE);

		if (sbeam_off_by_nfcOnOff != EINA_TRUE)
			return;

		LOGD("Turning S Beam on \n");

		_change_nfc_sbeam_setting();

		if (sbeam_item != NULL)
			elm_genlist_item_update(sbeam_item);

		/* unset internal flag */
		sbeam_off_by_nfcOnOff = EINA_FALSE;
#endif
	}
	else
	{
		/* predefined item setting disabled */
		elm_object_item_disabled_set(predefined_item, EINA_TRUE);
		elm_object_item_disabled_set(predefined_item_help, EINA_TRUE);

#ifdef _SBEAM_SUPPORT_
		/* sbeam setting disabled */
		elm_object_item_disabled_set(sbeam_item, EINA_TRUE);
		elm_object_item_disabled_set(sbeam_item_help, EINA_TRUE);

		if (sbeam_state == VCONFKEY_NFC_SBEAM_OFF)
			return;

		LOGD("Turning S Beam off \n");

		_change_nfc_sbeam_setting();

		if (sbeam_item != NULL)
			elm_genlist_item_update(sbeam_item);

		/* set internal flag */
		sbeam_off_by_nfcOnOff = EINA_TRUE;
#endif
	}

	LOGD("[%s(): %d] END <<<<", __FUNCTION__, __LINE__);
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

static Evas_Object *_create_main_layout(Evas_Object *parent)
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

static char *_gl_radio_text_get(void *data, Evas_Object *obj, const char *part)
{
	int index = (int) data;

	if (!strcmp(part, "elm.text"))
	{
		return strdup(predefined_item_list[index]);
	}

	return NULL;
}

static Evas_Object *_gl_radio_content_get(void *data, Evas_Object *obj, const char *part)
{
	int index = (int) data;
	Evas_Object *radio = NULL;

	if (!strcmp(part, "elm.icon") || !strcmp(part, "elm.swallow.icon"))
	{
		radio = elm_radio_add(obj);
		elm_radio_state_value_set(radio, index);
		elm_radio_group_add(radio, radio_main);

		if (index == state_index)
			elm_radio_value_set(radio, state_index);

		evas_object_size_hint_weight_set(radio, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(radio, EVAS_HINT_FILL, EVAS_HINT_FILL);

		return radio;
	}

	return NULL;
}

static Eina_Bool _gl_radio_state_get(void *data, Evas_Object *obj, const char *part)
{
	return EINA_FALSE;
}

static void _gl_radio_sel(void *data, Evas_Object *obj, void *event_info)
{
	Elm_Object_Item *item = (Elm_Object_Item *) event_info;
	int result = -1;
	int index = 0;

	if (item)
	{
		index = (int) elm_object_item_data_get(item);

		LOGD("[Genlist] Selected Text : %s\n", predefined_item_list[index]);

		state_index = index;
		result = vconf_set_str(VCONFKEY_NFC_PREDEFINED_ITEM, predefined_item_list[index]);
		if (!result )
		{
			LOGD("[%s(): %d] vconf_set_str success\n", __FUNCTION__, __LINE__);
		}
		else
		{
			LOGD("[%s(): %d] vconf_set_str failed\n", __FUNCTION__, __LINE__);
		}

		elm_genlist_item_update(item);
	}
}

static Evas_Object *_ug_nfc_create_predefined_item_layout(void *data)
{
	LOGD("[%s(): %d] BEGIN >>>>", __FUNCTION__, __LINE__);

	ugdata_t *ug_data = (ugdata_t *)data;
	Evas_Object *layout = NULL;
	Evas_Object *genlist = NULL;
	FILE *fp = NULL;
	int index = 0;
	char buf[1024] = { 0 , };

	layout = elm_layout_add(ug_data->base_naviframe);
	if (layout == NULL)
	{
		LOGD("[%s(): %d] layout is null", __FUNCTION__, __LINE__);
		return NULL;
	}
	elm_layout_theme_set(layout, "layout", "application", "noindicator");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(layout);

	/* make a list */
	genlist = elm_genlist_add(layout);
	if (genlist == NULL)
	{
		LOGD("[%s(): %d] genlist is null", __FUNCTION__, __LINE__);
		return NULL;
	}

	if (!radio_main)
	{
		radio_main = elm_radio_add(genlist);
		elm_radio_state_value_set(radio_main, 0);
		elm_radio_value_set(radio_main, 0);
	}

	itc_radio.item_style = "1text.1icon.2";
	itc_radio.func.text_get = _gl_radio_text_get;
	itc_radio.func.content_get = _gl_radio_content_get;
	itc_radio.func.state_get = _gl_radio_state_get;
	itc_radio.func.del = NULL;

	/* file open to make a list */
	if ((fp = fopen(PREDEFINED_ITEM_FILE_PATH, "r")) == NULL)
	{
		LOGD("file open error");
		return NULL;
	}

	if (fscanf(fp, "%d", &predefined_item_count))
	{
		LOGD("count [%d]", predefined_item_count);
	}

	if ((predefined_item_count < 0) || (predefined_item_count > MAX_PREDEFINED_ITEM_COUNT))
	{
		LOGD("The value of item_count is wrong");
		fclose(fp);
		return NULL;
	}

	predefined_item_count++; /* for none menu */

	predefined_item_list = malloc(sizeof(char *) * predefined_item_count);

	for (index = 0; index < predefined_item_count; index++) {
		if (index == 0)
		{
			LOGD("first item");

			predefined_item_list[index] = strdup(IDS_NONE);
			elm_genlist_item_append(genlist, &itc_radio, (void *)index, NULL, ELM_GENLIST_ITEM_NONE, _gl_radio_sel, NULL);
		}
		else
		{
			if (fscanf(fp, "%s", buf))
			{
				LOGD("buf [%s]", buf);
				predefined_item_list[index] = strdup(buf);

				elm_genlist_item_append(genlist, &itc_radio, (void *)index, NULL, ELM_GENLIST_ITEM_NONE, _gl_radio_sel, NULL);
			}
		}
	}

	evas_object_show(genlist);
	elm_object_part_content_set(layout, "elm.swallow.content", genlist);

	fclose(fp);

	LOGD("[%s(): %d] END <<<<", __FUNCTION__, __LINE__);

	return layout;
}

static void _back_clicked_cb_from_predefined_item(void *data, Evas_Object *obj, void *event_info)
{
	LOGD("[%s(): %d] BEGIN >>>>", __FUNCTION__, __LINE__);

	ugdata_t *ug_data = data;
	int index;

	elm_naviframe_item_pop(ug_data->base_naviframe);

	for (index  = 0; index < predefined_item_count; index++)
	{
		free(predefined_item_list[index]);
	}
	free(predefined_item_list);

	predefined_item_count = 0;

	if (radio_main)
	{
		evas_object_del(radio_main);
		radio_main = NULL;
	}

	LOGD("[%s(): %d] END <<<<", __FUNCTION__, __LINE__);
}

// get the state of item
static Eina_Bool _gl_state_get(void *data, Evas_Object *obj, const char *part)
{
	Eina_Bool result = EINA_FALSE;
	int enable = 0;

	LOGD("[%s(): %d] BEGIN >>>>", __FUNCTION__, __LINE__);

	gl_item_data *item_data = (gl_item_data *)data;

	if (item_data == NULL)
	{
		LOGD("[%s(): %d] item_data is null", __FUNCTION__, __LINE__);
		return EINA_FALSE;
	}

	if (item_data->type == NFC_ON_OFF)
	{
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
	}
#ifdef _SBEAM_SUPPORT_
	else if (item_data->type == NFC_S_BEAM)
	{
		if (!vconf_get_bool(VCONFKEY_NFC_SBEAM, &enable))
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
	}
#endif
	else if (item_data->type == NFC_PREDEFINED_ITEM)
	{
		char *selected_item = NULL;

		selected_item = vconf_get_str(VCONFKEY_NFC_PREDEFINED_ITEM);

		if (strcmp(selected_item, "None"))
		{
			LOGD("[%s(): %d] vconf_get_str=> is not None", __FUNCTION__, __LINE__);
			result = EINA_TRUE;
		}
	}
	else
	{
		LOGD("[%s(): %d] item_data->type error", __FUNCTION__, __LINE__);
	}

	LOGD("[%s(): %d] END <<<<", __FUNCTION__, __LINE__);

	return result;
}

// callback for 'deletion'
static void _gl_del(void *data, Evas_Object *obj)
{
	gl_item_data *item_data = (gl_item_data *)data;

	LOGD("[%s(): %d] BEGIN >>>>", __FUNCTION__, __LINE__);

	free(item_data);

	LOGD("[%s(): %d] END <<<<", __FUNCTION__, __LINE__);
}

static void _gl_sel_activation(void *data, Evas_Object *obj, void *event_info)
{
	LOGD("[%s(): %d] BEGIN >>>>", __FUNCTION__, __LINE__);

	Elm_Object_Item *item = (Elm_Object_Item *)event_info;
	gl_item_data *item_data = (gl_item_data *)data;

	elm_genlist_item_selected_set(item, 0);

	if ((item == NULL) || (item_data == NULL))
	{
		LOGD("[%s(): %d] item or item_data is null\n", __FUNCTION__, __LINE__);
		return;
	}

	if (item_data->type == NFC_ON_OFF)
	{
		if (_get_pending_status())
		{
			LOGD("[%s(): %d] pending status \n", __FUNCTION__, __LINE__);
			return;
		}

		_change_nfc_onoff_setting(item_data->data);

		elm_genlist_item_update(item);
	}
#ifdef _SBEAM_SUPPORT_
	else if (item_data->type == NFC_S_BEAM)
	{
		_change_nfc_sbeam_setting();

		elm_genlist_item_update(item);
	}
#endif
	else if (item_data->type == NFC_PREDEFINED_ITEM)
	{
		Evas_Object *predefined_item_layout = NULL;
		Evas_Object *l_button = NULL;
		ugdata_t *ug_data = item_data->data;

		LOGD("[%s(): %d] NFC_PREDEFINED_ITEM", __FUNCTION__, __LINE__);

		/* create setting view */
		predefined_item_layout = _ug_nfc_create_predefined_item_layout(ug_data);
		if (predefined_item_layout == NULL)
			return;

		/* Push navifreme */
		l_button = elm_button_add(ug_data->base_naviframe);
		elm_object_style_set(l_button, "naviframe/back_btn/default");
		evas_object_smart_callback_add(l_button, "clicked", _back_clicked_cb_from_predefined_item, ug_data);

		ug_data->base_navi_it = elm_naviframe_item_push(ug_data->base_naviframe, IDS_PREDEFINED_ITEM, l_button, NULL, predefined_item_layout, NULL);
	}
	else
	{
		LOGD("[%s(): %d] item_data->type error", __FUNCTION__, __LINE__);
	}

	LOGD("[%s(): %d] END <<<<", __FUNCTION__, __LINE__);
}

static void _check_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	gl_item_data *item_data = (gl_item_data *)data;

	LOGD("[%s(): %d] BEGIN >>>>", __FUNCTION__, __LINE__);

	if (item_data == NULL)
	{
		LOGD("[%s(): %d] item_data is null\n", __FUNCTION__, __LINE__);
		return;
	}

	if (item_data->type == NFC_ON_OFF)
	{
		if (_get_pending_status())
		{
			LOGD("[%s(): %d] pending status \n", __FUNCTION__, __LINE__);
			return;
		}

		_change_nfc_onoff_setting(item_data->data);

		if (on_off_item != NULL)
			elm_genlist_item_update(on_off_item);
	}
#ifdef _SBEAM_SUPPORT_
	else if (item_data->type == NFC_S_BEAM)
	{
		_change_nfc_sbeam_setting();

		if (sbeam_item != NULL)
			elm_genlist_item_update(sbeam_item);
	}
#endif
	else
	{
		LOGD("[%s(): %d] item_data->type error", __FUNCTION__, __LINE__);
	}

	LOGD("[%s(): %d] END <<<<", __FUNCTION__, __LINE__);
}

static Evas_Object *_gl_content_get(void *data, Evas_Object *obj, const char *part)
{
	LOGD("[%s(): %d] BEGIN >>>>", __FUNCTION__, __LINE__);
	Evas_Object *content = NULL;
	int on;
	int result = EINA_FALSE;
	ugdata_t *ug_data = NULL;

	gl_item_data *item_data = (gl_item_data *)data;
	if (item_data == NULL)
	{
		LOGD("[%s(): %d] item_data is null", __FUNCTION__, __LINE__);
		return NULL;
	}

	ug_data = item_data->data;
	if (ug_data == NULL)
	{
		LOGD("[%s(): %d] ug_data is null", __FUNCTION__, __LINE__);
		return NULL;
	}

	if (item_data->type == NFC_ON_OFF)
	{
		if (_get_pending_status())
		{
			content = elm_progressbar_add(obj);
			elm_object_style_set(content, "list_process");
			evas_object_size_hint_align_set(content, EVAS_HINT_FILL, 0.5);
			evas_object_size_hint_weight_set(content, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
			evas_object_show(content);
			elm_progressbar_pulse(content, EINA_TRUE);
		}
		else
		{
			content = elm_check_add(obj);
			evas_object_propagate_events_set(content, EINA_FALSE);
			evas_object_smart_callback_add(content, "changed", _check_changed_cb, item_data);

			if (((result = vconf_get_bool(VCONFKEY_NFC_STATE, &on)) == 0) && (on != 0))
			{
				LOGD("[%s(): %d] vconf_get_bool true", __FUNCTION__, __LINE__);
				elm_check_state_set(content, EINA_TRUE);
			}
			else
			{
				LOGD("[%s(): %d] vconf_get_bool false", __FUNCTION__, __LINE__);
				elm_check_state_set(content, EINA_FALSE);
			}

			elm_object_style_set(content, "on&off");
		}
	}
#ifdef _SBEAM_SUPPORT_
	else if (item_data->type == NFC_S_BEAM)
	{
		content = elm_check_add(obj);
		evas_object_propagate_events_set(content, EINA_FALSE);
		evas_object_smart_callback_add(content, "changed", _check_changed_cb, item_data);

		if (((result = vconf_get_bool(VCONFKEY_NFC_SBEAM, &on)) == 0) && (on != 0))
		{
			LOGD("[%s(): %d] vconf_get_bool true", __FUNCTION__, __LINE__);
			elm_check_state_set(content, EINA_TRUE);
		}
		else
		{
			LOGD("[%s(): %d] vconf_get_bool false", __FUNCTION__, __LINE__);
			elm_check_state_set(content, EINA_FALSE);
		}

		elm_object_style_set(content, "on&off");
	}
#endif
	else if (item_data->type == NFC_PREDEFINED_ITEM)
	{
		LOGD("[%s(): %d] NFC_PREDEFINED_ITEM ", __FUNCTION__, __LINE__);
	}
	else
	{
		LOGD("[%s(): %d] item_data->type error", __FUNCTION__, __LINE__);
	}

	LOGD("[%s(): %d] END <<<<", __FUNCTION__, __LINE__);

	return content;
}

static char *_gl_text_get_onoff(void *data, Evas_Object *obj, const char *part)
{
	LOGD("[%s(): %d] BEGIN >>>>", __FUNCTION__, __LINE__);

	gl_item_data *item_data = (gl_item_data *)data;
	char *text = NULL;

	LOGD("part:%s", part);

	// label for 'elm.text' part
	if (item_data == NULL)
	{
		LOGD("[%s(): %d] item_data is null", __FUNCTION__, __LINE__);
		return NULL;
	}

	if (item_data->type == NFC_ON_OFF)
	{
		text = strdup(IDS_NFC_NFC);
	}
#ifdef _SBEAM_SUPPORT_
	else if (item_data->type == NFC_S_BEAM)
	{
		text = strdup(IDS_NFC_S_BEAM);
	}
#endif
	else if (item_data->type == NFC_PREDEFINED_ITEM)
	{
		if (!strcmp(part, "elm.text.1"))
		{
			text = strdup(IDS_PREDEFINED_ITEM);
		}
		else
		{
			text = vconf_get_str(VCONFKEY_NFC_PREDEFINED_ITEM);
			LOGD("[%s]", text);
		}
	}
	else
	{
		LOGD("[%s(): %d] type error", __FUNCTION__, __LINE__);
	}

	LOGD("[%s(): %d] END <<<<", __FUNCTION__, __LINE__);

	return text;
}

static char *_gl_text_get_help(void *data, Evas_Object *obj, const char *part)
{
	LOGD("[%s(): %d] BEGIN >>>>", __FUNCTION__, __LINE__);

	int index = (int) data;
	char *text = NULL;

	LOGD("index:%d", index);

	if (index == 0)
	{
		text = strdup(IDS_NFC_S_BEAM_DESCRIPTION_MSG_CHN);

	}
	else if (index == 1)
	{
		text = strdup(IDS_PREDEFINED_ITEM_DESCRIPTION_MSG);
	}

	LOGD("[%s(): %d] END <<<<", __FUNCTION__, __LINE__);

	return text;
}

static Evas_Object *_ug_nfc_create_setting_layout(void *data)
{
	LOGD("[%s(): %d] BEGIN >>>>", __FUNCTION__, __LINE__);

	ugdata_t *ug_data = (ugdata_t *)data;
	Evas_Object *layout = NULL;
	Evas_Object *genlist = NULL;
	int result = 0;
	int on = 0;

	/* Add layout*/
	layout = elm_layout_add(ug_data->base_naviframe);
	if (layout == NULL)
	{
		LOGD("[%s(): %d] layout is null", __FUNCTION__, __LINE__);
		return NULL;
	}
	elm_layout_theme_set(layout, "layout", "application", "noindicator");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(layout);

	/* make genlist */
	genlist = elm_genlist_add(layout);
	if (genlist == NULL)
	{
		LOGD("[%s(): %d] genlist is null", __FUNCTION__, __LINE__);
		return NULL;
	}
	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);

	itc_check.item_style = "dialogue/1text.1icon";
	itc_check.func.text_get = _gl_text_get_onoff;
	itc_check.func.content_get = _gl_content_get;
	itc_check.func.state_get = _gl_state_get;
	itc_check.func.del = _gl_del;

	itc_helptext.item_style = "multiline/1text";
	itc_helptext.func.text_get = _gl_text_get_help;
	itc_helptext.func.content_get = NULL;
	itc_helptext.func.state_get = NULL;
	itc_helptext.func.del = NULL;

	itc_2text.item_style = "dialogue/2text.3";
	itc_2text.func.text_get = _gl_text_get_onoff;
	itc_2text.func.content_get = _gl_content_get;
	itc_2text.func.state_get = _gl_state_get;
	itc_2text.func.del = _gl_del;


	/* 1. NFC setting */
	gl_item_data *on_off_data = NULL;
	on_off_data = malloc(sizeof(gl_item_data));
	on_off_data->type = NFC_ON_OFF;
	on_off_data->data = ug_data;
	on_off_item = elm_genlist_item_append(genlist, &itc_check, (void *)on_off_data, NULL, ELM_GENLIST_ITEM_NONE, _gl_sel_activation, (void *)on_off_data);

#ifdef _SBEAM_SUPPORT_
	/* 2. S beam setting */
	gl_item_data *sbeam_data = NULL;
	sbeam_data = malloc(sizeof(gl_item_data));
	sbeam_data->type = NFC_S_BEAM;
	sbeam_data->data = ug_data;
	sbeam_item = elm_genlist_item_append(genlist, &itc_check, (void *)sbeam_data, NULL, ELM_GENLIST_ITEM_NONE, _gl_sel_activation, (void *)sbeam_data);

	/* 2. S beam setting Help Text */
	sbeam_item_help = elm_genlist_item_append(genlist, &itc_helptext, (void *)0, NULL, ELM_GENLIST_ITEM_NONE, _gl_sel_activation, NULL);

	if (((result = vconf_get_bool(VCONFKEY_NFC_STATE, &on)) == 0) && (on == 0))
	{
		elm_object_item_disabled_set(sbeam_item, TRUE);
		elm_object_item_disabled_set(sbeam_item_help, TRUE);
	}
#endif

	/* 3. Predefined item setting */
	gl_item_data *predefined_item_data = NULL;
	predefined_item_data = malloc(sizeof(gl_item_data));
	predefined_item_data->type = NFC_PREDEFINED_ITEM;
	predefined_item_data->data = ug_data;
	predefined_item = elm_genlist_item_append(genlist, &itc_2text, (void *)predefined_item_data, NULL, ELM_GENLIST_ITEM_NONE, _gl_sel_activation, (void *)predefined_item_data);

	/* 3. Predefined item setting Help Text*/
	predefined_item_help = elm_genlist_item_append(genlist, &itc_helptext, (void *)1, NULL, ELM_GENLIST_ITEM_NONE, _gl_sel_activation, NULL);

	if (((result = vconf_get_bool(VCONFKEY_NFC_STATE, &on)) == 0) && (on == 0))
	{
		elm_object_item_disabled_set(predefined_item, TRUE);
		elm_object_item_disabled_set(predefined_item_help, TRUE);
	}

	evas_object_show(genlist);
	elm_object_part_content_set(layout, "elm.swallow.content", genlist);

	LOGD("[%s(): %d] END <<<<", __FUNCTION__, __LINE__);

	return layout;
}

static void _back_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	LOGD("[%s(): %d] BEGIN >>>>", __FUNCTION__, __LINE__);

	ug_destroy_me(data);

	LOGD("[%s(): %d] END <<<<", __FUNCTION__, __LINE__);
}

static void *__ug_nfc_setting_create(ui_gadget_h ug, enum ug_mode mode, service_h service, void *priv)
{
	LOGD("[%s(): %d] BEGIN >>>>", __FUNCTION__, __LINE__);

	ugdata_t *ug_data = (ugdata_t *)priv;
	Evas_Object *parent = NULL;
	Evas_Object *nfc_setting_layout = NULL;
	Evas_Object *l_button = NULL;

	/* set text domain */
	bindtextdomain(NFCUG_TEXT_DOMAIN, NFCUG_LOCALEDIR);

	parent = ug_get_parent_layout(ug);
	if (!parent)
		return NULL;
	ug_data->ug_win_main = parent;
	evas_object_show(ug_data->ug_win_main);
	ug_data->nfc_setting_ug = ug;

	ug_data->base_layout = _create_main_layout(ug_data->ug_win_main);
	ug_data->bg = _create_bg(ug_data->base_layout);
	elm_object_part_content_set(ug_data->base_layout, "elm.swallow.bg", ug_data->bg);

	ug_data->base_naviframe = elm_naviframe_add(ug_data->base_layout);
	elm_object_part_content_set(ug_data->base_layout, "elm.swallow.content", ug_data->base_naviframe);
	evas_object_show(ug_data->base_naviframe);

	/* create setting view */
	nfc_setting_layout = _ug_nfc_create_setting_layout(ug_data);
	if (nfc_setting_layout == NULL)
		return NULL;

	/* Push navifreme */
	l_button = elm_button_add(ug_data->base_naviframe);
	elm_object_style_set(l_button, "naviframe/back_btn/default");
	evas_object_smart_callback_add(l_button, "clicked", _back_clicked_cb, ug_data->nfc_setting_ug);

	ug_data->base_navi_it = elm_naviframe_item_push(ug_data->base_naviframe, IDS_NFC_NFC, l_button, NULL, nfc_setting_layout, NULL);

	/* Register activation changed callback */
	nfc_manager_initialize(NULL, NULL);
	nfc_manager_set_activation_changed_cb(_nfc_activation_changed_cb, ug_data);

	LOGD("[%s(): %d] END <<<<", __FUNCTION__, __LINE__);

	return ug_data->base_layout;
}

static void __ug_nfc_setting_destroy(ui_gadget_h ug, service_h service, void *priv)
{
	LOGD("[%s(): %d] BEGIN >>>>", __FUNCTION__, __LINE__);
	ugdata_t *ug_data = (ugdata_t *)priv;

	if (ug_data == NULL)
		return;

	nfc_manager_unset_activation_changed_cb();

	nfc_manager_deinitialize();

	evas_object_del(ug_get_layout(ug));

	LOGD("[%s(): %d] END <<<<", __FUNCTION__, __LINE__);
}

static void __ug_nfc_setting_start(ui_gadget_h ug, service_h service, void *priv)
{
	LOGD("[%s(): %d] BEGIN >>>>", __FUNCTION__, __LINE__);

	ugdata_t *ug_data = (ugdata_t *)priv;

	/* check nfc-device*/
	if (!nfc_manager_is_supported())
	{
		LOGD("It is not nfc device >>>>");
		_show_app_error_popup(ug_data);
	}

	LOGD("[%s(): %d] END <<<<", __FUNCTION__, __LINE__);
}

static void __ug_nfc_setting_pause(ui_gadget_h ug, service_h service, void *priv)
{
	LOGD("[%s(): %d] BEGIN >>>>", __FUNCTION__, __LINE__);
	LOGD("[%s(): %d] END <<<<", __FUNCTION__, __LINE__);
}

static void __ug_nfc_setting_resume(ui_gadget_h ug, service_h service, void *priv)
{
	LOGD("[%s(): %d] BEGIN >>>>", __FUNCTION__, __LINE__);
	LOGD("[%s(): %d] END <<<<", __FUNCTION__, __LINE__);
}

static void __ug_nfc_setting_message(ui_gadget_h ug, service_h msg, service_h service, void *priv)
{
	LOGD("[%s(): %d] BEGIN >>>>", __FUNCTION__, __LINE__);
	LOGD("[%s(): %d] END <<<<", __FUNCTION__, __LINE__);
}

static void __ug_nfc_setting_event(ui_gadget_h ug, enum ug_event event, service_h service, void *priv)
{
	LOGD("[%s(): %d] BEGIN >>>>", __FUNCTION__, __LINE__);
	LOGD("[%s(): %d] END <<<<", __FUNCTION__, __LINE__);
}

UG_MODULE_API int UG_MODULE_INIT(struct ug_module_ops *ops)
{
	ugdata_t *ug_data; // User defined private data

	LOGD("[%s(): %d] UG_MODULE_INIT!!\n", __FUNCTION__, __LINE__);

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
	ugdata_t *ug_data;

	if (!ops)
		return;

	ug_data = ops->priv;

	if (ug_data)
		free(ug_data);

	ops->priv = NULL;
}
