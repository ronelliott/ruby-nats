require "nats"

params = {
	:allow_reconnect => true,
	:servers => ["nats://localhost:4222"]
}

conn = NATS::Connection.new
conn.connect params
conn.publish "foo", "bar"
conn.close
