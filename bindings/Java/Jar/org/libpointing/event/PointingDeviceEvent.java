/* -*- mode: java -*-
 *
 * Bindings/Java/Jar/org/libpointing/PointingDevice.java --
 *
 * Initial software
 * Authors: Stéphane Huot
 * Copyright © Inria
 *
 * http://libpointing.org/
 *
 * This software may be used and distributed according to the terms of
 * the GNU General Public License version 2 or any later version.
 *
 */
package org.libpointing.event;

import java.util.EventObject;

public class PointingDeviceEvent extends EventObject {

	private static final long serialVersionUID = -6242952300655390835L;

	long timeStamp;
	int input_dx, input_dy;
	int buttons;
	
	public PointingDeviceEvent(Object source, long timeStamp, int input_dx, int input_dy, int buttons) {
		super(source);
		this.timeStamp = timeStamp;
		this.input_dx = input_dx;
		this.input_dy = input_dy;
		this.buttons = buttons;
	}

	/**
	 * @return the timeStamp
	 */
	public long getTimeStamp() {
		return timeStamp;
	}

	/**
	 * @return the dx
	 */
	public int getDx() {
		return input_dx;
	}

	/**
	 * @return the dy
	 */
	public int getDy() {
		return input_dy;
	}

	/**
	 * @return the buttons
	 */
	public int getButtons() {
		return buttons;
	}
	
	

}
