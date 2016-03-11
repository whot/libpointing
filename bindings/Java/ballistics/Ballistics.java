/* -*- mode: java -*-
 *
 * bindings/Java/consoleExample/Ballistics.java --
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
import org.libpointing.event.*;

import java.awt.*;
import java.awt.event.*;

import javax.swing.*;

class MyJPanel extends JPanel {
    Point cursor;

    MyJPanel() {
	cursor =  new Point(Ballistics.WIDTH/2, Ballistics.HEIGHT/2);
	setBackground(Color.white);
    }

    public void setCursor(Point p) {
	cursor = p;
	repaint();
    }

    public void moveCursor(Point p) {
	cursor.translate(p.x, p.y);
	repaint();
    }

    public void paintComponent(Graphics g) {
	super.paintComponent(g);
	g.drawLine(cursor.x-10, cursor.y, cursor.x+10, cursor.y);
	g.drawLine(cursor.x, cursor.y-10, cursor.x, cursor.y+10);
    }

}

public class Ballistics {
	
	static class MyPointingDeviceListener implements PointingDeviceListener {
	    TransferFunction func;
	    MyJPanel pan;

	    public void setTransferFunction(TransferFunction tf) {
		func = tf;
	    }

	    public void setPanel(MyJPanel panel) {
		pan = panel;
	    }

	    @Override
	    public void callback(PointingDeviceEvent e) 
	    { 
	        //System.out.println("Mouse moved at " + timeStamp + " nano s, dx = " + input_dx + " mickeys, dy = " + input_dy + " mickeys, buttons = " + buttons); 
	    	Point p = func.applyi(e.getDx(), e.getDy(), e.getTimeStamp());
	    	//System.out.println("Corresponding cursor movement using transfer function : dx = " + Dxy.deltax + " pixels dy = " + Dxy.deltay + " pixels");
	    	if (pan!= null) pan.moveCursor(p);
	    } 
	}
	
	
    public static final int WIDTH = 1024;
    public static final int HEIGHT = 768;
    Insets insets;

    Ballistics() {
	// Display device
	DisplayDevice output = new DisplayDevice("any:");
	System.out.println(output.getURI() + ", resolution = " + output.getResolution() + " PPI");
	
	// Pointing device
	//PointingDevice input = new PointingDevice("osxhid:/USB/fa120000/AppleUSBTCButtons");
	PointingDevice input = new PointingDevice("any:");
	System.out.println(input.getURI());
	
	// Transfer function
	TransferFunction func = new TransferFunction("system:", input, output);
	System.out.println("Transfer function URI: " + func.getURI());
	
	MyPointingDeviceListener listener = new MyPointingDeviceListener();
	listener.setTransferFunction(func); 
	input.addPointingDeviceListener(listener);

	final JFrame fen = new JFrame("Ballistics");
	fen.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);

	final MyJPanel pan = new MyJPanel();
	pan.setPreferredSize(new Dimension(WIDTH,HEIGHT));
	listener.setPanel(pan);

	fen.addKeyListener(new KeyAdapter()
    {
        public void keyTyped(KeyEvent e) {
		Point p = fen.getLocation();
		PointerInfo a = MouseInfo.getPointerInfo();
		Point b = a.getLocation();
		int x = (int) b.getX();
		int y = (int) b.getY();
		pan.setCursor(new Point(x-p.x-insets.left,y-p.y-insets.top));
	    }
	 });

	fen.getContentPane().add(pan);
	fen.pack();
	fen.setLocationRelativeTo(null);
	insets = fen.getInsets();
	fen.setVisible(true);
    }

    public static void main(String args[]) {
	//Schedule a job for the event-dispatching thread:
	//creating and showing this application's GUI.
	javax.swing.SwingUtilities.invokeLater(new Runnable() {
		public void run() {
		    new Ballistics();
		}
	    });
    }
}
