#include "myfat.h"
#include "esp_partition.h"
#include "esp_vfs_fat.h"
#include "wear_levelling.h"
#include <stdio.h>
#include <sys/stat.h>
#include <sys/statvfs.h>


// 分区表里的标签（必须与 partitions.csv 中 Name 列一致）
#define FAT_PARTITION_LABEL "storage"
// 挂载到 VFS 的路径
#define FAT_MOUNT_POINT "/storage"

// 磨损均衡句柄
wl_handle_t s_wl_handle = WL_INVALID_HANDLE;

int fat_mount(void) {
  // 防止重复挂载
  if (s_wl_handle != WL_INVALID_HANDLE) {
    fflush(stdout);
    return 0;
  }

  fflush(stdout);

  // 1. 查找分区
  const esp_partition_t *part = esp_partition_find_first(
      ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_FAT,
      FAT_PARTITION_LABEL);
  if (!part) {
    fflush(stdout);
    return -1;
  }
  fflush(stdout);

  // 2. 挂载配置（首次自动格式化）
  const esp_vfs_fat_mount_config_t mount_config = {
      .max_files = 4,
      .format_if_mount_failed = true,
      .allocation_unit_size = CONFIG_WL_SECTOR_SIZE,
      .use_one_fat = false, // 与官方例程保持一致
  };

  // 3. 挂载（带磨损均衡）
  esp_err_t err = esp_vfs_fat_spiflash_mount_rw_wl(
      FAT_MOUNT_POINT, FAT_PARTITION_LABEL, &mount_config, &s_wl_handle);

  if (err != ESP_OK) {
    fflush(stdout);
    s_wl_handle = WL_INVALID_HANDLE;
    return -2;
  }

  fflush(stdout);
  return 0;
}

int fat_unmount(void) {
  if (s_wl_handle == WL_INVALID_HANDLE) {
    fflush(stdout);
    return 0;
  }

  esp_err_t err =
      esp_vfs_fat_spiflash_unmount_rw_wl(FAT_MOUNT_POINT, s_wl_handle);
  if (err == ESP_OK) {
    s_wl_handle = WL_INVALID_HANDLE;
    fflush(stdout);
    return 0;
  }

  fflush(stdout);
  return -1;
}

const char *fat_get_mount_point(void) { return FAT_MOUNT_POINT; }

int fat_get_fs_info(const char *mount_point, uint64_t *total_bytes,
                    uint64_t *used_bytes) {
  if (!mount_point || !total_bytes || !used_bytes)
    return -1;

  uint64_t total = 0, free = 0;
  esp_err_t err = esp_vfs_fat_info(mount_point, &total, &free);
  if (err != ESP_OK) {
    fflush(stdout);
    return -2;
  }

  *total_bytes = total;
  *used_bytes = total - free; // 已用空间 = 总 - 空闲

  fflush(stdout);
  return 0;
}