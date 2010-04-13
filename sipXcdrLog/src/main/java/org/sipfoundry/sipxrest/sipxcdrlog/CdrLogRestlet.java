/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxrest.cdrlog;

import java.sql.Driver;
import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.ResultSet;
import java.sql.ResultSetMetaData;
import java.sql.SQLException;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.Timestamp;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.GregorianCalendar;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.transform.*;
import javax.xml.transform.stream.*;
import javax.xml.transform.dom.*;

import org.apache.log4j.Logger;
import org.restlet.Context;
import org.restlet.Restlet;
import org.restlet.Route;
import org.restlet.Router;
import org.restlet.data.MediaType;
import org.restlet.data.Method;
import org.restlet.data.Request;
import org.restlet.data.Response;
import org.restlet.data.Status;
import org.restlet.resource.DomRepresentation;

import org.sipfoundry.sipxrest.*;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
public class CdrLogRestlet extends Restlet {

    private static Logger logger = Logger.getLogger(CdrLogRestlet.class);
    private static String serviceName = "cdr";
    private Connection cdrConnection;
    private String cdrDBUrl = "jdbc:postgresql:SIPXCDR";
    private String sqlStmt = "SELECT caller_aor, callee_aor, callee_contact, start_time, (end_time - connect_time) AS duration, termination, callee_route from cdrs";
    private String sqlUserWhereStmt = "WHERE (caller_aor LIKE ? OR callee_aor LIKE ? ) ";
    private String sqlFromDateStmt = "AND start_time > ? ";
    private String sqlLimitStmt = "LIMIT ?";
    private String sqlOrderStmt = "ORDER BY start_time DESC ";


    public CdrLogRestlet() {
    }

    @Override
    public void handle(Request request, Response response) {
        try {
            Method httpMethod = request.getMethod();
            if (!httpMethod.equals(Method.GET)) {
                response.setStatus(Status.CLIENT_ERROR_METHOD_NOT_ALLOWED);
                return;
            }

            /*
             * Only GET is supported for CDR records.
             */
            String userId = (String) request.getAttributes().get(CdrLogParams.USER);

            String Ilimit = (String) request.getAttributes().get(CdrLogParams.LIMIT);
            int limit;
            if (Ilimit != null) {
               limit = Integer.parseInt(Ilimit);
            }
            else {
               limit = 1000;
            }

            String fromDate = (String) request.getAttributes().get(
                    CdrLogParams.FROMDATE);

            if ( userId == null ) {
                response.setStatus(Status.CLIENT_ERROR_BAD_REQUEST);
                return;
            }


            Timestamp fromTimeMs = null;
            if (fromDate != null) {
                SimpleDateFormat dateFormat = new SimpleDateFormat("yyyyMMdd");
                try {
                    Date cdrDate = dateFormat.parse(fromDate);
                    if (cdrDate == null) {
                        response.setStatus(Status.CLIENT_ERROR_BAD_REQUEST);
                        return;
                    }
                    else {
                        fromTimeMs = new Timestamp(cdrDate.getTime());
                    }
                }
                catch (ParseException e) {
                   logger.error("Invalid fromDate specified");
                   response.setStatus(Status.CLIENT_ERROR_BAD_REQUEST);
                   return;
                }
            }

            System.setProperty("jdbc.drivers", "org.postgresql.Driver");
            // Establish a connection to the CDR database.
            cdrConnection = DriverManager.getConnection(cdrDBUrl, "postgres", "");

            String sqlPrepareString;
            String userLike = '%' + userId + "@%";
            PreparedStatement qStatement;

            if (fromTimeMs != null) {
                sqlPrepareString = sqlStmt + " " + sqlUserWhereStmt + sqlFromDateStmt + sqlOrderStmt + sqlLimitStmt;
                qStatement = cdrConnection.prepareStatement(sqlPrepareString);
                qStatement.setString(1, userLike);
                qStatement.setString(2, userLike);
                qStatement.setTimestamp(3, fromTimeMs);
                qStatement.setInt(4, limit);
            }
            else {
                sqlPrepareString = sqlStmt + " " + sqlUserWhereStmt + sqlOrderStmt + sqlLimitStmt;
                qStatement = cdrConnection.prepareStatement(sqlPrepareString);
                qStatement.setString(1, userLike);
                qStatement.setString(2, userLike);
                qStatement.setInt(3, limit);
            }

            // Execute the SQL query to obtain the results.
            ResultSet qResults = qStatement.executeQuery();

            // Build an XML document to return the DB Query results.
            DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
            DocumentBuilder builder =factory.newDocumentBuilder();
            Document doc = builder.newDocument();
            doc.setXmlVersion("1.0");
            Element results = doc.createElement("Results");
            doc.appendChild(results);

            ResultSetMetaData qResultsMeta = qResults.getMetaData();
            int columnCount = qResultsMeta.getColumnCount();
            while ( qResults.next()) {
                Element row = doc.createElement("Row");
                results.appendChild(row);
                for (int col = 1; col <= columnCount; col++) {
                    String columnName = qResultsMeta.getColumnName(col);
                    Object columnValue = qResults.getObject(col);
                    Element node = doc.createElement(columnName);
                    if (columnValue != null) {
                        node.appendChild(doc.createTextNode(columnValue.toString()));
                    }
                    else {
                        node.appendChild(doc.createTextNode("null"));
                    }
                    row.appendChild(node);
                }
            }
            DomRepresentation rep = new DomRepresentation(MediaType.TEXT_XML,doc);
            response.setEntity(rep);
            response.setStatus(Status.SUCCESS_OK);

            // Close the connection to the CDR database.
            cdrConnection.close();
        } catch (Exception ex) {
            logger.error("An exception occured while processing the request. : ", ex);
            response.setStatus(Status.SERVER_ERROR_INTERNAL);
            return;

        }

    }

}



