

#ifndef __UDP_PERF_CLIENT_H_
#define __UDP_PERF_CLIENT_H_

#include "lwipopts.h"
#include "lwip/ip_addr.h"
#include "lwip/err.h"
#include "lwip/udp.h"
#include "lwip/inet.h"
#include "fmsh_print.h"
#include "platform.h"




/* used as indices into kLabel[] */
enum {
	KCONV_UNIT,
	KCONV_KILO,
	KCONV_MEGA,
	KCONV_GIGA,
};

/* labels for formats [KMG] */
const char kLabel_UDP[] =
{
	' ',
	'K',
	'M',
	'G'
};

/* used as type of print */
enum measure_t {
	BYTES,
	SPEED
};

/* Report Type */
enum report_type {
	/* The Intermediate report */
	INTER_REPORT,
	/* The client side test is done */
	UDP_DONE_CLIENT,
	/* Remote side aborted the test */
	UDP_ABORTED_REMOTE
};

struct interim_report {
	u64_t start_time;
	u64_t last_report_time;
	u32_t total_bytes;
};

struct perf_stats {
	u8_t client_id;
	u64_t start_time;
	u64_t total_bytes;
	u64_t cnt_datagrams;
	struct interim_report i_report;
};

/* seconds between periodic bandwidth reports */
#define INTERIM_REPORT_INTERVAL 5

/* Client port to connect */
#define UDP_CONN_PORT 5001

/* time in seconds to transmit packets */
#define UDP_TIME_INTERVAL 300

/* Server to connect with */
#define UDP_SERVER_IP_ADDRESS "192.168.1.11"

/* UDP buffer length in bytes */
#define UDP_SEND_BUFSIZE 1440

/* MAX UDP send retries */
#define MAX_SEND_RETRY 10

/* Number of parallel UDP clients */
#define NUM_OF_PARALLEL_CLIENTS 1

#endif /* __UDP_PERF_CLIENT_H_ */
