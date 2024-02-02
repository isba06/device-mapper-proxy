#include <linux/bio.h>
#include <linux/device-mapper.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/module.h>
#include <linux/init.h>


#define DM_MSG_PREFIX "dmp"

struct statistics {
        unsigned long long read_count;
        unsigned long long write_count;
        unsigned long long average_read_count;
        unsigned long long average_write_count;
	unsigned long long total_requests;
        unsigned long long block_average_size;
};

struct statistics stats = {
        .read_count = 0,
        .write_count = 0,
        .average_read_count = 0,
        .average_write_count = 0,
        .block_average_size = 0,
};

static int dmp_map(struct dm_target *ti, struct bio *bio)
{
        printk(KERN_DEBUG "\ndmp: start map\n");
        struct dm_dev * device = (struct dm_dev *) ti->private;

        bio_set_dev(bio, device->bdev);

        switch(bio_op(bio)) {
        case REQ_OP_READ:
                stats.read_count++;
                break;
        case REQ_OP_WRITE:
                stats.write_count++;
                break;
	default:
		return DM_MAPIO_KILL;
	}
        stats.total_requests = stats.read_count + stats.write_count;

        //bio_endio(bio);
        printk(KERN_DEBUG "\ndmp: last map\n");
        return DM_MAPIO_REMAPPED;
}


static int dmp_ctr(struct dm_target *ti, unsigned int argc, char **argv)
{
        printk(KERN_DEBUG "\ndmp: start constructor\n");
        struct dm_dev * device;

        if (argc != 1) {
                printk(KERN_CRIT "\nInvalid number of arguments.\n");
                ti->error = "Invalid argument count";
                return -EINVAL;
        }

        //dmproxy->start = (sector_t)start;
        device = kmalloc(sizeof(*device), GFP_KERNEL);

        if(device==NULL) {
                ti->error = "ERROR: Cannot allocate";
		printk(KERN_CRIT "\ndm: Cannot allocate\n");
                return -ENOMEM;
        }

        if (dm_get_device(ti, argv[0], dm_table_get_mode(ti->table), &(device))) {
                ti->error = "dmp: Device lookup failed";
                kfree(device);
                return -EINVAL;
        }

        ti->private = device;
        printk(KERN_DEBUG "\ndmp: last constructed\n");
        return 0;
}


static void dmp_dtr(struct dm_target *ti)
{
  struct dm_dev *device= (struct dm_dev *)ti->private;
  dm_put_device(ti, device);
  printk(KERN_DEBUG "\ndmp: destructed\n");
}


static struct target_type dmp_target = {
        .name = "dmp",
        .version = {1,0,0},
        .module = THIS_MODULE,
        .ctr = dmp_ctr,
        .dtr = dmp_dtr,
        .map = dmp_map,
};

static struct kobject* dmpstats_kobj;

static ssize_t sysfs_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
        return sprintf(buf, "\nread\n requests:%lld\n avg size:%lld\nwrite\n requests:%lld\n avg size:%lld\ntotal\n requests:%lld\n avg size:%lld\n",
                stats.read_count, stats.average_read_count, stats.write_count, stats.average_write_count, stats.total_requests, stats.block_average_size);
}

struct kobj_attribute dmpstats_attr = __ATTR(dmpstats, 0660, sysfs_show, NULL);

static int __init dmp_init(void)
{
        printk(KERN_DEBUG "\ndmp: start init\n");
        struct kobject mod_ko = (((struct module*)(THIS_MODULE))->mkobj).kobj;
        dmpstats_kobj = kobject_create_and_add("stat", &mod_ko);
        if (!dmpstats_kobj)
        {
                //DMERR("dmp: creating stat directory failed");
				printk(KERN_CRIT "\ndmp: creating stat directory failed\n");
                return -ENOMEM;
        }
        if (sysfs_create_file(dmpstats_kobj, &dmpstats_attr.attr))
        {
                sysfs_remove_file(dmpstats_kobj, &dmpstats_attr.attr);
                kobject_put(dmpstats_kobj);
                return -1;
        }

        int r = dm_register_target(&dmp_target);
        if (r < 0)
                printk(KERN_CRIT "\ndmp: register failed %d\n", r);
        printk(KERN_DEBUG "\ndmp: init end\n");
        return r;
}

static void __exit dmp_exit(void)
{
        printk(KERN_DEBUG "\ndmp: exit start\n");
        dm_unregister_target(&dmp_target);
        sysfs_remove_file(dmpstats_kobj, &dmpstats_attr.attr);
        kobject_put(dmpstats_kobj);
        DMINFO("dmp is removed from the kernel\n");
        printk(KERN_DEBUG "\ndmp: exit last\n");
        return;
}

module_init(dmp_init)
module_exit(dmp_exit)

MODULE_LICENSE("GPL");
