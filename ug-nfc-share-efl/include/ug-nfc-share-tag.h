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

#ifndef __UG_NFC_SHARE_TAG_H__
#define __UG_NFC_SHARE_TAG_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "ug-nfc-share-main.h"

#define UG_NFC_SHARE_LAST_FILE_NUMBER_KEY "db/nfc/last_file_number"
#define UG_NFC_SHARE_FILE_NAME_PREFIX _("nfc_receive_file_")

/* get and set function */
char* ug_nfc_share_get_display_str(void);
void ug_nfc_share_set_display_str(const char* text);
ug_nfc_share_tag_type ug_nfc_share_get_tag_type(void);
void ug_nfc_share_set_tag_type(ug_nfc_share_tag_type tag_type);
nfc_ndef_message_h ug_nfc_share_get_current_ndef(void *data);
ug_nfc_share_result_e ug_nfc_share_set_current_ndef(void *data, nfc_ndef_message_h ndef_msg);

/* util function */
char *ug_nfc_share_get_display_text_from_ndef(ug_nfc_share_tag_type record_type, nfc_ndef_message_h ndef_msg);
ug_nfc_share_result_e ug_nfc_share_make_ndef_message_from_file(nfc_ndef_message_h *msg, const char *path);
ug_nfc_share_result_e ug_nfc_share_make_ndef_message_from_multi_file(nfc_ndef_message_h *msg, const char *path[], int record_count); /* nfc_handover */
void ug_nfc_set_nfc_callback(void *user_data);
void ug_nfc_unset_nfc_callback(void);

#ifdef __cplusplus
}
#endif

#endif /*__UG_NFC_SHARE_MAIN_H__*/
