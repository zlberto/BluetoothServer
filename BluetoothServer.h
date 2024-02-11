#ifndef BLUETOOTH_SERVER_H
#define BLUETOOTH_SERVER_H

#include <gio/gio.h>
#include <string>

class BluetoothServer {
public:
    BluetoothServer();
    ~BluetoothServer();

    void init();
    void makeDiscoverable();
    void makePairable();
    void setAlias(const std::string& alias);
    void registerAgent();
    void powerOnAdapter();
    void startAdvertising();

private:
    GDBusConnection* connection;
    guint registration_id; // Keep track of the registration ID for cleanup
    static GDBusNodeInfo *introspection_data;

    static GDBusInterfaceVTable interface_vtable;

    
    static void onMethodCall(GDBusConnection* connection,
                           const gchar* sender,
                           const gchar* object_path,
                           const gchar* interface_name,
                           const gchar* method_name,
                           GVariant* parameters,
                           GDBusMethodInvocation* invocation,
                           gpointer user_data);

    static GVariant* onGetProperty(GDBusConnection* connection,
                                const gchar* sender,
                                const gchar* object_path,
                                const gchar* interface_name,
                                const gchar* property_name,
                                GError** error,
                                gpointer user_data);

    static gboolean onSetProperty(GDBusConnection* connection,
                                const gchar* sender,
                                const gchar* object_path,
                                const gchar* interface_name,
                                const gchar* property_name,
                                GVariant* value,
                                GError** error,
                                gpointer user_data);

    
    void handleRequestPinCode(GDBusMethodInvocation* invocation, GVariant* parameters);
    void handleRequestPasskey(GDBusMethodInvocation* invocation, GVariant* parameters);
    void handleRequestConfirmation(GDBusMethodInvocation* invocation, GVariant* parameters);

    int bluezAdapterSetProperty(const char *prop, GVariant *value);
    int bluezAgentCallMethod(const gchar *method, GVariant *param);
};

#endif // BLUETOOTH_SERVER_H
