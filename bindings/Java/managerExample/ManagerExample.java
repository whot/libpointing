/* -*- mode: java -*-
 *
 * Bindings/Java/consoleExample/ManagerExample.java --
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

import java.util.*;

import org.libpointing.*;
import org.libpointing.event.*;

public class ManagerExample {
	
	static class MyPointingDeviceManagerListener implements PointingDeviceManagerListener {

	    public void deviceAdded(PointingDeviceDescriptor desc)
	    {
	        //System.out.println(Arrays.toString(getDeviceList()));
	    	System.out.println("Device added:");
	        System.out.println("Device " + desc.name + " was added with URI: " + desc.devUri);
	        System.out.println("Vendor Id: " + desc.vendorID + " Product Id: " + desc.productID + "\n");
	    }

	    public void deviceRemoved(PointingDeviceDescriptor desc)
	    {
	        //System.out.println(Arrays.toString(getDeviceList()));
	    	System.out.println("Device removed:");
	        System.out.println("Device " + desc.name + " was removed with URI: " + desc.devUri + "\n");
	    }
	}
	
  public static void main(String args[]) {

    PointingDevice input = new PointingDevice("any:");
    
    PointingDeviceManager manager = PointingDeviceManager.getInstance();
    manager.addPointingDeviceManagerListener(new MyPointingDeviceManagerListener());
    // It is better to use callbacks, not getDeviceList
    // Because devices are found with delay, not immediately
    for (PointingDeviceDescriptor desc : manager.getDeviceList()) {
        System.out.println(desc.name);
    }
    //System.out.println(Arrays.toString(manager.getDeviceList()));

	while (true) {
		PointingDevice.idle(100);
	}
  }
}
