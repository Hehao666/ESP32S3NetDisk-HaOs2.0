#include "miniz.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

// 字节序转换(ESP32小端)
static mz_uint16 le16(const uint8_t *p)
{
    return (mz_uint16)p[0] | ((mz_uint16)p[1] << 8);
}

static mz_uint32 le32(const uint8_t *p)
{
    return (mz_uint32)p[0] | ((mz_uint32)p[1] << 8) | ((mz_uint32)p[2] << 16) | ((mz_uint32)p[3] << 24);
}

// 简易CRC32(标准ZIP校验)
__attribute__((unused))
static mz_uint32 mz_crc32(const void *data, mz_uint32 len)
{
    static const mz_uint32 crc_table[256] = {
        0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA, 0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3,
        0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988, 0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91,
        0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE, 0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,
        0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC, 0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5,
        0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172, 0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B,
        0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940, 0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
        0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116, 0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F,
        0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924, 0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D,
        0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A, 0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
        0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818, 0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01,
        0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E, 0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457,
        0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA, 0xFCB9887C, 0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
        0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2, 0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB,
        0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0, 0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9,
        0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086, 0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
        0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4, 0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD,
        0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A, 0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683,
        0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8, 0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
        0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE, 0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7,
        0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC, 0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5,
        0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252, 0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
        0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60, 0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79,
        0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236, 0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F,
        0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04, 0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,
        0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A, 0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713,
        0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38, 0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21,
        0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E, 0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
        0x88085AA6, 0xFF0F6A30, 0x66063B8A, 0x11010B1C, 0x8F659EBB, 0xF862AE2D, 0x616BFF97, 0x166CCF01,
        0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2, 0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB,
        0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0, 0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
        0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6, 0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF,
        0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94, 0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D};
    mz_uint32 crc = 0xFFFFFFFFU;
    const uint8_t *p = (const uint8_t *)data;
    while (len--)
        crc = (crc >> 8) ^ crc_table[(crc & 0xFF) ^ *p++];
    return crc ^ 0xFFFFFFFFU;
}

// ===================== ZIP 读取实现（预览/解压） =====================
int mz_zip_reader_init_file(mz_zip_archive *pZip, const char *path)
{
    if (!pZip || !path)
        return MZ_FALSE;
    memset(pZip, 0, sizeof(mz_zip_archive));

    FILE *f = fopen(path, "rb");
    if (!f)
        return MZ_FALSE;
    pZip->file_ptr = f;

    // 查找结束中央目录标记
    fseek(f, 0, SEEK_END);
    long file_len = ftell(f);
    long scan_pos = MZ_MAX(0, file_len - 65536);
    fseek(f, scan_pos, SEEK_SET);

    uint8_t buf[22];
    mz_uint total_files = 0;
    mz_uint64 cd_offset = 0;

    while (ftell(f) <= file_len - 22)
    {
        if (fread(buf, 1, 4, f) != 4)
            break;
        if (le32(buf) == ZIP_SIG_END_CENTRAL_DIR)
        {
            fread(buf + 4, 1, 18, f);
            total_files = le16(buf + 8);
            cd_offset = le32(buf + 16);
            break;
        }
        fseek(f, -3, SEEK_CUR);
    }

    if (total_files == 0 || cd_offset >= (mz_uint64)file_len)
    {
        fclose(f);
        return MZ_FALSE;
    }

    // 分配文件列表内存
    pZip->file_list = (mz_zip_file_info *)malloc(total_files * sizeof(mz_zip_file_info));
    if (!pZip->file_list)
    {
        fclose(f);
        return MZ_FALSE;
    }
    pZip->file_count = total_files;

    // 读取中央目录
    fseek(f, (long)cd_offset, SEEK_SET);
    for (mz_uint i = 0; i < total_files; i++)
    {
        uint8_t dir_hdr[46];
        if (fread(dir_hdr, 1, 46, f) != 46 || le32(dir_hdr) != ZIP_SIG_CENTRAL_DIR)
            break;

        mz_uint16 fname_len = le16(dir_hdr + 28);
        mz_uint16 extra_len = le16(dir_hdr + 30);
        mz_uint16 comment_len = le16(dir_hdr + 32);

        mz_zip_file_info *info = &pZip->file_list[i];
        info->method = le16(dir_hdr + 10);
        info->crc32 = le32(dir_hdr + 16);
        info->comp_size = le32(dir_hdr + 20);
        info->uncomp_size = le32(dir_hdr + 24);
        info->local_header_ofs = le32(dir_hdr + 42);

        // 读取文件名
        memset(info->filename, 0, 256);
        fread(info->filename, 1, MZ_MIN(fname_len, 255), f);
        fseek(f, extra_len + comment_len, SEEK_CUR);
    }
    return MZ_TRUE;
}

void mz_zip_reader_end(mz_zip_archive *pZip)
{
    if (!pZip)
        return;
    if (pZip->file_ptr)
        fclose((FILE *)pZip->file_ptr);
    if (pZip->file_list)
        free(pZip->file_list);
    memset(pZip, 0, sizeof(mz_zip_archive));
}

mz_uint mz_zip_reader_get_num_files(mz_zip_archive *pZip)
{
    return pZip ? pZip->file_count : 0;
}

const char *mz_zip_reader_get_filename(mz_zip_archive *pZip, mz_uint idx)
{
    if (!pZip || idx >= pZip->file_count)
        return NULL;
    return pZip->file_list[idx].filename;
}

int mz_zip_reader_extract_to_heap(mz_zip_archive *pZip, mz_uint idx, void **ppBuf, mz_uint32 *pSize)
{
    if (!pZip || !ppBuf || !pSize || idx >= pZip->file_count)
        return MZ_FALSE;
    FILE *f = (FILE *)pZip->file_ptr;
    mz_zip_file_info *info = &pZip->file_list[idx];

    *pSize = info->uncomp_size;
    *ppBuf = malloc(info->uncomp_size);
    if (!*ppBuf)
        return MZ_FALSE;

    // 定位到本地文件头
    fseek(f, (long)info->local_header_ofs, SEEK_SET);
    uint8_t local_hdr[30];
    fread(local_hdr, 1, 30, f);
    mz_uint16 fname_len = le16(local_hdr + 26);
    mz_uint16 extra_len = le16(local_hdr + 28);
    fseek(f, fname_len + extra_len, SEEK_CUR);

    // 读取文件数据(仅存储模式)
    if (fread(*ppBuf, 1, info->comp_size, f) != info->comp_size)
    {
        free(*ppBuf);
        *ppBuf = NULL;
        return MZ_FALSE;
    }
    return MZ_TRUE;
}

int mz_zip_reader_extract_to_file(mz_zip_archive *pZip, mz_uint idx, const char *dst_path)
{
    if (!pZip || !dst_path || idx >= pZip->file_count)
        return MZ_FALSE;
    FILE *f = (FILE *)pZip->file_ptr;
    mz_zip_file_info *info = &pZip->file_list[idx];

    // 仅支持 STORE 模式
    if (info->method != 0)
        return MZ_FALSE;

    // 定位到本地文件头
    fseek(f, (long)info->local_header_ofs, SEEK_SET);
    uint8_t local_hdr[30];
    if (fread(local_hdr, 1, 30, f) != 30)
        return MZ_FALSE;
    mz_uint16 fname_len = le16(local_hdr + 26);
    mz_uint16 extra_len = le16(local_hdr + 28);
    fseek(f, fname_len + extra_len, SEEK_CUR);

    // 流式写入目标文件，每次 16K
    FILE *out = fopen(dst_path, "wb");
    if (!out)
        return MZ_FALSE;

    uint8_t *buf = (uint8_t *)malloc(16384);
    if (!buf)
    {
        fclose(out);
        return MZ_FALSE;
    }

    mz_uint32 remaining = info->comp_size;
    while (remaining > 0)
    {
        mz_uint32 to_read = (remaining < 16384) ? remaining : 16384;
        mz_uint32 got = (mz_uint32)fread(buf, 1, to_read, f);
        if (got == 0)
        {
            free(buf);
            fclose(out);
            return MZ_FALSE;
        }
        fwrite(buf, 1, got, out);
        remaining -= got;
    }
    free(buf);
    fclose(out);
    return MZ_TRUE;
}

// ===================== ZIP 写入实现（压缩） =====================
int mz_zip_writer_init_file(mz_zip_archive *pZip, const char *path)
{
    if (!pZip || !path)
        return MZ_FALSE;
    memset(pZip, 0, sizeof(mz_zip_archive));
    FILE *f = fopen(path, "wb");
    if (!f)
        return MZ_FALSE;
    pZip->file_ptr = f;
    return MZ_TRUE;
}

int mz_zip_writer_add_file(mz_zip_archive *pZip, const char *arc_name, const char *src_path, int level)
{
    (void)level; // 强制忽略 level，永远只用 STORE
    FILE *f = (FILE *)pZip->file_ptr;
    FILE *in = fopen(src_path, "rb");
    if (!f || !in || !arc_name) {
        if (in)
            fclose(in);
        return MZ_FALSE;
    }

    // 1. 流式读取文件大小 + 计算 CRC32（每次 16K，不分配整文件缓冲）
    fseek(in, 0, SEEK_END);
    mz_uint32 file_size = (mz_uint32)ftell(in);
    fseek(in, 0, SEEK_SET);

    uint8_t *buf = (uint8_t *)malloc(16384);
    if (!buf)
    {
        fclose(in);
        return MZ_FALSE;
    }
    mz_uint32 crc = 0xFFFFFFFFU;
    mz_uint32 remaining = file_size;
    static const mz_uint32 crc_table[256] = {
        0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA, 0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3,
        0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988, 0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91,
        0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE, 0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,
        0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC, 0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5,
        0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172, 0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B,
        0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940, 0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
        0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116, 0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F,
        0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924, 0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D,
        0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A, 0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
        0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818, 0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01,
        0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E, 0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457,
        0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA, 0xFCB9887C, 0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
        0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2, 0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB,
        0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0, 0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9,
        0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086, 0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
        0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4, 0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD,
        0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A, 0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683,
        0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8, 0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
        0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE, 0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7,
        0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC, 0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5,
        0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252, 0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
        0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60, 0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79,
        0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236, 0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F,
        0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04, 0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,
        0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A, 0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713,
        0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38, 0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21,
        0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E, 0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
        0x88085AA6, 0xFF0F6A30, 0x66063B8A, 0x11010B1C, 0x8F659EBB, 0xF862AE2D, 0x616BFF97, 0x166CCF01,
        0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2, 0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB,
        0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0, 0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
        0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6, 0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF,
        0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94, 0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D};
    while (remaining > 0)
    {
        mz_uint32 to_read = (remaining < 16384) ? remaining : 16384;
        mz_uint32 got = (mz_uint32)fread(buf, 1, to_read, in);
        if (got == 0)
        {
            free(buf);
            fclose(in);
            return MZ_FALSE;
        }
        for (mz_uint32 i = 0; i < got; i++)
            crc = (crc >> 8) ^ crc_table[(crc & 0xFF) ^ buf[i]];
        remaining -= got;
    }
    crc ^= 0xFFFFFFFFU;

    // 2. 写本地头（先回到文件头偏移记录）
    mz_uint16 fname_len = (mz_uint16)strlen(arc_name);
    mz_uint64 local_hdr_ofs = ftell(f);

    // ==============================================
    // 严格按 ZIP 规范写：METHOD=0（STORE）
    // ==============================================
    uint8_t local_hdr[30] = {0};
    local_hdr[0] = 0x50; local_hdr[1] = 0x4B; local_hdr[2] = 0x03; local_hdr[3] = 0x04; // PK0304
    local_hdr[4] = 10; // 版本 1.0
    local_hdr[8] = 0; local_hdr[9] = 0; // 压缩方法 0=STORE
    local_hdr[14] = (crc >> 0) & 0xFF;
    local_hdr[15] = (crc >> 8) & 0xFF;
    local_hdr[16] = (crc >> 16) & 0xFF;
    local_hdr[17] = (crc >> 24) & 0xFF;
    local_hdr[18] = (file_size >> 0) & 0xFF;
    local_hdr[19] = (file_size >> 8) & 0xFF;
    local_hdr[20] = (file_size >> 16) & 0xFF;
    local_hdr[21] = (file_size >> 24) & 0xFF;
    local_hdr[22] = (file_size >> 0) & 0xFF;
    local_hdr[23] = (file_size >> 8) & 0xFF;
    local_hdr[24] = (file_size >> 16) & 0xFF;
    local_hdr[25] = (file_size >> 24) & 0xFF;
    local_hdr[26] = (fname_len >> 0) & 0xFF;
    local_hdr[27] = (fname_len >> 8) & 0xFF;

    fwrite(local_hdr, 1, 30, f);
    fwrite(arc_name, 1, fname_len, f);

    // 3. 第二遍：流式写文件数据（每次 16K，不再分配整文件缓冲）
    fseek(in, 0, SEEK_SET);
    remaining = file_size;
    while (remaining > 0)
    {
        mz_uint32 to_read = (remaining < 16384) ? remaining : 16384;
        mz_uint32 got = (mz_uint32)fread(buf, 1, to_read, in);
        if (got == 0)
        {
            free(buf);
            fclose(in);
            return MZ_FALSE;
        }
        fwrite(buf, 1, got, f);
        remaining -= got;
    }
    free(buf);
    fclose(in);

    // 4. 加到目录列表（给 finalize 写中央目录用）
    mz_uint new_cnt = pZip->file_count + 1;
    mz_zip_file_info *new_list = (mz_zip_file_info *)realloc(pZip->file_list, new_cnt * sizeof(mz_zip_file_info));
    if (!new_list)
        return MZ_FALSE;
    pZip->file_list = new_list;

    mz_zip_file_info *new_info = &pZip->file_list[pZip->file_count];
    strncpy(new_info->filename, arc_name, 255);
    new_info->crc32 = crc;
    new_info->comp_size = file_size;
    new_info->uncomp_size = file_size;
    new_info->method = 0; // STORE
    new_info->local_header_ofs = local_hdr_ofs;
    pZip->file_count = new_cnt;

    return MZ_TRUE;
}

int mz_zip_writer_finalize_archive(mz_zip_archive *pZip)
{
    FILE *f = (FILE *)pZip->file_ptr;
    mz_uint64 cd_start = ftell(f);

    // 写入中央目录
    for (mz_uint i = 0; i < pZip->file_count; i++)
    {
        mz_zip_file_info *info = &pZip->file_list[i];
        mz_uint16 fname_len = (mz_uint16)strlen(info->filename);
        uint8_t dir_hdr[46] = {0};

        dir_hdr[0] = 0x50;
        dir_hdr[1] = 0x4B;
        dir_hdr[2] = 0x01;
        dir_hdr[3] = 0x02;
        dir_hdr[10] = info->method & 0xFF;
        dir_hdr[16] = (info->crc32 >> 0) & 0xFF;
        dir_hdr[17] = (info->crc32 >> 8) & 0xFF;
        dir_hdr[18] = (info->crc32 >> 16) & 0xFF;
        dir_hdr[19] = (info->crc32 >> 24) & 0xFF;
        dir_hdr[20] = (info->comp_size >> 0) & 0xFF;
        dir_hdr[21] = (info->comp_size >> 8) & 0xFF;
        dir_hdr[22] = (info->comp_size >> 16) & 0xFF;
        dir_hdr[23] = (info->comp_size >> 24) & 0xFF;
        dir_hdr[24] = (info->uncomp_size >> 0) & 0xFF;
        dir_hdr[25] = (info->uncomp_size >> 8) & 0xFF;
        dir_hdr[26] = (info->uncomp_size >> 16) & 0xFF;
        dir_hdr[27] = (info->uncomp_size >> 24) & 0xFF;
        dir_hdr[28] = (fname_len >> 0) & 0xFF;
        dir_hdr[29] = (fname_len >> 8) & 0xFF;
        dir_hdr[42] = (info->local_header_ofs >> 0) & 0xFF;
        dir_hdr[43] = (info->local_header_ofs >> 8) & 0xFF;
        dir_hdr[44] = (info->local_header_ofs >> 16) & 0xFF;
        dir_hdr[45] = (info->local_header_ofs >> 24) & 0xFF;

        fwrite(dir_hdr, 1, 46, f);
        fwrite(info->filename, 1, fname_len, f);
    }

    // 写入结束中央目录标记
    mz_uint64 cd_end = ftell(f);
    uint8_t eocd[22] = {0};
    eocd[0] = 0x50;
    eocd[1] = 0x4B;
    eocd[2] = 0x05;
    eocd[3] = 0x06;
    eocd[8] = (pZip->file_count >> 0) & 0xFF;
    eocd[9] = (pZip->file_count >> 8) & 0xFF;
    eocd[10] = eocd[8];
    eocd[11] = eocd[9];
    mz_uint32 cd_size = (mz_uint32)(cd_end - cd_start);
    eocd[12] = (cd_size >> 0) & 0xFF;
    eocd[13] = (cd_size >> 8) & 0xFF;
    eocd[14] = (cd_size >> 16) & 0xFF;
    eocd[15] = (cd_size >> 24) & 0xFF;
    eocd[16] = (cd_start >> 0) & 0xFF;
    eocd[17] = (cd_start >> 8) & 0xFF;
    eocd[18] = (cd_start >> 16) & 0xFF;
    eocd[19] = (cd_start >> 24) & 0xFF;
    fwrite(eocd, 1, 22, f);

    return MZ_TRUE;
}

void mz_zip_writer_end(mz_zip_archive *pZip)
{
    if (!pZip)
        return;
    if (pZip->file_ptr)
        fclose((FILE *)pZip->file_ptr);
    if (pZip->file_list)
        free(pZip->file_list);
    memset(pZip, 0, sizeof(mz_zip_archive));
}

// ===================== SD 卡封装接口 =====================
int sd_zip_get_file_list(const char *zip_path, char (*file_list)[256], int max_files)
{
    mz_zip_archive zip;
    if (!mz_zip_reader_init_file(&zip, zip_path))
        return -1;

    mz_uint cnt = zip.file_count;
    mz_uint read_cnt = MZ_MIN(cnt, (mz_uint)max_files);
    for (mz_uint i = 0; i < read_cnt; i++)
    {
        strncpy(file_list[i], zip.file_list[i].filename, 255);
        file_list[i][255] = '\0';
    }
    mz_zip_reader_end(&zip);
    return (int)read_cnt;
}

int sd_zip_extract_all(const char *zip_path, const char *out_dir)
{
    mz_zip_archive zip;
    if (!mz_zip_reader_init_file(&zip, zip_path))
        return -1;

    mz_uint total = zip.file_count;
    for (mz_uint i = 0; i < total; i++)
    {
        char dst_path[512] = {0};
        const char *filename = zip.file_list[i].filename;
        snprintf(dst_path, sizeof(dst_path), "%s/%s", out_dir, filename);

        // ==============================================
        // 🔥 只加这段：自动创建文件所在的目录（严格按你的写法）
        // ==============================================
        char path_buf[513];
        strncpy(path_buf, dst_path, sizeof(path_buf) - 1);
        char *last_slash = strrchr(path_buf, '/');
        if (last_slash)
        {
            *last_slash = '\0';
            mkdir(path_buf, 0755); // 你已经包含 <sys/stat.h>
        }

        mz_zip_reader_extract_to_file(&zip, i, dst_path);
    }

    mz_zip_reader_end(&zip);
    return 0;
}

int sd_zip_compress_files(const char *zip_path, const char **file_paths, int file_count)
{
    if (file_count <= 0 || !file_paths)
        return -1;

    mz_zip_archive zip;
    if (!mz_zip_writer_init_file(&zip, zip_path))
        return -1;

    for (int i = 0; i < file_count; i++)
    {
        const char *full_path = file_paths[i]; // ✅ 完整路径，用来读文件
        const char *zip_in_path = full_path;   // ✅ ZIP 内部显示路径

        // 1. 先去掉 /sdcard/ 或 /fat/
        if (strncmp(zip_in_path, "/sdcard/", 8) == 0)
        {
            zip_in_path += 8;
        }
        else if (strncmp(zip_in_path, "/fat/", 5) == 0)
        {
            zip_in_path += 5;
        }

        // 2. 找到【第一个 /】，只保留后面的内容（去掉当前目录）
        const char *first_slash = strchr(zip_in_path, '/');
        if (first_slash)
        {
            zip_in_path = first_slash + 1; // ✅ 保留后面所有目录结构
        }
        mz_zip_writer_add_file(&zip, zip_in_path, full_path, 0);
    }

    mz_zip_writer_finalize_archive(&zip);
    mz_zip_writer_end(&zip);
    return 0;
}