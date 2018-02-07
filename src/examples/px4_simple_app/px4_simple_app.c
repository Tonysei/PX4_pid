#include <px4_config.h>
#include <px4_tasks.h>
#include <px4_posix.h>
#include <unistd.h>
#include <stdio.h>
#include <poll.h>
#include <string.h>
#include <math.h>

#include <uORB/uORB.h>
#include <uORB/topics/sensor_combined.h>
#include <uORB/topics/vehicle_attitude.h>

__EXPORT int px4_simple_app_main(int argc, char *argv[]);

int px4_simple_app_main(int argc, char *argv[])
{
    PX4_INFO("Hello Sky!");

    /* 订阅 sensor_combined主题*/
    int sensor_sub_fd = orb_subscribe(ORB_ID(sensor_combined));
    orb_set_interval(sensor_sub_fd, 200);//限制更新频率为5 Hz

    /* 1.公告attitude主题 */
    struct vehicle_attitude_s att;
    memset(&att, 0, sizeof(att));
    orb_advert_t att_pub = orb_advertise(ORB_ID(vehicle_attitude), &att);

    /* 可以在此等待多个主题 */
    px4_pollfd_struct_t fds[] =
    {
        { .fd = sensor_sub_fd,   .events = POLLIN },
    };

    int error_counter = 0;

    for (int i = 0; i < 5; i++)
    {
        /* 等待1000ms获取数据 */
        int poll_ret = px4_poll(fds, 1, 1000);

        /* 处理结果*/
        if (poll_ret == 0)  //未得到数据
        {
            PX4_ERR("Got no data within a second");
        }
        else if (poll_ret < 0)  //严重错误
        {
            if (error_counter < 10 || error_counter % 50 == 0)
            {
                /* use a counter to prevent flooding (and slowing us down) */
                PX4_ERR("ERROR return value from poll(): %d", poll_ret);
            }
            error_counter++;
        }
        else    //抓到数据
        {
            if (fds[0].revents & POLLIN)
            {
                /* 复制sensor_combined公告主题 */
                struct sensor_combined_s raw;
                orb_copy(ORB_ID(sensor_combined), sensor_sub_fd, &raw);

                PX4_INFO("Accelerometer:\t%8.4f\t%8.4f\t%8.4f",
                     (double)raw.accelerometer_m_s2[0],
                     (double)raw.accelerometer_m_s2[1],
                     (double)raw.accelerometer_m_s2[2]);

                /* q在.msg中定义*/
                att.q[0] = raw.accelerometer_m_s2[0];
                att.q[1] = raw.accelerometer_m_s2[1];
                att.q[2] = raw.accelerometer_m_s2[2];
                /*2. 发布attitude主题*/
                orb_publish(ORB_ID(vehicle_attitude), att_pub, &att);
            }
            /* if (fds[1..n].revents & POLLIN) {} */
        }
    }

    PX4_INFO("exiting");

    return 0;
}
