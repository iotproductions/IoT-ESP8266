#ifndef __ALGORITHM_H__
#define __ALGORITHM_H__
#include "stdint.h"
#include "stdbool.h"

#define	NUMBER_PULSE_PER_LIT 123456
#define MQTT_MAX_WATER_INDEX_CUBIC		99999 // Maximum Water Index(m3)
#define MQTT_MAX_WATER_INDEX_LIT		999   // Maximum Water Index(lit)

#define    T_cycle_level_1   50
#define    T_cycle_level_2   70
#define    T_cycle_level_3   80
#define    T_cycle_level_4   100
#define    T_cycle_level_5   140
#define    T_cycle_level_6   180
#define    T_cycle_level_7   200
#define    T_cycle_level_8   300
#define    T_cycle_level_9   400
#define    T_cycle_level_10  600
#define    T_cycle_level_11  700
#define    T_cycle_level_12  800
#define    T_cycle_level_13  900
#define    T_cycle_level_14  1200
#define    T_cycle_level_15  3000

#define    K_level_1   20
#define    K_level_2   19
#define    K_level_3   18
#define    K_level_4   17
#define    K_level_5   16
#define    K_level_6   15
#define    K_level_7   14
#define    K_level_8   13
#define    K_level_9   12
#define    K_level_10  11
#define    K_level_11  8
#define    K_level_12  7
#define    K_level_13  4
#define    K_level_14  1

extern bool      sleep_status;              //0 sleep, 1 active
extern uint32_t  T_cycle_level[16];
extern uint32_t  K_level[15];
extern uint32_t  T_end;
extern uint32_t  T_cycle;
extern uint32_t  calculator_T_cycle;
extern uint32_t  percent_T_cycle;
extern uint32_t  constant_k;
extern uint32_t  fulseCounter;
extern uint32_t  water_value_m3;            // Water volume(m3)
extern uint16_t  water_value_lt;            // Water volume(Lit)
extern uint16_t  pin_percent;		    // Battery level(%)

extern void water_counter_algorithm(void);

#endif
