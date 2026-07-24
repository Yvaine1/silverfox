#ifndef L1C_R50_H
#define L1C_R50_H

extern void l1c_nr_timecritical_msg_monitor(void *pvParameters);
extern void l1c_nr_timecritical_main_interrupt(void *pvParameters);
extern void l1c_nr_timecritical_main();
extern void l1c_r50_test_send_msg(void *pvParameters);

#endif