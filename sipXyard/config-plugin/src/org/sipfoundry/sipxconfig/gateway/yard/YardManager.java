package org.sipfoundry.sipxconfig.gateway.yard;

import org.sipfoundry.sipxconfig.address.AddressType;

public interface YardManager {
    public static final AddressType WS_SIP_ADDRESS = new AddressType("yardWsSipPort", 5065);
    public static final AddressType BRIDGE_ESL_ADDRESS = new AddressType("yardBridgeEslPort", 11020);
    public static final AddressType SWITCH_ESL_ADDRESS = new AddressType("yardSwitchEslPort", 11000);
    public static final AddressType TCP_UDP_ADDRESS = new AddressType("yardTcpUdpPort", 35080);
    public static final AddressType BRIDGE_TCP_UDP_PORT = new AddressType("yardBridgeTcpUdpPort", 35082);
    public static final AddressType FS_RTP_RTCP_ADDRESS = new AddressType("yardFreeswitchRtp", 16384);
    public static final AddressType YARD_WS_PORT = new AddressType("yardWsPort");
    public static final AddressType YARD_HTTP_PORT = new AddressType("yardHttpPort");
}
