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
package org.libpointing;

public class PointingDeviceNotInitializedException extends Exception {

	private static final long serialVersionUID = 4598718003162565699L;

	private String addMessage = "";

	public PointingDeviceNotInitializedException() {
		this("");
	}
	
	public PointingDeviceNotInitializedException(String message) {
		super();
		addMessage = message;
	}
	
	@Override
	public String getMessage() {
		if (addMessage != null && ! addMessage.equals(""))
			return addMessage;
		return "PointingDevice not initialized";
	}
	
}
