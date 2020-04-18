/*
 * @name Filesystem Database Module
 * @author Branden Lee
 * @version 1.00
 * @license GNU LGPL v3
 * @brief filesystem database and utilities
 */

#ifndef BOOKFILER_MODULE_PDF_EXPORT_H
#define BOOKFILER_MODULE_PDF_EXPORT_H

// config
#include "core/config.hpp"

// c++17
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <unordered_map>

/* boost 1.72.0
 * License: Boost Software License (similar to BSD and MIT)
 */
#include <boost/config.hpp> // for BOOST_SYMBOL_EXPORT

/* rapidjson v1.1 (2016-8-25)
 * Developed by Tencent
 * License: MITs
 */
#include <rapidjson/document.h>
#include <rapidjson/filewritestream.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/reader.h> // rapidjson::ParseResult
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

// Local Project
#include "Interface.hpp"

/*
 * bookfiler = BookFiler™
 */
namespace bookfiler {

/*
 * This widget uses the MVC design pattern.
 * This is the controller, the view is a QT widget, and the model is the API
 * storage
 */
class ModuleExport : public PdfInterface {
private:
  std::vector<std::shared_ptr<Pdf>> pdfList;

public:
  ModuleExport();
  ~ModuleExport(){};

  void init() { printf("Filesystem Database Module: init()\n"); }
  void registerSettings(
      std::shared_ptr<rapidjson::Document> moduleRequest,
      std::shared_ptr<std::unordered_map<
          std::string,
          std::function<void(std::shared_ptr<rapidjson::Document>)>>>
          moduleCallbackMap);
  void setSettings(std::shared_ptr<rapidjson::Value> data);
  std::shared_ptr<Pdf> newPdf();
};

// Exporting `my_namespace::module` variable with alias name `module`
// (Has the same effect as `BOOST_DLL_ALIAS(my_namespace::module, module)`)
extern "C" BOOST_SYMBOL_EXPORT ModuleExport bookfilerPdfModule;
ModuleExport bookfilerPdfModule;

} // namespace bookfiler

#endif
// end BOOKFILER_MODULE_PDF_EXPORT_H
