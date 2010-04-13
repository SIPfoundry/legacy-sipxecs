/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.site.acd;

import org.apache.tapestry.test.Creator;
import org.sipfoundry.sipxconfig.acd.AcdQueue;

import junit.framework.TestCase;

public class EditAcdQueueTest extends TestCase {

	public void testGetAcdQueueUriWithNullAcdServer() {
		Creator creator = new Creator();
		EditAcdQueue queuePage =
			(EditAcdQueue) creator.newInstance(EditAcdQueue.class);
		// new instance of queue has null server
		queuePage.setAcdQueue(new AcdQueue());
		assertNull("AcdQueueUri should be null", queuePage.getAcdQueueUri());
	}
}
