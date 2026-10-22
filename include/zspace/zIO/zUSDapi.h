#pragma once
#include <array>
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
class ZSPACE_IO USDdoc {
public:
  USDdoc();
  ~USDdoc();
  USDdoc(const USDdoc &) = delete;
  USDdoc &operator=(const USDdoc &) = delete;
  static bool supported();
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
  struct Impl;
  std::unique_ptr<Impl> impl;
};
} // namespace zSpace
