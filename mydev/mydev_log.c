#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "mydev"
#define CLASS_NAME  "mydev"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("shusei");
MODULE_DESCRIPTION("mydev: log-only char device");
MODULE_VERSION("0.1");

static dev_t devno;
static struct cdev cdev_mydev;
static struct class *mydev_class;
static struct device *mydev_dev;

static int mydev_open(struct inode *inode, struct file *file)
{
    pr_info("mydev: open\n");
    return 0;
}

static int mydev_release(struct inode *inode, struct file *file)
{
    pr_info("mydev: release\n");
    return 0;
}

static ssize_t mydev_read(struct file *file, char __user *buf, size_t cnt, loff_t *ppos)
{
    /* 何も返さない（EOF） */
    return 0;
}

static ssize_t mydev_write(struct file *file, const char __user *buf, size_t cnt, loff_t *ppos)
{
    char kbuf[256];

    size_t n = (cnt < sizeof(kbuf)-1) ? cnt : (sizeof(kbuf)-1);
    if (copy_from_user(kbuf, buf, n))
        return -EFAULT;
    kbuf[n] = '\0';

    pr_info("mydev: write '%s' (len=%zu)\n", kbuf, cnt);
    return cnt; /* ユーザに「全部書けた」と伝える */
}

static const struct file_operations fops = {
    .owner   = THIS_MODULE,
    .open    = mydev_open,
    .release = mydev_release,
    .read    = mydev_read,
    .write   = mydev_write,
};

static int __init mydev_init(void)
{
    int ret;

    /* 1) 主要/副番号を確保 */
    ret = alloc_chrdev_region(&devno, 0, 1, DEVICE_NAME);
    if (ret) return ret;

    /* 2) cdev を作成して fops を紐づけ */
    cdev_init(&cdev_mydev, &fops);
    cdev_mydev.owner = THIS_MODULE;
    ret = cdev_add(&cdev_mydev, devno, 1);
    if (ret) goto err_unregister;

    /* 3) /dev/mydev を作るための class/device */
    mydev_class = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(mydev_class)) { ret = PTR_ERR(mydev_class); goto err_cdev; }

    mydev_dev = device_create(mydev_class, NULL, devno, NULL, DEVICE_NAME);
    if (IS_ERR(mydev_dev)) { ret = PTR_ERR(mydev_dev); goto err_class; }

    pr_info("mydev: loaded (major=%d)\n", MAJOR(devno));
    return 0;

err_class:
    class_destroy(mydev_class);
err_cdev:
    cdev_del(&cdev_mydev);
err_unregister:
    unregister_chrdev_region(devno, 1);
    return ret;
}

static void __exit mydev_exit(void)
{
    device_destroy(mydev_class, devno);
    class_destroy(mydev_class);
    cdev_del(&cdev_mydev);
    unregister_chrdev_region(devno, 1);
    pr_info("mydev: unloaded\n");
}

module_init(mydev_init);
module_exit(mydev_exit);
