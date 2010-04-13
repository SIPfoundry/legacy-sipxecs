/**
 * Copyright (C) 2007 - 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.commons.dhcp;

/**
 * [Enter descriptive text here]
 * <p>
 *
 * @author Mardy Marshall
 */
public enum HardwareAddressType {
    ETHERNET(1), // Ethernet
    EXPERIMENTAL_ENET(2), // Experimental Ethernet (3Mb)
    AMATEUR_RADIO(3), // Amateur Radio AX.25
    PRONET(4), // Proteon ProNET Token Ring
    CHAOS(5), // Chaos
    IEEE_802(6), // IEEE 802 Networks
    ARCNET(7), // ARCNET
    HYPERCHANNEL(8), // Hyperchannel
    LANSTAR(9), // Lanstar
    AUTONET(10), // Autonet Short Address
    LOCALTALK(11), // LocalTalk
    LOCALNET(12), // LocalNet (IBM PCNet or SYTEK LocalNET)
    ULTRA_LINK(13), // Ultra link
    SMDS(14), // SMDS
    FRAME_RELAY(15), // Frame Relay
    ATM1(16), // Asynchronous Transmission Mode (ATM)
    HDLC(17), // HDLC
    FIBER_CHANNEL(18), // Fibre Channel
    ATM2(19), // Asynchronous Transmission Mode (ATM)
    SERIAL(20), // Serial Line
    ATM3(21), // Asynchronous Transmission Mode (ATM)
    MIL_STD_188_220(22), // MIL-STD-188-220
    METRICOM(23), // Metricom
    IEEE_1394(24), // IEEE 1394.1995
    MAPOS(25), // MAPOS
    TWINAXIAL(26), // Twinaxial
    EUI_64(27), // EUI-64
    HIPARP(28), // HIPARP
    IOS_7816_3(29), // IP and ARP over ISO 7816-3
    ARPSEC(30), // ARPSec
    IPSEC(31), // IPsec tunnel
    INFINBAND(32), // InfiniBand (TM)
    TIA_102(33), // TIA-102 Project 25 Common Air Interface (CAI)
    INVALID(-1);

    /**
     * Storage for integer representation of enumeration.
     */
    private int integerValue;

    /**
     * Default constructor.
     *
     * @param integerValue
     *            Integer representation of enumeration.
     */
    HardwareAddressType(int integerValue) {
        this.integerValue = integerValue;
    }

    /**
     * Retrieve the integer representation of this enumeration.
     *
     * @return Integer representation of this enumeration.
     */
    public int toInt() {
        return integerValue;
    }

    /**
     * Map the given integer to a corresponding ENUM value.
     *
     * @param integerValue
     *            to convert.
     * @return Corresponding ENUM value. If no match, returns INVALID.
     */
    public static HardwareAddressType toEnum(int integerValue) {
        switch (integerValue) {
            case 1:
                return ETHERNET;
            case 2:
                return EXPERIMENTAL_ENET;
            case 3:
                return AMATEUR_RADIO;
            case 4:
                return PRONET;
            case 5:
                return CHAOS;
            case 6:
                return IEEE_802;
            case 7:
                return ARCNET;
            case 8:
                return HYPERCHANNEL;
            case 9:
                return LANSTAR;
            case 10:
                return AUTONET;
            case 11:
                return LOCALTALK;
            case 12:
                return LOCALNET;
            case 13:
                return ULTRA_LINK;
            case 14:
                return SMDS;
            case 15:
                return FRAME_RELAY;
            case 16:
                return ATM1;
            case 17:
                return HDLC;
            case 18:
                return FIBER_CHANNEL;
            case 19:
                return ATM2;
            case 20:
                return SERIAL;
            case 21:
                return ATM3;
            case 22:
                return MIL_STD_188_220;
            case 23:
                return METRICOM;
            case 24:
                return IEEE_1394;
            case 25:
                return MAPOS;
            case 26:
                return TWINAXIAL;
            case 27:
                return EUI_64;
            case 28:
                return HIPARP;
            case 29:
                return IOS_7816_3;
            case 30:
                return ARPSEC;
            case 31:
                return IPSEC;
            case 32:
                return INFINBAND;
            case 33:
                return TIA_102;
            default:
                return INVALID;
        }
    }

}
