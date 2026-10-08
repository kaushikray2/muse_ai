/**
 * \file service_gnss.h
 * \brief GNSS data structure and service API.
 * \details Exposes parsed fix, position, HDOP, time, and satellite counts.
 * \author Kaushik Ray
 * \date Last Modified: 30 Sep 2026
 * \copyright (C) 2026 Siliconbrane Inc. All rights reserved.
 * \note Proprietary source. Unauthorized use or distribution is prohibited.
 */

#ifndef SERVICE_SERVICE_GNSS_H_
#define SERVICE_SERVICE_GNSS_H_

#include <stdbool.h>
#include <stdint.h>

#define GPS_SENTENCE_MAX_LENGTH 256

typedef struct {
    int32_t latitude_uDeg;      /* latitude in microdegrees */
    int32_t longitude_uDeg;     /* longitude in microdegrees */
    int32_t altitude_cm;        /* altitude in centimeters */

    int32_t speed_tenths;      /* speed in 0.1 knots */
    int32_t course_tenths;     /* course in 0.1 degrees */

    uint8_t num_satellites;
    uint8_t gps_satellites_in_view;
    uint8_t glonass_satellites_in_view;
    uint8_t galileo_satellites_in_view;
    uint8_t beidou_satellites_in_view;
    uint8_t qzss_satellites_in_view;
    uint8_t navic_satellites_in_view;
    uint16_t hdop_x10;          /* HDOP multiplied by 10; divide by 10 for HDOP */

    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;

    bool fix_valid;
    uint8_t fix_type;

} gps_data_t;

void service_gps_init(void);
void service_gps_tick(void);
const gps_data_t* service_gps_get_data(void);
bool service_gps_is_valid(void);

/* Debug/test helper - call from `main()` under a test macro */
void service_gps_test_parse(void);
bool service_gnss_parse_and_check_hdop(void);


#endif /* SERVICE_SERVICE_GNSS_H_ */
