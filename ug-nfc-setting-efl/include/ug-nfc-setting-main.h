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


#ifndef __UG_NFC_SETTING_MAIN_H__
#define __UG_NFC_SETTING_MAIN_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <ui-gadget-module.h>
#include <Evas.h>
#include <Elementary.h>
#include <dlog.h>
#include <nfc.h>

#define PACKAGE "ug-setting-nfc-efl"
#ifdef PREFIX
#undef PREFIX
#endif
#define PREFIX "/opt/ug/"
#define EDJ_PATH 		PREFIX"/res/edje/"PACKAGE
#define EDJ_FILE			EDJ_PATH"/ug-nfc-setting-efl.edj"
#define LOCALE_PATH		PREFIX"/res/locale"

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "UG_NFC_SETTING_EFL"

#define NFCUG_TEXT_DOMAIN		PACKAGE
#define NFCUG_LOCALEDIR		LOCALE_PATH

#define POPUP_TEXT_SIZE	1024

/* Registered string in STMS NFC */
#define IDS_NFC_NFC					dgettext(PACKAGE, "IDS_NFC_BODY_NFC")
#define IDS_NFC_S_BEAM					dgettext(PACKAGE, "IDS_NFC_OPT_S_BEAM")
#define IDS_NFC_S_BEAM_DESCRIPTION_MSG_CHN		dgettext(PACKAGE, "IDS_NFC_BODY_S_BEAM_DESCRIPTION_MSG_CHN")
#define IDS_CLOSE					dgettext("sys_string", "IDS_COM_POP_CLOSE")

#define IDS_NFC_SERVICE_IS_NOT_SUPPORTED		_("NFC service is not supported")

typedef enum _NFCSettingType {
	NFC_ON_OFF = 0,
#ifdef _SBEAM_SUPPORT_
	NFC_S_BEAM,
#endif
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

#ifdef __cplusplus
}
#endif

#endif /*__UG_NFC_SETTING_MAIN_H__*/
