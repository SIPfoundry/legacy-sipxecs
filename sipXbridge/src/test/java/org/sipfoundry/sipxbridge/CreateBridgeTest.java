package org.sipfoundry.sipxbridge;

import java.util.Map;


public class CreateBridgeTest extends AbstractSymmitronTestCase {
    
    public void testCreateBridge() throws Exception {
        Object[] args = new Object[1];
        args[0] = this.clientHandle;
        Map retval = (Map) super.client.execute("sipXbridge.createBridge", args);
        super.checkStandardMap(retval);
        assertNotNull ( retval.get(Symmitron.BRIDGE_ID));
        String bridgeId = (String) retval.get(Symmitron.BRIDGE_ID);
        args = new Object[2];
        args[0] = this.clientHandle;
        args[1] = bridgeId;
        retval = (Map) super.client.execute("sipXbridge.destroyBridge",args);
        super.checkStandardMap( retval);
        
    }

}
