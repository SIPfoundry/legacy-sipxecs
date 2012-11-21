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

#ifndef INSTRUMENTATION_H_SIPXTACKLIB
#define	INSTRUMENTATION_H_SIPXTACKLIB

#include <stdint.h>

void system_tap_sip_rx(
  const char* src,
  unsigned short srcPort,
  const char* dst,
  unsigned short dstPort,
  const char* sipMessage,
  int size);


void system_tap_sip_tx(
  const char* src,
  unsigned short srcPort,
  const char* dst,
  unsigned short dstPort,
  const char* sipMessage,
  int size);

void system_tap_sip_msg_created(intptr_t pointerAddress);
void system_tap_sip_msg_destroyed(intptr_t pointerAddress);

#endif	/* INSTRUMENTATION_H_SIPXTACKLIB */

