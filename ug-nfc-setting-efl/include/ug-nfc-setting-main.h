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
