/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.commons.freeswitch;

public class FaxReceive extends CallCommand {

    private String m_faxResultCode;
    private String m_faxSuccess;
    private String m_faxRemoteStationId;
    private String m_faxTotalPages;
    private String m_faxResultText;
    
    public FaxReceive(FreeSwitchEventSocketInterface fses, String pathname) {
        super(fses);
        m_faxSuccess = "0";
        // Start receiving
        m_command = "rxfax\nexecute-app-arg: " + pathname;
    }
    
    public boolean handleEvent(FreeSwitchEvent event) {

        if (event.isEmpty()) {
            m_hungup = true;
        }

        if (event.getEventValue("Event-Name", "")
                        .contentEquals("CHANNEL_EXECUTE_COMPLETE")) {
            
            // get various freshly set fax related channel variable values

            m_faxResultCode = event.getEventValue("variable_fax_result_code");
            m_faxSuccess    = event.getEventValue("variable_fax_success");
            m_faxRemoteStationId = event.getEventValue("variable_fax_remote_station_id");
            m_faxTotalPages = event.getEventValue("variable_fax_document_total_pages");
            m_faxResultText = event.getEventValue("variable_fax_result_text"); 
            m_finished = true;
        }
        
        return isFinished();
    }
    
    public String getResultCode() {
        if(m_faxResultCode == null) 
            return "";
            
        return m_faxResultCode;
    }
    
    public boolean rxSuccess() {
        return m_faxSuccess.equals("1");
    }    
    
    public String getRemoteStationId() {
        
        return m_faxRemoteStationId;
    }    
    
    public String getResultText() {
        if(m_faxResultText == null)
            return "";
        return m_faxResultText;
    } 
    
    public int faxTotalPages() {
        if(m_faxTotalPages == null) 
            return 0;
        
        return Integer.parseInt(m_faxTotalPages);
    }    
}
