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

#include "utl/Instrumentation.h"

static const char* system_tap_queue_enqueue_queue;
static int system_tap_queue_enqueue_eventType;
static int system_tap_queue_enqueue_queueSize;

void system_tap_queue_enqueue(const char* queue, int eventType, int queueSize)
{
  system_tap_queue_enqueue_queue = queue;
  system_tap_queue_enqueue_eventType = eventType;
  system_tap_queue_enqueue_queueSize = queueSize;
}

static const char* system_tap_queue_dequeue_queue;
static int system_tap_queue_dequeue_eventType;
static int system_tap_queue_dequeue_queueSize;

void system_tap_queue_dequeue(const char* queue, int eventType, int queueSize)
{
  system_tap_queue_dequeue_queue = queue;
  system_tap_queue_dequeue_eventType = eventType;
  system_tap_queue_dequeue_queueSize = queueSize;
}

static int system_tap_timer_create_expireTime;
void system_tap_timer_create(int expireTime)
{
  system_tap_timer_create_expireTime = expireTime;
}

static int system_tap_timer_fire_precision;
static int system_tap_timer_fire_precision_overheadUseq;
static int system_tap_timer_fire_expireTime;
void system_tap_timer_fire(int expireTime, int precision, int overheadUseq)
{
  system_tap_timer_fire_expireTime = expireTime;
  system_tap_timer_fire_precision = precision;
  system_tap_timer_fire_precision_overheadUseq = overheadUseq;
}

static int system_tap_timer_destroy_flag;
void system_tap_timer_destroy()
{
  system_tap_timer_destroy_flag = true;
}

static intptr_t system_tap_object_created_pointerAddress;
static const char* system_tap_object_created_className;
void system_tap_object_created(intptr_t pointerAddress, const char* className)
{
  system_tap_object_created_pointerAddress = pointerAddress;
  system_tap_object_created_className = className;
}

static intptr_t system_tap_object_destroyed_pointerAddress;
static const char* system_tap_object_destroyed_className;
void system_tap_object_destroyed(intptr_t pointerAddress, const char* className)
{
  system_tap_object_destroyed_pointerAddress = pointerAddress;
  system_tap_object_destroyed_className = className;
}

