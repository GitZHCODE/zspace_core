#pragma once
#if defined(ZSPACE_IO_OPENUSD)
#include <climits>
#include <cmath>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <zspace/zIO/zUSDapi.h>
namespace USDdocBridge {
static zSpace::USDdoc document;
static zSpace::zUSDMetadata metadata;
static zSpace::zUSDPrimInfo prim;
static zSpace::zUSDAttribute attribute;
static zSpace::zUSDData valueData;
static std::map<std::string, zSpace::zUSDValue> customLayerData;
static std::vector<std::string> customKeys;
static zSpace::zUSDEdits edits;
static bool editing = false;
static std::string error;
template <class F> int query(F f) {
  try {
    error.clear();
    f();
    return 1;
  } catch (const std::exception &e) {
    error = e.what();
    return 0;
  }
}
static void requireEdit() {
  if (!editing)
    throw std::runtime_error("No edit batch.");
}
template <class T> zSpace::zUSDData buffer(const void *p, int count) {
  if (count < 0 || (!p && count))
    throw std::runtime_error("Invalid USD buffer.");
  if (!count)
    return std::vector<T>();
  auto first = static_cast<const T *>(p);
  return std::vector<T>(first, first + count);
}
} // namespace USDdocBridge
extern "C" {
EMSCRIPTEN_KEEPALIVE int USDdoc_open(const char *path) {
  return USDdocBridge::query([&] {
    USDdocBridge::editing = false;
    USDdocBridge::document.open(path ? path : "");
  });
}
EMSCRIPTEN_KEEPALIVE void USDdoc_close() {
  USDdocBridge::document.close();
  USDdocBridge::editing = false;
  USDdocBridge::edits = {};
  USDdocBridge::valueData = false;
}
EMSCRIPTEN_KEEPALIVE int USDdoc_readMetadata() {
  return USDdocBridge::query(
      [] { USDdocBridge::metadata = USDdocBridge::document.readMetadata(); });
}
EMSCRIPTEN_KEEPALIVE double USDdoc_metersPerUnit() {
  return USDdocBridge::metadata.metersPerUnit;
}
EMSCRIPTEN_KEEPALIVE const char *USDdoc_upAxis() {
  return USDdocBridge::metadata.upAxis.c_str();
}
EMSCRIPTEN_KEEPALIVE int USDdoc_primCount() {
  return static_cast<int>(USDdocBridge::metadata.prims.size());
}
EMSCRIPTEN_KEEPALIVE const char *USDdoc_primPath(int i) {
  auto &a = USDdocBridge::metadata.prims;
  return i >= 0 && size_t(i) < a.size() ? a[i].path.c_str() : "";
}
EMSCRIPTEN_KEEPALIVE const char *USDdoc_primType(int i) {
  auto &a = USDdocBridge::metadata.prims;
  return i >= 0 && size_t(i) < a.size() ? a[i].type.c_str() : "";
}
EMSCRIPTEN_KEEPALIVE int USDdoc_readPrim(const char *p) {
  return USDdocBridge::query([&] {
    USDdocBridge::prim = USDdocBridge::document.readPrim(p ? p : "");
  });
}
EMSCRIPTEN_KEEPALIVE const char *USDdoc_type() {
  return USDdocBridge::prim.type.c_str();
}
EMSCRIPTEN_KEEPALIVE int USDdoc_mirrored() {
  return USDdocBridge::prim.mirrored;
}
EMSCRIPTEN_KEEPALIVE int USDdoc_attributeCount() {
  return static_cast<int>(USDdocBridge::prim.attributes.size());
}
EMSCRIPTEN_KEEPALIVE const char *USDdoc_attributeName(int i) {
  auto &a = USDdocBridge::prim.attributes;
  return i >= 0 && size_t(i) < a.size() ? a[i].c_str() : "";
}
EMSCRIPTEN_KEEPALIVE int USDdoc_readBool(const char *p, const char *n) {
  return USDdocBridge::query([&] {
    USDdocBridge::valueData =
        USDdocBridge::document.readBool(p ? p : "", n ? n : "");
  });
}
EMSCRIPTEN_KEEPALIVE int USDdoc_readInt(const char *p, const char *n) {
  return USDdocBridge::query([&] {
    USDdocBridge::valueData = USDdocBridge::document.readInt(p ? p : "", n ? n : "");
  });
}
EMSCRIPTEN_KEEPALIVE int USDdoc_readFloat(const char *p, const char *n) {
  return USDdocBridge::query([&] {
    USDdocBridge::valueData =
        USDdocBridge::document.readFloat(p ? p : "", n ? n : "");
  });
}
EMSCRIPTEN_KEEPALIVE int USDdoc_readDouble(const char *p, const char *n) {
  return USDdocBridge::query([&] {
    USDdocBridge::valueData =
        USDdocBridge::document.readDouble(p ? p : "", n ? n : "");
  });
}
EMSCRIPTEN_KEEPALIVE int USDdoc_readString(const char *p, const char *n) {
  return USDdocBridge::query([&] {
    USDdocBridge::valueData =
        USDdocBridge::document.readString(p ? p : "", n ? n : "");
  });
}
EMSCRIPTEN_KEEPALIVE int USDdoc_readIntArray(const char *p, const char *n) {
  return USDdocBridge::query([&] {
    USDdocBridge::valueData =
        USDdocBridge::document.readIntArray(p ? p : "", n ? n : "");
  });
}
EMSCRIPTEN_KEEPALIVE int USDdoc_readFloatArray(const char *p, const char *n) {
  return USDdocBridge::query([&] {
    USDdocBridge::valueData =
        USDdocBridge::document.readFloatArray(p ? p : "", n ? n : "");
  });
}
EMSCRIPTEN_KEEPALIVE int USDdoc_readDoubleArray(const char *p, const char *n) {
  return USDdocBridge::query([&] {
    USDdocBridge::valueData =
        USDdocBridge::document.readDoubleArray(p ? p : "", n ? n : "");
  });
}
EMSCRIPTEN_KEEPALIVE int USDdoc_readAttribute(const char *p, const char *n) {
  return USDdocBridge::query([&] {
    USDdocBridge::attribute =
        USDdocBridge::document.readAttribute(p ? p : "", n ? n : "");
    USDdocBridge::valueData = std::move(USDdocBridge::attribute.value);
  });
}
EMSCRIPTEN_KEEPALIVE const char *USDdoc_valueType() {
  return USDdocBridge::attribute.type.c_str();
}
EMSCRIPTEN_KEEPALIVE const char *USDdoc_interpolation() {
  return USDdocBridge::attribute.interpolation.c_str();
}
EMSCRIPTEN_KEEPALIVE int USDdoc_custom() {
  return USDdocBridge::attribute.custom;
}
// Kind follows zUSDData's alternatives: scalar
// bool/int/uint/float/double/string, then int/uint/float/double/string arrays.
// Buffers live until the next query.
EMSCRIPTEN_KEEPALIVE int USDdoc_kind() {
  return static_cast<int>(USDdocBridge::valueData.index());
}
EMSCRIPTEN_KEEPALIVE double USDdoc_scalar() {
  return std::visit(
      [](const auto &v) -> double {
        if constexpr (std::is_arithmetic_v<std::decay_t<decltype(v)>>)
          return v;
        else
          return 0;
      },
      USDdocBridge::valueData);
}
EMSCRIPTEN_KEEPALIVE int USDdoc_count() {
  return std::visit(
      [](const auto &v) -> int {
        if constexpr (std::is_arithmetic_v<std::decay_t<decltype(v)>>)
          return 1;
        else
          return static_cast<int>(v.size());
      },
      USDdocBridge::valueData);
}
EMSCRIPTEN_KEEPALIVE const void *USDdoc_buffer() {
  return std::visit(
      [](const auto &v) -> const void * {
        using T = std::decay_t<decltype(v)>;
        if constexpr (std::is_same_v<T, std::vector<int>> ||
                      std::is_same_v<T, std::vector<unsigned int>> ||
                      std::is_same_v<T, std::vector<float>> ||
                      std::is_same_v<T, std::vector<double>>)
          return v.data();
        else
          return nullptr;
      },
      USDdocBridge::valueData);
}
EMSCRIPTEN_KEEPALIVE const char *USDdoc_string(int i) {
  auto &d = USDdocBridge::valueData;
  if (auto p = std::get_if<std::string>(&d))
    return p->c_str();
  if (auto a = std::get_if<std::vector<std::string>>(&d))
    return i >= 0 && size_t(i) < a->size() ? (*a)[i].c_str() : "";
  return "";
}
EMSCRIPTEN_KEEPALIVE int USDdoc_readCustomLayerData() {
  return USDdocBridge::query([] {
    using namespace USDdocBridge;
    customLayerData = document.readCustomLayerData();
    customKeys.clear();
    for (const auto &e : customLayerData)
      customKeys.push_back(e.first);
  });
}
EMSCRIPTEN_KEEPALIVE int USDdoc_customCount() {
  return static_cast<int>(USDdocBridge::customKeys.size());
}
EMSCRIPTEN_KEEPALIVE const char *USDdoc_customKey(int i) {
  auto &a = USDdocBridge::customKeys;
  return i >= 0 && size_t(i) < a.size() ? a[i].c_str() : "";
}
EMSCRIPTEN_KEEPALIVE int USDdoc_customValue(int i) {
  return USDdocBridge::query([&] {
    using namespace USDdocBridge;
    const auto &a = customLayerData.at(customKeys.at(i));
    attribute.type = a.type;
    valueData = a.value;
  });
}
EMSCRIPTEN_KEEPALIVE int USDdoc_readPoints(const char *p) {
  return USDdocBridge::query([&] {
    const auto points = USDdocBridge::document.readPoints(p ? p : "");
    std::vector<double> flat;
    flat.reserve(points.size() * 3);
    for (const auto &point : points)
      flat.insert(flat.end(), point.begin(), point.end());
    USDdocBridge::valueData = std::move(flat);
  });
}
EMSCRIPTEN_KEEPALIVE int USDdoc_beginEdits() {
  return USDdocBridge::query([] {
    USDdocBridge::edits = {};
    USDdocBridge::editing = true;
  });
}
EMSCRIPTEN_KEEPALIVE void USDdoc_cancelEdits() {
  USDdocBridge::edits = {};
  USDdocBridge::editing = false;
}
EMSCRIPTEN_KEEPALIVE int USDdoc_removePrim(const char *p) {
  return USDdocBridge::query([&] {
    USDdocBridge::requireEdit();
    USDdocBridge::edits.remove.emplace_back(p ? p : "");
  });
}
EMSCRIPTEN_KEEPALIVE int USDdoc_definePrim(const char *p, const char *t) {
  return USDdocBridge::query([&] {
    USDdocBridge::requireEdit();
    USDdocBridge::edits.prims.push_back({p ? p : "", t ? t : "", {}});
  });
}
EMSCRIPTEN_KEEPALIVE int USDdoc_setNumeric(int kind, const void *p, int count,
                                           double scalar) {
  return USDdocBridge::query([&] {
    using namespace USDdocBridge;
    if (!std::isfinite(scalar))
      throw std::runtime_error("Non-finite scalar.");
    switch (kind) {
    case 0:
      if (scalar != 0 && scalar != 1)
        throw std::runtime_error("Invalid bool.");
      valueData = bool(scalar);
      break;
    case 1:
      if (std::trunc(scalar) != scalar || scalar < INT_MIN || scalar > INT_MAX)
        throw std::runtime_error("Invalid int.");
      valueData = int(scalar);
      break;
    case 2:
      if (std::trunc(scalar) != scalar || scalar < 0 || scalar > UINT_MAX)
        throw std::runtime_error("Invalid uint.");
      valueData = static_cast<unsigned int>(scalar);
      break;
    case 3:
      valueData = float(scalar);
      break;
    case 4:
      valueData = scalar;
      break;
    case 6:
      valueData = buffer<int>(p, count);
      break;
    case 7:
      valueData = buffer<unsigned int>(p, count);
      break;
    case 8:
      valueData = buffer<float>(p, count);
      break;
    case 9:
      valueData = buffer<double>(p, count);
      break;
    default:
      throw std::runtime_error("Invalid numeric kind.");
    }
  });
}
EMSCRIPTEN_KEEPALIVE int USDdoc_setString(const char *s) {
  return USDdocBridge::query(
      [&] { USDdocBridge::valueData = std::string(s ? s : ""); });
}
EMSCRIPTEN_KEEPALIVE void USDdoc_beginStrings() {
  USDdocBridge::valueData = std::vector<std::string>();
}
EMSCRIPTEN_KEEPALIVE int USDdoc_appendString(const char *s) {
  return USDdocBridge::query([&] {
    std::get<std::vector<std::string>>(USDdocBridge::valueData)
        .emplace_back(s ? s : "");
  });
}
EMSCRIPTEN_KEEPALIVE int USDdoc_setAttribute(const char *name, const char *type,
                                             const char *interpolation,
                                             int custom) {
  return USDdocBridge::query([&] {
    using namespace USDdocBridge;
    requireEdit();
    if (edits.prims.empty())
      throw std::runtime_error("Define a prim first.");
    zSpace::zUSDAttribute a;
    a.type = type ? type : "";
    a.value = std::move(valueData);
    a.interpolation = interpolation ? interpolation : "";
    a.custom = custom != 0;
    edits.prims.back().attributes[name ? name : ""] = std::move(a);
  });
}
EMSCRIPTEN_KEEPALIVE int USDdoc_setCustomLayerData(const char *name,
                                                   const char *type) {
  return USDdocBridge::query([&] {
    using namespace USDdocBridge;
    requireEdit();
    edits.customLayerData[name ? name : ""] = {type ? type : "",
                                               std::move(valueData)};
  });
}
EMSCRIPTEN_KEEPALIVE int USDdoc_removeCustomLayerData(const char *name) {
  return USDdocBridge::query([&] {
    using namespace USDdocBridge;
    requireEdit();
    edits.removeCustomLayerData.emplace_back(name ? name : "");
  });
}
EMSCRIPTEN_KEEPALIVE int USDdoc_commitEdits() {
  return USDdocBridge::query([] {
    using namespace USDdocBridge;
    requireEdit();
    document.applyEdits(edits);
    edits = {};
    editing = false;
  });
}
EMSCRIPTEN_KEEPALIVE int USDdoc_save(const char *p) {
  return USDdocBridge::query([&] { USDdocBridge::document.save(p ? p : ""); });
}
EMSCRIPTEN_KEEPALIVE const char *USDdoc_error() {
  return USDdocBridge::error.c_str();
}
}
#endif
