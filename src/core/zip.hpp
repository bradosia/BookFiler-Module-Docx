/*
 * @name Bookfiler™ Docx Module
 * @author Branden Lee
 * @version 1.00
 * @license MIT
 * @brief docx manipulation.
 */

#ifndef BOOKFILER_MODULE_DOCX_ZIP_H
#define BOOKFILER_MODULE_DOCX_ZIP_H

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
  ZipFileEntry(mz_zip_file *zipFilePtr);
  ~ZipFileEntry();
  long long accessed_date, creation_date, modified_date;
  std::string compressionName, crypt, filePath;
  int16_t level;
  uint32_t ratio;
};

class ZipFileMap {
public:
  ZipFileMap();
  ~ZipFileMap();
  std::unordered_map<std::string, std::shared_ptr<ZipFileEntry>> getMap();
  bool insert(mz_zip_file *zipFilePtr);

  std::unordered_map<std::string, std::shared_ptr<ZipFileEntry>> m;
};

class ZipReader {
public:
  ZipReader();
  ~ZipReader();
  int32_t open(std::string fileName);
  int32_t extractEntryAll(std::shared_ptr<ZipFileMap> ZipFileEntryMap);
  /* If a path without a directory is used the the error is received:
   * MZ_OPEN_ERROR	-111	Stream open error
   */
  int32_t saveToFile(std::string resourcePath, std::string);
  int32_t getResourceSize(std::string resourcePath, long long &resourceSize);
  int32_t saveToMemory(std::string resourcePath, char *&buffer, long long resourceSize);

private:
  int32_t err = MZ_OK;
  void *reader = nullptr;
};

void zipFileEntryMapPrintAll(std::shared_ptr<ZipFileMap> zipFileMap);

#endif // BOOKFILER_MODULE_DOCX_ZIP_H
