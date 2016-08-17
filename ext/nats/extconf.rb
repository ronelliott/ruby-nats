require "mkmf"
have_library "nats", "natsLib_Release"
have_header "nats/nats.h"
create_makefile("nats")
