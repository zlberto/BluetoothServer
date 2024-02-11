#include "tbluetooth/BluetoothServer.h"
#include <iostream>

// Define the interface VTable to link method names with callbacks
static const gchar *agent_introspection_xml =
    "<node>"
    "  <interface name='org.bluez.Agent1'>"
    "    <method name='Release' />"
    "    <method name='RequestPinCode'>"
    "      <arg type='o' name='device' direction='in'/>"
    "      <arg type='s' name='pincode' direction='out'/>"
    "    </method>"
    "    <method name='DisplayPinCode'>"
    "      <arg type='o' name='device' direction='in'/>"
    "      <arg type='s' name='pincode' direction='in'/>"
    "    </method>"
    "    <method name='RequestPasskey'>"
    "      <arg type='o' name='device' direction='in'/>"
    "      <arg type='u' name='passkey' direction='out'/>"
    "    </method>"
    "    <method name='DisplayPasskey'>"
    "      <arg type='o' name='device' direction='in'/>"
    "      <arg type='u' name='passkey' direction='in'/>"
    "      <arg type='q' name='entered' direction='in'/>"
    "    </method>"
    "    <method name='RequestConfirmation'>"
    "      <arg type='o' name='device' direction='in'/>"
    "      <arg type='u' name='passkey' direction='in'/>"
    "    </method>"
    "    <method name='RequestAuthorization'>"
    "      <arg type='o' name='device' direction='in'/>"
    "    </method>"
    "    <method name='AuthorizeService'>"
    "      <arg type='o' name='device' direction='in'/>"
    "      <arg type='s' name='uuid' direction='in'/>"
    "    </method>"
    "    <method name='Cancel' />"
    "  </interface>"
    "</node>";

#define AGENT_PATH "/org/bluez/AutoPinAgent"

GDBusNodeInfo* BluetoothServer::introspection_data = nullptr;


GDBusInterfaceVTable BluetoothServer::interface_vtable = {
    // Initialize with your callbacks
    onMethodCall, // Method call callback
    onGetProperty, // Property get callback, or nullptr if not needed
    onSetProperty, // Property set callback, or nullptr if not needed
    nullptr, // No need for padding in user code
};


BluetoothServer::BluetoothServer() {
    // Initialize GDBus connection
    GError* error = nullptr;
    connection = g_bus_get_sync(G_BUS_TYPE_SYSTEM, nullptr, &error);
    if (!connection) {
        std::cerr << "Error connecting to D-Bus: " << error->message << std::endl;
        g_error_free(error);
        exit(EXIT_FAILURE);
    }
}

BluetoothServer::~BluetoothServer() {
    // Cleanup GDBus connection
    if (connection) {
        g_object_unref(connection);
    }
}

void BluetoothServer::onMethodCall(GDBusConnection* connection,
                                  const gchar* sender,
                                  const gchar* object_path,
                                  const gchar* interface_name,
                                  const gchar* method_name,
                                  GVariant* parameters,
                                  GDBusMethodInvocation* invocation,
                                  gpointer user_data) {

    BluetoothServer *server = static_cast<BluetoothServer*>(user_data);
    if (g_strcmp0(method_name, "RequestPinCode") == 0) {
        server->handleRequestPinCode(invocation, parameters);
    } else if (g_strcmp0(method_name, "RequestPasskey") == 0) {
        server->handleRequestPasskey(invocation, parameters);
    } else if (g_strcmp0(method_name, "RequestConfirmation") == 0) {
        server->handleRequestConfirmation(invocation, parameters);
    } else {
        // Method not recognized or not implemented
        g_dbus_method_invocation_return_error(invocation, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED, "Method '%s' not supported.", method_name);
    }
}

 GVariant *BluetoothServer::onGetProperty(GDBusConnection *connection, const gchar *sender, const gchar *object_path,
                                         const gchar *interface_name, const gchar *property_name, GError **error,
                                         gpointer user_data) {
    // Handle get property calls here
    return nullptr;
}

gboolean BluetoothServer::onSetProperty(GDBusConnection *connection, const gchar *sender, const gchar *object_path,
                                        const gchar *interface_name, const gchar *property_name, GVariant *value,
                                        GError **error, gpointer user_data) {
    // Handle set property calls here
    return FALSE;
}



int BluetoothServer::bluezAdapterSetProperty(const char *prop, GVariant *value)
{
	GVariant *result;
	GError *error = NULL;

    GVariant* params = g_variant_new("(ssv)", "org.bluez.Adapter1", prop, value);

	result = g_dbus_connection_call_sync(this->connection,
                                         "org.bluez",
                                         "/org/bluez/hci0",
                                         "org.freedesktop.DBus.Properties",
                                         "Set",
                                         params, // Correctly formatted parameters
                                         NULL, // Expecting no return value
                                         G_DBUS_CALL_FLAGS_NONE,
                                         -1, // No timeout
                                         NULL, // No cancellable
                                         &error);

    if (error) {
        std::cerr << "Error setting Bluetooth alias: " << error->message << std::endl;
        g_error_free(error); // Clean up error
        return 1;
    }

    // Only unref result if it's not NULL
    g_variant_unref(result);
    return 0;
}

int BluetoothServer::bluezAgentCallMethod(const gchar *method, GVariant *param)
{
    GVariant *result;
    GError *error = NULL;

    result = g_dbus_connection_call_sync(this->connection,
                "org.bluez",
                "/org/bluez",
                "org.bluez.AgentManager1",
                method,
                param,
                NULL,
                G_DBUS_CALL_FLAGS_NONE,
                -1,
                NULL,
                &error);

    if(error != NULL) {
        g_print("Register %s: %s\n", AGENT_PATH, error->message);
        return 1;
	}

    g_variant_unref(result);
    return 0;
}

void BluetoothServer::setAlias(const std::string& alias) {
    int err;

    err = this->bluezAdapterSetProperty("Alias", g_variant_new("s", alias.c_str()));
    if(err) {
		g_print("Not able to enable the adapter\n");
	} else {
        g_print("Bluetooth alias set successfully.\n");
    }
}


void BluetoothServer::powerOnAdapter() {
    int err;

    err = this->bluezAdapterSetProperty("Powered", g_variant_new("b", TRUE));
    if(err) {
		g_print("Not able to enable the adapter\n");
	} else {
        g_print("Adapter enabled successfully\n");
    }
}

void BluetoothServer::makeDiscoverable() {
    int err;

    err = this->bluezAdapterSetProperty("Discoverable", g_variant_new("b", TRUE));
    if(err) {
		g_print("Not able to Discoverable the adapter\n");
	} else {
        g_print("Adapter Discoverable successfully\n");
    }
}

void BluetoothServer::makePairable() {
    // Placeholder for making device pairable
    int err;

    err = this->bluezAdapterSetProperty("Pairable", g_variant_new("b", TRUE));
    if(err) {
		g_print("Not able to Pairable the adapter\n");
	} else {
        g_print("Adapter Pairable successfully\n");
    }
}

void BluetoothServer::registerAgent() {
    GError *error = NULL;
    int err;

    BluetoothServer::introspection_data = g_dbus_node_info_new_for_xml(agent_introspection_xml, NULL);
    if (introspection_data == nullptr) {
        std::cerr << "Failed to parse introspection XML" << std::endl;
        return;
    }

    //const GDBusInterfaceInfo *interface_info = g_dbus_node_info_lookup_interface(introspection_data, "org.bluez.Agent1");


    guint registration_id = g_dbus_connection_register_object(
        this->connection,
        AGENT_PATH, // The D-Bus object path of your agent
        BluetoothServer::introspection_data->interfaces[0],
        &BluetoothServer::interface_vtable, // Your GDBusInterfaceVTable with callbacks
        this, // User data passed to callbacks
        NULL, // User data free function
        &error // GError for error handling
    );

    if (error) {
        std::cerr << "Error registering object: " << error->message << std::endl;
        g_error_free(error);
        return;
    }

    // Agent registration
    err = this->bluezAgentCallMethod("RegisterAgent", g_variant_new("(os)", AGENT_PATH, "NoInputNoOutput"));
    if (err) {
        std::cerr << "Error registering agent: " << std::endl;
    }
    std::cout << "Agent registered succesfully";

    // Set the agent as the default agent
    err = this->bluezAgentCallMethod("RequestDefaultAgent", g_variant_new("(o)", AGENT_PATH));
    if (err) {
        std::cerr << "Error requesting default agent: " << std::endl;
        this->bluezAgentCallMethod("UnregisterAgent", g_variant_new("(o)", AGENT_PATH));
    }
    std::cout << "Agent registered and set as default successfully." << std::endl;
}

void BluetoothServer::handleRequestPinCode(GDBusMethodInvocation* invocation, GVariant* parameters) {
    // For NoInputNoOutput, we cannot show a pin code. Auto-confirm if needed or log.
    std::cout << "Pin code request received, automatically confirmed due to NoInputNoOutput capability." << std::endl;
    // Normally, you would not return a pin code here, but for the sake of completeness:
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(s)", ""));
}

void BluetoothServer::handleRequestPasskey(GDBusMethodInvocation* invocation, GVariant* parameters) {
    // For NoInputNoOutput, we cannot display or input a passkey. Auto-confirm if needed or log.
    std::cout << "Passkey request received, automatically confirmed due to NoInputNoOutput capability." << std::endl;
    // Normally, you would not return a passkey here, but for completeness:
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(u)", 0));
}

void BluetoothServer::handleRequestConfirmation(GDBusMethodInvocation* invocation, GVariant* parameters) {
    // Since there's no user interface to confirm the passkey, auto-confirm the request.
    std::cout << "Pairing confirmation request received, automatically confirmed due to NoInputNoOutput capability." << std::endl;
    g_dbus_method_invocation_return_value(invocation, NULL);
}

void BluetoothServer::startAdvertising() {
  std::cout << "Start Advertising." << std::endl;

    GVariantBuilder builder;
    g_variant_builder_init(&builder, G_VARIANT_TYPE("a{sv}"));

    // If you have specific advertisement data, add it here. Otherwise, it's an empty array.
    // Example: 
    g_variant_builder_add(&builder, "{sv}", "Key", g_variant_new_int32(1));

    GVariant* advData = g_variant_builder_end(&builder); // Finalize the builder

    GError* error = nullptr;
    g_dbus_connection_call_sync(
        connection,
        "org.bluez",
        "/org/bluez/hci0",
        "org.bluez.LEAdvertisingManager1",
        "RegisterAdvertisement",
        g_variant_new("(oa{sv})", "/com/example/ble/advertisement", advData),
        NULL, // No return type expected
        G_DBUS_CALL_FLAGS_NONE,
        -1, // No timeout
        NULL, // No cancellable
        &error);

    if (error) {
        std::cerr << "Failed to start advertising: " << error->message << std::endl;
        g_error_free(error);
    } else {
        std::cout << "Advertising started successfully." << std::endl;
    }
}



