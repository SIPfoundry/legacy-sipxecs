/**
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.preflight;

import static org.sipfoundry.preflight.ResultClass.*;

/**
 * [Enter descriptive text here]
 * <p>
 * 
 * @author Mardy Marshall
 */
public enum ResultCode {
    NONE(0, "Passed", UUT), //
    SOCKET_BIND_FAILURE(128, "Network access failure.", TOOL), //
    LOCAL_HOST_FAILURE(129, "Unable to determine local IP address.", TOOL), //
    TIMEOUT_FAILURE(130, "Timeout waiting for network response.", UUT), //
    TRANSMIT_FAILURE(131, "Network transmission failure.", TOOL), //
    IGNORE(132, "Other network traffic, ignore.", TOOL), //
    MULTIPLE_SERVERS_FAILURE(133, "Multiple DHCP servers detected.", UUT), //
    ROGUE_RESPONSE_FAILURE(134, "Unrecognized response received.", UUT), //
    DHCP_DISCOVER_REJECTED(135, "DHCP DISCOVER was rejected.", UUT), //
    DHCP_REQUEST_REJECTED(136, "DHCP REQUEST was rejected.", UUT), //
    MISSING_CONFIG(137, "Missing mandatory DHCP configuration parameters.", UUT), //
    UNSUPPORTED_NAPTR_REGEX(138, "DNS test does not currently support regex NAPTR records.", WARNING), //
    SRV_TARGET_UNRESOLVED(139, "SRV target could not be resolved.", UUT), //
    SRV_TARGET_UNREACHABLE(140, "SRV target is unreachable.", UUT), //
    NO_SRV_RECORDS(141, "DNS Server did not report any SRV records.", WARNING), //
    NTP_SERVERS_MISSING(142, "No NTP servers available.", WARNING), //
    NTP_TEST_FAILURE(143, "NTP test network failure.", TOOL), //
    NTP_SERVER_FAILURE(144, "NTP Server request failure.", UUT), //
    TFTP_SERVER_MISSING(145, "No TFTP server available.", WARNING), //
    TFTP_ADDRESS_MALFORMED(146, "TFTP Server address is malformed.", UUT), //
    TFTP_TARGET_UNRESOLVED(147, "TFTP server address could not be resolved.", UUT), //
    MULTIPLE_TFTP_TARGETS(148, "Mismatch in DNS TFTP server records.", UUT), //
    TFTP_TARGET_UNREACHABLE(149, "TFTP server is unreachable.", UUT), //
    TFTP_CLIENT_FAILURE(150, "TFTP client encountered unrecoverable error.", TOOL), //
    TFTP_GET_FAILED(151, "TFTP get of test file failed.", UUT), //
    TFTP_CONTENTS_FAILED(152, "TFTP test file did not verify.", UUT), //
    INVALID(255, "INVALID", TOOL); //

    /**
     * Storage for integer representation of enumeration.
     */
    private int integerValue;

    /**
     * Storage for string representation of enumeration.
     */
    private String stringValue;

    /**
     * Storage for class representation of enumeration.
     */
    private ResultClass resultClass;

    /**
     * Default constructor.
     * 
     * @param integerValue
     *            Integer representation of enumeration.
     * @param stringValue
     *            String representation of enumeration.
     * @param resultClass
     *            Class representation of enumeration.
     */
    ResultCode(int integerValue, String stringValue, ResultClass resultClass) {
        this.integerValue = integerValue;
        this.stringValue = stringValue;
        this.resultClass = resultClass;
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
     * Retrieve the string representation of this enumeration.
     * 
     * @return String representation of this enumeration.
     */
    public String toString() {
        return stringValue;
    }

    /**
     * Retrieve the class representation of this enumeration.
     * 
     * @return ResultClass representation of this enumeration.
     */
    public ResultClass toResultClass() {
        return resultClass;
    }

    /**
     * Map the given integer to a corresponding ENUM value.
     * 
     * @param integerValue
     *            to convert.
     * @return Corresponding ENUM value. If no match, returns INVALID.
     */
    public static ResultCode toEnum(int integerValue) {
        switch (integerValue) {
            case 128:
                return SOCKET_BIND_FAILURE;
            case 129:
                return LOCAL_HOST_FAILURE;
            case 130:
                return TIMEOUT_FAILURE;
            case 131:
                return TRANSMIT_FAILURE;
            case 132:
                return MULTIPLE_SERVERS_FAILURE;
            case 133:
                return ROGUE_RESPONSE_FAILURE;
            case 134:
                return MISSING_CONFIG;
            case 0:
                return NONE;
            default:
                return INVALID;
        }
    }

}
