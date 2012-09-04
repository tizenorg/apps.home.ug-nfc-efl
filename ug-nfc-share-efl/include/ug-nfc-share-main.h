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


#ifndef __UG_NFC_SHARE_MAIN_H__
#define __UG_NFC_SHARE_MAIN_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <ui-gadget-module.h>
#include <Evas.h>
#include <Elementary.h>
#include <dlog.h>
#include <stdbool.h>
#include <string.h>

#include <Ecore_X.h>
#include <utilX.h>
#include <bundle.h>
#include <assert.h>
#include <glib.h>

#include <nfc.h>

#include "ug-nfc-share-common-error.h"
#include "ug-nfc-share-common-debug.h"
#include "ug-nfc-share-common-util.h"

#define PACKAGE "ug-share-nfc-efl"
#ifdef PREFIX
#undef PREFIX
#endif
#define PREFIX "/opt/ug/"
#define EDJ_PATH 		PREFIX"/res/edje/"PACKAGE
#define EDJ_FILE        		EDJ_PATH"/ug-nfc-share-efl.edj"
#define IMAGES_PATH 		PREFIX"/res/images/"PACKAGE
#define LOCALE_PATH		PREFIX"/res/locale"

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "UG_NFC_SHARE_EFL"

#define NFC_SHARE_SERVICE			"com.samsung.nfc-share-service"
#define NFC_SHARE_SERVICE_SHARE_VIA_UG		"http://tizen.org/appcontrol/operation/nfc_share_via_ug"

#define GET_SAFE_STRING(x) ((x) != NULL ? (x) : "NULL")
#define STRING_AND_SIZE(x) (x), strlen((x))
#define NULL_TO_EMPTY_STRING(x) (((x) != NULL) ? (x) : "")

#define UG_NFC_SHARE_HANDOVER_FILE_SIZE      (10 * 1024) /* 10kB */
#define UG_NFC_SHARE_DB_MAX_ROW_COUNT        100


/* Buffer length for Text */
#define POPUP_TEXT_SIZE	1024
#define LABEL_TEXT_SIZE		1024

#define NFCUG_TEXT_DOMAIN		PACKAGE
#define NFCUG_LOCALEDIR		LOCALE_PATH

/* Registered string in STMS NFC */
#define IDS_SHARE_VIA_NFC							dgettext(PACKAGE, "IDS_NFC_HEADER_SHARE_VIA_NFC")
#define IDS_GENTLY_TOUCH_PHONES_TOGETHER_TO_SHARE				dgettext(PACKAGE, "IDS_NFC_BODY_GENTLY_TOUCH_PHONES_TOGETHER_TO_SHARE")
#define IDS_TO_CANCEL_SHARING_TAP_CANCEL					dgettext(PACKAGE, "IDS_NFC_BODY_TO_CANCEL_SHARING_TAP_CANCEL")
#define IDS_TRANSFER_FAILED							dgettext(PACKAGE, "IDS_NFC_POP_TRANSFER_FAILED")
#define IDS_SERVICE_NOT_AVAILABLE_NFC_TURNED_OFF_TURN_ON_NFC_Q		dgettext(PACKAGE, "IDS_NFC_POP_SERVICE_NOT_AVAILABLE_NFC_TURNED_OFF_TURN_ON_NFC_Q")
//#define IDS_TAG_SHARED								dgettext(PACKAGE, "IDS_NFC_POP_TAG_SHARED")
#define IDS_TAG_SHARED								_("Shared")
//#define IDS_FAILED_TO_SHARE_TAG						dgettext(PACKAGE, "IDS_NFC_POP_FAILED_TO_SHARE_TAG")
#define IDS_FAILED_TO_SHARE_TAG						_("Failed")

#define IDS_HANDOVER	_("HANDOVER")
#define IDS_NFC_SERVICE_IS_NOT_SUPPORTED		_("NFC service is not supported")

/* System string */
#define IDS_YES				dgettext("sys_string", "IDS_COM_SK_YES")
#define IDS_NO				dgettext("sys_string", "IDS_COM_SK_NO")
#define IDS_OK				dgettext("sys_string", "IDS_COM_SK_OK")
#define IDS_NONE			dgettext("sys_string", "IDS_COM_BODY_NONE")
#define IDS_UNKNOWN			dgettext("sys_string", "IDS_COM_BODY_UNKNOWN")
#define IDS_CANCEL			dgettext("sys_string", "IDS_COM_SK_CANCEL")
#define IDS_CLOSE			dgettext("sys_string", "IDS_COM_POP_CLOSE")

/**** common lib functions endzz  **********/

typedef enum
{
	UG_NFC_SHARE_TAG_CONTACT = 0,
	UG_NFC_SHARE_TAG_URL,
	UG_NFC_SHARE_TAG_MPLAYER,
	UG_NFC_SHARE_TAG_MEMO,
	UG_NFC_SHARE_TAG_BT,
	UG_NFC_SHARE_TAG_WIFI,
	UG_NFC_SHARE_TAG_PROFILE,
	UG_NFC_SHARE_TAG_ALLSHARE,
	UG_NFC_SHARE_TAG_FILE,
	UG_NFC_SHARE_TAG_HANDOVER,
	UG_NFC_SHARE_TAG_UNKNOWN,
	UG_NFC_SHARE_TAG_MAX
} ug_nfc_share_tag_type;

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
	ui_gadget_h nfc_share_ug;
	Evas_Object* base_layout;
	Evas_Object *bg;

	nfc_ndef_message_h current_ndef;
	bundle *bd;
	Evas_Object* base_naviframe;
	Elm_Object_Item *base_navi_it;
}ugdata_t;


#ifdef __cplusplus
}
#endif

#endif /*__UG_NFC_SHARE_MAIN_H__*/
