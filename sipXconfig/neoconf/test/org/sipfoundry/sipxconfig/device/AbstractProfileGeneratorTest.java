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

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.test.TestUtil;

public class AbstractProfileGeneratorTest extends TestCase {

    private static class ProfileGeneratorStub extends AbstractProfileGenerator {
        ProfileGeneratorStub() {
            setTemplateRoot(TestUtil.getTestSourceDirectory(AbstractProfileGeneratorTest.class));
        }

        protected void generateProfile(ProfileContext context, OutputStream out)
                throws IOException {
            out.write(context.getProfileTemplate().getBytes("US-ASCII"));
        }
    }

    private static class DoublingProfileFilter implements ProfileFilter {
        public void copy(InputStream in, OutputStream out) throws IOException {
            int i;
            while ((i = in.read()) != -1) {
                out.write(i);
                out.write(i);
            }
        }
    }

    protected void setUp() throws Exception {
        super.setUp();
    }

    public void testCopy() throws IOException {
        MemoryProfileLocation location = new MemoryProfileLocation();
        AbstractProfileGenerator pg = new ProfileGeneratorStub();
        pg.copy(location, "CopyFileTest.txt", "whatever");
        assertEquals("test file contents\n", location.toString());
    }

    public void testGenerateProfileContextStringString() {
        MemoryProfileLocation location = new MemoryProfileLocation();
        AbstractProfileGenerator pg = new ProfileGeneratorStub();
        pg.generate(location, new ProfileContext(null, "bongo"), null, "ignored");
        assertEquals("bongo", location.toString());
    }

    public void testGenerateProfileContextStringProfileFilterString() {
        MemoryProfileLocation location = new MemoryProfileLocation();
        AbstractProfileGenerator pg = new ProfileGeneratorStub();
        pg.generate(location, new ProfileContext(null, "bongo"), new DoublingProfileFilter(),
                "ignored");
        assertEquals("bboonnggoo", location.toString());
    }
}
