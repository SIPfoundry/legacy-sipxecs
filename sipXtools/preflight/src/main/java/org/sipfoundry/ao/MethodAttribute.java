/**
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.ao;

public class MethodAttribute {
	static final int LOW_PRIORITY = 0;
	static final int NORMAL_PRIORITY = 1;
	static final int HIGH_PRIORITY = 2;

	protected int priority;
	protected boolean blocking;
	protected boolean synchronous;
}
