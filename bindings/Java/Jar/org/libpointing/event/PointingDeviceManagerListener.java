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

import java.util.EventListener;

import org.libpointing.PointingDeviceDescriptor;

public interface PointingDeviceManagerListener extends EventListener {

	public void deviceAdded(PointingDeviceDescriptor desc);
	public void deviceRemoved(PointingDeviceDescriptor desc);
	
}
