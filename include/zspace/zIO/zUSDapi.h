#pragma once
#include <array>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <map>
#include <memory>
#include <string>
#include <variant>
#include <vector>
#include <zspace/export.h>

namespace zSpace {
// Numeric vectors are flat XYZ triples. USD type retains point/normal/vector
// role. No OpenUSD headers or JSON are exposed across this boundary.
using zUSDData = std::variant<bool, int, unsigned int, float, double,
                              std::string, std::vector<int>,
                              std::vector<unsigned int>, std::vector<float>,
                              std::vector<double>, std::vector<std::string>>;
struct zUSDValue {
  std::string type;
  zUSDData value = false;
};
struct zUSDAttribute : zUSDValue {
  std::string interpolation;
  bool custom = true;
};
struct zUSDPrimInfo {
  std::string path, type;
  std::vector<std::string> attributes;
  bool mirrored = false;
};
struct zUSDMetadata {
  double metersPerUnit = 0.01;
  std::string upAxis;
  std::vector<zUSDPrimInfo> prims;
};
struct zUSDPrimEdit {
  std::string path, type;
  std::map<std::string, zUSDAttribute> attributes;
};
struct zUSDEdits {
  std::vector<std::string> remove;
  std::vector<zUSDPrimEdit> prims;
  std::map<std::string, zUSDValue> customLayerData;
  std::vector<std::string> removeCustomLayerData;
};
class USDPrim;
class USDAttribute;
class ZSPACE_IO USDdoc {
public:
  USDdoc();
  ~USDdoc();
  USDdoc(const USDdoc &) = delete;
  USDdoc &operator=(const USDdoc &) = delete;
  static bool supported();
  USDPrim getPrimAtPath(const std::string &path);
  USDPrim definePrim(const std::string &path, const std::string &type);
  void removePrim(const std::string &path);
  void setCustomLayerData(const std::map<std::string, zUSDValue> &values);
  std::vector<std::array<double, 3>> readWorldPoints(const std::string &path) const { return readPoints(path); }
  void open(const std::string &path);
  void close();
  zUSDMetadata readMetadata() const;
  zUSDPrimInfo readPrim(const std::string &path) const;
  zUSDAttribute readAttribute(const std::string &path,
                              const std::string &name) const;
  // Missing attribute: tryRead returns false without modifying output.
  // Missing prim, unreadable values and incompatible types throw.
  bool hasAttribute(const std::string &path, const std::string &name) const;
  bool readBool(const std::string &path, const std::string &name) const;
  bool tryReadBool(const std::string &path, const std::string &name,
                   bool &out) const;
  int readInt(const std::string &path, const std::string &name) const;
  bool tryReadInt(const std::string &path, const std::string &name,
                  int &out) const;
  float readFloat(const std::string &path, const std::string &name) const;
  bool tryReadFloat(const std::string &path, const std::string &name,
                    float &out) const;
  double readDouble(const std::string &path, const std::string &name) const;
  bool tryReadDouble(const std::string &path, const std::string &name,
                     double &out) const;
  std::string readString(const std::string &path,
                         const std::string &name) const;
  bool tryReadString(const std::string &path, const std::string &name,
                     std::string &out) const;
  std::vector<int> readIntArray(const std::string &path,
                                const std::string &name) const;
  bool tryReadIntArray(const std::string &path, const std::string &name,
                       std::vector<int> &out) const;
  std::vector<float> readFloatArray(const std::string &path,
                                    const std::string &name) const;
  bool tryReadFloatArray(const std::string &path, const std::string &name,
                         std::vector<float> &out) const;
  std::vector<double> readDoubleArray(const std::string &path,
                                      const std::string &name) const;
  bool tryReadDoubleArray(const std::string &path, const std::string &name,
                          std::vector<double> &out) const;
  // Append typed attributes to a prim in a batch; applyEdits commits the batch.
  static void writeBool(zUSDPrimEdit &prim, const std::string &name,
                        const bool &value,
                        const std::string &interpolation = "",
                        bool custom = true);
  static void writeInt(zUSDPrimEdit &prim, const std::string &name,
                       const int &value, const std::string &interpolation = "",
                       bool custom = true);
  static void writeFloat(zUSDPrimEdit &prim, const std::string &name,
                         const float &value,
                         const std::string &interpolation = "",
                         bool custom = true);
  static void writeDouble(zUSDPrimEdit &prim, const std::string &name,
                          const double &value,
                          const std::string &interpolation = "",
                          bool custom = true);
  static void writeString(zUSDPrimEdit &prim, const std::string &name,
                          const std::string &value,
                          const std::string &interpolation = "",
                          bool custom = true);
  static void writeIntArray(zUSDPrimEdit &prim, const std::string &name,
                            const std::vector<int> &value,
                            const std::string &interpolation = "",
                            bool custom = true);
  static void writeFloatArray(zUSDPrimEdit &prim, const std::string &name,
                              const std::vector<float> &value,
                              const std::string &interpolation = "",
                              bool custom = true);
  static void writeDoubleArray(zUSDPrimEdit &prim, const std::string &name,
                               const std::vector<double> &value,
                               const std::string &interpolation = "",
                               bool custom = true);
  // Flat XYZ triples, retaining USD point/normal role. No coordinate
  // conversion.
  static void writePoints(zUSDPrimEdit &prim, const std::string &name,
                          const std::vector<float> &value,
                          const std::string &interpolation = "",
                          bool custom = true);
  static void writeNormals(zUSDPrimEdit &prim, const std::string &name,
                           const std::vector<float> &value,
                           const std::string &interpolation = "",
                           bool custom = true);
  std::map<std::string, zUSDValue> readCustomLayerData() const;
  // Default-time world-space coordinates, in source units and source up axis.
  std::vector<std::array<double, 3>> readPoints(const std::string &path) const;
  // All edits commit together. Failure leaves the open document unchanged.
  void applyEdits(const zUSDEdits &edits);
  void save(const std::string &path) const;
  std::string text() const;

private:
  friend class USDPrim;
  friend class USDAttribute;
  std::shared_ptr<int> lifetime = std::make_shared<int>(0);
  void createAttribute(const std::string &path, const std::string &name, const std::string &type, bool custom);
  zUSDData getAttributeValue(const std::string &path, const std::string &name) const;
  std::string attributeType(const std::string &path, const std::string &name) const;
  void setAttributeValue(const std::string &path, const std::string &name, const zUSDData &value);
  struct Impl;
  std::unique_ptr<Impl> impl;
};
// Lightweight path handles; close/open or document destruction invalidates them.
class ZSPACE_IO USDAttribute {
public:
  bool isValid() const;
  std::string getTypeName() const;
  zUSDData get() const;
  template<class T> T get() const {
    const auto value = get();
    if (const auto *exact = std::get_if<T>(&value)) return *exact;
    // Preserve supported float/double conversions; no implicit bool/int coercion.
    if constexpr (std::is_same_v<T, float> || std::is_same_v<T, double>) {
      using Other = std::conditional_t<std::is_same_v<T, float>, double, float>;
      if (const auto *v = std::get_if<Other>(&value)) {
        if (!std::isfinite(*v) || std::abs(double(*v)) > std::numeric_limits<T>::max())
          throw std::runtime_error("USD numeric overflow.");
        return static_cast<T>(*v);
      }
    } else if constexpr (std::is_same_v<T, std::vector<float>> || std::is_same_v<T, std::vector<double>>) {
      using E = typename T::value_type;
      using Other = std::conditional_t<std::is_same_v<E, float>, double, float>;
      if (const auto *v = std::get_if<std::vector<Other>>(&value)) {
        T out; out.reserve(v->size());
        for (auto n : *v) {
          if (!std::isfinite(n) || std::abs(double(n)) > std::numeric_limits<E>::max())
            throw std::runtime_error("USD numeric overflow.");
          out.push_back(static_cast<E>(n));
        }
        return out;
      }
    }
    throw std::runtime_error("USD attribute type mismatch.");
  }
  void set(const zUSDData &value);
private:
  friend class USDPrim;
  USDAttribute(USDdoc &doc, std::string path, std::string name);
  USDdoc &document() const;
  USDdoc *owner;
  std::weak_ptr<int> lifetime;
  std::string path, name;
};
class ZSPACE_IO USDPrim {
public:
  bool isValid() const;
  std::string getTypeName() const;
  const std::string &getPath() const { return path; }
  USDAttribute getAttribute(const std::string &name) const;
  USDAttribute createAttribute(const std::string &name, const std::string &type, bool custom = true);
private:
  friend class USDdoc;
  USDPrim(USDdoc &doc, std::string path);
  USDdoc &document() const;
  USDdoc *owner;
  std::weak_ptr<int> lifetime;
  std::string path;
};
} // namespace zSpace
