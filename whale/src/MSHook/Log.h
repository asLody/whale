/* Cydia Substrate - Powerful Code Insertion Platform
 * Copyright (C) 2008-2011  Jay Freeman (saurik)
*/

/* GNU Lesser General Public License, Version 3 {{{ */
/*
 * Substrate is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * Substrate is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Substrate.  If not, see <http://www.gnu.org/licenses/>.
**/
/* }}} */

#ifndef SUBSTRATE_LOG_HPP
#define SUBSTRATE_LOG_HPP

#include <android/log.h>

#define MSLogLevelNotice ANDROID_LOG_INFO
#define MSLogLevelWarning ANDROID_LOG_WARN
#define MSLogLevelError ANDROID_LOG_ERROR

#define DEBUG 1
#define EXE_PRINTF 0
#ifndef LOG_TAG
	# define LOG_TAG "Native_X"
#endif

#if DEBUG
	#ifdef EXE_PRINTF
		#define LOGD(fmt,...) printf("[%12s] " fmt "\n", __FUNCTION__,##__VA_ARGS__)
		#define LOGI(fmt,...) printf("[%12s] " fmt "\n", __FUNCTION__,##__VA_ARGS__)
		#define LOGV(fmt,...) printf("[%12s] " fmt "\n", __FUNCTION__,##__VA_ARGS__)
		#define LOGW(fmt,...) printf("[%12s] " fmt "\n", __FUNCTION__,##__VA_ARGS__)
		#define LOGE(fmt,...) printf("[%12s] " fmt "\n", __FUNCTION__,##__VA_ARGS__)
		#define LOGF(fmt,...) printf("[%12s] " fmt "\n", __FUNCTION__,##__VA_ARGS__)

	#else
		#define LOGD(fmt,...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "[%s]" fmt, __FUNCTION__,##__VA_ARGS__)
		#define LOGI(fmt,...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, "[%s]" fmt, __FUNCTION__,##__VA_ARGS__)
		#define LOGV(fmt,...) __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, "[%s]" fmt, __FUNCTION__,##__VA_ARGS__)
		#define LOGW(fmt,...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, "[%s]" fmt, __FUNCTION__,##__VA_ARGS__)
		#define LOGE(fmt,...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, "[%s]" fmt, __FUNCTION__,##__VA_ARGS__)
		#define LOGF(fmt,...) __android_log_print(ANDROID_LOG_FATAL, LOG_TAG, "[%s]" fmt, __FUNCTION__,##__VA_ARGS__)
	#endif
#else
	#define LOGD(...) while(0){}
	#define LOGI(...) while(0){}
	#define LOGV(...) while(0){}
	#define LOGW(...) while(0){}
	#define LOGE(...) while(0){}
	#define LOGW(...) while(0){}
#endif

#define MSLog(level, fmt,...) do { \
	printf("[%12s] " fmt "\n", __FUNCTION__,##__VA_ARGS__); \
    __android_log_print(level, LOG_TAG, "[%s]" fmt, __FUNCTION__,##__VA_ARGS__); \
} while (false)
#endif//SUBSTRATE_LOG_HPP
