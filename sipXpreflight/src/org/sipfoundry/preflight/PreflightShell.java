/**
 * Copyright (C) 2007 - 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.preflight;

import org.sipfoundry.commons.util.JournalService;

public class PreflightShell {
    public static void main(String[] args) {
        class ConsoleJournalService implements JournalService {
            private boolean isEnabled = true;

            public void enable() {
                isEnabled = true;
            }

            public void disable() {
                isEnabled = false;
            }

            public void print(String message) {
                if (isEnabled) {
                    System.out.print(message);
                }
            }

            public void println(String message) {
                if (isEnabled) {
                    System.out.println(message);
                }
            }
        }

        ConsoleTestRunner userInterface = new ConsoleTestRunner(new ConsoleJournalService());
        userInterface.validate(args);
    }
}
