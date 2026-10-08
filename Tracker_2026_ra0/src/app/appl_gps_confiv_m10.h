#ifndef UBLOX_M10_H
#define UBLOX_M10_H

/* 
 * Puts the M10Q into Software Backup Mode for minimum power consumption.
 */
void UBLOX_EnterSoftwareBackup(void);

/* 
 * Wakes the M10Q up from Software Backup Mode.
 */
void UBLOX_WakeUp(void);

#endif // UBLOX_M10_H
