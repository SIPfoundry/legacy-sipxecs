package org.sipfoundry.sipxbridge;

import java.util.Map;

import org.apache.log4j.Logger;

public class SignInSignOutTest extends AbstractSymmitronTestCase {
    Logger logger = Logger.getLogger(SignInSignOutTest.class);
    
    public void setUp() throws Exception {
        super.setUp();
    }
    
    public void testSignInSignOut() {
        try {
            super.start();
            String[] myHandle = new String[1] ;
            myHandle[0] = "nat:12345";
            Map retval = (Map) client.execute("sipXbridge.signIn",
                    (Object[]) myHandle);
            client.execute("sipXbridge.signOut",(Object[]) myHandle);
           
            
        } catch (Exception ex) {
            ex.printStackTrace();
            fail("Unexpected exception ");
        }
    }

}
