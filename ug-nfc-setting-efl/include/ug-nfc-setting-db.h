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


#ifndef __UG_NFC_SETTING_DB_H__
#define __UG_NFC_SETTING_DB_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <sqlite3.h>

/************************** API **************************/
int _ug_nfc_setting_db_open(void);
int _ug_nfc_setting_db_close(void);
int _ug_nfc_setting_db_open_table(int *table_handle);
int _ug_nfc_setting_db_get_count(int *count);
int _ug_nfc_setting_db_get_next_record(int table_handle,
					char **app_id, char **pkgName,
					char **iconPath, char **msgDes);
int _ug_nfc_setting_db_get_pkgName(char *app_id, char **pkgName);
int _ug_nfc_setting_db_close_table(int table_handle);


#ifdef __cplusplus
}
#endif

#endif /*__UG_NFC_SETTING_DB_H__*/

