/* -*- mode: java -*-
 *
 * Bindings/Java/Jar/org/libpointing/PointingDevice.java --
 *
 * Initial software
 * Authors: Gery Casiez, Stéphane Huot, Izzat Mukhanov
 * Copyright © Inria
 *
 * http://libpointing.org/
 *
 * This software may be used and distributed according to the terms of
 * the GNU General Public License version 2 or any later version.
 *
 */
package org.libpointing;

import javax.swing.event.EventListenerList;

import org.libpointing.event.PointingDeviceEvent;
import org.libpointing.event.PointingDeviceListener;

public class PointingDevice {

    // Load the library
    static {
    	System.loadLibrary("pointing");
    }
    
    // Native method declaration
    private native long initPointingDevice(String uri);
    private native void releasePointingDevice();
    public native String getURI();
    public native double getResolution();
    public native double getUpdateFrequency();
    public native boolean isActive();
    public native int getVendorID();
    public native String getVendor();
    public native int getProductID();
    public native String getProduct();
    public native void setDebugLevel(int i);
    public static native void idle(int t);

    // Java part
    private long nativeHandle;

    public PointingDevice(String uri) {
        nativeHandle = initPointingDevice(uri);
    }
    
    @Override
    public void finalize() {
    	releasePointingDevice();
    }

    protected EventListenerList listenerList = new EventListenerList();
    
    private void callback(long timeStamp, int input_dx, int input_dy, int buttons) {
	    Object[] listeners = listenerList.getListenerList();
	    PointingDeviceEvent e = new PointingDeviceEvent(PointingDevice.this, timeStamp, input_dx, input_dy, buttons);
	    // Process the listeners last to first, notifying
	    // those that are interested in this event
	    for (int i = listeners.length-2; i>=0; i-=2) {
	    	if (listeners[i]==PointingDeviceListener.class) {
	    		((PointingDeviceListener)listeners[i+1]).callback(e);
	    	}
	    }
    }
    
    public void addPointingDeviceListener(PointingDeviceListener l) {
    	listenerList.add(PointingDeviceListener.class, l);
    }
    
    public void removePointingDeviceListener(PointingDeviceListener l) {
    	listenerList.remove(PointingDeviceListener.class, l);
    }
    
    public PointingDeviceListener[] getPointingDeviceListeners() {
    	return listenerList.getListeners(PointingDeviceListener.class);
    }
}
