/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.cmcprov;

import java.io.BufferedReader;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.PrintWriter;
import java.net.URLDecoder;
import java.util.Hashtable;
import java.util.List;
import java.util.Map;
import java.util.Properties;
import java.util.HashMap;
import java.io.File;
import java.util.Scanner;
import java.util.ArrayList;
import java.io.BufferedWriter;
import java.io.FileWriter;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.device.Profile;
import org.sipfoundry.sipxconfig.phone.Phone;

public class LoginServlet extends ProvisioningServlet {
    public static final String PARAMETER_ENCODING = "UTF-8";
    public static final String USERNAME_PASSWORD_CANNOT_BE_MISSING_ERROR = "Username and password cannot be missing";
    public static final String USERNAME_PASSWORD_ARE_INVALID_ERROR = "Either username or password are invalid";
    public static final String INVALID_CREDIDENTIALS = "Your credentials are not recognized";

    private static final Log LOG = LogFactory.getLog(LoginServlet.class);
    private static final String WWW_DIR_PROPERTY = "www.dir";
    private static final String PHONE_DIR_PROPERTY = "sipxconfig.phone.dir";
    private static final String CONF_RESOURCE = "/counterpath/cmcprov.properties";
    private static final String CONTACTS_LIST_FILE_SUBFIX = "-directory.xml";
    private static final String WEBDAV_DIR = "/webdav/";
    private static final String TFTP_RELATIVE_PATH = "/profile/tftproot/";
    private static final String PRESENCE_AGENT = "presenceAgent";
    private static final String OUTPUT_TYPE = "output_type";
    private static final String BRIA_MOBILE = "-briamobile.xml";
    private static final String USERNAME = "username";
    private static final String PASSWORD = "password";
    private static final String SILK_1600 = "SILK/1600";
    private static final String SILK_8000 = "SILK/8000";
    private static final String GSM = "GSM";
    private static final String DISPLAY = "display";
    private static final String ACCOUNT_NAME = "accountName";
    private static final String DOMAIN = "domain";
    private static final String PRESENCE = "presence";
    private static final String ENABLED = "enabled";
    private static final String PROTOCOL = "protocol";
    private static final String STUN_SERVER = "stunServer";
    private static final String CODECS = "codecs";
    private static final String PRIORITY = "priority";
    private static final String INI = ".ini";
    private static final String XML = "xml";
    private static final String DOT = ".";
    private static final String EMPTY_STRING = "";
    private static final String DOUBLE_QUOTE = "\"";
    private static final String FULL_STOP = "\\.";
    private static final String VALUE = "\" value=";
    private static final String TAG_CLOSE = "/>\n";
    private static final String CODEC_START_TAG = "\t\t\t\t\t<codec name=\"";

    @Override
    protected void doPost(HttpServletRequest req, HttpServletResponse resp) throws javax.servlet.ServletException,
            java.io.IOException {
        PrintWriter out = resp.getWriter();
        String reqType = EMPTY_STRING;
        try {
            Map<String, String> parameters;
            try {
                parameters = getParameterMapFromBody(req);
            } catch (java.io.IOException error) {
                throw new FailureDataException("Cannot extract parameters");
            }
            reqType = parameters.get(OUTPUT_TYPE);
            User user = authenticateRequest(parameters);
            Phone phone = getProvisioningContext().getPhoneForUser(user);
            if (phone == null) {
                throw new FailureDataException(INVALID_CREDIDENTIALS);
            }
            Profile[] profileFilenames = phone.getProfileTypes();

            FileInputStream fin = new FileInputStream(getProvisioningContext().getConfDir() + CONF_RESOURCE);
            Properties properties = new Properties();

            try {
                properties.load(fin);
            } catch (IOException ex) {
                LOG.error("loading error:  " + CONF_RESOURCE + ex.getMessage());
            }

            fin.close();

            updateContactList(user, phone, properties.getProperty(WWW_DIR_PROPERTY),
                    properties.getProperty(PHONE_DIR_PROPERTY));

            if (reqType == XML) {
                String uploadDirectory = getProvisioningContext().getUploadDirectory();
                translateINItoXML(new File(uploadDirectory, profileFilenames[0].getName()));
                attachFile(new File(uploadDirectory, profileFilenames[0].getName().replaceAll(INI, BRIA_MOBILE)),
                        out);
            } else {
                uploadPhoneProfile(profileFilenames[0].getName(), out);
            }

        } catch (FailureDataException e) {
            LOG.error("Login error: " + e.getMessage());
            if (reqType == XML) {
                out.write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<cpc_mobile version=\"1.0\">\n<login_response>\n"
                        + "<status success=\"false\" error_text=\"No match for username and password.\"/>\n"
                        + "</login_response>\n</cpc_mobile>");
            } else {
                buildFailureResponse(out, e.getMessage());
            }
        }
    }

    protected User authenticateRequest(Map<String, String> parameters) {
        /*
         * Authenticates a login requests. Returns a User object representing the authenticated
         * user if succesful, otherwise throws a Exception.
         */
        User user;
        String username;
        String password;
        if (!parameters.containsKey(USERNAME) || !parameters.containsKey(PASSWORD)) {
            throw new FailureDataException(USERNAME_PASSWORD_CANNOT_BE_MISSING_ERROR);
        }
        username = parameters.get(USERNAME);
        password = parameters.get(PASSWORD);
        user = getProvisioningContext().getUser(username, password);
        if (user == null) {
            throw new FailureDataException(USERNAME_PASSWORD_ARE_INVALID_ERROR);
        }
        return user;
    }

    protected static Map<String, String> getParameterMapFromBody(HttpServletRequest req) throws java.io.IOException {
        Hashtable<String, String> parameters = new Hashtable<String, String>();
        BufferedReader br = req.getReader();
        String line = br.readLine();
        if (line == null) {
            return parameters;
        }

        if (!line.contains("<?xml version=")) {
            String[] pairs = line.split("\\&");
            for (int i = 0; i < pairs.length; i++) {
                String[] fields = pairs[i].split("=");
                if (fields.length == 2) {
                    String name = URLDecoder.decode(fields[0], PARAMETER_ENCODING);
                    String value = URLDecoder.decode(fields[1], PARAMETER_ENCODING);
                    parameters.put(name.toLowerCase(), value);
                }
            }
            parameters.put(OUTPUT_TYPE, "txt");
        } else {
            String fulltext = line;
            while (br.ready()) {
                line = br.readLine();
                fulltext = fulltext + line;
            }
            int usrindex = fulltext.indexOf("user=\"");
            parameters.put(
                    USERNAME,
                    fulltext.substring(usrindex + 6, fulltext.substring(usrindex + 6).indexOf(DOUBLE_QUOTE)
                            + usrindex + 6));
            int passwdindex = fulltext.indexOf("password=\"");
            parameters.put(
                    PASSWORD,
                    fulltext.substring(passwdindex + 10, fulltext.substring(passwdindex + 10).indexOf(DOUBLE_QUOTE)
                            + passwdindex + 10));
            parameters.put(OUTPUT_TYPE, XML);
        }

        return parameters;
    }

    private void updateContactList(User user, Phone phone, String wwwdir, String phonedir) {
        String domainName = this.getProvisioningContext().getDomainName();
        String phoneBookName = phone.getSerialNumber() + CONTACTS_LIST_FILE_SUBFIX;
        String contactListFilePath = wwwdir + WEBDAV_DIR + user.getUserName() + DOT + domainName + DOT
                + phoneBookName;
        String phoneBookFilePath = phonedir + TFTP_RELATIVE_PATH + phoneBookName;

        ContactSynchronizer synchronizer = ContactSynchronizer.getInstance(phoneBookFilePath, contactListFilePath);
        synchronizer.synChronize();
    }

    private static void translateINItoXML(File inputfile) {
        String inputfilename = inputfile.getPath();
        String outputfilename = inputfilename.replaceAll(INI, BRIA_MOBILE);
        LOG.debug("Input File is: " + inputfilename + "\nOutput File is: " + outputfilename);

        Map<String, String> validAudioCodecs = buildAudioCodecsMap();
        List<String> validVideoCodecs = buildVideoCodecsList();
        Map<String, String> parameterTranslation = buildParameterTranslationMap();
        Map<String, String> coreTranslation = buildCoreTranslationMap();

        File outputfile = new File(outputfilename);
        if (!inputfile.exists()) {
            LOG.error("Input File is non existant");
        } else if (outputfile.exists() && (inputfile.lastModified() < outputfile.lastModified())) {
            LOG.debug("Outfile found to be newer than input file... no changes will be made.");
        } else {
            LOG.debug("Outfile found to be older than input file or non existant... begin update sequence.");
            // Write out header
            BufferedWriter bw = null;
            try {
                LOG.debug("Writing header.");
                bw = new BufferedWriter(new FileWriter(outputfile));
                bw.write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<cpc_mobile version=\"1.0\">\n"
                        + "<login_response>\n\t<status success=\"true\"/>\n</login_response>\n"
                        + "<branding>\n\t<settings_data>\n\t\t<core_data_list>\n");
            } catch (IOException e) {
                LOG.error("XMLTranlation: Failed to write xml header." + e.getMessage());
            }

            List<String> audiocodecs = new ArrayList<String>();
            Map<Integer, String> audiocodecsPriority = new HashMap<Integer, String>();
            List<String> videocodecs = new ArrayList<String>();
            Map<Integer, String> videocodecsPriority = new HashMap<Integer, String>();
            List<Map<String, String>> proxyList = new ArrayList<Map<String, String>>();
            String proxyString = EMPTY_STRING;
            Map<String, String> currentProxy = null;
            Map<String, String> coreSettings = new HashMap<String, String>();

            // Ingest INI File
            try {
                LOG.debug("INFO: Ingesting ini file.");
                Scanner s = new Scanner(inputfile);
                while (s.hasNextLine()) {
                    String line = s.nextLine();
                    if (line.contains(":")) {
                        String[] splitline = line.split(":|=", 4);
                        if (splitline[0].equals(CODECS) && splitline[2].equals(PRIORITY)
                                && validAudioCodecs.containsKey(splitline[1])) {
                            int priority = Integer.parseInt(splitline[3].substring(1, splitline[3].length() - 1)
                                    .split(FULL_STOP, 2)[0]);
                            audiocodecsPriority.put(priority, validAudioCodecs.get(splitline[1]));
                            audiocodecs.add(validAudioCodecs.get(splitline[1]));
                        }
                        if (splitline[0].equals(CODECS) && splitline[2].equals(ENABLED)
                                && validAudioCodecs.containsKey(splitline[1]) && splitline[3].equals("\"true\"")) {
                            audiocodecs.add(validAudioCodecs.get(splitline[1]));
                        }
                        if (splitline[0].equals(CODECS) && splitline[2].equals(PRIORITY)
                                && validVideoCodecs.contains(splitline[1])) {
                            int priority = Integer.parseInt(splitline[3].substring(1, splitline[3].length() - 1)
                                    .split(FULL_STOP, 2)[0]);
                            videocodecsPriority.put(priority, splitline[1]);
                        }
                        if (splitline[0].equals(CODECS) && splitline[2].equals(ENABLED)
                                && validVideoCodecs.contains(splitline[1]) && splitline[3].equals("\"true\"")) {
                            videocodecs.add(splitline[1]);
                        }
                        if (splitline[0].equals("proxies")) {
                            // Handle parameters used with setting up accounts.
                            if (!proxyString.equals(splitline[1])) {
                                if (!proxyString.equals(EMPTY_STRING)) {
                                    proxyList.add(currentProxy);
                                }
                                currentProxy = new HashMap<String, String>();
                                proxyString = splitline[1];
                            }
                            if (coreTranslation.containsKey(splitline[2])) {
                                coreSettings.put(coreTranslation.get(splitline[2]), splitline[3]);
                            } else if (parameterTranslation.containsKey(splitline[2])) {
                                currentProxy.put(parameterTranslation.get(splitline[2]), splitline[3]);
                            }
                            // else
                            // currentProxy.put(splitline[2],splitline[3]);
                        }
                    }
                }
            } catch (IOException e) {
                LOG.error("XMLTranslation: Failed to ingest ini file.\n" + e.getMessage());
            }

            writeCoreSettings(bw, coreSettings);
            writeCodecs(bw, audiocodecs, audiocodecsPriority, videocodecs, videocodecsPriority);
            writeAccount(bw, proxyList);

            // Write out footer
            try {
                LOG.debug("Writing footer.");
                bw.write("\t\t</core_data_list>\n\t</settings_data>\n</branding>\n</cpc_mobile>");
                bw.close();
            } catch (IOException e) {
                LOG.error("Failed to write xml footer" + e.getMessage());
            }
        }
        LOG.debug("XML Translation File Creation Complete");
    }

    private static void writeCoreSettings(BufferedWriter bw, Map<String, String> coreSettings) {
        // Write out core settings list
        try {
            LOG.debug("Writing core settings list. ");

            // Add custom modifications for core settings
            String tval = "\"false\"";
            if (!coreSettings.get(STUN_SERVER).equals(EMPTY_STRING)) {
                tval = "\"true\"";
            }
            coreSettings.put("useStun", tval);
            coreSettings.put("useStun3G", tval);

            for (String key : coreSettings.keySet()) {
                bw.write("\t\t\t<data name=\"" + key + VALUE + coreSettings.get(key) + TAG_CLOSE);
            }
        } catch (IOException e) {
            LOG.error("Failed to write core settings list" + e.getMessage());
        }
    }

    private static void writeCodecs(BufferedWriter bw, List<String> audiocodecs,
            Map<Integer, String> audiocodecsPriority, List<String> videocodecs,
            Map<Integer, String> videocodecsPriority) {
        // Write out codecs list
        try {
            LOG.debug("Writing codec list... ");
            bw.write("\t\t\t<codec_list>\n\t\t\t\t<codec_list_audio_wifi>\n");
            for (int i = audiocodecsPriority.size(); i >= 0; i--) {
                if (audiocodecs.contains(audiocodecsPriority.get(i))) {
                    bw.write(CODEC_START_TAG + audiocodecsPriority.get(i) + "\" enabled=\"true\"/>\n");
                }
            }
            bw.write("\t\t\t\t</codec_list_audio_wifi>\n");
            bw.write("\t\t\t\t<codec_list_audio_cell>\n");
            for (int i = audiocodecsPriority.size(); i >= 0; i--) {
                if (audiocodecs.contains(audiocodecsPriority.get(i))) {
                    bw.write(CODEC_START_TAG + audiocodecsPriority.get(i) + "\" enabled=\"true\"/>\n");
                }
            }
            bw.write("\t\t\t\t</codec_list_audio_cell>\n");
            bw.write("\t\t\t\t<codec_list_video_wifi>\n");
            for (String videocodec : videocodecs) {
                bw.write(CODEC_START_TAG + videocodec + "\" enabled=\"true\"/>\n");
            }
            bw.write("\t\t\t\t</codec_list_video_wifi>\n");
            bw.write("\t\t\t</codec_list>\n");
        } catch (IOException e) {
            LOG.error("Failed to write codec list" + e.getMessage());
        }
    }

    private static void writeAccount(BufferedWriter bw, List<Map<String, String>> proxyList) {
        // Write out account list
        try {
            LOG.debug("Writing account list.");
            bw.write("\t\t\t<account_list>\n");

            // Iterate through each account
            for (Map<String, String> account : proxyList) {
                if (account.get(ENABLED).equals("\"false\"")) {
                    break;
                }
                if (account.get(PROTOCOL) == null) {
                    account.put(PROTOCOL, "\"sip\"");
                }
                if (account.get(DISPLAY) == null) {
                    account.put(DISPLAY, account.get(PROTOCOL));
                }
                if (account.get(ACCOUNT_NAME) == null) {
                    account.put(ACCOUNT_NAME,
                            DOUBLE_QUOTE + account.get(DISPLAY).replace(DOUBLE_QUOTE, EMPTY_STRING) + " - "
                                    + account.get(PROTOCOL).replace(DOUBLE_QUOTE, EMPTY_STRING) + DOUBLE_QUOTE);
                }
                if (account.containsKey(PRESENCE)) {
                    if (account.get(PRESENCE).equals("\"0\"")) {
                        account.put(PRESENCE, "\"false\"");
                    } else if (account.get(PRESENCE).equals("\"1\"")) {
                        account.put(PRESENCE, "\"true\"");
                        account.put(PRESENCE_AGENT, "\"true\"");
                    } else if (account.get(PRESENCE).equals("\"2\"")) {
                        account.put(PRESENCE, "\"true\"");
                        account.put(PRESENCE_AGENT, "\"false\"");
                    }
                }

                bw.write("\t\t\t\t<account protocol=" + account.get(PROTOCOL) + ">\n");
                for (String key : account.keySet()) {
                    if (key.equals(ENABLED)) {
                        continue;
                    }
                    if (key.equals(PROTOCOL)) {
                        continue;
                    }
                    bw.write("\t\t\t\t\t<data name=\"" + key + VALUE + account.get(key) + TAG_CLOSE);
                }
                bw.write("\t\t\t\t</account>\n");
            }

            bw.write("\t\t\t</account_list>\n");
        } catch (IOException e) {
            LOG.error("Failed to write account list " + e.getMessage());
        }
    }

    private static Map<String, String> buildCoreTranslationMap() {
        HashMap<String, String> coreTranslation = new HashMap<String, String>();
        coreTranslation.put("ice_enabled", "useICE");
        coreTranslation.put("stun_server", STUN_SERVER);

        return coreTranslation;
    }

    private static Map<String, String> buildParameterTranslationMap() {
        Map<String, String> parameterTranslation = new HashMap<String, String>();
        parameterTranslation.put("display_name", DISPLAY);
        parameterTranslation.put("account_name", ACCOUNT_NAME);
        parameterTranslation.put("xmpp_resource", "xmppResource");
        parameterTranslation.put(DOMAIN, DOMAIN);
        parameterTranslation.put(USERNAME, USERNAME);
        parameterTranslation.put(PASSWORD, PASSWORD);
        parameterTranslation.put("authorization_username", "authName");
        parameterTranslation.put("voicemail_url", "vmNumber");
        parameterTranslation.put("outbound_proxy", "outboundProxy");
        parameterTranslation.put("presence_operating_mode", PRESENCE);
        parameterTranslation.put("default_session_refresh_interval_in_seconds", "regRefresh");
        parameterTranslation.put(ENABLED, ENABLED);
        parameterTranslation.put(PROTOCOL, PROTOCOL);

        return parameterTranslation;
    }

    private static List<String> buildVideoCodecsList() {
        List<String> validVideoCodecs = new ArrayList<String>();
        validVideoCodecs.add("H264");
        validVideoCodecs.add("VP8");

        return validVideoCodecs;
    }

    private static Map<String, String> buildAudioCodecsMap() {
        Map<String, String> validAudioCodecs = new HashMap<String, String>();
        validAudioCodecs.put(SILK_1600, SILK_1600);
        validAudioCodecs.put(SILK_8000, SILK_8000);
        validAudioCodecs.put("g711a", "G711a");
        validAudioCodecs.put("g711u", "G711u");
        validAudioCodecs.put("g722", "G722");
        validAudioCodecs.put("g729", "G729");
        validAudioCodecs.put("ilbc", "iLBC");
        validAudioCodecs.put(GSM, GSM);

        return validAudioCodecs;
    }
}
