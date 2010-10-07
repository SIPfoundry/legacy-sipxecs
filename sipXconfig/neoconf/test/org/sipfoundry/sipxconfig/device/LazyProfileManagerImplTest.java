/*
 *
 *
 * Copyright (C) 2010 eZuce, Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.device;

import java.util.Arrays;
import java.util.Collection;
import java.util.Date;

import junit.framework.TestCase;

import static java.lang.Thread.sleep;
import static org.easymock.classextension.EasyMock.createMock;
import static org.easymock.classextension.EasyMock.replay;
import static org.easymock.classextension.EasyMock.verify;

public class LazyProfileManagerImplTest extends TestCase {

    public void testGenerateProfiles() throws Exception {
        generateProfiles(true);
        generateProfiles(false);
    }

    private void generateProfiles(boolean restart) throws Exception {
        ProfileManager manager = createMock(ProfileManager.class);
        manager.generateProfile(201, restart, null);
        manager.generateProfile(202, restart, null);
        replay(manager);

        LazyProfileManagerImpl lazyManager = new LazyProfileManagerImpl();
        lazyManager.setSleepInterval(100);
        lazyManager.setTarget(manager);
        lazyManager.init();
        lazyManager.generateProfiles(Arrays.asList(new Integer[] {201, 202}), restart, null);
        sleep(300);

        verify(manager);
    }

    public void testGenerateProfile() throws Exception {
        generateProfile(true);
        generateProfile(false);
    }

    private void generateProfile(boolean restart) throws Exception {
        ProfileManager manager = createMock(ProfileManager.class);
        manager.generateProfile(201, restart, null);
        replay(manager);

        LazyProfileManagerImpl lazyManager = new LazyProfileManagerImpl();
        lazyManager.setSleepInterval(100);
        lazyManager.setTarget(manager);
        lazyManager.init();
        lazyManager.generateProfile(201, restart, null);
        sleep(300);

        verify(manager);
    }

    public void testRestartDevices() throws Exception {

        ProfileManager manager = createMock(ProfileManager.class);
        manager.restartDevice(201, null);
        manager.restartDevice(202, null);
        replay(manager);

        LazyProfileManagerImpl lazyManager = new LazyProfileManagerImpl();
        lazyManager.setSleepInterval(100);
        lazyManager.setTarget(manager);
        lazyManager.init();
        lazyManager.restartDevices(Arrays.asList(new Integer[] {201, 202}), null);
        sleep(300);

        verify(manager);
    }

    public void testRestartDevice() throws Exception {

        ProfileManager manager = createMock(ProfileManager.class);
        manager.restartDevice(201, null);
        replay(manager);

        LazyProfileManagerImpl lazyManager = new LazyProfileManagerImpl();
        lazyManager.setSleepInterval(100);
        lazyManager.setTarget(manager);
        lazyManager.init();
        lazyManager.restartDevice(201, null);
        sleep(300);

        verify(manager);
    }

}
