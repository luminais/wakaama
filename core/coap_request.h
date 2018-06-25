#ifndef _COAP_REQUEST_H_
#define _COAP_REQUEST_H_

uint8_t coap_file_get(lwm2m_context_t * contextP,
                                   lwm2m_uri_t * uriP,
                                   void * fromSessionH,
                                   coap_packet_t * message,
                                   coap_packet_t * response);

#endif /* _COAP_REQUEST_H_ */

