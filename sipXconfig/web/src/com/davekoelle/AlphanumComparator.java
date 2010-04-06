/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package com.davekoelle;

import java.util.Comparator;

public class AlphanumComparator implements Comparator
{
  // TO USE THIS:
  //   Use the static "sort" method from the java.util.Collections class:
  //
  //   Collections.sort(your list, new AlphanumComparator());
  //

  char[] numbers = {'1','2','3','4','5','6','7','8','9','0'};

  private boolean isIn(char ch, char[] chars)
  {
    for (int i=0; i < chars.length; i++) {
      if (ch == chars[i]) return true;
    }
    return false;
  }

  private boolean inChunk(char ch, String s)
  {
    if (s.length() == 0) return true;

    char s0 = s.charAt(0);
    int chunkType = 0;   // 0 = alphabetic, 1 = numeric

    if (isIn(s0,numbers)) chunkType = 1;

    if ((chunkType == 0) && (isIn(ch,numbers))) return false;
    if ((chunkType == 1) && (!isIn(ch,numbers))) return false;

    return true;
  }

  public int compare(Object o1, Object o2)
  {
    // This is soo much easier in a pattern-matching
    // language like Perl!

    String s1 = (String)o1;
    String s2 = (String)o2;

    int thisMarker = 0;  int thisNumericChunk = 0;  String thisChunk = new String();
    int thatMarker = 0;  int thatNumericChunk = 0;  String thatChunk = new String();

    while ((thisMarker < s1.length()) && (thatMarker < s2.length())) {
      char thisCh = s1.charAt(thisMarker);
      char thatCh = s2.charAt(thatMarker);

      thisChunk = "";
      thatChunk = "";

      while ((thisMarker < s1.length()) && inChunk(thisCh,thisChunk)) {
        thisChunk = thisChunk + thisCh;
        thisMarker++;
        if (thisMarker < s1.length()) thisCh = s1.charAt(thisMarker);
      }

      while ((thatMarker < s2.length()) && inChunk(thatCh,thatChunk)) {
        thatChunk = thatChunk + thatCh;
        thatMarker++;
        if (thatMarker < s2.length()) thatCh = s2.charAt(thatMarker);
      }

      int thisChunkType = isIn(thisChunk.charAt(0),numbers) ? 1:0;
      int thatChunkType = isIn(thatChunk.charAt(0),numbers) ? 1:0;

      // If both chunks contain numeric characters, sort them numerically
      int result = 0;
      if ((thisChunkType == 1) && (thatChunkType == 1)) {
        thisNumericChunk = Integer.parseInt(thisChunk);
        thatNumericChunk = Integer.parseInt(thatChunk);
        if (thisNumericChunk < thatNumericChunk) result = -1;
        if (thisNumericChunk > thatNumericChunk) result =  1;
      } else {
        result = thisChunk.compareTo(thatChunk);
      }

      if (result != 0) return result;
    }

    return 0;
  }
}
