package org.sipfoundry.callcontroller;

import java.security.Principal;

public class TrustedPrincipal implements Principal {

    @Override
    public String getName() {
        return "~~id~sipxcallcontroller";
    }

}
