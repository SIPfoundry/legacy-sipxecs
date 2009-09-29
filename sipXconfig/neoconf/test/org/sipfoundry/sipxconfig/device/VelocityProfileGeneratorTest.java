/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.device;

import java.util.HashMap;
import java.util.Map;

import junit.framework.TestCase;

import org.apache.velocity.app.VelocityEngine;

public class VelocityProfileGeneratorTest extends TestCase {

    private VelocityProfileGenerator m_generator;
    private MemoryProfileLocation m_location;

    protected void setUp() throws Exception {
        VelocityEngine engine = new VelocityEngine();

        engine.setProperty("resource.loader", "class");
        engine.setProperty("class.resource.loader.class",
                "org.apache.velocity.runtime.resource.loader.ClasspathResourceLoader");
        engine.init();

        m_generator = new VelocityProfileGenerator();
        m_generator.setVelocityEngine(engine);

        m_location = new MemoryProfileLocation();
    }

    public void testGenerateProfile() {
        ProfileContext context = new ProfileContextStub();

        m_generator.generate(m_location, context, null, "ignored");

        assertEquals("testPhone has 17 lines", m_location.toString());
    }

    public static final class ProfileContextStub extends ProfileContext {
        private ProfileContextStub() {
            super(null, "org/sipfoundry/sipxconfig/device/velocity_test.vm");
        }

        public Map<String, Object> getContext() {
            HashMap<String, Object> c = new HashMap<String, Object>();
            c.put("phone", "testPhone");
            c.put("cfg", this);
            return c;
        }

        public int getLines() {
            return 17;
        }
    }
}
