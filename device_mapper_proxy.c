#include <linux/bio.h>
#include <linux/device-mapper.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>


#define DM_MSG_PREFIX "dmp"

struct dmp_proxy {
        struct dm_dev *dev;
        sector_t start;
};

struct Stats {
	unsigned long long read_count;
	unsigned long long write_count;
	unsigned long long average_read_count;
	unsigned long long average_write_count;
	unsigned long long amount_requests;
	unsigned long long block_average_size;

};

struct Stats stats = {
	.read_count = 0,
        .write_count = 0,
        .average_read_count = 0,
        .average_write_count = 0,
        .amount_requests = 0,
        .block_average_size = 0,
};

static int dmp_map(struct dm_target *ti, struct bio *bio)
{
        struct dm_proxy *dmproxy = (struct dm_proxy) ti->private;
        //printk(KERN_CRIT "\n<<in function basic_target_map \n");

	switch(bio_op(bio)) {
	case REQ_OP_READ:
		stats->read_count++;
		break;
	case REQ_OP_WRITE:
		stats->write_count++;
		//printk(KERN_CRIT "\n basic_target_map : bio is a write request.... \n");
		break;
	}

        stats.amount_requests = stats.read_count + stats.write_count;

        //bio->bi_bdev = mdt->dev->bdev;
	bio_set_dev(bio, ti->dev->bdev)
       	bio_endio(bio);

        //printk(KERN_CRIT "\n>>out function basic_target_map \n");
        return DM_MAPIO_SUBMITTED;
}


static int dmp_ctr(struct dm_target *ti, unsigned int argc, char **argv)
{
        struct dm_proxy dmproxy;
        unsigned long long start;

        if (argc != 2) {
                printk(KERN_CRIT "\n Invalid number of arguments.\n");
                ti->error = "Invalid argument count";
                return -EINVAL;
        }

        //mdt = kmalloc(sizeof(struct my_dm_target), GFP_KERNEL);

        dmproxy->start=(sector_t)start;

        if (dm_get_device(ti, argv[0], dm_table_get_mode(ti->table), &dmproxy->dev)) {
                ti->error = "dm-proxy: Device lookup failed";
                goto bad;
        }

        ti->private = dmproxy->dev;

        //printk(KERN_CRIT "\n>>constructed\n");
        return 0;

  bad:
        //kfree(mdt);
        //printk(KERN_CRIT "\n>>out function basic_target_ctr with errorrrrrrrrrr \n");
        return -EINVAL;
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

module_dm(dmp)

MODULE_LICENSE("GPL");


