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
    private static final String CONF_DIR_PROPERTY = "conf.dir";
    private static final String CONF_RESOURCE = "/counterpath/cmcprov.properties";
    private static final String CONTACTS_LIST_FILE_SUBFIX = "-directory.xml";
    private static final String WEBDAV_DIR = "/webdav/";
    private static final String TFTP_RELATIVE_PATH = "/profile/tftproot/";
    private static final String DOT = ".";

    @Override
    protected void doPost(HttpServletRequest req, HttpServletResponse resp) throws javax.servlet.ServletException,
            java.io.IOException {
        PrintWriter out = resp.getWriter();
        String reqType = "";
        try {
            Map<String, String> parameters;
            try {
                parameters = getParameterMapFromBody(req);
            } catch (java.io.IOException error) {
                throw new FailureDataException("Cannot extract parameters");
            }
            reqType = parameters.get("output_type");
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

            if (reqType == "xml") {
                String uploadDirectory = getProvisioningContext().getUploadDirectory();
                translateINItoXML(new File(uploadDirectory, profileFilenames[0].getName()));
                attachFile(
                        new File(uploadDirectory, profileFilenames[0].getName()
                                .replaceAll(".ini", "-briamobile.xml")), out);
            } else {
                uploadPhoneProfile(profileFilenames[0].getName(), out);
            }

        } catch (FailureDataException e) {
            LOG.error("Login error: " + e.getMessage());
            if (reqType == "xml") {
                out.write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<cpc_mobile version=\"1.0\">\n<login_response>\n<status success=\"false\" error_text=\"No match for username and password.\"/>\n</login_response>\n</cpc_mobile>");
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

    protected Map<String, String> getParameterMapFromBody(HttpServletRequest req) throws java.io.IOException {
        Hashtable<String, String> parameters = new Hashtable<String, String>();
        BufferedReader br = req.getReader();
        String line = br.readLine();
        if (line == null) {
            return parameters;
        }

        if (!br.ready()) {
            String[] pairs = line.split("\\&");
            for (int i = 0; i < pairs.length; i++) {
                String[] fields = pairs[i].split("=");
                if (fields.length == 2) {
                    String name = URLDecoder.decode(fields[0], PARAMETER_ENCODING);
                    String value = URLDecoder.decode(fields[1], PARAMETER_ENCODING);
                    parameters.put(name.toLowerCase(), value);
                }
            }
            parameters.put("output_type", "txt");
        } else {
            boolean inLogin = false;
            while (br.ready()) {
                line = br.readLine();
                if (line.trim().length() > 6 && line.trim().substring(0, 6).equals("user=\"")) {
                    parameters.put("username", line.trim().substring(6, line.trim().length() - 1));
                } else if (line.trim().length() > 10 && line.trim().substring(0, 10).equals("password=\"")) {
                    parameters.put("password", line.trim().substring(10, line.trim().length() - 1));
                }
            }
            parameters.put("output_type", "xml");

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

    private void translateINItoXML(File inputfile) {
        String inputfilename = inputfile.getPath();
        String outputfilename = inputfilename.replaceAll(".ini", "-briamobile.xml");

        ArrayList<String> validAudioCodecs = new ArrayList<String>() {
            {
                add("SILK/1600");
                add("SILK/8000");
                add("g711a");
                add("g711u");
                add("g722");
                add("g729");
                add("ilbc");
                add("GSM");
            }
        };

        ArrayList<String> validVideoCodecs = new ArrayList<String>() {
            {
                add("H264");
                add("VP8");
            }
        };

        HashMap<String, String> parameterTranslation = new HashMap<String, String>();
        parameterTranslation.put("display_name", "display");
        parameterTranslation.put("account_name", "accountName");
        parameterTranslation.put("xmpp_resource", "xmppResource");
        parameterTranslation.put("domain", "domain");
        parameterTranslation.put("username", "username");
        parameterTranslation.put("password", "password");
        parameterTranslation.put("authorization_username", "authName");
        parameterTranslation.put("voicemail_url", "vmNumber");
        parameterTranslation.put("outbound_proxy", "outboundProxy");
        parameterTranslation.put("presence_operating_mode", "presence");
        parameterTranslation.put("default_session_refresh_interval_in_seconds", "regRefresh");
        parameterTranslation.put("enabled", "enabled");
        parameterTranslation.put("protocol", "protocol");

        HashMap<String, String> coreTranslation = new HashMap<String, String>();
        coreTranslation.put("ice_enabled", "useICE");
        coreTranslation.put("stun_server", "stunServer");

        int tablevel = 0;
        LOG.debug("Input File is: " + inputfilename);
        LOG.debug("Output File is: " + outputfilename);

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
                bw.write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<cpc_mobile version=\"1.0\">\n<login_response>\n\t<status success=\"true\"/>\n</login_response>\n<branding>\n\t<settings_data>\n\t\t<core_data_list>\n");
            } catch (IOException e) {
                LOG.error("XMLTranlation: Failed to write xml header." + e.getMessage());
            }

            ArrayList<String> audiocodecs = new ArrayList<String>();
            HashMap<Integer, String> audiocodecs_priority = new HashMap<Integer, String>();
            ArrayList<String> videocodecs = new ArrayList<String>();
            HashMap<Integer, String> videocodecs_priority = new HashMap<Integer, String>();
            ArrayList<HashMap<String, String>> proxyList = new ArrayList<HashMap<String, String>>();
            String proxyString = "";
            HashMap<String, String> currentProxy = null;
            HashMap<String, String> coreSettings = new HashMap<String, String>();

            // Ingest INI File
            try {
                LOG.debug("INFO: Ingesting ini file.");
                Scanner s = new Scanner(inputfile);
                while (s.hasNextLine()) {
                    String line = s.nextLine();
                    if (line.contains(":")) {
                        String[] splitline = line.split(":|=", 4);
                        if (splitline[0].equals("codecs") && splitline[2].equals("priority")
                                && validAudioCodecs.contains(splitline[1])) {
                            int priority = Integer.parseInt(splitline[3].substring(1, splitline[3].length() - 1)
                                    .split("\\.", 2)[0]);
                            audiocodecs_priority.put(priority, splitline[1]);
                            audiocodecs.add(splitline[1]);
                        }
                        if (splitline[0].equals("codecs") && splitline[2].equals("enabled")
                                && validAudioCodecs.contains(splitline[1])) {
                            audiocodecs.add(splitline[1]);
                        }
                        if (splitline[0].equals("codecs") && splitline[2].equals("priority")
                                && validVideoCodecs.contains(splitline[1])) {
                            int priority = Integer.parseInt(splitline[3].substring(1, splitline[3].length() - 1)
                                    .split("\\.", 2)[0]);
                            videocodecs_priority.put(priority, splitline[1]);
                            videocodecs.add(splitline[1]);
                        }
                        if (splitline[0].equals("codecs") && splitline[2].equals("enabled")
                                && validVideoCodecs.contains(splitline[1])) {
                            videocodecs.add(splitline[1]);
                        }
                        if (splitline[0].equals("proxies")) {
                            // Handle parameters used with setting up accounts.
                            if (!proxyString.equals(splitline[1])) {
                                if (!proxyString.equals(""))
                                    proxyList.add(currentProxy);
                                currentProxy = new HashMap<String, String>();
                                proxyString = splitline[1];
                            }
                            if (coreTranslation.containsKey(splitline[2]))
                                coreSettings.put(coreTranslation.get(splitline[2]), splitline[3]);
                            else if (parameterTranslation.containsKey(splitline[2]))
                                currentProxy.put(parameterTranslation.get(splitline[2]), splitline[3]);
                            // else
                            // currentProxy.put(splitline[2],splitline[3]);
                        }
                    }
                }
            } catch (IOException e) {
                LOG.error("XMLTranslation: Failed to ingest ini file.\n" + e.getMessage());
            }

            // Write out core settings list
            try {
                LOG.debug("Writing core settings list. ");

                // Add custom modifications for core settings
                String tval = "\"false\"";
                if (!coreSettings.get("stunServer").equals(""))
                    tval = "\"true\"";
                coreSettings.put("useStun", tval);
                coreSettings.put("useStun3G", tval);

                for (String key : coreSettings.keySet()) {
                    bw.write("\t\t\t<data name=\"" + key + "\" value=" + coreSettings.get(key) + "/>\n");
                }
            } catch (IOException e) {
                LOG.error("Failed to write core settings list" + e.getMessage());
            }
            // Write out codecs list
            try {
                LOG.debug("Writing codec list... ");
                bw.write("\t\t\t<codec_list>\n\t\t\t\t<codec_list_audio_wifi>\n");
                for (int i = audiocodecs_priority.size(); i >= 0; i--)
                    if (audiocodecs.contains(audiocodecs_priority.get(i)))
                        bw.write("\t\t\t\t\t<codec name=\"" + audiocodecs_priority.get(i)
                                + "\" enabled=\"true\"/>\n");
                bw.write("\t\t\t\t</codec_list_audio_wifi>\n");
                bw.write("\t\t\t\t<codec_list_audio_cell>\n");
                for (int i = audiocodecs_priority.size(); i >= 0; i--)
                    if (audiocodecs.contains(audiocodecs_priority.get(i)))
                        bw.write("\t\t\t\t\t<codec name=\"" + audiocodecs_priority.get(i)
                                + "\" enabled=\"true\"/>\n");
                bw.write("\t\t\t\t</codec_list_audio_cell>\n");
                bw.write("\t\t\t\t<codec_list_video_wifi>\n");
                for (int i = videocodecs_priority.size(); i >= 0; i--)
                    if (audiocodecs.contains(audiocodecs_priority.get(i)))
                        bw.write("\t\t\t\t\t<codec name=\"" + videocodecs_priority.get(i)
                                + "\" enabled=\"true\"/>\n");
                bw.write("\t\t\t\t</codec_list_video_wifi>\n");
                bw.write("\t\t\t</codec_list>\n");
            } catch (IOException e) {
                LOG.error("Failed to write codec list" + e.getMessage());
            }

            // Write out account list
            try {
                LOG.debug("Writing account list.");
                bw.write("\t\t\t<account_list>\n");

                // Iterate through each account
                for (HashMap<String, String> account : proxyList) {
                    if (account.get("enabled").equals("\"false\""))
                        break;
                    if (account.get("protocol") == null)
                        account.put("protocol", "\"sip\"");
                    if (account.get("display") == null)
                        account.put("display", account.get("protocol"));
                    if (account.get("accountName") == null)
                        account.put("accountName", "\"" + account.get("display").replace("\"", "") + " - "
                                + account.get("protocol").replace("\"", "") + "\"");

                    if (account.containsKey("presence")) {
                        if (account.get("presence").equals("\"0\"")) {
                            account.put("presence", "\"false\"");
                        } else if (account.get("presence").equals("\"1\"")) {
                            account.put("presence", "\"true\"");
                            account.put("presenceAgent", "\"true\"");
                        } else if (account.get("presence").equals("\"2\"")) {
                            account.put("presence", "\"true\"");
                            account.put("presenceAgent", "\"false\"");
                        }
                    }

                    bw.write("\t\t\t\t<account protocol=" + account.get("protocol") + ">\n");
                    for (String key : account.keySet()) {
                        if (key.equals("enabled"))
                            continue;
                        if (key.equals("protocol"))
                            continue;
                        bw.write("\t\t\t\t\t<data name=\"" + key + "\" value=" + account.get(key) + "/>\n");
                    }
                    bw.write("\t\t\t\t</account>\n");
                }

                bw.write("\t\t\t</account_list>\n");
            } catch (IOException e) {
                LOG.error("Failed to write account list " + e.getMessage());
            }

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

}
