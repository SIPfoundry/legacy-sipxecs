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

import java.io.InputStreamReader;
import java.io.StringWriter;

import org.apache.velocity.app.VelocityEngine;
import org.sipfoundry.sipxconfig.IntegrationTestCase;

public class VelocityProfileGeneratorTestIntegration extends IntegrationTestCase {
    private VelocityEngine m_velocityEngine;

    public void setVelocityEngine(VelocityEngine velocityEngine) {
        m_velocityEngine = velocityEngine;
    }

    public void testLocalMacros() throws Exception {
        // 2 templates contain the same macro
        InputStreamReader templateA = new InputStreamReader(getClass().getResourceAsStream(
                "macro_a.test.vm"));
        InputStreamReader templateB = new InputStreamReader(getClass().getResourceAsStream(
                "macro_b.test.vm"));

        StringWriter writer = new StringWriter();
        m_velocityEngine.evaluate(null, writer, "macro_a", templateA);
        m_velocityEngine.evaluate(null, writer, "macro_b", templateB);

        // we are checking if all macros are local, if they are not macro_b would not overwrite
        // macro_a
        assertEquals("a: X\nb: X\n", writer.toString());
    }
}
