/************************************************************************************************/
/*                                                                                              */
/*  Copyright 2010  Broadcom Corporation                                                        */
/*                                                                                              */
/*     Unless you and Broadcom execute a separate written software license agreement governing  */
/*     use of this software, this software is licensed to you under the terms of the GNU        */
/*     General Public License version 2 (the GPL), available at                                 */
/*                                                                                              */
/*          http://www.broadcom.com/licenses/GPLv2.php                                          */
/*                                                                                              */
/*     with the following added to such license:                                                */
/*                                                                                              */
/*     As a special exception, the copyright holders of this software give you permission to    */
/*     link this software with independent modules, and to copy and distribute the resulting    */
/*     executable under terms of your choice, provided that you also meet, for each linked      */
/*     independent module, the terms and conditions of the license of that module.              */
/*     An independent module is a module which is not derived from this software.  The special  */
/*     exception does not apply to any modifications of the software.                           */
/*                                                                                              */
/*     Notwithstanding the above, under no circumstances may you combine this software in any   */
/*     way with any other Broadcom software provided under a license other than the GPL,        */
/*     without Broadcom's express prior written consent.                                        */
/*                                                                                              */
/************************************************************************************************/

#include <linux/kernel.h>
#include <linux/mmc-poll/mmc_poll.h>
#include <linux/mmc-poll/mmc_poll_stack.h>
#include <mach/sdio_platform.h>
#include "kona_mmc.h"

#ifdef DEBUG
static void print_mmcinfo(struct mmc *mmc);
#endif

int mmc_poll_stack_init(void **mmc_p, int dev_num, int *mmc_poll_dev_num)
{
	int status;
	int kona_dev_num;
	struct mmc **mmc = (struct mmc **)mmc_p;

	if (mmc == NULL || mmc_poll_dev_num == NULL || dev_num < 0)
		return -1;

	mmc_initialize();

	/* dev_num = 0 must be mmc0 and SDIO2
	 *           1 must be mmc1 and SDIO3
	 */
	kona_dev_num = dev_num + 2;

	/* e.g) eMMC instance number 2 and for SD Card use 1 */
	status = kona_mmc_init(kona_dev_num);
	if (status < 0) {
		pr_err("kona_mmc_init failed kona_dev:%d\n", kona_dev_num);
		return -1;
	}

	pr_debug("kona_mmc_init done %d\n", kona_dev_num);

	/* Use 0 for first device registered via kona_mmc_init etc */
	*mmc = find_mmc_device(0);
	if (*mmc == NULL) {
		pr_err("No mmc device found\n");
		return -1;
	}

	if (mmc_init(*mmc) < 0) {
		pr_err("mmc init failed kona_dev:%d\n", kona_dev_num);
		return -1;
	}
	pr_debug("mmc_init done kona_dev:%d\n", kona_dev_num);

#ifdef DEBUG
	print_mmcinfo(*mmc);
#endif

	/* Use 0 for the first device registered via kona_mmc_init etc */
	*mmc_poll_dev_num = 0;

	return 0;
}

#ifdef DEBUG
static void print_mmcinfo(struct mmc *mmc)
{
	pr_info("Device: %s\n", mmc->name);
	pr_info("Manufacturer ID: %x\n", mmc->cid[0] >> 24);
	pr_info("OEM: %x\n", (mmc->cid[0] >> 8) & 0xffff);
	pr_info("Name: %c%c%c%c%c\n", mmc->cid[0] & 0xff,
		(mmc->cid[1] >> 24), (mmc->cid[1] >> 16) & 0xff,
		(mmc->cid[1] >> 8) & 0xff, mmc->cid[1] & 0xff);

	pr_info("Tran Speed: %d\n", mmc->tran_speed);
	pr_info("Rd Block Len: %d\n", mmc->read_bl_len);

	pr_info("%s version %d.%d\n", IS_SD(mmc) ? "SD" : "MMC",
		(mmc->version >> 4) & 0xf, mmc->version & 0xf);

	pr_info("High Capacity: %s\n", mmc->high_capacity ? "Yes" : "No");
	pr_info("Capacity: %lld\n", mmc->capacity);

	pr_info("Bus Width: %d-bit\n", mmc->bus_width);
}
#endif
