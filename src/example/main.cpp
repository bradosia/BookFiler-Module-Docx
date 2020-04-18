// C++
#include <iomanip>
#include <iostream>
#include <memory>
#include <unordered_map>

// Local Project
#include "../core/zip.hpp"

int main(int argc, char *argv[]) {
  std::shared_ptr<ZipFileMap> fileEntryMap = std::make_shared<ZipFileMap>();
  std::shared_ptr<ZipReader> zipReader = std::make_shared<ZipReader>();
  std::string fileName = "resources/test.docx";
  std::string imagePath = "word/media/image1.png";
  std::string outPath = "a.png";
  std::string documentPath = "word/document.xml";
  int32_t err = MZ_OK;

  err = zipReader->open(fileName);
  if (err != MZ_OK) {
    std::cout << "Error " << err << ": could not open archive." << fileName
              << std::endl;
    return 1;
  }

  err = zipReader->extractEntryAll(fileEntryMap);
  if (err != MZ_OK) {
    std::cout << "Error " << err << ": could not extract file entries."
              << fileName << std::endl;
  }

  zipFileEntryMapPrintAll(fileEntryMap);

  err = zipReader->saveToFile(imagePath, outPath);
  if (err != MZ_OK) {
    std::cout << "Error " << err << ": could not save to file." << std::endl;
  }

  long long resourceSize = 0;
  err = zipReader->getResourceSize(imagePath, resourceSize);
  if (err != MZ_OK) {
    std::cout << "Error " << err << ": could not get resource size."
              << std::endl;
  }

  char *buffer = new char[resourceSize];
  err = zipReader->saveToMemory(imagePath, buffer, resourceSize);
  if (err != MZ_OK) {
    std::cout << "Error " << err << ": could not save to memory." << std::endl;
  }

  std::cout << std::string(buffer, resourceSize).substr(0, 50) << std::endl;

  system("pause");
  return 0;
}
