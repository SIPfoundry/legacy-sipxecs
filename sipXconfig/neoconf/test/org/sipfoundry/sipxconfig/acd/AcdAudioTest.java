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


public class AcdAudioTest extends BeanWithSettingsTestCase {

    private AcdAudio m_audio;

    protected void setUp() throws Exception {
        super.setUp();
        m_audio = new AcdAudio();
        initializeBeanWithSettings(m_audio);
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
