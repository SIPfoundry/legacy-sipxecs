/*
 *
 *
 * Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.moh;

import java.util.Arrays;
import java.util.Collection;

import org.sipfoundry.sipxconfig.dialplan.CallTag;
import org.sipfoundry.sipxconfig.dialplan.DialPattern;
import org.sipfoundry.sipxconfig.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.dialplan.InternalForwardRule;
import org.sipfoundry.sipxconfig.dialplan.config.FullTransform;
import org.sipfoundry.sipxconfig.feature.Feature;
import org.sipfoundry.sipxconfig.freeswitch.FreeswitchFeature;

public class MohRule extends InternalForwardRule {

    public MohRule(String hostNameAndPort, String defaultMohUser) {
        super(new DialPattern(defaultMohUser + ".", 0), new MohTransform(hostNameAndPort));
        setName("Music on Hold");
        setDescription("Forward music-on-hold calls to IVR");
    }

    @Override
    public CallTag getCallTag() {
        return CallTag.MOH;
    }

    private static class MohTransform extends FullTransform {
        public MohTransform(String hostNameAndPort) {
            setUser("IVR");
            setHost(hostNameAndPort);
            setUrlParams("action=moh", "moh=u{vdigits}");
        }
    }

    @Override
    public Collection<Feature> getAffectedFeaturesOnChange() {
        return Arrays.asList(DialPlanContext.FEATURE, FreeswitchFeature.FEATURE);
    }
}
