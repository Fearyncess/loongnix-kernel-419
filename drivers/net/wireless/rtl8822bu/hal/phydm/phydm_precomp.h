/******************************************************************************
 *
 * Copyright(c) 2007 - 2017  Realtek Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * The full GNU General Public License is included in this distribution in the
 * file called LICENSE.
 *
 * Contact Information:
 * wlanfae <wlanfae@realtek.com>
 * Realtek Corporation, No. 2, Innovation Road II, Hsinchu Science Park,
 * Hsinchu 300, Taiwan.
 *
 * Larry Finger <Larry.Finger@lwfinger.net>
 *
 *****************************************************************************/

#ifndef	__ODM_PRECOMP_H__
#define __ODM_PRECOMP_H__

#include "phydm_types.h"
#include "phydm_features.h"
#include "halrf/halrf_features.h"

#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
#include "Precomp.h"		/* We need to include mp_precomp.h due to batch file setting. */
#else
#define		TEST_FALG___		1
#endif

/* 2 Config Flags and Structs - defined by each ODM type */

#if (DM_ODM_SUPPORT_TYPE == ODM_AP)
#include "../8192cd_cfg.h"
#include "../odm_inc.h"

#include "../8192cd.h"
#include "../8192cd_util.h"
#ifdef _BIG_ENDIAN_
#define	ODM_ENDIAN_TYPE				ODM_ENDIAN_BIG
#else
#define	ODM_ENDIAN_TYPE				ODM_ENDIAN_LITTLE
#endif

#include "../8192cd_headers.h"
#include "../8192cd_debug.h"

#elif (DM_ODM_SUPPORT_TYPE == ODM_CE)
#ifdef DM_ODM_CE_MAC80211
#include "../wifi.h"
#include "rtl_phydm.h"
#endif
#define __PACK
#define __WLAN_ATTRIB_PACK__
#elif (DM_ODM_SUPPORT_TYPE == ODM_WIN)
#include "mp_precomp.h"
#define	ODM_ENDIAN_TYPE				ODM_ENDIAN_LITTLE
#define __PACK
#define __WLAN_ATTRIB_PACK__
#endif

/* 2 OutSrc Header Files */

#include "phydm.h"
#include "phydm_hwconfig.h"
#include "phydm_phystatus.h"
#include "phydm_debug.h"
#include "phydm_regdefine11ac.h"
#include "phydm_regdefine11n.h"
#include "phydm_interface.h"
#include "phydm_reg.h"

#if (DM_ODM_SUPPORT_TYPE & ODM_CE) && !defined(DM_ODM_CE_MAC80211)

void phy_set_tx_power_limit(
	struct dm_struct	*dm,
	u8	*regulation,
	u8	*band,
	u8	*bandwidth,
	u8	*rate_section,
	u8	*rf_path,
	u8	*channel,
	u8	*power_limit
);

enum hal_status rtw_phydm_fw_iqk(
	struct dm_struct	*dm,
	u8 clear,
	u8 segment
);

enum hal_status rtw_phydm_cfg_phy_para(
	struct dm_struct	*dm,
	enum phydm_halmac_param config_type,
	u32 offset,
	u32 data,
	u32 mask,
	enum rf_path e_rf_path,
	u32 delay_time
);

#endif

#if (DM_ODM_SUPPORT_TYPE & ODM_AP)
#define RTL8703B_SUPPORT		0
#define RTL8188F_SUPPORT		0
#define RTL8723D_SUPPORT		0
#endif

/* JJ ADD 20161014 */
#if (DM_ODM_SUPPORT_TYPE & (ODM_CE|ODM_AP|ODM_IOT))
#define RTL8710B_SUPPORT		0
#endif

#if RTL8188E_SUPPORT == 1
#define RTL8188E_T_SUPPORT 1
#ifdef CONFIG_SFW_SUPPORTED
#define RTL8188E_S_SUPPORT 1
#else
#define RTL8188E_S_SUPPORT 0
#endif
#endif

#if (DM_ODM_SUPPORT_TYPE & (ODM_WIN | ODM_CE))
#define	RTL8195B_SUPPORT 0	/*Just for PHYDM API development*/
#define	RTL8198F_SUPPORT 0	/*Just for PHYDM API development*/
#endif

#if (RTL8188E_SUPPORT == 1)
#include "rtl8188e/hal8188erateadaptive.h" /* for  RA,Power training */
#include "rtl8188e/halhwimg8188e_mac.h"
#include "rtl8188e/halhwimg8188e_rf.h"
#include "rtl8188e/halhwimg8188e_bb.h"
#include "rtl8188e/phydm_regconfig8188e.h"
#include "rtl8188e/phydm_rtl8188e.h"
#include "rtl8188e/hal8188ereg.h"
#include "rtl8188e/version_rtl8188e.h"
#if (DM_ODM_SUPPORT_TYPE == ODM_CE)
#include "rtl8188e_hal.h"
#include "halrf/rtl8188e/halrf_8188e_ce.h"
#endif
#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
#include "halrf/rtl8188e/halrf_8188e_win.h"
#endif
#if (DM_ODM_SUPPORT_TYPE == ODM_AP)
#include "halrf/rtl8188e/halrf_8188e_ap.h"
#endif
#endif /* 88E END */

#if (RTL8192E_SUPPORT == 1)

#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
#include "halrf/rtl8192e/halrf_8192e_win.h" /*FOR_8192E_IQK*/
#elif (DM_ODM_SUPPORT_TYPE == ODM_AP)
#include "halrf/rtl8192e/halrf_8192e_ap.h" /*FOR_8192E_IQK*/
#elif (DM_ODM_SUPPORT_TYPE == ODM_CE)
#include "halrf/rtl8192e/halrf_8192e_ce.h" /*FOR_8192E_IQK*/
#endif

#include "rtl8192e/phydm_rtl8192e.h" /* FOR_8192E_IQK */
#include "rtl8192e/version_rtl8192e.h"
#if (DM_ODM_SUPPORT_TYPE != ODM_AP)
#include "rtl8192e/halhwimg8192e_bb.h"
#include "rtl8192e/halhwimg8192e_mac.h"
#include "rtl8192e/halhwimg8192e_rf.h"
#include "rtl8192e/phydm_regconfig8192e.h"
#include "rtl8192e/hal8192ereg.h"
#endif
#if (DM_ODM_SUPPORT_TYPE == ODM_CE)
#include "rtl8192e_hal.h"
#endif
#endif /* 92E END */

#if (RTL8812A_SUPPORT == 1)

#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
#include "halrf/rtl8812a/halrf_8812a_win.h"
#elif (DM_ODM_SUPPORT_TYPE == ODM_AP)
#include "halrf/rtl8812a/halrf_8812a_ap.h"
#elif (DM_ODM_SUPPORT_TYPE == ODM_CE)
#include "halrf/rtl8812a/halrf_8812a_ce.h"
#endif

/* #include "halrf/rtl8812a/halrf_8812a.h"  */ /* FOR_8812_IQK */
#if (DM_ODM_SUPPORT_TYPE != ODM_AP)
#include "rtl8812a/halhwimg8812a_bb.h"
#include "rtl8812a/halhwimg8812a_mac.h"
#include "rtl8812a/halhwimg8812a_rf.h"
#include "rtl8812a/phydm_regconfig8812a.h"
#include "rtl8812a/phydm_rtl8812a.h"
#endif

#if (DM_ODM_SUPPORT_TYPE == ODM_CE)
#include "rtl8812a_hal.h"
#endif
#include "rtl8812a/version_rtl8812a.h"

#endif /* 8812 END */

#if (RTL8814A_SUPPORT == 1)

#include "rtl8814a/halhwimg8814a_mac.h"
#include "rtl8814a/halhwimg8814a_rf.h"
#include "rtl8814a/halhwimg8814a_bb.h"
#include "rtl8814a/version_rtl8814a.h"
#include "rtl8814a/phydm_rtl8814a.h"
#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
#include "halrf/rtl8814a/halrf_8814a_win.h"
#elif (DM_ODM_SUPPORT_TYPE == ODM_CE)
#include "halrf/rtl8814a/halrf_8814a_ce.h"
#elif (DM_ODM_SUPPORT_TYPE == ODM_AP)
#include "halrf/rtl8814a/halrf_8814a_ap.h"
#endif
#include "rtl8814a/phydm_regconfig8814a.h"
#if (DM_ODM_SUPPORT_TYPE == ODM_CE)
#include "rtl8814a_hal.h"
#include "halrf/rtl8814a/halrf_iqk_8814a.h"
#endif
#endif /* 8814 END */

#if (RTL8881A_SUPPORT == 1)/* FOR_8881_IQK */
#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
#include "halrf/rtl8821a/halrf_iqk_8821a_win.h"
#elif (DM_ODM_SUPPORT_TYPE == ODM_CE)
#include "halrf/rtl8821a/halrf_iqk_8821a_ce.h"
#else
#include "halrf/rtl8821a/halrf_iqk_8821a_ap.h"
#endif
/* #include "rtl8881a/HalHWImg8881A_BB.h" */
/* #include "rtl8881a/HalHWImg8881A_MAC.h" */
/* #include "rtl8881a/HalHWImg8881A_RF.h" */
/* #include "rtl8881a/odm_RegConfig8881A.h" */
#endif

#if (RTL8723B_SUPPORT == 1)
#include "rtl8723b/halhwimg8723b_mac.h"
#include "rtl8723b/halhwimg8723b_rf.h"
#include "rtl8723b/halhwimg8723b_bb.h"
#include "rtl8723b/phydm_regconfig8723b.h"
#include "rtl8723b/phydm_rtl8723b.h"
#include "rtl8723b/hal8723breg.h"
#include "rtl8723b/version_rtl8723b.h"
#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
#include "halrf/rtl8723b/halrf_8723b_win.h"
#elif (DM_ODM_SUPPORT_TYPE == ODM_CE)
#include "halrf/rtl8723b/halrf_8723b_ce.h"
#include "rtl8723b/halhwimg8723b_mp.h"
#include "rtl8723b_hal.h"
#else
#include "halrf/rtl8723b/halrf_8723b_ap.h"
#endif
#endif

#if (RTL8821A_SUPPORT == 1)
#include "rtl8821a/halhwimg8821a_mac.h"
#include "rtl8821a/halhwimg8821a_rf.h"
#include "rtl8821a/halhwimg8821a_bb.h"
#include "rtl8821a/phydm_regconfig8821a.h"
#include "rtl8821a/phydm_rtl8821a.h"
#include "rtl8821a/version_rtl8821a.h"
#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
#include "halrf/rtl8821a/halrf_8821a_win.h"
#elif (DM_ODM_SUPPORT_TYPE == ODM_CE)
#include "halrf/rtl8821a/halrf_8821a_ce.h"
#include "halrf/rtl8821a/halrf_iqk_8821a_ce.h"/*for IQK*/
#include "halrf/rtl8812a/halrf_8812a_ce.h"/*for IQK,LCK,Power-tracking*/
#include "rtl8812a_hal.h"
#else
#endif
#endif

#if (DM_ODM_SUPPORT_TYPE == ODM_CE) && defined(DM_ODM_CE_MAC80211)
#include "../halmac/halmac_reg2.h"
#endif


#if (RTL8822B_SUPPORT == 1)
#include "rtl8822b/halhwimg8822b_mac.h"
#include "rtl8822b/halhwimg8822b_rf.h"
#include "rtl8822b/halhwimg8822b_bb.h"
#include "rtl8822b/phydm_regconfig8822b.h"
#include "halrf/rtl8822b/halrf_8822b.h"
#include "rtl8822b/phydm_rtl8822b.h"
#include "rtl8822b/phydm_hal_api8822b.h"
#include "rtl8822b/version_rtl8822b.h"

#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
#elif (DM_ODM_SUPPORT_TYPE == ODM_CE)
#ifdef DM_ODM_CE_MAC80211
#include "../halmac/halmac_reg_8822b.h"
#else
#include <hal_data.h>		/* struct HAL_DATA_TYPE */
#include <rtl8822b_hal.h>	/* RX_SMOOTH_FACTOR, reg definition and etc.*/
#endif
#elif (DM_ODM_SUPPORT_TYPE == ODM_AP)
#endif

#endif

#if (RTL8703B_SUPPORT == 1)
#include "rtl8703b/phydm_regconfig8703b.h"
#include "rtl8703b/halhwimg8703b_mac.h"
#include "rtl8703b/halhwimg8703b_rf.h"
#include "rtl8703b/halhwimg8703b_bb.h"
#include "halrf/rtl8703b/halrf_8703b.h"
#include "rtl8703b/version_rtl8703b.h"
#if (DM_ODM_SUPPORT_TYPE == ODM_CE)
#include "rtl8703b_hal.h"
#endif
#endif

#if (RTL8188F_SUPPORT == 1)
#include "rtl8188f/halhwimg8188f_mac.h"
#include "rtl8188f/halhwimg8188f_rf.h"
#include "rtl8188f/halhwimg8188f_bb.h"
#include "rtl8188f/hal8188freg.h"
#include "rtl8188f/phydm_rtl8188f.h"
#include "rtl8188f/phydm_regconfig8188f.h"
#include "halrf/rtl8188f/halrf_8188f.h" /* for IQK,LCK,Power-tracking */
#include "rtl8188f/version_rtl8188f.h"
#if (DM_ODM_SUPPORT_TYPE == ODM_CE)
#include "rtl8188f_hal.h"
#endif
#endif

#if (RTL8723D_SUPPORT == 1)
#if (DM_ODM_SUPPORT_TYPE != ODM_AP)

#include "rtl8723d/halhwimg8723d_bb.h"
#include "rtl8723d/halhwimg8723d_mac.h"
#include "rtl8723d/halhwimg8723d_rf.h"
#include "rtl8723d/phydm_regconfig8723d.h"
#include "rtl8723d/hal8723dreg.h"
#include "rtl8723d/phydm_rtl8723d.h"
#include "halrf/rtl8723d/halrf_8723d.h"
#include "rtl8723d/version_rtl8723d.h"
#endif
#if (DM_ODM_SUPPORT_TYPE == ODM_CE)
#include "rtl8723d_hal.h"
#endif
#endif /* 8723D End */

/* JJ ADD 20161014 */
#if (RTL8710B_SUPPORT == 1)
#if (DM_ODM_SUPPORT_TYPE != ODM_AP)

#include "rtl8710b/halhwimg8710b_bb.h"
#include "rtl8710b/halhwimg8710b_mac.h"
#include "rtl8710b/halhwimg8710b_rf.h"
#include "rtl8710b/phydm_regconfig8710b.h"
#include "rtl8710b/hal8710breg.h"
#include "rtl8710b/phydm_rtl8710b.h"
#include "halrf/rtl8710b/halrf_8710b.h"
#include "rtl8710b/version_rtl8710b.h"
#endif
#if (DM_ODM_SUPPORT_TYPE == ODM_CE)
#include "rtl8710b_hal.h"
#endif
#endif /* 8710B End */

#if (RTL8197F_SUPPORT == 1)
#include "rtl8197f/halhwimg8197f_mac.h"
#include "rtl8197f/halhwimg8197f_rf.h"
#include "rtl8197f/halhwimg8197f_bb.h"
#include "rtl8197f/phydm_hal_api8197f.h"
#include "rtl8197f/version_rtl8197f.h"
#include "rtl8197f/phydm_rtl8197f.h"
#include "rtl8197f/phydm_regconfig8197f.h"
#include "halrf/rtl8197f/halrf_8197f.h"
#include "halrf/rtl8197f/halrf_iqk_8197f.h"
#endif

#if (RTL8821C_SUPPORT == 1)
#include "rtl8821c/phydm_hal_api8821c.h"
#include "rtl8821c/halhwimg8821c_mac.h"
#include "rtl8821c/halhwimg8821c_rf.h"
#include "rtl8821c/halhwimg8821c_bb.h"
#include "rtl8821c/phydm_regconfig8821c.h"
#include "halrf/rtl8821c/halrf_8821c.h"
#include "rtl8821c/version_rtl8821c.h"
#if (DM_ODM_SUPPORT_TYPE == ODM_CE)
#include "rtl8821c_hal.h"
#endif
#endif

#if (RTL8195B_SUPPORT == 1)
#include "rtl8195b/phydm_hal_api8195b.h"
#endif

#if (RTL8198F_SUPPORT == 1)
#include "rtl8198f/phydm_hal_api8198F.h"
#endif

#endif /* __ODM_PRECOMP_H__ */
