/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.openfire.vcard.provider;

import java.awt.image.BufferedImage;
import java.io.BufferedReader;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.io.StringReader;
import java.net.ConnectException;
import java.net.HttpURLConnection;
import java.net.MalformedURLException;
import java.net.URL;
import java.util.List;

import javax.imageio.ImageIO;
import javax.net.ssl.HttpsURLConnection;
import javax.net.ssl.SSLContext;
import javax.net.ssl.TrustManager;
import javax.net.ssl.X509TrustManager;
import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.stream.StreamSource;

import org.apache.commons.codec.binary.Base64;
import org.dom4j.Document;
import org.dom4j.DocumentHelper;
import org.dom4j.Element;
import org.dom4j.Node;
import org.dom4j.io.DocumentResult;
import org.dom4j.io.DocumentSource;
import org.dom4j.io.SAXReader;
import org.jivesoftware.util.Log;
import org.sipfoundry.openfire.vcard.provider.SipXVCardProvider;

public class RestInterface {

    public static final String REST_CALL_PROTO = "https://";
    public static final String REST_CALL_URL_CONTACT_INFO = "/sipxconfig/rest/my/contact-information";
    public static final String REST_CALL_PORT = "8443";
    public static final int SSL_CONNECTION_TIMEOUT = 10000; // milliseconds
    public static final int CONNECTION_TIMEOUT = 30000; // milliseconds
    public static final int READ_TIMEOUT = 30000; // milliseconds

    private static RestInterface instance = null;

    synchronized public static RestInterface getInstance() {
        if (instance == null)
            instance = new RestInterface();

        return instance;

    }

    public static String sendRequest(String method, String sipXserver, String username, String password,
            Element vcardElement) throws ConnectException {
        try {
            StringBuilder urlStr = new StringBuilder().append(REST_CALL_PROTO).append(sipXserver).append(":")
                    .append(REST_CALL_PORT).append(REST_CALL_URL_CONTACT_INFO);
            Log.debug("call REST URL " + urlStr.toString() + "[username:" + username + "]" );
            URL serverURL = new URL(urlStr.toString());
            HttpsURLConnection conn = (HttpsURLConnection) serverURL.openConnection();
            conn.setConnectTimeout(SSL_CONNECTION_TIMEOUT);
            conn.setReadTimeout(READ_TIMEOUT);

            conn.setDoOutput(true);
            conn.setRequestMethod(method);

            String val = (new StringBuilder(username).append(":").append(password)).toString();
            byte[] base = val.getBytes();
            String authorizationString = "Basic " + new String(new Base64().encode(base));
            conn.setRequestProperty("Authorization", authorizationString);

            if (vcardElement != null) {
                conn.setDoInput(true);
                conn.setRequestProperty("Content-type", "text/xml");
                OutputStreamWriter wr = new OutputStreamWriter(conn.getOutputStream());
                String vcardXml = RestInterface.buildXMLContactInfo(vcardElement);
                String contactXml = RestInterface.sendRequest(SipXVCardProvider.QUERY_METHOD, sipXserver, username,
                        password, null);
                if (contactXml != null) {
                    vcardXml = refillMissingContactInfo(vcardXml, contactXml);
                }

                wr.write(vcardXml);

                wr.flush();
                wr.close();
            }

            conn.connect();

            Log.debug("response code " + conn.getResponseCode() + " response message " + conn.getResponseMessage());

            BufferedReader rd = new BufferedReader(new InputStreamReader(conn.getInputStream()));
            StringBuilder resp = new StringBuilder();
            String line;
            while ((line = rd.readLine()) != null) {
                resp.append(line);
            }
            rd.close();

            if (conn.getResponseCode() >= 200 && conn.getResponseCode() < 300) {
                return resp.toString();
            } else {
                Log.error("Response code " + conn.getResponseCode() + ":" + conn.getResponseMessage() + " "
                        + resp.toString());
                return null;
            }
        }

        catch (ConnectException ex) {
            Log.error("In sendRequest ConnectException " + ex.getMessage());
            throw ex;
        }

        catch (IOException ex) {
            Log.error("In sendRequest IOException " + ex.getMessage());
            return null;
        }

        catch (Exception ex) {
            Log.error("Exception " + ex.getMessage());
            return null;
        }
    }

    public String buildXMLContactInfoXSLT(Element e) {
        try {
            String x = e.asXML().replace("xmlns=\"vcard-temp\"", ""); // xmlns causes dom4j xpath
            // not working somehow.
            Document vcardDoc = DocumentHelper.parseText(x);

            Log.debug("before XSLT " + vcardDoc.getRootElement().asXML());
            InputStream inStream = this.getClass().getResourceAsStream("/contactInfo.xsl");
            Document contactDoc = styleDocument(vcardDoc, inStream);

            Log.debug("After XSLT " + contactDoc.getRootElement().asXML());
            return contactDoc.getRootElement().asXML();
        } catch (Exception ex) {
            Log.error(ex.getMessage());
            return null;
        }

    }

    public static String buildXMLContactInfo(Element e) {
        try {
            String x = e.asXML().replace("xmlns=\"vcard-temp\"", ""); // xmlns causes dom4j xpath
            // not working somehow.

            Log.debug("In buildXMLContactInfo vcard string is " + x);
            Document vcardDoc = DocumentHelper.parseText(x);
            Element el = vcardDoc.getRootElement();

            StringBuilder xbuilder = new StringBuilder("<contact-information>");

            xbuilder.append("<firstName>");
            xbuilder.append(getNodeText(el, "N/GIVEN"));
            xbuilder.append("</firstName>");
            xbuilder.append("<lastName>");
            xbuilder.append(getNodeText(el, "N/FAMILY"));
            xbuilder.append("</lastName>");
            xbuilder.append("<jobTitle>");
            xbuilder.append(getNodeText(el, "TITLE"));
            xbuilder.append("</jobTitle>");
            xbuilder.append("<jobDept>");
            xbuilder.append(getNodeText(el, "ORG/ORGUNIT"));
            xbuilder.append("</jobDept>");
            xbuilder.append("<companyName>");
            xbuilder.append(getNodeText(el, "ORG/ORGNAME"));
            xbuilder.append("</companyName>");

            xbuilder.append("<officeAddress>");
            xbuilder.append("<street>");
            xbuilder.append(getNodeText(el, "ADR/STREET"));
            xbuilder.append("</street>");
            xbuilder.append("<city>");
            xbuilder.append(getNodeText(el, "ADR/LOCALITY"));
            xbuilder.append("</city>");
            xbuilder.append("<country>");
            xbuilder.append(getNodeText(el, "ADR/CTRY"));
            xbuilder.append("</country>");
            xbuilder.append("<state>");
            xbuilder.append(getNodeText(el, "ADR/REGION"));
            xbuilder.append("</state>");
            xbuilder.append("<zip>");
            xbuilder.append(getNodeText(el, "ADR/PCODE"));
            xbuilder.append("</zip>");
            xbuilder.append("</officeAddress>");

            /*
             * if (!(getNodeText(el, "JABBERID").equals("unknown"))) { xbuilder.append("<imId>");
             * xbuilder.append(getNodeText(el, "JABBERID"));xbuilder.append("</imId>"); }
             */
            xbuilder.append("<emailAddress>");
            xbuilder.append(getNodeText(el, "EMAIL/USERID"));
            xbuilder.append("</emailAddress>");
            xbuilder.append("</contact-information>");

            Log.debug("contact info generated by buildXMLContactInfo is " + xbuilder.toString());

            return xbuilder.toString();
        } catch (Exception ex) {
            Log.error(ex.getMessage());
            return null;
        }

    }

    public Element buildVCardFromXMLContactInfoXSLT(String xmlString) {
        try {
            Document contactDoc = DocumentHelper.parseText(xmlString);
            InputStream inStream = this.getClass().getResourceAsStream("/vCardTemp.xsl");

            Log.debug("before XSLT " + contactDoc.getRootElement().asXML());
            Document vcardDoc = styleDocument(contactDoc, inStream);
            Log.debug("After XSLT " + vcardDoc.getRootElement().asXML());

            return vcardDoc.getRootElement();
        } catch (Exception ex) {
            Log.error(ex.getMessage());
            return null;
        }

    }

    //
    // Refill the contact information available in sipX but not mentioned in the vcard update
    // request from XMM client.
    // Otherwise, those information will be deleted by sipX.
    //
    public static String refillMissingContactInfo(String vcardXml, String contactXml) {
        try {
            SAXReader sreader = new SAXReader();

            Document contactDoc = sreader.read(new StringReader(contactXml));
            Element contactRootElement = contactDoc.getRootElement();

            Document vcardDoc = sreader.read(new StringReader(vcardXml));
            Element vcardRootElement = vcardDoc.getRootElement();

            for (Element el : (List<Element>) contactRootElement.elements()) {
                Element vElement;
                if ((vElement = vcardRootElement.element(el.getName())) == null) {
                    Log.debug(" In refillMissingContactInfo Element = [" + el.getName() + "] not found!");
                    vcardRootElement.add(el.createCopy());
                } else {
                    String newStr = refillMissingContactInfo(vElement.asXML(), el.asXML());
                    if ((newStr.compareTo(vElement.asXML()) != 0)) {
                        Document vcardSubDoc = sreader.read(new StringReader(newStr));
                        Element vcardSubRootElement = vcardSubDoc.getRootElement();
                        vcardRootElement.remove(vElement);
                        vcardRootElement.add(vcardSubRootElement.createCopy());
                    }
                }
            }
            Log.debug("vcard XML string after refill is " + vcardRootElement.asXML());
            return vcardRootElement.asXML();
        } catch (Exception ex) {
            Log.error(ex.getMessage());
            return null;
        }

    }

    public static Element buildVCardFromXMLContactInfo(String userName, String xmlstring, Element avatarFromDB) {
        try {
            SAXReader sreader = new SAXReader();

            Log.debug("In buildVCardFromXMLContactInfo contactInfo is " + xmlstring);

            Document contactDoc = sreader.read(new StringReader(xmlstring));
            Element rootElement = contactDoc.getRootElement();

            StringBuilder xbuilder = new StringBuilder("<vCard xmlns='vcard-temp'>");

            xbuilder.append("<FN>");
            xbuilder.append(getNodeText(rootElement, "firstName") + " " + getNodeText(rootElement, "lastName"));
            xbuilder.append("</FN>");
            xbuilder.append("<N>");
            xbuilder.append("<FAMILY>");
            xbuilder.append(getNodeText(rootElement, "lastName"));
            xbuilder.append("</FAMILY>");
            xbuilder.append("<GIVEN>");
            xbuilder.append(getNodeText(rootElement, "firstName"));
            xbuilder.append("</GIVEN>");
            xbuilder.append("<MIDDLE/>");
            xbuilder.append("</N>");

            xbuilder.append("<NICKNAME>");
            xbuilder.append("");
            xbuilder.append("</NICKNAME>");
            xbuilder.append("<URL>");
            xbuilder.append("");
            xbuilder.append("</URL>");
            xbuilder.append("<BDAY>");
            xbuilder.append("");
            xbuilder.append("</BDAY>");
            xbuilder.append("<ORG>");
            xbuilder.append("<ORGNAME>");
            xbuilder.append(getNodeText(rootElement, "companyName"));
            xbuilder.append("</ORGNAME>");
            xbuilder.append("<ORGUNIT>");
            xbuilder.append(getNodeText(rootElement, "jobDept"));
            xbuilder.append("</ORGUNIT>");
            xbuilder.append("</ORG>");
            xbuilder.append("<TITLE>");
            xbuilder.append(getNodeText(rootElement, "jobTitle"));
            xbuilder.append("</TITLE>");
            xbuilder.append("<ROLE/>");
            xbuilder.append("<TEL>");
            xbuilder.append("<WORK/>");
            xbuilder.append("<VOICE/>");
            xbuilder.append("<NUMBER>");
            xbuilder.append(userName + "(Work)");
            xbuilder.append("</NUMBER>");
            xbuilder.append("</TEL>");

            xbuilder.append("<TEL>");
            xbuilder.append("<WORK/>");
            xbuilder.append("<FAX/>");
            xbuilder.append("<NUMBER>");
            xbuilder.append(getNodeText(rootElement, "faxNumber") + "(FAX)");
            xbuilder.append("</NUMBER>");
            xbuilder.append("</TEL>");

            xbuilder.append("<TEL>");
            xbuilder.append("<WORK/>");
            xbuilder.append("<MSG/>");
            xbuilder.append("<NUMBER/>");
            xbuilder.append("</TEL>");

            xbuilder.append("<ADR>");
            xbuilder.append("<WORK/>");
            xbuilder.append("<EXTADD>");
            xbuilder.append("</EXTADD>");
            xbuilder.append("<STREET>");
            xbuilder.append(getNodeText(rootElement, "officeAddress/street"));
            xbuilder.append("</STREET>");
            xbuilder.append("<LOCALITY>");
            xbuilder.append(getNodeText(rootElement, "officeAddress/city"));
            xbuilder.append("</LOCALITY>");
            xbuilder.append("<REGION>");
            xbuilder.append(getNodeText(rootElement, "officeAddress/state"));
            xbuilder.append("</REGION>");
            xbuilder.append("<PCODE>");
            xbuilder.append(getNodeText(rootElement, "officeAddress/zip"));
            xbuilder.append("</PCODE>");
            xbuilder.append("<CTRY>");
            xbuilder.append(getNodeText(rootElement, "officeAddress/country"));
            xbuilder.append("</CTRY>");
            xbuilder.append("</ADR>");

            xbuilder.append("<TEL>");
            xbuilder.append("<HOME/>");
            xbuilder.append("<VOICE/>");
            xbuilder.append("<NUMBER>");
            xbuilder.append(getNodeText(rootElement, "homePhoneNumber") + "(Home)");
            xbuilder.append("</NUMBER>");
            xbuilder.append("</TEL>");

            xbuilder.append("<TEL>");
            xbuilder.append("<HOME/>");
            xbuilder.append("<FAX/>");
            xbuilder.append("<NUMBER/>");
            xbuilder.append("</TEL>");

            xbuilder.append("<TEL>");
            xbuilder.append("<CELL/>");
            xbuilder.append("<VOICE/>");
            xbuilder.append("<NUMBER>");
            xbuilder.append(getNodeText(rootElement, "cellPhoneNumber") + "(Mobile)");
            xbuilder.append("</NUMBER>");
            xbuilder.append("</TEL>");

            xbuilder.append("<TEL>");
            xbuilder.append("<HOME/>");
            xbuilder.append("<MSG/>");
            xbuilder.append("<NUMBER/>");
            xbuilder.append("</TEL>");

            xbuilder.append("<ADR>");
            xbuilder.append("<HOME/>");
            xbuilder.append("<EXTADD/>");
            xbuilder.append("<STREET>");
            xbuilder.append(getNodeText(rootElement, "homeAddress/street"));
            xbuilder.append("</STREET>");
            xbuilder.append("<LOCALITY>");
            xbuilder.append(getNodeText(rootElement, "homeAddress/city"));
            xbuilder.append("</LOCALITY>");
            xbuilder.append("<REGION>");
            xbuilder.append(getNodeText(rootElement, "homeAddress/state"));
            xbuilder.append("</REGION>");
            xbuilder.append("<PCODE>");
            xbuilder.append(getNodeText(rootElement, "homeAddress/zip"));
            xbuilder.append("</PCODE>");
            xbuilder.append("<CTRY>");
            xbuilder.append(getNodeText(rootElement, "homeAddress/country"));
            xbuilder.append("</CTRY>");
            xbuilder.append("</ADR>");

            xbuilder.append("<EMAIL>");
            xbuilder.append("<INTERNET/>");
            xbuilder.append("<PREF/>");
            xbuilder.append("<USERID>");
            xbuilder.append(getNodeText(rootElement, "emailAddress"));
            xbuilder.append("</USERID>");
            xbuilder.append("</EMAIL>");

            xbuilder.append("<JABBERID>");
            xbuilder.append(getNodeText(rootElement, "imId"));
            xbuilder.append("</JABBERID>");
            xbuilder.append("<DESC/>");

            if (avatarFromDB == null) {
                String encodedStr = getEncodedAvatar(getNodeText(rootElement, "avatar"));
                if (encodedStr != null) {
                    xbuilder.append("<PHOTO>");
                    xbuilder.append("<TYPE>image/png</TYPE>");
                    xbuilder.append("<BINVAL>");
                    xbuilder.append(encodedStr);
                    xbuilder.append("</BINVAL>");
                    xbuilder.append("</PHOTO>");
                }
            }

            xbuilder.append("</vCard>");

            // The following are Not supported by XMPP vcard temp XEP-0054 so far.
            /*
             * <contact-information> <assistantName>adfafd</assistantName>
             * <location>afaf</location> <assistantPhoneNumber>afdadf</assistantPhoneNumber>
             * <imDisplayName>201_IM_test</imDisplayName> <alternateImId>afa</alternateImId>
             * <alternateEmailAddress>afafd</alternateEmailAddress>
             * </contact-information>-bash-3.2$
             */

            Log.debug("vcard generated by buildVCardFromXMLContactinfo is " + xbuilder.toString());
            Document vcardDoc = sreader.read(new StringReader(xbuilder.toString()));
            Element vcardNode = vcardDoc.getRootElement();

            if (avatarFromDB != null) {
                vcardNode.add(avatarFromDB);
            }

            return vcardNode;
        }

        catch (Exception ex) {
            Log.error(ex.getMessage());
            return null;
        }

    }

    public static String getNodeText(Element element, String nodeName) {
        Node node = element.selectSingleNode(nodeName);
        if (node != null) {
            return node.getText();
        }

        return "";
    }

    public static String getTextFromNodes(Element element, String nameNode, String criteriaNode, String valueNode) {
        List nlist = element.selectNodes(nameNode);
        for (int i = 0; i < nlist.size(); i++) {
            Element el = (Element) (nlist.get(i));
            Node cNode = el.selectSingleNode(criteriaNode);
            if (cNode != null) {
                Node vNode = el.selectSingleNode(valueNode);
                if (vNode != null) {
                    return vNode.getText();
                }
            }

        }

        return "";
    }

    public static Document styleDocument(Document document, InputStream stylesheet) throws Exception {

        // load the transformer using JAXP
        TransformerFactory factory = TransformerFactory.newInstance();
        Transformer transformer = factory.newTransformer(new StreamSource(stylesheet));

        // now lets style the given document
        DocumentSource source = new DocumentSource(document);
        DocumentResult result = new DocumentResult();
        transformer.transform(source, result);

        // return the transformed document
        Document transformedDoc = result.getDocument();
        return transformedDoc;
    }

    public static String getEncodedAvatar(String avatarURL) {
        Log.debug("Avatar URL " + avatarURL);
        return getPngStringTimeout(avatarURL);
    }

    public static String getPngString(URL url) {
        try {
            ByteArrayOutputStream os = new ByteArrayOutputStream();
            BufferedImage image = ImageIO.read(url);
            ImageIO.write(image, "png", os);

            return new String(new Base64().encode(os.toByteArray()));
        } catch (IOException e) {
            Log.error("In getPngString, error:" + e.getMessage());
            return null;
        } catch (Exception e) {
            Log.error(e.getMessage());
            return null;
        }
    }

    public static String getPngStringTimeout(String urlStr) {
        try {
            URL url = new URL(urlStr);
            HttpURLConnection conn = (HttpURLConnection) url.openConnection();
            if (url.getProtocol().equalsIgnoreCase("https")) {
                TrustManager[] trustAllCerts = new TrustManager[] {
                    new X509TrustManager() {
                        public java.security.cert.X509Certificate[] getAcceptedIssuers() {
                            return null;
                        }
                        public void checkClientTrusted(java.security.cert.X509Certificate[] certs, String authType) {
                        }
                        public void checkServerTrusted(java.security.cert.X509Certificate[] certs, String authType) {
                        }
                    }
                };

                SSLContext sc = SSLContext.getInstance("TLS");
                sc.init(null, trustAllCerts, new java.security.SecureRandom());
                ((HttpsURLConnection) conn).setSSLSocketFactory(sc.getSocketFactory());
            }
            conn.setConnectTimeout(CONNECTION_TIMEOUT);
            conn.setReadTimeout(READ_TIMEOUT);
            conn.setRequestMethod("GET");

            conn.connect();

            ByteArrayOutputStream os = new ByteArrayOutputStream();
            BufferedImage image = ImageIO.read(conn.getInputStream());
            ImageIO.write(image, "png", os);

            return new String(new Base64().encode(os.toByteArray()));
        } catch (MalformedURLException e) {
            Log.error("In getPngStringTimeout, MalformedURLException:" + e.getMessage());
            return null;
        } catch (IOException e) {
            Log.error("In getPngStringTimeout, IOException:" + e.getMessage());
            return null;
        } catch (Exception e) {
            Log.error("In getPngStringTimeout, Exception:" + e.getMessage());
            return null;
        }

    }
}
