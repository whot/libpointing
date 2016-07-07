/* -*- mode: java -*-
 *
 * Bindings/Java/Jar/org/libpointing/DisplayDeviceDescriptor.java --
 *
 * Initial software
 * Authors: Izzat Mukhanov
 * Copyright Inria
 *
 * http://libpointing.org/
 *
 * This software may be used and distributed according to the terms of
 * the GNU General Public License version 2 or any later version.
 *
 */
package org.libpointing;

public class DisplayDeviceDescriptor {

	public String devUri;
	public String name;
    public int width; // In pixels
    public int height;

    public DisplayDeviceDescriptor(String sDevUri, String sName, int sWidth, int sHeight)
    {
    	devUri = sDevUri;
        name = sName;
    	width = sWidth;
    	height = sHeight;
    }
}