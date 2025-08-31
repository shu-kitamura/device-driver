#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>

#define DEVICE_NAME "mydev"
#define CLASS_NAME  "mydev"
#define BUF_SIZE    4096

MODULE_LICENSE("GPL");
MODULE_AUTHOR("shusei");
MODULE_DESCRIPTION("mydev: simple buffer char device");
MODULE_VERSION("0.2");

static dev_t devno;
static struct cdev cdev_mydev;
static struct class *mydev_class;
static struct device *mydev_dev;

static char   data_buf[BUF_SIZE];
static size_t data_len;
static DEFINE_MUTEX(data_lock);

static int mydev_open(struct inode *inode, struct file *file)
{
    return 0;
}

static int mydev_release(struct inode *inode, struct file *file)
{
    return 0;
}

static ssize_t mydev_read(struct file *file, char __user *buf, size_t cnt, loff_t *ppos)
{
    ssize_t n;

    if (mutex_lock_interruptible(&data_lock))
        return -ERESTARTSYS;

    if (*ppos >= data_len) { /* もう読み切った → EOF */
        mutex_unlock(&data_lock);
        return 0;
    }

    n = min_t(size_t, cnt, data_len - *ppos);

    if (copy_to_user(buf, data_buf + *ppos, n)) {
        mutex_unlock(&data_lock);
        return -EFAULT;
    }

    *ppos += n;
    mutex_unlock(&data_lock);
    return n;
}

static ssize_t mydev_write(struct file *file, const char __user *buf, size_t cnt, loff_t *ppos)
{
    size_t n = min_t(size_t, cnt, BUF_SIZE);

    if (mutex_lock_interruptible(&data_lock))
        return -ERESTARTSYS;

    if (copy_from_user(data_buf, buf, n)) {
        mutex_unlock(&data_lock);
        return -EFAULT;
    }

    data_len = n; /* 上書き保存 */
    /* ファイルオフセットは read 時に使う。write では特に進めない */
    mutex_unlock(&data_lock);

    pr_info("mydev: stored %zu bytes\n", n);
    return cnt; /* ユーザ視点は「全部書けた」でOK */
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

    ret = alloc_chrdev_region(&devno, 0, 1, DEVICE_NAME);
    if (ret) return ret;

    cdev_init(&cdev_mydev, &fops);
    cdev_mydev.owner = THIS_MODULE;
    ret = cdev_add(&cdev_mydev, devno, 1);
    if (ret) goto err_unregister;

    mydev_class = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(mydev_class)) { ret = PTR_ERR(mydev_class); goto err_cdev; }

    mydev_dev = device_create(mydev_class, NULL, devno, NULL, DEVICE_NAME);
    if (IS_ERR(mydev_dev)) { ret = PTR_ERR(mydev_dev); goto err_class; }

    mutex_init(&data_lock);
    data_len = 0;

    pr_info("mydev(buf): loaded (major=%d)\n", MAJOR(devno));
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
    pr_info("mydev(buf): unloaded\n");
}

module_init(mydev_init);
module_exit(mydev_exit);
