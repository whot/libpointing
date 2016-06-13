# Control Mouse on Mac Os

With this sample application one can change the transfer function on Mac Os with any transfer function available in libpointing.

In the default initialization Subpixeling is used for the default PointingDevice.

You can change it to make it work with your code.

With this application you can introduce SubPixeling behavior into your system.

## Limitations

This application does not change the transfer function globally in the system. It only allows you to modify mouse events.

Therefore, the system transfer function is not changed properly. The cursor position is modified with libpointing callbacks.

However, if the cursor coordinates are modified with EventTap the cursor behaves normally.

Note that there is no support for the Magic Trackpad (external).