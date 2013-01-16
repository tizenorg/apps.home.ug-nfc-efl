/*
  * Copyright (c) 2012, 2013 Samsung Electronics Co., Ltd.
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


#ifndef __UG_NFC_SETTING_MAIN_H__
#define __UG_NFC_SETTING_MAIN_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdio.h>
#include <ui-gadget-module.h>
#include <Evas.h>
#include <Ecore.h>
#include <Ecore_X.h>
#include <Elementary.h>
#include <bundle.h>
#include <dlog.h>
#include <vconf.h>
#include <nfc.h>

#define LOCALE_PATH		LOCALEDIR
#define EDJ_PATH 		EDJDIR
#define EDJ_FILE			EDJ_PATH"/"PACKAGE".edj"

#define PREDEFINED_ITEM_FILE_PATH		"/usr/apps/org.tizen.57r43275q7/share"

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "UG_NFC_EFL_SETTING"

#define NFCUG_TEXT_DOMAIN		PACKAGE
#define NFCUG_LOCALEDIR		LOCALE_PATH

#define POPUP_TEXT_SIZE	1024

/* Registered string in STMS NFC */
#define IDS_NFC_NFC					dgettext(PACKAGE, "IDS_NFC_BODY_NFC")
#define IDS_NFC_DESCRIPTION_MSG			dgettext(PACKAGE, "IDS_NFC_BODY_WHEN_NFC_IS_ON_YOU_CAN_SEND_OR_RECEIVE_DATA_WHEN_YOUR_DEVICE_TOUCHES_OTHER_NFC_CAPABLE_DEVICES_OR_NFC_TAGS")
#define IDS_NFC_S_BEAM					dgettext(PACKAGE, "IDS_NFC_OPT_S_BEAM")
#define IDS_NFC_S_BEAM_DESCRIPTION_MSG		dgettext(PACKAGE, "IDS_NFC_BODY_WHEN_THIS_FEATURE_IS_TURNED_ON_YOU_CAN_BEAM_FILES_TO_OTHER_NFC_AND_WI_FI_DIRECT_DEVICES_BY_HOLDING_THE_DEVICES_CLOSE_TOGETHER_HELP_MSG")
#define IDS_PREDEFINED_ITEM				dgettext(PACKAGE, "IDS_NFC_BODY_PREDEFINED_ITEM_ABB")
#define IDS_PREDEFINED_ITEM_DESCRIPTION_MSG		dgettext(PACKAGE, "IDS_NFC_BODY_YOU_CAN_BEAM_PREDEFINED_ITEMS_TO_ANOTHER_NFC_DEVICE_JUST_BRING_THE_DEVICES_CLOSE_TOGETHER_MSG")
#define IDS_SECURITY_POLICY_RESTRICTS_USE_OF_PS	dgettext(PACKAGE, "IDS_COM_BODY_SECURITY_POLICY_RESTRICTS_USE_OF_PS")

#define IDS_NONE					dgettext("sys_string", "IDS_COM_BODY_NONE")
#define IDS_CLOSE					dgettext("sys_string", "IDS_COM_POP_CLOSE")

/* will be deleted */
#define IDS_NFC_SERVICE_IS_NOT_SUPPORTED		_("NFC service is not supported")

typedef enum _NFCSettingType {
	NFC_ON_OFF = 0,
	NFC_PREDEFINED_ITEM,
	NFC_SETTING_MAX
} NFCSettingType;

enum
{
	UG_FONT_LIST = 0,
	UG_FONT_SBEAM_TITLE,
	UG_FONT_LIVEBOX,
	UG_FONT_HELP,
	UG_FONT_MAX,
};

enum
{
	UG_ALIGN_LEFT,
	UG_ALIGN_CENTER,
	UG_ALIGN_RIGHT,
};

typedef struct _ugdata_t
{
	Evas_Object* ug_win_main;
	ui_gadget_h nfc_setting_ug;
	Evas_Object* base_layout;
	Evas_Object *bg;

	Evas_Object* base_naviframe;
	Elm_Object_Item *base_navi_it;
}ugdata_t;

typedef struct _gl_item_data_t
{
	NFCSettingType type;
	ugdata_t *data;
}gl_item_data;


/************************** API **************************/
void _change_nfc_predefined_item_setting(void *data);
bool _ug_nfc_check_predefined_item_available(void);
Evas_Object *_ug_nfc_create_predefined_item_list(void *data);
void _ug_nfc_destroy_predefined_item_list(void *data, Evas_Object *obj, void *event_info);


#ifdef __cplusplus
}
#endif

#endif /*__UG_NFC_SETTING_MAIN_H__*/
