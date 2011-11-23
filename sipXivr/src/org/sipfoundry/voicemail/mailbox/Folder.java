/*
 *
 *
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.voicemail.mailbox;

public enum Folder {
    INBOX {
        public String toString() {
            return "inbox";
        }
    },

    SAVED {
        public String toString() {
            return "saved";
        }
    },

    DELETED {
        public String toString() {
            return "deleted";
        }
    },

    CONFERENCE {
        public String toString() {
            return "conference";
        }
    }
}