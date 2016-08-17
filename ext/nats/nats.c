#include <stdio.h>
#include <nats/nats.h>
#include <ruby.h>

typedef struct {
    natsConnection *conn;
} Connection;

static void ruby_nats_raise(natsStatus s) {
    rb_raise(rb_eException, natsStatus_GetText(s));
}

static void ruby_nats_on_error(
        natsConnection *nc,
        natsSubscription *sub,
        natsStatus err,
        void *closure) {
    ruby_nats_raise(err);
}

static void ruby_nats_destroy(void *self) {
    ((Connection*) self)->conn = NULL;
    free(self);
}

static VALUE ruby_nats_create(VALUE klass) {
    Connection *conn = malloc(sizeof(Connection));
    return Data_Wrap_Struct(klass, NULL, ruby_nats_destroy, conn);
}

static VALUE ruby_nats_close(VALUE self) {
    Connection *conn = NULL;
    Data_Get_Struct(self, Connection, conn);
    natsConnection_Destroy(conn->conn);
    return self;
}

static VALUE ruby_nats_connect(VALUE self, VALUE params) {
    Connection *conn = NULL;
    Data_Get_Struct(self, Connection, conn);

    natsOptions *opts = NULL;
    natsOptions_Create(&opts);
    natsOptions_SetErrorHandler(opts, ruby_nats_on_error, NULL);

    if (RTEST(params)) {
        VALUE val;

        val = rb_hash_aref(params, ID2SYM(rb_intern("allow_reconnect")));
        RTEST(val) && natsOptions_SetAllowReconnect(opts, RTEST(val));

        val = rb_hash_aref(params, ID2SYM(rb_intern("dont_randomize_servers")));
        RTEST(val) && natsOptions_SetNoRandomize(opts, RTEST(val));

        val = rb_hash_aref(params, ID2SYM(rb_intern("max_reconnect")));
        RTEST(val) && natsOptions_SetMaxReconnect(opts, NUM2INT(val));

        val = rb_hash_aref(params, ID2SYM(rb_intern("reconnect_time_wait")));
        RTEST(val) && natsOptions_SetReconnectWait(opts, NUM2INT(val));

        val = rb_hash_aref(params, ID2SYM(rb_intern("servers")));

        if (RTEST(val)) {
            int array_len = RARRAY_LEN(val);
            VALUE *val_arr = RARRAY_PTR(val);
            char **servers = malloc(sizeof(char*) * array_len);

            if (servers == NULL) {
                rb_raise(rb_eException, "Unable to allocate memory for server uri array");
            }

            for(int i = 0; i < array_len; i++) {
                char* uri = StringValuePtr(val_arr[i]);
                servers[i] = uri;
            }
            
            natsOptions_SetServers(opts, servers, array_len);
        }


        val = rb_hash_aref(params, ID2SYM(rb_intern("timeout")));
        RTEST(val) && natsOptions_SetTimeout(opts, NUM2INT(val));
    }

    natsStatus s = natsConnection_Connect(&(conn->conn), opts);

    if (s != NATS_OK) {
        ruby_nats_raise(s);
    }

    return self;
}

static VALUE ruby_nats_publish(VALUE self, VALUE subject, VALUE data) {
    Connection *conn = NULL;
    Data_Get_Struct(self, Connection, conn);
    natsStatus s = natsConnection_PublishString(
        conn->conn,
        StringValuePtr(subject),
        StringValuePtr(data));

    if (s == NATS_OK) {
        s = natsConnection_Flush(conn->conn);
    }

    if (s != NATS_OK) {
        ruby_nats_raise(s);
    }

    return self;
}

static VALUE ruby_nats_subscribe(VALUE self, VALUE subject, VALUE callback) {
    Connection *conn = NULL;
    Data_Get_Struct(self, Connection, conn);
    rb_raise(rb_eException, "Not Implemented!");
    return self;
}

static VALUE ruby_nats_subscribe_queuegroup(
        VALUE self,
        VALUE identifier,
        VALUE subject,
        VALUE callback) {
    Connection *conn = NULL;
    Data_Get_Struct(self, Connection, conn);
    rb_raise(rb_eException, "Not Implemented!");
    return self;
}

void Init_nats(void) {
    VALUE cNatsModule = rb_define_module("NATS");
    VALUE cNatsConnection = rb_define_class_under(cNatsModule, "Connection", rb_cObject);
    rb_define_alloc_func(cNatsConnection, ruby_nats_create);
    rb_define_method(cNatsConnection, "close", ruby_nats_close, 0);
    rb_define_method(cNatsConnection, "connect", ruby_nats_connect, 1);
    rb_define_method(cNatsConnection, "publish", ruby_nats_publish, 2);
    rb_define_method(cNatsConnection, "subscribe", ruby_nats_subscribe, 2);
    rb_define_method(cNatsConnection, "subscribe_qg", ruby_nats_subscribe_queuegroup, 3);
}
