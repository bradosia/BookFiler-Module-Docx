/*
 * @name Bookfiler™ Docx Module
 * @author Branden Lee
 * @version 1.00
 * @license MIT
 * @brief docx manipulation.
 */

#ifndef BOOKFILER_MODULE_DOCX_H
#define BOOKFILER_MODULE_DOCX_H

// c++17
#include <functional>
#include <memory>
#include <vector>

/* boost 1.72.0
 * License: Boost Software License (similar to BSD and MIT)
 */
#include <boost/signals2.hpp>

/* rapidjson v1.1 (2016-8-25)
 * Developed by Tencent
 * License: MITs
 */
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>

/*
 * bookfiler = BookFiler™
 */
namespace bookfiler {

#ifndef BOOKFILER_PIXMAP_H
#define BOOKFILER_PIXMAP_H
class Pixmap {
public:
  unsigned char *data;
  /* @param width pixel width
   * @param height pixel height
   * @param pixelBytes bytes in a pixel. Usually 8, 24 or 32.
   */
  long width;
  long height;
  long pixelBytes;
};
#endif // end BOOKFILER_PIXMAP_H

class DocxMonitor {
public:
  unsigned long available, total;
};

/* The docx format is dynamic
 * this class should also be dynamic
 * Use PDF for absolute page positioning
 * Can't get number of pages until document is rendered
 */
class Docx {
public:
  /* UTF8 encoded file path
   */
  void openFile(std::string);
  /* Get page count from the meta data the previous renderer stored
   */
  int getInfoPagesTotal();

  std::shared_ptr<Pixmap> getPixmap(int pageNum);
};

using settings_cb_t = std::function<void(std::shared_ptr<rapidjson::Document>)>;
class DocxInterface {
public:
  virtual void init() = 0;
  virtual void registerSettings(
      std::shared_ptr<rapidjson::Document> moduleRequest,
      std::shared_ptr<std::unordered_map<std::string, settings_cb_t>>) = 0;
  virtual void setSettings(std::shared_ptr<rapidjson::Value> data) = 0;
  virtual std::shared_ptr<Docx> newDocx() = 0;
  boost::signals2::signal<void(std::shared_ptr<Pixmap>)> imageUpdateSignal;
};

} // namespace bookfiler

#endif // end BOOKFILER_MODULE_DOCX_H
