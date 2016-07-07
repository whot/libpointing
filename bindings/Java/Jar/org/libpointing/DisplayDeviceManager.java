/* -*- mode: java -*-
 *
 * Bindings/Java/Jar/org/libpointing/DisplayDeviceManager.java --
 *
 * Initial software
 * Authors: Izzat Mukhanov
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

import org.libpointing.event.DisplayDeviceManagerListener;

public class DisplayDeviceManager {

    // Load the library
    static {
		System.loadLibrary("pointing");
		instance = new DisplayDeviceManager();
    }
	
	private native void setCallbacks();
	public native DisplayDeviceDescriptor[] getDeviceList();
	public native int size();

	private static DisplayDeviceManager instance;
	
    protected EventListenerList listenerList = new EventListenerList();
	
	private DisplayDeviceManager() {
		setCallbacks();
	}
	
	private void deviceAdded(DisplayDeviceDescriptor desc) {
    	Object[] listeners = listenerList.getListenerList();
    	// Process the listeners last to first, notifying
    	// those that are interested in this event
    	for (int i = listeners.length-2; i>=0; i-=2) {
    		if (listeners[i]==DisplayDeviceManagerListener.class) {
    			((DisplayDeviceManagerListener)listeners[i+1]).deviceAdded(desc);
    		}
    	}		
	}
	
	private void deviceRemoved(DisplayDeviceDescriptor desc) {
    	Object[] listeners = listenerList.getListenerList();
    	// Process the listeners last to first, notifying
    	// those that are interested in this event
    	for (int i = listeners.length-2; i>=0; i-=2) {
    		if (listeners[i]==DisplayDeviceManagerListener.class) {
    			((DisplayDeviceManagerListener)listeners[i+1]).deviceRemoved(desc);
    		}
    	}			
	}
	
	static public DisplayDeviceManager getInstance() {
		return instance;
	}
	
    public void addDisplayDeviceManagerListener(DisplayDeviceManagerListener l) {
    	listenerList.add(DisplayDeviceManagerListener.class, l);
    }
    
    public void removeDisplayDeviceManagerListener(DisplayDeviceManagerListener l) {
    	listenerList.remove(DisplayDeviceManagerListener.class, l);
    }
    
    public DisplayDeviceManagerListener[] getDisplayDeviceManagerListeners() {
    	return listenerList.getListeners(DisplayDeviceManagerListener.class);
    }
}
