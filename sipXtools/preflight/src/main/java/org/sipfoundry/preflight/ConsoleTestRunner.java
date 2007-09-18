/**
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.preflight;

/**
 * [Enter descriptive text here]
 * <p>
 * 
 * @author Mardy Marshall
 */
public class ConsoleTestRunner {
    JournalService journalService;
    
    ConsoleTestRunner(JournalService journalService) {
        this.journalService = journalService;
    }
    
    public void validate() {
        ResultCode results;
		NetworkResources networkResources = new NetworkResources();
		
        DHCP dhcp = new DHCP();
        DNS dns = new DNS();
        NTP ntp = new NTP();
        TFTP tftp = new TFTP();
        
		results = dhcp.validate(10, networkResources, journalService);
		System.err.println(results.toString());
		
		results = dns.validate(10, networkResources, journalService);
		System.err.println(results.toString());
		
		results = ntp.validate(10, networkResources, journalService);
		System.err.println(results.toString());
		
		results = tftp.validate(10, networkResources, journalService);
		System.err.println(results.toString());
    }

    public static final void main(String[] args) {
        class ConsoleJournalService implements JournalService {
            public void println(String message) {
                System.out.println(message);
            }
        }
        
        ConsoleTestRunner test = new ConsoleTestRunner(new ConsoleJournalService());
        test.validate();
    }

}
