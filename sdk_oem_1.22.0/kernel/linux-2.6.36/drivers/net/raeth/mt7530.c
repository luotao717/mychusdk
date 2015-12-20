/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * Copyright (C) 2013 John Crispin <blogic@openwrt.org>
 */

#include <linux/if.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/if_ether.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/netlink.h>
#include <linux/bitops.h>
#include <net/genetlink.h>
#include <linux/switch.h>
#include <linux/delay.h>
#include <linux/phy.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/lockdep.h>
#include <linux/workqueue.h>
#include <linux/of_device.h>

#include "mt7530.h"

#define MT7530_CPU_PORT		6
#define MT7530_NUM_PORTS	7
#define MT7530_NUM_VLANS	16
#define MT7530_NUM_VIDS		16

#define REG_ESW_VLAN_VTCR	0x90
#define REG_ESW_VLAN_VAWD1	0x94
#define REG_ESW_VLAN_VAWD2	0x98

enum {
	/* Global attributes. */
	MT7530_ATTR_ENABLE_VLAN,
};

struct mt7530_port {
	u16	pvid;
};

struct mt7530_vlan {
	u8	ports;
};

struct mt7530_priv {
	void __iomem		*base;
	struct mii_bus		*bus;
	struct switch_dev	swdev;

	bool			global_vlan_enable;
	struct mt7530_vlan	vlans[MT7530_NUM_VLANS];
	struct mt7530_port	ports[MT7530_NUM_PORTS];
};

struct mt7530_mapping {
	char	*name;
	u8	pvids[6];
	u8	vlans[8];
} mt7530_defaults[] = {
	{
		.name = "llllw",
		.pvids = { 1, 1, 1, 1, 2, 1 },
		.vlans = { 0, 0x6f, 0x50 },
	}, {
		.name = "wllll",
		.pvids = { 2, 1, 1, 1, 1, 1 },
		.vlans = { 0, 0x7e, 0x41 },
	},
};

#if 0
struct mt7530_mapping*
mt7530_find_mapping(struct device_node *np)
{
	const char *map;
	int i;

	if (of_property_read_string(np, "ralink,port-map", &map))
		return NULL;

	for (i = 0; i < ARRAY_SIZE(mt7530_defaults); i++)
		if (!strcmp(map, mt7530_defaults[i].name))
			return &mt7530_defaults[i];

	return NULL;
}
#endif

struct mt7530_mapping*
mt7530_find_mapping(void)
{
	/*
	const char *map;
	int i;

	if (of_property_read_string(np, "ralink,port-map", &map))
		return NULL;

	for (i = 0; i < ARRAY_SIZE(mt7530_defaults); i++)
		if (!strcmp(map, mt7530_defaults[i].name))
			return &mt7530_defaults[i];
  */
	return &mt7530_defaults[0];
}

static void
mt7530_apply_mapping(struct mt7530_priv *mt7530, struct mt7530_mapping *map)
{
	int i = 0;

	mt7530->global_vlan_enable = 1;

	for (i = 0; i < 6; i++)
		mt7530->ports[i].pvid = map->pvids[i];
	for (i = 0; i < 8; i++)
		mt7530->vlans[i].ports = map->vlans[i];
}

static int
mt7530_reset_switch(struct switch_dev *dev)
{
	struct mt7530_priv *priv = container_of(dev, struct mt7530_priv, swdev);

	memset(priv->ports, 0, sizeof(priv->ports));
	memset(priv->vlans, 0, sizeof(priv->vlans));

	return 0;
}

static int
mt7530_get_vlan_enable(struct switch_dev *dev,
			   const struct switch_attr *attr,
			   struct switch_val *val)
{
	struct mt7530_priv *priv = container_of(dev, struct mt7530_priv, swdev);

	val->value.i = priv->global_vlan_enable;

	return 0;
}

static int
mt7530_set_vlan_enable(struct switch_dev *dev,
			   const struct switch_attr *attr,
			   struct switch_val *val)
{
	struct mt7530_priv *priv = container_of(dev, struct mt7530_priv, swdev);

	priv->global_vlan_enable = val->value.i != 0;

	return 0;
}

static u32
mt7530_r32(struct mt7530_priv *priv, u32 reg)
{
	if (priv->bus) {
		u16 high, low;
    /*
		mdiobus_write(priv->bus, 0x1f, 0x1f, (reg >> 6) & 0x3ff);
		low = mdiobus_read(priv->bus, 0x1f, (reg >> 2) & 0xf);
		high = mdiobus_read(priv->bus, 0x1f, 0x10);

		return (high << 16) | (low & 0xffff);
		*/
	}

        return ioread32(priv->base + reg);
}

static void
mt7530_w32(struct mt7530_priv *priv, u32 reg, u32 val)
{
	if (priv->bus) {
		/*
		mdiobus_write(priv->bus, 0x1f, 0x1f, (reg >> 6) & 0x3ff);
		mdiobus_write(priv->bus, 0x1f, (reg >> 2) & 0xf,  val & 0xffff);
		mdiobus_write(priv->bus, 0x1f, 0x10, val >> 16);
		return;
		*/
	}

	iowrite32(val, priv->base + reg);
}

static void
mt7530_vtcr(struct mt7530_priv *priv, u32 cmd, u32 val)
{
	int i;

	mt7530_w32(priv, REG_ESW_VLAN_VTCR, BIT(31) | (cmd << 12) | val);

	for (i = 0; i < 20; i++) {
		u32 val = mt7530_r32(priv, REG_ESW_VLAN_VTCR);

		if ((val & BIT(31)) == 0)
			break;

		udelay(1000);
	}
	if (i == 20)
		printk("mt7530: vtcr timeout\n");
}

static int
mt7530_get_port_pvid(struct switch_dev *dev, int port, int *val)
{
	struct mt7530_priv *priv = container_of(dev, struct mt7530_priv, swdev);

  printk("mt7530: mt7530_get_port_pvid\n");
	if (port >= MT7530_NUM_PORTS)
		return -EINVAL;
/*
	*val = mt7530_r32(priv, 0x2014 + (0x100 * port));
	*val &= 0xff;
*/
	return 0;
}

static int
mt7530_set_port_pvid(struct switch_dev *dev, int port, int pvid)
{
	struct mt7530_priv *priv = container_of(dev, struct mt7530_priv, swdev);
    printk("mt7530: mt7530_set_port_pvid, port:%d, pvid:%d\n", port, pvid);
	if (port >= MT7530_NUM_PORTS)
		return -1;

	priv->ports[port].pvid = pvid;
/*
	pvid &= 0xff;
	mt7530_w32(priv, 0x2014 + (0x100 * port), pvid);
*/
	return 0;
}

static int
mt7530_get_vlan_ports(struct switch_dev *dev, struct switch_val *val)
{
	struct mt7530_priv *priv = container_of(dev, struct mt7530_priv, swdev);
	u32 member;
	int i;

    printk("mt7530: mt7530_get_vlan_ports\n");
	val->len = 0;

	if (val->port_vlan < 0 || val->port_vlan >= MT7530_NUM_VIDS)
		return -EINVAL;

	mt7530_vtcr(priv, 0, val->port_vlan);
	member = mt7530_r32(priv, REG_ESW_VLAN_VAWD1);
	member >>= 16;
	member &= 0xff;

	for (i = 0; i < MT7530_NUM_PORTS; i++) {
		struct switch_port *p;
		if (!(member & BIT(i)))
			continue;

		p = &val->value.ports[val->len++];
		p->id = i;
		p->flags = 0;
	}

	return 0;
}

static int
mt7530_set_vlan_ports(struct switch_dev *dev, struct switch_val *val)
{
	struct mt7530_priv *priv = container_of(dev, struct mt7530_priv, swdev);
	int ports = 0;
	int i;

	if (val->port_vlan < 0 || val->port_vlan >= MT7530_NUM_VIDS ||
			val->len > MT7530_NUM_PORTS)
		return -EINVAL;

	for (i = 0; i < val->len; i++) {
		struct switch_port *p = &val->value.ports[i];

		if (p->id >= MT7530_NUM_PORTS)
			return -EINVAL;

		ports |= BIT(p->id);
	}
	priv->vlans[val->port_vlan].ports = ports;

  printk("mt7530: mt7530_set_vlan_ports, val->port_vlan:%d, ports:%d\n", val->port_vlan, ports);
	return 0;
}

static int
mt7530_apply_config(struct switch_dev *dev)
{
	struct mt7530_priv *priv = container_of(dev, struct mt7530_priv, swdev);
	int i;

	if (!priv->global_vlan_enable) {
		mt7530_w32(priv, 0x2004, 0xff000);
		mt7530_w32(priv, 0x2104, 0xff000);
		mt7530_w32(priv, 0x2204, 0xff000);
		mt7530_w32(priv, 0x2304, 0xff000);
		mt7530_w32(priv, 0x2404, 0xff000);
		mt7530_w32(priv, 0x2504, 0xff000);
		mt7530_w32(priv, 0x2604, 0xff000);
		mt7530_w32(priv, 0x2010, 0x810000c);
		mt7530_w32(priv, 0x2110, 0x810000c);
		mt7530_w32(priv, 0x2210, 0x810000c);
		mt7530_w32(priv, 0x2310, 0x810000c);
		mt7530_w32(priv, 0x2410, 0x810000c);
		mt7530_w32(priv, 0x2510, 0x810000c);
		mt7530_w32(priv, 0x2610, 0x810000c);
		return 0;
	}

	// LAN/WAN ports as security mode
	mt7530_w32(priv, 0x2004, 0xff0003);
	mt7530_w32(priv, 0x2104, 0xff0003);
	mt7530_w32(priv, 0x2204, 0xff0003);
	mt7530_w32(priv, 0x2304, 0xff0003);
	mt7530_w32(priv, 0x2404, 0xff0003);
	mt7530_w32(priv, 0x2504, 0xff0003);
	// LAN/WAN ports as transparent port
	mt7530_w32(priv, 0x2010, 0x810000c0);
	mt7530_w32(priv, 0x2110, 0x810000c0);
	mt7530_w32(priv, 0x2210, 0x810000c0);
	mt7530_w32(priv, 0x2310, 0x810000c0);
	mt7530_w32(priv, 0x2410, 0x810000c0);
	mt7530_w32(priv, 0x2510, 0x810000c0);

	// set CPU/P7 port as user port
	mt7530_w32(priv, 0x2610, 0x81000000);
	mt7530_w32(priv, 0x2710, 0x81000000);

	mt7530_w32(priv, 0x2604, 0x20ff0003);
	mt7530_w32(priv, 0x2704, 0x20ff0003);
	mt7530_w32(priv, 0x2610, 0x81000000);

	for (i = 0; i < MT7530_NUM_VLANS; i++) {
		u8 ports = priv->vlans[i].ports;
		u32 val = mt7530_r32(priv, 0x100 + 4 * (i / 2));

		if (i % 2 == 0) {
			val &= 0xfff000;
			val |= i;
		} else {
			val &= 0xfff;
			val |= (i << 12);
		}
		mt7530_w32(priv, 0x100 + 4 * (i / 2), val);

		if (ports)
			mt7530_w32(priv, REG_ESW_VLAN_VAWD1, BIT(30) | (ports << 16) | BIT(0));
		else
			mt7530_w32(priv, REG_ESW_VLAN_VAWD1, 0);

		mt7530_vtcr(priv, 1, i);
	}

	for (i = 0; i < MT7530_NUM_PORTS; i++)
		mt7530_w32(priv, 0x2014 + (0x100 * i), 0x10000 | priv->ports[i].pvid);

	return 0;
}

static int
mt7530_get_port_link(struct switch_dev *dev,  int port,
			 struct switch_port_link *link)
{
	struct mt7530_priv *priv = container_of(dev, struct mt7530_priv, swdev);
	u32 speed, pmsr;

	if (port < 0 || port >= MT7530_NUM_PORTS)
		return -EINVAL;

	pmsr = mt7530_r32(priv, 0x3008 + (0x100 * port));

	link->link = pmsr & 1;
	link->duplex = (pmsr >> 1) & 1;
	speed = (pmsr >> 2) & 3;

	switch (speed) {
	case 0:
		link->speed = SWITCH_PORT_SPEED_10;
		break;
	case 1:
		link->speed = SWITCH_PORT_SPEED_100;
		break;
	case 2:
	case 3: /* forced gige speed can be 2 or 3 */
		link->speed = SWITCH_PORT_SPEED_1000;
		break;
	default:
		link->speed = SWITCH_PORT_SPEED_UNKNOWN;
		break;
	}

	return 0;
}

static const struct switch_attr mt7530_global[] = {
	{
		.type = SWITCH_TYPE_INT,
		.name = "enable_vlan",
		.description = "VLAN mode (1:enabled)",
		.max = 1,
		.id = MT7530_ATTR_ENABLE_VLAN,
		.get = mt7530_get_vlan_enable,
		.set = mt7530_set_vlan_enable,
	},
};

static const struct switch_attr mt7530_port[] = {
};

static const struct switch_attr mt7530_vlan[] = {
};

static const struct switch_dev_ops mt7530_ops = {
	.attr_global = {
		.attr = mt7530_global,
		.n_attr = ARRAY_SIZE(mt7530_global),
	},
	.attr_port = {
		.attr = mt7530_port,
		.n_attr = ARRAY_SIZE(mt7530_port),
	},
	.attr_vlan = {
		.attr = mt7530_vlan,
		.n_attr = ARRAY_SIZE(mt7530_vlan),
	},
	.get_vlan_ports = mt7530_get_vlan_ports,
	.set_vlan_ports = mt7530_set_vlan_ports,
	.get_port_pvid = mt7530_get_port_pvid,
	.set_port_pvid = mt7530_set_port_pvid,
	.get_port_link = mt7530_get_port_link,
	.apply_config = mt7530_apply_config,
	.reset_switch = mt7530_reset_switch,
};

int
mt7530_probe(struct device *dev, void __iomem *base, struct mii_bus *bus)
{
	struct switch_dev *swdev;
	struct mt7530_priv *mt7530;
	struct mt7530_mapping *map;
	int ret;

    printk("$$$$$$ enter:%s, dev:0x%x.\n", __func__, dev);
	if (bus && bus->phy_map[0x1f]->phy_id != 0x1beef)
		return 0;

	//mt7530 = devm_kzalloc(dev, sizeof(struct mt7530_priv), GFP_KERNEL);
	mt7530 = kzalloc(sizeof(struct mt7530_priv), GFP_KERNEL);//Jody
	if (!mt7530)
		return -ENOMEM;

	mt7530->base = base;
	mt7530->bus = bus;
	mt7530->global_vlan_enable = 1;

	swdev = &mt7530->swdev;
	swdev->name = "mt7530";
	swdev->alias = "mt7530";
	swdev->cpu_port = MT7530_CPU_PORT;
	swdev->ports = MT7530_NUM_PORTS;
	swdev->vlans = MT7530_NUM_VLANS;
	swdev->ops = &mt7530_ops;

	ret = register_switch(swdev, NULL);
	if (ret) {
		dev_err(dev, "failed to register mt7530\n");
		return ret;
	}

	printk("loaded mt7530 driver!!\n");

	//map = mt7530_find_mapping(dev->of_node);
	map = mt7530_find_mapping(); //Jody
	if (map)
		mt7530_apply_mapping(mt7530, map);
	mt7530_apply_config(swdev);

	return 0;
}
