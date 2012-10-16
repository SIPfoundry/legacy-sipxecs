/**
 *
 *
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.openfire.vcard.synchserver;

import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.Calendar;

import org.xmpp.packet.Message;
import org.xmpp.packet.Message.Type;

public class Util {
    public static Message createNotificationMessage(String server, String aor) {
        Message msg = new Message();
        msg.setFrom(server);
        msg.setTo(aor);
        msg.setType(Type.headline);
        msg.setSubject("Warning");
        StringBuilder builder = new StringBuilder();
        builder.append("From:");
        builder.append(server);
        builder.append("\n");
        builder.append("To:");
        builder.append(aor);
        builder.append("\n");
        builder.append("Time:");
        builder.append(getCurrentDate());
        builder.append("\n");
        builder.append("Detail: \n");
        builder.append("--- User info has changed on the server; you MUST relogin to synchronize before modifying any of your profile information including avatar ---");
        msg.setBody(builder.toString());

        return msg;
    }

    public static String getCurrentDate() {
        DateFormat dateFormat = new SimpleDateFormat("yyyy/MM/dd HH:mm:ss");
        Calendar cal = Calendar.getInstance();
        return dateFormat.format(cal.getTime());
    }
}
