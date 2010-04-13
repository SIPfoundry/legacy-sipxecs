/**
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.commons.util;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;

/**
 * Load the specified sipXcommons native library.
 * <p>
 *
 * @author Mardy Marshall
 */
public class LibraryLoader {
    static final String sipxcommons = "sipxcommons";

    public static void loadLibrary(String libname) throws UnsatisfiedLinkError {
        String sipXcommonsJarPath = null;
        String absoluteLibPath = null;
        String mappedLibName = System.mapLibraryName(libname);
        FileOutputStream fileOutputStream = null;
        InputStream inputStream = null;
        String libraryFile = null;

        try {
            System.loadLibrary(libname);
        } catch (UnsatisfiedLinkError e1) {
            // First see if the library is listed in the classpath. If it is, then
            // try to load the library directly.
            String javaClassPaths = System.getProperty("java.class.path");
            if (javaClassPaths != null) {
                String pathSeparator = System.getProperty("path.separator");
                if (pathSeparator == null) {
                    pathSeparator = ":";
                }
                String javaClassPathList[] = javaClassPaths.split(pathSeparator);
                for (int i = 0; i < javaClassPathList.length; i++) {
                    if (javaClassPathList[i].contains(mappedLibName)) {
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

                // Direct loading of the library didn't work so try loading from the directory
                // where the sipxcommons.jar file resides. If the library is not found, then
                // try extracting the library from the sipxcommons.jar file and then loading.
                for (int i = 0; i < javaClassPathList.length; i++) {
                    if (javaClassPathList[i].contains(sipxcommons) && javaClassPathList[i].contains(".jar")) {
                        sipXcommonsJarPath = javaClassPathList[i].substring(0, javaClassPathList[i].indexOf(sipxcommons));
                        break;
                    }
                }

                if (sipXcommonsJarPath == null) {
                    throw new UnsatisfiedLinkError("Failed to determine sipXcommons library path");
                }

                libraryFile = sipXcommonsJarPath + mappedLibName;
                File file = new File(libraryFile);
                if (!file.exists()) {
                    try {
                        inputStream = LibraryLoader.class.getResourceAsStream("/" + mappedLibName);
                        if (inputStream != null) {
                            int read;
                            byte[] buffer = new byte[4096];
                            fileOutputStream = new FileOutputStream(libraryFile);
                            while ((read = inputStream.read(buffer)) != -1) {
                                fileOutputStream.write(buffer, 0, read);
                            }
                            fileOutputStream.close();
                            inputStream.close();
                        }
                    } catch (Throwable e) {
                        try {
                            if (fileOutputStream != null) {
                                fileOutputStream.close();
                            }
                        } catch (IOException e2) {
                        }
                        try {
                            if (inputStream != null) {
                                inputStream.close();
                            }
                        } catch (IOException e2) {
                        }
                    }
                }

                // Now try loading.
                try {
                    System.load(libraryFile);
                } catch (UnsatisfiedLinkError e4) {
                    throw new UnsatisfiedLinkError("Failed to load library: " + mappedLibName);
                } catch (SecurityException e5) {
                    throw new UnsatisfiedLinkError(
                            "Security error: checkLink method doesn't allow loading of the specified dynamic library");
                }
            }
        }
    }
}
