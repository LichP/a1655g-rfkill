/***************************************************************************
 *   Copyright (C) 2011 Phil Stewart, Daniel Drake                         *
 *                                                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 *                                                                         *
 *   The full GNU General Public License is included in this distribution  *
 *   in the file called COPYING.                                           *
 *                                                                         *
 *   Author:                                                               *
 *    Phil Stewart <phil.stewart@lichp.co.uk>                              *
 *                                                                         *
 *   Derived from xo1-rfkill by:                                           *
 *    Daniel Drake <dsd@laptop.org>                                        *
 *                                                                         *
 *   Uses Wifi on/off commands from fsaa1655g by:                          *
 *    Martin Vecera <ja@marvec.org>                                        *
 ***************************************************************************/

#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/kernel.h>
#include <linux/rfkill.h>
#include <linux/i8042.h>

MODULE_AUTHOR("Phil Stewart <phil.stewart@lichp.co.uk>");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.1");
MODULE_DESCRIPTION("An rfkill switch for the WLAN radio on Fujitsu Siemens Amilo A1655G laptops");

#define WIFI_I8042_COMMAND	0xC5
#define WIFI_I8042_PARAM_ON	0x25
#define WIFI_I8042_PARAM_OFF	0x45

static int a1655g_i8042_send_command(unsigned char param, int command)
{
	int err;
	i8042_lock_chip();
	/* i8042_command accepts an array of parameters, the length of which
	 * is encoded in the command int starting at bit 12) */
	err = i8042_command(&param, command | (1 << 12));
	i8042_unlock_chip();
	return err;
}

static int rfkill_set_block(void *data, bool blocked)
{
	unsigned char wifi_i8042_param;
	if (blocked) {
		printk(KERN_INFO "a1655g-rfkill: Turning WLAN radio OFF");
		wifi_i8042_param = WIFI_I8042_PARAM_OFF;
	}
	else {
		printk(KERN_INFO "a1655g-rfkill: Turning WLAN radio ON");
		wifi_i8042_param = WIFI_I8042_PARAM_ON;
	}
	
	return a1655g_i8042_send_command(wifi_i8042_param, WIFI_I8042_COMMAND);
};

static const struct rfkill_ops rfkill_ops = {
	.set_block = rfkill_set_block,
};
        

static int __devinit a1655g_rfkill_probe(struct platform_device *pdev)
{
	struct rfkill *rfk;
	int r;

	rfk = rfkill_alloc(pdev->name, &pdev->dev, RFKILL_TYPE_WLAN,
			&rfkill_ops, NULL);
	if (!rfk)
		return -ENOMEM;

	r = rfkill_register(rfk);
	if (r) {
		rfkill_destroy(rfk);
		return r;
	}

	platform_set_drvdata(pdev, rfk);
	return 0;
}
 
static int __devexit a1655g_rfkill_remove(struct platform_device *pdev)
{
	struct rfkill *rfk = platform_get_drvdata(pdev);
	rfkill_unregister(rfk);
	rfkill_destroy(rfk);   
	return 0;
}

struct platform_device *a1655g_rfkill_plat_device;

static struct platform_driver a1655g_rfkill_driver = {
	.driver = {
		.name = "a1655g-rfkill",
		.owner = THIS_MODULE,
	},
	.probe	= a1655g_rfkill_probe,
	.remove	= __devexit_p(a1655g_rfkill_remove),
};

static int __init a1655g_rfkill_init(void)
{
	int err;
	printk(KERN_INFO "a1655g-rfkill: Registering as a platform device");
	a1655g_rfkill_plat_device = platform_device_register_simple("a1655g-rfkill", 0, NULL, 0);
	if (IS_ERR(a1655g_rfkill_plat_device)) {
		printk(KERN_INFO "a1655gf-rfkill: Error creating platform device");
		return PTR_ERR(a1655g_rfkill_plat_device);
	}
	if ((err = platform_driver_register(&a1655g_rfkill_driver))) {
		printk(KERN_INFO "a1655gf-rfkill: Error registering platform driver");
		platform_device_unregister(a1655g_rfkill_plat_device);
	}
	return 0;
}

static void __exit a1655g_rfkill_exit(void)
{
	platform_driver_unregister(&a1655g_rfkill_driver);
	platform_device_unregister(a1655g_rfkill_plat_device);
}

module_init(a1655g_rfkill_init);
module_exit(a1655g_rfkill_exit);

