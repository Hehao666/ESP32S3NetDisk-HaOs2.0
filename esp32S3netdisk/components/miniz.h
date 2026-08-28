#ifndef MINIZ_H
#define MINIZ_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

  // 基础类型定义
  typedef uint32_t mz_uint;
  typedef uint16_t mz_uint16;
  typedef uint32_t mz_uint32;
  typedef uint64_t mz_uint64;
  typedef int32_t mz_int32;
  typedef size_t mz_size_t;

#define MZ_TRUE 1
#define MZ_FALSE 0
#define MZ_MAX(a, b) ((a) > (b) ? (a) : (b))
#define MZ_MIN(a, b) ((a) < (b) ? (a) : (b))

// ZIP 签名常量
#define ZIP_SIG_LOCAL_FILE 0x04034B50U
#define ZIP_SIG_CENTRAL_DIR 0x02014B50U
#define ZIP_SIG_END_CENTRAL_DIR 0x06054B50U

  // ZIP 文件信息结构体
  typedef struct
  {
    char filename[256];         // 文件名
    mz_uint32 uncomp_size;      // 原始大小
    mz_uint32 comp_size;        // 压缩后大小
    mz_uint32 crc32;            // CRC32
    mz_uint16 method;           // 压缩方式(0=存储)
    mz_uint64 local_header_ofs; // 本地文件头偏移
  } mz_zip_file_info;

  // ZIP 操作句柄
  typedef struct mz_zip_archive
  {
    void *file_ptr;              // 文件指针(FILE*)
    mz_uint file_count;          // 文件总数
    mz_zip_file_info *file_list; // 文件列表
  } mz_zip_archive;

  // ===================== 对外接口 =====================
  /**
   * @brief  打开ZIP文件(读模式，用于预览/解压)
   * @param  pZip: 句柄指针
   * @param  path: ZIP文件路径
   * @return 成功返回MZ_TRUE
   */
  int mz_zip_reader_init_file(mz_zip_archive *pZip, const char *path);

  /**
   * @brief  关闭读取句柄，释放资源
   */
  void mz_zip_reader_end(mz_zip_archive *pZip);

  /**
   * @brief  获取ZIP内文件总数
   */
  mz_uint mz_zip_reader_get_num_files(mz_zip_archive *pZip);

  /**
   * @brief  获取指定索引的文件名
   */
  const char *mz_zip_reader_get_filename(mz_zip_archive *pZip, mz_uint idx);

  /**
   * @brief  提取单个文件到堆内存
   * @param  ppBuf: 输出堆内存指针(需手动free)
   * @param  pSize: 输出文件大小
   * @return 成功返回MZ_TRUE
   */
  int mz_zip_reader_extract_to_heap(mz_zip_archive *pZip, mz_uint idx, void **ppBuf, mz_uint32 *pSize);

  /**
   * @brief  提取单个文件到磁盘
   */
  int mz_zip_reader_extract_to_file(mz_zip_archive *pZip, mz_uint idx, const char *dst_path);

  /**
   * @brief  初始化ZIP写入器(创建新压缩包)
   */
  int mz_zip_writer_init_file(mz_zip_archive *pZip, const char *path);

  /**
   * @brief  向压缩包添加单个文件
   * @param  arc_name: 压缩包内文件名
   * @param  src_path: 源文件路径
   * @param  level: 压缩等级(0=仅存储,无压缩)
   */
  int mz_zip_writer_add_file(mz_zip_archive *pZip, const char *arc_name, const char *src_path, int level);

  /**
   * @brief  结束写入，补齐ZIP目录结构
   */
  int mz_zip_writer_finalize_archive(mz_zip_archive *pZip);

  /**
   * @brief  关闭写入句柄
   */
  void mz_zip_writer_end(mz_zip_archive *pZip);

  // ===================== SD卡封装接口 =====================
  /**
   * @brief  预览ZIP，获取内部所有文件列表
   * @param  zip_path: ZIP路径
   * @param  file_list: 输出文件名二维数组
   * @param  max_files: 最大读取文件数
   * @return 实际文件数量，失败返回-1
   */
  int sd_zip_get_file_list(const char *zip_path, char (*file_list)[256], int max_files);

  /**
   * @brief  解压整个ZIP到指定目录
   * @param  zip_path: ZIP路径
   * @param  out_dir: 输出目录
   * @return 0成功，非0失败
   */
  int sd_zip_extract_all(const char *zip_path, const char *out_dir);

  /**
   * @brief  多文件压缩为ZIP
   * @param  zip_path: 输出ZIP路径
   * @param  file_paths: 源文件路径数组
   * @param  file_count: 文件数量
   * @return 0成功，非0失败
   */
  int sd_zip_compress_files(const char *zip_path, const char **file_paths, int file_count);

#ifdef __cplusplus
}
#endif

#endif // MINIZ_H