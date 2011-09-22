/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * $
 */

/**
 * The CustomSSLSocketFacotry allows users to set timeout for the underneath socket.
 * The timeout is in milliseconds.
 */

package org.sipfoundry.openfire.vcard.provider;

import java.io.IOException;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.net.UnknownHostException;

import javax.net.ssl.SSLSocketFactory;

final public class CustomSSLSocketFactory extends SSLSocketFactory {
    private SSLSocketFactory originalFactory;
    private int socketTimeout;

    public CustomSSLSocketFactory(SSLSocketFactory originalFactory, int timeout) {
        super();

        this.originalFactory = originalFactory;
        socketTimeout = timeout;
    }

    @Override
    public String[] getDefaultCipherSuites() {
        return originalFactory.getDefaultCipherSuites();
    }

    @Override
    public String[] getSupportedCipherSuites() {
        return originalFactory.getSupportedCipherSuites();
    }

    @Override
    public Socket createSocket(String host, int port) throws IOException, UnknownHostException {
        Socket socket = originalFactory.createSocket();
        socket.setSoTimeout(socketTimeout);
        socket.connect(new InetSocketAddress(host, port));
        return socket;
    }

    @Override
    public Socket createSocket(InetAddress host, int port) throws IOException {

        Socket socket = originalFactory.createSocket();
        socket.setSoTimeout(socketTimeout);
        socket.connect(new InetSocketAddress(host, port));
        return socket;
    }

    @Override
    public Socket createSocket(InetAddress address, int port, InetAddress localAddress, int localPort)
            throws IOException {

        Socket socket = originalFactory.createSocket();
        socket.setSoTimeout(socketTimeout);
        socket.bind(new InetSocketAddress(localAddress, localPort));
        socket.connect(new InetSocketAddress(address, port));
        return socket;
    }

    @Override
    public Socket createSocket(Socket s, String host, int port, boolean autoClose) throws IOException {
        s.setSoTimeout(socketTimeout);
        return this.originalFactory.createSocket(s, host, port, autoClose);
    }

    @Override
    public Socket createSocket(String host, int port, InetAddress localHost, int localPort) throws IOException,
            UnknownHostException {
        Socket socket = originalFactory.createSocket();
        socket.setSoTimeout(socketTimeout);
        socket.bind(new InetSocketAddress(localHost, localPort));
        socket.connect(new InetSocketAddress(host, port));
        return socket;
    }
}
