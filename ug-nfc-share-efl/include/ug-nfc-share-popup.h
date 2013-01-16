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

#ifndef __UG_NFC_SHARE_POPUP_H__
#define __UG_NFC_SHARE_POPUP_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "ug-nfc-share-main.h"

typedef void (*UG_NFC_POPUP_USER_RESP_CB)(void *data, Evas_Object *obj, void *event_info);

enum {
	UG_NFC_POPUP_RESP_NONE = -1,
	UG_NFC_POPUP_RESP_TIMEOUT = -2,
	UG_NFC_POPUP_RESP_OK = -3,
	UG_NFC_POPUP_RESP_CANCEL = -4,
	UG_NFC_POPUP_RESP_CLOSE = -5
};


Evas_Object *ug_nfc_share_create_popup(void *data,
                                    Evas_Object *parent_layout,
                                    const char *description,
                                    const char *btn1_text,
                                    int btn1_type,
                                    const char *btn2_text,
                                    int btn2_type,
                                    const char *btn3_text,
                                    int btn3_type,
                                    bool is_alert_type,
                                    bool enable_timeout,
                                    UG_NFC_POPUP_USER_RESP_CB response_cb);
void ug_nfc_share_close_popup(Evas_Object* popup);

#ifdef __cplusplus
}
#endif


#endif /*__UG_NFC_SHARE_POPUP_H__*/

