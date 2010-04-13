/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.vm.attendant;

import java.io.File;

import junit.framework.TestCase;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.device.FileSystemProfileLocation;
import org.sipfoundry.sipxconfig.device.ProfileFilter;
import org.sipfoundry.sipxconfig.device.ProfileGenerator;
import org.sipfoundry.sipxconfig.vm.Mailbox;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.eq;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.isA;
import static org.easymock.EasyMock.isNull;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;

public class PersonalAttendantWriterTest extends TestCase {

    public void testWrite() {
        PersonalAttendant pa = new PersonalAttendant();
        Mailbox mailbox = new Mailbox(new File("/tmp/voicemail"), "200");

        ProfileGenerator profileGenerator = createMock(ProfileGenerator.class);
        profileGenerator.generate(isA(FileSystemProfileLocation.class),
                isA(PersonalAttendant.AttendantProfileContext.class), (ProfileFilter) isNull(),
                eq("PersonalAttendant.properties"));
        expectLastCall();

        CoreContext coreContext = createMock(CoreContext.class);
        coreContext.getDomainName();
        expectLastCall().andReturn("example.org");

        replay(profileGenerator, coreContext);

        PersonalAttendantWriter personalAttendantWriter = new PersonalAttendantWriter();
        personalAttendantWriter.setGenerator(profileGenerator);
        personalAttendantWriter.setCoreContext(coreContext);

        personalAttendantWriter.write(mailbox, pa);
        verify(profileGenerator, coreContext);
    }
}
