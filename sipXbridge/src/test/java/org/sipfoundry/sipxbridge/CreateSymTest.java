package org.sipfoundry.sipxbridge;

import java.util.Map;

public class CreateSymTest extends AbstractSymmitronTestCase {
    
    
    public void setUp() throws Exception {
        super.setUp();
        super.start();
        super.signIn();
    }
    
    public void testSymCreate() throws Exception {
        
        int count = 1;
        Object[] args = new Object[3];
        args[0] = clientHandle;
        args[1] = new Integer(count);
        args[2] = Symmitron.EVEN;
        
        Map retval = (Map) super.client.execute("sipXbridge.createSyms", args);
        super.checkStandardMap(retval);
        Object[] syms = (Object[]) retval.get(Symmitron.SYM_SESSION);
        assertEquals ("Should allocate only one sym", syms.length , count);
        
      
        args = new Object[2];
        args[0] = clientHandle;
        args[1] = (String)syms[0];
        retval = (Map) client.execute("sipXbridge.getSym", args);
        super.checkStandardMap(retval);
        Map symSession = (Map) retval.get(Symmitron.SYM_SESSION);
        Map receiverSession = ( Map) symSession.get("receiver");
        String ipAddr = (String) receiverSession.get("ipAddress");
        int port = (Integer) receiverSession.get("port");
        System.out.println("ipAddr = " + ipAddr);
        System.out.println("port = " + port);
       
        
        args = new Object[2];
        args[0] = clientHandle;
        args[1] = syms[0];
        retval = ( Map ) super.client.execute("sipXbridge.destroySym",args);
        System.out.println("retval = "  + retval);
        super.checkStandardMap( retval);
       
        
    }

}
