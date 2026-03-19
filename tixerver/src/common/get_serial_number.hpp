#ifndef GET_SERIAL_NUMBER_HPP
#define GET_SERIAL_NUMBER_HPP

#include <axparameter.h>
#include <glib.h>
#include <string>

std::string getCameraSerialNumber() {
    GError* error = nullptr;
    AXParameter* ax_param = ax_parameter_new(APP_NAME_STR, &error);
    
    if (!ax_param) {
        if (error) g_error_free(error);
        return "";
    }

    gchar* raw_serial = nullptr;
    std::string serial_number = "";

    // Получаем MAC-адрес (серийный номер в формате Axis)
    if (ax_parameter_get(ax_param, "root.Network.I0.MACAddress", &raw_serial, &error)) {
        if (raw_serial) {
            serial_number = raw_serial;
            g_free(raw_serial); // Очистка памяти GLib
        }
    } else {
        if (error) g_error_free(error);
    }

    ax_parameter_free(ax_param);
    return serial_number;
}

#endif // GET_SERIAL_NUMBER_HPP
