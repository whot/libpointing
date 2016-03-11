/* -*- mode: java -*-
 *
 * Bindings/Java/Jar/org/libpointing/DisplayDevice.java --
 *
 * Initial software
 * Authors: Gery Casiez, Izzat Mukhanov
 * Copyright Inria
 *
 * http://libpointing.org/
 *
 * This software may be used and distributed according to the terms of
 * the GNU General Public License version 2 or any later version.
 *
 */
package org.libpointing;

import java.awt.Rectangle;
import java.awt.Dimension;

public class DisplayDevice {

    // Load the library
    static {
        System.loadLibrary("pointing");
    }

    // Native method declaration
    private native long initDisplayDevice(String uri);
    private native void releaseDisplayDevice();
    public native String getURI();
    public native double getResolution();
    public native double getRefreshRate();
    public native Rectangle getBounds();
    public native Dimension getSize();
    public native void setDebugLevel(int debugLevel);

    // Java part
    private long nativeHandle;

    public DisplayDevice(String uri) {
        nativeHandle = initDisplayDevice(uri);
    }
    
    @Override
    public void finalize() {
        releaseDisplayDevice();
    }
}