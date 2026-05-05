
//change this this is a heavy bottleneck try to figure out how to use this properly without a bunch of loops
class ITCHParser {


static Order parse_add_order(const uint8_t* data) {
		Order order;

		
		//type of the message
		order.msg_type = data[0];

		// timestamp
		uint64_t timestamp = 0;
		for (int i = 0; i < 6; i++) {
			timestamp = (timestamp << 8) | data[5 + i];
		}
		order.timestamp_ns = timestamp;

		//referece number
		uint64_t order_ref = 0;
		for (int i = 0; i < 8; i++) {
			order_ref = (order_ref << 8) | data[11 + i];
		}
		order.order_ref = order_ref;


		//buy or sell
		order.side = data[19];
		uint32_t shares = 0;
		for (int j = 0; j < 4; j++) {
			shares = (shares << 8) | data[20 + j];
		}
		order.shares = shares;

		//stock symbol 
		char stock[8];
		for (int k = 0; k < 8; k++) {
			order.stock[k] = data[24 + k];
		}
		

		//price of unit
		uint32_t price = 0;
		for (int i = 0; i < 4; i++) {
			price = (price << 8) | data[32 + i];
		}
		order.price = price / 10000;
		return order;
	}
static Order parse_delete_order(const uint8_t* data) {
	Order order;

	// Message Type 'D' 
	order.msg_type = data[0];


	uint64_t timestamp = 0;
	for (int i = 0; i < 6; i++) {
		timestamp = (timestamp << 8) | data[5 + i];
	}
	order.timestamp_ns = timestamp;


	uint64_t order_ref = 0;
	for (int i = 0; i < 8; i++) {
		order_ref = (order_ref << 8) | data[11 + i];
	}
	order.order_ref = order_ref;

	return order;
}


};