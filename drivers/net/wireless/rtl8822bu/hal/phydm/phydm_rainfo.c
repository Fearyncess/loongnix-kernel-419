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

/* ************************************************************
 * include files
 * ************************************************************ */
#include "mp_precomp.h"
#include "phydm_precomp.h"

void phydm_h2C_debug(
	void		*dm_void,
	u32		*const dm_value,
	u32		*_used,
	char		*output,
	u32		*_out_len
)
{
	struct dm_struct		*dm = (struct dm_struct *)dm_void;
	u8			h2c_parameter[H2C_MAX_LENGTH] = {0};
	u8			phydm_h2c_id = (u8)dm_value[0];
	u8			i;
	u32			used = *_used;
	u32			out_len = *_out_len;

	PDM_SNPF(out_len, used, output + used, out_len - used,
		 "Phydm Send H2C_ID (( 0x%x))\n", phydm_h2c_id);
	for (i = 0; i < H2C_MAX_LENGTH; i++) {
		h2c_parameter[i] = (u8)dm_value[i + 1];
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "H2C: Byte[%d] = ((0x%x))\n", i,
			 h2c_parameter[i]);
	}

	odm_fill_h2c_cmd(dm, phydm_h2c_id, H2C_MAX_LENGTH, h2c_parameter);

	*_used = used;
	*_out_len = out_len;
}

void phydm_fw_fix_rate(
	void		*dm_void,
	u8		en,
	u8		macid,
	u8		bw,
	u8		rate

)
{
	struct dm_struct		*dm = (struct dm_struct *)dm_void;
	u32	reg_u32_tmp;

	if (dm->support_ic_type & PHYDM_IC_8051_SERIES) {

		reg_u32_tmp = (bw << 24) | (rate << 16) | (macid << 8) | en;
		odm_set_bb_reg(dm, 0x4a0, MASKDWORD, reg_u32_tmp);

	} else {

		if (en == 1)
			reg_u32_tmp = (0x60 << 24) | (macid << 16) | (bw << 8) | rate;
		else
			reg_u32_tmp = 0x40000000;

		odm_set_bb_reg(dm, 0x450, MASKDWORD, reg_u32_tmp);
	}
	if (en == 1) {
		PHYDM_DBG(dm, ODM_COMP_API, "FW fix TX rate[id =%d], %dM, Rate(%d)=",
			  macid, (20 << bw), rate);
		phydm_print_rate(dm, rate, ODM_COMP_API);
	} else {
		PHYDM_DBG(dm, ODM_COMP_API, "Auto Rate\n");
	}
}

void phydm_ra_debug(
	void		*dm_void,
	char		input[][16],
	u32		*_used,
	char		*output,
	u32		*_out_len
)
{
	struct dm_struct		*dm = (struct dm_struct *)dm_void;
	struct ra_table			*ra_tab = &dm->dm_ra_table;
	u32 used = *_used;
	u32 out_len = *_out_len;
	char	help[] = "-h";
	u32	var1[5] = {0};
	u8	i = 0;
	u32	reg_u32_tmp;

	for (i = 0; i < 5; i++) {
		if (input[i + 1])
			PHYDM_SSCANF(input[i + 1], DCMD_DECIMAL, &var1[i]);
	}

	if ((strcmp(input[1], help) == 0)) {
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "{1} {0:-,1:+} {ofst}: set offset\n");
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "{1} {100}: show offset\n");
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "{2} {en} {macid} {bw} {rate}: fw fix rate\n");

	} else if (var1[0] == 1) { /*Adjust PCR offset*/

		if (var1[1] == 100) {
			PDM_SNPF(out_len, used, output + used,
				 out_len - used,
				 "[Get] RA_ofst=((%s%d))\n",
				 ((ra_tab->RA_threshold_offset == 0) ? " " : ((ra_tab->RA_offset_direction)
						 ? "+" : "-")),
				 ra_tab->RA_threshold_offset);

		} else if (var1[1] == 0) {
			ra_tab->RA_offset_direction = 0;
			ra_tab->RA_threshold_offset = (u8)var1[2];
			PDM_SNPF(out_len, used, output + used,
				 out_len - used,
				 "[Set] RA_ofst=((-%d))\n",
				 ra_tab->RA_threshold_offset);
		} else if (var1[1] == 1) {
			ra_tab->RA_offset_direction = 1;
			ra_tab->RA_threshold_offset = (u8)var1[2];
			PDM_SNPF(out_len, used, output + used,
				 out_len - used,
				 "[Set] RA_ofst=((+%d))\n",
				 ra_tab->RA_threshold_offset);
		}

	} else if (var1[0] == 2) { /*FW fix rate*/

		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "[FW fix TX Rate] {en, macid,bw,rate}={%d, %d, %d, 0x%x}",
			 var1[1], var1[2], var1[3], var1[4]);

		phydm_fw_fix_rate(dm, (u8)var1[1], (u8)var1[2], (u8)var1[3], (u8)var1[4]);

	} else {
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "[Set] Error\n");
		/**/
	}
	*_used = used;
	*_out_len = out_len;
}



void odm_c2h_ra_para_report_handler(
	void	*dm_void,
	u8	*cmd_buf,
	u8	cmd_len
)
{
	struct dm_struct		*dm = (struct dm_struct *)dm_void;
#if (defined(CONFIG_RA_DBG_CMD))
	struct ra_table			*ra_tab = &dm->dm_ra_table;
#endif

	u8	para_idx = cmd_buf[0]; /*Retry Penalty, NH, NL*/
#if (defined(CONFIG_RA_DBG_CMD))
	u8	rate_type_start = cmd_buf[1];
	u8	rate_type_length = cmd_len - 2;
#endif
	u8	i;


	PHYDM_DBG(dm, DBG_RA, "[ From FW C2H RA Para ]  cmd_buf[0]= (( %d ))\n",
		  cmd_buf[0]);

#if (defined(CONFIG_RA_DBG_CMD))
	if (para_idx == RADBG_RTY_PENALTY) {
		PHYDM_DBG(dm, DBG_RA, " |rate index|   |RTY Penality index|\n");

		for (i = 0 ; i < (rate_type_length) ; i++) {
			if (ra_tab->is_ra_dbg_init)
				ra_tab->RTY_P_default[rate_type_start + i] = cmd_buf[2 + i];

			ra_tab->RTY_P[rate_type_start + i] = cmd_buf[2 + i];
			PHYDM_DBG(dm, DBG_RA, "%8d  %15d\n", (rate_type_start + i),
				  ra_tab->RTY_P[rate_type_start + i]);
		}

	} else	if (para_idx == RADBG_N_HIGH) {
		/**/
		PHYDM_DBG(dm, DBG_RA, " |rate index|    |N-High|\n");


	} else if (para_idx == RADBG_N_LOW) {
		PHYDM_DBG(dm, DBG_RA, " |rate index|   |N-Low|\n");
		/**/
	} else if (para_idx == RADBG_RATE_UP_RTY_RATIO) {
		PHYDM_DBG(dm, DBG_RA, " |rate index|   |rate Up RTY Ratio|\n");

		for (i = 0; i < (rate_type_length); i++) {
			if (ra_tab->is_ra_dbg_init)
				ra_tab->RATE_UP_RTY_RATIO_default[rate_type_start + i] = cmd_buf[2 + i];

			ra_tab->RATE_UP_RTY_RATIO[rate_type_start + i] = cmd_buf[2 + i];
			PHYDM_DBG(dm, DBG_RA, "%8d  %15d\n", (rate_type_start + i),
				  ra_tab->RATE_UP_RTY_RATIO[rate_type_start + i]);
		}
	} else	 if (para_idx == RADBG_RATE_DOWN_RTY_RATIO) {
		PHYDM_DBG(dm, DBG_RA, " |rate index|   |rate Down RTY Ratio|\n");

		for (i = 0; i < (rate_type_length); i++) {
			if (ra_tab->is_ra_dbg_init)
				ra_tab->RATE_DOWN_RTY_RATIO_default[rate_type_start + i] = cmd_buf[2 + i];

			ra_tab->RATE_DOWN_RTY_RATIO[rate_type_start + i] = cmd_buf[2 + i];
			PHYDM_DBG(dm, DBG_RA, "%8d  %15d\n", (rate_type_start + i),
				  ra_tab->RATE_DOWN_RTY_RATIO[rate_type_start + i]);
		}
	} else
#endif
		if (para_idx == RADBG_DEBUG_MONITOR1) {
			PHYDM_DBG(dm, DBG_FW_TRACE, "-------------------------------\n");
			if (dm->support_ic_type & PHYDM_IC_3081_SERIES) {
				PHYDM_DBG(dm, DBG_FW_TRACE, "%5s  %d\n", "RSSI =", cmd_buf[1]);
				PHYDM_DBG(dm, DBG_FW_TRACE, "%5s  0x%x\n", "rate =", cmd_buf[2] & 0x7f);
				PHYDM_DBG(dm, DBG_FW_TRACE, "%5s  %d\n", "SGI =",
					  (cmd_buf[2] & 0x80) >> 7);
				PHYDM_DBG(dm, DBG_FW_TRACE, "%5s  %d\n", "BW =", cmd_buf[3]);
				PHYDM_DBG(dm, DBG_FW_TRACE, "%5s  %d\n", "BW_max =", cmd_buf[4]);
				PHYDM_DBG(dm, DBG_FW_TRACE, "%5s  0x%x\n", "multi_rate0 =", cmd_buf[5]);
				PHYDM_DBG(dm, DBG_FW_TRACE, "%5s  0x%x\n", "multi_rate1 =", cmd_buf[6]);
				PHYDM_DBG(dm, DBG_FW_TRACE, "%5s  %d\n", "DISRA =",	cmd_buf[7]);
				PHYDM_DBG(dm, DBG_FW_TRACE, "%5s  %d\n", "VHT_EN =", cmd_buf[8]);
				PHYDM_DBG(dm, DBG_FW_TRACE, "%5s  %d\n", "SGI_support =",	cmd_buf[9]);
				PHYDM_DBG(dm, DBG_FW_TRACE, "%5s  %d\n", "try_ness =", cmd_buf[10]);
				PHYDM_DBG(dm, DBG_FW_TRACE, "%5s  0x%x\n", "pre_rate =", cmd_buf[11]);
			} else {
				PHYDM_DBG(dm, DBG_FW_TRACE, "%5s  %d\n", "RSSI =", cmd_buf[1]);
				PHYDM_DBG(dm, DBG_FW_TRACE, "%5s  %x\n", "BW =", cmd_buf[2]);
				PHYDM_DBG(dm, DBG_FW_TRACE, "%5s  %d\n", "DISRA =", cmd_buf[3]);
				PHYDM_DBG(dm, DBG_FW_TRACE, "%5s  %d\n", "VHT_EN =", cmd_buf[4]);
				PHYDM_DBG(dm, DBG_FW_TRACE, "%5s  %d\n", "Hightest rate =", cmd_buf[5]);
				PHYDM_DBG(dm, DBG_FW_TRACE, "%5s  0x%x\n", "Lowest rate =", cmd_buf[6]);
				PHYDM_DBG(dm, DBG_FW_TRACE, "%5s  0x%x\n", "SGI_support =", cmd_buf[7]);
				PHYDM_DBG(dm, DBG_FW_TRACE, "%5s  %d\n", "Rate_ID =",	cmd_buf[8]);
			}
			PHYDM_DBG(dm, DBG_FW_TRACE, "-------------------------------\n");
		} else	 if (para_idx == RADBG_DEBUG_MONITOR2) {
			PHYDM_DBG(dm, DBG_FW_TRACE, "-------------------------------\n");
			if (dm->support_ic_type & PHYDM_IC_3081_SERIES) {
				PHYDM_DBG(dm, DBG_FW_TRACE, "%5s  %d\n", "rate_id =", cmd_buf[1]);
				PHYDM_DBG(dm, DBG_FW_TRACE, "%5s  0x%x\n", "highest_rate =", cmd_buf[2]);
				PHYDM_DBG(dm, DBG_FW_TRACE, "%5s  0x%x\n", "lowest_rate =", cmd_buf[3]);

				for (i = 4; i <= 11; i++)
					PHYDM_DBG(dm, DBG_FW_TRACE, "RAMASK =  0x%x\n", cmd_buf[i]);
			} else {
				PHYDM_DBG(dm, DBG_FW_TRACE, "%5s  %x%x  %x%x  %x%x  %x%x\n", "RA Mask:",
					  cmd_buf[8], cmd_buf[7], cmd_buf[6], cmd_buf[5], cmd_buf[4], cmd_buf[3],
					  cmd_buf[2], cmd_buf[1]);
			}
			PHYDM_DBG(dm, DBG_FW_TRACE, "-------------------------------\n");
		} else	 if (para_idx == RADBG_DEBUG_MONITOR3) {
			for (i = 0; i < (cmd_len - 1); i++)
				PHYDM_DBG(dm, DBG_FW_TRACE, "content[%d] = %d\n", i, cmd_buf[1 + i]);
		} else	 if (para_idx == RADBG_DEBUG_MONITOR4)
			PHYDM_DBG(dm, DBG_FW_TRACE, "%5s  {%d.%d}\n", "RA version =", cmd_buf[1],
				  cmd_buf[2]);
		else if (para_idx == RADBG_DEBUG_MONITOR5) {
			PHYDM_DBG(dm, DBG_FW_TRACE, "%5s  0x%x\n", "Current rate =", cmd_buf[1]);
			PHYDM_DBG(dm, DBG_FW_TRACE, "%5s  %d\n", "Retry ratio =", cmd_buf[2]);
			PHYDM_DBG(dm, DBG_FW_TRACE, "%5s  %d\n", "rate down ratio =", cmd_buf[3]);
			PHYDM_DBG(dm, DBG_FW_TRACE, "%5s  0x%x\n", "highest rate =", cmd_buf[4]);
			PHYDM_DBG(dm, DBG_FW_TRACE, "%5s  {0x%x 0x%x}\n", "Muti-try =", cmd_buf[5],
				  cmd_buf[6]);
			PHYDM_DBG(dm, DBG_FW_TRACE, "%5s  0x%x%x%x%x%x\n", "RA mask =",
				  cmd_buf[11], cmd_buf[10], cmd_buf[9], cmd_buf[8], cmd_buf[7]);
		}
}

void phydm_ra_dynamic_retry_count(
	void	*dm_void
)
{
	struct dm_struct		*dm = (struct dm_struct *)dm_void;

	if (!(dm->support_ability & ODM_BB_DYNAMIC_ARFR))
		return;

	/*PHYDM_DBG(dm, DBG_RA, "dm->pre_b_noisy = %d\n", dm->pre_b_noisy );*/
	if (dm->pre_b_noisy != dm->noisy_decision) {
		if (dm->noisy_decision) {
			PHYDM_DBG(dm, DBG_DYN_ARFR, "Noisy Env. RA fallback\n");
			odm_set_mac_reg(dm, 0x430, MASKDWORD, 0x0);
			odm_set_mac_reg(dm, 0x434, MASKDWORD, 0x04030201);
		} else {
			PHYDM_DBG(dm, DBG_DYN_ARFR, "Clean Env. RA fallback\n");
			odm_set_mac_reg(dm, 0x430, MASKDWORD, 0x01000000);
			odm_set_mac_reg(dm, 0x434, MASKDWORD, 0x06050402);
		}
		dm->pre_b_noisy = dm->noisy_decision;
	}
}

void phydm_print_rate(
	void	*dm_void,
	u8	rate,
	u32	dbg_component
)
{
	struct dm_struct	*dm = (struct dm_struct *)dm_void;
	u8		legacy_table[12] = {1, 2, 5, 11, 6, 9, 12, 18, 24, 36, 48, 54};
	u8		rate_idx = rate & 0x7f; /*remove bit7 SGI*/
	u8		vht_en = (rate_idx >= ODM_RATEVHTSS1MCS0) ? 1 : 0;
	u8		b_sgi = (rate & 0x80) >> 7;

	PHYDM_DBG_F(dm, dbg_component, "( %s%s%s%s%d%s%s)\n",
		    ((rate_idx >= ODM_RATEVHTSS1MCS0)
		     && (rate_idx <= ODM_RATEVHTSS1MCS9)) ? "VHT 1ss  " : "",
		    ((rate_idx >= ODM_RATEVHTSS2MCS0)
		     && (rate_idx <= ODM_RATEVHTSS2MCS9)) ? "VHT 2ss " : "",
		    ((rate_idx >= ODM_RATEVHTSS3MCS0)
		     && (rate_idx <= ODM_RATEVHTSS3MCS9)) ? "VHT 3ss " : "",
		    (rate_idx >= ODM_RATEMCS0) ? "MCS " : "",
		    (vht_en) ? ((rate_idx - ODM_RATEVHTSS1MCS0) % 10) : ((
					    rate_idx >= ODM_RATEMCS0) ? (rate_idx - ODM_RATEMCS0) : ((
							    rate_idx <= ODM_RATE54M) ? legacy_table[rate_idx] : 0)),
		    (b_sgi) ? "-S" : "  ",
		    (rate_idx >= ODM_RATEMCS0) ? "" : "M");
}

void phydm_c2h_ra_report_handler(
	void	*dm_void,
	u8   *cmd_buf,
	u8   cmd_len
)
{
	struct dm_struct	*dm = (struct dm_struct *)dm_void;
	struct ra_table		*ra_tab = &dm->dm_ra_table;
	u8	macid = cmd_buf[1];
	u8	rate = cmd_buf[0];
	u8	curr_ra_ratio = 0xff;
	u8	curr_bw = 0xff;
	u8	rate_idx = rate & 0x7f; /*remove bit7 SGI*/
	u8	rate_order;
	struct cmn_sta_info			*sta = dm->phydm_sta_info[macid];

	if (cmd_len >= 7) {
		curr_ra_ratio = cmd_buf[5];
		curr_bw = cmd_buf[6];
		PHYDM_DBG(dm, DBG_RA, "RA retry ratio: [%d]:", curr_ra_ratio);
		/**/
	}

	if (cmd_buf[3] != 0) {
		if (cmd_buf[3] == 0xff) {
			PHYDM_DBG(dm, DBG_RA, "FW Level: Fix rate[%d]:", macid);
			/**/
		} else if (cmd_buf[3] == 1) {
			PHYDM_DBG(dm, DBG_RA, "Try Success[%d]:", macid);
			/**/
		} else if (cmd_buf[3] == 2) {
			PHYDM_DBG(dm, DBG_RA, "Try Fail & Try Again[%d]:", macid);
			/**/
		} else if (cmd_buf[3] == 3) {
			PHYDM_DBG(dm, DBG_RA, "rate Back[%d]:", macid);
			/**/
		} else if (cmd_buf[3] == 4) {
			PHYDM_DBG(dm, DBG_RA, "start rate by RSSI[%d]:", macid);
			/**/
		} else if (cmd_buf[3] == 5) {
			PHYDM_DBG(dm, DBG_RA, "Try rate[%d]:", macid);
			/**/
		}
	}

	PHYDM_DBG(dm, DBG_RA, "Tx rate Update[%d]:", macid);
	phydm_print_rate(dm, rate, DBG_RA);

	if (macid >= 128) {
		u8 gid_index = macid - 128;
		ra_tab->mu1_rate[gid_index] = rate;
	}

	/*ra_tab->link_tx_rate[macid] = rate;*/

	if (is_sta_active(sta)) {
		sta->ra_info.curr_tx_rate = rate;
		sta->ra_info.curr_tx_bw = (enum channel_width)curr_bw;
		sta->ra_info.curr_retry_ratio = curr_ra_ratio;

		/*if (sta->ra_info.curr_tx_bw <  sta->ra_info.ra_bw_mode)*/
	}

	/*trigger power training*/
#if (DM_ODM_SUPPORT_TYPE & (ODM_WIN | ODM_CE))

	rate_order = phydm_rate_order_compute(dm, rate_idx);

	if ((dm->is_one_entry_only) ||
	    ((rate_order > ra_tab->highest_client_tx_order)
	     && (ra_tab->power_tracking_flag == 1))
	   ) {
		halrf_update_pwr_track(dm, rate_idx);
		ra_tab->power_tracking_flag = 0;
	}

#endif

	/*trigger dynamic rate ID*/
	/*#if (defined(CONFIG_RA_DYNAMIC_RATE_ID))*/	/*dino will refine here later*/
#if 0
	if (dm->support_ic_type & (ODM_RTL8812 | ODM_RTL8192E))
		phydm_update_rate_id(dm, rate, macid);
#endif

}

void odm_ra_post_action_on_assoc(
	void	*dm_void
)
{
#if 0
	struct dm_struct	*dm = (struct dm_struct *)dm_void;

	dm->h2c_rarpt_connect = 1;
	phydm_rssi_monitor_check(dm);
	dm->h2c_rarpt_connect = 0;
#endif
}

void phydm_modify_RA_PCR_threshold(
	void		*dm_void,
	u8		RA_offset_direction,
	u8		RA_threshold_offset

)
{
	struct dm_struct		*dm = (struct dm_struct *)dm_void;
	struct ra_table			*ra_tab = &dm->dm_ra_table;

	ra_tab->RA_offset_direction = RA_offset_direction;
	ra_tab->RA_threshold_offset = RA_threshold_offset;
	PHYDM_DBG(dm, DBG_RA_MASK, "Set RA_threshold_offset = (( %s%d ))\n",
		  ((RA_threshold_offset == 0) ? " " : ((RA_offset_direction) ? "+" : "-")),
		  RA_threshold_offset);
}

#if 0 /*(DM_ODM_SUPPORT_TYPE == ODM_WIN)*/

void odm_refresh_rate_adaptive_mask_mp(
	void		*dm_void
)
{
	struct dm_struct				*dm = (struct dm_struct *)dm_void;
	struct ra_table					*ra_tab = &dm->dm_ra_table;
	void				*adapter	 =  dm->adapter;
	void				*target_adapter = NULL;
	HAL_DATA_TYPE			*hal_data = GET_HAL_DATA(adapter);
	PMGNT_INFO				mgnt_info = GetDefaultMgntInfo(adapter);
	void			*loop_adapter = GetDefaultAdapter(adapter);
	PMGNT_INFO					p_loop_mgnt_info = &loop_adapter->MgntInfo;
	HAL_DATA_TYPE				*loop_hal_data = GET_HAL_DATA(loop_adapter);

	u32		i;
	struct sta_info *entry;
	u8		ratr_state_new;

	PHYDM_DBG(dm, DBG_RA_MASK, "%s ======>\n", __func__);

	if (adapter->bDriverStopped) {
		PHYDM_DBG(dm, DBG_RA_MASK, "driver is going to unload\n");
		return;
	}

	if (!hal_data->bUseRAMask) {
		PHYDM_DBG(dm, DBG_RA_MASK, "driver does not control rate adaptive mask\n");
		return;
	}

	/* if default port is connected, update RA table for default port (infrastructure mode only) */
	/* Need to consider other ports for P2P cases*/

	while (loop_adapter) {
		p_loop_mgnt_info = &loop_adapter->MgntInfo;
		loop_hal_data = GET_HAL_DATA(loop_adapter);

		if (p_loop_mgnt_info->mAssoc && (!ACTING_AS_AP(loop_adapter))) {
			odm_refresh_ldpc_rts_mp(loop_adapter, dm, p_loop_mgnt_info->mMacId,
						p_loop_mgnt_info->IOTPeer, loop_hal_data->UndecoratedSmoothedPWDB);
			/*PHYDM_DBG(dm, DBG_RA_MASK, "Infrasture mode\n");*/

			ratr_state_new = phydm_rssi_lv_dec(dm,
							   loop_hal_data->UndecoratedSmoothedPWDB, p_loop_mgnt_info->Ratr_State);

			if ((p_loop_mgnt_info->Ratr_State != ratr_state_new)
			    || (ra_tab->up_ramask_cnt >= FORCED_UPDATE_RAMASK_PERIOD)) {
				ra_tab->up_ramask_cnt = 0;
				PHYDM_PRINT_ADDR(dm, DBG_RA_MASK, ("Target AP addr :"),
						 p_loop_mgnt_info->Bssid);
				PHYDM_DBG(dm, DBG_RA_MASK,
					  "Update RA Level: ((%x)) -> ((%x)),  RSSI = ((%d))\n\n",
					  mgnt_info->Ratr_State, ratr_state_new,
					  loop_hal_data->UndecoratedSmoothedPWDB);

				p_loop_mgnt_info->Ratr_State = ratr_state_new;
				adapter->HalFunc.UpdateHalRAMaskHandler(loop_adapter,
									p_loop_mgnt_info->mMacId, NULL);
			} else {
				PHYDM_DBG(dm, DBG_RA_MASK, "Stay in RA level  = (( %d ))\n\n",
					  ratr_state_new);
				/**/
			}
		}

		loop_adapter = GetNextExtAdapter(loop_adapter);
	}

	/*  */
	/* The following part configure AP/VWifi/IBSS rate adaptive mask. */
	/*  */

	if (mgnt_info->mIbss)	/* Target: AP/IBSS peer. */
		target_adapter = GetDefaultAdapter(adapter);
	else
		target_adapter = GetFirstAPAdapter(adapter);

	/* if extension port (softap) is started, updaet RA table for more than one clients associate */
	if (target_adapter != NULL) {
		for (i = 0; i < ODM_ASSOCIATE_ENTRY_NUM; i++) {
			entry = AsocEntry_EnumStation(target_adapter, i);

			if (is_sta_active((&GET_STA_INFO(entry)))) {
				odm_refresh_ldpc_rts_mp(target_adapter, dm, GET_STA_INFO(entry).mac_id,
							entry->IOTPeer, GET_STA_INFO(entry).rssi_stat.rssi);

				ratr_state_new = phydm_rssi_lv_dec(dm, GET_STA_INFO(entry).rssi_stat.rssi,
								   GET_STA_INFO(entry).ra_info.rssi_level);

				if ((GET_STA_INFO(entry).ra_info.rssi_level != ratr_state_new)
				    || (ra_tab->up_ramask_cnt >= FORCED_UPDATE_RAMASK_PERIOD)) {
					ra_tab->up_ramask_cnt = 0;
					PHYDM_PRINT_ADDR(dm, DBG_RA_MASK, ("Target AP addr :"),
							 GET_STA_INFO(entry).mac_addr);
					PHYDM_DBG(dm, DBG_RA_MASK,
						  "Update Tx RA Level: ((%x)) -> ((%x)),  RSSI = ((%d))\n",
						  GET_STA_INFO(entry).ra_info.rssi_level, ratr_state_new,
						  GET_STA_INFO(entry).rssi_stat.rssi);

					GET_STA_INFO(entry).ra_info.rssi_level = ratr_state_new;
					adapter->HalFunc.UpdateHalRAMaskHandler(target_adapter,
										GET_STA_INFO(entry).mac_id, entry);
				} else {
					PHYDM_DBG(dm, DBG_RA_MASK, "Stay in RA level  = (( %d ))\n\n",
						  ratr_state_new);
					/**/
				}

			}
		}
	}
}

#endif

#if (DM_ODM_SUPPORT_TYPE == ODM_AP)

void odm_refresh_rate_adaptive_mask_ap(
	void	*dm_void
)
{
	struct dm_struct		*dm = (struct dm_struct *)dm_void;
	struct ra_table			*ra_tab = &dm->dm_ra_table;
	struct rtl8192cd_priv *priv = dm->priv;
	struct aid_obj *aidarray;
	u32		i;
	struct sta_info *entry;
	struct cmn_sta_info	*sta;
	u8		ratr_state_new;

	if (priv->up_time % 2)
		return;

	for (i = 0; i < ODM_ASSOCIATE_ENTRY_NUM; i++) {
		entry = dm->odm_sta_info[i];
		sta = dm->phydm_sta_info[i];

		if (is_sta_active(sta)) {
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
			aidarray = container_of(entry, struct aid_obj, station);
			priv = aidarray->priv;
#endif

			if (!priv->pmib->dot11StationConfigEntry.autoRate)
				continue;

			ratr_state_new = phydm_rssi_lv_dec(dm, (u32)sta->rssi_stat.rssi,
							   sta->ra_info.rssi_level);

			if ((sta->ra_info.rssi_level != ratr_state_new)
			    || (ra_tab->up_ramask_cnt >= FORCED_UPDATE_RAMASK_PERIOD)) {
				ra_tab->up_ramask_cnt = 0;
				PHYDM_PRINT_ADDR(dm, DBG_RA_MASK, "Target AP addr :", sta->mac_addr);
				PHYDM_DBG(dm, DBG_RA_MASK,
					  "Update Tx RA Level: ((%x)) -> ((%x)),  RSSI = ((%d))\n",
					  sta->ra_info.rssi_level, ratr_state_new, sta->rssi_stat.rssi);

				sta->ra_info.rssi_level = ratr_state_new;
				phydm_gen_ramask_h2c_AP(dm, priv, entry, sta->ra_info.rssi_level);
			} else {
				PHYDM_DBG(dm, DBG_RA_MASK, "Stay in RA level  = (( %d ))\n\n",
					  ratr_state_new);
				/**/
			}
		}
	}
}
#endif

void phydm_rate_adaptive_mask_init(
	void	*dm_void
)
{
	struct dm_struct		*dm = (struct dm_struct *)dm_void;
	struct ra_table	*ra_t = &dm->dm_ra_table;

#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
	PADAPTER		adapter	= (PADAPTER)dm->adapter;
	PMGNT_INFO		mgnt_info = &(adapter->MgntInfo);
	HAL_DATA_TYPE	*hal_data = GET_HAL_DATA(adapter);


	if (mgnt_info->DM_Type == dm_type_by_driver)
		hal_data->bUseRAMask = true;
	else
		hal_data->bUseRAMask = false;

#endif

	ra_t->ldpc_thres = 35;
	ra_t->up_ramask_cnt = 0;
	ra_t->up_ramask_cnt_tmp = 0;

}

void phydm_refresh_rate_adaptive_mask(
	void	*dm_void
)
{
	struct dm_struct		*dm = (struct dm_struct *)dm_void;
	struct ra_table	*ra_t = &dm->dm_ra_table;

	PHYDM_DBG(dm, DBG_RA_MASK, "%s ======>\n", __func__);

	if (!(dm->support_ability & ODM_BB_RA_MASK)) {
		PHYDM_DBG(dm, DBG_RA_MASK, "Return: Not support\n");
		return;
	}

	if (!dm->is_linked)
		return;

	ra_t->up_ramask_cnt++;
	/*ra_t->up_ramask_cnt_tmp++;*/


#if (DM_ODM_SUPPORT_TYPE & ODM_AP)

	odm_refresh_rate_adaptive_mask_ap(dm);

#else

	phydm_ra_mask_watchdog(dm);

#endif

}

void phydm_show_sta_info(
	void		*dm_void,
	char		input[][16],
	u32		*_used,
	char		*output,
	u32		*_out_len,
	u32		input_num
)
{
	struct dm_struct	*dm = (struct dm_struct *)dm_void;
	struct cmn_sta_info		*sta = NULL;
	struct ra_sta_info			*ra = NULL;
#ifdef CONFIG_BEAMFORMING
	struct bf_cmn_info		*bf = NULL;
#endif
	char		help[] = "-h";
	u32		var1[10] = {0};
	u32		used = *_used;
	u32		out_len = *_out_len;
	u32		i, macid_start, macid_end;
	u8		tatal_sta_num = 0;

	PHYDM_SSCANF(input[1], DCMD_DECIMAL, &var1[0]);

	if ((strcmp(input[1], help) == 0)) {
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "All STA: {1}\n");
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "STA[macid]: {2} {macid}\n");
		return;
	} else if (var1[0] == 1) {
		macid_start = 0;
		macid_end = ODM_ASSOCIATE_ENTRY_NUM;
	} else if (var1[0] == 2) {
		macid_start = var1[1];
		macid_end = var1[1];
	} else {
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "Warning input value!\n");
		return;
	}

	for (i = macid_start; i < macid_end; i++) {

		sta = dm->phydm_sta_info[i];


		if (!is_sta_active(sta))
			continue;

		ra = &sta->ra_info;
#ifdef CONFIG_BEAMFORMING
		bf = &sta->bf_info;
#endif

		tatal_sta_num++;

		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "==[MACID: %d]============>\n", sta->mac_id);
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "AID:%d\n", sta->aid);
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "ADDR:%x-%x-%x-%x-%x-%x\n",
			 sta->mac_addr[5], sta->mac_addr[4],
			 sta->mac_addr[3], sta->mac_addr[2],
			 sta->mac_addr[1], sta->mac_addr[0]);
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "DM_ctrl:0x%x\n", sta->dm_ctrl);
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "BW:%d, MIMO_Type:0x%x\n", sta->bw_mode,
			 sta->mimo_type);
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "STBC_en:%d, LDPC_en=%d\n", sta->stbc_en,
			 sta->ldpc_en);

		/*[RSSI Info]*/
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "RSSI{All, OFDM, CCK}={%d, %d, %d}\n",
			 sta->rssi_stat.rssi, sta->rssi_stat.rssi_ofdm,
			 sta->rssi_stat.rssi_cck);

		/*[RA Info]*/
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "Rate_ID:%d, RSSI_LV:%d, ra_bw:%d, SGI_en:%d\n",

			 ra->rate_id, ra->rssi_level, ra->ra_bw_mode,
			 ra->is_support_sgi);

		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "VHT_en:%d, Wireless_set=0x%x, sm_ps=%d\n",
			 ra->is_vht_enable, sta->support_wireless_set,
			 sta->sm_ps);

		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "Dis{RA, PT}={%d, %d}, TxRx:%d, Noisy:%d\n",
			 ra->disable_ra, ra->disable_pt, ra->txrx_state,
			 ra->is_noisy);

		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "TX{Rate, BW}={0x%x, %d}, RTY:%d\n",
			 ra->curr_tx_rate, ra->curr_tx_bw,
			 ra->curr_retry_ratio);

		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "RA_Mask:0x%llx\n", ra->ramask);

		/*[TP]*/
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "TP{TX,RX}={%d, %d}\n",
			 sta->tx_moving_average_tp,
			 sta->rx_moving_average_tp);

#ifdef CONFIG_BEAMFORMING
		/*[Beamforming]*/
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "BF CAP{HT,VHT}={0x%x, 0x%x}\n",
			 bf->ht_beamform_cap, bf->vht_beamform_cap);
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "BF {p_aid,g_id}={0x%x, 0x%x}\n\n",
			 bf->p_aid, bf->g_id);
#endif
	}

	if (tatal_sta_num == 0) {
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "No Linked STA\n");
	}

	*_used = used;
	*_out_len = out_len;
}

#ifdef	PHYDM_3RD_REFORM_RA_MASK

u8 phydm_get_tx_stream_num(
	void		*dm_void,
	enum 	rf_type	mimo_type

) {
	struct dm_struct	*dm = (struct dm_struct *)dm_void;
	u8	tx_num = 1;

	if (mimo_type == RF_1T1R || mimo_type == RF_1T2R)
		tx_num = 1;
	else if (mimo_type == RF_2T2R || mimo_type == RF_2T3R  || mimo_type == RF_2T4R)
		tx_num = 2;
	else if (mimo_type == RF_3T3R || mimo_type == RF_3T4R)
		tx_num = 3;
	else if (mimo_type == RF_4T4R)
		tx_num = 4;
	else
	{
		PHYDM_DBG(dm, DBG_RA, "[Warrning] no mimo_type is found\n");
	}
	return tx_num;
}

u64 phydm_get_bb_mod_ra_mask(
	void		*dm_void,
	u8		macid
)
{
	struct dm_struct	*dm = (struct dm_struct *)dm_void;
	struct cmn_sta_info		*sta = dm->phydm_sta_info[macid];
	struct ra_sta_info			*ra = NULL;
	enum channel_width		bw = (enum channel_width)0;
	enum wireless_set			wireless_mode = (enum wireless_set)0;
	u8		tx_stream_num = 1;
	u8		rssi_lv = 0;
	u64		ra_mask_bitmap = 0;

	if (is_sta_active(sta)) {

		ra = &sta->ra_info;
		bw = ra->ra_bw_mode;
		wireless_mode = sta->support_wireless_set;
		tx_stream_num = phydm_get_tx_stream_num(dm, sta->mimo_type);
		rssi_lv = ra->rssi_level;
		ra_mask_bitmap = ra->ramask;
	} else {
		PHYDM_DBG(dm, DBG_RA, "[Warning] %s invalid sta_info\n", __func__);
		return 0;
	}

	PHYDM_DBG(dm, DBG_RA, "macid=%d ori_RA_Mask= 0x%llx\n", sta->mac_id,
		  ra_mask_bitmap);
	PHYDM_DBG(dm, DBG_RA,
		  "wireless_mode=0x%x, tx_stream_num=%d, BW=%d, MimoPs=%d, rssi_lv=%d\n",
		  wireless_mode, tx_stream_num, bw, sta->sm_ps, rssi_lv);

	if (sta->sm_ps == SM_PS_STATIC) /*mimo_ps_enable*/
		tx_stream_num = 1;


	/*[Modify RA Mask by Wireless Mode]*/

	if (wireless_mode == WIRELESS_CCK)								/*B mode*/
		ra_mask_bitmap &= 0x0000000f;
	else if (wireless_mode == WIRELESS_OFDM)							/*G mode*/
		ra_mask_bitmap &= 0x00000ff0;
	else if (wireless_mode == (WIRELESS_CCK | WIRELESS_OFDM))			/*BG mode*/
		ra_mask_bitmap &= 0x00000ff5;
	else if (wireless_mode == (WIRELESS_CCK | WIRELESS_OFDM | WIRELESS_HT)) {
		/*N_2G*/
		if (tx_stream_num == 1) {
			if (bw == CHANNEL_WIDTH_40)
				ra_mask_bitmap &= 0x000ff015;
			else
				ra_mask_bitmap &= 0x000ff005;
		} else if (tx_stream_num == 2) {
			if (bw == CHANNEL_WIDTH_40)
				ra_mask_bitmap &= 0x0ffff015;
			else
				ra_mask_bitmap &= 0x0ffff005;
		} else if (tx_stream_num == 3)
			ra_mask_bitmap &= 0xffffff015;
	} else if (wireless_mode == (WIRELESS_OFDM | WIRELESS_HT)) {		/*N_5G*/

		if (tx_stream_num == 1) {
			if (bw == CHANNEL_WIDTH_40)
				ra_mask_bitmap &= 0x000ff030;
			else
				ra_mask_bitmap &= 0x000ff010;
		} else if (tx_stream_num == 2) {
			if (bw == CHANNEL_WIDTH_40)
				ra_mask_bitmap &= 0x0ffff030;
			else
				ra_mask_bitmap &= 0x0ffff010;
		} else if (tx_stream_num == 3)
			ra_mask_bitmap &= 0xffffff010;
	} else if (wireless_mode == (WIRELESS_CCK | WIRELESS_OFDM |
				     WIRELESS_VHT)) {
		/*AC_2G*/
		if (tx_stream_num == 1)
			ra_mask_bitmap &= 0x003ff015;
		else if (tx_stream_num == 2)
			ra_mask_bitmap &= 0xfffff015;
		else if (tx_stream_num == 3)
			ra_mask_bitmap &= 0x3fffffff010;


		if (bw == CHANNEL_WIDTH_20) {/* AC 20MHz doesn't support MCS9 */
			ra_mask_bitmap &= 0x1ff7fdfffff;
		}
	} else if (wireless_mode == (WIRELESS_OFDM | WIRELESS_VHT)) {		/*AC_5G*/

		if (tx_stream_num == 1)
			ra_mask_bitmap &= 0x003ff010;
		else if (tx_stream_num == 2)
			ra_mask_bitmap &= 0xfffff010;
		else  if (tx_stream_num == 3)
			ra_mask_bitmap &= 0x3fffffff010;

		if (bw == CHANNEL_WIDTH_20) /* AC 20MHz doesn't support MCS9 */
			ra_mask_bitmap &= 0x1ff7fdfffff;
	} else {
		PHYDM_DBG(dm, DBG_RA, "[Warrning] No RA mask is found\n");
		/**/
	}

	PHYDM_DBG(dm, DBG_RA, "Mod by mode=0x%llx\n", ra_mask_bitmap);


	/*[Modify RA Mask by RSSI level]*/
	if (wireless_mode != WIRELESS_CCK) {
		if (rssi_lv == 0)
			ra_mask_bitmap &= 0xffffffffffffffff;
		else if (rssi_lv == 1)
			ra_mask_bitmap &= 0xfffffffffffffff0;
		else if (rssi_lv == 2)
			ra_mask_bitmap &= 0xffffffffffffefe0;
		else if (rssi_lv == 3)
			ra_mask_bitmap &= 0xffffffffffffcfc0;
		else if (rssi_lv == 4)
			ra_mask_bitmap &= 0xffffffffffff8f80;
		else if (rssi_lv >= 5)
			ra_mask_bitmap &= 0xffffffffffff0f00;

	}
	PHYDM_DBG(dm, DBG_RA, "Mod by RSSI=0x%llx\n", ra_mask_bitmap);

	return ra_mask_bitmap;
}

u8 phydm_get_rate_id(
	void			*dm_void,
	u8			macid
)
{
	struct dm_struct	*dm = (struct dm_struct *)dm_void;
	struct cmn_sta_info		*sta = dm->phydm_sta_info[macid];
	struct ra_sta_info			*ra = NULL;
	enum channel_width		bw = (enum channel_width)0;
	enum wireless_set			wireless_mode = (enum wireless_set)0;
	u8	tx_stream_num = 1;
	u8	rate_id_idx = PHYDM_BGN_20M_1SS;

	if (is_sta_active(sta)) {

		ra = &sta->ra_info;
		bw = ra->ra_bw_mode;
		wireless_mode = sta->support_wireless_set;
		tx_stream_num = phydm_get_tx_stream_num(dm, sta->mimo_type);

	} else {
		PHYDM_DBG(dm, DBG_RA, "[Warning] %s: invalid sta_info\n", __func__);
		return 0;
	}

	PHYDM_DBG(dm, DBG_RA,
		  "macid=%d, wireless_set=0x%x, tx_stream_num=%d, BW=0x%x\n",
		  sta->mac_id, wireless_mode, tx_stream_num, bw);

	if (wireless_mode == WIRELESS_CCK)								/*B mode*/
		rate_id_idx = PHYDM_B_20M;
	else if (wireless_mode ==  WIRELESS_OFDM)						/*G mode*/
		rate_id_idx = PHYDM_G;
	else if (wireless_mode == (WIRELESS_CCK | WIRELESS_OFDM))			/*BG mode*/
		rate_id_idx = PHYDM_BG;
	else if (wireless_mode == (WIRELESS_OFDM | WIRELESS_HT)) {		/*GN mode*/

		if (tx_stream_num == 1)
			rate_id_idx = PHYDM_GN_N1SS;
		else if (tx_stream_num == 2)
			rate_id_idx = PHYDM_GN_N2SS;
		else if (tx_stream_num == 3)
			rate_id_idx = PHYDM_ARFR5_N_3SS;
	} else if (wireless_mode == (WIRELESS_CCK | WIRELESS_OFDM |
				     WIRELESS_HT)) {	/*BGN mode*/


		if (bw == CHANNEL_WIDTH_40) {
			if (tx_stream_num == 1)
				rate_id_idx = PHYDM_BGN_40M_1SS;
			else if (tx_stream_num == 2)
				rate_id_idx = PHYDM_BGN_40M_2SS;
			else if (tx_stream_num == 3)
				rate_id_idx = PHYDM_ARFR5_N_3SS;

		} else {
			if (tx_stream_num == 1)
				rate_id_idx = PHYDM_BGN_20M_1SS;
			else if (tx_stream_num == 2)
				rate_id_idx = PHYDM_BGN_20M_2SS;
			else if (tx_stream_num == 3)
				rate_id_idx = PHYDM_ARFR5_N_3SS;
		}
	} else if (wireless_mode == (WIRELESS_OFDM | WIRELESS_VHT)) {	/*AC mode*/

		if (tx_stream_num == 1)
			rate_id_idx = PHYDM_ARFR1_AC_1SS;
		else if (tx_stream_num == 2)
			rate_id_idx = PHYDM_ARFR0_AC_2SS;
		else if (tx_stream_num == 3)
			rate_id_idx = PHYDM_ARFR4_AC_3SS;
	} else if (wireless_mode == (WIRELESS_CCK | WIRELESS_OFDM |
				     WIRELESS_VHT)) {	/*AC 2.4G mode*/

		if (bw >= CHANNEL_WIDTH_80) {
			if (tx_stream_num == 1)
				rate_id_idx = PHYDM_ARFR1_AC_1SS;
			else if (tx_stream_num == 2)
				rate_id_idx = PHYDM_ARFR0_AC_2SS;
			else if (tx_stream_num == 3)
				rate_id_idx = PHYDM_ARFR4_AC_3SS;
		} else {
			if (tx_stream_num == 1)
				rate_id_idx = PHYDM_ARFR2_AC_2G_1SS;
			else if (tx_stream_num == 2)
				rate_id_idx = PHYDM_ARFR3_AC_2G_2SS;
			else if (tx_stream_num == 3)
				rate_id_idx = PHYDM_ARFR4_AC_3SS;
		}
	} else {
		PHYDM_DBG(dm, DBG_RA, "[Warrning] No rate_id is found\n");
		rate_id_idx = 0;
	}

	PHYDM_DBG(dm, DBG_RA, "Rate_ID=((0x%x))\n", rate_id_idx);

	return rate_id_idx;
}

void phydm_ra_h2c(
	void	*dm_void,
	u8	macid,
	u8	dis_ra,
	u8	dis_pt,
	u8	no_update_bw,
	u8	init_ra_lv,
	u64	ra_mask
)
{
	struct dm_struct		*dm = (struct dm_struct *)dm_void;
	struct cmn_sta_info			*sta = dm->phydm_sta_info[macid];
	struct ra_sta_info				*ra = NULL;
	u8		h2c_val[H2C_MAX_LENGTH] = {0};

	if (is_sta_active(sta)) {
		ra = &sta->ra_info;
	} else {
		PHYDM_DBG(dm, DBG_RA, "[Warning] %s invalid sta_info\n", __func__);
		return;
	}

	PHYDM_DBG(dm, DBG_RA, "%s ======>\n", __func__);
	PHYDM_DBG(dm, DBG_RA, "MACID=%d\n", sta->mac_id);

	if (dm->is_disable_power_training == true)
		dis_pt = true;
	else if (dm->is_disable_power_training == false)
		dis_pt = false;

	h2c_val[0] = sta->mac_id;
	h2c_val[1] = (ra->rate_id & 0x1f) | ((init_ra_lv & 0x3) << 5) |
		     (ra->is_support_sgi << 7);
	h2c_val[2] = (u8)((ra->ra_bw_mode) | (((sta->ldpc_en) ? 1 : 0) << 2) |
			  ((no_update_bw & 0x1) << 3) | (ra->is_vht_enable << 4) |
			  ((dis_pt & 0x1) << 6) | ((dis_ra & 0x1) << 7));

	h2c_val[3] = (u8)(ra_mask & 0xff);
	h2c_val[4] = (u8)((ra_mask & 0xff00) >> 8);
	h2c_val[5] = (u8)((ra_mask & 0xff0000) >> 16);
	h2c_val[6] = (u8)((ra_mask & 0xff000000) >> 24);

	PHYDM_DBG(dm, DBG_RA, "PHYDM h2c[0x40]=0x%x %x %x %x %x %x %x\n",
		  h2c_val[6], h2c_val[5], h2c_val[4], h2c_val[3], h2c_val[2], h2c_val[1],
		  h2c_val[0]);

	odm_fill_h2c_cmd(dm, PHYDM_H2C_RA_MASK, H2C_MAX_LENGTH, h2c_val);

#if (defined(PHYDM_COMPILE_ABOVE_3SS))
	if (dm->support_ic_type & (PHYDM_IC_ABOVE_3SS)) {

		h2c_val[3] = (u8)((ra_mask >> 32) & 0x000000ff);
		h2c_val[4] = (u8)(((ra_mask >> 32) & 0x0000ff00) >> 8);
		h2c_val[5] = (u8)(((ra_mask >> 32) & 0x00ff0000) >> 16);
		h2c_val[6] = (u8)(((ra_mask >> 32) & 0xff000000) >> 24);

		PHYDM_DBG(dm, DBG_RA, "PHYDM h2c[0x46]=0x%x %x %x %x %x %x %x\n",
			  h2c_val[6], h2c_val[5], h2c_val[4], h2c_val[3], h2c_val[2], h2c_val[1],
			  h2c_val[0]);

		odm_fill_h2c_cmd(dm, PHYDM_RA_MASK_ABOVE_3SS, 5, h2c_val);
	}
#endif
}

void phydm_ra_registed(
	void	*dm_void,
	u8	macid,
	u8	rssi_from_assoc
)
{
	struct dm_struct		*dm = (struct dm_struct *)dm_void;
	struct ra_table	*ra_t = &dm->dm_ra_table;
	struct cmn_sta_info			*sta = dm->phydm_sta_info[macid];
	struct ra_sta_info				*ra = NULL;
	u8	init_ra_lv;
	u64	ra_mask;

	if (is_sta_active(sta)) {
		ra = &sta->ra_info;
	} else {
		PHYDM_DBG(dm, DBG_RA_MASK, "[Warning] %s invalid sta_info\n", __func__);
		PHYDM_DBG(dm, DBG_RA_MASK, "MACID=%d\n", macid);
		return;
	}

	PHYDM_DBG(dm, DBG_RA_MASK, "%s ======>\n", __func__);
	PHYDM_DBG(dm, DBG_RA_MASK, "MACID=%d, rssi_from_assoc=%d\n",
		  sta->mac_id, rssi_from_assoc);

#if (RTL8188E_SUPPORT == 1) && (RATE_ADAPTIVE_SUPPORT == 1)
	if (dm->support_ic_type == ODM_RTL8188E)
		ra->rate_id = phydm_get_rate_id_88e(dm, macid);
	else
#endif
	{
		ra->rate_id = phydm_get_rate_id(dm, macid);
	}

	/*ra->is_vht_enable = (sta->support_wireless_set | WIRELESS_VHT) ? 1 : 0;*/
	/*ra->disable_ra = 0;*/
	/*ra->disable_pt = 0;*/
	ra_mask = phydm_get_bb_mod_ra_mask(dm, macid);


	if (rssi_from_assoc > 40)
		init_ra_lv = 3;
	else if (rssi_from_assoc > 20)
		init_ra_lv = 2;
	else
		init_ra_lv = 1;

	if (ra_t->record_ra_info)
		ra_t->record_ra_info(dm, macid, sta, ra_mask);

#if (RTL8188E_SUPPORT == 1) && (RATE_ADAPTIVE_SUPPORT == 1)
	if (dm->support_ic_type == ODM_RTL8188E)
		/*Driver RA*/
		odm_ra_update_rate_info_8188e(dm, macid, ra->rate_id, (u32)ra_mask,
					      ra->is_support_sgi);
	else
#endif
	{
		/*FW RA*/
		phydm_ra_h2c(dm, macid, ra->disable_ra, ra->disable_pt, 0, init_ra_lv,
			     ra_mask);
	}



}

void phydm_ra_offline(
	void	*dm_void,
	u8	macid
)
{
	struct dm_struct		*dm = (struct dm_struct *)dm_void;
	struct ra_table	*ra_t = &dm->dm_ra_table;
	struct cmn_sta_info			*sta = dm->phydm_sta_info[macid];
	struct ra_sta_info				*ra = NULL;

	if (is_sta_active(sta)) {
		ra = &sta->ra_info;
	} else {
		PHYDM_DBG(dm, DBG_RA, "[Warning] %s invalid sta_info\n", __func__);
		return;
	}

	PHYDM_DBG(dm, DBG_RA, "%s ======>\n", __func__);
	PHYDM_DBG(dm, DBG_RA, "MACID=%d\n", sta->mac_id);

	odm_memory_set(dm, &ra->rate_id, 0, sizeof(struct ra_sta_info));
	ra->disable_ra = 1;
	ra->disable_pt = 1;

	if (ra_t->record_ra_info)
		ra_t->record_ra_info(dm, macid, sta, 0);

	if (dm->support_ic_type != ODM_RTL8188E)
		phydm_ra_h2c(dm, macid, ra->disable_ra, ra->disable_pt, 0, 0, 0);
}

void phydm_ra_mask_watchdog(
	void	*dm_void
)
{
	struct dm_struct		*dm = (struct dm_struct *)dm_void;
	struct ra_table	*ra_t = &dm->dm_ra_table;
	struct cmn_sta_info			*sta = NULL;
	struct ra_sta_info				*ra = NULL;
	u8		macid;
	u64		ra_mask;
	u8		rssi_lv_new;

	if (!(dm->support_ability & ODM_BB_RA_MASK))
		return;

	if (((!dm->is_linked)) || (dm->phydm_sys_up_time % 2) == 1)
		return;

	PHYDM_DBG(dm, DBG_RA_MASK, "%s ======>\n", __func__);

	ra_t->up_ramask_cnt++;

	for (macid = 0; macid < ODM_ASSOCIATE_ENTRY_NUM; macid++) {

		sta = dm->phydm_sta_info[macid];

		if (!is_sta_active(sta))
			continue;

		ra = &sta->ra_info;

		if (ra->disable_ra)
			continue;


		/*to be modified*/
#if ((RTL8812A_SUPPORT == 1) || (RTL8821A_SUPPORT == 1))
		if ((dm->support_ic_type == ODM_RTL8812) ||
		    ((dm->support_ic_type == ODM_RTL8821) && (dm->cut_version == ODM_CUT_A))
		   ) {

			if (sta->rssi_stat.rssi < ra_t->ldpc_thres) {

#if (DM_ODM_SUPPORT_TYPE == ODM_CE)
				set_ra_ldpc_8812(sta, true);		/*LDPC TX enable*/
#elif (DM_ODM_SUPPORT_TYPE == ODM_WIN)
				{
					MgntSet_TX_LDPC(macid, true);
				}
#endif
				PHYDM_DBG(dm, DBG_RA_MASK, "RSSI=%d, ldpc_en =TRUE\n",
					  sta->rssi_stat.rssi);

			} else if (sta->rssi_stat.rssi > (ra_t->ldpc_thres + 3)) {
#if (DM_ODM_SUPPORT_TYPE == ODM_CE)
				set_ra_ldpc_8812(sta, false);	/*LDPC TX disable*/
#elif (DM_ODM_SUPPORT_TYPE == ODM_WIN)
				{
					MgntSet_TX_LDPC(macid, false);
				}
#endif
				PHYDM_DBG(dm, DBG_RA_MASK, "RSSI=%d, ldpc_en =FALSE\n",
					  sta->rssi_stat.rssi);
			}
		}
#endif

		rssi_lv_new = phydm_rssi_lv_dec(dm, (u32)sta->rssi_stat.rssi,
						ra->rssi_level);

		if ((ra->rssi_level != rssi_lv_new) ||
		    (ra_t->up_ramask_cnt >= FORCED_UPDATE_RAMASK_PERIOD)) {
			PHYDM_DBG(dm, DBG_RA_MASK, "RSSI LV:((%d))->((%d))\n", ra->rssi_level,
				  rssi_lv_new);

			ra->rssi_level = rssi_lv_new;
			ra_t->up_ramask_cnt = 0;

			ra_mask = phydm_get_bb_mod_ra_mask(dm, macid);

			if (ra_t->record_ra_info)
				ra_t->record_ra_info(dm, macid, sta, ra_mask);

#if (RTL8188E_SUPPORT == 1) && (RATE_ADAPTIVE_SUPPORT == 1)
			if (dm->support_ic_type == ODM_RTL8188E)
				/*Driver RA*/
				odm_ra_update_rate_info_8188e(dm, macid, ra->rate_id, (u32)ra_mask,
							      ra->is_support_sgi);
			else
#endif
			{
				/*FW RA*/
				phydm_ra_h2c(dm, macid, ra->disable_ra, ra->disable_pt, 1, 0, ra_mask);
			}
		}
	}

}
#endif

u8 phydm_vht_en_mapping(
	void			*dm_void,
	u32			wireless_mode
)
{
	struct dm_struct		*dm = (struct dm_struct *)dm_void;
	u8			vht_en_out = 0;

	if ((wireless_mode == PHYDM_WIRELESS_MODE_AC_5G) ||
	    (wireless_mode == PHYDM_WIRELESS_MODE_AC_24G) ||
	    (wireless_mode == PHYDM_WIRELESS_MODE_AC_ONLY)
	   ) {
		vht_en_out = 1;
		/**/
	}

	PHYDM_DBG(dm, DBG_RA, "wireless_mode= (( 0x%x )), VHT_EN= (( %d ))\n",
		  wireless_mode, vht_en_out);
	return vht_en_out;
}

u8 phydm_rate_id_mapping(
	void			*dm_void,
	u32			wireless_mode,
	u8			rf_type,
	u8			bw
)
{
	struct dm_struct		*dm = (struct dm_struct *)dm_void;
	u8			rate_id_idx = 0;

	PHYDM_DBG(dm, DBG_RA,
		  "wireless_mode= (( 0x%x )), rf_type = (( 0x%x )), BW = (( 0x%x ))\n",
		  wireless_mode, rf_type, bw);


	switch (wireless_mode) {
	case PHYDM_WIRELESS_MODE_N_24G: {
		if (bw == CHANNEL_WIDTH_40) {
			if (rf_type == RF_1T1R)
				rate_id_idx = PHYDM_BGN_40M_1SS;
			else if (rf_type == RF_2T2R)
				rate_id_idx = PHYDM_BGN_40M_2SS;
			else
				rate_id_idx = PHYDM_ARFR5_N_3SS;

		} else {
			if (rf_type == RF_1T1R)
				rate_id_idx = PHYDM_BGN_20M_1SS;
			else if (rf_type == RF_2T2R)
				rate_id_idx = PHYDM_BGN_20M_2SS;
			else
				rate_id_idx = PHYDM_ARFR5_N_3SS;
		}
	}
	break;

	case PHYDM_WIRELESS_MODE_N_5G: {
		if (rf_type == RF_1T1R)
			rate_id_idx = PHYDM_GN_N1SS;
		else if (rf_type == RF_2T2R)
			rate_id_idx = PHYDM_GN_N2SS;
		else
			rate_id_idx = PHYDM_ARFR5_N_3SS;
	}

	break;

	case PHYDM_WIRELESS_MODE_G:
		rate_id_idx = PHYDM_BG;
		break;

	case PHYDM_WIRELESS_MODE_A:
		rate_id_idx = PHYDM_G;
		break;

	case PHYDM_WIRELESS_MODE_B:
		rate_id_idx = PHYDM_B_20M;
		break;


	case PHYDM_WIRELESS_MODE_AC_5G:
	case PHYDM_WIRELESS_MODE_AC_ONLY: {
		if (rf_type == RF_1T1R)
			rate_id_idx = PHYDM_ARFR1_AC_1SS;
		else if (rf_type == RF_2T2R)
			rate_id_idx = PHYDM_ARFR0_AC_2SS;
		else
			rate_id_idx = PHYDM_ARFR4_AC_3SS;
	}
	break;

	case PHYDM_WIRELESS_MODE_AC_24G: {
		/*Becareful to set "Lowest rate" while using PHYDM_ARFR4_AC_3SS in 2.4G/5G*/
		if (bw >= CHANNEL_WIDTH_80) {
			if (rf_type == RF_1T1R)
				rate_id_idx = PHYDM_ARFR1_AC_1SS;
			else if (rf_type == RF_2T2R)
				rate_id_idx = PHYDM_ARFR0_AC_2SS;
			else
				rate_id_idx = PHYDM_ARFR4_AC_3SS;
		} else {
			if (rf_type == RF_1T1R)
				rate_id_idx = PHYDM_ARFR2_AC_2G_1SS;
			else if (rf_type == RF_2T2R)
				rate_id_idx = PHYDM_ARFR3_AC_2G_2SS;
			else
				rate_id_idx = PHYDM_ARFR4_AC_3SS;
		}
	}
	break;

	default:
		rate_id_idx = 0;
		break;
	}

	PHYDM_DBG(dm, DBG_RA, "RA rate ID = (( 0x%x ))\n", rate_id_idx);

	return rate_id_idx;
}

void phydm_update_hal_ra_mask(
	void			*dm_void,
	u32			wireless_mode,
	u8			rf_type,
	u8			bw,
	u8			mimo_ps_enable,
	u8			disable_cck_rate,
	u32			*ratr_bitmap_msb_in,
	u32			*ratr_bitmap_lsb_in,
	u8			tx_rate_level
)
{
	struct dm_struct		*dm = (struct dm_struct *)dm_void;
	u32			ratr_bitmap = *ratr_bitmap_lsb_in,
				ratr_bitmap_msb = *ratr_bitmap_msb_in;

	/*PHYDM_DBG(dm, DBG_RA_MASK, "phydm_rf_type = (( %x )), rf_type = (( %x ))\n", phydm_rf_type, rf_type);*/
	PHYDM_DBG(dm, DBG_RA_MASK,
		  "Platfoem original RA Mask = (( 0x %x | %x ))\n", ratr_bitmap_msb,
		  ratr_bitmap);

	switch (wireless_mode) {
	case PHYDM_WIRELESS_MODE_B: {
		ratr_bitmap &= 0x0000000f;
	}
	break;

	case PHYDM_WIRELESS_MODE_G: {
		ratr_bitmap &= 0x00000ff5;
	}
	break;

	case PHYDM_WIRELESS_MODE_A: {
		ratr_bitmap &= 0x00000ff0;
	}
	break;

	case PHYDM_WIRELESS_MODE_N_24G:
	case PHYDM_WIRELESS_MODE_N_5G: {
		if (mimo_ps_enable)
			rf_type = RF_1T1R;

		if (rf_type == RF_1T1R) {
			if (bw == CHANNEL_WIDTH_40)
				ratr_bitmap &= 0x000ff015;
			else
				ratr_bitmap &= 0x000ff005;
		} else if (rf_type == RF_2T2R || rf_type == RF_2T4R
			   || rf_type == RF_2T3R) {
			if (bw == CHANNEL_WIDTH_40)
				ratr_bitmap &= 0x0ffff015;
			else
				ratr_bitmap &= 0x0ffff005;
		} else { /*3T*/

			ratr_bitmap &= 0xfffff015;
			ratr_bitmap_msb &= 0xf;
		}
	}
	break;

	case PHYDM_WIRELESS_MODE_AC_24G: {
		if (rf_type == RF_1T1R)
			ratr_bitmap &= 0x003ff015;
		else if (rf_type == RF_2T2R || rf_type == RF_2T4R || rf_type == RF_2T3R)
			ratr_bitmap &= 0xfffff015;
		else {/*3T*/

			ratr_bitmap &= 0xfffff010;
			ratr_bitmap_msb &= 0x3ff;
		}

		if (bw == CHANNEL_WIDTH_20) {/* AC 20MHz doesn't support MCS9 */
			ratr_bitmap &= 0x7fdfffff;
			ratr_bitmap_msb &= 0x1ff;
		}
	}
	break;

	case PHYDM_WIRELESS_MODE_AC_5G: {
		if (rf_type == RF_1T1R)
			ratr_bitmap &= 0x003ff010;
		else if (rf_type == RF_2T2R || rf_type == RF_2T4R || rf_type == RF_2T3R)
			ratr_bitmap &= 0xfffff010;
		else {/*3T*/

			ratr_bitmap &= 0xfffff010;
			ratr_bitmap_msb &= 0x3ff;
		}

		if (bw == CHANNEL_WIDTH_20) {/* AC 20MHz doesn't support MCS9 */
			ratr_bitmap &= 0x7fdfffff;
			ratr_bitmap_msb &= 0x1ff;
		}
	}
	break;

	default:
		break;
	}

	if (wireless_mode != PHYDM_WIRELESS_MODE_B) {
		if (tx_rate_level == 0)
			ratr_bitmap &=  0xffffffff;
		else if (tx_rate_level == 1)
			ratr_bitmap &=  0xfffffff0;
		else if (tx_rate_level == 2)
			ratr_bitmap &=  0xffffefe0;
		else if (tx_rate_level == 3)
			ratr_bitmap &=  0xffffcfc0;
		else if (tx_rate_level == 4)
			ratr_bitmap &=  0xffff8f80;
		else if (tx_rate_level >= 5)
			ratr_bitmap &=  0xffff0f00;

	}

	if (disable_cck_rate)
		ratr_bitmap &= 0xfffffff0;

	PHYDM_DBG(dm, DBG_RA_MASK,
		  "wireless_mode= (( 0x%x )), rf_type = (( 0x%x )), BW = (( 0x%x )), MimoPs_en = (( %d )), tx_rate_level= (( 0x%x ))\n",
		  wireless_mode, rf_type, bw, mimo_ps_enable, tx_rate_level);

	/*PHYDM_DBG(dm, DBG_RA_MASK, "111 Phydm modified RA Mask = (( 0x %x | %x ))\n", ratr_bitmap_msb, ratr_bitmap);*/

	*ratr_bitmap_lsb_in = ratr_bitmap;
	*ratr_bitmap_msb_in = ratr_bitmap_msb;
	PHYDM_DBG(dm, DBG_RA_MASK, "Phydm modified RA Mask = (( 0x %x | %x ))\n",
		  *ratr_bitmap_msb_in, *ratr_bitmap_lsb_in);

}

u8 phydm_rssi_lv_dec(
	void			*dm_void,
	u32			rssi,
	u8			ratr_state
)
{
	struct dm_struct	*dm = (struct dm_struct *)dm_void;
	u8	rssi_lv_table[RA_FLOOR_TABLE_SIZE] = {20, 34, 38, 42, 46, 50, 100}; /*MCS0 ~ MCS4 , VHT1SS MCS0 ~ MCS4 , G 6M~24M*/
	u8	new_rssi_lv = 0;
	u8	i;

	PHYDM_DBG(dm, DBG_RA_MASK,
		  "curr RA level=(%d), Table_ori=[%d, %d, %d, %d, %d, %d]\n",
		  ratr_state, rssi_lv_table[0], rssi_lv_table[1], rssi_lv_table[2],
		  rssi_lv_table[3], rssi_lv_table[4], rssi_lv_table[5]);

	for (i = 0; i < RA_FLOOR_TABLE_SIZE; i++) {
		if (i >= (ratr_state))
			rssi_lv_table[i] += RA_FLOOR_UP_GAP;
	}

	PHYDM_DBG(dm, DBG_RA_MASK,
		  "RSSI=(%d), Table_mod=[%d, %d, %d, %d, %d, %d]\n",
		  rssi, rssi_lv_table[0], rssi_lv_table[1], rssi_lv_table[2],
		  rssi_lv_table[3], rssi_lv_table[4], rssi_lv_table[5]);

	for (i = 0; i < RA_FLOOR_TABLE_SIZE; i++) {
		if (rssi < rssi_lv_table[i]) {
			new_rssi_lv = i;
			break;
		}
	}
	return	new_rssi_lv;
}

u8 phydm_rate_order_compute(
	void	*dm_void,
	u8	rate_idx
)
{
	u8		rate_order = 0;

	if (rate_idx >= ODM_RATEVHTSS4MCS0) {
		rate_idx -= ODM_RATEVHTSS4MCS0;
		/**/
	} else if (rate_idx >= ODM_RATEVHTSS3MCS0) {
		rate_idx -= ODM_RATEVHTSS3MCS0;
		/**/
	} else if (rate_idx >= ODM_RATEVHTSS2MCS0) {
		rate_idx -= ODM_RATEVHTSS2MCS0;
		/**/
	} else if (rate_idx >= ODM_RATEVHTSS1MCS0) {
		rate_idx -= ODM_RATEVHTSS1MCS0;
		/**/
	} else if (rate_idx >= ODM_RATEMCS24) {
		rate_idx -= ODM_RATEMCS24;
		/**/
	} else if (rate_idx >= ODM_RATEMCS16) {
		rate_idx -= ODM_RATEMCS16;
		/**/
	} else if (rate_idx >= ODM_RATEMCS8) {
		rate_idx -= ODM_RATEMCS8;
		/**/
	}
	rate_order = rate_idx;

	return rate_order;

}

u8 phydm_rate2ss(
	void	*dm_void,
	u8	rate_idx
)
{
	u8		ret = 0xff;
	u8		i, j;
	u8		search_idx;
	u32		ss_mapping_tab[4][3] = {{0x00000000, 0x003ff000, 0x000ff000},
		{0x00000000, 0xffc00000, 0x0ff00000},
		{0x000003ff, 0x0000000f, 0xf0000000},
		{0x000ffc00, 0x00000ff0, 0x00000000}
	};
	if (rate_idx < 32) {
		search_idx = rate_idx;
		j = 0;
	} else if (rate_idx < 64) {
		search_idx = rate_idx - 32;
		j = 1;
	} else {
		search_idx = rate_idx - 64;
		j = 2;
	}
	for (i = 0; i < 4; i++)
		if (ss_mapping_tab[i][j] & BIT(search_idx))
			ret = i;
	return ret;
}

u8 phydm_rate2plcp(
	void	*dm_void,
	u8	rate_idx
)
{
	u8		rate2ss = 0;
	u8		vht_en = 0;
	u8		ltftime = 0;
	u8		plcptime = 0xff;

	if (rate_idx < ODM_RATE6M) {
		plcptime = 192;
		/* CCK PLCP = 192us (long preamble) */
	} else if (rate_idx < ODM_RATEMCS0) {
		plcptime = 20;
		/* LegOFDM PLCP = 20us */
	} else {
		if (rate_idx < ODM_RATEVHTSS1MCS0)
			plcptime = 32;
		/* HT mode PLCP = 20us + 12us + 4us x Nss */
		else
			plcptime = 36;
		/* VHT mode PLCP = 20us + 16us + 4us x Nss */
		rate2ss = phydm_rate2ss(dm_void, rate_idx);
		if (rate2ss == 0xff)
			return 0xff;
		else
			ltftime = (rate2ss + 1) * 4;
		plcptime += ltftime;
		/**/
	}
	return plcptime;

}

u8 phydm_get_plcp(
	void	*dm_void,
	u16	macid
)
{
	u8		plcp_time = 0;
	struct dm_struct	*dm = (struct dm_struct *)dm_void;
	struct cmn_sta_info			*sta = NULL;
	struct ra_sta_info				*ra = NULL;
	sta = dm->phydm_sta_info[macid];
	ra = &sta->ra_info;
	plcp_time = phydm_rate2plcp(dm, ra->curr_tx_rate);
	return plcp_time;
}


void phydm_ra_common_info_update(
	void	*dm_void
)
{
	struct dm_struct	*dm = (struct dm_struct *)dm_void;
	struct ra_table		*ra_tab = &dm->dm_ra_table;
	struct cmn_sta_info			*sta = NULL;
	u16		macid;
	u8		rate_order_tmp;
	u8		cnt = 0;

	ra_tab->highest_client_tx_order = 0;
	ra_tab->power_tracking_flag = 1;

	if (!dm->number_linked_client)
		return;

	for (macid = 0; macid < ODM_ASSOCIATE_ENTRY_NUM; macid++) {
		sta = dm->phydm_sta_info[macid];

		if (is_sta_active(sta)) {
			rate_order_tmp = phydm_rate_order_compute(dm,
					 (sta->ra_info.curr_tx_rate & 0x7f));

			if (rate_order_tmp >= (ra_tab->highest_client_tx_order)) {
				ra_tab->highest_client_tx_order = rate_order_tmp;
				ra_tab->highest_client_tx_rate_order = macid;
			}

			cnt++;

			if (cnt == dm->number_linked_client)
				break;
		}
	}
	PHYDM_DBG(dm, DBG_RA,
		  "MACID[%d], Highest Tx order Update for power traking: %d\n",
		  (ra_tab->highest_client_tx_rate_order), (ra_tab->highest_client_tx_order));
}

void phydm_ra_info_watchdog(
	void	*dm_void
)
{
	struct dm_struct	*dm = (struct dm_struct *)dm_void;

	phydm_ra_common_info_update(dm);
#if (defined(CONFIG_RA_DYNAMIC_RTY_LIMIT))
	phydm_ra_dynamic_retry_limit(dm);
#endif
	phydm_ra_dynamic_retry_count(dm);
	phydm_refresh_rate_adaptive_mask(dm);

#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
	odm_refresh_basic_rate_mask(dm);
#endif
}

void phydm_ra_info_init(
	void	*dm_void
)
{
	struct dm_struct	*dm = (struct dm_struct *)dm_void;
	struct ra_table		*ra_tab = &dm->dm_ra_table;

	ra_tab->highest_client_tx_rate_order = 0;
	ra_tab->highest_client_tx_order = 0;
	ra_tab->RA_threshold_offset = 0;
	ra_tab->RA_offset_direction = 0;

#if (RTL8822B_SUPPORT == 1)
	if (dm->support_ic_type == ODM_RTL8822B) {
		u32	ret_value;

		ret_value = odm_get_bb_reg(dm, 0x4c8, MASKBYTE2);
		odm_set_bb_reg(dm, 0x4cc, MASKBYTE3, (ret_value - 1));
	}
#endif

#ifdef CONFIG_RA_DYNAMIC_RTY_LIMIT
	phydm_ra_dynamic_retry_limit_init(dm);
#endif

#ifdef CONFIG_RA_DYNAMIC_RATE_ID
	phydm_ra_dynamic_rate_id_init(dm);
#endif

#ifdef CONFIG_RA_DBG_CMD
	odm_ra_para_adjust_init(dm);
#endif

	phydm_rate_adaptive_mask_init(dm);

}

u8 odm_find_rts_rate(
	void			*dm_void,
	u8			tx_rate,
	boolean		is_erp_protect
)
{
	struct dm_struct		*dm = (struct dm_struct *)dm_void;
	u8	rts_ini_rate = ODM_RATE6M;

	if (is_erp_protect) /* use CCK rate as RTS*/
		rts_ini_rate = ODM_RATE1M;
	else {
		switch (tx_rate) {
		case ODM_RATEVHTSS3MCS9:
		case ODM_RATEVHTSS3MCS8:
		case ODM_RATEVHTSS3MCS7:
		case ODM_RATEVHTSS3MCS6:
		case ODM_RATEVHTSS3MCS5:
		case ODM_RATEVHTSS3MCS4:
		case ODM_RATEVHTSS3MCS3:
		case ODM_RATEVHTSS2MCS9:
		case ODM_RATEVHTSS2MCS8:
		case ODM_RATEVHTSS2MCS7:
		case ODM_RATEVHTSS2MCS6:
		case ODM_RATEVHTSS2MCS5:
		case ODM_RATEVHTSS2MCS4:
		case ODM_RATEVHTSS2MCS3:
		case ODM_RATEVHTSS1MCS9:
		case ODM_RATEVHTSS1MCS8:
		case ODM_RATEVHTSS1MCS7:
		case ODM_RATEVHTSS1MCS6:
		case ODM_RATEVHTSS1MCS5:
		case ODM_RATEVHTSS1MCS4:
		case ODM_RATEVHTSS1MCS3:
		case ODM_RATEMCS15:
		case ODM_RATEMCS14:
		case ODM_RATEMCS13:
		case ODM_RATEMCS12:
		case ODM_RATEMCS11:
		case ODM_RATEMCS7:
		case ODM_RATEMCS6:
		case ODM_RATEMCS5:
		case ODM_RATEMCS4:
		case ODM_RATEMCS3:
		case ODM_RATE54M:
		case ODM_RATE48M:
		case ODM_RATE36M:
		case ODM_RATE24M:
			rts_ini_rate = ODM_RATE24M;
			break;
		case ODM_RATEVHTSS3MCS2:
		case ODM_RATEVHTSS3MCS1:
		case ODM_RATEVHTSS2MCS2:
		case ODM_RATEVHTSS2MCS1:
		case ODM_RATEVHTSS1MCS2:
		case ODM_RATEVHTSS1MCS1:
		case ODM_RATEMCS10:
		case ODM_RATEMCS9:
		case ODM_RATEMCS2:
		case ODM_RATEMCS1:
		case ODM_RATE18M:
		case ODM_RATE12M:
			rts_ini_rate = ODM_RATE12M;
			break;
		case ODM_RATEVHTSS3MCS0:
		case ODM_RATEVHTSS2MCS0:
		case ODM_RATEVHTSS1MCS0:
		case ODM_RATEMCS8:
		case ODM_RATEMCS0:
		case ODM_RATE9M:
		case ODM_RATE6M:
			rts_ini_rate = ODM_RATE6M;
			break;
		case ODM_RATE11M:
		case ODM_RATE5_5M:
		case ODM_RATE2M:
		case ODM_RATE1M:
			rts_ini_rate = ODM_RATE1M;
			break;
		default:
			rts_ini_rate = ODM_RATE6M;
			break;
		}
	}

	if (*dm->band_type == ODM_BAND_5G) {
		if (rts_ini_rate < ODM_RATE6M)
			rts_ini_rate = ODM_RATE6M;
	}
	return rts_ini_rate;

}

#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)

void odm_refresh_basic_rate_mask(
	void	*dm_void
)
{
	struct dm_struct		*dm = (struct dm_struct *)dm_void;
	void		*adapter	 =  dm->adapter;
	static u8		stage = 0;
	u8			cur_stage = 0;
	OCTET_STRING	os_rate_set;
	PMGNT_INFO		mgnt_info = GetDefaultMgntInfo(((PADAPTER)adapter));
	u8			rate_set[5] = {MGN_1M, MGN_2M, MGN_5_5M, MGN_11M, MGN_6M};

	if (dm->support_ic_type != ODM_RTL8812
	    && dm->support_ic_type != ODM_RTL8821)
		return;

	if (dm->is_linked == false)	/* unlink Default port information */
		cur_stage = 0;
	else if (dm->rssi_min < 40)	/* link RSSI  < 40% */
		cur_stage = 1;
	else if (dm->rssi_min > 45)	/* link RSSI > 45% */
		cur_stage = 3;
	else
		cur_stage = 2;					/* link  25% <= RSSI <= 30% */

	if (cur_stage != stage) {
		if (cur_stage == 1) {
			FillOctetString(os_rate_set, rate_set, 5);
			FilterSupportRate(mgnt_info->mBrates, &os_rate_set, false);
			phydm_set_hw_reg_handler_interface(dm, HW_VAR_BASIC_RATE,
							   (u8 *)&os_rate_set);
		} else if (cur_stage == 3 && (stage == 1 || stage == 2))
			phydm_set_hw_reg_handler_interface(dm, HW_VAR_BASIC_RATE,
							   (u8 *)(&mgnt_info->mBrates));
	}

	stage = cur_stage;
}

#if 0
void odm_refresh_ldpc_rts_mp(
	void			*adapter,
	struct dm_struct			*dm,
	u8				m_mac_id,
	u8				iot_peer,
	s32				undecorated_smoothed_pwdb
)
{
	boolean					is_ctl_ldpc = false;
	struct ra_table	*ra_t = &dm->dm_ra_table;

	if (dm->support_ic_type != ODM_RTL8821
	    && dm->support_ic_type != ODM_RTL8812)
		return;

	if ((dm->support_ic_type == ODM_RTL8821) && (dm->cut_version == ODM_CUT_A))
		is_ctl_ldpc = true;
	else if (dm->support_ic_type == ODM_RTL8812 &&
		 iot_peer == HT_IOT_PEER_REALTEK_JAGUAR_CCUTAP)
		is_ctl_ldpc = true;

	if (is_ctl_ldpc) {
		if (undecorated_smoothed_pwdb < (ra_t->ldpc_thres - 5))
			MgntSet_TX_LDPC(m_mac_id, true);
		else if (undecorated_smoothed_pwdb > ra_t->ldpc_thres)
			MgntSet_TX_LDPC(m_mac_id, false);
	}
}
#endif

#elif (DM_ODM_SUPPORT_TYPE & ODM_AP)

void phydm_gen_ramask_h2c_AP(
	void			*dm_void,
	struct rtl8192cd_priv *priv,
	struct sta_info *entry,
	u8			rssi_level
)
{
	struct dm_struct	*dm = (struct dm_struct *)dm_void;

	if (dm->support_ic_type == ODM_RTL8812) {
#if (RTL8812A_SUPPORT == 1)
		UpdateHalRAMask8812(priv, entry, rssi_level);
		/**/
#endif
	} else if (dm->support_ic_type == ODM_RTL8188E) {
#if (RTL8188E_SUPPORT == 1)
#ifdef TXREPORT
		add_RATid(priv, entry);
		/**/
#endif
#endif
	} else {
#ifdef CONFIG_WLAN_HAL
		GET_HAL_INTERFACE(priv)->UpdateHalRAMaskHandler(priv, entry, rssi_level);
#endif
	}
}

#endif

#if (defined(CONFIG_RA_DYNAMIC_RTY_LIMIT))

void phydm_retry_limit_table_bound(
	void	*dm_void,
	u8	*retry_limit,
	u8	offset
)
{
	struct dm_struct		*dm = (struct dm_struct *)dm_void;
	struct ra_table		*ra_tab = &dm->dm_ra_table;

	if (*retry_limit >  offset) {
		*retry_limit -= offset;

		if (*retry_limit < ra_tab->retrylimit_low)
			*retry_limit = ra_tab->retrylimit_low;
		else if (*retry_limit > ra_tab->retrylimit_high)
			*retry_limit = ra_tab->retrylimit_high;
	} else
		*retry_limit = ra_tab->retrylimit_low;
}

void phydm_reset_retry_limit_table(
	void	*dm_void
)
{
	struct dm_struct		*dm = (struct dm_struct *)dm_void;
	struct ra_table		*ra_tab = &dm->dm_ra_table;
	u8			i;

	u8 per_rate_retrylimit_table_20M[ODM_RATEMCS15 + 1] = {
		1, 1, 2, 4,					/*CCK*/
		2, 2, 4, 6, 8, 12, 16, 18,		/*OFDM*/
		2, 4, 6, 8, 12, 18, 20, 22,		/*20M HT-1SS*/
		2, 4, 6, 8, 12, 18, 20, 22		/*20M HT-2SS*/
	};
	u8 per_rate_retrylimit_table_40M[ODM_RATEMCS15 + 1] = {
		1, 1, 2, 4,					/*CCK*/
		2, 2, 4, 6, 8, 12, 16, 18,		/*OFDM*/
		4, 8, 12, 16, 24, 32, 32, 32,		/*40M HT-1SS*/
		4, 8, 12, 16, 24, 32, 32, 32		/*40M HT-2SS*/
	};

	memcpy(&ra_tab->per_rate_retrylimit_20M[0],
	       &per_rate_retrylimit_table_20M[0], ODM_NUM_RATE_IDX);
	memcpy(&ra_tab->per_rate_retrylimit_40M[0],
	       &per_rate_retrylimit_table_40M[0], ODM_NUM_RATE_IDX);

	for (i = 0; i < ODM_NUM_RATE_IDX; i++) {
		phydm_retry_limit_table_bound(dm,
					      &ra_tab->per_rate_retrylimit_20M[i],
					      0);
		phydm_retry_limit_table_bound(dm,
					      &ra_tab->per_rate_retrylimit_40M[i],
					      0);
	}
}

void phydm_ra_dynamic_retry_limit_init(
	void	*dm_void
)
{
	struct dm_struct		*dm = (struct dm_struct *)dm_void;
	struct ra_table			*ra_tab = &dm->dm_ra_table;

	ra_tab->retry_descend_num = RA_RETRY_DESCEND_NUM;
	ra_tab->retrylimit_low = RA_RETRY_LIMIT_LOW;
	ra_tab->retrylimit_high = RA_RETRY_LIMIT_HIGH;

	phydm_reset_retry_limit_table(dm);

}

void phydm_ra_dynamic_retry_limit(
	void	*dm_void
)
{
	struct dm_struct		*dm = (struct dm_struct *)dm_void;
	struct ra_table		*ra_tab = &dm->dm_ra_table;
	u8	i, retry_offset;
	u32	ma_rx_tp;


	if (dm->pre_number_active_client == dm->number_active_client) {
		PHYDM_DBG(dm, DBG_RA,
			  " pre_number_active_client ==  number_active_client\n");
		return;

	} else {
		if (dm->number_active_client == 1) {
			phydm_reset_retry_limit_table(dm);
			PHYDM_DBG(dm, DBG_RA, "one client only->reset to default value\n");
		} else {
			retry_offset = dm->number_active_client * ra_tab->retry_descend_num;

			for (i = 0; i < ODM_NUM_RATE_IDX; i++) {
				phydm_retry_limit_table_bound(dm,
							      &ra_tab->per_rate_retrylimit_20M[i],
							      retry_offset);
				phydm_retry_limit_table_bound(dm,
							      &ra_tab->per_rate_retrylimit_40M[i],
							      retry_offset);
			}
		}
	}
}
#endif

#if (defined(CONFIG_RA_DYNAMIC_RATE_ID))
void phydm_ra_dynamic_rate_id_on_assoc(
	void	*dm_void,
	u8	wireless_mode,
	u8	init_rate_id
)
{
	struct dm_struct	*dm = (struct dm_struct *)dm_void;

	PHYDM_DBG(dm, DBG_RA,
		  "[ON ASSOC] rf_mode = ((0x%x)), wireless_mode = ((0x%x)), init_rate_id = ((0x%x))\n",
		  dm->rf_type, wireless_mode, init_rate_id);

	if ((dm->rf_type == RF_2T2R) || (dm->rf_type == RF_2T3R)
	    || (dm->rf_type == RF_2T4R)) {
		if ((dm->support_ic_type & (ODM_RTL8812 | ODM_RTL8192E)) &&
		    (wireless_mode & (ODM_WM_N24G | ODM_WM_N5G))
		   ) {
			PHYDM_DBG(dm, DBG_RA, "[ON ASSOC] set N-2SS ARFR5 table\n");
			odm_set_mac_reg(dm, 0x4a4, MASKDWORD,
					0xfc1ffff);	/*N-2SS, ARFR5, rate_id = 0xe*/
			odm_set_mac_reg(dm, 0x4a8, MASKDWORD,
					0x0);		/*N-2SS, ARFR5, rate_id = 0xe*/
		} else if ((dm->support_ic_type & (ODM_RTL8812)) &&
			   (wireless_mode & (ODM_WM_AC_5G | ODM_WM_AC_24G | ODM_WM_AC_ONLY))
			  ) {
			PHYDM_DBG(dm, DBG_RA, "[ON ASSOC] set AC-2SS ARFR0 table\n");
			odm_set_mac_reg(dm, 0x444, MASKDWORD,
					0x0fff);	/*AC-2SS, ARFR0, rate_id = 0x9*/
			odm_set_mac_reg(dm, 0x448, MASKDWORD,
					0xff01f000);		/*AC-2SS, ARFR0, rate_id = 0x9*/
		}
	}

}

void phydm_ra_dynamic_rate_id_init(
	void	*dm_void
)
{
	struct dm_struct	*dm = (struct dm_struct *)dm_void;

	if (dm->support_ic_type & (ODM_RTL8812 | ODM_RTL8192E)) {
		odm_set_mac_reg(dm, 0x4a4, MASKDWORD,
				0xfc1ffff);	/*N-2SS, ARFR5, rate_id = 0xe*/
		odm_set_mac_reg(dm, 0x4a8, MASKDWORD,
				0x0);		/*N-2SS, ARFR5, rate_id = 0xe*/

		odm_set_mac_reg(dm, 0x444, MASKDWORD,
				0x0fff);		/*AC-2SS, ARFR0, rate_id = 0x9*/
		odm_set_mac_reg(dm, 0x448, MASKDWORD,
				0xff01f000);	/*AC-2SS, ARFR0, rate_id = 0x9*/
	}
}

void phydm_update_rate_id(
	void	*dm_void,
	u8	rate,
	u8	platform_macid
)
{
#if 0

	struct dm_struct	*dm = (struct dm_struct *)dm_void;
	struct ra_table		*ra_tab = &dm->dm_ra_table;
	u8		current_tx_ss;
	u8		rate_idx = rate & 0x7f; /*remove bit7 SGI*/
	u8		wireless_mode;
	u8		phydm_macid;
	struct sta_info	*entry;
	struct cmn_sta_info	*sta;


#if	0
	if (rate_idx >= ODM_RATEVHTSS2MCS0) {
		PHYDM_DBG(dm, DBG_RA, "rate[%d]: (( VHT2SS-MCS%d ))\n", platform_macid,
			  (rate_idx - ODM_RATEVHTSS2MCS0));
		/*dummy for SD4 check patch*/
	} else if (rate_idx >= ODM_RATEVHTSS1MCS0) {
		PHYDM_DBG(dm, DBG_RA, "rate[%d]: (( VHT1SS-MCS%d ))\n", platform_macid,
			  (rate_idx - ODM_RATEVHTSS1MCS0));
		/*dummy for SD4 check patch*/
	} else if (rate_idx >= ODM_RATEMCS0) {
		PHYDM_DBG(dm, DBG_RA, "rate[%d]: (( HT-MCS%d ))\n", platform_macid,
			  (rate_idx - ODM_RATEMCS0));
		/*dummy for SD4 check patch*/
	} else {
		PHYDM_DBG(dm, DBG_RA, "rate[%d]: (( HT-MCS%d ))\n", platform_macid,
			  rate_idx);
		/*dummy for SD4 check patch*/
	}
#endif

	phydm_macid = dm->phydm_macid_table[platform_macid];
	entry = dm->odm_sta_info[phydm_macid];
	sta = dm->phydm_sta_info[phydm_macid];

	if (is_sta_active(sta)) {
		wireless_mode = entry->wireless_mode;

		if ((dm->rf_type  == RF_2T2R) || (dm->rf_type  == RF_2T3R)
		    || (dm->rf_type  == RF_2T4R)) {
			if (wireless_mode & (ODM_WM_N24G | ODM_WM_N5G)) { /*N mode*/
				if (rate_idx >= ODM_RATEMCS8 && rate_idx <= ODM_RATEMCS15) { /*2SS mode*/

					sta->ra_info.rate_id  = ARFR_5_RATE_ID;
					PHYDM_DBG(dm, DBG_RA, "ARFR_5\n");
				}
			} else if (wireless_mode & (ODM_WM_AC_5G | ODM_WM_AC_24G |
						    ODM_WM_AC_ONLY)) {/*AC mode*/
				if (rate_idx >= ODM_RATEVHTSS2MCS0
				    && rate_idx <= ODM_RATEVHTSS2MCS9) {/*2SS mode*/

					sta->ra_info.rate_id  = ARFR_0_RATE_ID;
					PHYDM_DBG(dm, DBG_RA, "ARFR_0\n");
				}
			} else
				sta->ra_info.rate_id  = ARFR_0_RATE_ID;

			PHYDM_DBG(dm, DBG_RA, "UPdate_RateID[%d]: (( 0x%x ))\n", platform_macid,
				  sta->ra_info.rate_id);
		}
	}
#endif
}

#endif

#if (defined(CONFIG_RA_DBG_CMD))
void odm_ra_para_adjust_send_h2c(
	void	*dm_void
)
{
	struct dm_struct		*dm = (struct dm_struct *)dm_void;
	struct ra_table			*ra_tab = &dm->dm_ra_table;
	u8			h2c_parameter[6] = {0};

	h2c_parameter[0] = RA_FIRST_MACID;

	if (ra_tab->ra_para_feedback_req) { /*h2c_parameter[5]=1 ; ask FW for all RA parameters*/
		PHYDM_DBG(dm, DBG_RA, "[H2C] Ask FW for RA parameter\n");
		h2c_parameter[5] |= BIT(1); /*ask FW to report RA parameters*/
		h2c_parameter[1] = ra_tab->para_idx; /*ra_tab->para_idx;*/
		ra_tab->ra_para_feedback_req = 0;
	} else {
		PHYDM_DBG(dm, DBG_RA, "[H2C] Send H2C to FW for modifying RA parameter\n");

		h2c_parameter[1] =  ra_tab->para_idx;
		h2c_parameter[2] =  ra_tab->rate_idx;
		/* [8 bit]*/
		if (ra_tab->para_idx == RADBG_RTY_PENALTY
		    || ra_tab->para_idx == RADBG_RATE_UP_RTY_RATIO
		    || ra_tab->para_idx == RADBG_RATE_DOWN_RTY_RATIO) {
			h2c_parameter[3] = ra_tab->value;
			h2c_parameter[4] = 0;
		}
		/* [16 bit]*/
		else {
			h2c_parameter[3] = (u8)(((ra_tab->value_16) & 0xf0) >> 4); /*byte1*/
			h2c_parameter[4] = (u8)((ra_tab->value_16) & 0x0f);	/*byte0*/
		}
	}
	PHYDM_DBG(dm, DBG_RA, " h2c_parameter[1] = 0x%x\n", h2c_parameter[1]);
	PHYDM_DBG(dm, DBG_RA, " h2c_parameter[2] = 0x%x\n", h2c_parameter[2]);
	PHYDM_DBG(dm, DBG_RA, " h2c_parameter[3] = 0x%x\n", h2c_parameter[3]);
	PHYDM_DBG(dm, DBG_RA, " h2c_parameter[4] = 0x%x\n", h2c_parameter[4]);
	PHYDM_DBG(dm, DBG_RA, " h2c_parameter[5] = 0x%x\n", h2c_parameter[5]);

	odm_fill_h2c_cmd(dm, ODM_H2C_RA_PARA_ADJUST, 6, h2c_parameter);

}


void odm_ra_para_adjust(
	void		*dm_void
)
{
	struct dm_struct		*dm = (struct dm_struct *)dm_void;
	struct ra_table			*ra_tab = &dm->dm_ra_table;
	u8			rate_idx = ra_tab->rate_idx;
	u8			value = ra_tab->value;
	u8			pre_value = 0xff;

	if (ra_tab->para_idx == RADBG_RTY_PENALTY) {
		pre_value = ra_tab->RTY_P[rate_idx];
		ra_tab->RTY_P[rate_idx] = value;
		ra_tab->RTY_P_modify_note[rate_idx] = 1;
	} else if (ra_tab->para_idx == RADBG_N_HIGH) {
	} else if (ra_tab->para_idx == RADBG_N_LOW) {
	} else if (ra_tab->para_idx == RADBG_RATE_UP_RTY_RATIO) {
		pre_value = ra_tab->RATE_UP_RTY_RATIO[rate_idx];
		ra_tab->RATE_UP_RTY_RATIO[rate_idx] = value;
		ra_tab->RATE_UP_RTY_RATIO_modify_note[rate_idx] = 1;
	} else if (ra_tab->para_idx == RADBG_RATE_DOWN_RTY_RATIO) {
		pre_value = ra_tab->RATE_DOWN_RTY_RATIO[rate_idx];
		ra_tab->RATE_DOWN_RTY_RATIO[rate_idx] = value;
		ra_tab->RATE_DOWN_RTY_RATIO_modify_note[rate_idx] = 1;
	}
	PHYDM_DBG(dm, DBG_RA,
		  "Change RA Papa[%d], rate[ %d ],   ((%d))  ->  ((%d))\n", ra_tab->para_idx,
		  rate_idx, pre_value, value);
	odm_ra_para_adjust_send_h2c(dm);
}

void phydm_ra_print_msg(
	void		*dm_void,
	u8		*value,
	u8		*value_default,
	u8		*modify_note
)
{
	struct dm_struct		*dm = (struct dm_struct *)dm_void;
	struct ra_table			*ra_tab = &dm->dm_ra_table;
	u32 i;

	PHYDM_DBG(dm, DBG_RA,
		  " |rate index| |Current-value| |Default-value| |Modify?|\n");
	for (i = 0 ; i <= (ra_tab->rate_length); i++) {
#if (DM_ODM_SUPPORT_TYPE & (ODM_WIN))
		PHYDM_DBG(dm, DBG_RA, "     [ %d ]  %20d  %25d  %20s\n", i, value[i],
			  value_default[i], ((modify_note[i] == 1) ? "V" : " .  "));
#else
		PHYDM_DBG(dm, DBG_RA, "     [ %d ]  %10d  %14d  %14s\n", i, value[i],
			  value_default[i], ((modify_note[i] == 1) ? "V" : " .  "));
#endif
	}

}

void odm_RA_debug(
	void		*dm_void,
	u32		*const dm_value
)
{
	struct dm_struct		*dm = (struct dm_struct *)dm_void;
	struct ra_table			*ra_tab = &dm->dm_ra_table;

	ra_tab->is_ra_dbg_init = false;

	if (dm_value[0] == 100) { /*1 Print RA Parameters*/
		u8	default_pointer_value;
		u8	*pvalue;
		u8	*pvalue_default;
		u8	*pmodify_note;

		pvalue = pvalue_default = pmodify_note = &default_pointer_value;

		PHYDM_DBG(dm, DBG_RA,
			  "\n------------------------------------------------------------------------------------\n");

		if (dm_value[1] == RADBG_RTY_PENALTY) { /* [1]*/
			PHYDM_DBG(dm, DBG_RA, " [1] RTY_PENALTY\n");
			pvalue		=	&ra_tab->RTY_P[0];
			pvalue_default	=	&ra_tab->RTY_P_default[0];
			pmodify_note	=	(u8 *)&ra_tab->RTY_P_modify_note[0];
		} else if (dm_value[1] == RADBG_N_HIGH)   /* [2]*/
			PHYDM_DBG(dm, DBG_RA, " [2] N_HIGH\n");

		else if (dm_value[1] == RADBG_N_LOW)   /*[3]*/
			PHYDM_DBG(dm, DBG_RA, " [3] N_LOW\n");

		else if (dm_value[1] == RADBG_RATE_UP_RTY_RATIO) { /* [8]*/
			PHYDM_DBG(dm, DBG_RA, " [8] RATE_UP_RTY_RATIO\n");
			pvalue		=	&ra_tab->RATE_UP_RTY_RATIO[0];
			pvalue_default	=	&ra_tab->RATE_UP_RTY_RATIO_default[0];
			pmodify_note	=	(u8 *)&ra_tab->RATE_UP_RTY_RATIO_modify_note[0];
		} else if (dm_value[1] == RADBG_RATE_DOWN_RTY_RATIO) { /* [9]*/
			PHYDM_DBG(dm, DBG_RA, " [9] RATE_DOWN_RTY_RATIO\n");
			pvalue		=	&ra_tab->RATE_DOWN_RTY_RATIO[0];
			pvalue_default	=	&ra_tab->RATE_DOWN_RTY_RATIO_default[0];
			pmodify_note	=	(u8 *)&ra_tab->RATE_DOWN_RTY_RATIO_modify_note[0];
		}

		phydm_ra_print_msg(dm, pvalue, pvalue_default, pmodify_note);
		PHYDM_DBG(dm, DBG_RA,
			  "\n------------------------------------------------------------------------------------\n\n");

	} else if (dm_value[0] == 101) {
		ra_tab->para_idx = (u8)dm_value[1];

		ra_tab->ra_para_feedback_req = 1;
		odm_ra_para_adjust_send_h2c(dm);
	} else {
		ra_tab->para_idx = (u8)dm_value[0];
		ra_tab->rate_idx  = (u8)dm_value[1];
		ra_tab->value = (u8)dm_value[2];

		odm_ra_para_adjust(dm);
	}
}

void odm_ra_para_adjust_init(
	void		*dm_void
)
{
	struct dm_struct		*dm = (struct dm_struct *)dm_void;
	struct ra_table			*ra_tab = &dm->dm_ra_table;
	u8			i;
	u8			ra_para_pool_u8[3] = { RADBG_RTY_PENALTY,  RADBG_RATE_UP_RTY_RATIO, RADBG_RATE_DOWN_RTY_RATIO};
	u8			rate_size_ht_1ss = 20, rate_size_ht_2ss = 28,
				rate_size_ht_3ss = 36;	 /*4+8+8+8+8 =36*/
	u8			rate_size_vht_1ss = 10, rate_size_vht_2ss = 20,
				rate_size_vht_3ss = 30;	 /*10 + 10 +10 =30*/
#if 0
	/* RTY_PENALTY		=	1,   u8 */
	/* N_HIGH 				=	2, */
	/* N_LOW				=	3, */
	/* RATE_UP_TABLE		=	4, */
	/* RATE_DOWN_TABLE	=	5, */
	/* TRYING_NECESSARY	=	6, */
	/* DROPING_NECESSARY =	7, */
	/* RATE_UP_RTY_RATIO	=	8,  u8 */
	/* RATE_DOWN_RTY_RATIO=	9,  u8 */
	/* ALL_PARA		=	0xff */

#endif
	PHYDM_DBG(dm, DBG_RA, "odm_ra_para_adjust_init\n");

	/* JJ ADD 20161014 */
	if (dm->support_ic_type & (ODM_RTL8188F | ODM_RTL8195A | ODM_RTL8703B |
				   ODM_RTL8723B | ODM_RTL8188E | ODM_RTL8723D | ODM_RTL8710B))
		ra_tab->rate_length = rate_size_ht_1ss;
	else if (dm->support_ic_type & (ODM_RTL8192E | ODM_RTL8197F))
		ra_tab->rate_length = rate_size_ht_2ss;
	else if (dm->support_ic_type & (ODM_RTL8821 | ODM_RTL8881A | ODM_RTL8821C))
		ra_tab->rate_length = rate_size_ht_1ss + rate_size_vht_1ss;
	else if (dm->support_ic_type & (ODM_RTL8812 | ODM_RTL8822B))
		ra_tab->rate_length = rate_size_ht_2ss + rate_size_vht_2ss;
	else if (dm->support_ic_type == ODM_RTL8814A)
		ra_tab->rate_length = rate_size_ht_3ss + rate_size_vht_3ss;
	else
		ra_tab->rate_length = rate_size_ht_1ss;

	ra_tab->is_ra_dbg_init = true;
	for (i = 0; i < 3; i++) {
		ra_tab->ra_para_feedback_req = 1;
		ra_tab->para_idx	=	ra_para_pool_u8[i];
		odm_ra_para_adjust_send_h2c(dm);
	}
}

#endif /*#if (defined(CONFIG_RA_DBG_CMD))*/


