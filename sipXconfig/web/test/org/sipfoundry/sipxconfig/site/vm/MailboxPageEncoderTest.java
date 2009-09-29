/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.vm;

import junit.framework.TestCase;

import org.apache.tapestry.engine.ServiceEncoding;
import org.easymock.EasyMock;
import org.easymock.IMocksControl;

public class MailboxPageEncoderTest extends TestCase {
    private MailboxPageEncoder m_encoder;

    protected void setUp() {
        m_encoder = new MailboxPageEncoder();
        m_encoder.setUrl("myapp");
    }

    public void testDecode() {
        IMocksControl encodingControl = EasyMock.createControl();
        ServiceEncoding encoding = encodingControl.createMock(ServiceEncoding.class);
        encoding.getServletPath();
        encodingControl.andReturn("myapp");
        encoding.getPathInfo();
        encodingControl.andReturn("userid/inbox");
        encoding.setParameterValue("service", "external");
        encoding.setParameterValue("page", ManageVoicemail.PAGE);
        encoding.setParameterValue("sp", "Suserid/inbox");
        encodingControl.replay();

        m_encoder.decode(encoding);

        encodingControl.verify();
    }

    public void testEncode() {
        IMocksControl encodingControl = EasyMock.createControl();
        ServiceEncoding encoding = encodingControl.createMock(ServiceEncoding.class);
        encoding.getParameterValue("service");
        encodingControl.andReturn("external");
        encoding.getParameterValue("page");
        encodingControl.andReturn(ManageVoicemail.PAGE);
        encoding.getParameterValues("sp");
        encodingControl.andReturn(new String[] {"userid", "folderid"});
        encoding.setServletPath("myapp/userid/folderid");
        encoding.setParameterValue("service", null);
        encoding.setParameterValue("page", null);
        encoding.setParameterValue("sp", null);
        encodingControl.replay();

        m_encoder.encode(encoding);

        encodingControl.verify();
    }

}
