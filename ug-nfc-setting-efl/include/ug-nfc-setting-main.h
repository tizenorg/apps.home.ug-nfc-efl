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
#define LOCALE_PATH		PREFIX"/res/locale"

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "UG_NFC_SETTING_EFL"

#define NFCUG_TEXT_DOMAIN		PACKAGE
#define NFCUG_LOCALEDIR		LOCALE_PATH

/* Registered string in STMS NFC */
#define IDS_ACTIVATION		dgettext(PACKAGE, "IDS_NFC_POP_ACTIVATION")
#define IDS_NFC				dgettext(PACKAGE, "IDS_NFC_BODY_NFC")


typedef struct _ugdata_t
{
	Evas_Object* ug_win_main;
	struct ui_gadget *nfc_setting_ug;
	Evas_Object* base_layout;
	Evas_Object *bg;
	Evas_Object *check;

	Evas_Object* base_naviframe;
	Elm_Object_Item *base_navi_it;
}ugdata_t;

#ifdef __cplusplus
}
#endif

#endif /*__UG_NFC_SETTING_MAIN_H__*/
