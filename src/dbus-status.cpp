#include <dbus/dbus.h>
#include <fcitx/fcitx.h>
#include <fcitx/instance.h>
#include <fcitx/module.h>
#include <fcitx/module/dbus/fcitx-dbus.h>
#include <fcitx/module/dbus/dbusstuff.h>

#define FCITX_STATUS_DBUS_IFACE FCITX_DBUS_SERVICE ".Status"
#define FCITX_STATUS_DBUS_PATH "/Status"

static void *DBusStatusCreate(FcitxInstance *instance);
static void  DBusStatusDestroy(void *arg);

const char * fcitx_dbus_status_introspection_xml =
    "<!DOCTYPE node PUBLIC \"-//freedesktop//DTD D-BUS Object Introspection 1.0//EN\" "
    "\"http://www.freedesktop.org/standards/dbus/1.0/introspect.dtd\">"
    "<node name=\"/Status\">"
    "<interface name=\"" DBUS_INTERFACE_INTROSPECTABLE "\">"
    "<method name=\"Introspect\">"
    "<arg name=\"data\" direction=\"out\" type=\"s\"/>"
    "</method>"
    "</interface>"
    "<interface name=\"" FCITX_STATUS_DBUS_IFACE "\">"
    "<method name=\"Get\">"
    "<arg name=\"status_name\" direction=\"in\" type=\"s\"/>"
    "<arg name=\"short_description\" direction=\"out\" type=\"s\"/>"
    "<arg name=\"long_description\" direction=\"out\" type=\"s\"/>"
    "</method>"
    "</interface>"
    "</node>";

FCITX_DEFINE_PLUGIN(fcitx_dbus_status, module, FcitxModule) = {
    DBusStatusCreate,
    NULL,
    NULL,
    DBusStatusDestroy,
    NULL
};

typedef struct _FcitxDBustStatus {
    FcitxInstance *owner;
    DBusConnection *connection;
} FcitxDBusStatus;

static DBusHandlerResult FcitxDBusStatusEventHandler(DBusConnection *connection,
                                                     DBusMessage *msg,
                                                     void *user_data);

static DBusObjectPathVTable fcitxDBusStatusVTable = {
    NULL,
    &FcitxDBusStatusEventHandler,
    NULL,
    NULL,
    NULL,
    NULL
};

static void *DBusStatusCreate(FcitxInstance *instance)
{
    void *user_data = fcitx_utils_malloc0(sizeof(FcitxDBusStatus));
    FcitxDBusStatus *dbus_status = static_cast<FcitxDBusStatus *>(user_data);
    dbus_status->owner = instance;

    do {
        dbus_status->connection = FcitxDBusGetConnection(instance);
        if (!dbus_status->connection) {
            FcitxLog(ERROR, "DBus Not initialized");
            break;
        }

        dbus_bool_t succeeded
            = dbus_connection_register_object_path(dbus_status->connection,
                                                   FCITX_STATUS_DBUS_PATH,
                                                   &fcitxDBusStatusVTable,
                                                   user_data);
        if (!succeeded) {
            FcitxLog(ERROR, "Failed to register " FCITX_STATUS_DBUS_PATH);
            break;
        }

        return user_data;
    } while(0);

    free(user_data);
    return NULL;
}

static void DBusStatusDestroy(void *arg)
{
    FcitxDBusStatus *dbus_status = static_cast<FcitxDBusStatus *>(arg);
    if (dbus_status && dbus_status->connection) {
        dbus_connection_unregister_object_path(dbus_status->connection,
                                               FCITX_STATUS_DBUS_PATH);
    }
    free(dbus_status);
}

static DBusMessage *UnknownDBusMethod(DBusMessage *message)
{
    return dbus_message_new_error_printf(message,
                                         DBUS_ERROR_UNKNOWN_METHOD,
                                         "No such method with signature (%s)",
                                         dbus_message_get_signature(message));
}

static DBusHandlerResult FcitxDBusStatusEventHandler(DBusConnection *connection,
                                                     DBusMessage *message,
                                                     void *user_data)
{
    FcitxDBusStatus *dbus_status = static_cast<FcitxDBusStatus *>(user_data);
    DBusHandlerResult result = DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    DBusMessage *reply = NULL;

    if (dbus_message_is_method_call(message, DBUS_INTERFACE_INTROSPECTABLE, "Introspect")) {
        reply = dbus_message_new_method_return(message);
        dbus_message_append_args(reply,
                                 DBUS_TYPE_STRING, &fcitx_dbus_status_introspection_xml,
                                 DBUS_TYPE_INVALID);
    } else if (dbus_message_is_method_call(message, FCITX_STATUS_DBUS_IFACE, "Get")) {
        char *status_name = NULL;
        DBusError error;
        dbus_error_init(&error);
        dbus_bool_t succeeded
            = dbus_message_get_args(message, &error,
                                    DBUS_TYPE_STRING, &status_name,
                                    DBUS_TYPE_INVALID);
        if (succeeded) {
            FcitxUIComplexStatus *status
                = FcitxUIGetComplexStatusByName(dbus_status->owner,
                                                status_name);
            const char *shortDescription = "";
            const char *longDescription = "";
            if (status) {
                shortDescription = status->shortDescription;
                longDescription = status->longDescription;
            } else {
                // TODO
            }
            reply = dbus_message_new_method_return(message);
            dbus_message_append_args(reply,
                                     DBUS_TYPE_STRING, &shortDescription,
                                     DBUS_TYPE_STRING, &longDescription,
                                     DBUS_TYPE_INVALID);
        } else {
            // TODO: Use the error
            reply = UnknownDBusMethod(message);
        }
        dbus_error_free(&error);
    }

    if (reply) {
        dbus_connection_send(connection, reply, NULL);
        dbus_message_unref(reply);
        dbus_connection_flush(connection);
        result = DBUS_HANDLER_RESULT_HANDLED;
    }

    return result;
}
