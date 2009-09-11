/**
 * Copyright (C) 2007 - 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.preflight;

import java.util.concurrent.*;
import org.eclipse.swt.widgets.*;

import org.sipfoundry.commons.util.JournalService;

/**
 * [Enter descriptive text here]
 * <p>
 *
 * @author Mardy Marshall
 */
class ShellJournalService extends Thread implements JournalService {
    private boolean isEnabled = true;
    private final Display display;
    private final List list;
    private final LinkedBlockingQueue<String> messageQueue;
    private volatile String message;

    public ShellJournalService(Display display, List list) {
        this.display = display;
        this.list = list;
        messageQueue = new LinkedBlockingQueue<String>();

        super.setDaemon(true);
        start();
    }

    public void enable() {
        isEnabled = true;
    }

    public void disable() {
        isEnabled = false;
    }

    public void print(String message) {
        if (isEnabled) {
            try {
                messageQueue.put(message);
            } catch (InterruptedException e) {
            }
        }
    }

    public void println(String message) {
        if (isEnabled) {
            try {
                messageQueue.put(message);
            } catch (InterruptedException e) {
            }
        }
    }

    public void run() {
        for (;;) {
            try {
                message = messageQueue.take();
                String[] messages = message.split("\n", 32);
                if (!display.isDisposed()) {
                    display.asyncExec(new AsyncListUpdater(messages));
                }
            } catch (InterruptedException e) {
                return;
            }
        }
    }

    class AsyncListUpdater implements Runnable {
        private String[] messages;

        AsyncListUpdater(String[] messages) {
            this.messages = messages;
        }

        public void run() {
            if (list.isDisposed()) {
                return;
            }
        	for (String message : messages) {
            	list.add(message);
        	}
            list.setSelection(list.getItemCount() - 1);
            list.showSelection();
        }

    }
}
