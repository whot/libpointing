/* -*- mode: java -*-
 *
 * Bindings/Java/Jar/org/libpointing/DisplayDeviceManagerListener.java --
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
package org.libpointing.event;

import java.util.EventListener;

import org.libpointing.DisplayDeviceDescriptor;

public interface DisplayDeviceManagerListener extends EventListener {

	public void deviceAdded(DisplayDeviceDescriptor desc);
	public void deviceRemoved(DisplayDeviceDescriptor desc);
	
}
