/*
 *
 *
 * Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxprovision.auto;

import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.FilenameFilter;
import java.io.IOException;
import java.io.ByteArrayOutputStream;
import java.io.OutputStream;
import java.io.PrintWriter;
import java.lang.Double;
import java.lang.Long;
import java.lang.Math;
import java.net.URI;
import java.net.URISyntaxException;
import java.util.Random;
import java.util.regex.Pattern;

import javax.servlet.ServletException;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.apache.commons.io.IOUtils;
import org.apache.log4j.Logger;

import org.mortbay.http.HttpContext;
import org.mortbay.http.HttpServer;
import org.mortbay.jetty.servlet.ServletHandler;

import org.sipfoundry.sipxprovision.auto.Configuration;
import org.sipfoundry.sipxprovision.auto.Queue;


/**
 * A Jetty servlet that auto-provisions phones based on their HTTP requests.
 *
 * @author Paul Mossman
 */
@SuppressWarnings("serial")
public class Servlet extends HttpServlet {

    private static final Logger LOG = Logger.getLogger("Servlet");

    private static Queue m_queue = null;

    protected static Configuration m_config = null;

    protected static final short UNIQUE_ID_LENGTH = 3;

    private static final String MAC_RE_STR = "[0-9a-fA-F]{12}";

    /**
     * A subset of characters and digits that are unlikely to be confused with each
     * other when read verbalized.
     *
     * @see getUniqueId
     */
   private static final char UNIQUE_ID_CHARS[] = {
        'A', 'B',
        // C, D, E, & G sound like B
        'F', 'H', 'J',
        // I sounds like Y, and can be confused with 1
        // K sounds like J
        'L', 'M',
        // N sounds like M
        // O can be confused with 0
        // P sounds like B
        'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y',
        // Z, "zed" or "zee"?
        '1', '2', '3', '4', '5', '6', '7', '8', '9',
    };

    /**
     * Returns a simple unique-ish ID string (hash) generated from the specified seed.
     * The string is intentionally easy to read and verbalize.
     * <p>
     * Because java.util.Random is used, a given seed string will result in the same
     * ID across any (compliant) Java implementation on any platform.
     *
     * @param   seed_string the string to use as a seed for the unique ID
     * @return  the unique ID
     * @see     UNIQUE_ID_CHARS
     */
    protected static String getUniqueId(String seed_string) {

        if (null == seed_string) {
            seed_string = "";
        }
        seed_string = seed_string.toLowerCase();
        Long seed_num = new Long(1);
        for (int i = 0; i < seed_string.length(); i++) {
            seed_num += (seed_num * seed_string.charAt(i)) % (Long.MAX_VALUE - 1 / seed_string.length());
        }

        // A given seed string will generate the same ID across an Java implementation,
        // and on any platform.
        Random random = new Random(seed_num);

        StringBuffer hash = new StringBuffer();
        for (short x = 0; x < UNIQUE_ID_LENGTH; x++ ) {
            hash.append(UNIQUE_ID_CHARS[random.nextInt(UNIQUE_ID_CHARS.length-1)]);
        }

        return hash.toString();
    }

    private static final String POLYCOM_PATH_PREFIX = "/polycom-";

    private static final Pattern POLYCOM_PATH_RE =
    	Pattern.compile("^" + POLYCOM_PATH_PREFIX + MAC_RE_STR + ".cfg$");

    private static final String POLYCOM_OVERRIDES_PATH_PREFIX = "/polycom-overrides/";

    private static final Pattern POLYCOM_OVERRIDES_PATH_RE =
    	Pattern.compile("^" + POLYCOM_OVERRIDES_PATH_PREFIX + MAC_RE_STR + "-phone.cfg$");

    private static final Pattern EXACT_MAC_RE = Pattern.compile("^" + MAC_RE_STR + "$");

    private static final String NORTEL_IP_12X0_PATH_PREFIX = "/Nortel/config/SIP";

    private static final Pattern NORTEL_IP_12X0_PATH_RE =
        Pattern.compile("^" + NORTEL_IP_12X0_PATH_PREFIX + MAC_RE_STR + ".xml$");

    private boolean isMacAddress(String mac) {
        return EXACT_MAC_RE.matcher(mac).matches();
    }

    private static final String POLYCOM_UA_DELIMITER = "UA/";

    private static String extractMac(String path, String prefix) {
        return path.substring(prefix.length(), prefix.length()+12);
    }

    private void doNortelIp12X0Get(HttpServletRequest request, HttpServletResponse response)
        throws IOException {

        // Get the MAC address.
        Queue.DetectedPhone phone = new Queue.DetectedPhone();
        phone.mac = extractMac(request.getPathInfo(), NORTEL_IP_12X0_PATH_PREFIX );
        if (null == phone.mac) {
            doDefaultGet(request,response);
            return;
        }

        // Parse the User-Agent.
        String useragent = request.getHeader("User-Agent");
        if (useragent.contains("Gecko") && m_config.isDebugOn()) {
            // For testing purposes.
            useragent = "Nortel IP Phone 1230 (SIP12x0.01.01.04.00)";
        }
        LOG.info("Nortel IP 12x0 MAC: " + phone.mac + " User-Agent: " + useragent );
        try {

            // Model
            String token1 = "IP Phone ";
            int i1 = useragent.indexOf(token1);
            int i2 = useragent.indexOf(" ", i1 + token1.length());
            if (-1 == i1 || -1 == i2) {
                doDefaultGet(request, response);
                return;
            }
            String model_id = useragent.substring(i1 + token1.length(), i2);

            // This will throw an exception if the Model lookup fails.
            phone.model = Queue.lookupPhoneModel(model_id);
            phone.id = getUniqueId(phone.mac);
            LOG.info("   " + phone.id + " - " + phone.model.full_label);

            // Version
            i1 = useragent.indexOf("(");
            i2 = useragent.indexOf(")", i1 + 1);
            if (-1 != i1 && -1 != i2) {
                phone.version = useragent.substring(i1 + 1, i2);
            }
        }
        catch (Exception e) {
            LOG.debug("Failed to parse User-Agent header: ", e);
            doDefaultGet(request, response);
            return;
        }

        // Supply enough configuration for the phone to register.
        response.setContentType("text/plain");
        PrintWriter out = response.getWriter();
        out.println("Good!");
        // TODO: the config....

        // Queue the phone for auto-provisioning.
        m_queue.addDetectedPhone(phone);
    }

    private void doPolycomGet(HttpServletRequest request, HttpServletResponse response)
        throws IOException {

        Queue.DetectedPhone phone = new Queue.DetectedPhone();

        // The MAC must be the next part of the path.
        phone.mac = extractMac(request.getPathInfo(), POLYCOM_PATH_PREFIX);
        // TODO: this check won't be needed when regexp is used to trigger this method..... see doNortelIp12X0Get
        if (!isMacAddress(phone.mac)) {
            doDefaultGet(request,response);
            return;
        }

        // Parse the User-Agent.
        String useragent = request.getHeader("User-Agent");
        if (useragent.contains("Gecko") && m_config.isDebugOn()) {
            // For testing purposes.
            useragent = "FileTransport PolycomSoundStationIP-SSIP_6000-UA/3.2.0.0157";
        }
        LOG.info("Polycom MAC: " + phone.mac + " User-Agent: " + useragent );
        try {

            // Model
            int i1 = useragent.indexOf("-");
            int i2 = useragent.indexOf("-", i1+1);
            if (-1 == i1 || -1 == i2) {
                doDefaultGet(request, response);
                return;
            }
            String model_id = useragent.substring(i1+1, i2);

            // This will throw an exception if the Model lookup fails.
            phone.model = Queue.lookupPhoneModel(model_id);
            phone.id = getUniqueId(phone.mac);
            LOG.info("   " + phone.id + " - " + phone.model.full_label);

            // Version
            i1 = useragent.indexOf(POLYCOM_UA_DELIMITER);
            if (-1 != i1) {
                i2 = useragent.indexOf(" ", i1 + POLYCOM_UA_DELIMITER.length());
                if (-1 == i2) {
                    i2 = useragent.length();
                }
                phone.version = useragent.substring(i1 + POLYCOM_UA_DELIMITER.length(), i2);
            }
        }
        catch (Exception e) {
            LOG.debug("Failed to parse User-Agent header: ", e);
            doDefaultGet(request, response);
            return;
        }

        // Supply enough configuration for the phone to register.
        response.setContentType("text/plain");
        PrintWriter out = response.getWriter();

        out.println("<?xml version=\"1.0\" standalone=\"yes\"?>");
        out.println("<sip>");
        out.println("</sip>");
        out.println("<phone1>");
        out.println("   <reg");
        out.println("      reg.1.displayName=\"" + phone.id + " " + phone.mac + "\"");
        out.println("      reg.1.address=\"" + m_config.getProvisionSipUsername() + "@" + m_config.getSipDomainName() + "\"");
        out.println("      reg.1.label=\"" + phone.id + "\"");
        out.println("      reg.1.auth.userId=\"" + m_config.getProvisionSipUsername() + "\"");
        out.println("      reg.1.auth.password=\"" + m_config.getProvisionSipPassword() + "\"");
        out.println("      reg.1.ringType=\"9\""); // Highest Double Trill
        out.println("      reg.1.server.1.address=\"" + m_config.getSipDomainName() + "\"");
        out.println("      reg.1.server.1.transport=\"UDPonly\"");
        out.println("   />");
        out.println("</phone1>");
        out.flush();

// TODO: Further customize, requires more data from sipXconfig:
//  - NTP & timezone
//  - Hotline AA dial: needs AA "number".
//  - MOH URI (<sip> section)
//  - Outbound Proxy (<sip> section)

        // Queue the phone for auto-provisioning.
        m_queue.addDetectedPhone(phone);
    }

    /**
     * Starts the Jetty servlet that handles auto-provision requests
     */
    public static void start() {
        try {
            // The configuration.
            m_config = new Configuration();
            m_config.configureLog4j();

            LOG.info("START.");
            LOG.debug(String.format("Unique ID - length: %d  set size: %d  combinations: %d.",
                    UNIQUE_ID_LENGTH, UNIQUE_ID_CHARS.length,
                    new Double(Math.pow(UNIQUE_ID_CHARS.length, UNIQUE_ID_LENGTH)).intValue()));

            // Dump the configuration into the log.
            ByteArrayOutputStream stream = new ByteArrayOutputStream();
            m_config.dumpConfiguration(new PrintWriter(stream));
            String[] lines = stream.toString().split("\\n");
            for (String line : lines){
                LOG.info(line);
            }

            // This is what causes non-provisioned Polycoms to the HTTP request to the servlet.
            initializeStaticConfig(m_config);

            // Start the queue thread.
            m_queue = new Queue(m_config);
            m_queue.start();

            // Start up jetty.
            HttpServer server = new HttpServer();

            // Bind the port on all interfaces.
            server.addListener(":" + m_config.getServletPort());

            HttpContext httpContext = new HttpContext();
            httpContext.setContextPath("/");

            // Setup the servlet to call the class when the URL is fetched.
            ServletHandler servletHandler = new ServletHandler();
            servletHandler.addServlet(Servlet.class.getCanonicalName(), m_config.getServletUriPath()
            		+ "/*", Servlet.class.getName());
            httpContext.addHandler(servletHandler);
            server.addContext(httpContext);

            // Start it up.
            LOG.info(String.format("Starting %s servlet on *:%d%s", Servlet.class.getCanonicalName(),
            		m_config.getServletPort(), m_config.getServletUriPath()));
            server.start();
        } catch (Exception e) {
            LOG.error("Failed to start the servlet:", e);
        }
    }

    /**
     * This content is static only while the servlet is running.  A configuration change of the
     * servlet port will change the content, but this will be done during the servlet re-start.
     */
    private static void initializeStaticConfig(Configuration config) {

        // Polycom SoundPoint IP family.
        File polycom_default_file = new File(config.getTftpPath(), "/000000000000.cfg");
        try {
            java.io.FileOutputStream fos = new java.io.FileOutputStream(polycom_default_file);
            java.io.PrintStream ps = new java.io.PrintStream(fos);

            /* TODO - try using the Velocity template instead.....  (Velocity template would need update....)

    http://sipxecs.sipfoundry.org/rep/sipXecs/main/sipXconfig/neoconf/test/org/sipfoundry/sipxconfig/device/
       VelocityProfileGeneratorTest.java
       velocity_test.vm

    main/sipXconfig/neoconf/src/org/sipfoundry/sipxconfig/device/ProfileContext.java:        context.put("cfg", this);

This may or may not work for all files, since we'll end up with a lot of "empty" attributes, for example:

   The one which would be done by doPolycomGet.....
   <codecs>
      <preferences
      />
   </codecs>

   Hopefully this does not cause a Polycom problems...

*/
            String servlet_root_url = String.format("http://%s:%d", config.getHostname(), config.getServletPort());
            String config_path_url = String.format("%s%s%s[PHONE_MAC_ADDRESS].cfg", servlet_root_url,
                    config.getServletUriPath(), POLYCOM_PATH_PREFIX);
            ps.println("<?xml version=\"1.0\" encoding=\"utf-8\" standalone=\"yes\"?");
            ps.println("<APPLICATION");
            ps.println("   APP_FILE_PATH=\"sip.ld\"");
            ps.println("   CONFIG_FILES=\"" + config_path_url + ", polycom_phone1.cfg, polycom_sip.cfg\"");
            ps.println("   MISC_FILES=\"\"");
            ps.println("   LOG_FILE_DIRECTORY=\"\"");
            ps.println("   OVERRIDES_DIRECTORY=\"" + servlet_root_url + "\""); // See doPolycomOverridesGet()
            ps.println("   CONTACTS_DIRECTORY=\"\"");
            ps.println("   LICENSE_DIRECTORY=\"\">");
            ps.println("<APPLICATION_SPIP601");
            ps.println("   APP_FILE_PATH_SPIP601=\"sip.ld\"");
            ps.println("   CONFIG_FILES_SPIP601=\"" + config_path_url +
                    ", polycom_phone1_3.1.X.cfg, polycom_sip_3.1.X.cfg\" />");
            ps.println("</APPLICATION>");
            fos.close();
            LOG.info(String.format("Wrote Polycom default file %s.", polycom_default_file.getName()));
        }
        catch(IOException e ) {
            LOG.error("Failed to write " + polycom_default_file + ":", e);
        }

        // Copy the Polycom static files.  (See: http://list.sipfoundry.org/archive/sipx-dev/msg20157.html)
        File polycom_src_dir = new File(System.getProperty("conf.dir") + "/polycom");
        try {
	        
	        FilenameFilter cfg_filter = new FilenameFilter() {
	        	public boolean accept(File dir, String name) {
	                return name.startsWith("polycom_") && name.endsWith(".cfg");
	            }
	        };

	        File[] files = polycom_src_dir.listFiles(cfg_filter);
	        if (null == files) {
	           LOG.error(String.format("No Polycom static files found at %s.", polycom_src_dir.getAbsolutePath()));
	        }
	        else {
	        	int copy_count = 0;
		        for (File file : files) {
		        	
		        	File dest_file = new File(config.getTftpPath(), file.getName());
		        	if (!dest_file.exists()) {
		        		
			        	OutputStream output = new BufferedOutputStream(new FileOutputStream(dest_file));
			        	FileInputStream input = null;
			        	try {
			        		input = new FileInputStream(file);
			        	    IOUtils.copy(input, output);
			        	    copy_count++;
			        	} 
			        	catch (IOException e) {
			        		LOG.error(String.format("Failed to copy Polycom static file %s.", file.getName()), e);
			        	}
			        	finally {
			        	   IOUtils.closeQuietly(input);
			        	   IOUtils.closeQuietly(output);
			        	}
		        	}
		        }
	        	
	        	LOG.info(String.format("Copied %d (of %d) Polycom static files from %s.", copy_count, files.length,
	        			polycom_src_dir.getAbsolutePath()));
	        }
        }
        catch(Exception e ) {
            LOG.error("Failed to copy Polycom static files from " + polycom_src_dir.getAbsolutePath() + ":", e);
        }
        
    }

    private void logAndOut(String msg, PrintWriter out) {
        LOG.debug(msg);
        out.println(msg);
    }

    // TODO: Test case (x2 phone types) when MAC is too short (should not invoke the do___Get...)
    public void doGet(HttpServletRequest request, HttpServletResponse response) throws ServletException, IOException {

        // What kind of phone might this be?
        String path = request.getPathInfo();
        LOG.info("GET - " + path);
        // TODO: regexp
        if (path.startsWith(POLYCOM_PATH_PREFIX)) {

            // Polycom SoundPoint IP family
            doPolycomGet(request, response);
        }
        // TODO: regexp
        else if (path.endsWith("-phone.cfg")) {

            // A Polycom looking for it's OVERRIDES file.  (This does not result in auto-provisioning.)
            doPolycomOverridesGet(response);
        }
        else if (NORTEL_IP_12X0_PATH_RE.matcher(path).matches()) {

            // Nortel IP 12x0
            doNortelIp12X0Get(request, response);
        }
        else if (path.startsWith("/debug") && m_config.isDebugOn()) {
            // Do some debugging.
            LOG.debug("Debugging...." + path);
            response.setContentType("text/plain");
            PrintWriter out = response.getWriter();

            logAndOut("getPathInfo(): '" + request.getPathInfo() + "'", out);
            logAndOut("getQueryString(): '" + request.getQueryString() + "'", out);
            logAndOut("getRequestURI(): '" + request.getRequestURI() + "'", out);
            logAndOut("getRemoteUser(): '" + request.getRemoteUser() + "'", out);
            logAndOut("getServletPath(): '" + request.getServletPath() + "'", out);
            logAndOut("getContextPath(): '" + request.getContextPath() + "'", out);
            logAndOut("getLocalAddr(): '" + request.getLocalAddr() + "'", out);
            logAndOut("getServerName(): '" + request.getServerName() + "'", out);
            logAndOut("getServerPort(): '" + request.getServerPort() + "'", out);
            logAndOut("", out);

            logAndOut("request: " + request, out);
            m_config.dumpConfiguration(out);
        }
        else {
            // Unknown.
            doDefaultGet(request, response);
        }
    }

    private void doPolycomOverridesGet(HttpServletResponse response) throws IOException {
        // Supply configuration that will clear the local user preferences.  (Language, contrast, etc.)
        response.setContentType("text/plain");
        PrintWriter out = response.getWriter();
        out.println("<?xml version=\"1.0\" standalone=\"yes\"?>");
        out.println("<PHONE_CONFIG><OVERRIDES/></PHONE_CONFIG>");
    }

    // TODO - test cases for various problems that could occur with
    private static void doDefaultGet(HttpServletRequest request, HttpServletResponse response) throws IOException {

        LOG.debug("Un-recognized path pattern.  Redirecting to sipXconfig.");

        // Try to redirect with the servername used in the request.  Helpful if the client can't
        // resolve the FQDN that we're using.
        String redirect_location = m_config.getConfigurationUri();
        try {
            URI config_uri = new URI(redirect_location);
            URI redirect_uri = new URI(config_uri.getScheme(), null, request.getServerName(),
                    config_uri.getPort(), config_uri.getPath(), null, null);

            redirect_location = redirect_uri.toString();
        }
        catch (URISyntaxException e) {
            LOG.debug("Failed to customize re-direct URI based on request parameters: ", e);
        }

        // The JavaScript to redirect.
        response.setContentType("text/html");
        PrintWriter out = response.getWriter();
        out.println("<html><head><script language=\"JavaScript\">");
        out.println("<!-- Hide script from old browsers");
        out.println("location = '" + redirect_location + "'");
        out.println("// End script hiding from old browsers -->");
        out.println("</script></head><body></body></html>");
    }

    public static void main(String[] args) {

        // Send log4j output to the console.
        org.apache.log4j.BasicConfigurator.configure();

        System.out.println("MAIN - " + Servlet.class.getCanonicalName().toString());
    }
}