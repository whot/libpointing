/* -*- mode: java -*-
 *
 * Bindings/Java/consoleExample/ConsoleExample.java --
 *
 * Initial software
 * Authors: Gery Casiez, Stéphane Huot
 * Copyright © Inria
 *
 * http://libpointing.org/
 *
 * This software may be used and distributed according to the terms of
 * the GNU General Public License version 2 or any later version.
 *
 */

import org.libpointing.*;
import java.awt.geom.Point2D;
import java.awt.Dimension;
import java.awt.Rectangle;
import org.libpointing.event.PointingDeviceEvent;
import org.libpointing.event.PointingDeviceListener;

public class ConsoleExample {
	
	static class MyPointingDeviceListener implements PointingDeviceListener {
	    TransferFunction func;

	    public void setTransferFunction(TransferFunction tf) {
		   func = tf;
	    }

		@Override
		public void callback(PointingDeviceEvent e) {
			System.out.println("Callback from pointing device: " + ((PointingDevice)e.getSource()).getURI());
	        System.out.println("Mouse moved at " +
	        		e.getTimeStamp() + " nano s, dx = " +
	        		e.getDx() + " mickeys, dy = " +
	        		e.getDy() + " mickeys, buttons = " + e.getButtons());
	        Point2D.Double p = func.applyd(e.getDx(), e.getDy(), e.getTimeStamp());
	        System.out.println("Corresponding cursor movement using transfer function : dx = " + p.x + " pixels dy = " + p.y + " pixels");
		} 
	}
	
  public static void main(String args[]) {
    // Display device
    DisplayDevice output = new DisplayDevice("any:");
    System.out.println(output.getURI() + ", resolution = " + output.getResolution() + " PPI");

    // Pointing device
    PointingDevice input = new PointingDevice("any:?debugLevel=1");
    System.out.println(input.getURI());

    // Transfer function
    TransferFunction func = new TransferFunction("system:", input, output);
    System.out.println("Transfer function URI: " + func.getURI());
 
    MyPointingDeviceListener listener = new MyPointingDeviceListener();
    listener.setTransferFunction(func);
    input.addPointingDeviceListener(listener);
	while (true) {
		PointingDevice.idle(100);
  	}
}
}
