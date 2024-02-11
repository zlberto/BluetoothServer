# Bluetooth Server

```
#include <iostream>
#include BluetoothServer.h"

int main() {
    // Create a BluetoothServer instance with your desired device alias
    BluetoothServer server;
	server.registerAgent();
	server.setAlias("MyIoTDeviceTreatmo");
	server.powerOnAdapter();
	server.makeDiscoverable();
	server.makePairable();
	server.startAdvertising();
	

	// Initialize the GLib main loop
    GMainLoop* loop = g_main_loop_new(NULL, FALSE);

	// Start the main loop
    g_main_loop_run(loop);

    // Cleanup the main loop when the application exits
    g_main_loop_unref(loop);

    return 0;
}
```
