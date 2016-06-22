#include <dbus/dbus.h>
#include <fcitx/fcitx.h>
#include <fcitx/instance.h>
#include <fcitx/module.h>
#include <fcitx/module/dbus/fcitx-dbus.h>

static void *DBusStatusCreate(FcitxInstance *instance);
static void  DBusStatusDestroy(void *arg);

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

static void *DBusStatusCreate(FcitxInstance *instance)
{
    void *user_data = fcitx_utils_malloc0(sizeof(FcitxDBusStatus));
    FcitxDBusStatus *dbus_status = static_cast<FcitxDBusStatus *>(user_data);

    dbus_status->owner = instance;
    dbus_status->connection = FcitxDBusGetConnection(instance);
    if (!dbus_status->connection) {
        FcitxLog(ERROR, "DBus Not initialized");
        goto ERROR;
    }

    return user_data;

ERROR:
    free(user_data);
    return NULL;
}

static void DBusStatusDestroy(void *arg)
{
    FcitxDBusStatus *dbus_status = static_cast<FcitxDBusStatus *>(arg);
    free(dbus_status);
}
