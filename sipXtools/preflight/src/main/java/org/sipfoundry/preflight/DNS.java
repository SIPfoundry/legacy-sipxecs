/**
 * Copyright (C) 2007 - 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.preflight;

import java.io.IOException;
import java.net.InetAddress;
import java.net.UnknownHostException;
import java.util.LinkedList;
import org.xbill.DNS.*;

import org.sipfoundry.commons.dhcp.NetworkResources;
import org.sipfoundry.commons.util.JournalService;

import static org.sipfoundry.preflight.ResultCode.*;

/**
 * [Enter descriptive text here]
 * <p>
 *
 * @author Mardy Marshall
 */
public class DNS {
    public ResultCode validate(int timeout, NetworkResources networkResources, JournalService journalService, InetAddress bindAddress) {
        ResultCode results = NONE;
        SimpleResolver resolver = null;
        String domainName;
        LinkedList<NAPTRRecord> naptrRecords;
        LinkedList<String> targets;

        journalService.println("Starting DNS servers test.");

        try {
            resolver = new SimpleResolver();
            resolver.setLocalAddress(bindAddress);
            resolver.setTimeout(timeout);
        } catch (UnknownHostException e) {
            e.printStackTrace();
        }

        // Determine which domain name to use.  If a SIP domain name has been specified, use it.
        // Otherwise use the domain name that was discovered via DHCP.
        if (networkResources.sipDomainName != null) {
            domainName = networkResources.sipDomainName;
        } else {
            domainName = networkResources.domainName;
        }

        for (InetAddress dnsServer : networkResources.domainNameServers) {
            targets = null;
            resolver.setAddress(dnsServer);

            journalService.println("DNS testing of name server: " + dnsServer.getCanonicalHostName());

            // NAPTR Lookup.
            journalService.println("  NAPTR Lookup for " + domainName + " domain:");
            naptrRecords = new LinkedList<NAPTRRecord>();
            lookupNAPTRRecords(domainName, resolver, naptrRecords, journalService);

            // SRV Lookup.
            if (naptrRecords.isEmpty()) {
                // Build up default SRV target list.
                targets = new LinkedList<String>();
            	targets.add(new String("_sip._udp." + domainName));
            	targets.add(new String("_sip._tcp." + domainName));
            	targets.add(new String("_sips._tcp." + domainName));
            } else {
                // Build up SRV target list from NAPTR records.
                // TODO: For now, only support static target.  Add regexp support later.
                for (NAPTRRecord naptrRecord : naptrRecords) {
                    if ((naptrRecord.getFlags().compareToIgnoreCase("s") == 0)
                            && (naptrRecord.getReplacement() != null)) {
                        if (targets == null) {
                            targets = new LinkedList<String>();
                        }
                        targets.add(naptrRecord.getReplacement().toString());
                    }
                }
            }

            if (targets != null) {
                results = lookupSRVRecords(timeout, resolver, targets, true, journalService);
            } else {
                journalService.println("No static NAPTR targets found, skipping SRV tests.");
                results = UNSUPPORTED_NAPTR_REGEX;
            }
        }

        return results;
    }

    private void lookupNAPTRRecords(String domainName, Resolver resolver, LinkedList<NAPTRRecord> naptrRecords, JournalService journalService) {
        Record[] records = null;
        int result = Lookup.UNRECOVERABLE;
        NAPTRRecord naptrRecord;

        try {
            Lookup naptrLookup = new Lookup(domainName, Type.NAPTR);
            naptrLookup.setResolver(resolver);
            records = naptrLookup.run();
            result = naptrLookup.getResult();
        } catch (TextParseException e) {
            e.printStackTrace();
        }
        switch (result) {
            case Lookup.SUCCESSFUL:
                if (records != null) {
                    for (int i = 0; i < records.length; i++) {
                        naptrRecord = (NAPTRRecord) records[i];
                        naptrRecords.add(naptrRecord);
                        journalService.println("    Order:   " + naptrRecord.getOrder());
                        journalService.println("    Pref:    " + naptrRecord.getPreference());
                        journalService.println("    Flags:   " + naptrRecord.getFlags());
                        journalService.println("    Service: " + naptrRecord.getService());
                        journalService.println("    Regexp:  " + naptrRecord.getRegexp());
                        journalService.println("    Target:  " + naptrRecord.getReplacement());
                        journalService.println("");
                    }
                } else {
                    journalService.println("    No NAPTR records found.\n");
                }
                break;
            case Lookup.UNRECOVERABLE:
                journalService.println("    Unrecoverable error.\n");
                break;
            case Lookup.TRY_AGAIN:
                journalService.println("    Lookup timeout.\n");
                break;
            case Lookup.HOST_NOT_FOUND:
                journalService.println("    Host not found.\n");
                break;
            case Lookup.TYPE_NOT_FOUND:
                journalService.println("    No NAPTR records found.\n");
                break;
        }

    }

    private ResultCode lookupSRVRecords(int timeout, Resolver resolver, LinkedList<String> targets, boolean ping, JournalService journalService) {
        ResultCode results = NONE;
        Record[] records = null;
        int result = Lookup.UNRECOVERABLE;

        for (String target : targets) {
            journalService.println("  SRV Lookup for Target: " + target);
            try {
                Lookup srvLookup = new Lookup(target, Type.SRV);
                srvLookup.setResolver(resolver);
                records = srvLookup.run();
                result = srvLookup.getResult();
            } catch (TextParseException e) {
                e.printStackTrace();
            }
            switch (result) {
                case Lookup.SUCCESSFUL:
                    if (records != null) {
                        for (int i = 0; i < records.length; i++) {
                            SRVRecord srvRecord = (SRVRecord) records[i];
                            journalService.println("    Priority: " + srvRecord.getPriority());
                            journalService.println("    Weight:   " + srvRecord.getWeight());
                            journalService.println("    Port:     " + srvRecord.getPort());
                            String targetMessage = new String("    Target:   " + srvRecord.getTarget());
                            // Try to retrieve A RECORD for target.
                            Lookup aLookup = new Lookup(srvRecord.getTarget(), Type.A);
                            aLookup.setResolver(resolver);
                            Record[] aRecords = aLookup.run();
                            switch (aLookup.getResult()) {
                                case Lookup.SUCCESSFUL:
                                    if (aRecords != null) {
                                        InetAddress targetAddress = ((ARecord)aRecords[0]).getAddress();
                                        targetMessage += " (" + targetAddress.getHostAddress() + ")";
                                        if (ping) {
                                            try {
                                            	if (targetAddress.isReachable(timeout * 1000)) {
                                            	    targetMessage += " Is reachable.";
                                            	} else {
                                            	    targetMessage += " Is NOT reachable.";
                                            	    results = SRV_TARGET_UNREACHABLE;
                                            	}
                                        	} catch (IOException e) {
                                        	    targetMessage += " Is NOT reachable.";
                                        	    results = SRV_TARGET_UNREACHABLE;
                                        	}
                                        }
                                    } else {
                                        targetMessage += " could not be resolved.";
                                        results = SRV_TARGET_UNRESOLVED;
                                    }
                                    break;
                                case Lookup.UNRECOVERABLE:
                                    targetMessage += " [Unrecoverable error]";
                                    results = SRV_TARGET_UNRESOLVED;
                                    break;
                                case Lookup.TRY_AGAIN:
                                    targetMessage += " [Lookup timeout]";
                                    results = SRV_TARGET_UNRESOLVED;
                                    break;
                                case Lookup.HOST_NOT_FOUND:
                                    targetMessage += " could not be resolved.";
                                    results = SRV_TARGET_UNRESOLVED;
                                    break;
                                case Lookup.TYPE_NOT_FOUND:
                                    targetMessage += " could not be resolved.";
                                    results = SRV_TARGET_UNRESOLVED;
                                    break;
                            }

                            journalService.println(targetMessage);
                        }
                    } else {
                        journalService.println("    No SRV records found.");
                        results = NO_SRV_RECORDS;
                    }
                    break;
                case Lookup.UNRECOVERABLE:
                    journalService.println("    Unrecoverable error.");
                    results = NO_SRV_RECORDS;
                    break;
                case Lookup.TRY_AGAIN:
                    journalService.println("    Lookup timeout.");
                    results = NO_SRV_RECORDS;
                    break;
                case Lookup.HOST_NOT_FOUND:
                    journalService.println("    No SRV records found.");
                    results = NO_SRV_RECORDS;
                    break;
                case Lookup.TYPE_NOT_FOUND:
                    journalService.println("    Type not found.");
                    results = NO_SRV_RECORDS;
                    break;
            }
            journalService.println("");
        }

        return results;
    }

}
