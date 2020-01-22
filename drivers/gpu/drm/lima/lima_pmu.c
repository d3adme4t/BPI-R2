// SPDX-License-Identifier: GPL-2.0 OR MIT
/* Copyright 2017-2019 Qiang Yu <yuq825@gmail.com> */

#include <linux/of.h>
#include <linux/iopoll.h>
#include <linux/device.h>

#include "lima_device.h"
#include "lima_pmu.h"
#include "lima_regs.h"

#define pmu_write(reg, data) writel(data, ip->iomem + reg)
#define pmu_read(reg) readl(ip->iomem + reg)

static int lima_pmu_wait_cmd(struct lima_ip *ip)
{
	struct lima_device *dev = ip->dev;
	int err;
	u32 v;

	err = readl_poll_timeout(ip->iomem + LIMA_PMU_INT_RAWSTAT,
				 v, v & LIMA_PMU_INT_CMD_MASK,
				 100, 100000);
	if (err) {
		dev_err(dev->dev, "timeout wait pmd cmd\n");
		return err;
	}

	pmu_write(LIMA_PMU_INT_CLEAR, LIMA_PMU_INT_CMD_MASK);
	return 0;
}

int lima_pmu_init(struct lima_ip *ip)
{
	printk ("LIMA: Entering pmu_init.\n");
	int err;
	u32 stat;

	struct lima_device *dev = ip->dev;
	struct device_node *np = dev->dev->of_node;

	if (of_property_read_u32(np, "switch-delay", &ip->data.switch_delay))
	ip->data.switch_delay = 0xff;
	
	printk("LIMA: pmu_init sw_delay:%u\n",ip->data.switch_delay);

	pmu_write(LIMA_PMU_INT_MASK, 0);

	/* If this value is too low, when in high GPU clk freq,
	 * GPU will be in unstable state.
	 */
	pmu_write(LIMA_PMU_SW_DELAY, ip->data.switch_delay);

	/* status reg 1=off 0=on */
	stat = pmu_read(LIMA_PMU_STATUS);
	
	printk ("LIMA: pmu_init stat:%u\n",stat);

	/* power up all ip */
	if (stat) {
		pmu_write(LIMA_PMU_POWER_UP, stat);
		err = lima_pmu_wait_cmd(ip);
		if (err)
			{
			printk ("LIMA: pmu_init Err:%i\n",err);
			return err;
			}
			printk ("LIMA: pmu_init NO Err:%i\n",err);
	}
	return 0;
}

void lima_pmu_fini(struct lima_ip *ip)
{

}
