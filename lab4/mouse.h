#include <lcom/lcf.h>

int(mouse_subscribe_int(uint8_t *bit_no));

int(mouse_unsubscribe_int());

bool(check_output_buffer_full());

bool(check_input_buffer());

// takes the global variable bytes and builds a packet using its info
void build_packet(struct packet * p);

bool (mouse_stream_handler());
