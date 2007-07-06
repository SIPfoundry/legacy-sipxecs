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
