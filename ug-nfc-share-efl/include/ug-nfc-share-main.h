/*
 * Copyright (c) 2000 - 2012 Samsung Electronics Co., Ltd All Rights Reserved
 *
 * This file is part of the ug-nfc-efl
 * Written by Junyong Sim <junyong.sim@samsung.com>
 *
 * PROPRIETARY/CONFIDENTIAL
 *
 * This software is the confidential and proprietary information of SAMSUNG ELECTRONICS ("Confidential Information").
 * You shall not disclose such Confidential Information and shall use it only in accordance
 * with the terms of the license agreement you entered into with SAMSUNG ELECTRONICS.
 * SAMSUNG make no representations or warranties about the suitability of the software,
 * either express or implied, including but not limited to the implied warranties of merchantability,
 * fitness for a particular purpose, or non-infringement.
 * SAMSUNG shall not be liable for any damages suffered by licensee as a result of using,
 * modifying or distributing this software or its derivatives.
 *
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
#include <appsvc.h>

#include <appcore-efl.h>
#include <Ecore_X.h>
#include <utilX.h>

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
#define EDJ_FILE        	EDJ_PATH"/ug-nfc-share-efl.edj"
#define IMAGES_PATH 	PREFIX"/res/images/"PACKAGE
#define LOCALE_PATH		PREFIX"/res/locale"

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "UG_NFC_SHARE_EFL"

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
#define IDS_PS_SELECTED										dgettext(PACKAGE, "IDS_NFC_BODY_PS_SELECTED")
#define IDS_SHARE_VIA_NFC									dgettext(PACKAGE, "IDS_NFC_HEADER_SHARE_VIA_NFC")
#define IDS_GENTLY_TOUCH_PHONES_TOGETHER_TO_SHARE		dgettext(PACKAGE, "IDS_NFC_BODY_GENTLY_TOUCH_PHONES_TOGETHER_TO_SHARE")
#define IDS_TO_CANCEL_SHARING_TAP_CANCEL				dgettext(PACKAGE, "IDS_NFC_BODY_TO_CANCEL_SHARING_TAP_CANCEL")
#define IDS_PS_TAG_SHARED									dgettext(PACKAGE, "IDS_NFC_POP_PS_TAG_SHARED")
#define IDS_FAILED_TO_SHARE_TAG							dgettext(PACKAGE, "IDS_NFC_POP_FAILED_TO_SHARE_TAG")

#define IDS_HANDOVER	_("HANDOVER")
#define IDS_TRANSFER_FAILED								dgettext(PACKAGE, "IDS_NFC_POP_TRANSFER_FAILED")
#define IDS_UNABLE_TO_SHARE_FILES_MAXIMUM_FILE_SIZE_EXCEEDED_SHARE_FILES_VIA_BLUETOOTH_PAIRED_DEVICES_MAY_BE_DISCONNECTED_CONTINUE_Q \
	_("File size exceeds allowed limit. Share files via Bluetooth. Continue?")
#define IDS_SERVICE_NOT_AVAILABLE_NFC_TURNED_OFF_TURN_ON_NFC_Q		_("Service not available. NFC turned off. Turn on NFC?")

/* System string */
#define IDS_YES				dgettext("sys_string", "IDS_COM_SK_YES")
#define IDS_NO				dgettext("sys_string", "IDS_COM_SK_NO")
#define IDS_OK			dgettext("sys_string", "IDS_COM_SK_OK")
#define IDS_NONE		dgettext("sys_string", "IDS_COM_BODY_NONE")
#define IDS_UNKNOWN	dgettext("sys_string", "IDS_COM_BODY_UNKNOWN")
#define IDS_CANCEL		dgettext("sys_string", "IDS_COM_SK_CANCEL")

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


typedef struct _ugdata_t
{
	Evas_Object* ug_win_main;
	struct ui_gadget *nfc_share_ug;
	Evas_Object* base_layout;
	Evas_Object *bg;

	Evas_Object* base_naviframe;
	Elm_Object_Item *base_navi_it;

	nfc_ndef_message_h current_ndef;
	char *uri;
}ugdata_t;


#ifdef __cplusplus
}
#endif

#endif /*__UG_NFC_SHARE_MAIN_H__*/
