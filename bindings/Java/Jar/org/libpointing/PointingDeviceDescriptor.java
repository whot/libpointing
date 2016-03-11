/* -*- mode: java -*-
 *
 * Bindings/Java/Jar/org/libpointing/PointingDeviceDescriptor.java --
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

public class PointingDeviceDescriptor {

	public String devUri;
	public String name;
    public int vendorID;
    public int productID;

    public PointingDeviceDescriptor(String sDevUri, String sName, int sVendorID, int sProductID)
    {
    	devUri = sDevUri;
    	name = sName;
    	vendorID = sVendorID;
    	productID = sProductID;
    }
}