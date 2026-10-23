#include <zspace/zInterface/functionsets/zFnMeshDynamics.h>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <stdexcept>
#include <unordered_map>

namespace zSpace {
namespace {
constexpr double pi = 3.14159265358979323846;
std::uint64_t origamiEdgeKey(int a, int b) {
    const auto endpoints = std::minmax(a, b);
    return (std::uint64_t(std::uint32_t(endpoints.first)) << 32) | std::uint32_t(endpoints.second);
}
struct V {
    double x=0, y=0, z=0;
    V() = default;
    V(double a,double b,double c):x(a),y(b),z(c) {}
    V(const zVector& p):x(p.x),y(p.y),z(p.z) {}
    V operator+(V b) const { return {x+b.x,y+b.y,z+b.z}; }
    V operator-(V b) const { return {x-b.x,y-b.y,z-b.z}; }
    V operator*(double s) const { return {x*s,y*s,z*s}; }
    double dot(V b) const { return x*b.x+y*b.y+z*b.z; }
    V cross(V b) const { return {y*b.z-z*b.y,z*b.x-x*b.z,x*b.y-y*b.x}; }
    double length() const { return std::sqrt(dot(*this)); }
    zVector vec() const { return zVector(x,y,z); }
};
double corner(V a,V b,V c) {
    const V u=a-b,v=c-b;
    return std::atan2(u.cross(v).length(),u.dot(v));
}
void validSettings(const zOrigamiSettings& s) {
    for(double x : {s.axial,s.fold,s.facet,s.face,s.damping})
        if(!std::isfinite(x)||x<0) throw std::invalid_argument("Origami stiffness and damping must be finite and nonnegative.");
    if(!std::isfinite(s.foldAmount)||std::abs(s.foldAmount)>1)
        throw std::invalid_argument("Origami fold amount must be between -1 and 1.");
}
}

ZSPACE_INLINE void zFnMeshDynamics::prepareOrigami() {
    if(particlesObj.size()!=static_cast<std::size_t>(numVertices()))
        throw std::runtime_error("Create mesh dynamics before preparing origami.");
    origamiEdges.clear(); origamiTriangles.clear(); origamiAngles.clear();
    zInt2DArray tris; getMeshTriangles(tris);
    zPoint* p=getRawVertexPositions();
    std::unordered_map<std::uint64_t,int> edges;
    edges.reserve(static_cast<std::size_t>(numEdges()) * 2);
    for(const auto& face:tris) for(std::size_t j=0;j+2<face.size();j+=3) {
        const std::array<int,3> t{face[j],face[j+1],face[j+2]};
        if((V(p[t[1]])-V(p[t[0]])).cross(V(p[t[2]])-V(p[t[0]])).length()<=1e-14)
            throw std::runtime_error("Origami requires nondegenerate triangles.");
        origamiTriangles.push_back(t);
        std::array<double,3> angles;
        for(int k=0;k<3;k++) {
            angles[k]=corner(p[t[(k+1)%3]],p[t[k]],p[t[(k+2)%3]]);
            const int a=t[k],b=t[(k+1)%3],c=t[(k+2)%3];
            const std::uint64_t key=origamiEdgeKey(a,b);
            auto found=edges.find(key);
            if(found==edges.end()) {
                OrigamiEdge e; e.a=a;e.b=b;e.c=c;e.rest=(V(p[b])-V(p[a])).length();
                edges[key]=static_cast<int>(origamiEdges.size()); origamiEdges.push_back(e);
            } else {
                auto& e=origamiEdges[found->second];
                if(e.d!=-1||e.a!=b||e.b!=a)
                    throw std::runtime_error("Origami requires a consistently wound manifold mesh.");
                e.d=c;
            }
        }
        origamiAngles.push_back(angles);
    }
    zIntArray meshEdges; getEdgeData(meshEdges);
    for(std::size_t i=0;i+1<meshEdges.size();i+=2) {
        const auto it=edges.find(origamiEdgeKey(meshEdges[i],meshEdges[i+1]));
        if(it!=edges.end()) {
            auto& e=origamiEdges[it->second];
            e.source=static_cast<int>(i/2);
            // Original panel boundaries may bend. Only added diagonals enforce panel flatness.
            e.assignment=2;
        }
    }
    for(auto& pObj:particlesObj) { zFnParticle fn(pObj); zVector zero; fn.setVelocity(zero);fn.clearForce(); }
}

ZSPACE_INLINE void zFnMeshDynamics::setOrigamiCrease(int edgeId,int assignment,double angleRadians) {
    if(assignment < -1 || assignment > 2 || !std::isfinite(angleRadians) || angleRadians<0 || angleRadians>=pi)
        throw std::invalid_argument("Invalid origami crease assignment or angle (0 <= angle < pi).");
    for(auto& e:origamiEdges) if(e.source==edgeId) {
        if(e.d<0) throw std::invalid_argument("Boundary edges cannot be folding creases.");
        e.assignment=assignment;
        e.target=(assignment==1?1:assignment==-1?-1:0)*angleRadians;
        return;
    }
    throw std::invalid_argument("Origami crease edge ID is not in the input mesh.");
}

ZSPACE_INLINE void zFnMeshDynamics::getOrigamiForces(const zOrigamiSettings& s,zVectorArray& output,zOrigamiDiagnostics& diag,zOrigamiForceComponents* components) {
    validSettings(s); diag={};
    const std::size_t n=particlesObj.size();
    if(origamiTriangles.empty()||n!=static_cast<std::size_t>(numVertices()))
        throw std::runtime_error("Origami rest mesh has not been prepared.");
    std::vector<V> p(n),v(n),f(n);
    std::vector<double> mass(n),stiffness(n,0),damping(n,0);
    for(std::size_t i=0;i<n;i++) {
        zFnParticle fn(particlesObj[i]);p[i]=fn.getPosition();v[i]=fn.getVelocity();mass[i]=fn.getMass();
        if(!std::isfinite(mass[i])||mass[i]<=0) throw std::runtime_error("Origami masses must be positive.");
    }
    zVectorArray* component=nullptr;
    if(components) for(auto* values:{&components->axial,&components->crease,&components->facet,&components->face,&components->damping}) values->assign(n,zVector());
    auto add=[&](int i,V force,double k) {
        f[i]=f[i]+force;stiffness[i]+=k;
        if(component) (*component)[i]+=force.vec();
    };
    for(const auto& e:origamiEdges) {
        const V edge=p[e.b]-p[e.a]; const double len=edge.length();
        if(len<=e.rest*1e-8) throw std::runtime_error("Origami edge collapsed.");
        const V axis=edge*(1/len);
        const double k=s.axial/e.rest;
        const V axial=axis*(k*(len-e.rest));
        component=components?&components->axial:nullptr;
        add(e.a,axial,k);add(e.b,axial*(-1),k);
        diag.maxStrain=std::max(diag.maxStrain,std::abs(len/e.rest-1));
        const double c=2*s.damping*std::sqrt(k*std::min(mass[e.a],mass[e.b]));
        const V drag=(v[e.b]-v[e.a])*c;
        component=components?&components->damping:nullptr;
        add(e.a,drag,0);add(e.b,drag*(-1),0);damping[e.a]+=c;damping[e.b]+=c;
        if(e.d<0||e.assignment==2) continue;
        V n1=edge.cross(p[e.c]-p[e.a]),n2=(edge*(-1)).cross(p[e.d]-p[e.b]);
        const double a1=n1.length(),a2=n2.length();
        if(a1<=e.rest*e.rest*1e-10||a2<=e.rest*e.rest*1e-10)
            throw std::runtime_error("Origami facet collapsed near a crease.");
        n1=n1*(1/a1);n2=n2*(1/a2);
        // Positive valley folds lift both opposite vertices above an initially +Z sheet.
        const double theta=std::atan2(axis.dot(n2.cross(n1)),n1.dot(n2));
        const double error=std::remainder(theta-e.target*s.foldAmount,2*pi);
        if(e.source<0) diag.maxPanelAngleError=std::max(diag.maxPanelAngleError,std::abs(theta));
        else if(e.assignment==-1||e.assignment==1)
            diag.maxCreaseAngleError=std::max(diag.maxCreaseAngleError,std::abs(error));
        const double kh=e.rest*(e.assignment==0?s.facet:s.fold);
        if(kh>0) diag.maxAngleError=std::max(diag.maxAngleError,std::abs(error));
        const V gc=n1*(len/a1),gd=n2*(len/a2);
        const double tc=(p[e.c]-p[e.a]).dot(edge)/(len*len),td=(p[e.d]-p[e.a]).dot(edge)/(len*len);
        const V ga=gc*(tc-1)+gd*(td-1),gb=gc*(-tc)+gd*(-td);
        const int ids[4]={e.a,e.b,e.c,e.d};const V g[4]={ga,gb,gc,gd};
        component=components?(e.assignment==0?&components->facet:&components->crease):nullptr;
        for(int q=0;q<4;q++) add(ids[q],g[q]*(-kh*error),kh*g[q].dot(g[q]));
    }
    component=components?&components->face:nullptr;
    for(std::size_t i=0;i<origamiTriangles.size();i++) {
        const auto& t=origamiTriangles[i];
        for(int q=0;q<3;q++) {
            const int b=t[q],a=t[(q+1)%3],c=t[(q+2)%3];
            const V u=p[a]-p[b],w=p[c]-p[b];const V raw=u.cross(w);
            const double norm=raw.length();
            if(norm<=1e-14) throw std::runtime_error("Origami triangle collapsed.");
            const V normal=raw*(1/norm);
            const double error=std::atan2(norm,u.dot(w))-origamiAngles[i][q];
            const V ga=u.cross(normal)*(1/u.dot(u)),gc=normal.cross(w)*(1/w.dot(w)),gb=(ga+gc)*(-1);
            const int ids[3]={a,b,c};const V g[3]={ga,gb,gc};
            for(int j=0;j<3;j++) add(ids[j],g[j]*(-s.face*error),s.face*g[j].dot(g[j]));
        }
    }
    output.resize(n);diag.stableTimeStep=0.02;
    for(std::size_t i=0;i<n;i++) {
        output[i]=f[i].vec();
        if(!std::isfinite(f[i].length())) throw std::runtime_error("Non-finite origami force.");
        zFnParticle fn(particlesObj[i]);if(fn.getFixed()) continue;
        diag.maxForce=std::max(diag.maxForce,f[i].length());diag.maxSpeed=std::max(diag.maxSpeed,v[i].length());
        // Include angular constraints and damping in addition to the paper's axial bound.
        if(stiffness[i]>0) diag.stableTimeStep=std::min(diag.stableTimeStep,0.1*std::sqrt(mass[i]/stiffness[i]));
        if(damping[i]>0) diag.stableTimeStep=std::min(diag.stableTimeStep,0.25*mass[i]/damping[i]);
    }
}

ZSPACE_INLINE void zFnMeshDynamics::stepOrigami(const zOrigamiSettings& s,double timeStep,zOrigamiDiagnostics& diag) {
    if(!std::isfinite(timeStep)||timeStep<=0) throw std::invalid_argument("Origami timestep must be positive.");
    zVectorArray forces;getOrigamiForces(s,forces,diag);
    zPointArray next(particlesObj.size());zVectorArray velocities(particlesObj.size());
    double remaining=timeStep,minimumStableStep=diag.stableTimeStep;
    int substeps=0;
    // Subdivide the requested interval; a stability cap must not discard simulation time.
    while(remaining>timeStep*1e-12) {
        if(++substeps>10000) throw std::runtime_error("Origami timestep needs too many stable substeps. Reduce the timestep or inspect degenerate facets.");
        const double dt=std::min(remaining,diag.stableTimeStep);
        minimumStableStep=std::min(minimumStableStep,diag.stableTimeStep);
        for(std::size_t i=0;i<particlesObj.size();i++) {
            zFnParticle fn(particlesObj[i]);V p=fn.getPosition(),v=fn.getVelocity();
            if(!fn.getFixed()) {v=v+V(forces[i])*(dt/fn.getMass());p=p+v*dt;}
            else v=V();
            if(!std::isfinite(p.length())||!std::isfinite(v.length())) throw std::runtime_error("Origami step became non-finite.");
            next[i]=p.vec();velocities[i]=v.vec();
        }
        for(std::size_t i=0;i<particlesObj.size();i++) {
            zFnParticle fn(particlesObj[i]);*fn.getRawPosition()=next[i];fn.setVelocity(velocities[i]);fn.clearForce();
        }
        remaining-=dt;
        getOrigamiForces(s,forces,diag);
    }
    setVertexPositions(next);
    computeMeshNormals();
    diag.stableTimeStep=minimumStableStep;
}
}
