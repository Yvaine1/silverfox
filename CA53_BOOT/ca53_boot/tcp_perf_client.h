

#ifndef __TCP_PERF_CLIENT_H_
#define __TCP_PERF_CLIENT_H_

#include "lwipopts.h"
#include "lwip/ip_addr.h"
#include "lwip/err.h"
#include "lwip/tcp.h"
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
const char kLabel_TCP[] =
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
	TCP_DONE_CLIENT,
	/* Remote side aborted the test */
	TCP_ABORTED_REMOTE
};

struct interim_report {
	u64_t start_time;
	u64_t last_report_time;
	u32_t total_bytes;
	u32_t report_interval_time;
};

struct perf_stats {
	u8_t client_id;
	u64_t start_time;
	u64_t end_time;
	u64_t total_bytes;
	struct interim_report i_report;
};

/* seconds between periodic bandwidth reports */
#define INTERIM_REPORT_INTERVAL 5

/* Client port to connect */
#define TCP_CONN_PORT 5001

/* time in seconds to transmit packets */
#define TCP_TIME_INTERVAL 300

/* Server to connect with */
#define TCP_SERVER_IP_ADDRESS "192.168.1.1"

#define TCP_SEND_BUFSIZE (5*TCP_MSS)

#endif /* __TCP_PERF_CLIENT_H_ */
