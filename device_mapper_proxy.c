#include <linux/bio.h>
#include <linux/device-mapper.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>


#define DM_MSG_PREFIX "basictarget"

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

static int basictarget_map(struct dm_target *ti, struct bio *bio)
{
        struct Stats *stats = (struct Stats *) ti->private;

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

        //bio->bi_bdev = mdt->dev->bdev;
	bio_set_dev(bio, ti->dev->bdev)
       	bio_endio(bio);

        printk(KERN_CRIT "\n>>out function basic_target_map \n");
        return DM_MAPIO_SUBMITTED;
}


static int basictarget_ctr(struct dm_target *ti, unsigned int argc, char **argv)
{
        struct dm_proxy dmproxy;
        unsigned long long start;

        printk(KERN_CRIT "\n >>in function basic_target_ctr \n");

        if (argc != 2) {
                printk(KERN_CRIT "\n Invalid no.of arguments.\n");
                ti->error = "Invalid argument count";
                return -EINVAL;
        }

        //mdt = kmalloc(sizeof(struct my_dm_target), GFP_KERNEL);

        if(mdt==NULL)
        {
                printk(KERN_CRIT "\n Mdt is null\n");
                ti->error = "dm-basic_target: Cannot allocate linear context";
                return -ENOMEM;
        }

        if(sscanf(argv[1], "%llu", &start) != 1)
        {
                ti->error = "dm-basic_target: Invalid device sector";
                goto bad;
        }

        mdt->start=(sector_t)start;

        if (dm_get_device(ti, argv[0], dm_table_get_mode(ti->table), &mdt->dev)) {
                ti->error = "dm-basic_target: Device lookup failed";
                goto bad;
        }

        ti->private = mdt;


        printk(KERN_CRIT "\n>>out function basic_target_ctr \n");
        return 0;

  bad:
        kfree(mdt);
        printk(KERN_CRIT "\n>>out function basic_target_ctr with errorrrrrrrrrr \n");
        return -EINVAL;
}


static void basictarget_dtr(struct dm_target *ti)
{
  struct my_dm_target *mdt = (struct my_dm_target *) ti->private;
  printk(KERN_CRIT "\n<<in function basic_target_dtr \n");
  dm_put_device(ti, mdt->dev);
  kfree(mdt);
  printk(KERN_CRIT "\n>>out function basic_target_dtr \n");
}


static struct target_type basictarget_target = {
	.name = "basic_target",
	.version = {1,0,0},
	.features = DM_TARGET_NOWAIT,
	.module = THIS_MODULE,
	.ctr = basictarget_ctr,
	.dtr = basictarget_dtr,
	.map = basictarget_map,
};

module_dm(basictarget)

MODULE_LICENSE("GPL");


