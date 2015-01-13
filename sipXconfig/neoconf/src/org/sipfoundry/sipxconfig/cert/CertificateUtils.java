/*
 * Copyright (C) 2012 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.cert;

import static java.lang.String.format;

import java.io.File;
import java.io.IOException;
import java.io.Reader;
import java.io.StringReader;
import java.io.Writer;
import java.security.GeneralSecurityException;
import java.security.KeyPair;
import java.security.PrivateKey;
import java.security.Security;
import java.security.cert.CertificateException;
import java.security.cert.X509Certificate;
import java.util.ArrayList;
import java.util.List;

import org.apache.commons.io.IOUtils;
import org.apache.commons.lang.StringUtils;
import org.bouncycastle.asn1.x500.X500Name;
import org.bouncycastle.cert.X509v3CertificateBuilder;
import org.bouncycastle.cert.jcajce.JcaX509CertificateConverter;
import org.bouncycastle.jce.X509Principal;
import org.bouncycastle.jce.provider.BouncyCastleProvider;
import org.bouncycastle.openssl.PEMReader;
import org.bouncycastle.openssl.PEMWriter;
import org.bouncycastle.operator.ContentSigner;
import org.bouncycastle.operator.OperatorCreationException;
import org.bouncycastle.operator.jcajce.JcaContentSignerBuilder;
import org.jfree.util.Log;
import org.sipfoundry.sipxconfig.common.UserException;

public final class CertificateUtils {
    private static final String PROVIDER = "BC";
    private static final int MAX_HEADER_LINE_COUNT = 512;
    private static final String START_RSA_KEY = "-----BEGIN RSA PRIVATE KEY-----";

    static {
        Security.addProvider(new BouncyCastleProvider());
    }

    private CertificateUtils() {
    }

    public static String getProvider() {
        // making method instead of constant ensure static code block to add provider
        // has been executed
        return PROVIDER;
    }

    public static X509Certificate readCertificate(String in) {
        return readCertificate(new StringReader(in));
    }

    public static X509Certificate readCertificate(Reader in) {
        //read first certificate found
        Object o = readObject(in).get(0);
        if (!(o instanceof X509Certificate)) {
            String msg = format("Certificate was expected but found %s instead", o.getClass().getSimpleName());
            throw new UserException(msg);
        }
        return (X509Certificate) o;
    }

    public static X509Certificate[] readCertificates(Reader in) {
        List<Object> listO = readObject(in);
        List<X509Certificate> certs = new ArrayList<X509Certificate>();
        for (Object o : listO) {
            if ((o instanceof X509Certificate)) {
                certs.add((X509Certificate) o);
            }
        }
        if (certs.size() > 0) {
            return certs.toArray(new X509Certificate[listO.size()]);
        }
        String msg = format("Certificate was expected but not found");
        throw new UserException(msg);
    }

    public static X509Certificate[] readCertificates(String in) {
        return readCertificates(new StringReader(in));
    }

    public static X500Name x500(String txt) {
        return X500Name.getInstance(new X509Principal(txt).getEncoded());
    }

    public static X509Certificate generateCert(X509v3CertificateBuilder gen, String algorithm, PrivateKey key)
        throws GeneralSecurityException {
        ContentSigner sigGen;
        try {
            sigGen = new JcaContentSignerBuilder(algorithm).setProvider(PROVIDER).build(key);
            JcaX509CertificateConverter converter = new JcaX509CertificateConverter().setProvider(PROVIDER);
            return converter.getCertificate(gen.build(sigGen));
        } catch (OperatorCreationException e) {
            throw new GeneralSecurityException(e);
        } catch (CertificateException e) {
            throw new GeneralSecurityException(e);
        }
    }

    public static List<Object> readObject(Reader in) {
        PEMReader rdr = new PEMReader(in);
        List<Object> list = new ArrayList<Object>();
        try {
            for (int i = 0; i < MAX_HEADER_LINE_COUNT; i++) {
                Object o = rdr.readObject();
                if (o != null) {
                    list.add(o);
                }
            }
            rdr.close();
            if (!list.isEmpty()) {
                return list;
            }
        } catch (IOException e) {
            throw new UserException("Error reading certificate. " + e.getMessage(), e);
        }

        throw new UserException("No recognized security information was found. Files "
                + "should be in PEM style format.");
    }

    public static PrivateKey readCertificateKey(String in) {
        return readCertificateKey(new StringReader(in));
    }

    public static PrivateKey readCertificateKey(Reader in) {
        Object o = readObject(in).get(0);
        if (o instanceof KeyPair) {
            return ((KeyPair) o).getPrivate();
        }
        if (o instanceof PrivateKey) {
            return (PrivateKey) o;
        }

        String msg = format("Private key was expected but found %s instead", o.getClass().getSimpleName());
        throw new UserException(msg);
    }

    public static String convertSslKeyToRSA(File file) {
        Runtime runtime = Runtime.getRuntime();
        try {
            Process process = runtime.exec("openssl rsa -in " + file.getAbsolutePath() + " -check");
            String result = IOUtils.toString(process.getInputStream());
            return StringUtils.join(new String []{START_RSA_KEY, StringUtils.substringAfter(result, START_RSA_KEY)});
        } catch (Exception ex) {
            Log.error("Cannot Convert key to RSA ", ex);
            return null;
        }
    }

    public static void writeObject(Writer w, Object o, String description) {
        PEMWriter pw = new PEMWriter(w);
        try {
            if (description != null) {
                w.write(description);
            }
            pw.writeObject(o);
            pw.close();
        } catch (IOException e) {
            throw new UserException("Problem updating certificate authority. " + e.getMessage(), e);
        }
    }

    static String stripPath(String s) {
        if (StringUtils.isBlank(s)) {
            return s;
        }
        int i = s.lastIndexOf('/');
        if (i < 0) {
            return s;
        }
        if (i  + 1 == s.length()) {
            return StringUtils.EMPTY;
        }
        return s.substring(i + 1);
    }
}
