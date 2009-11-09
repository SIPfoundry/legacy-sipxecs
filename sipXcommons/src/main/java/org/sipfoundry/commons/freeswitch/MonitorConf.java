/*
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.commons.freeswitch;

import java.util.Vector;

public class MonitorConf extends CallCommand {
    
   public boolean start() {
       m_finished = false;
       // Send the command to the socket
       m_fses.cmd("event plain CUSTOM conference::maintenance");
       return false;
   }
     
    
   public MonitorConf(FreeSwitchEventSocketInterface fses) {         
       super(fses);
   }
   
   public boolean handleEvent(FreeSwitchEvent event) {

       Vector<String> response = event.getResponse();
       
       if(response != null) {
           if(response.size() > 1) {
               if(response.get(1).startsWith("Reply-Text: +OK")) {
                   m_finished = true;
               }
           }
       }       
       return isFinished();
   }
   
}
