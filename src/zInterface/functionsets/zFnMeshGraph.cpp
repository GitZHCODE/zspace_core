#include <zspace/zInterface/functionsets/zFnMesh.h>
#include <zspace/zInterface/functionsets/zFnGraph.h>
#include <algorithm>
#include <cmath>
#include <limits>
#include <set>
#include <stdexcept>

namespace zSpace {
void zFnMesh::graphMesh(zObjectGraph& graph, double offset)
{
    if (!hasObject()) throw std::logic_error("graphMesh requires an attached mesh.");
    if (!std::isfinite(offset) || offset <= 0) throw std::invalid_argument("graphMesh offset must be finite and positive.");
    zPointArray input, positions;
    zIntArray edges, counts, connects;
    zFnGraph fn(graph); fn.getVertexPositions(input); fn.getEdgeData(edges);
    if (edges.empty() || edges.size()%2) throw std::invalid_argument("graphMesh requires graph edges.");
    struct Ray { int edge; double x,y,angle; int left=-1,right=-1; };
    std::vector<std::vector<Ray>> rings(input.size());
    std::vector<int> centers(input.size(),-1);
    std::set<std::pair<int,int>> unique;
    for (const auto& p:input)
        if (!std::isfinite(p.x)||!std::isfinite(p.y)||!std::isfinite(p.z)) throw std::invalid_argument("graphMesh positions must be finite.");
    for (size_t e=0;e<edges.size();e+=2) {
        int a=edges[e],b=edges[e+1];
        if (a<0||b<0||a>=int(input.size())||b>=int(input.size())||a==b||!unique.emplace(std::min(a,b),std::max(a,b)).second)
            throw std::invalid_argument("graphMesh requires distinct valid graph edges.");
        double x=double(input[b].x)-input[a].x,y=double(input[b].y)-input[a].y,length=std::hypot(x,y);
        if (length<=1e-10) throw std::invalid_argument("graphMesh cannot offset a vertical or collapsed member in XY.");
        x/=length;y/=length;
        rings[a].push_back({int(e/2),x,y,std::atan2(y,x)});
        rings[b].push_back({int(e/2),-x,-y,std::atan2(-y,-x)});
    }
    auto corner=[&](int vertex,double x,double y) {
        double px=double(input[vertex].x)+x,py=double(input[vertex].y)+y;
        if (!std::isfinite(px)||!std::isfinite(py)||std::abs(px)>std::numeric_limits<float>::max()||std::abs(py)>std::numeric_limits<float>::max())
            throw std::invalid_argument("graphMesh offset exceeds coordinate range.");
        positions.emplace_back(float(px),float(py),input[vertex].z);return int(positions.size()-1);
    };
    // One centre and one corner per sector. Adjacent members share their corner
    // indices, so junctions are joined without halfedge construction or welding.
    std::vector<int> left(edges.size(),-1),right(edges.size(),-1);
    for (int v=0;v<int(input.size());++v) {
        auto& ring=rings[v];if(ring.empty())continue;
        centers[v]=int(positions.size());positions.push_back(input[v]);
        std::sort(ring.begin(),ring.end(),[](const Ray& a,const Ray& b){return a.angle<b.angle;});
        if(ring.size()==1) {
            auto& r=ring[0];r.left=corner(v,-r.y*offset,r.x*offset);r.right=corner(v,r.y*offset,-r.x*offset);
        } else for(size_t i=0;i<ring.size();++i) {
            auto& a=ring[i];auto& b=ring[(i+1)%ring.size()];
            double denominator=1-(a.x*b.x+a.y*b.y);
            if(denominator<=1e-12)throw std::invalid_argument("graphMesh has overlapping outgoing directions.");
            int id=corner(v,offset*(-a.y+b.y)/denominator,offset*(a.x-b.x)/denominator);
            a.left=id;b.right=id;
        }
        for(const auto& r:ring) {size_t end=2*r.edge+(edges[2*r.edge]==v?0:1);left[end]=r.left;right[end]=r.right;}
    }
    for(size_t e=0;e<edges.size();e+=2) {
        int a=centers[edges[e]],b=centers[edges[e+1]];
        for(const auto& face: {zIntArray{a,right[e],left[e+1],b},zIntArray{a,b,right[e+1],left[e]}}) {
            // Every projected corner must turn left. This rejects a width that
            // consumes a short member before publishing a folded/zero-area face.
            for(int i=0;i<4;++i) {
                auto p=positions[face[i]],q=positions[face[(i+1)%4]],r=positions[face[(i+2)%4]];
                double cross=(double(q.x)-p.x)*(double(r.y)-q.y)-(double(q.y)-p.y)*(double(r.x)-q.x);
                if(!(cross>0))throw std::invalid_argument("graphMesh ribbon folds or collapses; reduce offset or junction sharpness.");
            }
            counts.push_back(4);connects.insert(connects.end(),face.begin(),face.end());
        }
    }
    zObjectMesh candidate;zFnMesh out(candidate);out.create(positions,counts,connects);out.computeMeshNormals();
    *meshObj=std::move(candidate);
}
}
