/**
 * Copyright (C) 2007 - 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.commons.dhcp;

import java.util.LinkedList;
import java.io.DataInputStream;
import java.io.IOException;

import org.sipfoundry.commons.util.JournalService;

/**
 * [Enter descriptive text here]
 * <p>
 *
 * @author Mardy Marshall
 */
public abstract class DHCPOption {
    public enum Code {
        SUBNET_MASK(1),
        TIME_OFFSET(2),
        ROUTER(3),
        TIME_SERVER(4),
        NAME_SERVER(5),
        DOMAIN_NAME_SERVER(6),
        LOG_SERVER(7),
        HOST_NAME(12),
        DOMAIN_NAME(15),
        NTP_SERVER(42),
        REQUESTED_ADDRESS(50),
        LEASE_TIME(51),
        DHCP_MESSAGE_TYPE(53),
        SERVER_IDENTIFIER(54),
        PARAMETER_REQUEST(55),
        ERROR_MESSAGE(56),
        MESSAGE_SIZE(57),
        VENDOR_IDENTIFIER(60),
        CLIENT_IDENTIFIER(61),
        TFTP_SERVER(66),
        BOOT_FILE(67),
        SMTP_SERVER(69),
        WWW_SERVER(72),
        SIP_SERVER(120),
        OPTION_150(150),
        END_OF_OPTIONS(255),
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
        Code(int integerValue) {
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
        public static Code toEnum(int integerValue) {
            switch (integerValue) {
                case 1:
                    return SUBNET_MASK;
                case 2:
                    return TIME_OFFSET;
                case 3:
                    return ROUTER;
                case 4:
                    return TIME_SERVER;
                case 5:
                    return NAME_SERVER;
                case 6:
                    return DOMAIN_NAME_SERVER;
                case 7:
                    return LOG_SERVER;
                case 12:
                    return HOST_NAME;
                case 15:
                    return DOMAIN_NAME;
                case 42:
                    return NTP_SERVER;
                case 50:
                    return REQUESTED_ADDRESS;
                case 51:
                    return LEASE_TIME;
                case 53:
                    return DHCP_MESSAGE_TYPE;
                case 54:
                    return SERVER_IDENTIFIER;
                case 55:
                    return PARAMETER_REQUEST;
                case 56:
                    return ERROR_MESSAGE;
                case 57:
                    return MESSAGE_SIZE;
                case 60:
                    return VENDOR_IDENTIFIER;
                case 61:
                    return CLIENT_IDENTIFIER;
                case 66:
                    return TFTP_SERVER;
                case 67:
                    return BOOT_FILE;
                case 69:
                    return SMTP_SERVER;
                case 72:
                    return WWW_SERVER;
                case 120:
                    return SIP_SERVER;
                case 150:
                    return OPTION_150;
                case 255:
                    return END_OF_OPTIONS;
                default:
                    return INVALID;
            }
        }
    }

    private Code code;
    private int length;

    public Code getCode() {
        return code;
    }

    public void setCode(Code code) {
        this.code = code;
    }

    public int getLength() {
        return length;
    }

    public void setLength(int length) {
        this.length = length;
    }

    public abstract byte[] marshal() throws IOException;

    public static LinkedList<DHCPOption> unmarshalOptions(DataInputStream dataStream) throws IOException {
        Code unmarshalCode;
        LinkedList<DHCPOption> optionsList = new LinkedList<DHCPOption>();

        while (dataStream.available() > 0) {
            unmarshalCode = Code.toEnum(dataStream.readByte() & 0xFF);
            switch (unmarshalCode) {
                case SUBNET_MASK:
                    SubnetMaskOption subnetMaskOption = new SubnetMaskOption();
                    subnetMaskOption.unmarshal(dataStream);
                    optionsList.add(subnetMaskOption);
                    break;
                case TIME_OFFSET:
                    TimeOffsetOption timeOffsetOption = new TimeOffsetOption();
                    timeOffsetOption.unmarshal(dataStream);
                    optionsList.add(timeOffsetOption);
                    break;
                case ROUTER:
                    RouterOption routerOption = new RouterOption();
                    routerOption.unmarshal(dataStream);
                    optionsList.add(routerOption);
                    break;
                case TIME_SERVER:
                    TimeServerOption timerServerOption = new TimeServerOption();
                    timerServerOption.unmarshal(dataStream);
                    optionsList.add(timerServerOption);
                    break;
                case NAME_SERVER:
                    NameServerOption nameServerOption = new NameServerOption();
                    nameServerOption.unmarshal(dataStream);
                    optionsList.add(nameServerOption);
                    break;
                case DOMAIN_NAME_SERVER:
                    DomainNameServerOption domainNameServerOption = new DomainNameServerOption();
                    domainNameServerOption.unmarshal(dataStream);
                    optionsList.add(domainNameServerOption);
                    break;
                case LOG_SERVER:
                    LogServerOption logServerOption = new LogServerOption();
                    logServerOption.unmarshal(dataStream);
                    optionsList.add(logServerOption);
                    break;
                case HOST_NAME:
                    HostNameOption hostNameOption = new HostNameOption();
                    hostNameOption.unmarshal(dataStream);
                    optionsList.add(hostNameOption);
                    break;
                case DOMAIN_NAME:
                    DomainNameOption domainNameOption = new DomainNameOption();
                    domainNameOption.unmarshal(dataStream);
                    optionsList.add(domainNameOption);
                    break;
                case NTP_SERVER:
                    NTPServerOption ntpServerOption = new NTPServerOption();
                    ntpServerOption.unmarshal(dataStream);
                    optionsList.add(ntpServerOption);
                    break;
                case REQUESTED_ADDRESS:
                    RequestedAddressOption requestedAddressOption = new RequestedAddressOption();
                    requestedAddressOption.unmarshal(dataStream);
                    optionsList.add(requestedAddressOption);
                    break;
                case LEASE_TIME:
                    LeaseTimeOption leaseTimeOption = new LeaseTimeOption();
                    leaseTimeOption.unmarshal(dataStream);
                    optionsList.add(leaseTimeOption);
                    break;
                case DHCP_MESSAGE_TYPE:
                    MessageTypeOption messageTypeOption = new MessageTypeOption();
                    messageTypeOption.unmarshal(dataStream);
                    optionsList.add(messageTypeOption);
                    break;
                case SERVER_IDENTIFIER:
                    ServerIdentifierOption serverIdentifierOption = new ServerIdentifierOption();
                    serverIdentifierOption.unmarshal(dataStream);
                    optionsList.add(serverIdentifierOption);
                    break;
                case PARAMETER_REQUEST:
                    ParameterRequestOption parameterRequestOption = new ParameterRequestOption();
                    parameterRequestOption.unmarshal(dataStream);
                    optionsList.add(parameterRequestOption);
                    break;
                case ERROR_MESSAGE:
                    MessageOption messageOption = new MessageOption();
                    messageOption.unmarshal(dataStream);
                    optionsList.add(messageOption);
                    break;
                case MESSAGE_SIZE:
                    MessageSizeOption messageSizeOption = new MessageSizeOption();
                    messageSizeOption.unmarshal(dataStream);
                    optionsList.add(messageSizeOption);
                    break;
                case VENDOR_IDENTIFIER:
                    VendorIdentifierOption vendorIdentifierOption = new VendorIdentifierOption();
                    vendorIdentifierOption.unmarshal(dataStream);
                    optionsList.add(vendorIdentifierOption);
                    break;
                case CLIENT_IDENTIFIER:
                    ClientIdentifierOption clientIdentifierOption = new ClientIdentifierOption();
                    clientIdentifierOption.unmarshal(dataStream);
                    optionsList.add(clientIdentifierOption);
                    break;
                case TFTP_SERVER:
                    TFTPServerOption tftpServerOption = new TFTPServerOption();
                    tftpServerOption.unmarshal(dataStream);
                    optionsList.add(tftpServerOption);
                    break;
                case BOOT_FILE:
                    BootFileOption bootFileOption = new BootFileOption();
                    bootFileOption.unmarshal(dataStream);
                    optionsList.add(bootFileOption);
                    break;
                case SMTP_SERVER:
                    SMTPServerOption smtpServerOption = new SMTPServerOption();
                    smtpServerOption.unmarshal(dataStream);
                    optionsList.add(smtpServerOption);
                    break;
                case WWW_SERVER:
                    WWWServerOption wwwServerOption = new WWWServerOption();
                    wwwServerOption.unmarshal(dataStream);
                    optionsList.add(wwwServerOption);
                    break;
                case SIP_SERVER:
                    SIPServerOption sipServerOption = new SIPServerOption();
                    sipServerOption.unmarshal(dataStream);
                    optionsList.add(sipServerOption);
                    break;
                case OPTION_150:
                    Option150Option option150Option = new Option150Option();
                    option150Option.unmarshal(dataStream);
                    optionsList.add(option150Option);
                    break;
                case END_OF_OPTIONS:
                    return optionsList;
                default:
                    // Eat up this unsupported option.
                    int length = dataStream.readByte() & 0xFF;
                    dataStream.skipBytes(length);
            }

        }

        return optionsList;

    }

    public static void dumpOptions(LinkedList<DHCPOption> optionsList, JournalService journalService) {
        for (DHCPOption option : optionsList) {
            switch (option.getCode()) {
                case SUBNET_MASK:
                    SubnetMaskOption subnetMaskOption = (SubnetMaskOption) option;
                    journalService.println("  Subnet Mask: " + subnetMaskOption.toString());
                    break;
                case TIME_OFFSET:
                    TimeOffsetOption timeOffsetOption = (TimeOffsetOption) option;
                    journalService.println("  Time Offset: " + timeOffsetOption.toString());
                    break;
                case ROUTER:
                    RouterOption routerOption = (RouterOption) option;
                    journalService.println("  Router: " + routerOption.toString());
                    break;
                case TIME_SERVER:
                    TimeServerOption timeServerOption = (TimeServerOption) option;
                    journalService.println("  Time Server: " + timeServerOption.toString());
                    break;
                case NAME_SERVER:
                    NameServerOption nameServerOption = (NameServerOption) option;
                    journalService.println("  Name Server: " + nameServerOption.toString());
                    break;
                case DOMAIN_NAME_SERVER:
                    DomainNameServerOption domainNameServerOption = (DomainNameServerOption) option;
                    journalService.println("  DNS Server: " + domainNameServerOption.toString());
                    break;
                case LOG_SERVER:
                    LogServerOption logServerOption = (LogServerOption) option;
                    journalService.println("  Log Server: " + logServerOption.toString());
                    break;
                case HOST_NAME:
                    HostNameOption hostNameOption = (HostNameOption) option;
                    journalService.println("  Host Name: " + hostNameOption.toString());
                    break;
                case DOMAIN_NAME:
                    DomainNameOption domainNameOption = (DomainNameOption) option;
                    journalService.println("  Domain Name: " + domainNameOption.toString());
                    break;
                case NTP_SERVER:
                    NTPServerOption ntpServerOption = (NTPServerOption) option;
                    journalService.println("  NTP Server: " + ntpServerOption.toString());
                    break;
                case LEASE_TIME:
                    LeaseTimeOption leaseTimeOption = (LeaseTimeOption) option;
                    journalService.println("  Lease Time: " + leaseTimeOption.toString());
                    break;
                case REQUESTED_ADDRESS:
                    RequestedAddressOption requestedAddressOption = (RequestedAddressOption) option;
                    journalService.println("  Requested IP Address: " + requestedAddressOption.toString());
                    break;
                case DHCP_MESSAGE_TYPE:
                    MessageTypeOption messageTypeOption = (MessageTypeOption) option;
                    journalService.println("  Message Type: " + messageTypeOption.toString());
                    break;
                case SERVER_IDENTIFIER:
                    ServerIdentifierOption serverIdentifierOption = (ServerIdentifierOption) option;
                    journalService.println("  Server Identifier: " + serverIdentifierOption.toString());
                    break;
                case PARAMETER_REQUEST:
                    ParameterRequestOption parameterRequestOption = (ParameterRequestOption) option;
                    journalService.println("  Parameter Request: " + parameterRequestOption.toString());
                    break;
                case ERROR_MESSAGE:
                    MessageOption messageOption = (MessageOption) option;
                    journalService.println("  Message: " + messageOption.toString());
                    break;
                case MESSAGE_SIZE:
                    MessageSizeOption messageSizeOption = (MessageSizeOption) option;
                    journalService.println("  Max Message Size: " + messageSizeOption.toString());
                    break;
                case VENDOR_IDENTIFIER:
                    VendorIdentifierOption vendorIdentifierOption = (VendorIdentifierOption) option;
                    journalService.println("  Vendor Identifier: " + vendorIdentifierOption.toString());
                    break;
                case CLIENT_IDENTIFIER:
                    ClientIdentifierOption clientIdentifierOption = (ClientIdentifierOption) option;
                    journalService.println("  Client Identifier: " + clientIdentifierOption.toString());
                    break;
                case TFTP_SERVER:
                    TFTPServerOption tftpServerOption = (TFTPServerOption) option;
                    journalService.println("  TFTP Server: " + tftpServerOption.toString());
                    break;
                case BOOT_FILE:
                    BootFileOption bootFileOption = (BootFileOption) option;
                    journalService.println("  Boot File: " + bootFileOption.toString());
                    break;
                case SMTP_SERVER:
                    SMTPServerOption smtpServerOption = (SMTPServerOption) option;
                    journalService.println("  SMTP Server: " + smtpServerOption.toString());
                    break;
                case WWW_SERVER:
                    WWWServerOption wwwServerOption = (WWWServerOption) option;
                    journalService.println("  WWW Server: " + wwwServerOption.toString());
                    break;
                case SIP_SERVER:
                    SIPServerOption sipServerOption = (SIPServerOption) option;
                    journalService.println("  SIP Server: " + sipServerOption.toString());
                    break;
                case OPTION_150:
                    Option150Option option150Option = (Option150Option) option;
                    journalService.println("  Option 150 Server: " + option150Option.toString());
                    break;
                default:
            }

        }
    }

    public static void extractOptions(LinkedList<DHCPOption> optionsList, NetworkResources networkResources) {
        for (DHCPOption option : optionsList) {
            switch (option.getCode()) {
                case SUBNET_MASK:
                    SubnetMaskOption subnetMaskOption = (SubnetMaskOption) option;
                    networkResources.subnetMask = subnetMaskOption.getMask();
                    break;
                case TIME_OFFSET:
                    TimeOffsetOption timeOffsetOption = (TimeOffsetOption) option;
                    networkResources.timeOffset = timeOffsetOption.getTimeOffset();
                    break;
                case ROUTER:
                    RouterOption routerOption = (RouterOption) option;
                    networkResources.routers = routerOption.getServerList();
                    break;
                case TIME_SERVER:
                    // TimeServerOption timeServerOption = (TimeServerOption)option;
                    break;
                case NAME_SERVER:
                    // NameServerOption nameServerOption = (NameServerOption)option;
                    break;
                case DOMAIN_NAME_SERVER:
                    DomainNameServerOption domainNameServerOption = (DomainNameServerOption) option;
                    networkResources.domainNameServers = domainNameServerOption.getServerList();
                    break;
                case LOG_SERVER:
                    // LogServerOption logServerOption = (LogServerOption)option;
                    break;
                case HOST_NAME:
                    // HostNameOption hostNameOption = (HostNameOption)option;
                    break;
                case DOMAIN_NAME:
                    DomainNameOption domainNameOption = (DomainNameOption) option;
                    networkResources.domainName = domainNameOption.getDomainName();
                    break;
                case NTP_SERVER:
                    NTPServerOption ntpServerOption = (NTPServerOption) option;
                    networkResources.ntpServers = ntpServerOption.getServerList();
                    break;
                case LEASE_TIME:
                    // LeaseTimeOption leaseTimeOption = (LeaseTimeOption)option;
                    break;
                case REQUESTED_ADDRESS:
                    // RequestedAddressOption requestedAddressOption = (RequestedAddressOption)option;
                    break;
                case DHCP_MESSAGE_TYPE:
                    // MessageTypeOption messageTypeOption = (MessageTypeOption)option;
                    break;
                case SERVER_IDENTIFIER:
                    // ServerIdentifierOption serverIdentifierOption = (ServerIdentifierOption)option;
                    break;
                case PARAMETER_REQUEST:
                    // ParameterRequestOption parameterRequestOption = (ParameterRequestOption)option;
                    break;
                case ERROR_MESSAGE:
                    // MessageOption messageOption = (MessageOption)option;
                    break;
                case MESSAGE_SIZE:
                    // MessageSizeOption messageSizeOption = (MessageSizeOption)option;
                    break;
                case VENDOR_IDENTIFIER:
                    // VendorIdentifierOption vendorIdentifierOption = (VendorIdentifierOption)option;
                    break;
                case CLIENT_IDENTIFIER:
                    // ClientIdentifierOption clientIdentifierOption = (ClientIdentifierOption)option;
                    break;
                case TFTP_SERVER:
                    TFTPServerOption tftpServerOption = (TFTPServerOption) option;
                    networkResources.configServer = tftpServerOption.getServerName();
                    break;
                case SMTP_SERVER:
                    // SMTPServerOption smtpServerOption = (SMTPServerOption)option;
                    break;
                case WWW_SERVER:
                    // WWWServerOption wwwServerOption = (WWWServerOption)option;
                    break;
                case SIP_SERVER:
                    SIPServerOption sipServerOption = (SIPServerOption) option;
                    networkResources.sipServers = sipServerOption.getServerList();
                    break;
                default:
            }

        }
    }

}
