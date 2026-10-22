#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <zspace/zIO/zUSDapi.h>
#if defined(ZSPACE_IO_OPENUSD)
#include <pxr/usd/usd/primRange.h>
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usdGeom/metrics.h>
#include <pxr/usd/usdGeom/pointBased.h>
#include <pxr/usd/usdGeom/primvarsAPI.h>
#include <pxr/usd/usdGeom/xformCache.h>

#include <pxr/usd/ar/resolver.h>
#include <pxr/usd/sdf/layer.h>
#include <pxr/usd/sdf/schema.h>
namespace zSpace {
PXR_NAMESPACE_USING_DIRECTIVE

struct USDdoc::Impl {
  UsdStageRefPtr stage;
};
namespace {
UsdPrim getPrim(const UsdStageRefPtr &stage, const std::string &path) {
  if (!stage)
    throw std::runtime_error("No USD document.");
  SdfPath p(path);
  if (!p.IsAbsolutePath() || !p.IsPrimPath())
    throw std::runtime_error("Expected an absolute prim path.");
  auto prim = stage->GetPrimAtPath(p);
  if (!prim)
    throw std::runtime_error("USD prim not found: " + path);
  return prim;
}
template <class T> bool extract(const VtValue &v, zUSDData &out) {
  if (!v.IsHolding<T>())
    return false;
  if constexpr (std::is_arithmetic_v<T>)
    if (!std::isfinite(static_cast<double>(v.UncheckedGet<T>())))
      throw std::runtime_error("Non-finite USD value.");
  out = v.UncheckedGet<T>();
  return true;
}
template <class T> bool extractArray(const VtValue &v, zUSDData &out) {
  if (!v.IsHolding<VtArray<T>>())
    return false;
  const auto &a = v.UncheckedGet<VtArray<T>>();
  if constexpr (std::is_arithmetic_v<T>)
    for (auto n : a)
      if (!std::isfinite(static_cast<double>(n)))
        throw std::runtime_error("Non-finite USD value.");
  out = std::vector<T>(a.begin(), a.end());
  return true;
}
template <class V, class T>
bool extractVectors(const VtValue &v, zUSDData &out) {
  if (!v.IsHolding<VtArray<V>>())
    return false;
  std::vector<T> a;
  a.reserve(v.UncheckedGet<VtArray<V>>().size() * 3);
  for (const auto &p : v.UncheckedGet<VtArray<V>>())
    for (int i = 0; i < 3; ++i) {
      if (!std::isfinite(static_cast<double>(p[i])))
        throw std::runtime_error("Non-finite USD value.");
      a.push_back(p[i]);
    }
  out = std::move(a);
  return true;
}
zUSDData decode(const VtValue &v) {
  zUSDData out;
  if (v.IsHolding<TfToken>())
    return v.UncheckedGet<TfToken>().GetString();
  if (v.IsHolding<VtArray<TfToken>>()) {
    std::vector<std::string> a;
    for (const auto &t : v.UncheckedGet<VtArray<TfToken>>())
      a.push_back(t.GetString());
    return a;
  }
  if (extract<bool>(v, out) || extract<int>(v, out) ||
      extract<unsigned int>(v, out) || extract<float>(v, out) ||
      extract<double>(v, out) || extract<std::string>(v, out) ||
      extractArray<int>(v, out) || extractArray<unsigned int>(v, out) ||
      extractArray<float>(v, out) || extractArray<double>(v, out) ||
      extractArray<std::string>(v, out) ||
      extractVectors<GfVec3f, float>(v, out) ||
      extractVectors<GfVec3d, double>(v, out))
    return out;
  throw std::runtime_error("Unsupported USD value type: " + v.GetTypeName());
}
template <class T> void validate(T n) {
  if (!std::isfinite(static_cast<double>(n)))
    throw std::runtime_error("Non-finite USD value.");
}
template <class T> VtValue encodeScalar(const zUSDValue &a) {
  const auto n = std::get<T>(a.value);
  validate(n);
  return VtValue(n);
}
template <class T> VtValue encodeArray(const zUSDValue &a) {
  const auto &data = std::get<std::vector<T>>(a.value);
  VtArray<T> out;
  out.reserve(data.size());
  for (const auto n : data) {
    validate(n);
    out.push_back(n);
  }
  return VtValue(out);
}
template <class V, class T> VtValue encodeVectors(const zUSDValue &a) {
  const auto &data = std::get<std::vector<T>>(a.value);
  if (data.size() % 3)
    throw std::runtime_error("Vector array requires flat triples.");
  VtArray<V> out;
  out.reserve(data.size() / 3);
  for (size_t i = 0; i < data.size(); i += 3) {
    for (int k = 0; k < 3; ++k)
      validate(data[i + k]);
    out.push_back(V(data[i], data[i + 1], data[i + 2]));
  }
  return VtValue(out);
}
VtValue typed(const zUSDValue &a) {
  const auto &t = a.type;
  if (t == "bool")
    return VtValue(std::get<bool>(a.value));
  if (t == "int")
    return encodeScalar<int>(a);
  if (t == "uint")
    return encodeScalar<unsigned int>(a);
  if (t == "float")
    return encodeScalar<float>(a);
  if (t == "double")
    return encodeScalar<double>(a);
  if (t == "string")
    return VtValue(std::get<std::string>(a.value));
  if (t == "token")
    return VtValue(TfToken(std::get<std::string>(a.value)));
  if (t == "int[]")
    return encodeArray<int>(a);
  if (t == "uint[]")
    return encodeArray<unsigned int>(a);
  if (t == "float[]")
    return encodeArray<float>(a);
  if (t == "double[]")
    return encodeArray<double>(a);
  if (t == "point3f[]" || t == "normal3f[]" || t == "vector3f[]" ||
      t == "float3[]")
    return encodeVectors<GfVec3f, float>(a);
  if (t == "point3d[]" || t == "normal3d[]" || t == "vector3d[]" ||
      t == "double3[]")
    return encodeVectors<GfVec3d, double>(a);
  if (t == "string[]") {
    const auto &v = std::get<std::vector<std::string>>(a.value);
    return VtValue(VtArray<std::string>(v.begin(), v.end()));
  }
  if (t == "token[]") {
    VtArray<TfToken> v;
    for (const auto &s : std::get<std::vector<std::string>>(a.value))
      v.push_back(TfToken(s));
    return VtValue(v);
  }
  throw std::runtime_error("Unsupported USD authoring type: " + t);
}
} // namespace
USDdoc::USDdoc() : impl(new Impl) {}
USDdoc::~USDdoc() = default;
bool USDdoc::supported() { return true; }
void USDdoc::open(const std::string &path) {
  auto layer = SdfLayer::OpenAsAnonymous(path);
  auto stage =
      layer ? UsdStage::Open(layer,
                             ArGetResolver().CreateDefaultContextForAsset(path))
            : UsdStageRefPtr();
  if (!stage)
    throw std::runtime_error("Cannot open USD document: " + path);
  impl->stage = stage;
}
void USDdoc::close() { impl->stage.Reset(); }
zUSDMetadata USDdoc::readMetadata() const {
  auto stage = impl->stage;
  if (!stage)
    throw std::runtime_error("No USD document.");
  zUSDMetadata out;
  out.metersPerUnit = UsdGeomGetStageMetersPerUnit(stage);
  out.upAxis = UsdGeomGetStageUpAxis(stage).GetString();
  for (const auto &p : stage->Traverse()) {
    zUSDPrimInfo info;
    info.path = p.GetPath().GetString();
    info.type = p.GetTypeName().GetString();
    out.prims.push_back(std::move(info));
  }
  return out;
}
zUSDPrimInfo USDdoc::readPrim(const std::string &path) const {
  auto p = getPrim(impl->stage, path);
  zUSDPrimInfo out;
  out.path = path;
  out.type = p.GetTypeName().GetString();
  for (const auto &a : p.GetAttributes())
    out.attributes.push_back(a.GetName().GetString());
  UsdGeomXformCache cache;
  out.mirrored = cache.GetLocalToWorldTransform(p).GetDeterminant() < 0;
  return out;
}
zUSDAttribute USDdoc::readAttribute(const std::string &path,
                                    const std::string &name) const {
  auto a = getPrim(impl->stage, path).GetAttribute(TfToken(name));
  VtValue v;
  UsdGeomPrimvar pv(a);
  if (!(pv ? pv.ComputeFlattened(&v) : (a && a.Get(&v))))
    throw std::runtime_error("Missing or unreadable USD attribute: " + name);
  zUSDAttribute out;
  out.type = a.GetTypeName().GetAsToken().GetString();
  out.value = decode(v);
  out.interpolation = pv ? pv.GetInterpolation().GetString() : "";
  out.custom = a.IsCustom();
  return out;
}
std::map<std::string, zUSDValue> USDdoc::readCustomLayerData() const {
  if (!impl->stage)
    throw std::runtime_error("No USD document.");
  std::map<std::string, zUSDValue> out;
  for (const auto &entry : impl->stage->GetRootLayer()->GetCustomLayerData()) {
    auto type = SdfSchema::GetInstance().FindType(entry.second);
    if (!type)
      throw std::runtime_error("Unsupported customLayerData type: " +
                               entry.first);
    out[entry.first] = {type.GetAsToken().GetString(), decode(entry.second)};
  }
  return out;
}
std::vector<std::array<double, 3>>
USDdoc::readPoints(const std::string &path) const {
  auto p = getPrim(impl->stage, path);
  UsdGeomPointBased geometry(p);
  VtArray<GfVec3f> values;
  if (!geometry || !geometry.GetPointsAttr().Get(&values))
    throw std::runtime_error("Prim has no readable points.");
  std::vector<std::array<double, 3>> points;
  points.reserve(values.size());
  UsdGeomXformCache cache;
  const auto transform = cache.GetLocalToWorldTransform(p);
  for (const auto &point : values) {
    const auto v = transform.Transform(GfVec3d(point));
    for (int k = 0; k < 3; ++k)
      if (!std::isfinite(v[k]))
        throw std::runtime_error("Non-finite USD point.");
    points.push_back({v[0], v[1], v[2]});
  }
  return points;
}
void USDdoc::applyEdits(const zUSDEdits &edits) {
  if (!impl->stage)
    throw std::runtime_error("No USD document.");
  auto layer = SdfLayer::CreateAnonymous();
  layer->TransferContent(impl->stage->GetRootLayer());
  auto stage = UsdStage::Open(layer, impl->stage->GetPathResolverContext());
  if (!stage)
    throw std::runtime_error("Cannot create USD edit transaction.");
  for (const auto &name : edits.remove) {
    SdfPath path(name);
    if (!path.IsAbsolutePath() || !path.IsPrimPath())
      throw std::runtime_error("Expected absolute prim path.");
    if (stage->GetPrimAtPath(path) &&
        (!stage->RemovePrim(path) || stage->GetPrimAtPath(path)))
      throw std::runtime_error("Cannot remove composed prim: " + name);
  }
  for (const auto &spec : edits.prims) {
    SdfPath path(spec.path);
    if (!path.IsAbsolutePath() || !path.IsPrimPath())
      throw std::runtime_error("Expected absolute prim path.");
    auto p = stage->DefinePrim(path, TfToken(spec.type));
    if (!p)
      throw std::runtime_error("Cannot define USD prim.");
    for (const auto &entry : spec.attributes) {
      const auto &a = entry.second;
      auto type = SdfSchema::GetInstance().FindType(TfToken(a.type));
      if (!type)
        throw std::runtime_error("Unknown USD attribute type: " + a.type);
      auto attr = p.CreateAttribute(TfToken(entry.first), type, a.custom);
      if (!attr.Set(typed(a)))
        throw std::runtime_error("Cannot author USD attribute: " + entry.first);
      if (!a.interpolation.empty() &&
          !attr.SetMetadata(TfToken("interpolation"),
                            VtValue(TfToken(a.interpolation))))
        throw std::runtime_error("Cannot set interpolation.");
    }
  }
  auto metadata = layer->GetCustomLayerData();
  for (const auto &key : edits.removeCustomLayerData)
    metadata.erase(key);
  for (const auto &entry : edits.customLayerData)
    metadata[entry.first] = typed(entry.second);
  layer->SetCustomLayerData(metadata);
  impl->stage = stage;
}
void USDdoc::save(const std::string &path) const {
  if (!impl->stage || !impl->stage->GetRootLayer()->Export(path))
    throw std::runtime_error("Cannot export USD document: " + path);
}
std::string USDdoc::text() const {
  std::string out;
  if (!impl->stage || !impl->stage->GetRootLayer()->ExportToString(&out))
    throw std::runtime_error("Cannot serialize USD document.");
  return out;
}
} // namespace zSpace
#else
namespace zSpace {
struct USDdoc::Impl {};
USDdoc::USDdoc() : impl(new Impl) {}
USDdoc::~USDdoc() = default;
bool USDdoc::supported() { return false; }
void USDdoc::close() {}
#define USD_DISABLED                                                           \
  throw std::runtime_error(                                                    \
      "Core USD support is disabled; enable ZSPACE_IO_WITH_OPENUSD.")
void USDdoc::open(const std::string &) { USD_DISABLED; }
zUSDMetadata USDdoc::readMetadata() const { USD_DISABLED; }
zUSDPrimInfo USDdoc::readPrim(const std::string &) const { USD_DISABLED; }
zUSDAttribute USDdoc::readAttribute(const std::string &,
                                    const std::string &) const {
  USD_DISABLED;
}
std::map<std::string, zUSDValue> USDdoc::readCustomLayerData() const {
  USD_DISABLED;
}
std::vector<std::array<double, 3>>
USDdoc::readPoints(const std::string &) const {
  USD_DISABLED;
}
void USDdoc::applyEdits(const zUSDEdits &) { USD_DISABLED; }
void USDdoc::save(const std::string &) const { USD_DISABLED; }
std::string USDdoc::text() const { USD_DISABLED; }
#undef USD_DISABLED
} // namespace zSpace
#endif

// Backend-independent typed interface. Backend-disabled operations still throw
// through readPrim/readAttribute, and constructing a batch performs no IO.
namespace zSpace {
namespace {
template <class T>
T usdTyped(const zUSDAttribute &a, const std::string &context) {
  if constexpr (std::is_same_v<T, float> || std::is_same_v<T, double>) {
    if (a.type == "float" || a.type == "double") {
      const double v = a.type == "float" ? double(std::get<float>(a.value))
                                         : std::get<double>(a.value);
      if (!std::isfinite(v) || std::abs(v) > std::numeric_limits<T>::max())
        throw std::runtime_error("USD numeric overflow: " + context);
      return static_cast<T>(v);
    }
  } else if constexpr (std::is_same_v<T, std::vector<float>> ||
                       std::is_same_v<T, std::vector<double>>) {
    if (a.type == "float[]" || a.type == "double[]") {
      T out;
      const auto copy = [&](const auto &values) {
        out.reserve(values.size());
        for (auto v : values) {
          using E = typename T::value_type;
          if (!std::isfinite(v) ||
              std::abs(double(v)) > std::numeric_limits<E>::max())
            throw std::runtime_error("USD numeric overflow: " + context);
          out.push_back(static_cast<E>(v));
        }
      };
      if (a.type == "float[]")
        copy(std::get<std::vector<float>>(a.value));
      else
        copy(std::get<std::vector<double>>(a.value));
      return out;
    }
  } else {
    const bool matches =
        (std::is_same_v<T, bool> && a.type == "bool") ||
        (std::is_same_v<T, int> && a.type == "int") ||
        (std::is_same_v<T, std::string> &&
         (a.type == "string" || a.type == "token")) ||
        (std::is_same_v<T, std::vector<int>> && a.type == "int[]");
    if (matches)
      if (const auto *value = std::get_if<T>(&a.value))
        return *value;
  }
  throw std::runtime_error("Incorrect USD attribute type at " + context + ": " +
                           a.type);
}
} // namespace
bool USDdoc::hasAttribute(const std::string &path,
                          const std::string &name) const {
  const auto info = readPrim(path);
  return std::find(info.attributes.begin(), info.attributes.end(), name) !=
         info.attributes.end();
}
bool USDdoc::readBool(const std::string &path, const std::string &name) const {
  return usdTyped<bool>(readAttribute(path, name), path + "." + name);
}
bool USDdoc::tryReadBool(const std::string &path, const std::string &name,
                         bool &out) const {
  if (!hasAttribute(path, name))
    return false;
  out = readBool(path, name);
  return true;
}
void USDdoc::writeBool(zUSDPrimEdit &prim, const std::string &name,
                       const bool &value, const std::string &interpolation,
                       bool custom) {
  zUSDAttribute a;
  a.type = "bool";
  a.value = value;
  a.interpolation = interpolation;
  a.custom = custom;
  prim.attributes[name] = std::move(a);
}
int USDdoc::readInt(const std::string &path, const std::string &name) const {
  return usdTyped<int>(readAttribute(path, name), path + "." + name);
}
bool USDdoc::tryReadInt(const std::string &path, const std::string &name,
                        int &out) const {
  if (!hasAttribute(path, name))
    return false;
  out = readInt(path, name);
  return true;
}
void USDdoc::writeInt(zUSDPrimEdit &prim, const std::string &name,
                      const int &value, const std::string &interpolation,
                      bool custom) {
  zUSDAttribute a;
  a.type = "int";
  a.value = value;
  a.interpolation = interpolation;
  a.custom = custom;
  prim.attributes[name] = std::move(a);
}
float USDdoc::readFloat(const std::string &path,
                        const std::string &name) const {
  return usdTyped<float>(readAttribute(path, name), path + "." + name);
}
bool USDdoc::tryReadFloat(const std::string &path, const std::string &name,
                          float &out) const {
  if (!hasAttribute(path, name))
    return false;
  out = readFloat(path, name);
  return true;
}
void USDdoc::writeFloat(zUSDPrimEdit &prim, const std::string &name,
                        const float &value, const std::string &interpolation,
                        bool custom) {
  zUSDAttribute a;
  a.type = "float";
  a.value = value;
  a.interpolation = interpolation;
  a.custom = custom;
  prim.attributes[name] = std::move(a);
}
double USDdoc::readDouble(const std::string &path,
                          const std::string &name) const {
  return usdTyped<double>(readAttribute(path, name), path + "." + name);
}
bool USDdoc::tryReadDouble(const std::string &path, const std::string &name,
                           double &out) const {
  if (!hasAttribute(path, name))
    return false;
  out = readDouble(path, name);
  return true;
}
void USDdoc::writeDouble(zUSDPrimEdit &prim, const std::string &name,
                         const double &value, const std::string &interpolation,
                         bool custom) {
  zUSDAttribute a;
  a.type = "double";
  a.value = value;
  a.interpolation = interpolation;
  a.custom = custom;
  prim.attributes[name] = std::move(a);
}
std::string USDdoc::readString(const std::string &path,
                               const std::string &name) const {
  return usdTyped<std::string>(readAttribute(path, name), path + "." + name);
}
bool USDdoc::tryReadString(const std::string &path, const std::string &name,
                           std::string &out) const {
  if (!hasAttribute(path, name))
    return false;
  out = readString(path, name);
  return true;
}
void USDdoc::writeString(zUSDPrimEdit &prim, const std::string &name,
                         const std::string &value,
                         const std::string &interpolation, bool custom) {
  zUSDAttribute a;
  a.type = "string";
  a.value = value;
  a.interpolation = interpolation;
  a.custom = custom;
  prim.attributes[name] = std::move(a);
}
std::vector<int> USDdoc::readIntArray(const std::string &path,
                                      const std::string &name) const {
  return usdTyped<std::vector<int>>(readAttribute(path, name),
                                    path + "." + name);
}
bool USDdoc::tryReadIntArray(const std::string &path, const std::string &name,
                             std::vector<int> &out) const {
  if (!hasAttribute(path, name))
    return false;
  out = readIntArray(path, name);
  return true;
}
void USDdoc::writeIntArray(zUSDPrimEdit &prim, const std::string &name,
                           const std::vector<int> &value,
                           const std::string &interpolation, bool custom) {
  zUSDAttribute a;
  a.type = "int[]";
  a.value = value;
  a.interpolation = interpolation;
  a.custom = custom;
  prim.attributes[name] = std::move(a);
}
std::vector<float> USDdoc::readFloatArray(const std::string &path,
                                          const std::string &name) const {
  return usdTyped<std::vector<float>>(readAttribute(path, name),
                                      path + "." + name);
}
bool USDdoc::tryReadFloatArray(const std::string &path, const std::string &name,
                               std::vector<float> &out) const {
  if (!hasAttribute(path, name))
    return false;
  out = readFloatArray(path, name);
  return true;
}
void USDdoc::writeFloatArray(zUSDPrimEdit &prim, const std::string &name,
                             const std::vector<float> &value,
                             const std::string &interpolation, bool custom) {
  zUSDAttribute a;
  a.type = "float[]";
  a.value = value;
  a.interpolation = interpolation;
  a.custom = custom;
  prim.attributes[name] = std::move(a);
}
std::vector<double> USDdoc::readDoubleArray(const std::string &path,
                                            const std::string &name) const {
  return usdTyped<std::vector<double>>(readAttribute(path, name),
                                       path + "." + name);
}
bool USDdoc::tryReadDoubleArray(const std::string &path,
                                const std::string &name,
                                std::vector<double> &out) const {
  if (!hasAttribute(path, name))
    return false;
  out = readDoubleArray(path, name);
  return true;
}
void USDdoc::writeDoubleArray(zUSDPrimEdit &prim, const std::string &name,
                              const std::vector<double> &value,
                              const std::string &interpolation, bool custom) {
  zUSDAttribute a;
  a.type = "double[]";
  a.value = value;
  a.interpolation = interpolation;
  a.custom = custom;
  prim.attributes[name] = std::move(a);
}
void USDdoc::writePoints(zUSDPrimEdit &prim, const std::string &name,
                         const std::vector<float> &value,
                         const std::string &interpolation, bool custom) {
  if (value.size() % 3 != 0)
    throw std::runtime_error("point3f[] requires XYZ triples: " + name);
  for (float v : value)
    if (!std::isfinite(v))
      throw std::runtime_error("Non-finite point3f[] value: " + name);
  zUSDAttribute attribute;
  attribute.type = "point3f[]";
  attribute.value = value;
  attribute.interpolation = interpolation;
  attribute.custom = custom;
  prim.attributes[name] = std::move(attribute);
}
void USDdoc::writeNormals(zUSDPrimEdit &prim, const std::string &name,
                          const std::vector<float> &value,
                          const std::string &interpolation, bool custom) {
  if (value.size() % 3 != 0)
    throw std::runtime_error("normal3f[] requires XYZ triples: " + name);
  for (float v : value)
    if (!std::isfinite(v))
      throw std::runtime_error("Non-finite normal3f[] value: " + name);
  zUSDAttribute attribute;
  attribute.type = "normal3f[]";
  attribute.value = value;
  attribute.interpolation = interpolation;
  attribute.custom = custom;
  prim.attributes[name] = std::move(attribute);
}
} // namespace zSpace
