package org.sipfoundry.sipximbot;

import org.sipfoundry.commons.freeswitch.CallCommand;
import org.sipfoundry.commons.freeswitch.FreeSwitchEvent;
import org.sipfoundry.commons.freeswitch.FreeSwitchEventSocketInterface;

public class ListenIn extends CallCommand {
    
    public boolean start() {
        m_finished = false;
        // Send the command to the socket
        m_fses.cmd("api " + m_command);
        return false;
    }
    
    
    public ListenIn(FreeSwitchEventSocketInterface fses, String uri, 
                    String uuid, String sipDomain) {
        super(fses);
        m_command = "originate sofia/" + sipDomain + "/"  + uri +  
                     " &eavesdrop(" + uuid + ")" + " XML default '" +
                     "My Assistant" + "' '' ";
    }  
    
    public boolean handleEvent(FreeSwitchEvent event) {

        String content = event.getContent();
        
        if(content != null) {
            if(content.startsWith("+OK")) {
                m_finished = true;
            }
        
            if(content.startsWith("-ERR")) {
                m_finished = true;
            }
        }
        
        return isFinished();
    }
}
