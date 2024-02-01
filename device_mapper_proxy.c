#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/init.h>
#include <linux/bio.h>
#include <linux/device-mapper.h>



struct my_dm_target {
        struct dm_dev *dev;
        sector_t start;
};


static int basic_target_map(struct dm_target *ti, struct bio *bio)
{
        struct my_dm_target *mdt = (struct my_dm_target *) ti->private;
        printk(KERN_CRIT "\n<<in function basic_target_map \n");

        switch(bio_op(bio)) {
        case REQ_OP_READ:

                break;
        case REQ_OP_WRITE:

                break;
        }

        bio->bi_bdev = mdt->dev->bdev;

        if((bio->bi_opf & WRITE) == WRITE)
                printk(KERN_CRIT "\n basic_target_map : bio is a write request.... \n");
        else
                printk(KERN_CRIT "\n basic_target_map : bio is a read request.... \n");
        submit_bio(bio);


        printk(KERN_CRIT "\n>>out function basic_target_map \n");
        return DM_MAPIO_SUBMITTED;
}


static int basic_target_ctr(struct dm_target *ti, unsigned int argc, char **argv)
{
        struct my_dm_target *mdt;
        unsigned long long start;

        printk(KERN_CRIT "\n >>in function basic_target_ctr \n");

        if (argc != 2) {
                printk(KERN_CRIT "\n Invalid no.of arguments.\n");
                ti->error = "Invalid argument count";
                return -EINVAL;
        }

        mdt = kmalloc(sizeof(struct my_dm_target), GFP_KERNEL);

        if(mdt==NULL)
        {
                printk(KERN_CRIT "\n Mdt is null\n");
                ti->error = "dm-basic_target: Cannot allocate linear context";
                return -ENOMEM;
        }

        if(sscanf(argv[1], "%llu", &start)!=1)
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


static void basic_target_dtr(struct dm_target *ti)
{
  struct my_dm_target *mdt = (struct my_dm_target *) ti->private;
  printk(KERN_CRIT "\n<<in function basic_target_dtr \n");
  dm_put_device(ti, mdt->dev);
  kfree(mdt);
  printk(KERN_CRIT "\n>>out function basic_target_dtr \n");
}


static struct target_type basic_target = {

  .name = "basic_target",
  .version = {1,0,0},
  .module = THIS_MODULE,
  .ctr = basic_target_ctr,
  .dtr = basic_target_dtr,
  .map = basic_target_map,
};


static int init_basic_target(void)
{
  int result;
  result = dm_register_target(&basic_target);
  if(result < 0)
    printk(KERN_CRIT "\n Error in registering target \n");
  return 0;
}


static void cleanup_basic_target(void)
{
  dm_unregister_target(&basic_target);

}

module_init(init_basic_target)
module_exit(cleanup_basic_target)
MODULE_LICENSE("GPL");

