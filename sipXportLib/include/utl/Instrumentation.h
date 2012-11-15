/*
 *
 *
 * Copyright (c) eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 */

#ifndef INSTRUMENTATION_H_SIPXPORTLIB
#define	INSTRUMENTATION_H_SIPXPORTLIB

#include <stdint.h>

bool system_tap_start_portlib_instrumentation(bool enableTrace);
bool system_tap_stop_portlib_instrumentation();

void system_tap_queue_enqueue(const char* queue, int eventType, int queueSize);
void system_tap_queue_dequeue(const char* queue, int eventType, int queueSize);
void system_tap_timer_create(int expireTime);
void system_tap_timer_destroy();
void system_tap_timer_fire(int expireTime, int precision, int overheadUseq);
void system_tap_object_created(intptr_t pointerAddress, const char* className);
void system_tap_object_destroyed(intptr_t pointerAddress, const char* className);
void system_tap_report_object_count(long created, long deleted);
void system_tap_report_object_trace(intptr_t pointerAddress, const char* trace);

#endif	/* INSTRUMENTATION_H_SIPXPORTLIB */

