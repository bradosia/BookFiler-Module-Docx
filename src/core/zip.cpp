// Local Project
#include "zip.hpp"

ZipFileEntry::ZipFileEntry(mz_zip_file *zipFilePtr) {
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

  // external file attribute (permissions?)
  external_fa = zipFilePtr->external_fa;
  internal_fa = zipFilePtr->internal_fa;
  comment_size = zipFilePtr->comment_size;
  version_madeby = zipFilePtr->version_madeby;
  version_needed = zipFilePtr->version_needed;
  compressed_size = zipFilePtr->compressed_size;
  uncompressed_size = zipFilePtr->uncompressed_size;
  compression_method = zipFilePtr->compression_method;

  // not inherited
  accessed_date = zipFilePtr->accessed_date;
  creation_date = zipFilePtr->creation_date;
  modified_date = zipFilePtr->modified_date;

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
}
ZipFileEntry::~ZipFileEntry() {}

ZipFileMap::ZipFileMap(){};

ZipFileMap::~ZipFileMap(){};

std::unordered_map<std::string, std::shared_ptr<ZipFileEntry>>
ZipFileMap::getMap() {
  return m;
};

bool ZipFileMap::insert(mz_zip_file *zipFilePtr) {
  m.insert({zipFilePtr->filename, std::make_shared<ZipFileEntry>(zipFilePtr)});
  return true;
}

ZipReader::ZipReader() { mz_zip_reader_create(&reader); }

ZipReader::~ZipReader() { mz_zip_reader_delete(&reader); }

int32_t ZipReader::open(std::string fileName) {
  int32_t err = mz_zip_reader_open_file(reader, fileName.c_str());
  return err;
}

int32_t
ZipReader::extractEntryAll(std::shared_ptr<ZipFileMap> ZipFileEntryMap) {
  int32_t err = mz_zip_reader_goto_first_entry(reader);

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

int32_t ZipReader::saveToFile(std::string resourcePath, std::string outName) {
  int32_t err = MZ_OK;
  err = mz_zip_reader_locate_entry(reader, resourcePath.c_str(), 0);
  if (err != MZ_OK) {
    return err;
  }
  err = mz_zip_reader_entry_save_file(reader, outName.c_str());
  if (err != MZ_OK) {
    return err;
  }
  return err;
}
int32_t ZipReader::getResourceSize(std::string resourcePath,
                                   long long &resourceSize) {
  int32_t err = MZ_OK;
  // find resource by path
  err = mz_zip_reader_locate_entry(reader, resourcePath.c_str(), 0);
  if (err != MZ_OK) {
    return err;
  }
  // save resource
  resourceSize = mz_zip_reader_entry_save_buffer_length(reader);
  return err;
}
int32_t ZipReader::saveToMemory(std::string resourcePath, char *&buffer,
                                long long resourceSize) {
  int32_t err = MZ_OK;
  // find resource by path
  err = mz_zip_reader_locate_entry(reader, resourcePath.c_str(), 0);
  if (err != MZ_OK) {
    return err;
  }
  // save resource
  err = mz_zip_reader_entry_save_buffer(reader, (void *)buffer, resourceSize);
  if (err != MZ_OK) {
    return err;
  }
  return err;
}

void zipFileEntryMapPrintAll(std::shared_ptr<ZipFileMap> zipFileMap) {
  std::cout << std::left << std::setw(10) << "Packed" << std::setw(10)
            << "Unpacked" << std::setw(10) << "Ratio" << std::setw(10)
            << "Method" << std::setw(2) << "C" << std::setw(10) << "Perms"
            << std::setw(20) << "Date Time" << std::setw(10) << "CRC-32"
            << std::setw(20) << "Name" << std::endl;
  for (auto zipFileIt : zipFileMap->getMap()) {
    auto zipFilePtr = zipFileIt.second;
    // std::time_t t = zipFilePtr->modified_date;
    std::cout << std::left << std::dec << std::setw(10)
              << zipFilePtr->compressed_size << std::setw(10)
              << zipFilePtr->uncompressed_size << std::setw(10)
              << zipFilePtr->ratio << std::setw(10)
              << zipFilePtr->compressionName << std::setw(2)
              << zipFilePtr->crypt << std::setw(10) << zipFilePtr->external_fa
              << std::setw(20)
              << std::put_time(std::localtime(&zipFilePtr->modified_date),
                               "%Y-%m-%d %H:%M:%S ")
              << std::setw(10) << std::hex << zipFilePtr->crc << std::setw(20)
              << zipFilePtr->filePath << std::endl;
  }
}
