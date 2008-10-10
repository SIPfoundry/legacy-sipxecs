package org.sipfoundry.fswitchtester;

public class FreeSwitchTesterException extends RuntimeException {
    
    public FreeSwitchTesterException ( Exception ex) {
        super(ex);
    }
    
    public FreeSwitchTesterException ( String message, Exception ex) {
        super( message, ex );
    }
    
    public FreeSwitchTesterException ( String message) {
        super(message);
    }

}
