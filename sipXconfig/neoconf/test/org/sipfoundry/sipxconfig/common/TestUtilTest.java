/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.common;

import java.io.File;
import java.io.IOException;
import java.util.Properties;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.test.TestUtil;


public class TestUtilTest extends TestCase {

    public void testGetClasspathDirectory() {
        String classpathDir = TestUtil.getClasspathDirectory(TestUtilTest.class);
        File thisClassFile = new File(classpathDir + "/org/sipfoundry/sipxconfig/common/TestUtilTest.class");
        assertTrue(thisClassFile.exists());
    }

    public void testGetProjectDirectory() {
        String projectDirectory = TestUtil.getProjectDirectory();
        assertEquals("neoconf", new File(projectDirectory).getName());
    }

    public void testGetSysDirProperties() throws IOException {
        Properties props = new Properties();
        String out = System.getProperty("java.io.tmpdir");
        TestUtil.setSysDirProperties(props, "etc", out);
        assertEquals("etc", props.getProperty("sysdir.etc"));
        assertEquals(out, props.getProperty("sysdir.data"));
        assertEquals(out, props.getProperty("sysdir.phone"));
        assertEquals(out, props.getProperty("sysdir.log"));
    }

    public void testGetTopBuildDirectory() {
        String buildDir = TestUtil.getBuildDirectory("neoconf");
        File topBuild = new File(buildDir).getParentFile();
        File configProperties = new File(topBuild, "build.properties");
        assertTrue(configProperties.exists());
    }
}
