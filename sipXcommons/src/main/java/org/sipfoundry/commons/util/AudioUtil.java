/**
 *
 *
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.commons.util;

import java.io.EOFException;
import java.io.File;

import javax.sound.sampled.AudioInputStream;
import javax.sound.sampled.AudioSystem;

import org.jaudiotagger.audio.AudioFile;
import org.jaudiotagger.audio.AudioFileIO;

public class AudioUtil {
    public static long extractWavDuration(File audioFile) {
        try {
            AudioInputStream ais = AudioSystem.getAudioInputStream(audioFile);
            float secs = ais.getFrameLength() / ais.getFormat().getFrameRate();
            return Math.round(secs); // Round up.
        } catch (EOFException e) {
            return 0;
        } catch (Exception e) {
            String trouble = "Message::getDuration Problem determining duration of " + audioFile.getAbsolutePath();
            throw new RuntimeException(trouble, e);
        }
    }

    public static long extractMp3Duration(File audioFile) {
        try {
            AudioFile mp3File = AudioFileIO.read(audioFile);
            return mp3File.getAudioHeader().getTrackLength();
        } catch (Exception e) {
            String trouble = "Message::getDuration Problem determining duration of " + audioFile.getAbsolutePath();
            throw new RuntimeException(trouble, e);
        }
    }
    
    /**
     * Utility method to avoid duplicate code.
     * From this location access is granted from ivr and config
     * 
     * @param groupName
     * @return the name of the playlist folder when MoH is set for Group
     */
    public static String getGroupMoHFolderName(String groupName)
    {
        String prefix = "group_";
        return prefix + groupName;
    }
}
