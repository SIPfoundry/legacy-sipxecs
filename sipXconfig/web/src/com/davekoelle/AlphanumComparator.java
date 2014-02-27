/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package com.davekoelle;

import java.util.Arrays;
import java.util.Comparator;
import java.util.List;

public class AlphanumComparator implements Comparator<String> {
    // TO USE THIS:
    // Use the static "sort" method from the java.util.Collections class:
    //
    // Collections.sort(your list, new AlphanumComparator());
    //

    private static List<Character> NUMBERS = Arrays.asList(new Character[] {
        '1', '2', '3', '4', '5', '6', '7', '8', '9', '0'
    });

    private static boolean inChunk(char ch, String s) {
        if (s.length() == 0) {
            return true;
        }

        char s0 = s.charAt(0);
        int chunkType = 0; // 0 = alphabetic, 1 = numeric

        if (NUMBERS.contains(s0)) {
            chunkType = 1;
        }

        if ((chunkType == 0) && (NUMBERS.contains(ch))) {
            return false;
        }
        if ((chunkType == 1) && (!NUMBERS.contains(ch))) {
            return false;
        }

        return true;
    }

    @Override
    public int compare(String s1, String s2) {
        // This is soo much easier in a pattern-matching language like Perl!

        if (s1 == null && s2 != null) {
            return -1;
        }
        if (s1 != null && s2 == null) {
            return 1;
        }
        if (s1 == null && s2 == null) {
            return 0;
        }

        int thisMarker = 0;
        int thisNumericChunk = 0;
        String thisChunk = new String();
        int thatMarker = 0;
        int thatNumericChunk = 0;
        String thatChunk = new String();

        while ((thisMarker < s1.length()) && (thatMarker < s2.length())) {
            char thisCh = s1.charAt(thisMarker);
            char thatCh = s2.charAt(thatMarker);

            thisChunk = "";
            thatChunk = "";

            while ((thisMarker < s1.length()) && inChunk(thisCh, thisChunk)) {
                thisChunk = thisChunk + thisCh;
                thisMarker++;
                if (thisMarker < s1.length()) {
                    thisCh = s1.charAt(thisMarker);
                }
            }

            while ((thatMarker < s2.length()) && inChunk(thatCh, thatChunk)) {
                thatChunk = thatChunk + thatCh;
                thatMarker++;
                if (thatMarker < s2.length()) {
                    thatCh = s2.charAt(thatMarker);
                }
            }

            int thisChunkType = NUMBERS.contains(thisChunk.charAt(0)) ? 1 : 0;
            int thatChunkType = NUMBERS.contains(thatChunk.charAt(0)) ? 1 : 0;

            // If both chunks contain numeric characters, sort them numerically
            int result = 0;
            if ((thisChunkType == 1) && (thatChunkType == 1)) {
                thisNumericChunk = Integer.parseInt(thisChunk);
                thatNumericChunk = Integer.parseInt(thatChunk);
                if (thisNumericChunk < thatNumericChunk) {
                    result = -1;
                }
                if (thisNumericChunk > thatNumericChunk) {
                    result = 1;
                }
            } else {
                result = thisChunk.compareTo(thatChunk);
            }

            if (result != 0) {
                return result;
            }
        }

        return 0;
    }
}
