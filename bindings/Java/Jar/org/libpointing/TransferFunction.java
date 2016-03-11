/* -*- mode: java -*-
 *
 * Bindings/Java/Jar/org/libpointing/TransferFunction.java --
 *
 * Initial software
 * Authors: Gery Casiez, Izzat Mukhanov
 * Copyright Inria
 *
 * http://libpointing.org/
 *
 * This software may be used and distributed according to the terms of
 * the GNU General Public License version 2 or any later version.
 *
 */
package org.libpointing;

import java.awt.Point;
import java.awt.geom.Point2D;

public class TransferFunction {

	// Load the library
    static {
		System.loadLibrary("pointing");
    }
    
    // Native method declaration
    private native long initTransferFunction(String uri, PointingDevice input, DisplayDevice output);
    private native void releaseTransferFunction();
    public native String getURI();
    public native Point applyi(int input_dx, int input_dy, long timestamp);
    public native Point2D.Double applyd(int input_dx, int input_dy, long timestamp);
    public native void clearState();

    // Java part
    private long nativeHandle;

    public TransferFunction(String uri, PointingDevice input, DisplayDevice output) {
    	nativeHandle = initTransferFunction(uri, input, output);
    }
    
    @Override
    public void finalize() {
        releaseTransferFunction();
    }
}