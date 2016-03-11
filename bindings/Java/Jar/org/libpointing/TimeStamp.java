/* -*- mode: java -*-
 *
 * Bindings/Java/Jar/org/libpointing/TimeStamp.java --
 *
 * Initial software
 * Authors: Gery Casiez
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

public class TimeStamp {
    // Native method declaration
    public native long getTimestamp();
    
    //Load the library
    static {
		System.loadLibrary("pointing");
    }
}
