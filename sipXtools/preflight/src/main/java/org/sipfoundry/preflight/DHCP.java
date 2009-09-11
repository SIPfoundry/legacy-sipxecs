/**
 * Copyright (C) 2007 - 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.preflight;

import java.io.IOException;
import java.net.*;
import java.util.*;

import org.sipfoundry.commons.dhcp.*;
import org.sipfoundry.commons.util.JournalService;

import static org.sipfoundry.commons.dhcp.DHCPOption.Code.*;
import static org.sipfoundry.commons.dhcp.HardwareAddressType.*;
import static org.sipfoundry.commons.dhcp.MessageType.*;
import static org.sipfoundry.commons.dhcp.MessageTypeOption.Type.*;
import static org.sipfoundry.preflight.ResultCode.*;

/**
 * [Enter descriptive text here]
 * <p>
 *
 * @author Mardy Marshall
 */
public class DHCP {
    private String chaddr = "00:D0:1E:FF:FF:FE";
    private InetAddress ciaddr = null;
    private InetAddress yiaddr = null;
    private InetAddress siaddr = null;
    private InetAddress giaddr = null;
    private InetAddress broadcastAddress = null;
    private DatagramSocket serverSocket;
    private static int MAX_TIMEOUT_FACTOR = 3;

    public ResultCode validate(int timeout, NetworkResources networkResources, JournalService journalService, InetAddress bindAddress) {
        ResultCode results = NONE;
        boolean useInform = false;
        int timeoutFactor = 1;

        journalService.println("Starting DHCP server test.");

        for ( ;; ) {
            if (!useInform) {
                results = performDiscover(timeout * timeoutFactor, networkResources, journalService, bindAddress);
                if (results == SOCKET_BIND_FAILURE) {
                    // Discovery failed due to a bind issue, try performing an Inform request.
                    useInform = true;
                } else if (results == TIMEOUT_FAILURE) {
                    ++timeoutFactor;
                    if (timeoutFactor > MAX_TIMEOUT_FACTOR) {
                        journalService.println("Network timeout.  Giving up.");
                        break;
                    } else {
                        journalService.println("Network timeout.  Retrying with timeout increased to " + timeout * timeoutFactor + " seconds.");
                    	continue;
                    }
                } else {
                    break;
                }
            }
            if (useInform) {
                results = performInform(timeout * timeoutFactor, networkResources, journalService, bindAddress);
                if (results == TIMEOUT_FAILURE) {
                    ++timeoutFactor;
                    if (timeoutFactor > MAX_TIMEOUT_FACTOR) {
                        journalService.println("Network timeout.  Giving up.");
                        break;
                    } else {
                        journalService.println("Network timeout.  Retrying with timeout increased to " + timeout * timeoutFactor + " seconds.");
                    }
                } else {
                    break;
                }
            }
        }

        return results;
    }

    private ResultCode performDiscover(int timeout, NetworkResources networkResources, JournalService journalService, InetAddress bindAddress) {
        ResultCode results = NONE;
        Random xidFactory = new Random();
        InetAddress[] serverAddress = new InetAddress[1];
        InetAddress[] assignedAddress = new InetAddress[1];

        try {
            // Attempt to bind to the DHCP Server port.
            serverSocket = new DatagramSocket(67, bindAddress);
            serverSocket.setSoTimeout(timeout * 1000);
        } catch (SocketException e) {
            if (e.getClass() == java.net.BindException.class) {
                journalService.println("Socket Bind Exception: " + e.getMessage());
            } else {
                e.printStackTrace();
                serverSocket.close();
            }
            return SOCKET_BIND_FAILURE;
        }

        int xid = xidFactory.nextInt();

        journalService.println("Sending DHCPDISCOVER request.");
        results = sendDiscover(xid, assignedAddress, serverAddress, journalService);
        journalService.println("");
        if (results != NONE) {
            serverSocket.close();
            return results;
        }

        journalService.println("Sending DHCPREQUEST for: " + assignedAddress[0].getHostAddress());
        results = sendRequest(xid, assignedAddress[0], serverAddress[0], networkResources, journalService);
        journalService.println("");
        if (results != NONE) {
            serverSocket.close();
            return results;
        }

        journalService.println("Sending DHCPRELEASE for: " + assignedAddress[0].getHostAddress());
        xid = xidFactory.nextInt();
        results = sendRelease(xid, assignedAddress[0], serverAddress[0], journalService);
        journalService.println("");

        serverSocket.close();

        return results;
    }

    private ResultCode sendDiscover(int xid, InetAddress[] assignedAddress, InetAddress[] serverAddress, JournalService journalService) {
        ResultCode results;

        try {
            ciaddr = InetAddress.getByName("0.0.0.0");
            yiaddr = InetAddress.getByName("0.0.0.0");
            siaddr = InetAddress.getByName("0.0.0.0");
            giaddr = InetAddress.getLocalHost();
            broadcastAddress = InetAddress.getByName("255.255.255.255");
        } catch (IOException e) {
            journalService.println("  Failed to retrieve local host address.");
            return LOCAL_HOST_FAILURE;
        }

        DHCPMessage discoverMessage = new DHCPMessage();

        discoverMessage.setOp(BOOTREQUEST);
        discoverMessage.setHtype(ETHERNET);
        discoverMessage.setHlen(6);
        discoverMessage.setHops(0);
        discoverMessage.setXid(xid);
        discoverMessage.setSecs(0);
        discoverMessage.setBroadcast(false);
        discoverMessage.setCiaddr(ciaddr);
        discoverMessage.setYiaddr(yiaddr);
        discoverMessage.setSiaddr(siaddr);
        discoverMessage.setGiaddr(giaddr);
        discoverMessage.setChaddr(chaddr);

        discoverMessage.addOption(new MessageTypeOption(DHCPDISCOVER));
        discoverMessage.addOption(new MessageSizeOption(590));
        discoverMessage.addOption(new LeaseTimeOption(360));
        discoverMessage.addOption(new VendorIdentifierOption("Pingtel"));
        discoverMessage.addOption(new ClientIdentifierOption(ETHERNET, chaddr));

        byte data[] = discoverMessage.marshal();

        DatagramPacket outgoing = new DatagramPacket(data, data.length, broadcastAddress, 67);

        try {
            serverSocket.send(outgoing);
        } catch (IOException e) {
            journalService.println("  Failure in send of packet: " + e.getMessage());
            return TRANSMIT_FAILURE;
        }

        DatagramPacket incoming = new DatagramPacket(new byte[1500], 1500);
        DHCPMessage tmpMessage = null;
        DHCPMessage offerMessage = null;
        boolean responseReceived = false;
        InetAddress firstResponseAddress = null;
        boolean multipleResponses = false;

        for (;;) {
            try {
                serverSocket.receive(incoming);
            } catch (IOException e) {
                if (responseReceived) {
                    results = validateOfferMessage(offerMessage, xid, serverAddress, journalService);
                    if (results == NONE) {
                        assignedAddress[0] = offerMessage.getYiaddr();
                    } else if (results == IGNORE) {
                        journalService.println("  Read timeout: " + e.getMessage());
                        results = TIMEOUT_FAILURE;
                    }
                    return results;
                } else {
                    journalService.println("  Read timeout: " + e.getMessage());
                    return TIMEOUT_FAILURE;
                }
            }

            tmpMessage = new DHCPMessage();
            tmpMessage.unmarshal(incoming.getData());

            if ((tmpMessage.getOp() == BOOTREQUEST) && (tmpMessage.getXid() == xid)) {
                continue;  // Ignore.
            }

            if (tmpMessage.getXid() != xid) {
                continue;  // Ignore.
            }

            if (responseReceived) {
            	// Ignore response if from the same server that initially responded.
            	if (firstResponseAddress.getHostAddress().compareTo(incoming.getAddress().getHostAddress()) != 0) {
            		// Not the same, therefore multiple response error.
            		if (multipleResponses == false) {
            			multipleResponses = true;
            			journalService.println("  Multiple responses. Server: " + firstResponseAddress.getHostAddress());
            			journalService.println("  Multiple responses. Server: " + incoming.getAddress().getHostAddress());
            		} else {
            			journalService.println("  Multiple responses. Server: " + incoming.getAddress().getHostAddress());
            		}
            	}
            } else {
                firstResponseAddress = incoming.getAddress();
                offerMessage = tmpMessage;
                responseReceived = true;
            }
        }
    }

    private ResultCode validateOfferMessage(DHCPMessage offerMessage, int xid, InetAddress[] serverAddress, JournalService journalService) {
        ResultCode results = ROGUE_RESPONSE_FAILURE;

        if (offerMessage.getXid() != xid) {
            results = IGNORE;  // Ignore other DHCP traffic.
        } else {
            LinkedList<DHCPOption> optionsList = offerMessage.getOptions();
            for (DHCPOption option : optionsList) {
                if (option.getCode() == SERVER_IDENTIFIER) {
                    ServerIdentifierOption serverIdentifierOption = (ServerIdentifierOption) option;
                    serverAddress[0] = serverIdentifierOption.getServerIdentifier();
                } else if (option.getCode() == DHCP_MESSAGE_TYPE) {
                    MessageTypeOption.Type messageType = ((MessageTypeOption) option).getMessageType();
                    journalService.println(messageType.toString() + " responce received.");
                    if (messageType == DHCPOFFER) {
                        DHCPOption.dumpOptions(optionsList, journalService);
                        results = NONE;
                    } else {
                        // Unrecognized response.
                        journalService.println("  Invalid response to DHCPDISCOVER request.");
                        results = DHCP_DISCOVER_REJECTED;
                    }
                }
            }
        }

        return results;
    }

    private ResultCode sendRequest(int xid, InetAddress assignedAddress, InetAddress serverAddress, NetworkResources networkResources, JournalService journalService) {
        ResultCode results;

        try {
            ciaddr = InetAddress.getByName("0.0.0.0");
            yiaddr = assignedAddress;
            siaddr = InetAddress.getByName("0.0.0.0");
            giaddr = InetAddress.getLocalHost();
            broadcastAddress = InetAddress.getByName("255.255.255.255");
        } catch (IOException e) {
            journalService.println("  Failed to retreive local host address.");
            return LOCAL_HOST_FAILURE;
        }

        DHCPMessage requestMessage = new DHCPMessage();

        requestMessage.setOp(BOOTREQUEST);
        requestMessage.setHtype(ETHERNET);
        requestMessage.setHlen(6);
        requestMessage.setHops(0);
        requestMessage.setXid(xid);
        requestMessage.setSecs(0);
        requestMessage.setBroadcast(false);
        requestMessage.setCiaddr(ciaddr);
        requestMessage.setYiaddr(yiaddr);
        requestMessage.setSiaddr(siaddr);
        requestMessage.setGiaddr(giaddr);
        requestMessage.setChaddr(chaddr);

        requestMessage.addOption(new MessageTypeOption(DHCPREQUEST));
        requestMessage.addOption(new MessageSizeOption(590));
        requestMessage.addOption(new RequestedAddressOption(assignedAddress));
        requestMessage.addOption(new ParameterRequestOption(SUBNET_MASK, ROUTER, DOMAIN_NAME, DOMAIN_NAME_SERVER, TFTP_SERVER, BOOT_FILE, SIP_SERVER, NTP_SERVER, TIME_OFFSET));
        requestMessage.addOption(new LeaseTimeOption(360));
        requestMessage.addOption(new VendorIdentifierOption("Pingtel"));
        requestMessage.addOption(new ClientIdentifierOption(ETHERNET, chaddr));
        requestMessage.addOption(new ServerIdentifierOption(serverAddress));

        byte data[] = requestMessage.marshal();

        DatagramPacket outgoing = new DatagramPacket(data, data.length, broadcastAddress, 67);

        try {
            serverSocket.send(outgoing);
        } catch (IOException e) {
            journalService.println("  Failure in send of packet: " + e.getMessage());
            return TRANSMIT_FAILURE;
        }

        DatagramPacket incoming = new DatagramPacket(new byte[1500], 1500);
        DHCPMessage tmpMessage = null;
        DHCPMessage ackMessage = null;
        boolean responseReceived = false;
        InetAddress firstResponseAddress = null;
        boolean multipleResponses = false;

        for (;;) {
            try {
                serverSocket.receive(incoming);
            } catch (IOException e) {
                if (responseReceived) {
                    results = processAck(ackMessage, xid, networkResources, journalService);
                    if (results == IGNORE) {
                        journalService.println("  Read timeout: " + e.getMessage());
                        results = TIMEOUT_FAILURE;
                    }
                    return results;
                } else {
                    journalService.println("  Read timeout: " + e.getMessage());
                    return TIMEOUT_FAILURE;
                }
            }

            tmpMessage = new DHCPMessage();
            tmpMessage.unmarshal(incoming.getData());

            if ((tmpMessage.getOp() == BOOTREQUEST) && (tmpMessage.getXid() == xid)) {
                continue;  // Ignore.
            }

            if (tmpMessage.getXid() != xid) {
                continue;  // Ignore.
            }

            if (responseReceived) {
            	// Ignore response if from the same server that initially responded.
            	if (firstResponseAddress.getHostAddress().compareTo(incoming.getAddress().getHostAddress()) != 0) {
            		// Not the same, therefore multiple response error.
            		if (multipleResponses == false) {
            			multipleResponses = true;
            			journalService.println("  Multiple responses. Server: " + firstResponseAddress.getHostAddress());
            			journalService.println("  Multiple responses. Server: " + incoming.getAddress().getHostAddress());
            		} else {
            			journalService.println("  Multiple responses. Server: " + incoming.getAddress().getHostAddress());
            		}
            	}
            } else {
                firstResponseAddress = incoming.getAddress();
                ackMessage = tmpMessage;
                responseReceived = true;
            }
        }
    }

    private ResultCode sendRelease(int xid, InetAddress assignedAddress, InetAddress serverAddress, JournalService journalService) {
        try {
            ciaddr = assignedAddress;
            yiaddr = InetAddress.getByName("0.0.0.0");
            siaddr = InetAddress.getByName("0.0.0.0");
            giaddr = InetAddress.getLocalHost();
        } catch (IOException e) {
            journalService.println("  Failed to retreive local host address.");
            return LOCAL_HOST_FAILURE;
        }

        DHCPMessage releaseMessage = new DHCPMessage();

        releaseMessage.setOp(BOOTREQUEST);
        releaseMessage.setHtype(ETHERNET);
        releaseMessage.setHlen(6);
        releaseMessage.setHops(0);
        releaseMessage.setXid(xid);
        releaseMessage.setSecs(0);
        releaseMessage.setBroadcast(false);
        releaseMessage.setCiaddr(ciaddr);
        releaseMessage.setYiaddr(yiaddr);
        releaseMessage.setSiaddr(siaddr);
        releaseMessage.setGiaddr(giaddr);
        releaseMessage.setChaddr(chaddr);

        releaseMessage.addOption(new MessageTypeOption(DHCPRELEASE));
        releaseMessage.addOption(new ClientIdentifierOption(ETHERNET, chaddr));
        releaseMessage.addOption(new ServerIdentifierOption(serverAddress));

        byte data[] = releaseMessage.marshal();

        DatagramPacket outgoing = new DatagramPacket(data, data.length, serverAddress, 67);

        try {
            serverSocket.send(outgoing);
        } catch (IOException e) {
            journalService.println("  Failure in send of packet: " + e.getMessage());
            return TRANSMIT_FAILURE;
        }

        return NONE;
    }

    private ResultCode performInform(int timeout, NetworkResources networkResources, JournalService journalService, InetAddress bindAddress) {
        Random xidFactory = new Random();
        int xid = xidFactory.nextInt();
        ResultCode results;

        try {
            // Attempt to bind to the DHCP Client port.
            serverSocket = new DatagramSocket(68, bindAddress);
            serverSocket.setSoTimeout(timeout * 1000);
        } catch (SocketException e) {
            if (e.getClass() == java.net.BindException.class) {
                journalService.println("  Socket Bind Exception: " + e.getMessage());
            } else {
                e.printStackTrace();
            }
            return SOCKET_BIND_FAILURE;
        }

        try {
            ciaddr = InetAddress.getByName("0.0.0.0");
            yiaddr = InetAddress.getLocalHost();
            siaddr = InetAddress.getByName("0.0.0.0");
            giaddr = InetAddress.getByName("0.0.0.0");
            broadcastAddress = InetAddress.getByName("255.255.255.255");
        } catch (IOException e) {
            serverSocket.close();
            journalService.println("  Failed to retreive local host address.");
            return LOCAL_HOST_FAILURE;
        }

        DHCPMessage informMessage = new DHCPMessage();

        informMessage.setOp(BOOTREQUEST);
        informMessage.setHtype(ETHERNET);
        informMessage.setHlen(6);
        informMessage.setHops(0);
        informMessage.setXid(xid);
        informMessage.setSecs(0);
        informMessage.setBroadcast(false);
        informMessage.setCiaddr(ciaddr);
        informMessage.setYiaddr(yiaddr);
        informMessage.setSiaddr(siaddr);
        informMessage.setGiaddr(giaddr);
        informMessage.setChaddr(chaddr);

        informMessage.addOption(new MessageTypeOption(DHCPINFORM));
        informMessage.addOption(new MessageSizeOption(590));
        informMessage.addOption(new ParameterRequestOption(SUBNET_MASK, ROUTER, DOMAIN_NAME, DOMAIN_NAME_SERVER, TFTP_SERVER, BOOT_FILE, SIP_SERVER, NTP_SERVER, TIME_OFFSET));
        informMessage.addOption(new VendorIdentifierOption("Pingtel"));
        informMessage.addOption(new ClientIdentifierOption(ETHERNET, chaddr));

        byte data[] = informMessage.marshal();

        DatagramPacket outgoing = new DatagramPacket(data, data.length, broadcastAddress, 67);

        try {
            serverSocket.send(outgoing);
        } catch (IOException e) {
            serverSocket.close();
            journalService.println("  Failure in send of packet: " + e.getMessage());
            return TRANSMIT_FAILURE;
        }

        DatagramPacket incoming = new DatagramPacket(new byte[1500], 1500);
        DHCPMessage tmpMessage = null;
        DHCPMessage ackMessage = null;
        boolean responseReceived = false;
        InetAddress firstResponseAddress = null;
        boolean multipleResponses = false;

        for (;;) {
            try {
                serverSocket.receive(incoming);
            } catch (IOException e) {
                serverSocket.close();
                if (responseReceived) {
                    results = processAck(ackMessage, xid, networkResources, journalService);
                    if (results == IGNORE) {
                        journalService.println("  Read timeout: " + e.getMessage());
                        results = TIMEOUT_FAILURE;
                    }
                    return results;
                } else {
                    journalService.println("  Read timeout: " + e.getMessage());
                    return TIMEOUT_FAILURE;
                }
            }

            tmpMessage = new DHCPMessage();
            tmpMessage.unmarshal(incoming.getData());

            if ((tmpMessage.getOp() == BOOTREQUEST) && (tmpMessage.getXid() == xid)) {
                continue;  // Ignore.
            }

            if (tmpMessage.getXid() != xid) {
                continue;  // Ignore.
            }

            if (responseReceived) {
            	// Ignore response if from the same server that initially responded.
            	if (firstResponseAddress.getHostAddress().compareTo(incoming.getAddress().getHostAddress()) != 0) {
            		// Not the same, therefore multiple response error.
            		if (multipleResponses == false) {
            			multipleResponses = true;
            			journalService.println("  Multiple responses. Server: " + firstResponseAddress.getHostAddress());
            			journalService.println("  Multiple responses. Server: " + incoming.getAddress().getHostAddress());
            		} else {
            			journalService.println("  Multiple responses. Server: " + incoming.getAddress().getHostAddress());
            		}
            	}
            } else {
                firstResponseAddress = incoming.getAddress();
                ackMessage = tmpMessage;
                responseReceived = true;
            }
        }
    }

    private ResultCode processAck(DHCPMessage ackMessage, int xid, NetworkResources networkResources, JournalService journalService) {
        ResultCode results = NONE;

        if (ackMessage.getXid() != xid) {
            results = IGNORE;  // Ignore other DHCP traffic.
        }

        if (results == NONE) {
            LinkedList<DHCPOption> optionsList = ackMessage.getOptions();
            for (DHCPOption option : optionsList) {
                if (option.getCode() == DHCP_MESSAGE_TYPE) {
                    MessageTypeOption.Type messageType = ((MessageTypeOption) option).getMessageType();
                    journalService.println(messageType.toString() + " responce received.");
                    if (messageType == DHCPACK) {
                        DHCPOption.dumpOptions(optionsList, journalService);

                        networkResources.siadr = ackMessage.getSiaddr();
                        networkResources.sname = ackMessage.getSName();
                        networkResources.file = ackMessage.getFile();

                        // Verify that the minimum set of options are returned.
                        DHCPOption.extractOptions(ackMessage.getOptions(), networkResources);

                        if (networkResources.routers == null) {
                            journalService.println("  Missing ROUTER option from DHCPACK.");
                            results = MISSING_CONFIG;
                        }

                        if (networkResources.domainName == null) {
                            journalService.println("  Missing DOMAIN NAME option from DHCPACK.");
                            results = MISSING_CONFIG;
                        }

                        if (networkResources.domainNameServers == null) {
                            journalService.println("  Missing DNS SERVER option from DHCPACK.");
                            results = MISSING_CONFIG;
                        }

                        if (networkResources.ntpServers == null) {
                            journalService.println("  Missing NTP SERVER option from DHCPACK.");
                        }

                        if (networkResources.configServer == null) {
                            journalService.println("  Missing TFTP SERVER option from DHCPACK.");
                        }
                    } else {
                        results = DHCP_REQUEST_REJECTED;
                    }
                }
            }
        }

        return results;
    }

}
