/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Copyright (C) 2016 Takuro Ashie <ashie@clear-code.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 */

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

const char *introspection_xml =
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

static DBusMessage *HandleIntrospection(FcitxDBusStatus *dbus_status,
                                        DBusConnection *connection,
                                        DBusMessage *message)
{
    DBusMessage *reply = dbus_message_new_method_return(message);
    if (!reply) {
        FcitxLog(ERROR, "Failed to allocate DBusMessage for reply!");
        return NULL;
    }
    dbus_message_append_args(reply,
                             DBUS_TYPE_STRING, &introspection_xml,
                             DBUS_TYPE_INVALID);
    return reply;
}

static DBusMessage *HandleGetMethod(FcitxDBusStatus *dbus_status,
                                    DBusConnection *connection,
                                    DBusMessage *message)
{
    DBusMessage *reply = NULL;
    char *status_name = NULL;
    DBusError error;
    dbus_error_init(&error);

    dbus_bool_t succeeded
        = dbus_message_get_args(message, &error,
                                DBUS_TYPE_STRING, &status_name,
                                DBUS_TYPE_INVALID);

    if (succeeded) {
        // TDOO: Should also search simple statuses
        FcitxUIComplexStatus *status
            = FcitxUIGetComplexStatusByName(dbus_status->owner,
                                            status_name);
        const char *shortDescription = "";
        const char *longDescription = "";
        if (status) {
            shortDescription = status->shortDescription;
            longDescription = status->longDescription;
        }
        reply = dbus_message_new_method_return(message);
        dbus_message_append_args(reply,
                                 DBUS_TYPE_STRING, &shortDescription,
                                 DBUS_TYPE_STRING, &longDescription,
                                 DBUS_TYPE_INVALID);
    } else {
        FcitxLog(ERROR, "Invalid arguments for Get method: %s",
                 error.message);
        reply = UnknownDBusMethod(message);
    }
    dbus_error_free(&error);

    return reply;
}

static struct MethodEntry {
    const char *interface;
    const char *name;
    DBusMessage *(*handler)(FcitxDBusStatus *dbus_status,
                            DBusConnection *connection,
                            DBusMessage *message);
} method_handlers[] = {
    {
        DBUS_INTERFACE_INTROSPECTABLE,
        "Introspect",
        HandleIntrospection,
    },
    {
        FCITX_STATUS_DBUS_IFACE,
        "Get",
        HandleGetMethod,
    },
};

static DBusHandlerResult FcitxDBusStatusEventHandler(DBusConnection *connection,
                                                     DBusMessage *message,
                                                     void *user_data)
{
    FcitxDBusStatus *dbus_status = static_cast<FcitxDBusStatus *>(user_data);
    DBusHandlerResult result = DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    DBusMessage *reply = NULL;

    for (int i = 0; i < sizeof(method_handlers) / sizeof(MethodEntry); i++) {
        MethodEntry &method = method_handlers[i];
        if (dbus_message_is_method_call(message, method.interface, method.name))
            reply = method.handler(dbus_status, connection, message);
    }

    if (reply) {
        dbus_connection_send(connection, reply, NULL);
        dbus_message_unref(reply);
        dbus_connection_flush(connection);
        result = DBUS_HANDLER_RESULT_HANDLED;
    }

    return result;
}
