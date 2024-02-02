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
        unsigned long long avg_read_blck;
        unsigned long long avg_write_blck;
        unsigned long long total_requests;
        unsigned long long avg_size_blck;
};

static struct statistics stats = {
        .read_count = 0,
        .write_count = 0,
        .avg_read_blck = 0,
        .avg_write_blck = 0,
        .avg_size_blck = 0,
};

static int dmp_map(struct dm_target *ti, struct bio *bio)
{
        struct dm_dev * device = (struct dm_dev *) ti->private;
        bio_set_dev(bio, device->bdev);

	printk(KERN_DEBUG "bio_size: %u\n", bio->bi_iter.bi_size);

        switch(bio_op(bio)) {
        case REQ_OP_READ:
                ++stats.read_count;

                if(stats.avg_read_blck == 0)
                        stats.avg_read_blck = bio->bi_iter.bi_size;
                else
                        stats.avg_read_blck = (stats.avg_read_blck * stats.read_count + bio->bi_iter.bi_size) / (stats.read_count + 1);
                break;

        case REQ_OP_WRITE:
                ++stats.write_count;
		if(stats.avg_write_blck == 0)
                    	stats.avg_write_blck = bio->bi_iter.bi_size;
                else
                    	stats.avg_write_blck = (stats.avg_write_blck * stats.write_count + bio->bi_iter.bi_size) / (stats.write_count + 1);
                break;

        default:
                return DM_MAPIO_KILL;
        }

        stats.total_requests = stats.read_count + stats.write_count;
	stats.avg_size_blck = (stats.read_count * stats.avg_read_blck + stats.write_count * stats.avg_write_blck) / (stats.read_count + stats.write_count);
        printk(KERN_DEBUG "\nread: %llu\nwrite: %llu\naverage read: %llu\naverage write: %llu\naverage block: %llu\ntotal: %llu\n",
                        stats.read_count, stats.write_count, stats.avg_read_blck, stats.avg_write_blck, stats.avg_size_blck, stats.total_requests);
        return DM_MAPIO_REMAPPED;
}

static int dmp_ctr(struct dm_target *ti, unsigned int argc, char **argv)
{
        struct dm_dev * device;

        if (argc != 1) {
                printk(KERN_CRIT "\nInvalid number of arguments.\n");
                ti->error = "Invalid argument count";
                return -EINVAL;
        }

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

        return 0;
}

static void dmp_dtr(struct dm_target *ti)
{
  struct dm_dev *device= (struct dm_dev *)ti->private;
  dm_put_device(ti, device);
 }


static struct target_type dmp_target = {
        .name = "dmp",
        .version = {2,0,0},
        .module = THIS_MODULE,
        .ctr = dmp_ctr,
        .dtr = dmp_dtr,
        .map = dmp_map,
};

static ssize_t sysfs_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
        return sprintf(buf, "\nread\n requests:%lld\n avg size:%lld\nwrite\n requests:%lld\n avg size:%lld\ntotal\n requests:%lld\n avg size:%lld\n",
                stats.read_count, stats.avg_read_blck, stats.write_count, stats.avg_write_blck, stats.total_requests, stats.avg_size_blck);
}

static struct kobject* dmpstats_kobj;
struct kobj_attribute dmpstats_attr = __ATTR(volumes, 0660, sysfs_show, NULL);

static int __init dmp_init(void)
{
        struct kobject mod_ko = (((struct module*)(THIS_MODULE))->mkobj).kobj;
        dmpstats_kobj = kobject_create_and_add("stat", &mod_ko);
        if (!dmpstats_kobj) {
                printk(KERN_CRIT "\ndmp: creating stat directory failed\n");
                return -ENOMEM;
        }
        if (sysfs_create_file(dmpstats_kobj, &dmpstats_attr.attr)) {
                sysfs_remove_file(dmpstats_kobj, &dmpstats_attr.attr);
                kobject_put(dmpstats_kobj);
                return -1;
        }

        int r = dm_register_target(&dmp_target);

        if (r < 0)
                printk(KERN_CRIT "\ndmp: register failed %d\n", r);

        return r;
}

static void __exit dmp_exit(void)
{
        dm_unregister_target(&dmp_target);
        sysfs_remove_file(dmpstats_kobj, &dmpstats_attr.attr);
        kobject_put(dmpstats_kobj);
        return;
}

module_init(dmp_init)
module_exit(dmp_exit)

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ismail Bayramov");
