/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.bulk.vcard;

import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.Reader;
import java.util.ArrayList;
import java.util.List;

import junit.framework.TestCase;

import org.apache.commons.collections.Closure;

public class VcardParserImplTest extends TestCase {

    public void testParse() throws Exception {

        InputStream testPhonebookFile = getClass().getResourceAsStream("testPhonebook.vcf");
        Reader reader = new InputStreamReader(testPhonebookFile);
        VcardParserImpl parser = new VcardParserImpl();
        final List<String[]> entriesList = new ArrayList<String[]>();
        Closure add = new Closure() {
            public void execute(Object item) {
                entriesList.add((String[]) item);
            }
        };

        parser.parse(reader, add);
        assertEquals(9, entriesList.size());

        String[] entry1 = entriesList.get(0);
        assertEquals(22, entry1.length);
        assertEquals("34 Jean 2Luc4", entry1[0]);
        assertEquals("Picard12", entry1[1]);
        assertEquals("1234", entry1[2]);

        String[] entry2 = entriesList.get(1);
        assertEquals(22, entry2.length);
        assertEquals("Luke", entry2[0]);
        assertEquals("Skywalker", entry2[1]);
        assertEquals("1235", entry2[2]);

        String[] entry3 = entriesList.get(2);
        assertEquals(22, entry3.length);
        assertEquals("Venus", entry3[0]);
        assertEquals("Williams", entry3[1]);
        assertEquals("(998) 678-5667", entry3[2]);

        String[] entry4 = entriesList.get(3);
        assertEquals(22, entry4.length);
        assertEquals("Cindrella", entry4[0]);
        assertEquals("", entry4[1]);
        assertEquals("+1-213-555-1234", entry4[2]);

        String[] entry5 = entriesList.get(4);
        assertEquals(22, entry5.length);
        assertEquals("Micha3l", entry5[0]);
        assertEquals("Jackson", entry5[1]);
        assertEquals("+40 (21)313 17 -98", entry5[2]);

        String[] entry6 = entriesList.get(5);
        assertEquals(22, entry6.length);
        assertEquals("John", entry6[0]);
        assertEquals("Mc'Donald", entry6[1]);
        assertEquals("ESN-8776", entry6[2]);

        String[] entry7 = entriesList.get(6);
        assertEquals(22, entry7.length);
        assertEquals("Frank", entry7[0]);
        assertEquals("Dawson", entry7[1]);
        assertEquals("+1-919-676-9515", entry7[2]);
        assertEquals("+34(345)112-345", entry7[3]);
        assertEquals("+34 (445) 43 22", entry7[4]);
        assertEquals("+1-919-676-9564", entry7[5]);
        assertEquals("Frank_Dawson@Lotus.com", entry7[6]);
        assertEquals("fdawson@earthlink.net", entry7[7]);
        assertEquals("Lotus Development Corporation", entry7[8]);
        assertEquals("Senior Programmer", entry7[9]);
        assertEquals("IT Dept", entry7[10]);
        assertEquals("501 E. Middlefield Rd.", entry7[11]);
        assertEquals("94043", entry7[12]);
        assertEquals("U.S.A.", entry7[13]);
        assertEquals("CA", entry7[14]);
        assertEquals("Mountain View", entry7[15]);
        assertEquals("6544 Battleford Drive", entry7[16]);
        assertEquals("27613-3502", entry7[17]);
        assertEquals("U.S.A.", entry7[18]);
        assertEquals("NC", entry7[19]);
        assertEquals("Raleigh", entry7[20]);
        assertEquals("workpostoffice", entry7[21]);

        String[] entry8 = entriesList.get(7);
        assertEquals(22, entry8.length);
        assertEquals("Cristiano", entry8[0]);
        assertEquals("Ronaldo", entry8[1]);
        assertEquals("123 (89) 123", entry8[2]);
        assertEquals("(123) 23 44", entry8[3]);
        assertEquals("123 34 55", entry8[4]);
        assertEquals("123 32 321", entry8[5]);
        assertEquals("cristiano.ronaldo@realmadrid.com", entry8[6]);
        assertEquals("ronaldo@yahoo.com", entry8[7]);
        assertEquals("Real Madrid", entry8[8]);
        assertEquals("Football Player", entry8[9]);
        assertEquals("Player", entry8[10]);
        assertEquals("Avenida de Concha Espina", entry8[11]);
        assertEquals("29002", entry8[12]);
        assertEquals("Spain", entry8[13]);
        assertEquals("Madrid", entry8[14]);
        assertEquals("Madrid", entry8[15]);
        assertEquals("Concha Espina", entry8[16]);
        assertEquals("28036", entry8[17]);
        assertEquals("Spain", entry8[18]);
        assertEquals("Madrid", entry8[19]);
        assertEquals("Madrid", entry8[20]);
        assertEquals("Real Madrid postoffice", entry8[21]);

        String[] entry9 = entriesList.get(8);
        assertEquals(22, entry9.length);
        assertEquals("Zinedine", entry9[0]);
        assertEquals("Zidane", entry9[1]);
        assertEquals("123 (323) 32", entry9[2]);
        assertEquals("7643 25", entry9[3]);
        assertEquals("123 43 2223", entry9[4]);
        assertEquals("111 222 333", entry9[5]);
        assertEquals("zinedine.zidane@realmadrid.com", entry9[6]);
        assertEquals("zidane@gmail.com", entry9[7]);
        assertEquals("Real Madrid", entry9[8]);
        assertEquals("Assistant Manager", entry9[9]);
        assertEquals("Trainning", entry9[10]);
        assertEquals("Avenue Le-jour-se-leve", entry9[11]);
        assertEquals("92100", entry9[12]);
        assertEquals("France", entry9[13]);
        assertEquals("France", entry9[14]);
        assertEquals("Paris", entry9[15]);
        assertEquals("Real Madrid Street", entry9[16]);
        assertEquals("24334", entry9[17]);
        assertEquals("Spain", entry9[18]);
        assertEquals("Madrid", entry9[19]);
        assertEquals("Madrid", entry9[20]);
        assertEquals("", entry9[21]);
    }

    public void testParseContactWithPictureAttachment() throws Exception {
        // the contact used for this test was generated with Microsoft Outlook 2007 and
        // it contains a picture attachment

        InputStream contact = getClass().getResourceAsStream("SainaNehwal.vcf");
        Reader reader = new InputStreamReader(contact);
        VcardParserImpl parser = new VcardParserImpl();
        final List<String[]> entriesList = new ArrayList<String[]>();
        Closure add = new Closure() {
            public void execute(Object item) {
                entriesList.add((String[]) item);
            }
        };

        parser.parse(reader, add);
        assertEquals(1, entriesList.size());

        String[] entry = entriesList.get(0);
        assertEquals("Saina", entry[0]);
        assertEquals("Nehwal", entry[1]);
        assertEquals("182182", entry[2]);
        assertEquals("99867", entry[3]);
        assertEquals("456", entry[4]);
        assertEquals("6655", entry[5]);
        assertEquals("nehwalSaina@abc.com", entry[6]);
        assertEquals(null, entry[7]);
        assertEquals("ABC", entry[8]);
        assertEquals("Design Engineer", entry[9]);
        assertEquals("SCS", entry[10]);
        assertEquals("3rd Phase JPN", entry[11]);
        assertEquals("560060", entry[12]);
        assertEquals("India", entry[13]);
        assertEquals("Karnataka", entry[14]);
        assertEquals("Bangalore", entry[15]);
        assertEquals("JP Nagar", entry[16]);
        assertEquals("561111", entry[17]);
        assertEquals("India", entry[18]);
        assertEquals("Karnataka", entry[19]);
        assertEquals("Bangalore", entry[20]);
        assertEquals("", entry[21]);
    }
}
