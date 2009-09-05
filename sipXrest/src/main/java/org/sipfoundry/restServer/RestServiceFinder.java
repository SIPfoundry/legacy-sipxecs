package org.sipfoundry.restServer;

import java.io.File;
import java.io.FileInputStream;
import java.io.FilenameFilter;
import java.io.IOException;
import java.lang.reflect.Method;
import java.net.URL;
import java.net.URLClassLoader;
import java.util.ArrayList;
import java.util.List;
import java.util.jar.JarEntry;
import java.util.jar.JarInputStream;

import org.restlet.Restlet;
import org.apache.log4j.Logger;

class JarFilter implements FilenameFilter {
    public boolean accept(File dir, String name) {
        return (name.endsWith(".jar"));
    }
}

public class RestServiceFinder {

    // Parameters
    private static final Class[] parameters = new Class[]{URL.class};
    private static Logger logger = Logger.getLogger(RestServiceFinder.class);

    private List<IPlugin> pluginCollection;

    public RestServiceFinder() {
        pluginCollection = new ArrayList<IPlugin>(5);
    }

    public void search(String directory) throws Exception {
        logger.debug("Location to search " + directory);
        File dir = new File(directory);
        logger.debug("Searching directory for plugins");
        if (dir.isFile()) {
            return;
        }

        File[] files = dir.listFiles(new JarFilter());
        for (File f : files) {
            logger.debug("Jar files in directory = " + f.toString());
            List<String> classNames = getClassNames(f.getAbsolutePath());
            for (String className : classNames) {
                // Remove the ".class" at the back
                String name = className.substring(0, className.length() - 6);
                Class clazz = getClass(f, name);
                Class[] interfaces = clazz.getInterfaces();
                for (Class c : interfaces) {
                    // Implement the IPlugin interface
                    if (c.getName().endsWith("IPlugin")) {
                        logger.debug("Plugin support found in  = " + name);
                        pluginCollection.add((IPlugin)clazz.newInstance());
                    }
                }
            }
        }
    }

    protected List<String> getClassNames(String jarName) throws IOException {
        ArrayList<String> classes = new ArrayList<String>(10);
        JarInputStream jarFile = new JarInputStream(new FileInputStream(jarName));
        JarEntry jarEntry;
        while (true) {
            jarEntry = jarFile.getNextJarEntry();
            if (jarEntry == null) {
                break;
            }
            if (jarEntry.getName().endsWith(".class")) {
                classes.add(jarEntry.getName().replaceAll("/", "\\."));
            }
        }

        return classes;
    }

    public Class getClass(File file, String name) throws Exception {
        addURL(file.toURL());

        URLClassLoader clazzLoader;
        Class clazz;
        String filePath = file.getAbsolutePath();
        filePath = "jar:file://" + filePath + "!/";
        URL url = new File(filePath).toURL();
        clazzLoader = new URLClassLoader(new URL[]{url});
        clazz = clazzLoader.loadClass(name);
        return clazz;

    }

    public void addURL(URL u) throws IOException {
        URLClassLoader sysLoader = (URLClassLoader) ClassLoader.getSystemClassLoader();
        URL urls[] = sysLoader.getURLs();
        for (int i = 0; i < urls.length; i++) {
            if (urls[i].toString().equalsIgnoreCase(u.toString())) {
                return;
            }
        }
        Class sysclass = URLClassLoader.class;
        try {
            Method method = sysclass.getDeclaredMethod("addURL", parameters);
            method.setAccessible(true);
            method.invoke(sysLoader, new Object[]{u});
        } catch (Throwable t) {
            t.printStackTrace();
            throw new IOException("Error, could not add URL to system classloader");
        }
    }


    public List<IPlugin> getPluginCollection() {
        return pluginCollection;
    }

    public void setPluginCollection(List<IPlugin> pluginCollection) {
        this.pluginCollection = pluginCollection;
    }
}
