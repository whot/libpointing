/* -*- mode: java -*-
 *
 * Bindings/Java/Jar/org/libpointing/PointingDeviceManager.java --
 *
 * Initial software
 * Authors: Izzat Mukhanov, Stéphane Huot
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

import org.libpointing.event.PointingDeviceManagerListener;

public class PointingDeviceManager {

    // Load the library
    static {
		System.loadLibrary("pointing");
		instance = new PointingDeviceManager();
    }
	
	private native void setCallbacks();
	public native PointingDeviceDescriptor[] getDeviceList();
	public native int size();

	private static PointingDeviceManager instance;
	
    protected EventListenerList listenerList = new EventListenerList();
	
	private PointingDeviceManager() {
		setCallbacks();
	}
	
	private void deviceAdded(PointingDeviceDescriptor desc) {
    	Object[] listeners = listenerList.getListenerList();
    	// Process the listeners last to first, notifying
    	// those that are interested in this event
    	for (int i = listeners.length-2; i>=0; i-=2) {
    		if (listeners[i]==PointingDeviceManagerListener.class) {
    			((PointingDeviceManagerListener)listeners[i+1]).deviceAdded(desc);
    		}
    	}		
	}
	
	private void deviceRemoved(PointingDeviceDescriptor desc) {
    	Object[] listeners = listenerList.getListenerList();
    	// Process the listeners last to first, notifying
    	// those that are interested in this event
    	for (int i = listeners.length-2; i>=0; i-=2) {
    		if (listeners[i]==PointingDeviceManagerListener.class) {
    			((PointingDeviceManagerListener)listeners[i+1]).deviceRemoved(desc);
    		}
    	}			
	}
	
	static public PointingDeviceManager getInstance() {
		return instance;
	}
	
    public void addPointingDeviceManagerListener(PointingDeviceManagerListener l) {
    	listenerList.add(PointingDeviceManagerListener.class, l);
    }
    
    public void removePointingDeviceManagerListener(PointingDeviceManagerListener l) {
    	listenerList.remove(PointingDeviceManagerListener.class, l);
    }
    
    public PointingDeviceManagerListener[] getPointingDeviceManagerListeners() {
    	return listenerList.getListeners(PointingDeviceManagerListener.class);
    }
}
