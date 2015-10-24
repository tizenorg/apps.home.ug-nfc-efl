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
#include <app_manager.h>
#include <app_info.h>
#include <glib.h>



#define LOCALE_PATH	LOCALEDIR
#define ICON_PATH	ICONDIR
#define EDJ_PATH	EDJDIR
#define EDJ_FILE	EDJ_PATH"/"PACKAGE".edj"

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "UG_NFC_SETTING"

#define NFCUG_TEXT_DOMAIN	PACKAGE
#define NFCUG_LOCALEDIR	LOCALE_PATH

#define POPUP_TEXT_SIZE	1024


/* Registered string in STMS NFC */
#define IDS_NFC_NFC	\
	dgettext(PACKAGE, "IDS_NFC_BODY_NFC")
#define IDS_NFC_DESCRIPTION_MSG	\
	dgettext(PACKAGE, "IDS_NFC_BODY_WHEN_NFC_IS_ON_YOU_CAN_SEND_OR_RECEIVE_DATA_WHEN_YOUR_DEVICE_TOUCHES_OTHER_NFC_CAPABLE_DEVICES_OR_NFC_TAGS")
#define IDS_NFC_S_BEAM	\
	dgettext(PACKAGE, "IDS_NFC_OPT_S_BEAM")
#define IDS_USE_NFC_IN_HOME	\
	dgettext(PACKAGE, "IDS_NFC_OPT_USE_NFC_IN_HOME_ABB")
#define IDS_NFC_TURNED_ON	\
	dgettext(PACKAGE, "IDS_NFC_BODY_NFC_TURNED_ON")
#define IDS_NFC_SECURE_STORAGE_HEADER	\
	dgettext(PACKAGE, "IDS_NFC_HEADER_NFC_SECURE_STORAGE_ABB")
#define IDS_NFC_SECURE_STORAGE_BODY	\
	dgettext(PACKAGE, "IDS_NFC_MBODY_NFC_SECURE_STORAGE")
#define IDS_NFC_SECURE_STORAGE_ITEM_MAN	\
	dgettext(PACKAGE, "IDS_NFC_OPT_MANUAL")
#define IDS_NFC_SECURE_STORAGE_ITEM_AUTO	\
	dgettext(PACKAGE, "IDS_NFC_OPT_AUTOMATIC")
#define IDS_NFC_DO_NOT_SHOW_AGAIN	\
	dgettext(PACKAGE, "IDS_NFC_BODY_DO_NOT_SHOW_AGAIN_ABB2")
#define IDS_FAILED_TO_TURN_ON_NFC	\
	dgettext(PACKAGE, "IDS_NFC_HEADER_FAILED_TO_TURN_ON_NFC_ABB")
#define IDS_AN_ERROR_OCCURRED_WHILE_TURNING_ON_NFC_TRY_AGAIN	\
	dgettext(PACKAGE, "IDS_NFC_POP_AN_ERROR_OCCURRED_WHILE_TURNING_ON_NFC_TRY_AGAIN")

#define IDS_SECURITY_POLICY_RESTRICTS_USE_OF_PS	\
	dgettext(PACKAGE, "IDS_COM_BODY_SECURITY_POLICY_RESTRICTS_USE_OF_PS")
#define IDS_NONE	\
	dgettext(PACKAGE, "IDS_COM_BODY_NONE")
#define IDS_CLOSE	\
	dgettext(PACKAGE, "IDS_COM_POP_CLOSE")
#define IDS_OK	\
	dgettext(PACKAGE, "IDS_COM_SK_OK")
#define IDS_NFC_SERVICE_IS_NOT_SUPPORTED	\
	dgettext(PACKAGE, "IDS_TAGS_POP_NFC_SERVICE_IS_NOT_SUPPORTED")

#define IDS_NFC_SECURE_STORAGE_ITEM_MAN_DSC	\
	dgettext(PACKAGE, "IDS_ST_BODY_USE_YOUR_SIM_CARD_FOR_SECURE_STORAGE")
#define IDS_NFC_SECURE_STORAGE_ITEM_AUTO_DSC	\
	dgettext(PACKAGE, "IDS_ST_BODY_SWITCH_AUTOMATICALLY_BETWEEN_USING_YOUR_SIM_CARD_AND_THE_ON_DEVICE_SECURE_STORAGE")
#define IDS_NFC_SECURE_STORAGE_TIPS	\
	dgettext(PACKAGE, "IDS_ST_BODY_SELECTING_MANUAL_WILL_DISABLE_SOME_SECURE_FEATURES_SUCH_AS_NFC_PAYMENT_THAT_STORE_DATA_IN_THE_ON_DEVICE_SECURE_STORAGE")

/* The following texts will be used by NFC Manager */
// IDS_NFC_TPOP_TAG_TYPE_NOT_SUPPORTED


typedef enum {
	MENU_NFC = 0,
	MENU_MAX
} NFCSettingMenuType;

typedef enum {
	NFC_PREDEFINED_ITEM = 0,
	NFC_SECURE_STORAGE,
	NFC_SECURE_STORAGE_MAN,
	NFC_SECURE_STORAGE_MAN_DES,
	NFC_SECURE_STORAGE_AUTO,
	NFC_SECURE_STORAGE_AUTO_DES,
	NFC_SETTING_MAX
} NFCSettingType;

enum {
	UG_FONT_LIST = 0,
	UG_FONT_SBEAM_TITLE,
	UG_FONT_LIVEBOX,
	UG_FONT_MAX,
};

enum {
	UG_ALIGN_LEFT,
	UG_ALIGN_CENTER,
	UG_ALIGN_RIGHT,
};

typedef struct {
	Evas_Object* ug_win_main;
	ui_gadget_h nfc_setting_ug;
	Evas_Object* base_layout;
	Evas_Object *bg;
	Evas_Object* ns_genlist;
	Evas_Object* pd_genlist;
	Evas_Object* ss_genlist;
	Evas_Object* base_naviframe;
	Evas_Object* ns_on_off;
	Evas_Object* pd_on_off;
	Evas_Object* popup;

	Elm_Object_Item *base_navi_it;

	NFCSettingMenuType menu_type;
	guint h_ctxpop_res_t;
	guint h_auto_t;
	guint onoff_comp_t;
	Evas_Object* more_bt;
	Evas_Object* ctxpopup; /* for more button */

	app_control_h service;
}ugdata_t;

typedef struct {
	NFCSettingType type;
	ugdata_t *data;
} gl_item_data;

/************************** API **************************/
void _ug_nfc_secure_storage_create(void *data);

#ifdef __cplusplus
}
#endif

#endif /*__UG_NFC_SETTING_MAIN_H__*/
