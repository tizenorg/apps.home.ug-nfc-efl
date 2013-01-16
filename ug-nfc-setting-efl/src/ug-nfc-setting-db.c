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


#include "ug-nfc-setting-main.h"
#include "ug-nfc-setting-db.h"


#define DB_FILE_PATH			"/opt/usr/dbspace/.net-nfcpush.db"
#define DB_TABLE_NAME			"NfcPushMsgTable"

/* sqlite> .schema 		*/
/* CREATE TABLE NfcPushMsgTable ( AppId TEXT PRIMARY KEY, PkgName TEXT, IconPath TEXT, MsgDesc TEXT ); */

#define DB_QUERY_LEN			512

static sqlite3 *g_predefined_item_db;
static sqlite3_stmt *g_predefined_item_pstmt;


static void _db_finalize_statement(void)
{
	LOGD("BEGIN >>>>");

	int db_ret = SQLITE_OK;

	if (g_predefined_item_pstmt != NULL)
	{
		db_ret = sqlite3_finalize(g_predefined_item_pstmt);
		g_predefined_item_pstmt = NULL;

		if(db_ret != SQLITE_OK)
		{
			LOGD("Failed to sqlite3_finalize error[%d]", db_ret);
		}
	}

	LOGD("END <<<<");
}


int _ug_nfc_setting_db_open(void)
{
	LOGD("BEGIN >>>>");

	int db_ret = SQLITE_OK;

	db_ret = sqlite3_open(DB_FILE_PATH, &g_predefined_item_db);
	if(db_ret != SQLITE_OK)
	{
		LOGD("Failed to open database. error[%d]", db_ret);
	}

	LOGD("END <<<<");

	return db_ret;
}

int _ug_nfc_setting_db_close(void)
{
	LOGD("BEGIN >>>>");

	int db_ret = SQLITE_OK;

	if (g_predefined_item_db != NULL)
	{
		db_ret = sqlite3_close(g_predefined_item_db);
		if(db_ret != SQLITE_OK)
		{
			LOGD("Failed to close database. error[%d]", db_ret);
			return db_ret;
		}
		g_predefined_item_db = NULL;
	}

	LOGD("END <<<<");

	return db_ret;
}

int _ug_nfc_setting_db_open_table(int *table_handle)
{
	LOGD("BEGIN >>>>");

	int db_ret = SQLITE_OK;
	int count = 0;

	/* get num_of_index */
	_ug_nfc_setting_db_get_count(&count);

	if (count > 0)
	{
		char query[DB_QUERY_LEN] = { 0, };
		sqlite3_stmt *stmt = NULL;

		snprintf(query, DB_QUERY_LEN, "select * from %s", DB_TABLE_NAME);
		LOGD("query [%s]", query);

		db_ret = sqlite3_prepare_v2(g_predefined_item_db, query, strlen(query), &stmt, NULL);
		if (db_ret != SQLITE_OK)
		{
			LOGD("Failed to sqlite3_prepare_v2 error[%d]", db_ret);
			if (g_predefined_item_pstmt != NULL)
			{
				db_ret = sqlite3_finalize(stmt);
				stmt = NULL;
				if(db_ret != SQLITE_OK)
				{
					LOGD("Failed to sqlite3_finalize error[%d]", db_ret);
				}
			}
			return db_ret;
		}

		*table_handle = (int)stmt;
	}
	else
	{
		LOGD("no item");
		db_ret = SQLITE_ERROR;
	}

	LOGD("END <<<<");

	return db_ret;
}

int _ug_nfc_setting_db_get_count(int *count)
{
	LOGD("BEGIN >>>>");

	int db_ret = SQLITE_OK;
	char query[DB_QUERY_LEN] = { 0, };

	snprintf(query, DB_QUERY_LEN,
			"select count(AppId) from %s ",
			DB_TABLE_NAME);
	LOGD("query [%s]", query);

	db_ret = sqlite3_prepare_v2(g_predefined_item_db, query, strlen(query), &g_predefined_item_pstmt, NULL);
	if (db_ret != SQLITE_OK)
	{
		LOGD("Failed to sqlite3_prepare_v2 error[%d]", db_ret);
		_db_finalize_statement();
		return db_ret;
	}

	db_ret = sqlite3_step(g_predefined_item_pstmt);
	if (db_ret != SQLITE_ROW)
	{
		LOGD("Failed to sqlite3_step error[%d]", db_ret);
		_db_finalize_statement();
		return db_ret;
	}

	*count = sqlite3_column_int(g_predefined_item_pstmt, 0);

	_db_finalize_statement();

	LOGD("END <<<<");

	return db_ret;
}

int _ug_nfc_setting_db_get_next_record(int table_handle,
					char **app_id, char **pkgName,
					char **iconPath, char **msgDes)
{
	LOGD("BEGIN >>>>");

	int db_ret = SQLITE_OK;
	sqlite3_stmt *stmt = NULL;

	stmt = (sqlite3_stmt *)table_handle;
	if (stmt == NULL)
	{
		return -1;
	}

	db_ret = sqlite3_step(stmt);
	if (db_ret != SQLITE_DONE)
	{
		LOGD("success to get next record");

		if ((char *)sqlite3_column_text(stmt, 0) != NULL)
			*app_id = strdup((char *)sqlite3_column_text(stmt, 0));
		if ((char *)sqlite3_column_text(stmt, 1) != NULL)
			*pkgName = strdup((char *)sqlite3_column_text(stmt, 1));
		if ((char *)sqlite3_column_text(stmt, 2) != NULL)
			*iconPath = strdup((char *)sqlite3_column_text(stmt, 2));
		if ((char *)sqlite3_column_text(stmt, 3) != NULL)
			*msgDes = strdup((char *)sqlite3_column_text(stmt, 3));
	}

	LOGD("END <<<<");

	return SQLITE_OK;
}

int _ug_nfc_setting_db_get_pkgName(char *app_id, char **pkgName)
{
	LOGD("BEGIN >>>>");

	int db_ret = SQLITE_OK;
	char query[DB_QUERY_LEN] = { 0, };

	snprintf(query, DB_QUERY_LEN,
			"select PkgName from %s where AppId =\"%s\"", DB_TABLE_NAME, app_id);
	LOGD("query [%s]", query);

	db_ret = sqlite3_prepare_v2(g_predefined_item_db, query, strlen(query), &g_predefined_item_pstmt, NULL);
	if (db_ret != SQLITE_OK)
	{
		LOGD("Failed to sqlite3_prepare_v2 error[%d]", db_ret);
		_db_finalize_statement();
		return db_ret;
	}

	db_ret = sqlite3_step(g_predefined_item_pstmt);
	if (db_ret != SQLITE_ROW)
	{
		LOGD("Failed to sqlite3_step error[%d]", db_ret);
		_db_finalize_statement();
		return db_ret;
	}

	*pkgName = strdup((char *)sqlite3_column_text(g_predefined_item_pstmt, 0));

	_db_finalize_statement();

	LOGD("END <<<<");

	return db_ret;
}

int _ug_nfc_setting_db_close_table(int table_handle)
{
	LOGD("BEGIN >>>>");

	int db_ret = SQLITE_OK;
	sqlite3_stmt *stmt = NULL;

	stmt = (sqlite3_stmt *)table_handle;
	if (stmt == NULL)
		return -1;

	db_ret = sqlite3_finalize(stmt);
	if(db_ret != SQLITE_OK)
	{
		LOGD("Failed to sqlite3_finalize error[%d]", db_ret);
	}

	LOGD("END <<<<");

	return db_ret;
}



