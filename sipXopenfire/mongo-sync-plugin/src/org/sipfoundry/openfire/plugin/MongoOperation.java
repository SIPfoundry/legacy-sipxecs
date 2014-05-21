package org.sipfoundry.openfire.plugin;

public enum MongoOperation {
    INSERT, UPDATE, DELETE;

    public static MongoOperation fromString(String op) {
        MongoOperation result = null;
        if ("i".equalsIgnoreCase(op)) {
            result = INSERT;
        } else if ("u".equalsIgnoreCase(op)) {
            result = UPDATE;
        } else if ("d".equalsIgnoreCase(op)) {
            result = DELETE;
        }

        return result;
    }
}
