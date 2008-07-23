/**
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.commons.util;

import java.lang.reflect.*;

/**
 * Load the specified sipXcommons native library.
 * <p>
 * 
 * @author Mardy Marshall
 */
public class LibraryLoader {
    static final String sipXcommons = "sipXcommons";

    public static void loadLibrary(String libname) throws UnsatisfiedLinkError {
        String sipXcommonsClassPath = null;
        String absoluteLibPath = null;

        try {
            // First attempt to simply load the library.
            System.loadLibrary(libname);
        } catch (UnsatisfiedLinkError e1) {
            // Since it threw an exception, try to see if the library is listed in
            // the classpath. If it is, then try to load the library directly. If
            // not, then update the native library path to include the sipXcommons
            // directory and try again. To determine the path, parse the
            // "java.class.path" and search for sipXcommons.
            String javaClassPaths = System.getProperty("java.class.path");
            if (javaClassPaths != null) {
                String pathSeparator = System.getProperty("path.separator");
                if (pathSeparator == null) {
                    pathSeparator = ":";
                }
                String javaClassPathList[] = javaClassPaths.split(pathSeparator);
                for (int i = 0; i < javaClassPathList.length; i++) {
                    if (javaClassPathList[i].contains(libname)) {
                        absoluteLibPath = javaClassPathList[i];
                        break;
                    }
                }

                // Try loading directly.
                if (absoluteLibPath != null) {
                    boolean absoluteLoadSucceeded = true;
                    try {
                        System.load(absoluteLibPath);
                    } catch (UnsatisfiedLinkError e4) {
                        absoluteLoadSucceeded = false;
                    } catch (SecurityException e5) {
                        absoluteLoadSucceeded = false;
                    }
                    if (absoluteLoadSucceeded) {
                        return;
                    }
                }

                // Direct loading of the library didn't work so update the library path.
                for (int i = 0; i < javaClassPathList.length; i++) {
                    if (javaClassPathList[i].contains(sipXcommons)) {
                        sipXcommonsClassPath = javaClassPathList[i].substring(0, javaClassPathList[i].indexOf(sipXcommons) + sipXcommons.length());
                        break;
                    }
                }

                if (sipXcommonsClassPath == null) {
                    throw new UnsatisfiedLinkError("Failed to determine sipXcommons library path");
                }

                // Now that the sipXcommons class path has been determined, add it
                // to the default class loaders "user_paths" field.
                try {
                    // Get a copy of the user_paths field.
                    Field user_paths_field = ClassLoader.class.getDeclaredField("usr_paths");
                    user_paths_field.setAccessible(true);
                    String[] user_paths_copy = (String[]) user_paths_field.get(null);
                    // Scan it to see if the sipXcommons path is already included.
                    for (int i = 0; i < user_paths_copy.length; i++) {
                        if (sipXcommonsClassPath.equals(user_paths_copy[i])) {
                            // If it is, no sense going any further.
                            throw new UnsatisfiedLinkError("Failed to load library: " + libname);
                        }
                    }
                    // Append the sipXcommons path and update the user_paths field.
                    String[] updated_user_paths = new String[user_paths_copy.length + 1];
                    System.arraycopy(user_paths_copy, 0, updated_user_paths, 0, user_paths_copy.length);
                    updated_user_paths[user_paths_copy.length] = sipXcommonsClassPath;
                    user_paths_field.set(null, updated_user_paths);
                } catch (IllegalAccessException e2) {
                    throw new UnsatisfiedLinkError("Failed to get permissions to set library path");
                } catch (NoSuchFieldException e3) {
                    throw new UnsatisfiedLinkError("Failed to get field handle to set library path");
                }
                try {
                    System.loadLibrary(libname);
                } catch (UnsatisfiedLinkError e4) {
                    throw new UnsatisfiedLinkError("Failed to load library: " + libname);
                } catch (SecurityException e5) {
                    throw new UnsatisfiedLinkError("Security error: checkLink method doesn't allow loading of the specified dynamic library");
                }
            }
        }
    }
}
