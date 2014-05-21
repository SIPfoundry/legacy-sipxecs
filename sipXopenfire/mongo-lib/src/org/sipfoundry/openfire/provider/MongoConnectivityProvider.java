package org.sipfoundry.openfire.provider;

import org.jivesoftware.openfire.provider.ConnectivityProvider;
import org.sipfoundry.commons.util.UnfortunateLackOfSpringSupportFactory;

public class MongoConnectivityProvider implements ConnectivityProvider {

    @Override
    public void verifyDataSource() throws IllegalArgumentException {
        try {
            UnfortunateLackOfSpringSupportFactory.getOpenfiredb();
        } catch (Exception e) {
            throw new IllegalArgumentException(e);
        }
    }

}
