package org.sipfoundry.commons.freeswitch;

import org.apache.commons.lang.StringUtils;

public class ConfCommand extends CallCommand {
   String  m_command; // eg. list, lock, mute, deaf, undeaf etc.
   String  m_confName;
   boolean m_success;
   String  m_errString;
   String m_response;
   CallCommandLocalizer m_localizer;

   public  boolean start() {
       m_finished = false;
       // Send the command to the socket
       m_fses.cmd("api conference " + m_confName + " " + m_command);
       return false;
   }

   public ConfCommand(FreeSwitchEventSocketInterface fses, String confName, String confCommand, CallCommandLocalizer localizer) {
       super(fses);
       m_confName = confName;
       m_command = confCommand;
       m_success = false;
       m_localizer = localizer;
   }

   public boolean isSucessful() {
       return m_success;
   }

   public String GetErrString() {
       return m_errString;
   }

   public String getResponse() {
    return m_response;
}

public boolean handleEvent(FreeSwitchEvent event) {

       String content = event.getContent();
       m_response = content;
       if(content != null) {
           if(content.startsWith("OK")) {
               m_finished = true;
               m_success = true;
           }

           if(content.startsWith("Non-Existant")) {
               m_errString = m_localizer.localize("user_not_exists");
               m_finished = true;
           }

           if(content.startsWith("Conference") && content.endsWith("not found\n")) {
               m_errString = m_localizer.localize("conference_empty");
               m_finished = true;
           }

           if(content.startsWith("-ERR")) {
               m_finished = true;
           }
       }
       return isFinished();
   }
}
