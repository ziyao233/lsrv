/*
 *	lsrv
 *	File:/src/Debug.h
 *	Date:2022.08.30
 *	By MIT License.
 *	Copyright (c) 2022 Ziyao.
 */

#ifndef __LSRV_DEBUG_H__
#define __LSRV_DEBUG_H__

#ifdef __LSRV_DEBUG__

#include<assert.h>
#define lsrv_assert(cond) assert(cond)

#else

#define lsrv_assert(cond)

#endif

#endif	// __LSRV_DEBUG_H__
