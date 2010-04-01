/*
 * 
 * 
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 */
package org.sipfoundry.sipximbot;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import javax.servlet.ServletException;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;

import org.apache.log4j.Logger;
import org.w3c.dom.Document;

/**
 * A RESTful interface for sending IMs
 * 
 * ones service at the moment:
 *    Send IM
 *    
 * Prefix is /IM/*
 * Path is
 * /{username}/
 *      /sendIM   PUT (data is the IM text) sends an IM to the user indicated to {username}
 */
public class ImbotServlet extends HttpServlet {
    private static final long serialVersionUID = 1L;
    private static final String METHOD_POST = "POST";
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipximbot");

    public void doPut(HttpServletRequest request, HttpServletResponse response) throws ServletException, IOException {
        doIt(request, response);
    }
    
    public void doPost(HttpServletRequest request, HttpServletResponse response) throws ServletException, IOException {
        doIt(request, response);
    }

    public void doIt(HttpServletRequest request, HttpServletResponse response) throws ServletException, IOException {
        String method = request.getMethod().toUpperCase();
        
        // For (minimal) security, check the request is arriving from THIS machine
        // TODO (change this once using https and authentication of remote party)
        if (!request.getLocalAddr().equals(request.getRemoteAddr())) {
            response.sendError(403); // Send 403 Forbidden
            return;
        }

        String pathInfo = request.getPathInfo();
        String[] subDirs = pathInfo.split("/");
        if (subDirs.length < 3) {
            response.sendError(404);    // path not found
            return;
        }
        
        // The first element is empty (the leading slash)
        // The second element is the username
        String userString = subDirs[1];
        // The third element is the "context" (ie. "sendIM")
        String context = subDirs[2];
        context = context.toLowerCase();
        
        // Load the list of valid users 
        // (it is static, so don't worry about sucking it in each time, it'll only 
        // be re-read if it has changed on disk)
        FullUsers fullUsers = FullUsers.update();
        FullUser user = fullUsers.isValidUser(userString);
        if (user != null) {
            PrintWriter pw = response.getWriter();
            LOG.info(String.format("ImbotServlet::doIt %s %s", method, pathInfo));

            if (context.startsWith("send")) {
                if (method.equals(METHOD_POST)) {
                    InputStreamReader input = new InputStreamReader(request.getInputStream());
                    BufferedReader buffer = new BufferedReader(input);                  
                    
                    String instantMsg = "";
                    
                    String line = buffer.readLine();
                    while(line != null) {
                        instantMsg += line;
                        line = buffer.readLine();
                    }
                    
                    if(context.equals("sendvmentryim")) {
                        if(user.getVMEntryIM()) {
                            IMBot.sendIM(user, instantMsg);
                        }
                    } else if(context.equals("sendvmexitim")) {
                        if(user.getVMExitIM()) {
                            IMBot.sendIM(user, instantMsg);
                        }
                    } else {
                        IMBot.sendIM(user, instantMsg);
                    }
                }

            } else if(context.compareToIgnoreCase("addToRoster") == 0) {
                IMBot.AddToRoster(user);
            } else {    
                response.sendError(400, "context not understood");
            }

            pw.close();
        } else {
            response.sendError(404, "User not found");
            LOG.info(String.format("ImbotServlet::doIt %s not found", userString));
        }
    }

}
