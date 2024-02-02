#include <linux/bio.h>
#include <linux/device-mapper.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/module.h>
#include <linux/init.h>


#define DM_MSG_PREFIX "dmp"

struct dm_proxy {
        struct dm_dev *dev;
        sector_t start;
};

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
        .total_requests = 0,
        .block_average_size = 0,
};

static int dmp_map(struct dm_target *ti, struct bio *bio)
{
        struct dm_proxy *dmproxy = (struct dm_proxy *) ti->private;
        //printk(KERN_CRIT "\n<<in function basic_target_map \n");

	switch(bio_op(bio)) {
	case REQ_OP_READ:
		stats.read_count++;
		break;
	case REQ_OP_WRITE:
		stats.write_count++;
		//printk(KERN_CRIT "\n basic_target_map : bio is a write request.... \n");
		break;
	}

        stats.total_requests = stats.read_count + stats.write_count;

        //bio->bi_bdev = mdt->dev->bdev;
	bio_set_dev(bio, dmproxy->dev->bdev);
       	bio_endio(bio);

        //printk(KERN_CRIT "\n>>out function basic_target_map \n");
        return DM_MAPIO_SUBMITTED;
}


static int dmp_ctr(struct dm_target *ti, unsigned int argc, char **argv)
{
        struct dm_proxy *dmproxy;
        unsigned long long start;

        if (argc != 2) {
                printk(KERN_CRIT "\n Invalid number of arguments.\n");
                ti->error = "Invalid argument count";
                return -EINVAL;
        }

        //mdt = kmalloc(sizeof(struct my_dm_target), GFP_KERNEL);

        dmproxy->start = (sector_t)start;

        if (dm_get_device(ti, argv[0], dm_table_get_mode(ti->table), &(dmproxy->dev))) {
                ti->error = "dm-proxy: Device lookup failed";
                return -EINVAL;
        }

        ti->private = dmproxy->dev;

        return 0;
        
}


static void dmp_dtr(struct dm_target *ti)
{
  //struct my_dm_target *mdt = (struct my_dm_target *) ti->private;
  //printk(KERN_CRIT "\n<<in function basic_target_dtr \n");
  struct dm_proxy *dmproxy = (struct dm_proxy *)ti->private;
  dm_put_device(ti, dmproxy->dev);
  //kfree(mdt);
  printk(KERN_CRIT "\n>>destructed\n");
}


static struct target_type dmp_target = {
	.name = "dmp",
	.version = {1,0,0},
	//.features = DM_TARGET_NOWAIT,
	.module = THIS_MODULE,
	.ctr = dmp_ctr,
	.dtr = dmp_dtr,
	.map = dmp_map,
};

struct kobj_attribute dmpstats_attr = __ATTR(dmpstats, 0660, sysfs_show, NULL);

static struct kobject* dmpstats_kobj;

static ssize_t sysfs_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf) 
{
        return sprintf(buf, "\nread\n requests:%lld\n avg size:%lld\n write\n requests:%lld\n avg size:%lld\n total\n requests:%lld\n avg size:%lld\n", 
                stats.read_count, stats.average_read_count, stats.write_count, stats.average_write_count, stats.total_requests, stats.block_average_size);
}

static int __init dmp_init(void) 
{
        struct kobject mod_ko = (((struct module*)(THIS_MODULE))->mkobj).kobj;
	dmpstats_kobj = kobject_create_and_add("stat", &mod_ko);
	if (!dmpstats_kobj)
	{
		DMERR("creating stat directory failed");
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
	        DMERR("register failed %d", r);

	return r;
}

static void __exit dmp_exit(void)
{
        dm_unregister_target(&dmp_target);  
	sysfs_remove_file(dmpstats_kobj, &dmpstats_attr.attr);
	kobject_put(dmpstats_kobj);
  
        DMINFO("dmp is removed from the kernel\n");
        return;
}

module_init(dmp_init)
module_exit(dmp_exit)

MODULE_LICENSE("GPL");


