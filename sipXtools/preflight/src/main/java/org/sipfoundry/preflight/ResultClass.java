/**
 * Copyright (C) 2007 - 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.preflight;

/**
 * [Enter descriptive text here]
 * <p>
 *
 * @author Mardy Marshall
 */
public enum ResultClass {
    WARNING, // The results are to be interpreted as a warning.
    TOOL, // The results pertain to the behavior of the test tool.
    UUT; // The results pertain to the behavior of the test target.
}
