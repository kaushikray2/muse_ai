/**
 * \file appl_tracker_sm.h
 * \brief High-level application state machine for PawPrint Tracker.
 */
#ifndef APPL_TRACKER_SM_H
#define APPL_TRACKER_SM_H

#include "appl_headers.h"

/* Initializes or resets the state machine upon wakeup */
void appl_tracker_sm_reset(void);

/* The periodic tick function to be placed in the 5ms schedule table */
void appl_tracker_sm_tick(void);

#endif /* APPL_TRACKER_SM_H */
