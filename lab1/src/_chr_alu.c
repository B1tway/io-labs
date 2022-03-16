#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt
#include <linux/cdev.h>    /* essential for dev_t */
#include <linux/kernel.h>  /* essential for KERNEL_INFO */
#include <linux/module.h>  /* essential for modules' macros */
#include <linux/proc_fs.h> /* essential for procfs */
#include <linux/slab.h>    /* essential for kmalloc, kfree */
#include <linux/uaccess.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Bitway");
MODULE_DESCRIPTION("lab1");

#define MOD_NAME "chr_alu"

#define COMP_NAME "var2"
#define DEV_NAME "/dev/" COMP_NAME
#define PROC_NAME "/proc/" COMP_NAME
#define KBUF_SIZE (size_t)((2) * PAGE_SIZE)
#define RES_SIZE 1024
static size_t dev_idx = 0;
static size_t proc_idx = 0;

static int results[RES_SIZE];
static size_t result_counter = 0;

static bool dev_create(dev_t *first_dev_id, int major, int minor, u32 count,
                       struct cdev *cdev, const char *name,
                       const struct file_operations *fops);
static bool dev_remove(struct cdev *cdev, dev_t *first_dev_id, u32 count);
static int dev_open(struct inode *ptr_inode, struct file *file);
static ssize_t dev_read(struct file *file, char __user *user_buf, size_t length,
                        loff_t *offset);
static ssize_t dev_write(struct file *file, const char __user *user_buf,
                         size_t length, loff_t *offset);
static int dev_release(struct inode *ptr_inode, struct file *file);

static dev_t dev_first_device_id;
static u32 dev_count = 1;
static int dev_major = 410, dev_minor = 0;
static struct cdev *dev_cdev = NULL;
static const struct file_operations dev_fops = {.owner = THIS_MODULE,
                                                .open = dev_open,
                                                .read = dev_read,
                                                .write = dev_write,
                                                .release = dev_release};

static int proc_open(struct inode *ptr_inode, struct file *file);
static ssize_t proc_read(struct file *file, char __user *user_buf,
                         size_t length, loff_t *offset);
static ssize_t proc_write(struct file *file, const char __user *user_buf,
                          size_t length, loff_t *offset);
static int proc_release(struct inode *ptr_inode, struct file *file);

static struct proc_dir_entry *proc_entry = NULL;
static const struct proc_ops proc_ops = {
    .proc_open = proc_open,
    .proc_read = proc_read,
    .proc_write = proc_write,
    .proc_release = proc_release,
};

static int __init init_chr_comp(void) {

  pr_info("module initing");

  proc_entry =
      proc_create(COMP_NAME, 0444, NULL, &proc_ops); // 0444 -> r--r--r--
  if (proc_entry == NULL)
    return -EINVAL;

  if (dev_create(&dev_first_device_id, dev_major, dev_minor, dev_count,
                 dev_cdev, COMP_NAME, &dev_fops) == false)
    return -EINVAL;

  return 0;
}

static void __exit cleanup_chr_comp(void) {
  pr_info("module cleaning up");

  proc_remove(proc_entry);

  dev_remove(dev_cdev, &dev_first_device_id, dev_count);
}

module_init(init_chr_comp);
module_exit(cleanup_chr_comp);

static int proc_open(struct inode *ptr_inode, struct file *file) {
  pr_info(" file " PROC_NAME " opening\n");
  char *kbuf = kmalloc(KBUF_SIZE, GFP_KERNEL);
  file->private_data = kbuf;
  return 0;
}

static ssize_t proc_read(struct file *file, char __user *user_buf,
                         size_t length, loff_t *offset) {

  char *kbuf = file->private_data;
  pr_info("%d\n", results[result_counter - 1]);
  int count = sprintf(kbuf, "%d\n", results[result_counter - 1]);
  if (kbuf != NULL) {
    pr_info("Data: %s\n", kbuf);
  }
  return simple_read_from_buffer(user_buf, length, offset, kbuf, count);
}

static ssize_t proc_write(struct file *file, const char __user *user_buf,
                          size_t length, loff_t *offset) {
  return -EINVAL;
}

static int proc_release(struct inode *ptr_inode, struct file *file) {
  pr_info("file " PROC_NAME " closed\n");
  char *kbuf = file->private_data;
  if (kbuf) {
    kfree(kbuf);
  }
  file->private_data = NULL;
  return 0;
}

static bool dev_create(dev_t *first_dev_id, int major, int minor, u32 count,
                       struct cdev *cdev, const char *name,
                       const struct file_operations *fops) {
  *first_dev_id = MKDEV(major, minor);
  if (register_chrdev_region(*first_dev_id, count, name)) {
    return false;
  }

  cdev = cdev_alloc();
  if (cdev == NULL) {
    unregister_chrdev_region(*first_dev_id,
                             count); // void can't check for the errors
    return false;
  }

  cdev_init(cdev, fops);
  if (cdev_add(cdev, *first_dev_id, count) == -1) {
    unregister_chrdev_region(*first_dev_id,
                             count); // void can't check for the errors
    cdev_del(cdev);
    return false;
  }

  return true;
}

static bool dev_remove(struct cdev *cdev, dev_t *first_dev_id, u32 count) {
  if (cdev)
    cdev_del(cdev); // void can't check for errors

  unregister_chrdev_region(*first_dev_id,
                           count); // void can't check for the errors
  return true;
}

static int dev_open(struct inode *ptr_inode, struct file *file) {
  pr_info("file " DEV_NAME " opening\n");
  char *kbuf = kmalloc(KBUF_SIZE, GFP_KERNEL);
  file->private_data = kbuf;
  return 0;
}

static ssize_t dev_read(struct file *file, char __user *user_buf, size_t length,
                        loff_t *offset) {
  if (*offset > 0)
    return 0;
  *offset += length;

  return length;
}

inline bool add_result(int a, int b, char op) {
  if (result_counter == RES_SIZE) {
    result_counter = 0;
  }
  int result = 0;
  switch (op) {
  case '+':
    result = a + b;
    break;
  case '-':
    result = a - b;
    break;
  case '*':
    result = a * b;
    break;
  case '/':
    if (b == 0) {
      return false;
    }
    result = a / b;
    break;
  default:
    return false;
  }
  results[result_counter++] = result;
  return true;
}

static ssize_t dev_write(struct file *file, const char __user *user_buf,
                         size_t length, loff_t *offset) {
  char *kbuf = file->private_data;
  int n = length - copy_from_user(kbuf, user_buf, length);
  int a, b;
  char op;
  pr_info("Data: %s", kbuf);
  size_t count = sscanf(kbuf, "%d%c%d", &a, &op, &b);
  printk(KERN_INFO MOD_NAME
         ": dev_write: count = %zu, a = %d, b = %d, op = %c, length = %zu\n",
         n, a, b, op, length);
  add_result(a, b, op);
  return n;
}

static int dev_release(struct inode *ptr_inode, struct file *file) {
  pr_info("file " DEV_NAME " closing\n");

  char *kbuf = file->private_data;
  if (kbuf) {
    kfree(kbuf);
  }
  file->private_data = NULL;

  return 0;
}