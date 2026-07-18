#include "py/runtime.h"
#include "py/mphal.h"
#include "driver/twai.h"

typedef struct {
    mp_obj_base_t base;
    twai_handle_t handle;
    int tx_pin;
    int rx_pin;
    uint32_t baudrate;
} mycan_obj_t;

const mp_obj_type_t mycan_type;

static mp_obj_t mycan_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args) {
    enum { ARG_tx, ARG_rx, ARG_baudrate };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_tx, MP_ARG_INT | MP_ARG_REQUIRED, {.u_int = 5} },
        { MP_QSTR_rx, MP_ARG_INT | MP_ARG_REQUIRED, {.u_int = 4} },
        { MP_QSTR_baudrate, MP_ARG_INT, {.u_int = 250000} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all_kw_array(n_args, n_kw, all_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    mycan_obj_t *self = mp_obj_malloc(mycan_obj_t, &mycan_type);
    self->tx_pin = args[ARG_tx].u_int;
    self->rx_pin = args[ARG_rx].u_int;
    self->baudrate = args[ARG_baudrate].u_int;

    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(self->tx_pin, self->rx_pin, TWAI_MODE_NORMAL);
    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_250KBITS();
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    esp_err_t err = twai_driver_install_v2(&g_config, &t_config, &f_config, &self->handle);
    if (err != ESP_OK) {
        mp_raise_msg_varg(&mp_type_OSError, "CAN init failed: 0x%x", err);
    }
    twai_start_v2(self->handle);
    return MP_OBJ_FROM_PTR(self);
}

static mp_obj_t mycan_send(mp_obj_t self_in, mp_obj_t data_in, mp_obj_t id_in) {
    mycan_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(data_in, &bufinfo, MP_BUFFER_READ);
    uint32_t id = mp_obj_get_int(id_in);

    twai_message_t tx_msg;
    tx_msg.identifier = id;
    tx_msg.data_length_code = bufinfo.len > 8 ? 8 : bufinfo.len;
    memcpy(tx_msg.data, bufinfo.buf, tx_msg.data_length_code);
    tx_msg.self = false;
    tx_msg.extd = false;
    tx_msg.rtr = false;

    esp_err_t err = twai_transmit_v2(self->handle, &tx_msg, pdMS_TO_TICKS(100));
    if (err != ESP_OK) mp_raise_msg_varg(&mp_type_OSError, "CAN send failed: 0x%x", err);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_3(mycan_send_obj, mycan_send);

static mp_obj_t mycan_recv(mp_obj_t self_in) {
    mycan_obj_t *self = MP_OBJ_TO_PTR(self_in);
    twai_message_t rx_msg;
    esp_err_t err = twai_receive_v2(self->handle, &rx_msg, 0);
    if (err == ESP_OK) {
        mp_obj_t tuple[2];
        tuple[0] = mp_obj_new_int(rx_msg.identifier);
        tuple[1] = mp_obj_new_bytes(rx_msg.data, rx_msg.data_length_code);
        return mp_obj_new_tuple(2, tuple);
    }
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(mycan_recv_obj, mycan_recv);

static mp_obj_t mycan_close(mp_obj_t self_in) {
    mycan_obj_t *self = MP_OBJ_TO_PTR(self_in);
    twai_stop_v2(self->handle);
    twai_driver_uninstall_v2(self->handle);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(mycan_close_obj, mycan_close);

static const mp_rom_map_elem_t mycan_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_send), MP_ROM_PTR(&mycan_send_obj) },
    { MP_ROM_QSTR(MP_QSTR_recv), MP_ROM_PTR(&mycan_recv_obj) },
    { MP_ROM_QSTR(MP_QSTR_close), MP_ROM_PTR(&mycan_close_obj) },
};
static MP_DEFINE_CONST_DICT(mycan_locals_dict, mycan_locals_dict_table);

MP_DEFINE_CONST_OBJ_TYPE(
    mycan_type,
    MP_QSTR_CAN,
    MP_TYPE_FLAG_NONE,
    make_new, mycan_make_new,
    locals_dict, &mycan_locals_dict
);

static const mp_rom_map_elem_t mycan_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_mycan) },
    { MP_ROM_QSTR(MP_QSTR_CAN), MP_ROM_PTR(&mycan_type) },
};
static MP_DEFINE_CONST_DICT(mycan_module_globals, mycan_module_globals_table);

const mp_obj_module_t mycan_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&mycan_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_mycan, mycan_user_cmodule);
