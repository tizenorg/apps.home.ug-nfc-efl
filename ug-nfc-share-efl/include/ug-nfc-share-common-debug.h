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


#ifndef __UG_NFC_SHARE_DEBUG_H__
#define __UG_NFC_SHARE_DEBUG_H__

#define COLOR_RED 		"\033[0;31m"
#define COLOR_GREEN 	"\033[0;32m"
#define COLOR_BROWN 	"\033[0;33m"
#define COLOR_BLUE 		"\033[0;34m"
#define COLOR_PURPLE 	"\033[0;35m"
#define COLOR_CYAN 		"\033[0;36m"
#define COLOR_LIGHTBLUE "\033[0;37m"
#define COLOR_END		"\033[0;m"

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "UG_SHARE_NFC_EFL"

#define UG_NFC_SHARE_ENABLE_DLOG

#ifdef UG_NFC_SHARE_ENABLE_DLOG
#define UG_NFC_SHARE_DEBUG(fmt, ...)\
	do { \
		LOGD(COLOR_BROWN" " fmt COLOR_END,##__VA_ARGS__);\
	} while (0)
#define UG_NFC_SHARE_DEBUG_ERR(fmt, ...)\
	do { \
		LOGE(COLOR_RED" " fmt COLOR_END,##__VA_ARGS__);\
	} while (0)
#define UG_NFC_SHARE_BEGIN() \
	do { \
		LOGD(COLOR_BLUE"BEGIN >>>>"COLOR_END);\
	} while (0)
#define UG_NFC_SHARE_END() \
	do { \
		LOGD(COLOR_BLUE"END <<<<"COLOR_END);\
	} while (0)
#else
#define UG_NFC_SHARE_DEBUG(fmt, ...) \
	do { \
		printf("\n [%s: %s: %s(): %d] " fmt"\n",  APPNAME, rindex(__FILE__, '/')+1, __FUNCTION__, __LINE__, ##__VA_ARGS__);\
	} while (0)
#define UG_NFC_SHARE_BEGIN() \
	do { \
		printf("\n [%s: %s: %d] : BEGIN >>>> %s() \n", APPNAME, rindex(__FILE__, '/')+1,  __LINE__ , __FUNCTION__);\
	} while (0)
#define UG_NFC_SHARE_END() \
	do { \
		printf("\n [%s: %s: %d]: END   <<<< %s()\n", APPNAME, rindex(__FILE__, '/')+1,  __LINE__ , __FUNCTION__); \
	} while (0)
#endif

#define ret_if(expr) \
	do { \
		if(expr) { \
			UG_NFC_SHARE_DEBUG_ERR("(%s) ", #expr); \
			UG_NFC_SHARE_END();\
			return; \
		} \
	} while (0)
#define retm_if(expr, fmt, arg...) \
	do { \
		if(expr) { \
			UG_NFC_SHARE_DEBUG_ERR("(%s) "fmt, #expr, ##arg); \
			UG_NFC_SHARE_END();\
			return; \
		} \
	} while (0)
#define retv_if(expr, val) \
	do { \
		if(expr) { \
			UG_NFC_SHARE_DEBUG_ERR("(%s) ", #expr); \
			UG_NFC_SHARE_END();\
			return (val); \
		} \
	} while (0)
#define retvm_if(expr, val, fmt, arg...) \
	do { \
		if(expr) { \
			UG_NFC_SHARE_DEBUG_ERR("(%s) "fmt, #expr, ##arg); \
			UG_NFC_SHARE_END();\
			return (val); \
		} \
	} while (0)

#endif /*__UG_NFC_SHARE_DEBUG_H__*/

