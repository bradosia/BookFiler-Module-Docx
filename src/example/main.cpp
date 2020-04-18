// C++
#include <iomanip>
#include <iostream>
#include <memory>
#include <unordered_map>

/* Minizip 2.7.0
 * License: zlib
 */
#include "mz.h"
#include "mz_os.h"
#include "mz_strm.h"
#include "mz_strm_buf.h"
#include "mz_strm_split.h"
#include "mz_zip.h"
#include "mz_zip_rw.h"

/* ZipFileEntry
 * Extracts additional information from mz_zip_file
 */
class ZipFileEntry : public mz_zip_file {
public:
  ZipFileEntry(mz_zip_file *zipFilePtr) {
    filePath = std::string(zipFilePtr->filename, zipFilePtr->filename_size);
    /* The pointer is invalidated when going to next
     * the data must be copied
     */
    crc = zipFilePtr->crc;
    flag = zipFilePtr->flag;
    zip64 = zipFilePtr->zip64;
    comment = zipFilePtr->comment;
    extrafield = zipFilePtr->extrafield;
    extrafield_size = zipFilePtr->extrafield_size;
    disk_number = zipFilePtr->disk_number;
    disk_offset = zipFilePtr->disk_offset;
    external_fa = zipFilePtr->external_fa;
    internal_fa = zipFilePtr->internal_fa;
    comment_size = zipFilePtr->comment_size;
    accessed_date = zipFilePtr->accessed_date;
    creation_date = zipFilePtr->creation_date;
    modified_date = zipFilePtr->modified_date;
    version_madeby = zipFilePtr->version_madeby;
    version_needed = zipFilePtr->version_needed;
    compressed_size = zipFilePtr->compressed_size;
    uncompressed_size = zipFilePtr->uncompressed_size;
    compression_method = zipFilePtr->compression_method;

    ratio = 0;
    if (zipFilePtr->uncompressed_size > 0)
      ratio = (uint32_t)((zipFilePtr->compressed_size * 100) /
                         zipFilePtr->uncompressed_size);

    /* Display a '*' if the file is encrypted */
    if (zipFilePtr->flag & MZ_ZIP_FLAG_ENCRYPTED)
      crypt = '*';
    else
      crypt = ' ';

    switch (zipFilePtr->compression_method) {
    case MZ_COMPRESS_METHOD_STORE:
      compressionName = "Stored";
      break;
    case MZ_COMPRESS_METHOD_DEFLATE:
      level = (int16_t)((zipFilePtr->flag & 0x6) / 2);
      if (level == 0)
        compressionName = "Defl:N";
      else if (level == 1)
        compressionName = "Defl:X";
      else if ((level == 2) || (level == 3))
        compressionName = "Defl:F"; /* 2: fast , 3: extra fast */
      else
        compressionName = "Defl:?";
      break;
    case MZ_COMPRESS_METHOD_BZIP2:
      compressionName = "BZip2";
      break;
    case MZ_COMPRESS_METHOD_LZMA:
      compressionName = "LZMA";
      break;
    default:
      compressionName = "?";
    }

    mz_zip_time_t_to_tm(zipFilePtr->modified_date, &tmu_date);
    year = tmu_date.tm_year;
    month = tmu_date.tm_mon;
    day = tmu_date.tm_mday;
    hour = tmu_date.tm_hour;
    min = tmu_date.tm_min;
    sec = tmu_date.tm_sec;
    date = std::to_string(year) + "-" + std::to_string(month) + "-" +
           std::to_string(day);
  }
  int year, month, day, hour, min, sec;
  std::string compressionName, crypt, filePath, date;
  int16_t level;
  struct tm tmu_date;
  uint32_t ratio;
};

class ZipFileMap {
public:
  ZipFileMap(){};
  ~ZipFileMap(){};
  std::unordered_map<std::string, std::shared_ptr<ZipFileEntry>> getMap() {
    return m;
  };
  bool insert(mz_zip_file *zipFilePtr) {
    m.insert(
        {zipFilePtr->filename, std::make_shared<ZipFileEntry>(zipFilePtr)});
    return true;
  }
  std::unordered_map<std::string, std::shared_ptr<ZipFileEntry>> m;
};

void printAll(std::shared_ptr<ZipFileMap> zipFileMap) {
  std::cout << std::left << std::setw(10) << "Packed" << std::setw(10)
            << "Unpacked" << std::setw(10) << "Ratio" << std::setw(10)
            << "Method" << std::setw(10) << "Attribs" << std::setw(10) << "Date"
            << std::setw(10) << "Time" << std::setw(10) << "CRC-32"
            << std::setw(20) << "Name" << std::endl;
  for (auto zipFileIt : zipFileMap->getMap()) {
    auto zipFilePtr = zipFileIt.second;
    std::cout << std::left << std::setw(10) << zipFilePtr->compressed_size
              << std::setw(10) << zipFilePtr->uncompressed_size << std::setw(10)
              << zipFilePtr->ratio << std::setw(10)
              << zipFilePtr->compressionName << std::setw(10)
              << zipFilePtr->crypt << std::setw(10) << zipFilePtr->external_fa
              << std::setw(10) << zipFilePtr->date << std::setw(10)
              << zipFilePtr->crc << std::setw(20) << zipFilePtr->filePath
              << std::endl;
  }
}

void zip_save(std::shared_ptr<ZipFileMap> zipFileMap) {
  std::cout << std::left << std::setw(10) << "Packed" << std::setw(10)
            << "Unpacked" << std::setw(10) << "Ratio" << std::setw(10)
            << "Method" << std::setw(10) << "Attribs" << std::setw(10) << "Date"
            << std::setw(10) << "Time" << std::setw(10) << "CRC-32"
            << std::setw(20) << "Name" << std::endl;
  for (auto zipFileIt : zipFileMap->getMap()) {
    auto zipFilePtr = zipFileIt.second;
    std::cout << std::left << std::setw(10) << zipFilePtr->compressed_size
              << std::setw(10) << zipFilePtr->uncompressed_size << std::setw(10)
              << zipFilePtr->ratio << std::setw(10)
              << zipFilePtr->compressionName << std::setw(10)
              << zipFilePtr->crypt << std::setw(10) << zipFilePtr->external_fa
              << std::setw(10) << zipFilePtr->date << std::setw(10)
              << zipFilePtr->crc << std::setw(20) << zipFilePtr->filePath
              << std::endl;
  }
}

class ZipReader {
public:
    ZipReader(){
        mz_zip_reader_create(&reader);
    }
    ~ZipReader(){
       mz_zip_reader_delete(&reader);
    }
    int32_t open(std::string fileName){
        err = mz_zip_reader_open_file(reader, fileName.c_str());
        return err;
    }
    int32_t extractEntryAll(std::shared_ptr<ZipFileMap> ZipFileEntryMap){
        err = mz_zip_reader_goto_first_entry(reader);

        if (err != MZ_OK && err != MZ_END_OF_LIST) {
          std::cout << "Error " << err << ": going to first entry in archive"
                    << std::endl;
          mz_zip_reader_delete(&reader);
          return err;
        }

        mz_zip_file *fileInfo = nullptr;
        while (err == MZ_OK) {
          err = mz_zip_reader_entry_get_info(reader, &fileInfo);
          if (err != MZ_OK) {
            std::cout << "Error " << err << ": getting entry info in archive"
                      << std::endl;
            break;
          }
          ZipFileEntryMap->insert(fileInfo);
          err = mz_zip_reader_goto_next_entry(reader);
          if (err != MZ_OK && err != MZ_END_OF_LIST) {
            std::cout << "Error " << err << ": going to next entry in archive"
                      << std::endl;
            break;
          }
        }

        if (err == MZ_END_OF_LIST)
          return MZ_OK;
        return err;
    }
private:
    int32_t err = MZ_OK;
    void *reader = nullptr;
};


int main(int argc, char *argv[]) {
  std::shared_ptr<ZipFileMap> fileEntryMap = std::make_shared<ZipFileMap>();
  std::shared_ptr<ZipReader > zipReader = std::make_shared<ZipReader>();
  std::string fileName = "resources/test.docx";
  int32_t err = MZ_OK;

  err = zipReader->open(fileName);
  if (err != MZ_OK) {
    std::cout << "Error " << err << ": could not open archive." << fileName
              << std::endl;
  }

  err = zipReader->extractEntryAll(fileEntryMap);
  if (err != MZ_OK) {
    std::cout << "Error " << err << ": could not extract file entries." << fileName
              << std::endl;
  }

  printAll(fileEntryMap);

  system("pause");
  return 0;
}
