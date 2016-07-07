/* -*- mode: java -*-
 *
 * Bindings/Java/Jar/org/libpointing/PointingDeviceListener.java --
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

public interface PointingDeviceListener extends EventListener {

	public void callback(PointingDeviceEvent e);
	
}
