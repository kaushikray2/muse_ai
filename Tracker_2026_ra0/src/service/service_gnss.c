/**
 * \file service_gnss.c
 * \brief Buffers and parses GNSS NMEA sentences.
 * \details Updates fix, position, timing, HDOP, and per-constellation satellite data.
 * \author Kaushik Ray
 * \date Last Modified: 30 Sep 2026
 * \copyright (C) 2026 Siliconbrane Inc. All rights reserved.
 * \note Proprietary source. Unauthorized use or distribution is prohibited.
 */

#include "service_headers.h"

gps_data_t gps;
static char nmea_buf[GPS_SENTENCE_MAX_LENGTH];
static uint16_t nmea_idx = 0;

/* ---------------------------------------------------------
   Utility: Parse decimal string to fixed-point integer
   without using floats. Handles sign and decimal rounding.
--------------------------------------------------------- */
static int32_t parse_fixed_point(const char *s, int32_t scale)
{
    if (!s || *s == '\0') return 0;

    int i = 0;
    int32_t sign = 1;
    if (s[i] == '-') { sign = -1; i++; }
    else if (s[i] == '+') { i++; }

    int32_t int_val = 0;
    while (s[i] >= '0' && s[i] <= '9') {
        int_val = int_val * 10 + (s[i] - '0');
        i++;
    }

    int32_t frac_val = 0;
    if (s[i] == '.') {
        i++;
        int32_t frac_multiplier = scale;
        while (s[i] >= '0' && s[i] <= '9') {
            frac_multiplier /= 10;
            if (frac_multiplier > 0) {
                frac_val += (s[i] - '0') * frac_multiplier;
            } else if (frac_multiplier == 0) {
                // Round up if the next dropping digit is >= 5
                if (s[i] >= '5') {
                    frac_val += 1;
                }
                break;
            }
            i++;
        }
    }

    return sign * ((int_val * scale) + frac_val);
}

/* ---------------------------------------------------------
   Utility: Convert DDMM.MMMM ? microdegrees (int32)
   No floats, no pointers
--------------------------------------------------------- */
static int32_t parse_coord_microdeg(const char *s)
{
    if (!s || *s == '\0') return 0;

    int dot = -1;
    for (int i = 0; s[i]; i++) {
        if (s[i] == '.') { dot = i; break; }
    }

    /* need at least MM before the dot */
    if (dot < 2) return 0;

    int deg_len = dot - 2; /* degrees length (2 for lat, 3 for lon sometimes) */

    /* parse degrees */
    int32_t deg = 0;
    for (int i = 0; i < deg_len; i++) {
        char c = s[i];
        if (c < '0' || c > '9') return 0;
        deg = deg * 10 + (c - '0');
    }

    /* parse minutes: two digits before dot plus optional fractional digits after dot */
    int idx = deg_len;
    /* at least two minute digits required */
    if (!(s[idx] >= '0' && s[idx] <= '9')) return 0;
    int32_t min_int = 0;
    /* take up to two integer minute digits (should be exactly 2 as per NMEA) */
    for (int i = 0; i < 2 && s[idx] && s[idx] != '.'; i++, idx++) {
        if (s[idx] < '0' || s[idx] > '9') return 0;
        min_int = min_int * 10 + (s[idx] - '0');
    }

    /* if we stopped before dot and next char isn't '.' then input malformed */
    if (s[idx] != '.') {
        /* allow missing fractional part (e.g., "4530" -> no dot) */
        /* compute microdegrees using minutes integer only */
        int64_t add = ((int64_t)min_int * 1000000LL) / 60LL;
        return (int32_t)(deg * 1000000 + add);
    }

    /* parse fractional minute digits */
    idx++; /* skip '.' */
    int32_t frac = 0;
    int frac_len = 0;
    while (s[idx] >= '0' && s[idx] <= '9' && frac_len < 9) {
        frac = frac * 10 + (s[idx] - '0');
        frac_len++;
        idx++;
    }

    /* scale = 10^frac_len; compute minutes_scaled = min_int + frac/scale
       We'll compute add = (minutes_scaled * 1e6) / 60
       => add = ((min_int * scale + frac) * 1e6) / (60 * scale)
    */
    int64_t scale = 1;
    for (int i = 0; i < frac_len; i++) scale *= 10;

    int64_t minutes_scaled_num = (int64_t)min_int * scale + frac; /* numerator */
    int64_t denom = 60LL * scale;
    int64_t numer = minutes_scaled_num * 1000000LL;
    /* round to nearest microdegree */
    int64_t add = (numer + (denom / 2)) / denom;

    int32_t micro = (int32_t)(deg * 1000000 + add);
    return micro;
}

/* ---------------------------------------------------------
   Checksum Validation
--------------------------------------------------------- */
static bool nmea_checksum_ok(const char *s)
{
    if (s[0] != '$') return false;

    uint8_t sum = 0;
    int i = 1;

    while (s[i] != '*' && s[i] != 0) {
        sum ^= s[i];
        i++;
    }

    if (s[i] != '*') return true;

    char c1 = s[i+1];
    char c2 = s[i+2];
    if (!isxdigit(c1) || !isxdigit(c2)) return false;

    uint8_t expected =
        (uint8_t)((isdigit(c1)?c1-'0':toupper(c1)-'A'+10) << 4) |
        (uint8_t)(isdigit(c2)?c2-'0':toupper(c2)-'A'+10);

    return sum == expected;
}

/* ---------------------------------------------------------
   Split fields without pointers
--------------------------------------------------------- */
static int split_fields(char *buf, char *fields[], int max)
{
    int count = 1;
    fields[0] = buf;

    for (int i = 0; buf[i] != 0; i++) {
        if (buf[i] == ',' || buf[i] == '*') {
            buf[i] = 0;
            if (count >= max) break;
            fields[count++] = &buf[i + 1];
        }
    }
    return count;
}

/* ---------------------------------------------------------
   Parse GGA (Supports GNGGA, GPGGA, GAGGA, GBGGA)
--------------------------------------------------------- */
static void parse_gga(char *s)
{
    char *fields[10];
    int n = split_fields(s, fields, 10);
    if (n < 10) return;

    /* time */
    if (strlen(fields[1]) >= 6) {
        gps.hour   = (fields[1][0]-'0')*10 + (fields[1][1]-'0');
        gps.minute = (fields[1][2]-'0')*10 + (fields[1][3]-'0');
        gps.second = (fields[1][4]-'0')*10 + (fields[1][5]-'0');
    }

    /* latitude */
    gps.latitude_uDeg = parse_coord_microdeg(fields[2]);
    if (fields[3][0] == 'S') gps.latitude_uDeg = -gps.latitude_uDeg;

    /* longitude */
    gps.longitude_uDeg = parse_coord_microdeg(fields[4]);
    if (fields[5][0] == 'W') gps.longitude_uDeg = -gps.longitude_uDeg;

    /* fix */
    gps.fix_type = fields[6][0] - '0';
    gps.fix_valid = (gps.fix_type > 0);

    /* satellites (Accommodates high multi-GNSS tracking values like 25+) */
    gps.num_satellites = (uint8_t)atoi(fields[7]);

    /* HDOP ï¿½10 (Fixed: accurate handling of decimal fractions) */
    gps.hdop_x10 = (uint16_t)parse_fixed_point(fields[8], 10);

    /* altitude in cm (Fixed: accurate handling of decimal fractions) */
    gps.altitude_cm = parse_fixed_point(fields[9], 100);
}

/* ---------------------------------------------------------
   Parse RMC (Supports GNRMC, GPRMC, GARMC, GBRMC)
--------------------------------------------------------- */
static void parse_rmc(char *s)
{
    char *fields[10];
    int n = split_fields(s, fields, 10);
    if (n < 10) return;

    /* time */
    if (strlen(fields[1]) >= 6) {
        gps.hour   = (fields[1][0]-'0')*10 + (fields[1][1]-'0');
        gps.minute = (fields[1][2]-'0')*10 + (fields[1][3]-'0');
        gps.second = (fields[1][4]-'0')*10 + (fields[1][5]-'0');
    }

    gps.fix_valid = (fields[2][0] == 'A');

    gps.latitude_uDeg = parse_coord_microdeg(fields[3]);
    if (fields[4][0] == 'S') gps.latitude_uDeg = -gps.latitude_uDeg;

    gps.longitude_uDeg = parse_coord_microdeg(fields[5]);
    if (fields[6][0] == 'W') gps.longitude_uDeg = -gps.longitude_uDeg;

    /* speed in tenths (Fixed: parses floating values like 0.03 safely) */
    gps.speed_tenths = parse_fixed_point(fields[7], 10);

    /* course in tenths (Fixed: parses floating values like 127.44 safely) */
    gps.course_tenths = parse_fixed_point(fields[8], 10);

    /* date */
    if (strlen(fields[9]) >= 6) {
        gps.day   = (fields[9][0]-'0')*10 + (fields[9][1]-'0');
        gps.month = (fields[9][2]-'0')*10 + (fields[9][3]-'0');
        gps.year  = 2000 + (fields[9][4]-'0')*10 + (fields[9][5]-'0');
    }
}

static void parse_gsv(char *s)
{
    char *fields[4];
    int field_count = split_fields(s, fields, 4);

    if (field_count < 4 || atoi(fields[2]) != 1)
    {
        return;
    }

    uint8_t satellites_in_view = (uint8_t)atoi(fields[3]);

    if (fields[0][1] == 'G')
    {
        switch (fields[0][2])
        {
            case 'P':
                gps.gps_satellites_in_view = satellites_in_view;
                break;
            case 'L':
                gps.glonass_satellites_in_view = satellites_in_view;
                break;
            case 'A':
                gps.galileo_satellites_in_view = satellites_in_view;
                break;
            case 'B':
                gps.beidou_satellites_in_view = satellites_in_view;
                break;
            case 'Q':
                gps.qzss_satellites_in_view = satellites_in_view;
                break;
            case 'I':
                gps.navic_satellites_in_view = satellites_in_view;
                break;
            default:
                break;
        }
    }
    else if (fields[0][1] == 'B' && fields[0][2] == 'D')
    {
        gps.beidou_satellites_in_view = satellites_in_view;
    }
}

/* ---------------------------------------------------------
   Process sentence
--------------------------------------------------------- */
static void process_sentence(char *s)
{
    if (!nmea_checksum_ok(s)) return;
    if (strlen(s) < 6) return;

    // Checks positions [3][4][5] to support any NMEA talker ID (GP, GN, GA, GB, GL, etc.)
    if (s[3] == 'G' && s[4] == 'G' && s[5] == 'A')
        parse_gga(s);
    else if (s[3] == 'R' && s[4] == 'M' && s[5] == 'C')
        parse_rmc(s);
    else if (s[3] == 'G' && s[4] == 'S' && s[5] == 'V')
        parse_gsv(s);
}

/* ---------------------------------------------------------
   Init
--------------------------------------------------------- */
void service_gps_init(void)
{
    memset(&gps, 0, sizeof(gps));
    nmea_idx = 0;
}

/* ---------------------------------------------------------
   Tick
--------------------------------------------------------- */
void service_gps_tick(void)
{
    uint16_t bytes_processed = 0u;

    while ((bytes_processed < (SERVICE_UART_RING_BUFFER_SIZE - 1u)) &&
           (service_uart_available() > 0)) {
        char c = service_uart_read_byte();
        bytes_processed++;

        if (c == '$') {
            nmea_idx = 0;
            nmea_buf[nmea_idx++] = c;
        }
        else if (c == '\n' || c == '\r') {
            if (nmea_idx > 0) {
                nmea_buf[nmea_idx] = 0;
                process_sentence(nmea_buf);
                nmea_idx = 0;
            }
        }
        else if (nmea_idx < GPS_SENTENCE_MAX_LENGTH - 1) {
            nmea_buf[nmea_idx++] = c;
        }
    }
}

const gps_data_t* service_gps_get_data(void)
{
    return &gps;
}

bool service_gps_is_valid(void)
{
    return gps.fix_valid;
}

/* ---------------------------------------------------------
   State Machine Evaluator: Parse and Check HDOP
--------------------------------------------------------- */
bool service_gnss_parse_and_check_hdop(void)
{
    bool good_fix_achieved = false;

    /* Drain the ring buffer populated by the ISR */
    while (service_uart_available() > 0) {
        char c = service_uart_read_byte();

        if (c == '$') {
            nmea_idx = 0;
            nmea_buf[nmea_idx++] = c;
        }
        else if (c == '\n' || c == '\r') {
            if (nmea_idx > 0) {
                nmea_buf[nmea_idx] = 0;

                // Parse the completed string
                process_sentence(nmea_buf);
                nmea_idx = 0;

                /*
                 * STATE MACHINE EXIT CONDITION:
                 * We require a valid fix AND an HDOP of 2.0 or better (represented as <= 20).
                 * gps.hdop_x10 must be > 0 to prevent a false positive if HDOP parsed as 0.
                 */
                if (gps.fix_valid && gps.hdop_x10 > 0 && gps.hdop_x10 <= 20) {
                    good_fix_achieved = true;
                }
            }
        }
        else if (nmea_idx < GPS_SENTENCE_MAX_LENGTH - 1) {
            nmea_buf[nmea_idx++] = c;
        }
    }

    return good_fix_achieved;
}



