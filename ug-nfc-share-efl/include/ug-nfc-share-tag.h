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


#ifndef __UG_NFC_SHARE_TAG_H__
#define __UG_NFC_SHARE_TAG_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "ug-nfc-share-main.h"

/* get and set function */
#if 0
char* ug_nfc_share_get_display_str(void);
void ug_nfc_share_set_display_str(const char* text);
#endif
ug_nfc_share_tag_type ug_nfc_share_get_tag_type(void);
void ug_nfc_share_set_tag_type(ug_nfc_share_tag_type tag_type);
nfc_ndef_message_h ug_nfc_share_get_current_ndef(void *data);
ug_nfc_share_result_e ug_nfc_share_set_current_ndef(void *data, nfc_ndef_message_h ndef_msg);

/* util function */
#if 0
char *ug_nfc_share_get_display_text_from_ndef(ug_nfc_share_tag_type record_type, nfc_ndef_message_h ndef_msg);
#endif
ug_nfc_share_result_e ug_nfc_share_make_ndef_message_from_file(nfc_ndef_message_h *msg, const char *path);
ug_nfc_share_result_e ug_nfc_share_make_ndef_message_from_multi_file(nfc_ndef_message_h *msg, const char *path[], int record_count); /* nfc_handover */
void ug_nfc_set_nfc_callback(void *user_data);
void ug_nfc_unset_nfc_callback(void);

#ifdef __cplusplus
}
#endif

#endif /*__UG_NFC_SHARE_MAIN_H__*/
