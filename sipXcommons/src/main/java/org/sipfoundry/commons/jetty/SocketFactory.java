package org.sipfoundry.commons.jetty;

import org.mortbay.http.SocketListener;

public class SocketFactory {
    public static SocketListener createSocketListener(int port) {
        SocketListener socketListener = new SocketListener();
        socketListener.setPort(port);
        socketListener.setMaxThreads(32);
        socketListener.setMinThreads(4);
        socketListener.setLingerTimeSecs(30000);
        socketListener.setMaxIdleTimeMs(60000);
        return socketListener;
    }
}
