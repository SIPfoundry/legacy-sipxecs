/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.acd;

import org.sipfoundry.sipxconfig.TestHelper;

import junit.framework.TestCase;

public class AcdAudioTest extends TestCase {
    
    private AcdAudio m_audio;

    protected void setUp() throws Exception {
        m_audio = (AcdAudio) TestHelper.getApplicationContext().getBean("acdAudio");
    }

    public void testSetAudioFileName() {
        m_audio.setAudioFileName("http://acd/audio", "kuku.wav");
        assertEquals("kuku.wav", m_audio.getSettingValue("acd-audio/name"));
        assertEquals("http://acd/audio/kuku.wav",  m_audio.getSettingValue("acd-audio/uri"));
    }
    
    public void testSetAudioFileNameWithSpaces() {
        m_audio.setAudioFileName("http://acd/audio", "name with a space.wav");
        assertEquals("name with a space.wav", m_audio.getSettingValue("acd-audio/name"));
        assertEquals("http://acd/audio/name%20with%20a%20space.wav",  m_audio.getSettingValue("acd-audio/uri"));
    }    
}
