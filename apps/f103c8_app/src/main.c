
#include <sys/printk.h>
#include <zephyr.h>

static K_SEM_DEFINE(my_sem, 0, 1);

void main(void)
{
    while (1)
    {
        printk("Hello World! %s\n", CONFIG_BOARD);
        k_sleep(K_MSEC(10));
    }
}

void test_thread1(void *p1, void *p2, void *p3)
{
    k_sem_take(&my_sem, K_FOREVER);
    printk("Hello World! %s\n", CONFIG_BOARD);
}

void test_thread2(void *p1, void *p2, void *p3)
{
    k_sleep(K_SECONDS(1));
    k_sem_give(&my_sem);
}

K_THREAD_DEFINE(test_tid1, 1024, test_thread1, NULL, NULL, NULL, CONFIG_APPLICATION_INIT_PRIORITY, 0, 0);
K_THREAD_DEFINE(test_tid2, 1024, test_thread2, NULL, NULL, NULL, CONFIG_APPLICATION_INIT_PRIORITY, 0, 0);