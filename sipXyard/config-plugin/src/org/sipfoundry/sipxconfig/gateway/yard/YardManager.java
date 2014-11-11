/**
 *
 *
 * Copyright (c) 2014 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.gateway.yard;

import org.sipfoundry.sipxconfig.address.AddressType;

public interface YardManager {
    public static final AddressType WS_SIP_ADDRESS = new AddressType("yardWsSipPort", 5065, AddressType.Protocol.tcp_udp);
    public static final AddressType BRIDGE_ESL_ADDRESS = new AddressType("yardBridgeEslPort", 11020);
    public static final AddressType SWITCH_ESL_ADDRESS = new AddressType("yardSwitchEslPort", 11000);
    public static final AddressType TCP_UDP_ADDRESS = new AddressType("yardTcpUdpPort", 35080, AddressType.Protocol.tcp_udp);
    public static final AddressType BRIDGE_TCP_UDP_PORT = new AddressType("yardBridgeTcpUdpPort", 35082, AddressType.Protocol.tcp_udp);
    public static final AddressType FS_RTP_RTCP_ADDRESS = new AddressType("yardFreeswitchRtp", 16384, AddressType.Protocol.udp);
    public static final AddressType YARD_WS_PORT = new AddressType("yardWsPort");
    public static final AddressType YARD_HTTP_PORT = new AddressType("yardHttpPort");
    public static final AddressType MONIT_ADDRESS = new AddressType("monit", 38000);
}
