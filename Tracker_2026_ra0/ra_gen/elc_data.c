/* generated ELC source file - do not edit */
#include "r_elc_api.h"
const elc_cfg_t g_elc_cfg = { .link[ELC_PERIPHERAL_AD] = ELC_EVENT_NONE, /* No allocation */
.link[ELC_PERIPHERAL_TAU00] = ELC_EVENT_NONE, /* No allocation */
.link[ELC_PERIPHERAL_TAU01] = ELC_EVENT_NONE, /* No allocation */
.link[ELC_PERIPHERAL_IOPORT1] = ELC_EVENT_NONE, /* No allocation */
.link[ELC_PERIPHERAL_IOPORT2] = ELC_EVENT_NONE, /* No allocation */
.link[ELC_PERIPHERAL_ITLC] = ELC_EVENT_NONE, /* No allocation */
};

#if BSP_TZ_SECURE_BUILD

        void R_BSP_ElcCfgSecurityInit(void);

        /* Configure ELC Security Attribution. */
        void R_BSP_ElcCfgSecurityInit(void)
        {
 #if (2U == BSP_FEATURE_ELC_VERSION)
            uint32_t elcsarbc = UINT32_MAX;


    #if BSP_SECONDARY_CORE_BUILD
            /* Write the settings to ELCSARn Registers. */
            R_ELC->ELCSARB &= elcsarbc;
    #else
            /* Write the settings to ELCSARn Registers. */
            R_ELC->ELCSARA = 0xFFFFFFFEU;
            R_ELC->ELCSARB = elcsarbc;
    #endif
 #else
            uint16_t elcsarbc[2] = {0xFFFFU, 0xFFFFU};

            /* Write the settings to ELCSARn Registers. */
            R_ELC->ELCSARA = 0xFFFEU;
            R_ELC->ELCSARB = elcsarbc[0];
            R_ELC->ELCSARC = elcsarbc[1];
 #endif
        }
#endif
