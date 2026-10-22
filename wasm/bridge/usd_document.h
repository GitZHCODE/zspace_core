#pragma once
// Generic default-time USD queries. No application-specific prim names or units.
#if defined(ZSPACE_IO_OPENUSD)
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usd/primRange.h>
#include <pxr/usd/usdGeom/pointBased.h>
#include <pxr/usd/usdGeom/xformCache.h>
#include <pxr/usd/usdGeom/metrics.h>
#include <pxr/usd/usdGeom/primvarsAPI.h>
#include <nlohmann/json.hpp>

namespace usdDocument {
using namespace pxr;
using Json = nlohmann::json;
static UsdStageRefPtr stage;
static std::vector<double> numbers;
static std::string info, error;
template<class F> int query(F f) {
  try { error.clear(); numbers.clear(); info.clear(); f(); return 1; }
  catch(const std::exception& e) { numbers.clear(); info.clear(); error=e.what(); return 0; }
}
inline UsdPrim prim(const char* path) {
  if(!stage || !path) throw std::runtime_error("No USD document or prim path.");
  const SdfPath p(path);
  if(!p.IsAbsolutePath() || !p.IsPrimPath()) throw std::runtime_error("Expected an absolute USD prim path.");
  auto result=stage->GetPrimAtPath(p);
  if(!result) throw std::runtime_error(std::string("USD prim not found: ")+path);
  return result;
}
inline void number(double n) {
  if(!std::isfinite(n)) throw std::runtime_error("Non-finite USD numeric value.");
  numbers.push_back(n);
}
template<class T> bool scalar(const VtValue& v) {
  if(!v.IsHolding<T>()) return false;
  number(static_cast<double>(v.UncheckedGet<T>())); return true;
}
template<class T> bool numericArray(const VtValue& v) {
  if(!v.IsHolding<VtArray<T>>()) return false;
  for(const auto n:v.UncheckedGet<VtArray<T>>()) number(static_cast<double>(n));
  return true;
}
template<class T> bool vectors(const VtValue& v) {
  if(!v.IsHolding<VtArray<T>>()) return false;
  for(const auto& p:v.UncheckedGet<VtArray<T>>()) for(int i=0;i<3;++i) number(p[i]);
  return true;
}
}
extern "C" {
EMSCRIPTEN_KEEPALIVE int zspace_usd_document_open(const char* path) {
  return usdDocument::query([&] {
    if(!path) throw std::runtime_error("Missing USD file path.");
    auto next=pxr::UsdStage::Open(path);
    if(!next) throw std::runtime_error("Cannot open USD document; provide a self-contained file or USDZ package.");
    usdDocument::stage=next;
  });
}
EMSCRIPTEN_KEEPALIVE void zspace_usd_document_close() {
  usdDocument::stage.Reset(); usdDocument::numbers.clear(); usdDocument::info.clear();
}
EMSCRIPTEN_KEEPALIVE int zspace_usd_document_metadata() {
  return usdDocument::query([] {
    using namespace usdDocument;
    if(!stage) throw std::runtime_error("No USD document.");
    Json prims=Json::array();
    for(const auto& p:stage->Traverse()) prims.push_back({{"path",p.GetPath().GetString()},{"type",p.GetTypeName().GetString()}});
    info=Json({{"metersPerUnit",UsdGeomGetStageMetersPerUnit(stage)},
      {"upAxis",UsdGeomGetStageUpAxis(stage).GetString()},{"prims",prims}}).dump();
  });
}
EMSCRIPTEN_KEEPALIVE int zspace_usd_document_prim(const char* path) {
  return usdDocument::query([&] {
    using namespace usdDocument;
    auto p=prim(path); Json attrs=Json::array();
    for(const auto& a:p.GetAttributes()) attrs.push_back(a.GetName().GetString());
    UsdGeomXformCache cache; const auto matrix=cache.GetLocalToWorldTransform(p);
    info=Json({{"type",p.GetTypeName().GetString()},{"attributes",attrs},
      {"mirrored",matrix.GetDeterminant()<0}}).dump();
  });
}
EMSCRIPTEN_KEEPALIVE int zspace_usd_document_attribute(const char* path,const char* name) {
  return usdDocument::query([&] {
    using namespace usdDocument;
    if(!name) throw std::runtime_error("Missing attribute name.");
    auto a=prim(path).GetAttribute(TfToken(name)); VtValue value;
    UsdGeomPrimvar pv(a);
    const bool ok=pv ? pv.ComputeFlattened(&value) : (a && a.Get(&value));
    if(!ok) throw std::runtime_error(std::string("Missing or unreadable USD attribute: ")+name);
    Json meta={{"type",a.GetTypeName().GetAsToken().GetString()},{"array",value.IsArrayValued()},
      {"interpolation",pv ? pv.GetInterpolation().GetString() : ""}};
    if(value.IsHolding<TfToken>()) meta["value"]=value.UncheckedGet<TfToken>().GetString();
    else if(value.IsHolding<std::string>()) meta["value"]=value.UncheckedGet<std::string>();
    else if(value.IsHolding<VtArray<TfToken>>()) {
      meta["value"]=Json::array(); for(const auto& v:value.UncheckedGet<VtArray<TfToken>>()) meta["value"].push_back(v.GetString());
    }
    else if(!(scalar<bool>(value)||scalar<int>(value)||scalar<unsigned int>(value)||scalar<float>(value)||scalar<double>(value)||
      numericArray<int>(value)||numericArray<unsigned int>(value)||numericArray<float>(value)||numericArray<double>(value)||
      vectors<GfVec3f>(value)||vectors<GfVec3d>(value)))
      throw std::runtime_error("Unsupported USD attribute type: "+a.GetTypeName().GetAsToken().GetString());
    info=meta.dump();
  });
}
EMSCRIPTEN_KEEPALIVE int zspace_usd_document_points(const char* path) {
  return usdDocument::query([&] {
    using namespace usdDocument;
    auto p=prim(path); UsdGeomPointBased geometry(p); VtArray<GfVec3f> points;
    if(!geometry || !geometry.GetPointsAttr().Get(&points)) throw std::runtime_error("Prim has no readable point geometry.");
    UsdGeomXformCache cache; const auto transform=cache.GetLocalToWorldTransform(p);
    for(const auto& point:points) { const auto v=transform.Transform(GfVec3d(point)); for(int k=0;k<3;++k) number(v[k]); }
    info="{\"space\":\"world\",\"components\":3}";
  });
}
// Numeric buffers are float64, valid until the next document query; callers copy them.
EMSCRIPTEN_KEEPALIVE const double* zspace_usd_document_numbers() {return usdDocument::numbers.data();}
EMSCRIPTEN_KEEPALIVE int zspace_usd_document_count() {return static_cast<int>(usdDocument::numbers.size());}
EMSCRIPTEN_KEEPALIVE const char* zspace_usd_document_info() {return usdDocument::info.c_str();}
EMSCRIPTEN_KEEPALIVE const char* zspace_usd_document_error() {return usdDocument::error.c_str();}
}
#endif

