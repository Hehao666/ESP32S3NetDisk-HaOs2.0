#ifndef MYFAT_H
#define MYFAT_H

#include <stdint.h>
#include "wear_levelling.h"
extern wl_handle_t s_wl_handle;
#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief 挂载内部 SPI Flash FAT 分区（/storage）
     * @return 0 成功，非 0 失败
     */
    int fat_mount(void);

    /**
     * @brief 卸载内部 FAT 分区
     * @return 0 成功，非 0 失败
     */
    int fat_unmount(void);

    /**
     * @brief 获取内部 FAT 分区的挂载点
     * @return 挂载点字符串，例如 "/storage"
     */
    const char *fat_get_mount_point(void);

    /**
     * @brief 获取指定挂载点的文件系统总容量和已用空间（单位：字节）
     * @param mount_point 挂载点路径（如 "/storage"）
     * @param total_bytes 输出总容量
     * @param used_bytes  输出已用容量
     * @return 0 成功，非 0 失败
     */
    int fat_get_fs_info(const char *mount_point, uint64_t *total_bytes, uint64_t *used_bytes);

#ifdef __cplusplus
}
#endif

#endif /* MYFAT_H */