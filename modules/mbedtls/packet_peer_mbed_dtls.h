/*************************************************************************/
/*  packet_peer_mbed_dtls.h                                              */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-present Godot Engine contributors (cf. AUTHORS.md).*/
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#ifndef PACKET_PEER_MBED_DTLS_H
#define PACKET_PEER_MBED_DTLS_H

#include "core/io/packet_peer_dtls.h"
#include "ssl_context_mbedtls.h"

#include <mbedtls/timing.h>

class PacketPeerMbedDTLS : public PacketPeerDTLS {
private:
	enum {
		PACKET_BUFFER_SIZE = 65536
	};

	uint8_t packet_buffer[PACKET_BUFFER_SIZE];

	Status status;
	String hostname;

	Ref<PacketPeerUDP> base;

	static PacketPeerDTLS *_create_func();

	static int bio_recv(void *ctx, unsigned char *buf, size_t len);
	static int bio_send(void *ctx, const unsigned char *buf, size_t len);
	void _cleanup();

protected:
	Ref<SSLContextMbedTLS> ssl_ctx;
	mbedtls_timing_delay_context timer;

	static void _bind_methods();

	Error _do_handshake();
	int _set_cookie();

public:
	virtual void poll();
	virtual Error accept_peer(Ref<PacketPeerUDP> p_base, Ref<CryptoKey> p_key, Ref<X509Certificate> p_cert = Ref<X509Certificate>(), Ref<X509Certificate> p_ca_chain = Ref<X509Certificate>(), Ref<CookieContextMbedTLS> p_cookies = Ref<CookieContextMbedTLS>());
	virtual Error connect_to_peer(Ref<PacketPeerUDP> p_base, bool p_validate_certs = true, const String &p_for_hostname = String(), Ref<X509Certificate> p_ca_certs = Ref<X509Certificate>());
	virtual Status get_status() const;

	virtual void disconnect_from_peer();

	virtual Error get_packet(const uint8_t **r_buffer, int &r_buffer_size);
	virtual Error put_packet(const uint8_t *p_buffer, int p_buffer_size);

	virtual int get_available_packet_count() const;
	virtual int get_max_packet_size() const;

	static void initialize_dtls();
	static void finalize_dtls();

	PacketPeerMbedDTLS();
	~PacketPeerMbedDTLS();
};

#endif // PACKET_PEER_MBED_DTLS_H
