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
#include "ug-nfc-setting-db.h"
#ifdef MDM_PHASE_2
#include <mdm.h>
#endif

#ifndef UG_MODULE_API
#define UG_MODULE_API __attribute__ ((visibility("default")))
#endif

static Elm_Genlist_Item_Class itc_sep;
static Elm_Genlist_Item_Class itc_sep_help;
static Elm_Genlist_Item_Class itc_check;
static Elm_Genlist_Item_Class itc_helptext;
static Elm_Genlist_Item_Class itc_2text;

static Elm_Object_Item *nfc_item = NULL;
#ifdef _SBEAM_SUPPORT_
static Elm_Object_Item *sbeam_item = NULL;
#endif
static Elm_Object_Item *predefined_item = NULL;

static bool pending_status = false;


#ifdef MDM_PHASE_2
static void _mdm_restricted_popup_response_cb(void *data, Evas_Object *obj, void *event_info)
{
	LOGD("BEGIN >>>>");
	ugdata_t *ug_data = (ugdata_t *)data;
	if (!ug_data)
		return;

	LOGD("END >>>>");
}

static void _mdm_restricted_popup(void *data)
{
	LOGD("BEGIN >>>>");

	char popup_str[POPUP_TEXT_SIZE] = { 0, };
	char *buf = NULL;
	ugdata_t *ug_data = (ugdata_t *)data;
	if (!ug_data)
		return;

	buf = strdup(IDS_SECURITY_POLICY_RESTRICTS_USE_OF_PS);
	snprintf(popup_str, POPUP_TEXT_SIZE, buf, IDS_NFC_NFC);
	ug_nfc_setting_create_popup(ug_data, ug_data->base_layout, popup_str, NULL, 0, NULL, 0, NULL, 0, true, true, _mdm_restricted_popup_response_cb);

	LOGD("END >>>>");
}
#endif

static void _show_app_error_popup_response_cb(void *data, Evas_Object *obj, void *event_info)
{
	ugdata_t *ug_data = (ugdata_t *)data;
	int result = (int)event_info;

	LOGD("BEGIN >>>>");

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

	LOGD("END <<<<");
}

static void _show_app_error_popup(void *data)
{
	ugdata_t *ug_data = (ugdata_t *)data;
	char popup_str[POPUP_TEXT_SIZE] = { 0, };

	LOGD("BEGIN >>>>");

	if (ug_data == NULL)
		return;

	memcpy(popup_str, IDS_NFC_SERVICE_IS_NOT_SUPPORTED, strlen(IDS_NFC_SERVICE_IS_NOT_SUPPORTED));

	ug_nfc_setting_create_popup(ug_data, ug_data->base_layout, popup_str, IDS_CLOSE, UG_NFC_POPUP_RESP_CLOSE, NULL, 0, NULL, 0, false, false, _show_app_error_popup_response_cb);

	LOGD("END <<<<");
}

static bool _get_pending_status(void)
{
	return pending_status;
}

static void _set_pending_status(bool status)
{
	pending_status = status;
}

static void _back_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	LOGD("BEGIN >>>>");

	_ug_nfc_setting_db_close();

	ug_destroy_me(data);

	LOGD("END <<<<");
}

/* Principle of NFC, S Beam On/Off 				*/
/*							*/
/* NFC & S Beam Off -> NFC On 	: NFC: On, S Beam: Off	*/
/* NFC & S Beam Off -> S Beam On 	: NFC: On, S Beam: On	*/
/* NFC & S Beam On -> S Beam Off 	: NFC: On, S Beam: Off	*/
/* NFC & S Beam On -> NFC Off 	: NFC: Off, S Beam: Off 	*/

static void _change_nfc_onoff_setting(void *data)
{
	int status;
	int result;

	LOGD("BEGIN >>>>");

	ugdata_t *ug_data = (ugdata_t *)data;
	if (!ug_data)
		return;

	/* check MDM */
#ifdef MDM_PHASE_2
	result = mdm_get_service();
	if (result == MDM_RESULT_SUCCESS)
	{
		result = mdm_get_allow_nfc();
		if (result == MDM_ALLOWED)
		{
			LOGD( "MDM_ALLOWED!\n");
			mdm_release_service();
		}
		else if (result == MDM_RESTRICTED)
		{
			LOGD( "MDM_RESTRICTED!\n");
			_mdm_restricted_popup(ug_data);
			return;
		}
		else
		{
			LOGD( "exception case!\n");
			return;
		}
	}
	else
	{
		LOGD( "failed to get mdm service!\n");
		return;
	}
#endif

	if ((result = vconf_get_bool(VCONFKEY_NFC_STATE, &status)) == 0)
	{
		LOGD("vconf_get_bool status [%d]\n", status);

		result = nfc_manager_set_activation(!status, NULL, NULL);
		_set_pending_status(true);
	}
	else
	{
		LOGD("vconf_get_bool failed\n");
	}

	LOGD("END <<<<");
}

#ifdef _SBEAM_SUPPORT_
static void _change_nfc_sbeam_setting(void *data)
{
	int status = 0;
	int result = -1;

	LOGD("BEGIN >>>>");

	ugdata_t *ug_data = (ugdata_t *)data;
	if (!ug_data)
		return;

	/* check MDM */
#ifdef MDM_PHASE_2
	result = mdm_get_service();
	if (result == MDM_RESULT_SUCCESS)
	{
		result = mdm_get_allow_nfc();
		if (result == MDM_ALLOWED)
		{
			LOGD( "MDM_ALLOWED!\n");
			mdm_release_service();
		}
		else if (result == MDM_RESTRICTED)
		{
			LOGD( "MDM_RESTRICTED!\n");
			_mdm_restricted_popup(ug_data);
			return;
		}
		else
		{
			LOGD( "exception case!\n");
			return;
		}
	}
	else
	{
		LOGD( "failed to get mdm service!\n");
		return;
	}
#endif

	if ((result = vconf_get_bool(VCONFKEY_NFC_SBEAM, &status)) == 0)
	{
		LOGD("vconf_get_bool status [%d]\n", status);

		if (status)
		{
			/* sbeam off */
			result = vconf_set_bool(VCONFKEY_NFC_SBEAM, FALSE);
			if (!result )
			{
				LOGD("vconf_set_bool success\n");
			}
			else
			{
				LOGD("vconf_set_bool failed\n");
			}
		}
		else
		{
			/* sbeam on */
			result = vconf_set_bool(VCONFKEY_NFC_SBEAM, TRUE);
			if (!result )
			{
				LOGD("vconf_set_bool success\n");
			}
			else
			{
				LOGD("vconf_set_bool failed\n");
			}

			/* nfc on */
			if ((result = vconf_get_bool(VCONFKEY_NFC_STATE, &status)) == 0)
			{
				LOGD("vconf_get_bool status [%d]\n", status);
				if (!status)
				{
					result = nfc_manager_set_activation(true, NULL, NULL);
					_set_pending_status(true);
				}
			}
			else
			{
				LOGD("vconf_get_bool failed\n");
			}
		}
	}
	else
	{
		LOGD("vconf_get_bool failed\n");
	}

	LOGD("END <<<<");
}
#endif

void _change_nfc_predefined_item_setting(void *data)
{
	int status = 0;
	int result = -1;

	LOGD("BEGIN >>>>");

	ugdata_t *ug_data = (ugdata_t *)data;
	if (!ug_data)
		return;

	/* check MDM */
#ifdef MDM_PHASE_2
	result = mdm_get_service();
	if (result == MDM_RESULT_SUCCESS)
	{
		result = mdm_get_allow_nfc();
		if (result == MDM_ALLOWED)
		{
			LOGD( "MDM_ALLOWED!\n");
			mdm_release_service();
		}
		else if (result == MDM_RESTRICTED)
		{
			LOGD( "MDM_RESTRICTED!\n");
			_mdm_restricted_popup(ug_data);
			return;
		}
		else
		{
			LOGD( "exception case!\n");
			return;
		}
	}
	else
	{
		LOGD( "failed to get mdm service!\n");
		return;
	}
#endif

	if ((result = vconf_get_bool(VCONFKEY_NFC_PREDEFINED_ITEM_STATE, &status)) == 0)
	{
		LOGD("vconf_get_bool status [%d]\n", status);

		if (status)
		{
			/* predefined item off */
			result = vconf_set_bool(VCONFKEY_NFC_PREDEFINED_ITEM_STATE, FALSE);
			if (!result )
			{
				LOGD("vconf_set_bool success\n");
			}
			else
			{
				LOGD("vconf_set_bool failed\n");
			}
		}
		else
		{
			/* predefined item on */
			result = vconf_set_bool(VCONFKEY_NFC_PREDEFINED_ITEM_STATE, TRUE);
			if (!result )
			{
				LOGD("vconf_set_bool success\n");
			}
			else
			{
				LOGD("vconf_set_bool failed\n");
			}
		}
	}
	else
	{
		LOGD("vconf_get_bool failed\n");
	}

	LOGD("END <<<<");
}

void _nfc_activation_changed_cb(bool activated , void *user_data)
{
	LOGD("BEGIN >>>>");

	ugdata_t *ug_data = (ugdata_t *)user_data;
	int predefined_item_state = 0;
	static bool predefined_item_off_by_nfcOnOff = false;
	int result;

	LOGD("nfc mode %s \n", activated ? "ON" : "OFF");


	if ((result = vconf_get_bool(VCONFKEY_NFC_PREDEFINED_ITEM_STATE, &predefined_item_state)) == 0)
	{
		LOGD("vconf_get_bool status [%d]\n", predefined_item_state);
	}
	else
	{
		LOGD("vconf_get_bool failed\n");
		return;
	}

	LOGD("The state of Predefined Item %d, Predefined Item was off by nfcOnOff %s \n",
			predefined_item_state, predefined_item_off_by_nfcOnOff == true ? "Yes" : "No");

	/* nfc setting ui updated */
	_set_pending_status(false);

	if (nfc_item != NULL)
		elm_genlist_item_update(nfc_item);

	if (activated == true)
	{
		/* predefined item setting enabled */
		elm_object_item_disabled_set(predefined_item, EINA_FALSE);

		if (predefined_item_off_by_nfcOnOff != true)
			return;

		LOGD("Turning Predefined Item on \n");

		_change_nfc_predefined_item_setting(ug_data);

		if (predefined_item != NULL)
			elm_genlist_item_update(predefined_item);

		/* unset internal flag */
		predefined_item_off_by_nfcOnOff = false;
	}
	else
	{
#ifdef _SBEAM_SUPPORT_
		int sbeam_state = 0;
#endif

		/* predefined item setting disabled */
		elm_object_item_disabled_set(predefined_item, EINA_TRUE);

		if (predefined_item_state == VCONFKEY_NFC_PREDEFINED_ITEM_OFF)
			return;

		LOGD("Turning Predefined Item off \n");

		_change_nfc_predefined_item_setting(ug_data);

		if (predefined_item != NULL)
			elm_genlist_item_update(predefined_item);

		/* set internal flag */
		predefined_item_off_by_nfcOnOff = true;

#ifdef _SBEAM_SUPPORT_
		/* sbeam off */
		if ((result = vconf_get_bool(VCONFKEY_NFC_SBEAM, &sbeam_state)) == 0)
		{
			LOGD("vconf_get_bool status [%d]\n", sbeam_state);

			if (sbeam_state)
			{
				/* sbeam off */
				result = vconf_set_bool(VCONFKEY_NFC_SBEAM, FALSE);
				if (!result )
				{
					LOGD("vconf_set_bool success\n");
				}
				else
				{
					LOGD("vconf_set_bool failed\n");
				}
			}
		}
		else
		{
			LOGD("vconf_get_bool failed\n");
		}
#endif
	}

	LOGD("END <<<<");
}

static Evas_Object *_create_bg(Evas_Object *parent, char *style)
{
	Evas_Object *bg = elm_bg_add(parent);

	evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	if (style)
		elm_object_style_set(bg, style);

	elm_win_resize_object_add(parent, bg);

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

// get the state of item
static Eina_Bool _gl_state_get(void *data, Evas_Object *obj, const char *part)
{
	Eina_Bool result = EINA_FALSE;
	int enable = 0;

	LOGD("BEGIN >>>>");

	gl_item_data *item_data = (gl_item_data *)data;

	if (item_data == NULL)
	{
		LOGD("item_data is null");
		return EINA_FALSE;
	}

	if (item_data->type == NFC_ON_OFF)
	{
		if (!vconf_get_bool(VCONFKEY_NFC_STATE, &enable))
		{
			if (enable != 0)
			{
				LOGD("vconf_get_bool true");
				result = EINA_TRUE;
			}
			else
			{
				LOGD("vconf_get_bool false");
			}
		}
		else
		{
			LOGD("vconf_get_bool error [%d]", result);
		}
	}
#ifdef _SBEAM_SUPPORT_
	else if (item_data->type == NFC_S_BEAM)
	{
		if (!vconf_get_bool(VCONFKEY_NFC_SBEAM, &enable))
		{
			if (enable != 0)
			{
				LOGD("vconf_get_bool true");
				result = EINA_TRUE;
			}
			else
			{
				LOGD("vconf_get_bool false");
			}
		}
		else
		{
			LOGD("vconf_get_bool error [%d]", result);
		}
	}
#endif
	else if (item_data->type == NFC_PREDEFINED_ITEM)
	{
		if (!vconf_get_bool(VCONFKEY_NFC_PREDEFINED_ITEM_STATE, &enable))
		{
			if (enable != 0)
			{
				LOGD("vconf_get_bool true");
				result = EINA_TRUE;
			}
			else
			{
				LOGD("vconf_get_bool false");
			}
		}
		else
		{
			LOGD("vconf_get_bool error [%d]", result);
		}
	}
	else
	{
		LOGD("item_data->type error");
	}

	LOGD("END <<<<");

	return result;
}

// callback for 'deletion'
static void _gl_del(void *data, Evas_Object *obj)
{
	gl_item_data *item_data = (gl_item_data *)data;

	LOGD("BEGIN >>>>");

	if (item_data != NULL)
		free(item_data);

	LOGD("END <<<<");
}

static void _gl_sel_activation(void *data, Evas_Object *obj, void *event_info)
{
	LOGD("BEGIN >>>>");

	Elm_Object_Item *item = (Elm_Object_Item *)event_info;
	gl_item_data *item_data = (gl_item_data *)data;

	elm_genlist_item_selected_set(item, 0);

	if ((item == NULL) || (item_data == NULL))
	{
		LOGD("item or item_data is null\n");
		return;
	}

	if (item_data->type == NFC_ON_OFF)
	{
		if (_get_pending_status())
		{
			LOGD("pending status \n");
			return;
		}

		_change_nfc_onoff_setting(item_data->data);

		elm_genlist_item_update(item);
	}
#ifdef _SBEAM_SUPPORT_
	else if (item_data->type == NFC_S_BEAM)
	{
		_change_nfc_sbeam_setting(item_data->data);

		elm_genlist_item_update(item);
	}
#endif
	else if (item_data->type == NFC_PREDEFINED_ITEM)
	{
		Evas_Object *predefined_item_list = NULL;
		Evas_Object *l_button = NULL;
		ugdata_t *ug_data = item_data->data;

		LOGD("NFC_PREDEFINED_ITEM");

		/* create setting view */
		predefined_item_list = _ug_nfc_create_predefined_item_list(ug_data);
		if (predefined_item_list == NULL)
			return;

		/* Push navifreme */
		l_button = elm_button_add(ug_data->base_naviframe);
		elm_object_style_set(l_button, "naviframe/back_btn/default");
		evas_object_smart_callback_add(l_button, "clicked", _ug_nfc_destroy_predefined_item_list, ug_data);

		ug_data->base_navi_it = elm_naviframe_item_push(ug_data->base_naviframe, IDS_PREDEFINED_ITEM, l_button, NULL, predefined_item_list, NULL);
	}
	else
	{
		LOGD("item_data->type error");
	}

	LOGD("END <<<<");
}

static void _check_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	gl_item_data *item_data = (gl_item_data *)data;

	LOGD("BEGIN >>>>");

	if (item_data == NULL)
	{
		LOGD("item_data is null\n");
		return;
	}

	if (item_data->type == NFC_ON_OFF)
	{
		if (_get_pending_status())
		{
			LOGD("pending status \n");
			return;
		}

		_change_nfc_onoff_setting(item_data->data);

		if (nfc_item != NULL)
			elm_genlist_item_update(nfc_item);
	}
#ifdef _SBEAM_SUPPORT_
	else if (item_data->type == NFC_S_BEAM)
	{
		_change_nfc_sbeam_setting(item_data->data);

		if (sbeam_item != NULL)
			elm_genlist_item_update(sbeam_item);
	}
#endif
	else if (item_data->type == NFC_PREDEFINED_ITEM)
	{
		_change_nfc_predefined_item_setting(item_data->data);

		if (predefined_item != NULL)
			elm_genlist_item_update(predefined_item);
	}
	else
	{
		LOGD("item_data->type error");
	}

	LOGD("END <<<<");
}

static Evas_Object *_gl_content_get(void *data, Evas_Object *obj, const char *part)
{
	LOGD("BEGIN >>>>");
	Evas_Object *content = NULL;
	int on;
	int result = EINA_FALSE;
	ugdata_t *ug_data = NULL;

	gl_item_data *item_data = (gl_item_data *)data;
	if (item_data == NULL)
	{
		LOGD("item_data is null");
		return NULL;
	}

	ug_data = item_data->data;
	if (ug_data == NULL)
	{
		LOGD("ug_data is null");
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
	}
#ifdef _SBEAM_SUPPORT_
	else if (item_data->type == NFC_S_BEAM)
	{
		content = elm_check_add(obj);
		evas_object_propagate_events_set(content, EINA_FALSE);
		evas_object_smart_callback_add(content, "changed", _check_changed_cb, item_data);

		if (((result = vconf_get_bool(VCONFKEY_NFC_SBEAM, &on)) == 0) && (on != 0))
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
#endif
	else if (item_data->type == NFC_PREDEFINED_ITEM)
	{
		content = elm_check_add(obj);
		evas_object_propagate_events_set(content, EINA_FALSE);
		evas_object_smart_callback_add(content, "changed", _check_changed_cb, item_data);

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
		LOGD("item_data->type error");
	}

	LOGD("END <<<<");

	return content;
}

static char *_gl_text_get_onoff(void *data, Evas_Object *obj, const char *part)
{
	LOGD("BEGIN >>>>");

	gl_item_data *item_data = (gl_item_data *)data;
	char *text = NULL;

	LOGD("part:%s", part);

	// label for 'elm.text' part
	if (item_data == NULL)
	{
		LOGD("item_data is null");
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
			char *app_id = NULL;

			app_id = vconf_get_str(VCONFKEY_NFC_PREDEFINED_ITEM);
			if (app_id != NULL)
			{
				LOGD("app_id=> [%s]", app_id);
				_ug_nfc_setting_db_get_pkgName(app_id, &text);

				free(app_id);
			}
		}
	}
	else
	{
		LOGD("type error");
	}

	LOGD("END <<<<");

	return text;
}

static char *_gl_text_get_help(void *data, Evas_Object *obj, const char *part)
{
	LOGD("BEGIN >>>>");

	int index = (int) data;
	char *text = NULL;

	LOGD("index:%d", index);

	if (index == 0)
	{
		text = strdup(IDS_NFC_DESCRIPTION_MSG);

	}
	else if (index == 1)
	{
		text = strdup(IDS_PREDEFINED_ITEM_DESCRIPTION_MSG);
	}
	else if (index == 2)
	{
		text = strdup(IDS_NFC_S_BEAM_DESCRIPTION_MSG);
	}

	LOGD("END <<<<");

	return text;
}

static Evas_Object *_ug_nfc_create_nfc_setting_list(void *data)
{
	LOGD("BEGIN >>>>");

	ugdata_t *ug_data = (ugdata_t *)data;
	Evas_Object *genlist = NULL;
	Elm_Object_Item* seperator = NULL;
	int result = 0;
	int on = 0;


	/* make genlist */
	genlist = elm_genlist_add(ug_data->base_naviframe);
	if (genlist == NULL)
	{
		LOGD("genlist is null");
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

	itc_2text.item_style = "dialogue/2text.1icon.10";
	itc_2text.func.text_get = _gl_text_get_onoff;
	itc_2text.func.content_get = _gl_content_get;
	itc_2text.func.state_get = _gl_state_get;
	itc_2text.func.del = _gl_del;


	/* SEPARATOR */
	seperator = elm_genlist_item_append(genlist, &itc_sep, NULL, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	elm_genlist_item_select_mode_set(seperator, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

	/* 1. NFC setting */
	gl_item_data *on_off_data = NULL;
	on_off_data = malloc(sizeof(gl_item_data));
	if (!on_off_data)
		return NULL;
	on_off_data->type = NFC_ON_OFF;
	on_off_data->data = ug_data;
	nfc_item = elm_genlist_item_append(genlist, &itc_check, (void *)on_off_data, NULL, ELM_GENLIST_ITEM_NONE, _gl_sel_activation, (void *)on_off_data);

	/* SEPARATOR.2 */
	seperator = elm_genlist_item_append(genlist, &itc_sep_help, NULL, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	elm_genlist_item_select_mode_set(seperator, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

	/* 1. NFC setting Help Text */
	elm_genlist_item_append(genlist, &itc_helptext, (void *)0, NULL, ELM_GENLIST_ITEM_NONE, _gl_sel_activation, NULL);

	/* SEPARATOR */
	seperator = elm_genlist_item_append(genlist, &itc_sep, NULL, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	elm_genlist_item_select_mode_set(seperator, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

	if (_ug_nfc_check_predefined_item_available())
	{
		LOGD("success to create predefined item");

		/* 2. Predefined item setting */
		gl_item_data *predefined_item_data = NULL;
		predefined_item_data = malloc(sizeof(gl_item_data));
		if (!predefined_item_data)
			return NULL;
		predefined_item_data->type = NFC_PREDEFINED_ITEM;
		predefined_item_data->data = ug_data;
		predefined_item = elm_genlist_item_append(genlist, &itc_2text, (void *)predefined_item_data, NULL, ELM_GENLIST_ITEM_NONE, _gl_sel_activation, (void *)predefined_item_data);

		/* SEPARATOR.2 */
		seperator = elm_genlist_item_append(genlist, &itc_sep_help, NULL, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
		elm_genlist_item_select_mode_set(seperator, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

		/* 2. Predefined item setting Help Text*/
		elm_genlist_item_append(genlist, &itc_helptext, (void *)1, NULL, ELM_GENLIST_ITEM_NONE, _gl_sel_activation, NULL);

		/* SEPARATOR */
		seperator = elm_genlist_item_append(genlist, &itc_sep, NULL, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
		elm_genlist_item_select_mode_set(seperator, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

		if (((result = vconf_get_bool(VCONFKEY_NFC_STATE, &on)) == 0) && (on == 0))
		{
			result = vconf_set_bool(VCONFKEY_NFC_PREDEFINED_ITEM_STATE, FALSE);
			if (!result )
			{
				LOGD("vconf_set_bool success\n");
			}
			else
			{
				LOGD("vconf_set_bool failed\n");
			}
			elm_object_item_disabled_set(predefined_item, EINA_TRUE);
		}
	}

	evas_object_show(genlist);

	LOGD("END <<<<");

	return genlist;
}

#ifdef _SBEAM_SUPPORT_
static Evas_Object *_ug_nfc_create_sbeam_setting_list(void *data)
{
	LOGD("BEGIN >>>>");

	ugdata_t *ug_data = (ugdata_t *)data;
	Evas_Object *genlist = NULL;
	Elm_Object_Item* seperator = NULL;


	/* make genlist */
	genlist = elm_genlist_add(ug_data->base_naviframe);
	if (genlist == NULL)
	{
		LOGD("genlist is null");
		return NULL;
	}
	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);

	itc_sep.item_style = "dialogue/separator";
	itc_sep.func.text_get = NULL;
	itc_sep.func.content_get = NULL;
	itc_sep.func.state_get = NULL;
	itc_sep.func.del = NULL;

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


	/* seperator */
	seperator= elm_genlist_item_append(genlist, &itc_sep, NULL, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	elm_genlist_item_select_mode_set(seperator, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

	/* S beam setting */
	gl_item_data *sbeam_data = NULL;
	sbeam_data = malloc(sizeof(gl_item_data));
	if (!sbeam_data)
		return NULL;
	sbeam_data->type = NFC_S_BEAM;
	sbeam_data->data = ug_data;
	sbeam_item = elm_genlist_item_append(genlist, &itc_check, (void *)sbeam_data, NULL, ELM_GENLIST_ITEM_NONE, _gl_sel_activation, (void *)sbeam_data);

	/* S beam setting Help Text */
	elm_genlist_item_append(genlist, &itc_helptext, (void *)2, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);

	evas_object_show(genlist);

	LOGD("END <<<<");

	return genlist;
}
#endif

static void *__ug_nfc_setting_create(ui_gadget_h ug, enum ug_mode mode, service_h service, void *priv)
{
	LOGD("BEGIN >>>>");

	ugdata_t *ug_data = (ugdata_t *)priv;
	Evas_Object *parent = NULL;
	Evas_Object *nfc_setting_list = NULL;
	Evas_Object *l_button = NULL;
	char *type = NULL;

	/* set text domain */
	bindtextdomain(NFCUG_TEXT_DOMAIN, NFCUG_LOCALEDIR);

	/* create base view */
	parent = ug_get_parent_layout(ug);
	if (!parent)
		return NULL;
	ug_data->ug_win_main = parent;
	evas_object_show(ug_data->ug_win_main);
	ug_data->nfc_setting_ug = ug;

	ug_data->base_layout = _create_main_layout(ug_data->ug_win_main);
	ug_data->bg = _create_bg(ug_data->ug_win_main, "group_list");
	elm_object_part_content_set(ug_data->base_layout, "elm.swallow.bg", ug_data->bg);

	ug_data->base_naviframe = elm_naviframe_add(ug_data->base_layout);
	elm_object_part_content_set(ug_data->base_layout, "elm.swallow.content", ug_data->base_naviframe);
	evas_object_show(ug_data->base_naviframe);

	/* parse parameter */
	service_get_extra_data(service, "type", &type);

	if (type == NULL)
	{
		LOGD("type is NULL");

		return NULL;
	}
	else
	{
		LOGD("type => [%s]", type);

		if (strncmp(type, "nfc", strlen("nfc")) == 0)
		{
			_ug_nfc_setting_db_open();

			nfc_setting_list = _ug_nfc_create_nfc_setting_list(ug_data);
		}
#ifdef _SBEAM_SUPPORT_
		else if (strncmp(type, "sbeam", strlen("sbeam")) == 0)
		{
			nfc_setting_list = _ug_nfc_create_sbeam_setting_list(ug_data);
		}
#endif
		else
		{
			return NULL;
		}
	}

	if (nfc_setting_list == NULL)
		return NULL;

	/* Push navifreme */
	l_button = elm_button_add(ug_data->base_naviframe);
	elm_object_style_set(l_button, "naviframe/back_btn/default");
	evas_object_smart_callback_add(l_button, "clicked", _back_clicked_cb, ug_data->nfc_setting_ug);

	if (type == NULL)
	{
		ug_data->base_navi_it = elm_naviframe_item_push(ug_data->base_naviframe, IDS_NFC_NFC, l_button, NULL, nfc_setting_list, NULL);
	}
	else
	{
		if (strncmp(type, "sbeam", strlen("sbeam")) == 0)
		{
			ug_data->base_navi_it = elm_naviframe_item_push(ug_data->base_naviframe, IDS_NFC_S_BEAM, l_button, NULL, nfc_setting_list, NULL);
		}
		else
		{
			ug_data->base_navi_it = elm_naviframe_item_push(ug_data->base_naviframe, IDS_NFC_NFC, l_button, NULL, nfc_setting_list, NULL);
		}
	}
	/* Register activation changed callback */
	if (NFC_ERROR_NONE == nfc_manager_initialize(NULL, NULL))
	{
		LOGD("nfc_manager_initialize success\n");
		nfc_manager_set_activation_changed_cb(_nfc_activation_changed_cb, ug_data);
	}
	else
	{
		LOGD("nfc_manager_initialize FAIL!!!!\n");
	}

	LOGD("END <<<<");

	return ug_data->base_layout;
}

static void __ug_nfc_setting_destroy(ui_gadget_h ug, service_h service, void *priv)
{
	LOGD("BEGIN >>>>");
	ugdata_t *ug_data = (ugdata_t *)priv;

	if (ug_data == NULL)
		return;

	nfc_manager_unset_activation_changed_cb();

	if (nfc_manager_deinitialize() != NFC_ERROR_NONE)
	{
		LOGD("nfc_manager_deinitialize failed");
	}

	evas_object_del(ug_get_layout(ug));

	LOGD("END <<<<");
}

static void __ug_nfc_setting_start(ui_gadget_h ug, service_h service, void *priv)
{
	LOGD("BEGIN >>>>");

	ugdata_t *ug_data = (ugdata_t *)priv;

	/* check nfc-device*/
	if (!nfc_manager_is_supported())
	{
		LOGD("It is not nfc device >>>>");
		_show_app_error_popup(ug_data);
	}

	LOGD("END <<<<");
}

static void __ug_nfc_setting_pause(ui_gadget_h ug, service_h service, void *priv)
{
	LOGD("BEGIN >>>>");
	LOGD("END <<<<");
}

static void __ug_nfc_setting_resume(ui_gadget_h ug, service_h service, void *priv)
{
	LOGD("BEGIN >>>>");
	LOGD("END <<<<");
}

static void __ug_nfc_setting_message(ui_gadget_h ug, service_h msg, service_h service, void *priv)
{
	LOGD("BEGIN >>>>");
	LOGD("END <<<<");
}

static void __ug_nfc_setting_event(ui_gadget_h ug, enum ug_event event, service_h service, void *priv)
{
	LOGD("BEGIN >>>>");
	LOGD("END <<<<");
}

UG_MODULE_API int UG_MODULE_INIT(struct ug_module_ops *ops)
{
	ugdata_t *ug_data; // User defined private data

	LOGD("UG_MODULE_INIT!!\n");

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
