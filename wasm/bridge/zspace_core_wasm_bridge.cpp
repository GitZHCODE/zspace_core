#include <zspace/zInterface/functionsets/zFnMesh.h>
#include <zspace/zInterface/functionsets/zFnGraph.h>
#include <zspace/zInterface/functionsets/zFnMeshDynamics.h>
#include <zspace/zInterface/functionsets/zFnParticle.h>
#include <zspace/zInterface/objects/zObjectMesh.h>
#include <zspace/zIO/zIO.h>
#include <zspace/zIO/zIOResult.h>

#include "live_sketch.h"

#include <src/zIO/codecs/zCodecJSON.h>
#include <src/zIO/codecs/zCodecOBJ.h>
#include <src/zIO/codecs/zCodecUSD.h>
#include <src/zIO/internal/zIOData.h>
#include <src/zInterface/objects/zMeshObjectStorage.h>
#include <src/zCore/geometry/detail/zMeshProfile.h>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <deque>
#include <exception>
#include <map>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
#include "planarity_solver.h"
#include <cstdlib>

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#define ZSPACE_WASM_EXPORT EMSCRIPTEN_KEEPALIVE
#else
#define ZSPACE_WASM_EXPORT
#endif

namespace
{
	struct RenderBuffers
	{
		std::vector<float> positions;
		std::vector<float> normals;
		std::vector<float> colors;
		std::vector<float> faceColors;
		std::vector<float> faceCenters;
		std::vector<float> faceNormals;
		std::vector<float> edgeCenters;
		std::vector<float> edgeWeights;
		std::vector<float> pointPositions;
		std::vector<float> pointColors;
		std::vector<float> pointSizes;
		std::vector<float> linePositions;
		std::vector<float> lineColors;
		std::vector<float> lineWeights;
		std::vector<float> vectorOrigins;
		std::vector<float> vectorDirections;
		std::vector<float> vectorColors;
		std::vector<float> vectorWeights;
		std::vector<float> polygonPositions;
		std::vector<float> polygonColors;
		std::vector<std::uint32_t> polygonIndices;
		std::vector<std::uint32_t> indices;
		std::vector<std::uint32_t> edgeIndices;
		std::vector<std::uint32_t> faceCounts;
		std::vector<std::uint32_t> faceConnects;
		std::uint32_t faceCount = 0;
		std::uint32_t analysisCount = 0;
		double analysisMinValue = 0.0;
		double analysisMaxValue = 0.0;
		double analysisMaxAbsValue = 0.0;
		std::string lastError;
	};

	RenderBuffers gBuffers;
	RenderBuffers gUSDMeshBuffers;
	zSpace::io_detail::USDSceneData gUSDScene;
	zSpace::zOrigamiSettings gOrigamiSettings;
	zSpace::zOrigamiDiagnostics gOrigamiDiagnostics;
	std::map<int, std::pair<int, double>> gOrigamiCreases;
	zSpace::zObjectMesh gMeshObject;
	zSpace::zObjectMesh* gActiveMeshObject = &gMeshObject;
	zspace_live::SketchScene gSketchScene;
	bool gHasMesh = false;
	bool gSketchSetupDone = false;

	struct SolverParams
	{
		int mode = 0;
		double gravity = 0.0;
		zSpace::zVector direction = zSpace::zVector(0, 0, -1);
		double value = 0.0;
		double edgeLengthMultiplier = 1.0;
		double massMin = 1.0;
		double massMax = 1.0;
		double timeStep = 0.02;
		double vectorScale = 1.0;
		zSpace::zIntergrationType integrationType = zSpace::zEuler;
		bool displayForceVectors = true;
		bool gravityEnabled = true;
		bool vectorForceEnabled = true;
		bool edgeForceEnabled = true;
		bool residualForceEnabled = true;
		bool fixXY = false;
		double springStiffness = 1.0;
		double residualThreshold = 0.05;
		double areaForceTolerance = 0.001;
		double areaStrength = 0.35;
		double stepTolerance = 0.0001;
		double supportSize = 24.0;
		double dragStrength = 0.02;
		float drag = 0.2f;
	};

	void applySolverForceConstraint(zSpace::zVector& force, zSpace::zSolverForceConstraints constrainType)
	{
		if (constrainType == zSpace::zConstraintX || constrainType == zSpace::zConstraintXY || constrainType == zSpace::zConstraintZX) force.x = 0;
		if (constrainType == zSpace::zConstraintY || constrainType == zSpace::zConstraintXY || constrainType == zSpace::zConstraintYZ) force.y = 0;
		if (constrainType == zSpace::zConstraintZ || constrainType == zSpace::zConstraintYZ || constrainType == zSpace::zConstraintZX) force.z = 0;
	}

	class WasmMeshDynamics : public zSpace::zFnMeshDynamics
	{
	public:
		using zSpace::zFnMeshDynamics::zFnMeshDynamics;
		struct EdgePair
		{
			int a = -1;
			int b = -1;
		};
		zSpace::zVectorArray particleForces(bool clear = false)
		{
			zSpace::zVectorArray result;
			result.reserve(particlesObj.size());
			for (auto& particle : particlesObj) {
				zSpace::zFnParticle fn(particle);
				result.push_back(fn.getForce());
				if (clear) fn.clearForce();
			}
			return result;
		}
		zSpace::zPointArray dynamicPositions;
		zSpace::zDoubleArray edgeRestLengths;
		std::vector<EdgePair> edgePairs;
		std::vector<unsigned char> edgeActiveMask;
		std::vector<double> lastUpdateDisplacements;
		zSpace::zIntArray cachedTriangles;
		double cachedMeshScale = 1.0;

		const zSpace::zIntArray& getCachedTriangles()
		{
			if (cachedTriangles.empty() && meshObj)
			{
				zSpace::zInt2DArray faceTris;
				getMeshTriangles(faceTris);
				for (const auto& face : faceTris)
				{
					cachedTriangles.insert(cachedTriangles.end(), face.begin(), face.end());
				}
			}
			return cachedTriangles;
		}

		void addMinimizeAreaForce(double strength)
		{
			const int nV = static_cast<int>(dynamicPositions.size());
			if (nV == 0 || particlesObj.size() != static_cast<std::size_t>(nV)) return;

			const auto& tris = getCachedTriangles();
			const zSpace::zPoint* vPositions = dynamicPositions.data();

			for (std::size_t j = 0; j + 2 < tris.size(); j += 3)
			{
				const int i0 = tris[j + 0];
				const int i1 = tris[j + 1];
				const int i2 = tris[j + 2];
				zSpace::zPoint PA = vPositions[i0];
				zSpace::zPoint PB = vPositions[i1];
				zSpace::zPoint PC = vPositions[i2];

				zSpace::zVector AB = PB - PA;
				zSpace::zVector BC = PC - PB;
				zSpace::zVector CA = PA - PC;

				zSpace::zVector Normal = AB ^ BC;
				const double normalLength = Normal.length();
				if (!std::isfinite(normalLength) || normalLength <= 0.0) continue;
				Normal /= static_cast<float>(normalLength);

				const float halfStrength = static_cast<float>(0.5 * strength);
				zSpace::zVector f0 = (BC ^ Normal) * halfStrength;
				zSpace::zVector f1 = (CA ^ Normal) * halfStrength;
				zSpace::zVector f2 = (f0 * -1.0f) - f1;

				zSpace::zFnParticle(particlesObj[i0]).addForce(f0);
				zSpace::zFnParticle(particlesObj[i1]).addForce(f1);
				zSpace::zFnParticle(particlesObj[i2]).addForce(f2);
			}
		}

		void createLinked(zSpace::zObjectMesh& mesh)
		{
			meshObj = &mesh;
			particlesObj.clear();
			lastUpdateDisplacements.clear();
			cachedTriangles.clear();
			const auto& meshData = zSpace::zMeshObjectStorage::read(mesh);
			dynamicPositions = meshData.positions;

			zSpace::zPoint* positions = dynamicPositions.empty() ? nullptr : dynamicPositions.data();
			const int vertexCount = static_cast<int>(dynamicPositions.size());
			particlesObj.resize(vertexCount);
			for (int vertexId = 0; vertexId < vertexCount; ++vertexId)
			{
				zSpace::zFnParticle fnParticle(particlesObj[vertexId]);
				fnParticle.create(positions[vertexId], false);
			}

			edgeRestLengths.clear();
			edgeRestLengths.resize(std::max(0, meshData.numEdges()), 0.0f);
			edgePairs.clear();
			edgePairs.resize(std::max(0, meshData.numEdges()));
			edgeActiveMask.assign(edgePairs.size(), 1);
			double totalRestLength = 0.0;
			int restLengthCount = 0;
			for (int edgeId = 0; edgeId < meshData.numEdges(); ++edgeId)
			{
				const std::size_t index = static_cast<std::size_t>(edgeId) * 2;
				if (index + 1 >= meshData.edgeVertexIndices.size()) continue;
				const int v0 = meshData.edgeVertexIndices[index];
				const int v1 = meshData.edgeVertexIndices[index + 1];
				if (v0 < 0 || v1 < 0 || v0 >= vertexCount || v1 >= vertexCount) continue;
				edgePairs[edgeId] = { v0, v1 };
				const double length = (dynamicPositions[v1] - dynamicPositions[v0]).length();
				edgeRestLengths[edgeId] = length;
				if (std::isfinite(length) && length > 1.0e-9)
				{
					totalRestLength += length;
					++restLengthCount;
				}
			}
			cachedMeshScale = restLengthCount > 0 ? totalRestLength / static_cast<double>(restLengthCount) : 1.0;
		}

		void setInactiveEdges(const zSpace::zIntArray& edgeIds)
		{
			edgeActiveMask.assign(edgePairs.size(), 1);
			for (int edgeId : edgeIds)
			{
				if (edgeId < 0 || edgeId >= static_cast<int>(edgeActiveMask.size())) continue;
				edgeActiveMask[edgeId] = 0;
			}
		}

		bool isEdgeActive(int edgeId) const
		{
			if (edgeId < 0 || edgeId >= static_cast<int>(edgePairs.size())) return false;
			if (edgePairs[edgeId].a < 0 || edgePairs[edgeId].b < 0) return false;
			if (edgeId < static_cast<int>(edgeActiveMask.size())) return edgeActiveMask[edgeId] != 0;
			return true;
		}

		bool restoreLinkedPositions(const zSpace::zPointArray& positions)
		{
			if (positions.size() != dynamicPositions.size() || positions.size() != particlesObj.size()) return false;
			dynamicPositions = positions;
			zSpace::zPoint* linkedPositions = dynamicPositions.empty() ? nullptr : dynamicPositions.data();
			for (std::size_t i = 0; i < particlesObj.size(); ++i)
			{
				zSpace::zFnParticle fnParticle(particlesObj[i]);
				fnParticle.setPosition(&linkedPositions[i]);
			}
			lastUpdateDisplacements.assign(dynamicPositions.size(), 0.0);
			clearVelocityAndForce();
			return true;
		}

		double springRestLength(int edgeId, double currentLength, double edgeLengthMultiplier) const
		{
			const double multiplier = std::max(0.0, edgeLengthMultiplier);
			if (edgeId >= 0 && edgeId < static_cast<int>(edgeRestLengths.size()))
			{
				const double restLength = edgeRestLengths[edgeId];
				if (std::isfinite(restLength) && restLength > 1.0e-9) return restLength * multiplier;
			}
			return currentLength * multiplier;
		}

		void addGuardedSpringForce(double stiffness, double edgeLengthMultiplier, zSpace::zSolverForceConstraints constrainType = zSpace::zConstraintFree)
		{
			zSpace::zPoint* positions = dynamicPositions.empty() ? nullptr : dynamicPositions.data();
			if (!positions) return;
			for (int edgeId = 0; edgeId < static_cast<int>(edgePairs.size()); ++edgeId)
			{
				if (!isEdgeActive(edgeId)) continue;
				const int v0 = edgePairs[edgeId].a;
				const int v1 = edgePairs[edgeId].b;
				if (v0 < 0 || v1 < 0 || v0 >= static_cast<int>(particlesObj.size()) || v1 >= static_cast<int>(particlesObj.size())) continue;

				zSpace::zVector edgeVector = positions[v1] - positions[v0];
				const double edgeLength = edgeVector.length();
				if (!std::isfinite(edgeLength) || edgeLength <= 1.0e-9) continue;
				edgeVector.normalize();

				const double targetLength = springRestLength(edgeId, edgeLength, edgeLengthMultiplier);
				const double extension = edgeLength - targetLength;
				if (std::abs(extension) <= std::max(1.0e-9, cachedMeshScale * 1.0e-9)) continue;
				const float value = static_cast<float>(stiffness * extension);
				zSpace::zVector forceA = edgeVector * value;
				zSpace::zVector forceB = forceA * -1.0f;
				applySolverForceConstraint(forceA, constrainType);
				applySolverForceConstraint(forceB, constrainType);

				zSpace::zFnParticle particle0(particlesObj[v0]);
				zSpace::zFnParticle particle1(particlesObj[v1]);
				particle0.addForce(forceA);
				particle1.addForce(forceB);
			}
		}

		void addMassScaledGravityForce(double gravity, const zSpace::zVector& direction)
		{
			zSpace::zVector unitDirection = direction;
			if (unitDirection.length() <= 1.0e-9f) unitDirection = zSpace::zVector(0, 0, -1);
			unitDirection.normalize();

			for (auto& particle : particlesObj)
			{
				zSpace::zFnParticle fnParticle(particle);
				if (fnParticle.getFixed()) continue;

				const double mass = std::max(1.0e-6, fnParticle.getMass());
				zSpace::zVector force = unitDirection * static_cast<float>(gravity * mass);
				fnParticle.addForce(force);
			}
		}

		void setSupports(const zSpace::zIntArray& supports)
		{
			for (auto& particle : particlesObj)
			{
				zSpace::zFnParticle fnParticle(particle);
				fnParticle.setFixed(false);
			}

			for (int vertexId : supports)
			{
				if (vertexId < 0 || vertexId >= static_cast<int>(particlesObj.size())) continue;
				zSpace::zFnParticle fnParticle(particlesObj[vertexId]);
				fnParticle.setFixed(true);
			}
		}

		void setMassesFromVertexColors(double minMass, double maxMass, const zSpace::zColorArray& colors)
		{
			const double lo = std::max(1.0e-6, std::min(minMass, maxMass));
			const double hi = std::max(1.0e-6, std::max(minMass, maxMass));
			for (std::size_t i = 0; i < particlesObj.size(); ++i)
			{
				double red = i < colors.size() ? colors[i].r : 1.0;
				if (red > 1.0) red /= 255.0;
				red = std::clamp(red, 0.0, 1.0);
				const double mass = lo + (hi - lo) * red;
				zSpace::zFnParticle fnParticle(particlesObj[i]);
				fnParticle.setMass(mass);
			}
		}

		double getParticleMass(int vertexId)
		{
			if (vertexId < 0 || vertexId >= static_cast<int>(particlesObj.size()))
			{
				return 1.0;
			}

			zSpace::zFnParticle fnParticle(particlesObj[vertexId]);
			return std::max(1.0e-6, fnParticle.getMass());
		}

		void clearVelocityAndForce()
		{
			for (auto& particle : particlesObj)
			{
				zSpace::zFnParticle fnParticle(particle);
				zSpace::zVector zero(0, 0, 0);
				fnParticle.setVelocity(zero);
				fnParticle.setForce(zero);
			}
		}

		zSpace::zVectorArray previewSurfaceForces(double strength, double edgeStiffness, double multiplier, bool fixXY)
		{
			zSpace::zVectorArray saved, forces;
			for (auto& particle : particlesObj)
			{
				zSpace::zFnParticle fnParticle(particle);
				saved.push_back(fnParticle.getForce());
				fnParticle.clearForce();
			}
			try
			{
				addMinimizeAreaForce(strength);
				addGuardedSpringForce(edgeStiffness, multiplier, fixXY ? zSpace::zConstraintXY : zSpace::zConstraintFree);
				for (auto& particle : particlesObj)
				{
					zSpace::zFnParticle fnParticle(particle);
					forces.push_back(fnParticle.getForce());
				}
			}
			catch (...)
			{
				for (std::size_t i = 0; i < particlesObj.size(); ++i)
					zSpace::zFnParticle(particlesObj[i]).setForce(saved[i]);
				throw;
			}
			for (std::size_t i = 0; i < particlesObj.size(); ++i)
				zSpace::zFnParticle(particlesObj[i]).setForce(saved[i]);
			return forces;
		}

		void clearParticles()
		{
			particlesObj.clear();
			dynamicPositions.clear();
			edgeRestLengths.clear();
			edgePairs.clear();
			edgeActiveMask.clear();
			lastUpdateDisplacements.clear();
			cachedTriangles.clear();
			cachedMeshScale = 1.0;
		}

		bool limitForces(double meshScale, double dT, double maxStepRatio)
		{
			const double safeDt = std::max(1.0e-6, dT);
			const double safeScale = std::max(1.0e-6, meshScale);
			const double maxStep = safeScale * std::max(1.0e-6, maxStepRatio);

			for (auto& particle : particlesObj)
			{
				zSpace::zFnParticle fnParticle(particle);
				if (fnParticle.getFixed()) continue;

				zSpace::zVector force = fnParticle.getForce();
				if (!std::isfinite(force.x) || !std::isfinite(force.y) || !std::isfinite(force.z))
				{
					zSpace::zVector zero(0, 0, 0);
					fnParticle.setForce(zero);
					return false;
				}

				const double forceLength = force.length();
				if (!std::isfinite(forceLength))
				{
					zSpace::zVector zero(0, 0, 0);
					fnParticle.setForce(zero);
					return false;
				}
				if (forceLength <= 1.0e-9) continue;

				const double mass = std::max(1.0e-6, fnParticle.getMass());
				const double maxForce = (maxStep * mass) / (safeDt * safeDt);
				if (forceLength > maxForce)
				{
					force *= static_cast<float>(maxForce / forceLength);
					fnParticle.setForce(force);
				}
			}

			return true;
		}

		std::vector<double> vertexAreas()
		{
			std::vector<double> areas(dynamicPositions.size(), 0.0);
			const auto& tris = getCachedTriangles();
			for (std::size_t j = 0; j + 2 < tris.size(); j += 3)
			{
				const int a = tris[j], b = tris[j+1], c = tris[j+2];
				zSpace::zVector ab = dynamicPositions[b] - dynamicPositions[a];
				zSpace::zVector ac = dynamicPositions[c] - dynamicPositions[a];
				const double thirdArea = (ab ^ ac).length() / 6.0;
				if (!std::isfinite(thirdArea) || thirdArea <= 0.0) return {};
				areas[a] += thirdArea;
				areas[b] += thirdArea;
				areas[c] += thirdArea;
			}
			return areas;
		}

		bool updateAreaDescent(double dT, double maxVertexStep)
		{
			const auto previous = dynamicPositions;
			const auto areas = vertexAreas();
			if (areas.size() != previous.size()) return false;
			double meanArea = 0.0;
			for (double value : areas) meanArea += value;
			meanArea /= std::max<std::size_t>(1, areas.size());
			const auto& tris = getCachedTriangles();
			// Evaluate the same triangulation throughout backtracking and reject element flips.
			auto area = [&](const zSpace::zPointArray& points, bool checkOrientation) {
				double total = 0.0;
				for (std::size_t j = 0; j + 2 < tris.size(); j += 3)
				{
					const int a = tris[j], b = tris[j + 1], c = tris[j + 2];
					auto cross = [&](const zSpace::zPointArray& p) {
						const double ux = double(p[b].x)-p[a].x, uy = double(p[b].y)-p[a].y, uz = double(p[b].z)-p[a].z;
						const double vx = double(p[c].x)-p[a].x, vy = double(p[c].y)-p[a].y, vz = double(p[c].z)-p[a].z;
						return std::array<double, 3>{uy*vz-uz*vy, uz*vx-ux*vz, ux*vy-uy*vx};
					};
					const auto n = cross(points);
					const double twiceArea = std::hypot(n[0], n[1], n[2]);
					if (!std::isfinite(twiceArea) || twiceArea <= 0.0) return -1.0;
					if (checkOrientation)
					{
						const auto old = cross(previous);
						if (n[0]*old[0] + n[1]*old[1] + n[2]*old[2] <= 0.0) return -1.0;
					}
					total += twiceArea * 0.5;
				}
				return total;
			};
			const double previousArea = area(previous, false);
			if (previousArea <= 0.0) return false;
			zSpace::zVectorArray moves(previous.size(), zSpace::zVector());
			double largestMove = 0.0;
			for (std::size_t i = 0; i < particlesObj.size(); ++i)
			{
				zSpace::zFnParticle particle(particlesObj[i]);
				if (particle.getFixed()) continue;
				if (areas[i] <= 0.0) return false;
				// Positive diagonal preconditioning preserves area stationary points.
				const double mass = std::max(1.0e-6, particle.getMass()) * areas[i] / meanArea;
				moves[i] = particle.getForce() * static_cast<float>(dT / mass);
				const double length = moves[i].length();
				if (!std::isfinite(length)) return false;
				largestMove = std::max(largestMove, length);
			}
			double scale = largestMove > maxVertexStep ? maxVertexStep / largestMove : 1.0;
			bool accepted = largestMove == 0.0;
			for (int attempt = 0; !accepted && attempt < 30; ++attempt, scale *= 0.5)
			{
				for (std::size_t i = 0; i < previous.size(); ++i)
				{
					dynamicPositions[i] = previous[i];
					dynamicPositions[i] += moves[i] * static_cast<float>(scale);
				}
				const double nextArea = area(dynamicPositions, true);
				accepted = nextArea > 0.0 && nextArea <= previousArea;
			}
			lastUpdateDisplacements.assign(previous.size(), 0.0);
			for (std::size_t i = 0; i < previous.size(); ++i)
			{
				if (!accepted) dynamicPositions[i] = previous[i];
				zSpace::zVector delta = dynamicPositions[i]; delta -= previous[i];
				lastUpdateDisplacements[i] = delta.length();
			}
			clearVelocityAndForce();
			zSpace::zFnMesh meshFn(*meshObj);
			meshFn.setVertexPositions(dynamicPositions);
			meshFn.computeMeshNormals();
			return accepted;
		}

		bool updateGuarded(double dT, zSpace::zIntergrationType type, double maxVertexStep, bool computeNormals = true)
		{
			zSpace::zPoint* positions = dynamicPositions.empty() ? nullptr : dynamicPositions.data();
			const int vertexCount = static_cast<int>(dynamicPositions.size());
			lastUpdateDisplacements.assign(std::max(0, vertexCount), 0.0);
			if (!positions || vertexCount == 0 || vertexCount != static_cast<int>(particlesObj.size()))
			{
				clearVelocityAndForce();
				return false;
			}
			zSpace::zPointArray previous;
			previous.reserve(vertexCount);
			for (int i = 0; i < vertexCount; ++i) previous.push_back(positions[i]);

			const double stepLimit = std::max(1.0e-6, maxVertexStep);
			for (auto& particle : particlesObj)
			{
				zSpace::zFnParticle fnParticle(particle);
				if (fnParticle.getFixed()) continue;
				const zSpace::zVector force = fnParticle.getForce();
				const double mass = std::max(1.0e-6, fnParticle.getMass());
				const double predictedDx = (static_cast<double>(force.x) / mass) * dT * dT;
				const double predictedDy = (static_cast<double>(force.y) / mass) * dT * dT;
				const double predictedDz = (static_cast<double>(force.z) / mass) * dT * dT;
				const double predictedDisplacement = std::sqrt(
					(predictedDx * predictedDx) +
					(predictedDy * predictedDy) +
					(predictedDz * predictedDz)
				);
				if (!std::isfinite(predictedDisplacement) || predictedDisplacement > stepLimit)
				{
					clearVelocityAndForce();
					return false;
				}
				fnParticle.integrateForces(dT, type);
				fnParticle.updateParticle(true, false, false);
			}

			bool valid = true;
			for (int i = 0; i < vertexCount; ++i)
			{
				const zSpace::zPoint& point = positions[i];
				if (!std::isfinite(point.x) || !std::isfinite(point.y) || !std::isfinite(point.z))
				{
					valid = false;
					break;
				}

				const double dx = point.x - previous[i].x;
				const double dy = point.y - previous[i].y;
				const double dz = point.z - previous[i].z;
				const double displacement = std::sqrt((dx * dx) + (dy * dy) + (dz * dz));
				lastUpdateDisplacements[i] = displacement;
				if (!std::isfinite(displacement) || displacement > stepLimit)
				{
					valid = false;
					break;
				}
			}

			if (!valid)
			{
				for (int i = 0; i < vertexCount; ++i) positions[i] = previous[i];
				clearVelocityAndForce();
				return false;
			}

			zSpace::zFnMesh meshFn(*meshObj);
			meshFn.setVertexPositions(dynamicPositions);
			if (computeNormals) meshFn.computeMeshNormals();
			return true;
		}
	};

	WasmMeshDynamics gMeshDynamics;
	SolverParams gSolverParams;
	PlanaritySettings gPlanarity;
	double gPlanarityDeviations[3] = {0, 0, 0};
	zSpace::zVectorArray gPlanarityForces[3], gPlanarityResultant;
	bool evaluatePlanarity(bool apply)
	{
		gMeshDynamics.particleForces(true);
		bool satisfied = true, any = false;
		zSpace::zVectorArray previous(gMeshDynamics.dynamicPositions.size());
		for (int i = 0; i < 3; ++i) {
			gPlanarityDeviations[i] = 0;
			gPlanarityForces[i].assign(previous.size(), zSpace::zVector());
			if (!gPlanarity.enabled[i]) continue;
			any = true;
			zSpace::zDoubleArray deviations;
			zSpace::zVectorArray directions;
			bool done = false;
			if (i == 0) gMeshDynamics.addPlanarityForce(gPlanarity.strength[i], gPlanarity.tolerance[i],
				gPlanarity.quad ? zSpace::zQuadPlanar : zSpace::zVolumePlanar, deviations, directions, done,
				gSolverParams.fixXY ? zSpace::zConstraintXY : zSpace::zConstraintFree);
			if (i == 1) gMeshDynamics.addPlanarityForce_vertexgroups(gPlanarity.strength[i], gPlanarity.tolerance[i],
				gPlanarity.groups, gPlanarity.origins, gPlanarity.normals, deviations, directions, done);
			if (i == 2) gMeshDynamics.addRigidLineForce(gPlanarity.strength[i], gPlanarity.tolerance[i],
				gPlanarity.pairs, gPlanarity.lengths, deviations, directions, done);
			for (double d : deviations) {
				if (!std::isfinite(d) || d < 0) throw std::runtime_error("Unsupported or degenerate planarity constraint.");
				gPlanarityDeviations[i] = std::max(gPlanarityDeviations[i], d);
			}
			satisfied = satisfied && done;
			auto total = gMeshDynamics.particleForces();
			for (std::size_t v = 0; v < total.size(); ++v) gPlanarityForces[i][v] = total[v] - previous[v];
			previous = std::move(total);
		}
		gPlanarityResultant = std::move(previous);
		if (!apply) gMeshDynamics.particleForces(true);
		return any && satisfied;
	}
	zSpace::zPointArray gSolverInitialPositions;
	std::deque<zSpace::zPointArray> gSolverHistory;
	zSpace::zIntArray gSolverSupports;
	zSpace::zIntArray gSolverRemovedEdges;
	std::vector<unsigned char> gSolverSupportMask;
	bool gSolverReady = false;
	bool gSolverSupportUpdateBatch = false;
	bool gSolverConfigUpdateBatch = false;
	int gSolverFrame = 0;
	double gSolverMinResidual = 0.0;
	double gSolverMaxResidual = 0.0;
	double gSolverMaxAreaForceResidual = 0.0;
	int gSolverResidualCount = 0;
	bool gSolverEquilibriumReached = false;

	void appendVectorPrimitive(const zSpace::zPoint& origin, const zSpace::zVector& vector, const zSpace::zColor& color);

	bool isSolverSupportVertex(int vertexId)
	{
		if (vertexId >= 0 && vertexId < static_cast<int>(gSolverSupportMask.size()))
		{
			return gSolverSupportMask[vertexId] != 0;
		}
		return std::find(gSolverSupports.begin(), gSolverSupports.end(), vertexId) != gSolverSupports.end();
	}

	zSpace::zObjectMesh& activeMeshObject()
	{
		return gActiveMeshObject ? *gActiveMeshObject : gMeshObject;
	}

	void refreshSolverSupportMask()
	{
		gSolverSupportMask.clear();
		if (!gHasMesh) return;
		const auto& meshData = zSpace::zMeshObjectStorage::read(activeMeshObject());
		gSolverSupportMask.assign(std::max(0, meshData.numVertices()), 0);
		for (int vertexId : gSolverSupports)
		{
			if (vertexId < 0 || vertexId >= static_cast<int>(gSolverSupportMask.size())) continue;
			gSolverSupportMask[vertexId] = 1;
		}
	}

	void applyRemovedSolverEdges()
	{
		if (gSolverReady) gMeshDynamics.setInactiveEdges(gSolverRemovedEdges);
	}

	bool removeIntValue(zSpace::zIntArray& values, int value)
	{
		const auto oldSize = values.size();
		values.erase(std::remove(values.begin(), values.end(), value), values.end());
		return values.size() != oldSize;
	}

	void resetSolverState()
	{
		gPlanarity = {};
		gOrigamiCreases.clear();
		gOrigamiDiagnostics = {};
		gSolverReady = false;
		gMeshDynamics.clearParticles();
		gSolverFrame = 0;
		gSolverInitialPositions.clear();
		gSolverHistory.clear();
		gSolverSupports.clear();
		gSolverRemovedEdges.clear();
		gSolverSupportMask.clear();
		gSolverMinResidual = 0.0;
		gSolverMaxResidual = 0.0;
		gSolverResidualCount = 0;
		gSolverEquilibriumReached = false;
	}

	float ramp(float value, float minValue, float maxValue, int channel)
	{
		const float range = std::max(maxValue - minValue, 1.0e-6f);
		const float t = std::clamp((value - minValue) / range, 0.0f, 1.0f);
		if (channel == 0) return 0.05f + 0.72f * t;
		if (channel == 1) return 0.18f + 0.58f * (1.0f - std::abs(t - 0.5f) * 2.0f);
		return 0.82f - 0.5f * t;
	}

	std::string lowerExtension(const std::string& path)
	{
		const std::size_t dot = path.find_last_of('.');
		if (dot == std::string::npos) return "";
		std::string extension = path.substr(dot);
		std::transform(extension.begin(), extension.end(), extension.begin(),
			[](unsigned char value) { return static_cast<char>(std::tolower(value)); });
		return extension;
	}

	void clearMeshBuffers(RenderBuffers& buffers = gBuffers)
	{
		buffers.positions.clear();
		buffers.normals.clear();
		buffers.colors.clear();
		buffers.faceColors.clear();
		buffers.faceCenters.clear();
		buffers.faceNormals.clear();
		buffers.edgeCenters.clear();
		buffers.edgeWeights.clear();
		buffers.indices.clear();
		buffers.edgeIndices.clear();
		buffers.faceCounts.clear();
		buffers.faceConnects.clear();
		buffers.faceCount = 0;
	}

	void clearPrimitiveBuffers()
	{
		gBuffers.pointPositions.clear();
		gBuffers.pointColors.clear();
		gBuffers.pointSizes.clear();
		gBuffers.linePositions.clear();
		gBuffers.lineColors.clear();
		gBuffers.lineWeights.clear();
		gBuffers.vectorOrigins.clear();
		gBuffers.vectorDirections.clear();
		gBuffers.vectorColors.clear();
		gBuffers.vectorWeights.clear();
		gBuffers.polygonPositions.clear();
		gBuffers.polygonColors.clear();
		gBuffers.polygonIndices.clear();
	}

	zSpace::zIOResult installMeshData(zSpace::io_detail::MeshData& data, zSpace::zObjectMesh& object)
	{
		if (data.polygonCounts.empty() || data.polygonConnects.empty())
			return zSpace::zIOResult::error("Mesh data contains no polygons.");

		zSpace::zFnMesh meshFn(object);
		meshFn.create(data.positions, data.polygonCounts, data.polygonConnects);

		if (data.edgeConnects.size() % 2 == 0 && !data.edgeConnects.empty())
		{
			std::map<std::pair<int, int>, std::size_t> sourceEdges;
			for (std::size_t i = 0; i < data.edgeConnects.size(); i += 2)
			{
				const auto edge = std::minmax(data.edgeConnects[i], data.edgeConnects[i + 1]);
				sourceEdges[{ edge.first, edge.second }] = i / 2;
			}

			zSpace::zIntArray targetEdgeConnects;
			meshFn.getEdgeData(targetEdgeConnects);
			zSpace::zColorArray orderedColors;
			zSpace::zDoubleArray orderedWeights;
			for (std::size_t i = 0; i + 1 < targetEdgeConnects.size(); i += 2)
			{
				const auto endpoints = std::minmax(targetEdgeConnects[i], targetEdgeConnects[i + 1]);
				const auto source = sourceEdges.find({ endpoints.first, endpoints.second });
				if (source == sourceEdges.end()) continue;
				if (source->second < data.edgeColors.size())
					orderedColors.push_back(data.edgeColors[source->second]);
				if (source->second < data.edgeWeights.size())
					orderedWeights.push_back(data.edgeWeights[source->second]);
			}

			if (orderedColors.size() == static_cast<std::size_t>(meshFn.numEdges()))
				data.edgeColors = std::move(orderedColors);
			if (orderedWeights.size() == static_cast<std::size_t>(meshFn.numEdges()))
				data.edgeWeights = std::move(orderedWeights);
		}

		if (data.vertexColors.size() == data.positions.size())
			meshFn.setVertexColors(data.vertexColors);
		if (data.edgeColors.size() == static_cast<std::size_t>(meshFn.numEdges()))
			meshFn.setEdgeColors(data.edgeColors, false);
		if (data.edgeWeights.size() == static_cast<std::size_t>(meshFn.numEdges()))
			meshFn.setEdgeWeights(data.edgeWeights);
		if (data.faceColors.size() == data.polygonCounts.size())
			meshFn.setFaceColors(data.faceColors);
		if (data.faceNormals.size() == data.polygonCounts.size())
			meshFn.setFaceNormals(data.faceNormals);

		return zSpace::zIOResult::ok();
	}

	zSpace::zIOResult applyMeshData(zSpace::io_detail::MeshData& data)
	{
		const auto result = installMeshData(data, gMeshObject);
		if (!result) return result;
		gActiveMeshObject = &gMeshObject;
		gHasMesh = true;
		resetSolverState();
		return result;
	}

	zSpace::zIOResult extractMeshData(zSpace::io_detail::MeshData& data)
	{
		if (!gHasMesh) return zSpace::zIOResult::error("No active mesh.");

		zSpace::zFnMesh meshFn(activeMeshObject());
		data = {};
		meshFn.getVertexPositions(data.positions);
		meshFn.getFaceNormals(data.faceNormals);
		meshFn.getVertexColors(data.vertexColors);
		meshFn.getEdgeColors(data.edgeColors);
		meshFn.getEdgeWeights(data.edgeWeights);
		meshFn.getFaceColors(data.faceColors);

		for (std::size_t i = 0; i < data.edgeColors.size(); ++i)
		{
			const auto& color = data.edgeColors[i];
			const double weight = i < data.edgeWeights.size() ? data.edgeWeights[i] : 1.0;
			data.edgeAttributes.push_back({ color.r, color.g, color.b, color.a, weight });
		}

		meshFn.getPolygonData(data.polygonConnects, data.polygonCounts);
		meshFn.getEdgeData(data.edgeConnects);
		if (data.positions.empty() || data.polygonCounts.empty())
			return zSpace::zIOResult::error("Mesh contains no writable geometry.");
		return zSpace::zIOResult::ok();
	}

	void colorizeByHeight(const zSpace::zPointArray& meshPositions)
	{
		float minScalar = 1.0e9f;
		float maxScalar = -1.0e9f;
		for (const auto& position : meshPositions)
		{
			minScalar = std::min(minScalar, position.z);
			maxScalar = std::max(maxScalar, position.z);
		}

		for (const auto& position : meshPositions)
		{
			gBuffers.colors.push_back(ramp(position.z, minScalar, maxScalar, 0));
			gBuffers.colors.push_back(ramp(position.z, minScalar, maxScalar, 1));
			gBuffers.colors.push_back(ramp(position.z, minScalar, maxScalar, 2));
		}
	}

	void copyMeshToRenderBuffers(zSpace::zFnMesh& meshFn, const std::vector<float>& scalars, float minScalar, float maxScalar, RenderBuffers& buffers = gBuffers)
	{
		const auto& data = zSpace::zMeshObjectStorage::read(*meshFn.object());
		const auto& meshPositions = data.positions;
		const auto& meshNormals = data.vertexNormals;
		const auto& meshFaceNormals = data.faceNormals;
		const auto& meshColors = data.vertexColors;
		const auto& meshFaceColors = data.faceColors;
		const auto& polyConnects = data.faceVertexIndices;
		zSpace::zIntArray polyCounts;
		polyCounts.reserve(data.numFaces());
		for (int f = 0; f < data.numFaces(); ++f) polyCounts.push_back(data.faceOffsets[f + 1] - data.faceOffsets[f]);

		clearMeshBuffers(buffers);
		buffers.faceCount = static_cast<std::uint32_t>(polyCounts.size());

		buffers.positions.reserve(meshPositions.size() * 3);
		buffers.normals.reserve(meshPositions.size() * 3);
		buffers.colors.reserve(meshPositions.size() * 3);
		buffers.faceCenters.reserve(polyCounts.size() * 3);
		buffers.faceNormals.reserve(polyCounts.size() * 3);
		buffers.edgeIndices.reserve(data.numEdges() * 2);
		buffers.edgeCenters.reserve(data.numEdges() * 3);
		buffers.edgeWeights.reserve(data.numEdges());
		buffers.faceCounts.reserve(polyCounts.size());
		buffers.faceConnects.reserve(polyConnects.size());
		const bool hasFaceColors = meshFaceColors.size() == polyCounts.size();
		std::size_t triangleCount = 0;
		for (int count : polyCounts) if (count >= 3) triangleCount += count - 2;
		buffers.indices.reserve(triangleCount * 3);
		if (hasFaceColors) buffers.faceColors.reserve(triangleCount * 3);

		for (int count : polyCounts)
		{
			buffers.faceCounts.push_back(static_cast<std::uint32_t>(std::max(count, 0)));
		}
		for (int index : polyConnects)
		{
			buffers.faceConnects.push_back(static_cast<std::uint32_t>(std::max(index, 0)));
		}

		for (std::size_t i = 0; i < meshPositions.size(); ++i)
		{
			const zSpace::zPoint& position = meshPositions[i];
			buffers.positions.push_back(position.x);
			buffers.positions.push_back(position.y);
			buffers.positions.push_back(position.z);

			const zSpace::zVector normal = i < meshNormals.size() ? meshNormals[i] : zSpace::zVector(0, 0, 1);
			buffers.normals.push_back(normal.x);
			buffers.normals.push_back(normal.y);
			buffers.normals.push_back(normal.z);

			if (i < meshColors.size())
			{
				buffers.colors.push_back(meshColors[i].r);
				buffers.colors.push_back(meshColors[i].g);
				buffers.colors.push_back(meshColors[i].b);
			}
			else
			{
				const float scalar = i < scalars.size() ? scalars[i] : position.z;
				buffers.colors.push_back(ramp(scalar, minScalar, maxScalar, 0));
				buffers.colors.push_back(ramp(scalar, minScalar, maxScalar, 1));
				buffers.colors.push_back(ramp(scalar, minScalar, maxScalar, 2));
			}
		}

		std::size_t cursor = 0;
		for (std::size_t faceIndex = 0; faceIndex < polyCounts.size(); ++faceIndex)
		{
			const int count = polyCounts[faceIndex];
			if (count < 3 || cursor + static_cast<std::size_t>(count) > polyConnects.size())
			{
				cursor += std::max(count, 0);
				continue;
			}

			const int first = polyConnects[cursor];
			for (int i = 1; i < count - 1; ++i)
			{
				const int a = polyConnects[cursor + i];
				const int b = polyConnects[cursor + i + 1];
				if (first < 0 || a < 0 || b < 0) continue;
				if (first >= static_cast<int>(meshPositions.size()) ||
					a >= static_cast<int>(meshPositions.size()) ||
					b >= static_cast<int>(meshPositions.size())) continue;

				buffers.indices.push_back(static_cast<std::uint32_t>(first));
				buffers.indices.push_back(static_cast<std::uint32_t>(a));
				buffers.indices.push_back(static_cast<std::uint32_t>(b));

				if (hasFaceColors)
				{
					const zSpace::zColor& color = meshFaceColors[faceIndex];
					buffers.faceColors.push_back(color.r);
					buffers.faceColors.push_back(color.g);
					buffers.faceColors.push_back(color.b);
				}
			}
			cursor += static_cast<std::size_t>(count);
		}

		cursor = 0;
		for (std::size_t faceIndex = 0; faceIndex < polyCounts.size(); ++faceIndex)
		{
			const int count = polyCounts[faceIndex];
			if (count < 2 || cursor + static_cast<std::size_t>(count) > polyConnects.size())
			{
				cursor += std::max(count, 0);
				continue;
			}

			zSpace::zPoint center(0, 0, 0);
			for (int i = 0; i < count; ++i)
			{
				const int vertexIndex = polyConnects[cursor + i];
				if (vertexIndex >= 0 && static_cast<std::size_t>(vertexIndex) < meshPositions.size())
					center += meshPositions[vertexIndex];
			}
			center /= static_cast<float>(count);
			buffers.faceCenters.push_back(center.x);
			buffers.faceCenters.push_back(center.y);
			buffers.faceCenters.push_back(center.z);

			const zSpace::zVector faceNormal = faceIndex < meshFaceNormals.size()
				? meshFaceNormals[faceIndex]
				: zSpace::zVector(0, 0, 1);
			buffers.faceNormals.push_back(faceNormal.x);
			buffers.faceNormals.push_back(faceNormal.y);
			buffers.faceNormals.push_back(faceNormal.z);

			cursor += static_cast<std::size_t>(count);
		}

		// Flat storage already deduplicates in first-face encounter order and
		// records the original direction and number of corner uses.
		for (int edgeId = 0; edgeId < data.numEdges(); ++edgeId)
		{
			const int v0 = data.edgeVertexIndices[edgeId * 2], v1 = data.edgeVertexIndices[edgeId * 2 + 1];
			if (v0 < 0 || v0 == v1 || v1 >= static_cast<int>(meshPositions.size())) continue;
			const int a = data.edgeFirstVertices[edgeId], b = a == v0 ? v1 : v0;
			buffers.edgeIndices.push_back(static_cast<std::uint32_t>(a));
			buffers.edgeIndices.push_back(static_cast<std::uint32_t>(b));
			const double storedWeight = edgeId < static_cast<int>(data.edgeWeights.size()) ? data.edgeWeights[edgeId] : 1.0;
			const double displayWeight = std::abs(storedWeight - 1.0) > 1.0e-6
				? storedWeight : (data.edgeUseCounts[edgeId] <= 1 ? 3.0 : 1.0);
			buffers.edgeWeights.push_back(static_cast<float>(displayWeight));
			zSpace::zPoint start = meshPositions[a];
			const zSpace::zPoint center = (start + meshPositions[b]) * 0.5f;
			buffers.edgeCenters.push_back(center.x);
			buffers.edgeCenters.push_back(center.y);
			buffers.edgeCenters.push_back(center.z);
		}
	}

	void copyActiveMeshToRenderBuffers()
	{
		zSpace::zFnMesh meshFn(activeMeshObject());
		const auto& positions = zSpace::zMeshObjectStorage::read(activeMeshObject()).positions;

		float minScalar = 1.0e9f;
		float maxScalar = -1.0e9f;
		std::vector<float> scalars;
		scalars.reserve(positions.size());
		for (const auto& position : positions)
		{
			scalars.push_back(position.z);
			minScalar = std::min(minScalar, position.z);
			maxScalar = std::max(maxScalar, position.z);
		}
		copyMeshToRenderBuffers(meshFn, scalars, minScalar, maxScalar);
	}

	zSpace::zPointArray currentMeshPositions()
	{
		if (gSolverReady && !gMeshDynamics.dynamicPositions.empty())
			return gMeshDynamics.dynamicPositions;

		zSpace::zPointArray positions;
		zSpace::zFnMesh meshFn(activeMeshObject());
		meshFn.getVertexPositions(positions);
		return positions;
	}

	void restoreMeshPositions(const zSpace::zPointArray& positions)
	{
		gSolverReady = false;
		gMeshDynamics.clearParticles();
		zSpace::zFnMesh meshFn(activeMeshObject());
		zSpace::zPointArray restored = positions;
		meshFn.setVertexPositions(restored);
		meshFn.computeMeshNormals();
		copyActiveMeshToRenderBuffers();
		clearPrimitiveBuffers();
	}

	bool fastRestoreSolverPositions(const zSpace::zPointArray& positions)
	{
		if (!gSolverReady || !gMeshDynamics.restoreLinkedPositions(positions)) return false;

		zSpace::zFnMesh meshFn(activeMeshObject());
		zSpace::zPointArray restored = positions;
		meshFn.setVertexPositions(restored);
		meshFn.computeMeshNormals();
		copyActiveMeshToRenderBuffers();
		clearPrimitiveBuffers();
		gSolverEquilibriumReached = false;
		gSolverMinResidual = 0.0;
		gSolverMaxResidual = 0.0;
		gSolverResidualCount = 0;
		return true;
	}

	void applySolverParticleProperties()
	{
		if (!gSolverReady) return;
		const auto& meshData = zSpace::zMeshObjectStorage::read(activeMeshObject());
		const zSpace::zColorArray& colors = meshData.vertexColors;
		gMeshDynamics.setSupports(gSolverSupports);
		gMeshDynamics.setMassesFromVertexColors(gSolverParams.massMin, gSolverParams.massMax, colors);
	}

	void rebuildSolverDynamics()
	{
		if (!gHasMesh) throw std::runtime_error("No active mesh for dynamic relaxation.");
		gMeshDynamics.createLinked(activeMeshObject());
		gMeshDynamics.setInactiveEdges(gSolverRemovedEdges);
		if (gSolverParams.mode == 2)
		{
			gMeshDynamics.prepareOrigami();
			for (const auto& crease : gOrigamiCreases)
				gMeshDynamics.setOrigamiCrease(crease.first, crease.second.first, crease.second.second);
		}
		gSolverReady = true;
		refreshSolverSupportMask();
		applySolverParticleProperties();
		gMeshDynamics.clearVelocityAndForce();
	}

	zSpace::zVectorArray surfaceResultantForces()
	{
		auto forces = gMeshDynamics.previewSurfaceForces(gSolverParams.areaStrength,
			gSolverParams.edgeForceEnabled ? gSolverParams.springStiffness : 0.0,
			gSolverParams.edgeLengthMultiplier, gSolverParams.fixXY);
		zSpace::zVector direction = gSolverParams.direction;
		if (direction.length() <= 1.0e-9f) direction = zSpace::zVector(0, 0, -1);
		direction.normalize();
		for (std::size_t i = 0; i < forces.size(); ++i) {
			if (gSolverParams.gravityEnabled)
				forces[i] += direction * (gSolverParams.gravity * gMeshDynamics.getParticleMass(static_cast<int>(i)));
			if (gSolverParams.vectorForceEnabled) forces[i] += direction * gSolverParams.value;
		}
		return forces;
	}

	void updateSolverResidualDiagnostics(const zSpace::zPointArray& positions)
	{
		gSolverMaxAreaForceResidual = 0.0;
		gSolverMinResidual = 0.0;
		gSolverMaxResidual = 0.0;
		gSolverResidualCount = 0;
		gSolverEquilibriumReached = false;

		if (positions.empty()) return;
		if (gSolverParams.mode == 3 && gSolverReady) {
			const bool satisfied = evaluatePlanarity(false);
			double maxStep = 0;
			for (std::size_t i = 0; i < positions.size(); ++i) {
				if (isSolverSupportVertex(static_cast<int>(i))) continue;
				gSolverMaxResidual = std::max(gSolverMaxResidual, static_cast<double>(gPlanarityResultant[i].length()));
				if (i < gMeshDynamics.lastUpdateDisplacements.size()) maxStep = std::max(maxStep, gMeshDynamics.lastUpdateDisplacements[i]);
				++gSolverResidualCount;
			}
			gSolverEquilibriumReached = satisfied && maxStep < gSolverParams.stepTolerance && gSolverMaxResidual < gSolverParams.residualThreshold;
			return;
		}
		if (gSolverParams.mode == 2)
		{
			if (!gSolverReady) return;
			zSpace::zVectorArray forces;
			gMeshDynamics.getOrigamiForces(gOrigamiSettings, forces, gOrigamiDiagnostics);
			gSolverMaxResidual = gOrigamiDiagnostics.maxAngleError * 180.0 / 3.141592653589793;
			gSolverResidualCount = static_cast<int>(positions.size());
			gSolverEquilibriumReached = gSolverMaxResidual < gSolverParams.residualThreshold
				&& gOrigamiDiagnostics.maxStrain < 0.01
				&& gOrigamiDiagnostics.maxSpeed < 0.001 * gMeshDynamics.cachedMeshScale;
			return;
		}
		if (gSolverParams.mode == 1)
		{
			if (!gSolverReady) return;
			auto forces = surfaceResultantForces();
			bool hasResidual = false;
			double minResidual = 0.0;
			double maxResidual = 0.0;
			double maxStep = 0.0;
			for (std::size_t vertexId = 0; vertexId < positions.size(); ++vertexId)
			{
				if (isSolverSupportVertex(static_cast<int>(vertexId))) continue;
				const double residual = forces[vertexId].length();
				if (!std::isfinite(residual)) return;
				if (vertexId < gMeshDynamics.lastUpdateDisplacements.size())
					maxStep = std::max(maxStep, gMeshDynamics.lastUpdateDisplacements[vertexId]);
				if (!hasResidual)
				{
					minResidual = residual;
					maxResidual = residual;
					hasResidual = true;
				}
				else
				{
					minResidual = std::min(minResidual, residual);
					maxResidual = std::max(maxResidual, residual);
				}
				++gSolverResidualCount;
			}
			if (!hasResidual) return;
			gSolverMinResidual = minResidual;
			gSolverMaxResidual = maxResidual;
			gSolverEquilibriumReached = maxResidual < std::max(0.0, gSolverParams.residualThreshold)
				&& maxStep < gSolverParams.stepTolerance;
			return;
		}
		if ((!gSolverParams.gravityEnabled || std::abs(gSolverParams.gravity) <= 1.0e-12) &&
			(!gSolverParams.vectorForceEnabled || std::abs(gSolverParams.value) <= 1.0e-12) &&
			(!gSolverParams.edgeForceEnabled || std::abs(gSolverParams.springStiffness) <= 1.0e-12)) return;

		zSpace::zVector gravityVector = gSolverParams.direction;
		if (gravityVector.length() <= 1.0e-9f) gravityVector = zSpace::zVector(0, 0, -1);
		gravityVector.normalize();

		zSpace::zVectorArray vertexEdgeForces;
		vertexEdgeForces.assign(positions.size(), zSpace::zVector(0, 0, 0));

		for (int edgeId = 0; edgeId < static_cast<int>(gMeshDynamics.edgePairs.size()); ++edgeId)
		{
			if (!gMeshDynamics.isEdgeActive(edgeId)) continue;
			const int a = gMeshDynamics.edgePairs[edgeId].a;
			const int b = gMeshDynamics.edgePairs[edgeId].b;
			if (a < 0 || b < 0 || a >= static_cast<int>(positions.size()) || b >= static_cast<int>(positions.size())) continue;

			zSpace::zVector edgeVector(
				positions[b].x - positions[a].x,
				positions[b].y - positions[a].y,
				positions[b].z - positions[a].z
			);
			const double edgeLength = edgeVector.length();
			if (edgeLength <= 1.0e-9) continue;
			edgeVector.normalize();
			const double targetLength = gMeshDynamics.springRestLength(
				edgeId,
				edgeLength,
				gSolverParams.edgeLengthMultiplier
			);
			const double extension = edgeLength - targetLength;
			if (std::abs(extension) <= std::max(1.0e-9, gMeshDynamics.cachedMeshScale * 1.0e-9)) continue;
			zSpace::zVector force = edgeVector * static_cast<float>(gSolverParams.springStiffness * extension);
			zSpace::zVector oppositeForce = force * -1.0f;
			applySolverForceConstraint(force, gSolverParams.fixXY ? zSpace::zConstraintXY : zSpace::zConstraintFree);
			applySolverForceConstraint(oppositeForce, gSolverParams.fixXY ? zSpace::zConstraintXY : zSpace::zConstraintFree);
			vertexEdgeForces[a] += force;
			vertexEdgeForces[b] += oppositeForce;
		}

		bool hasResidual = false;
		double minResidual = 0.0;
		double maxResidual = 0.0;
		for (std::size_t vertexId = 0; vertexId < vertexEdgeForces.size(); ++vertexId)
		{
			if (isSolverSupportVertex(static_cast<int>(vertexId))) continue;

			zSpace::zVector gravityForce(0, 0, 0);
			const double mass = gSolverReady
				? gMeshDynamics.getParticleMass(static_cast<int>(vertexId))
				: std::max(1.0e-6, gSolverParams.massMax);
			if (gSolverParams.gravityEnabled)
				gravityForce = gravityVector * static_cast<float>(gSolverParams.gravity * mass);

			zSpace::zVector vectorForce(0, 0, 0);
			if (gSolverParams.vectorForceEnabled && std::abs(gSolverParams.value) > 1.0e-12)
				vectorForce = gravityVector * static_cast<float>(gSolverParams.value);

			zSpace::zVector edgeForce = gSolverParams.edgeForceEnabled
				? vertexEdgeForces[vertexId]
				: zSpace::zVector(0, 0, 0);

			const double gravityMagnitude = gravityForce.length();
			const double vectorMagnitude = vectorForce.length();
			const double edgeMagnitude = edgeForce.length();
			const double forceScale = gravityMagnitude + vectorMagnitude + edgeMagnitude;
			if (!std::isfinite(forceScale) || forceScale <= 1.0e-18) continue;

			zSpace::zVector resultantForce = gravityForce + vectorForce + edgeForce;
			const double residual = resultantForce.length() / forceScale;

			if (!std::isfinite(residual)) continue;
			if (!hasResidual)
			{
				minResidual = residual;
				maxResidual = residual;
				hasResidual = true;
			}
			else
			{
				minResidual = std::min(minResidual, residual);
				maxResidual = std::max(maxResidual, residual);
			}
			++gSolverResidualCount;
		}

		if (!hasResidual) return;
		gSolverMinResidual = minResidual;
		gSolverMaxResidual = maxResidual;
		gSolverEquilibriumReached = maxResidual < std::max(0.0, gSolverParams.residualThreshold);
	}

	void copySolverPreviewToPrimitiveBuffers(bool updateResiduals = true)
	{
		clearPrimitiveBuffers();
		if (!gHasMesh) return;

		zSpace::zPointArray positions;
		positions = currentMeshPositions();
		if (updateResiduals) updateSolverResidualDiagnostics(positions);

		const zSpace::zColor supportColor(0, 0, 0, 1);
		const zSpace::zColor resultantColor(1.0f, 0.72f, 0.0f, 1.0f);
		const zSpace::zColor springColor(1.0f, 0.08f, 0.02f, 1.0f);
		for (int vertexId : gSolverSupports)
		{
			if (vertexId < 0 || vertexId >= static_cast<int>(positions.size())) continue;
			const zSpace::zPoint& point = positions[vertexId];
			gBuffers.pointPositions.push_back(point.x);
			gBuffers.pointPositions.push_back(point.y);
			gBuffers.pointPositions.push_back(point.z);
			gBuffers.pointColors.push_back(supportColor.r);
			gBuffers.pointColors.push_back(supportColor.g);
			gBuffers.pointColors.push_back(supportColor.b);
			gBuffers.pointSizes.push_back(static_cast<float>(gSolverParams.supportSize));
		}

		const float displayLengthScale = static_cast<float>(std::max(0.0, gSolverParams.vectorScale));
		auto drawForces = [&](const zSpace::zVectorArray& forces, const zSpace::zColor& color) {
			for (std::size_t i=0;i<forces.size();++i) {
				if (isSolverSupportVertex(static_cast<int>(i))) continue;
				zSpace::zVector force=forces[i];force*=displayLengthScale;
				if (std::isfinite(force.x)&&std::isfinite(force.y)&&std::isfinite(force.z)&&force.length()>1.0e-9)
					appendVectorPrimitive(positions[i],force,color);
			}
		};
		if (gSolverParams.mode == 2)
		{
			const auto& data = zSpace::zMeshObjectStorage::read(activeMeshObject());
			for (const auto& entry : gOrigamiCreases)
			{
				if (entry.second.first == 0) continue;
				const std::size_t offset = static_cast<std::size_t>(entry.first) * 2;
				if (offset + 1 >= data.edgeVertexIndices.size()) continue;
				for (int endpoint = 0; endpoint < 2; ++endpoint)
				{
					const auto& p = positions[data.edgeVertexIndices[offset + endpoint]];
					gBuffers.linePositions.insert(gBuffers.linePositions.end(), {p.x,p.y,p.z});
					const bool valley = entry.second.first == 1;
					if (entry.second.first == 2) gBuffers.lineColors.insert(gBuffers.lineColors.end(), {0.5f,0.5f,0.5f});
					else gBuffers.lineColors.insert(gBuffers.lineColors.end(), {valley?0.05f:1.0f,0.1f,valley?1.0f:0.1f});
				}
				gBuffers.lineWeights.push_back(3.0f);
			}
			if (gSolverReady && gSolverParams.displayForceVectors && displayLengthScale > 0)
			{
				zSpace::zVectorArray forces;
				zSpace::zOrigamiForceComponents components;
				gMeshDynamics.getOrigamiForces(gOrigamiSettings, forces, gOrigamiDiagnostics, &components);
				drawForces(components.axial,springColor);
				drawForces(components.crease,zSpace::zColor(1,0,0.65f,1));
				drawForces(components.facet,zSpace::zColor(0.55f,0.2f,0.9f,1));
				drawForces(components.face,zSpace::zColor(0,0.65f,0.65f,1));
				drawForces(components.damping,zSpace::zColor(0.45f,0.45f,0.45f,1));
				drawForces(forces,resultantColor);
			}
			return;
		}
		if (!gSolverParams.displayForceVectors || displayLengthScale <= 1.0e-9f || positions.empty()) return;
		if (gSolverParams.mode == 3) {
			if (!gSolverReady) return;
			drawForces(gPlanarityForces[0], zSpace::zColor(0,0.75f,1,1));
			drawForces(gPlanarityForces[1], zSpace::zColor(0.7f,0.2f,0.9f,1));
			drawForces(gPlanarityForces[2], springColor);
			if (gSolverParams.residualForceEnabled) drawForces(gPlanarityResultant, resultantColor);
			return;
		}

		if (gSolverParams.mode == 1)
		{
			if (!gSolverReady) return;
			drawForces(gMeshDynamics.previewSurfaceForces(gSolverParams.areaStrength,0,1,false),zSpace::zColor(0,0.75f,1,1));
			if (gSolverParams.residualForceEnabled) drawForces(surfaceResultantForces(),resultantColor);
		}

		if (gSolverParams.gravityEnabled && std::abs(gSolverParams.gravity) > 1.0e-12)
		{
			const zSpace::zColor gravityColor(0.05f, 0.20f, 1.0f, 1.0f);
			zSpace::zVector gravityVector = gSolverParams.direction;
			if (gravityVector.length() <= 1.0e-9f) gravityVector = zSpace::zVector(0, 0, -1);
			gravityVector.normalize();
			for (std::size_t vertexId = 0; vertexId < positions.size(); ++vertexId)
			{
				if (isSolverSupportVertex(static_cast<int>(vertexId))) continue;
				const double mass = gSolverReady
					? gMeshDynamics.getParticleMass(static_cast<int>(vertexId))
					: std::max(1.0e-6, gSolverParams.massMax);
				zSpace::zVector force = gravityVector * static_cast<float>(gSolverParams.gravity * mass * displayLengthScale);
				appendVectorPrimitive(positions[vertexId], force, gravityColor);
			}
		}

		if (gSolverParams.vectorForceEnabled && std::abs(gSolverParams.value) > 1.0e-12)
		{
			const zSpace::zColor vectorColor(0.0f, 0.72f, 0.26f, 1.0f);
			zSpace::zVector vectorForce = gSolverParams.direction;
			if (vectorForce.length() <= 1.0e-9f) vectorForce = zSpace::zVector(0, 0, -1);
			vectorForce.normalize();
			vectorForce *= static_cast<float>(gSolverParams.value * displayLengthScale);
			for (std::size_t vertexId = 0; vertexId < positions.size(); ++vertexId)
			{
				if (isSolverSupportVertex(static_cast<int>(vertexId))) continue;
				appendVectorPrimitive(positions[vertexId], vectorForce, vectorColor);
			}
		}

		if (gSolverParams.edgeForceEnabled && std::abs(gSolverParams.springStiffness) > 1.0e-12)
		{
			const zSpace::zColor edgeColor = springColor;
			zSpace::zVectorArray vertexForces;
			vertexForces.assign(positions.size(), zSpace::zVector(0, 0, 0));

			for (int edgeId = 0; edgeId < static_cast<int>(gMeshDynamics.edgePairs.size()); ++edgeId)
			{
				if (!gMeshDynamics.isEdgeActive(edgeId)) continue;
				const int a = gMeshDynamics.edgePairs[edgeId].a;
				const int b = gMeshDynamics.edgePairs[edgeId].b;
				if (a < 0 || b < 0 || a >= static_cast<int>(positions.size()) || b >= static_cast<int>(positions.size())) continue;

				zSpace::zVector edgeVector = positions[b] - positions[a];
				const double edgeLength = edgeVector.length();
				if (edgeLength <= 1.0e-9) continue;
				edgeVector.normalize();
				const double targetLength = gMeshDynamics.springRestLength(
					edgeId,
					edgeLength,
					gSolverParams.edgeLengthMultiplier
				);
				const double extension = edgeLength - targetLength;
				if (std::abs(extension) <= std::max(1.0e-9, gMeshDynamics.cachedMeshScale * 1.0e-9)) continue;
				zSpace::zVector force = edgeVector * static_cast<float>(gSolverParams.springStiffness * extension);
				zSpace::zVector oppositeForce = force * -1.0f;
				applySolverForceConstraint(force, gSolverParams.fixXY ? zSpace::zConstraintXY : zSpace::zConstraintFree);
				applySolverForceConstraint(oppositeForce, gSolverParams.fixXY ? zSpace::zConstraintXY : zSpace::zConstraintFree);
				vertexForces[a] += force;
				vertexForces[b] += oppositeForce;
			}

			for (std::size_t i = 0; i < positions.size(); ++i)
			{
				if (isSolverSupportVertex(static_cast<int>(i))) continue;
				zSpace::zVector force = vertexForces[i];
				if (force.length() <= 1.0e-9) continue;
				force *= displayLengthScale;
				appendVectorPrimitive(positions[i], force, edgeColor);
			}
		}

		if (gSolverParams.residualForceEnabled && gSolverParams.mode != 1)
		{
			const zSpace::zColor residualColor = resultantColor;
			zSpace::zVectorArray residualForces;
			residualForces.assign(positions.size(), zSpace::zVector(0, 0, 0));

			zSpace::zVector unitDirection = gSolverParams.direction;
			if (unitDirection.length() <= 1.0e-9f) unitDirection = zSpace::zVector(0, 0, -1);
			unitDirection.normalize();

			for (std::size_t vertexId = 0; vertexId < positions.size(); ++vertexId)
			{
				if (isSolverSupportVertex(static_cast<int>(vertexId))) continue;
				if (gSolverParams.gravityEnabled && std::abs(gSolverParams.gravity) > 1.0e-12)
				{
					const double mass = gSolverReady
						? gMeshDynamics.getParticleMass(static_cast<int>(vertexId))
						: std::max(1.0e-6, gSolverParams.massMax);
					residualForces[vertexId] += unitDirection * static_cast<float>(gSolverParams.gravity * mass);
				}
				if (gSolverParams.vectorForceEnabled && std::abs(gSolverParams.value) > 1.0e-12)
					residualForces[vertexId] += unitDirection * static_cast<float>(gSolverParams.value);
			}

			if (gSolverParams.edgeForceEnabled && std::abs(gSolverParams.springStiffness) > 1.0e-12)
			{
				for (int edgeId = 0; edgeId < static_cast<int>(gMeshDynamics.edgePairs.size()); ++edgeId)
				{
					if (!gMeshDynamics.isEdgeActive(edgeId)) continue;
					const int a = gMeshDynamics.edgePairs[edgeId].a;
					const int b = gMeshDynamics.edgePairs[edgeId].b;
					if (a < 0 || b < 0 || a >= static_cast<int>(positions.size()) || b >= static_cast<int>(positions.size())) continue;

					zSpace::zVector edgeVector = positions[b] - positions[a];
					const double edgeLength = edgeVector.length();
					if (edgeLength <= 1.0e-9) continue;
					edgeVector.normalize();
					const double targetLength = gMeshDynamics.springRestLength(
						edgeId,
						edgeLength,
						gSolverParams.edgeLengthMultiplier
					);
					const double extension = edgeLength - targetLength;
					if (std::abs(extension) <= std::max(1.0e-9, gMeshDynamics.cachedMeshScale * 1.0e-9)) continue;
					zSpace::zVector force = edgeVector * static_cast<float>(gSolverParams.springStiffness * extension);
					zSpace::zVector oppositeForce = force * -1.0f;
					applySolverForceConstraint(force, gSolverParams.fixXY ? zSpace::zConstraintXY : zSpace::zConstraintFree);
					applySolverForceConstraint(oppositeForce, gSolverParams.fixXY ? zSpace::zConstraintXY : zSpace::zConstraintFree);
					residualForces[a] += force;
					residualForces[b] += oppositeForce;
				}
			}

			for (std::size_t vertexId = 0; vertexId < residualForces.size(); ++vertexId)
			{
				if (isSolverSupportVertex(static_cast<int>(vertexId))) continue;
				zSpace::zVector residual = residualForces[vertexId];
				if (residual.length() <= 1.0e-9) continue;
				residual *= displayLengthScale;
				appendVectorPrimitive(positions[vertexId], residual, residualColor);
			}
		}
	}

	void copySketchPrimitivesToRenderBuffers()
	{
		clearPrimitiveBuffers();

		const auto& points = gSketchScene.points();
		gBuffers.pointPositions.reserve(points.size() * 3);
		gBuffers.pointColors.reserve(points.size() * 3);
		gBuffers.pointSizes.reserve(points.size());
		for (const auto& point : points)
		{
			gBuffers.pointPositions.push_back(point.position.x);
			gBuffers.pointPositions.push_back(point.position.y);
			gBuffers.pointPositions.push_back(point.position.z);
			gBuffers.pointColors.push_back(point.color.r);
			gBuffers.pointColors.push_back(point.color.g);
			gBuffers.pointColors.push_back(point.color.b);
			gBuffers.pointSizes.push_back(point.size);
		}

		const auto& lines = gSketchScene.lines();
		gBuffers.linePositions.reserve(lines.size() * 6);
		gBuffers.lineColors.reserve(lines.size() * 6);
		gBuffers.lineWeights.reserve(lines.size());
		for (const auto& line : lines)
		{
			gBuffers.linePositions.push_back(line.start.x);
			gBuffers.linePositions.push_back(line.start.y);
			gBuffers.linePositions.push_back(line.start.z);
			gBuffers.linePositions.push_back(line.end.x);
			gBuffers.linePositions.push_back(line.end.y);
			gBuffers.linePositions.push_back(line.end.z);
			for (int i = 0; i < 2; ++i)
			{
				gBuffers.lineColors.push_back(line.color.r);
				gBuffers.lineColors.push_back(line.color.g);
				gBuffers.lineColors.push_back(line.color.b);
			}
			gBuffers.lineWeights.push_back(line.weight);
		}

		const auto& vectors = gSketchScene.vectors();
		gBuffers.vectorOrigins.reserve(vectors.size() * 3);
		gBuffers.vectorDirections.reserve(vectors.size() * 3);
		gBuffers.vectorColors.reserve(vectors.size() * 3);
		gBuffers.vectorWeights.reserve(vectors.size());
		for (const auto& vector : vectors)
		{
			gBuffers.vectorOrigins.push_back(vector.origin.x);
			gBuffers.vectorOrigins.push_back(vector.origin.y);
			gBuffers.vectorOrigins.push_back(vector.origin.z);
			gBuffers.vectorDirections.push_back(vector.vector.x);
			gBuffers.vectorDirections.push_back(vector.vector.y);
			gBuffers.vectorDirections.push_back(vector.vector.z);
			gBuffers.vectorColors.push_back(vector.color.r);
			gBuffers.vectorColors.push_back(vector.color.g);
			gBuffers.vectorColors.push_back(vector.color.b);
			gBuffers.vectorWeights.push_back(vector.weight);
		}

		const auto& polygons = gSketchScene.polygons();
		std::size_t polygonVertexCount = 0;
		std::size_t polygonTriangleCount = 0;
		for (const auto& polygon : polygons)
		{
			if (polygon.positions.size() < 3) continue;
			polygonVertexCount += polygon.positions.size();
			polygonTriangleCount += polygon.positions.size() - 2;
		}
		gBuffers.polygonPositions.reserve(polygonVertexCount * 3);
		gBuffers.polygonColors.reserve(polygonVertexCount * 3);
		gBuffers.polygonIndices.reserve(polygonTriangleCount * 3);
		for (const auto& polygon : polygons)
		{
			if (polygon.positions.size() < 3) continue;
			const std::uint32_t baseIndex = static_cast<std::uint32_t>(gBuffers.polygonPositions.size() / 3);
			for (const auto& position : polygon.positions)
			{
				gBuffers.polygonPositions.push_back(position.x);
				gBuffers.polygonPositions.push_back(position.y);
				gBuffers.polygonPositions.push_back(position.z);
				gBuffers.polygonColors.push_back(polygon.color.r);
				gBuffers.polygonColors.push_back(polygon.color.g);
				gBuffers.polygonColors.push_back(polygon.color.b);
			}
			for (std::uint32_t i = 1; i + 1 < polygon.positions.size(); ++i)
			{
				gBuffers.polygonIndices.push_back(baseIndex);
				gBuffers.polygonIndices.push_back(baseIndex + i);
				gBuffers.polygonIndices.push_back(baseIndex + i + 1);
			}
		}
	}

	zSpace::zColor colorFromSignedValue(
		double value,
		double threshold,
		double maxAbsValue,
		const zSpace::zColor& negativeColor,
		const zSpace::zColor& zeroColor,
		const zSpace::zColor& positiveColor)
	{
		if (std::abs(value) <= threshold) return zeroColor;
		const double range = std::max(maxAbsValue - threshold, 1.0e-9);
		const float t = static_cast<float>(std::clamp((std::abs(value) - threshold) / range, 0.0, 1.0));
		const zSpace::zColor& target = value > 0.0 ? positiveColor : negativeColor;
		return zSpace::zColor(
			zeroColor.r + (target.r - zeroColor.r) * t,
			zeroColor.g + (target.g - zeroColor.g) * t,
			zeroColor.b + (target.b - zeroColor.b) * t,
			1.0f);
	}

	double averageMeshEdgeLength(zSpace::zFnMesh& meshFn)
	{
		zSpace::zPointArray positions;
		zSpace::zIntArray edgeConnects;
		meshFn.getVertexPositions(positions);
		meshFn.getEdgeData(edgeConnects);
		double total = 0.0;
		int count = 0;
		for (std::size_t i = 0; i + 1 < edgeConnects.size(); i += 2)
		{
			const int a = edgeConnects[i];
			const int b = edgeConnects[i + 1];
			if (a < 0 || b < 0 || a >= static_cast<int>(positions.size()) || b >= static_cast<int>(positions.size())) continue;
			total += positions[a].distanceTo(positions[b]);
			++count;
		}
		return count > 0 ? total / static_cast<double>(count) : 1.0;
	}

	void appendVectorPrimitive(const zSpace::zPoint& origin, const zSpace::zVector& vector, const zSpace::zColor& color)
	{
		gBuffers.vectorOrigins.push_back(origin.x);
		gBuffers.vectorOrigins.push_back(origin.y);
		gBuffers.vectorOrigins.push_back(origin.z);
		gBuffers.vectorDirections.push_back(vector.x);
		gBuffers.vectorDirections.push_back(vector.y);
		gBuffers.vectorDirections.push_back(vector.z);
		gBuffers.vectorColors.push_back(color.r);
		gBuffers.vectorColors.push_back(color.g);
		gBuffers.vectorColors.push_back(color.b);
		gBuffers.vectorWeights.push_back(1.0f);
	}
}

extern "C"
{
	ZSPACE_WASM_EXPORT int zspace_sketch_set_param(const char* name, double value)
	{
		try
		{
			if (!name || !name[0]) throw std::runtime_error("Missing sketch parameter name.");
			gBuffers.lastError.clear();
			zspace_live::sketch::setParam(name, value);
			return 1;
		}
		catch (const std::exception& error)
		{
			gBuffers.lastError = error.what();
		}
		catch (...)
		{
			gBuffers.lastError = "Unknown zSpace Core WASM sketch parameter error.";
		}

		return 0;
	}

	ZSPACE_WASM_EXPORT int zspace_sketch_build()
	{
		try
		{
			gBuffers.lastError.clear();
			gSketchScene.clear();
			if (!gSketchSetupDone)
			{
				zspace_live::sketch::setup();
				gSketchSetupDone = true;
			}
			zspace_live::sketch::update(0.0f);
			zspace_live::sketch::draw(gSketchScene);
			if (!gSketchScene.hasDrawables()) throw std::runtime_error("Sketch draw() did not submit geometry.");

			if (gSketchScene.mesh())
			{
				gActiveMeshObject = gSketchScene.mesh();
				gHasMesh = true;
				copyActiveMeshToRenderBuffers();
			}
			else
			{
				gHasMesh = false;
				clearMeshBuffers();
			}

			copySketchPrimitivesToRenderBuffers();
			return 1;
		}
		catch (const std::exception& error)
		{
			gBuffers.lastError = error.what();
		}
		catch (...)
		{
			gBuffers.lastError = "Unknown zSpace Core WASM sketch error.";
		}

		gHasMesh = false;
		clearMeshBuffers();
		clearPrimitiveBuffers();
		return 0;
	}

	ZSPACE_WASM_EXPORT void* zspace_alloc(std::size_t bytes) { return std::malloc(bytes); }
	ZSPACE_WASM_EXPORT void zspace_free(void* pointer) { std::free(pointer); }

	ZSPACE_WASM_EXPORT int zspace_mesh_set_from_buffers(const float* positions, int positionCount,
		const std::uint32_t* counts, int faceCount, const std::uint32_t* connects, int connectCount)
	{
		try {
			gBuffers.lastError.clear();
			if (!positions || !counts || !connects || positionCount <= 0 || positionCount % 3 || faceCount <= 0 || connectCount <= 0)
				throw std::invalid_argument("Invalid mesh buffers.");
			zSpace::io_detail::MeshData data;
			data.positions.reserve(positionCount / 3);
			for (int i = 0; i < positionCount; i += 3) {
				for (int j = 0; j < 3; ++j) if (!std::isfinite(positions[i + j])) throw std::invalid_argument("Invalid mesh position.");
				data.positions.emplace_back(positions[i], positions[i + 1], positions[i + 2]);
			}
			std::size_t total = 0;
			for (int i = 0; i < faceCount; ++i) {
				if (counts[i] < 3 || counts[i] > static_cast<std::uint32_t>(connectCount)) throw std::invalid_argument("Invalid mesh face size.");
				total += counts[i];
				data.polygonCounts.push_back(static_cast<int>(counts[i]));
			}
			if (total != static_cast<std::size_t>(connectCount)) throw std::invalid_argument("Mesh connectivity length mismatch.");
			for (int i = 0; i < connectCount; ++i) {
				if (connects[i] >= data.positions.size()) throw std::invalid_argument("Mesh vertex ID out of range.");
				data.polygonConnects.push_back(static_cast<int>(connects[i]));
			}
			const auto result = applyMeshData(data);
			if (!result) throw std::runtime_error(result.message());
			clearPrimitiveBuffers();
			copyActiveMeshToRenderBuffers();
			return 1;
		} catch (const std::exception& e) { gBuffers.lastError = e.what(); return 0; }
	}

	ZSPACE_WASM_EXPORT int zspace_mesh_graph_from_buffers(const float* positions, int positionCount,
		const std::uint32_t* edges, int edgeIndexCount, double offset)
	{
		try {
			gBuffers.lastError.clear();
			if (!positions || !edges || positionCount<=0 || positionCount%3 || edgeIndexCount<=0 || edgeIndexCount%2)
				throw std::invalid_argument("Invalid graph buffers.");
			zSpace::zPointArray points; zSpace::zIntArray connects;
			for(int i=0;i<positionCount;i+=3) {
				if(!std::isfinite(positions[i])||!std::isfinite(positions[i+1])||!std::isfinite(positions[i+2]))
					throw std::invalid_argument("Graph positions must be finite.");
				points.emplace_back(positions[i],positions[i+1],positions[i+2]);
			}
			for(int i=0;i<edgeIndexCount;++i) {
				if(edges[i]>=points.size())throw std::invalid_argument("Graph vertex ID out of range.");
				connects.push_back(int(edges[i]));
			}
			zSpace::zObjectGraph graph;zSpace::zFnGraph(graph).create(points,connects);
			zSpace::zObjectMesh candidate;zSpace::zFnMesh(candidate).graphMesh(graph,offset);
			activeMeshObject()=std::move(candidate);resetSolverState();gHasMesh=true;
			clearPrimitiveBuffers();copyActiveMeshToRenderBuffers();return 1;
		} catch(const std::exception& e) {gBuffers.lastError=e.what();return 0;}
	}

	ZSPACE_WASM_EXPORT int zspace_mesh_read(const char* path)
	{
		try
		{
			if (!path || !path[0]) throw std::runtime_error("Missing mesh path.");
			gBuffers.lastError.clear();

			const std::string inputPath(path);
			zSpace::zIOResult result = zSpace::zIO::readMesh(inputPath, activeMeshObject());

			if (!result) throw std::runtime_error(result.message());
			resetSolverState();
			gHasMesh = true;
			clearPrimitiveBuffers();
			copyActiveMeshToRenderBuffers();
			return 1;
		}
		catch (const std::exception& error)
		{
			gBuffers.lastError = error.what();
		}
		catch (...)
		{
			gBuffers.lastError = "Unknown zSpace mesh read error.";
		}
		return 0;
	}

	ZSPACE_WASM_EXPORT int zspace_mesh_smooth(int iterations)
	{
		try
		{
			if (!gHasMesh) throw std::runtime_error("No active mesh to smooth.");
			gBuffers.lastError.clear();

			zSpace::zFnMesh meshFn(activeMeshObject());
			ZSPACE_MESH_TIMER(smoothTimer);
			meshFn.smoothMesh(std::clamp(iterations, 1, 6), false);
			ZSPACE_MESH_STAGE(smoothTimer, "smooth-call", meshFn.numPolygons());
			// smoothMesh leaves final normals ready; preserve the empty-mesh path.
			if (meshFn.numPolygons() == 0 || meshFn.numVertices() == 0) meshFn.computeMeshNormals();
			copyActiveMeshToRenderBuffers();
			ZSPACE_MESH_STAGE(smoothTimer, "render-export", meshFn.numPolygons());
			return 1;
		}
		catch (const std::exception& error)
		{
			gBuffers.lastError = error.what();
		}
		catch (...)
		{
			gBuffers.lastError = "Unknown zSpace mesh smoothing error.";
		}
		return 0;
	}

	ZSPACE_WASM_EXPORT int zspace_mesh_triangulate()
	{
		try
		{
			if (!gHasMesh) throw std::runtime_error("No active mesh to triangulate.");
			gBuffers.lastError.clear();

			zSpace::zFnMesh meshFn(activeMeshObject());
			meshFn.triangulate();
			meshFn.computeMeshNormals();
			copyActiveMeshToRenderBuffers();
			clearPrimitiveBuffers();
			return 1;
		}
		catch (const std::exception& error)
		{
			gBuffers.lastError = error.what();
		}
		catch (...)
		{
			gBuffers.lastError = "Unknown zSpace mesh triangulation error.";
		}
		return 0;
	}

	ZSPACE_WASM_EXPORT int zspace_mesh_planarity_analysis(
		int method,
		double threshold,
		float planarR, float planarG, float planarB,
		float nonPlanarR, float nonPlanarG, float nonPlanarB)
	{
		try
		{
			if (!gHasMesh) throw std::runtime_error("No active mesh for planarity analysis.");
			gBuffers.lastError.clear();
			gBuffers.analysisCount = 0;

			zSpace::zFnMesh meshFn(activeMeshObject());
			zSpace::zDoubleArray deviations;
			const zSpace::zPlanarSolverType solverType = method == 1 ? zSpace::zVolumePlanar : zSpace::zQuadPlanar;
			meshFn.getPlanarityDeviationPerFace(deviations, solverType, false, std::max(0.0, threshold));
			if (deviations.empty()) throw std::runtime_error("zFnMesh returned no planarity deviations.");

			const zSpace::zColor planarColor(planarR, planarG, planarB, 1.0f);
			const zSpace::zColor nonPlanarColor(nonPlanarR, nonPlanarG, nonPlanarB, 1.0f);
			zSpace::zColorArray colors;
			colors.reserve(deviations.size());
			for (double deviation : deviations)
			{
				const bool isPlanar = deviation >= 0.0 && deviation <= std::max(0.0, threshold);
				if (!isPlanar) ++gBuffers.analysisCount;
				colors.push_back(isPlanar ? planarColor : nonPlanarColor);
			}

			meshFn.setFaceColors(colors, false);
			meshFn.computeMeshNormals();
			copyActiveMeshToRenderBuffers();
			clearPrimitiveBuffers();
			return 1;
		}
		catch (const std::exception& error)
		{
			gBuffers.lastError = error.what();
		}
		catch (...)
		{
			gBuffers.lastError = "Unknown zSpace planarity analysis error.";
		}
		return 0;
	}

	ZSPACE_WASM_EXPORT int zspace_mesh_curvature_analysis(
		int kind,
		double threshold,
		float negativeR, float negativeG, float negativeB,
		float zeroR, float zeroG, float zeroB,
		float positiveR, float positiveG, float positiveB)
	{
		try
		{
			if (!gHasMesh) throw std::runtime_error("No active mesh for curvature analysis.");
			gBuffers.lastError.clear();
			zSpace::zFnMesh meshFn(activeMeshObject());

			zSpace::zDoubleArray values;
			if (kind == 0)
			{
				zSpace::zCurvatureArray principalCurvatures;
				zSpace::zVectorArray pVector1;
				zSpace::zVectorArray pVector2;
				meshFn.getPrincipalCurvatures(principalCurvatures, pVector1, pVector2);
				values.reserve(principalCurvatures.size());
				for (const auto& curvature : principalCurvatures)
					values.push_back(curvature.k1 * curvature.k2);
			}
			else if (kind == 1)
			{
				zSpace::zCurvatureArray principalCurvatures;
				zSpace::zVectorArray pVector1;
				zSpace::zVectorArray pVector2;
				meshFn.getPrincipalCurvatures(principalCurvatures, pVector1, pVector2);
				values.reserve(principalCurvatures.size());
				for (const auto& curvature : principalCurvatures)
					values.push_back((curvature.k1 + curvature.k2) * 0.5);
			}
			else
			{
				throw std::runtime_error("Unsupported curvature analysis type.");
			}

			if (values.empty()) throw std::runtime_error("zFnMesh returned no curvature values.");
			gBuffers.analysisCount = 0;
			gBuffers.analysisMinValue = 0.0;
			gBuffers.analysisMaxValue = 0.0;
			gBuffers.analysisMaxAbsValue = 0.0;
			double maxAbsValue = 0.0;
			double minValue = 0.0;
			double maxValue = 0.0;
			bool hasValue = false;
			for (double& value : values)
			{
				if (!std::isfinite(value)) value = 0.0;
				if (!hasValue)
				{
					minValue = value;
					maxValue = value;
					hasValue = true;
				}
				else
				{
					minValue = std::min(minValue, value);
					maxValue = std::max(maxValue, value);
				}
				if (std::abs(value) > 1.0e-12) ++gBuffers.analysisCount;
				maxAbsValue = std::max(maxAbsValue, std::abs(value));
			}
			gBuffers.analysisMinValue = minValue;
			gBuffers.analysisMaxValue = maxValue;
			gBuffers.analysisMaxAbsValue = maxAbsValue;
			const double displayThreshold = std::max(0.0, threshold);

			const zSpace::zColor negativeColor(negativeR, negativeG, negativeB, 1.0f);
			const zSpace::zColor zeroColor(zeroR, zeroG, zeroB, 1.0f);
			const zSpace::zColor positiveColor(positiveR, positiveG, positiveB, 1.0f);
			zSpace::zColorArray colors;
			colors.reserve(values.size());
			for (double value : values)
				colors.push_back(colorFromSignedValue(value, displayThreshold, maxAbsValue, negativeColor, zeroColor, positiveColor));

			meshFn.setVertexColors(colors, false);
			meshFn.computeMeshNormals();
			copyActiveMeshToRenderBuffers();
			clearPrimitiveBuffers();
			return 1;
		}
		catch (const std::exception& error)
		{
			gBuffers.lastError = error.what();
		}
		catch (...)
		{
			gBuffers.lastError = "Unknown zSpace curvature analysis error.";
		}
		return 0;
	}

	ZSPACE_WASM_EXPORT int zspace_mesh_principal_curvature_directions(
		float scale,
		float k1R, float k1G, float k1B,
		float k2R, float k2G, float k2B)
	{
		try
		{
			if (!gHasMesh) throw std::runtime_error("No active mesh for principal curvature directions.");
			gBuffers.lastError.clear();
			zSpace::zFnMesh meshFn(activeMeshObject());
			zSpace::zCurvatureArray principalCurvatures;
			zSpace::zVectorArray pVector1;
			zSpace::zVectorArray pVector2;
			zSpace::zPointArray positions;
			meshFn.getPrincipalCurvatures(principalCurvatures, pVector1, pVector2);
			meshFn.getVertexPositions(positions);
			if (positions.empty() || pVector1.empty() || pVector2.empty())
				throw std::runtime_error("zFnMesh returned no principal curvature directions.");

			copyActiveMeshToRenderBuffers();
			clearPrimitiveBuffers();
			const float vectorScale = static_cast<float>(averageMeshEdgeLength(meshFn)) * std::max(0.0f, scale);
			const zSpace::zColor k1Color(k1R, k1G, k1B, 1.0f);
			const zSpace::zColor k2Color(k2R, k2G, k2B, 1.0f);

			const std::size_t count = std::min({ positions.size(), pVector1.size(), pVector2.size() });
			gBuffers.vectorOrigins.reserve(count * 12);
			gBuffers.vectorDirections.reserve(count * 12);
			gBuffers.vectorColors.reserve(count * 12);
			gBuffers.vectorWeights.reserve(count * 4);
			for (std::size_t i = 0; i < count; ++i)
			{
				zSpace::zVector k1 = pVector1[i];
				zSpace::zVector k2 = pVector2[i];
				k1.normalize();
				k2.normalize();
				k1 *= vectorScale;
				k2 *= vectorScale;
				appendVectorPrimitive(positions[i], k1, k1Color);
				appendVectorPrimitive(positions[i], k1 * -1.0f, k1Color);
				appendVectorPrimitive(positions[i], k2, k2Color);
				appendVectorPrimitive(positions[i], k2 * -1.0f, k2Color);
			}
			return 1;
		}
		catch (const std::exception& error)
		{
			gBuffers.lastError = error.what();
		}
		catch (...)
		{
			gBuffers.lastError = "Unknown zSpace principal curvature direction error.";
		}
		return 0;
	}

	ZSPACE_WASM_EXPORT int zspace_solver_create_dynamics()
	{
		try
		{
			if (!gHasMesh) throw std::runtime_error("No active mesh for dynamic relaxation.");
			gBuffers.lastError.clear();
			gSolverInitialPositions = currentMeshPositions();
			gSolverHistory.clear();
			gSolverFrame = 0;
			rebuildSolverDynamics();
			if (!gSolverConfigUpdateBatch)
			{
				copyActiveMeshToRenderBuffers();
				copySolverPreviewToPrimitiveBuffers();
			}
			return 1;
		}
		catch (const std::exception& error)
		{
			gBuffers.lastError = error.what();
		}
		catch (...)
		{
			gBuffers.lastError = "Unknown zSpace dynamic relaxation create error.";
		}
		gSolverReady = false;
		return 0;
	}

	ZSPACE_WASM_EXPORT int zspace_solver_set_mode(int mode)
	{
		try
		{
			gBuffers.lastError.clear();
			if (mode < 0 || mode > 3) throw std::invalid_argument("Unknown mesh solver mode.");
			if (gSolverParams.mode != mode && gSolverReady && mode == 2) gMeshDynamics.prepareOrigami();
			gSolverParams.mode = mode;
			if (!gSolverConfigUpdateBatch) copySolverPreviewToPrimitiveBuffers();
			return 1;
		}
		catch (const std::exception& error)
		{
			gBuffers.lastError = error.what();
		}
		catch (...)
		{
			gBuffers.lastError = "Unknown zSpace mesh solver mode error.";
		}
		return 0;
	}

	ZSPACE_WASM_EXPORT double zspace_solver_max_area_force_residual()
	{
		return gSolverReady && gSolverParams.mode == 1 ? gSolverMaxAreaForceResidual : 0.0;
	}

	ZSPACE_WASM_EXPORT int zspace_origami_set_params(double axial, double fold, double facet, double face, double damping, double amount)
	{
		for (double value : {axial,fold,facet,face,damping}) if (!std::isfinite(value) || value < 0) return 0;
		if (!std::isfinite(amount) || std::abs(amount) > 1) return 0;
		gOrigamiSettings.axial=axial;gOrigamiSettings.fold=fold;gOrigamiSettings.facet=facet;
		gOrigamiSettings.face=face;gOrigamiSettings.damping=damping;gOrigamiSettings.foldAmount=amount;
		gSolverEquilibriumReached=false;
		return 1;
	}
	ZSPACE_WASM_EXPORT int zspace_origami_set_crease(int edge, int assignment, double degrees)
	{
		try {
			if (!gSolverReady || gSolverParams.mode != 2) throw std::runtime_error("Activate origami dynamics before assigning creases.");
			gMeshDynamics.setOrigamiCrease(edge,assignment,degrees*3.141592653589793/180.0);
			gOrigamiCreases[edge]={assignment,degrees*3.141592653589793/180.0};
			gSolverEquilibriumReached=false; return 1;
		} catch (const std::exception& error) {gBuffers.lastError=error.what();return 0;}
	}
	ZSPACE_WASM_EXPORT int zspace_origami_clear_creases()
	{
		try {
			for (const auto& entry:gOrigamiCreases) gMeshDynamics.setOrigamiCrease(entry.first,2,0);
			gOrigamiCreases.clear();gSolverEquilibriumReached=false;return 1;
		} catch (const std::exception& error) {gBuffers.lastError=error.what();return 0;}
	}
	ZSPACE_WASM_EXPORT double zspace_origami_max_strain() {return gOrigamiDiagnostics.maxStrain;}
	ZSPACE_WASM_EXPORT double zspace_origami_max_crease_error() {return gOrigamiDiagnostics.maxCreaseAngleError * 180.0 / 3.141592653589793;}
	ZSPACE_WASM_EXPORT double zspace_origami_max_panel_bend() {return gOrigamiDiagnostics.maxPanelAngleError * 180.0 / 3.141592653589793;}
	ZSPACE_WASM_EXPORT double zspace_origami_max_speed() {return gOrigamiDiagnostics.maxSpeed;}
	ZSPACE_WASM_EXPORT double zspace_origami_time_step() {return gOrigamiDiagnostics.stableTimeStep;}

	ZSPACE_WASM_EXPORT int zspace_solver_set_support_size(double size)
	{
		if (!std::isfinite(size) || size < 4.0 || size > 32.0) return 0;
		gSolverParams.supportSize = size;
		return 1;
	}

	ZSPACE_WASM_EXPORT int zspace_solver_set_surface_params(double areaStrength, double stepTolerance)
	{
		if (!std::isfinite(areaStrength) || areaStrength < 0.0 || !std::isfinite(stepTolerance) || stepTolerance <= 0.0) return 0;
		gSolverParams.areaStrength = areaStrength;
		gSolverParams.stepTolerance = stepTolerance;
		gSolverEquilibriumReached = false;
		return 1;
	}

	ZSPACE_WASM_EXPORT int zspace_planarity_constraints(const char* json)
	{
		try {
			auto config = readPlanarityConstraints(json, static_cast<int>(gMeshDynamics.dynamicPositions.size()));
			gPlanarity.groups = std::move(config.groups); gPlanarity.origins = std::move(config.origins);
			gPlanarity.normals = std::move(config.normals); gPlanarity.pairs = std::move(config.pairs);
			gPlanarity.lengths = std::move(config.lengths); gSolverEquilibriumReached = false;
			return 1;
		} catch (const std::exception& e) { gBuffers.lastError = e.what(); return 0; }
	}
	ZSPACE_WASM_EXPORT int zspace_planarity_constraints_from_buffers(
		const std::uint32_t* offsets, int groupCount, const std::uint32_t* vertices, int vertexIdCount,
		const double* origins, const double* normals, const std::uint32_t* pairs, const double* lengths, int pairCount)
	{
		try {
			gBuffers.lastError.clear();
			if (groupCount < 0 || vertexIdCount < 0 || pairCount < 0 || !offsets || offsets[0] != 0 ||
				offsets[groupCount] != static_cast<std::uint32_t>(vertexIdCount) ||
				(groupCount && (!vertices || !origins || !normals)) || (pairCount && (!pairs || !lengths)))
				throw std::invalid_argument("Invalid planarity buffers.");
			PlanaritySettings config;
			auto validId = [&](std::uint32_t id) {
				if (id >= gMeshDynamics.dynamicPositions.size()) throw std::invalid_argument("Planarity vertex ID out of range.");
				return static_cast<int>(id);
			};
			for (int i = 0; i < groupCount; ++i) {
				if (offsets[i] >= offsets[i + 1] || offsets[i + 1] > static_cast<std::uint32_t>(vertexIdCount))
					throw std::invalid_argument("Empty or invalid plane group.");
				zSpace::zIntArray ids;
				for (std::uint32_t j = offsets[i]; j < offsets[i + 1]; ++j) ids.push_back(validId(vertices[j]));
				for (int j = 0; j < 3; ++j) if (!std::isfinite(origins[3*i+j]) || !std::isfinite(normals[3*i+j]))
					throw std::invalid_argument("Invalid plane origin or normal.");
				zSpace::zVector normal(normals[3*i], normals[3*i+1], normals[3*i+2]);
				if (normal.length() < 1e-9) throw std::invalid_argument("Zero plane normal.");
				normal.normalize();
				config.groups.push_back(std::move(ids));
				config.origins.emplace_back(origins[3*i], origins[3*i+1], origins[3*i+2]);
				config.normals.push_back(normal);
			}
			for (int i = 0; i < pairCount; ++i) {
				const int a = validId(pairs[2*i]), b = validId(pairs[2*i+1]);
				if (a == b || !std::isfinite(lengths[i]) || lengths[i] < 0) throw std::invalid_argument("Invalid rigid pair.");
				config.pairs.emplace_back(a, b); config.lengths.push_back(lengths[i]);
			}
			gPlanarity.groups = std::move(config.groups); gPlanarity.origins = std::move(config.origins);
			gPlanarity.normals = std::move(config.normals); gPlanarity.pairs = std::move(config.pairs);
			gPlanarity.lengths = std::move(config.lengths); gSolverEquilibriumReached = false;
			return 1;
		} catch (const std::exception& e) { gBuffers.lastError = e.what(); return 0; }
	}

	ZSPACE_WASM_EXPORT int zspace_planarity_params(int mask, int quad, double strength, double tolerance,
		double groupStrength, double groupTolerance, double pairStrength, double pairTolerance)
	{
		for (double v : {strength,tolerance,groupStrength,groupTolerance,pairStrength,pairTolerance})
			if (!std::isfinite(v) || v < 0) return 0;
		gPlanarity.quad = quad != 0;
		for (int i=0;i<3;++i) gPlanarity.enabled[i] = (mask & (1 << i)) != 0;
		gPlanarity.strength[0]=strength; gPlanarity.tolerance[0]=tolerance;
		gPlanarity.strength[1]=groupStrength; gPlanarity.tolerance[1]=groupTolerance;
		gPlanarity.strength[2]=pairStrength; gPlanarity.tolerance[2]=pairTolerance;
		gSolverEquilibriumReached=false; return 1;
	}
	ZSPACE_WASM_EXPORT double zspace_planarity_deviation(int kind)
	{
		return kind >= 0 && kind < 3 ? gPlanarityDeviations[kind] : 0;
	}

	ZSPACE_WASM_EXPORT int zspace_solver_set_area_force_tolerance(double tolerance)
	{
		if (!std::isfinite(tolerance) || tolerance < 0.0) return 0;
		gSolverParams.areaForceTolerance = tolerance;
		gSolverEquilibriumReached = false;
		return 1;
	}

	ZSPACE_WASM_EXPORT int zspace_solver_set_params(
		double gravity,
		double directionX, double directionY, double directionZ,
		double value,
		double edgeLengthMultiplier,
		int unusedRestLengthMode,
		double massMin,
		double massMax,
		double timeStep,
		double vectorScale,
		double springStiffness,
		double residualThreshold,
		int integrationType,
		int displayForceVectors,
		int gravityEnabled,
		int vectorForceEnabled,
		int edgeForceEnabled,
		int fixXY,
		int residualForceEnabled)
	{
		try
		{
			gBuffers.lastError.clear();
			zSpace::zVector direction(directionX, directionY, directionZ);
			if (direction.length() < 1.0e-9) direction = zSpace::zVector(0, 0, -1);
			direction.normalize();

			gSolverParams.gravity = gravity;
			gSolverParams.direction = direction;
			gSolverParams.value = value;
			(void)unusedRestLengthMode;
			gSolverParams.edgeLengthMultiplier = std::max(0.0, edgeLengthMultiplier);
			gSolverParams.massMin = std::max(1.0e-6, massMin);
			gSolverParams.massMax = std::max(1.0e-6, massMax);
			gSolverParams.timeStep = std::clamp(timeStep, 0.0001, 1.0);
			gSolverParams.vectorScale = std::max(0.0, vectorScale);
			gSolverParams.springStiffness = std::max(0.0, springStiffness);
			gSolverParams.residualThreshold = std::max(0.0, residualThreshold);
			gSolverParams.integrationType = integrationType == static_cast<int>(zSpace::zRK4)
				? zSpace::zRK4
				: zSpace::zEuler;
			gSolverParams.displayForceVectors = displayForceVectors != 0;
			gSolverParams.gravityEnabled = gravityEnabled != 0;
			gSolverParams.vectorForceEnabled = vectorForceEnabled != 0;
			gSolverParams.edgeForceEnabled = edgeForceEnabled != 0;
			gSolverParams.fixXY = fixXY != 0;
			gSolverParams.residualForceEnabled = residualForceEnabled != 0;
			applySolverParticleProperties();
			if (!gSolverConfigUpdateBatch) copySolverPreviewToPrimitiveBuffers();
			return 1;
		}
		catch (const std::exception& error)
		{
			gBuffers.lastError = error.what();
		}
		catch (...)
		{
			gBuffers.lastError = "Unknown zSpace dynamic relaxation parameter error.";
		}
		return 0;
	}

	ZSPACE_WASM_EXPORT int zspace_solver_clear_supports()
	{
		try
		{
			gBuffers.lastError.clear();
			gSolverSupports.clear();
			if (gSolverSupportUpdateBatch || gSolverConfigUpdateBatch) return 1;
			refreshSolverSupportMask();
			applySolverParticleProperties();
			copySolverPreviewToPrimitiveBuffers();
			return 1;
		}
		catch (const std::exception& error)
		{
			gBuffers.lastError = error.what();
		}
		catch (...)
		{
			gBuffers.lastError = "Unknown zSpace dynamic relaxation support clear error.";
		}
		return 0;
	}

	ZSPACE_WASM_EXPORT int zspace_solver_add_support(int vertexId)
	{
		try
		{
			if (vertexId < 0) throw std::runtime_error("Support vertex id must be non-negative.");
			gBuffers.lastError.clear();
			if (std::find(gSolverSupports.begin(), gSolverSupports.end(), vertexId) == gSolverSupports.end())
				gSolverSupports.push_back(vertexId);
			if (gSolverSupportUpdateBatch || gSolverConfigUpdateBatch) return 1;
			refreshSolverSupportMask();
			applySolverParticleProperties();
			copySolverPreviewToPrimitiveBuffers();
			return 1;
		}
		catch (const std::exception& error)
		{
			gBuffers.lastError = error.what();
		}
		catch (...)
		{
			gBuffers.lastError = "Unknown zSpace dynamic relaxation support add error.";
		}
		return 0;
	}

	ZSPACE_WASM_EXPORT int zspace_solver_remove_support(int vertexId)
	{
		try
		{
			if (vertexId < 0) throw std::runtime_error("Support vertex id must be non-negative.");
			gBuffers.lastError.clear();
			removeIntValue(gSolverSupports, vertexId);
			if (gSolverSupportUpdateBatch || gSolverConfigUpdateBatch) return 1;
			refreshSolverSupportMask();
			applySolverParticleProperties();
			copySolverPreviewToPrimitiveBuffers();
			return 1;
		}
		catch (const std::exception& error)
		{
			gBuffers.lastError = error.what();
		}
		catch (...)
		{
			gBuffers.lastError = "Unknown zSpace dynamic relaxation support remove error.";
		}
		return 0;
	}

	ZSPACE_WASM_EXPORT int zspace_solver_clear_removed_edges()
	{
		try
		{
			gBuffers.lastError.clear();
			gSolverRemovedEdges.clear();
			applyRemovedSolverEdges();
			if (gSolverSupportUpdateBatch || gSolverConfigUpdateBatch) return 1;
			copySolverPreviewToPrimitiveBuffers();
			return 1;
		}
		catch (const std::exception& error)
		{
			gBuffers.lastError = error.what();
		}
		catch (...)
		{
			gBuffers.lastError = "Unknown zSpace solver removed-edge clear error.";
		}
		return 0;
	}

	ZSPACE_WASM_EXPORT int zspace_solver_remove_edge(int edgeId)
	{
		try
		{
			if (edgeId < 0) throw std::runtime_error("Removed edge id must be non-negative.");
			gBuffers.lastError.clear();
			const auto& meshData = zSpace::zMeshObjectStorage::read(activeMeshObject());
			if (edgeId >= meshData.numEdges()) throw std::runtime_error("Removed edge id is outside the active mesh.");
			if (std::find(gSolverRemovedEdges.begin(), gSolverRemovedEdges.end(), edgeId) == gSolverRemovedEdges.end())
				gSolverRemovedEdges.push_back(edgeId);
			applyRemovedSolverEdges();
			if (gSolverSupportUpdateBatch || gSolverConfigUpdateBatch) return 1;
			copySolverPreviewToPrimitiveBuffers();
			return 1;
		}
		catch (const std::exception& error)
		{
			gBuffers.lastError = error.what();
		}
		catch (...)
		{
			gBuffers.lastError = "Unknown zSpace solver edge remove error.";
		}
		return 0;
	}

	ZSPACE_WASM_EXPORT int zspace_solver_restore_edge(int edgeId)
	{
		try
		{
			if (edgeId < 0) throw std::runtime_error("Restored edge id must be non-negative.");
			gBuffers.lastError.clear();
			removeIntValue(gSolverRemovedEdges, edgeId);
			applyRemovedSolverEdges();
			if (gSolverSupportUpdateBatch || gSolverConfigUpdateBatch) return 1;
			copySolverPreviewToPrimitiveBuffers();
			return 1;
		}
		catch (const std::exception& error)
		{
			gBuffers.lastError = error.what();
		}
		catch (...)
		{
			gBuffers.lastError = "Unknown zSpace solver edge restore error.";
		}
		return 0;
	}

	ZSPACE_WASM_EXPORT int zspace_solver_remove_tension_edge(int edgeId)
	{
		return zspace_solver_remove_edge(edgeId);
	}

	ZSPACE_WASM_EXPORT int zspace_solver_restore_tension_edge(int edgeId)
	{
		return zspace_solver_restore_edge(edgeId);
	}

	ZSPACE_WASM_EXPORT int zspace_solver_begin_support_update()
	{
		gBuffers.lastError.clear();
		gSolverSupportUpdateBatch = true;
		return 1;
	}

	ZSPACE_WASM_EXPORT int zspace_solver_begin_config_update()
	{
		gBuffers.lastError.clear();
		gSolverConfigUpdateBatch = true;
		return 1;
	}

	ZSPACE_WASM_EXPORT int zspace_solver_end_support_update()
	{
		try
		{
			gSolverSupportUpdateBatch = false;
			gBuffers.lastError.clear();
			refreshSolverSupportMask();
			applySolverParticleProperties();
			if (!gSolverConfigUpdateBatch) copySolverPreviewToPrimitiveBuffers();
			return 1;
		}
		catch (const std::exception& error)
		{
			gBuffers.lastError = error.what();
		}
		catch (...)
		{
			gBuffers.lastError = "Unknown zSpace dynamic relaxation support update error.";
		}
		return 0;
	}

	ZSPACE_WASM_EXPORT int zspace_solver_end_config_update()
	{
		try
		{
			gSolverConfigUpdateBatch = false;
			gBuffers.lastError.clear();
			refreshSolverSupportMask();
			applySolverParticleProperties();
			copySolverPreviewToPrimitiveBuffers();
			return 1;
		}
		catch (const std::exception& error)
		{
			gBuffers.lastError = error.what();
		}
		catch (...)
		{
			gBuffers.lastError = "Unknown zSpace dynamic relaxation config update error.";
		}
		return 0;
	}

	ZSPACE_WASM_EXPORT int zspace_solver_support_count()
	{
		return static_cast<int>(gSolverSupports.size());
	}

	ZSPACE_WASM_EXPORT int zspace_solver_removed_edge_count()
	{
		return static_cast<int>(gSolverRemovedEdges.size());
	}

	ZSPACE_WASM_EXPORT int zspace_solver_tension_edge_count()
	{
		return zspace_solver_removed_edge_count();
	}

	ZSPACE_WASM_EXPORT int zspace_solver_is_edge_removed(int edgeId)
	{
		if (edgeId < 0) return 0;
		return std::find(gSolverRemovedEdges.begin(), gSolverRemovedEdges.end(), edgeId) != gSolverRemovedEdges.end() ? 1 : 0;
	}

	ZSPACE_WASM_EXPORT int zspace_solver_step(int steps)
	{
		try
		{
			gBuffers.lastError.clear();
			if (!gHasMesh)
			{
				gBuffers.lastError = "No active mesh for dynamic relaxation.";
				return 0;
			}
			if (!gSolverReady) rebuildSolverDynamics();
			const int stepCount = std::clamp(steps, 1, 240);
			const double uiTimeStep = std::clamp(gSolverParams.timeStep, 0.0001, 1.0);
			const int subStepCount = std::clamp(static_cast<int>(std::ceil(uiTimeStep / 0.01)), 1, 120);
			const double subTimeStep = uiTimeStep / static_cast<double>(subStepCount);
			if (gSolverEquilibriumReached)
			{
				copyActiveMeshToRenderBuffers();
				copySolverPreviewToPrimitiveBuffers(false);
				return 1;
			}

			for (int i = 0; i < stepCount; ++i)
			{
				const zSpace::zPointArray beforeStep = currentMeshPositions();
				gSolverHistory.push_back(beforeStep);
				if (gSolverHistory.size() > 240) gSolverHistory.pop_front();

				bool frameValid = true;
				for (int subStep = 0; subStep < subStepCount; ++subStep)
				{
					const double meshStepScale = std::max(gMeshDynamics.cachedMeshScale, 1.0e-3);
					const double maxVertexStep = meshStepScale * 0.5;

					zSpace::zVector gravityVector = gSolverParams.direction;
					if (gravityVector.length() <= 1.0e-9f) gravityVector = zSpace::zVector(0, 0, -1);
					gravityVector.normalize();

					if (gSolverParams.mode == 2)
					{
						gMeshDynamics.stepOrigami(gOrigamiSettings, subTimeStep, gOrigamiDiagnostics);
						continue;
					}
					if (gSolverParams.mode == 1)
					{
						gMeshDynamics.addMinimizeAreaForce(gSolverParams.areaStrength);
					}
					if (gSolverParams.mode == 3) evaluatePlanarity(true);
					else {
						if (gSolverParams.gravityEnabled && std::abs(gSolverParams.gravity) > 1.0e-12)
							gMeshDynamics.addMassScaledGravityForce(gSolverParams.gravity, gravityVector);
						if (gSolverParams.vectorForceEnabled && std::abs(gSolverParams.value) > 1.0e-12)
							gMeshDynamics.addLoadForce(gSolverParams.value, 0, gSolverParams.direction);
						if (gSolverParams.edgeForceEnabled)
							gMeshDynamics.addGuardedSpringForce(
								gSolverParams.springStiffness,
								gSolverParams.edgeLengthMultiplier,
								gSolverParams.fixXY ? zSpace::zConstraintXY : zSpace::zConstraintFree
							);
					}
					gMeshDynamics.addDragForce(gSolverParams.dragStrength, gSolverParams.drag);

					if (!gMeshDynamics.limitForces(meshStepScale, subTimeStep, 0.25) ||
						!gMeshDynamics.updateGuarded(subTimeStep, gSolverParams.integrationType, maxVertexStep, false))
					{
						frameValid = false;
						break;
					}
				}

				if (!frameValid)
				{
					if (!gSolverHistory.empty()) gSolverHistory.pop_back();
					restoreMeshPositions(beforeStep);
					rebuildSolverDynamics();
					gBuffers.lastError = "Dynamic relaxation step exceeded the stable force/displacement limit. Reduce gravity, edge strength, or time step.";
					return 0;
				}

				if (gSolverParams.mode == 2 || gSolverParams.mode == 1) {
					gMeshDynamics.lastUpdateDisplacements.resize(beforeStep.size());
					for (std::size_t j=0;j<beforeStep.size();++j) {
						zSpace::zVector delta=gMeshDynamics.dynamicPositions[j];delta-=beforeStep[j];
						gMeshDynamics.lastUpdateDisplacements[j]=delta.length();
					}
				}
				++gSolverFrame;

				updateSolverResidualDiagnostics(currentMeshPositions());
				if (gSolverEquilibriumReached) break;
			}

			if (gSolverParams.mode != 2)
			{
				zSpace::zFnMesh(activeMeshObject()).computeMeshNormals();
			}
			copyActiveMeshToRenderBuffers();
			copySolverPreviewToPrimitiveBuffers(false);
			return 1;
		}
		catch (const std::exception& error)
		{
			gBuffers.lastError = error.what();
		}
		catch (...)
		{
			gBuffers.lastError = "Unknown zSpace dynamic relaxation step error.";
		}
		return 0;
	}

	ZSPACE_WASM_EXPORT int zspace_solver_previous_frame()
	{
		try
		{
			if (!gHasMesh) throw std::runtime_error("No active mesh for dynamic relaxation.");
			gBuffers.lastError.clear();
			if (gSolverHistory.empty()) return 1;
			const zSpace::zPointArray previous = gSolverHistory.back();
			gSolverHistory.pop_back();
			if (!fastRestoreSolverPositions(previous)) restoreMeshPositions(previous);
			gSolverFrame = std::max(0, gSolverFrame - 1);
			if (!gSolverReady) rebuildSolverDynamics();
			copySolverPreviewToPrimitiveBuffers();
			return 1;
		}
		catch (const std::exception& error)
		{
			gBuffers.lastError = error.what();
		}
		catch (...)
		{
			gBuffers.lastError = "Unknown zSpace dynamic relaxation previous frame error.";
		}
		return 0;
	}

	ZSPACE_WASM_EXPORT int zspace_solver_reset()
	{
		try
		{
			if (!gHasMesh) throw std::runtime_error("No active mesh for dynamic relaxation.");
			gBuffers.lastError.clear();
			if (gSolverInitialPositions.empty()) gSolverInitialPositions = currentMeshPositions();
			if (!fastRestoreSolverPositions(gSolverInitialPositions)) restoreMeshPositions(gSolverInitialPositions);
			gSolverHistory.clear();
			gSolverFrame = 0;
			if (!gSolverReady) rebuildSolverDynamics();
			copySolverPreviewToPrimitiveBuffers();
			return 1;
		}
		catch (const std::exception& error)
		{
			gBuffers.lastError = error.what();
		}
		catch (...)
		{
			gBuffers.lastError = "Unknown zSpace dynamic relaxation reset error.";
		}
		return 0;
	}

	ZSPACE_WASM_EXPORT int zspace_solver_frame()
	{
		return gSolverFrame;
	}

	ZSPACE_WASM_EXPORT double zspace_solver_min_residual()
	{
		return gSolverMinResidual;
	}

	ZSPACE_WASM_EXPORT double zspace_solver_max_residual()
	{
		return gSolverMaxResidual;
	}

	ZSPACE_WASM_EXPORT int zspace_solver_residual_count()
	{
		return gSolverResidualCount;
	}

	ZSPACE_WASM_EXPORT int zspace_solver_equilibrium_reached()
	{
		return gSolverEquilibriumReached ? 1 : 0;
	}

	ZSPACE_WASM_EXPORT double zspace_solver_max_displacement()
	{
		double maxDisplacement = 0.0;
		for (double displacement : gMeshDynamics.lastUpdateDisplacements)
		{
			if (std::isfinite(displacement)) maxDisplacement = std::max(maxDisplacement, displacement);
		}
		return maxDisplacement;
	}

	ZSPACE_WASM_EXPORT int zspace_solver_update_preview()
	{
		try
		{
			if (!gHasMesh) throw std::runtime_error("No active mesh for dynamic relaxation preview.");
			gBuffers.lastError.clear();
			copySolverPreviewToPrimitiveBuffers();
			return 1;
		}
		catch (const std::exception& error)
		{
			gBuffers.lastError = error.what();
		}
		catch (...)
		{
			gBuffers.lastError = "Unknown zSpace dynamic relaxation preview error.";
		}
		return 0;
	}

	ZSPACE_WASM_EXPORT int zspace_mesh_write(const char* path)
	{
		try
		{
			if (!path || !path[0]) throw std::runtime_error("Missing mesh path.");
			gBuffers.lastError.clear();

			zSpace::io_detail::MeshData data;
			zSpace::zIOResult result = extractMeshData(data);
			if (!result) throw std::runtime_error(result.message());

			const std::string outputPath(path);
			const std::string extension = lowerExtension(outputPath);
			if (extension == ".obj") result = zSpace::io_detail::writeOBJ(outputPath, data);
			else if (extension == ".json") result = zSpace::io_detail::writeMeshJSON(outputPath, data);
			else if (extension == ".usd" || extension == ".usda" || extension == ".usdc" || extension == ".usdz")
#if defined(ZSPACE_IO_OPENUSD)
				result = zSpace::io_detail::writeMeshUSD(outputPath, data);
#else
				result = zSpace::zIOResult::error("USD mesh IO is not enabled in the WASM build yet.");
#endif
			else result = zSpace::zIOResult::error("Unsupported mesh extension: " + extension);

			if (!result) throw std::runtime_error(result.message());
			return 1;
		}
		catch (const std::exception& error)
		{
			gBuffers.lastError = error.what();
		}
		catch (...)
		{
			gBuffers.lastError = "Unknown zSpace mesh write error.";
		}
		return 0;
	}

	// Typed USD scene storage and render buffers are independent of active mesh/solver.
	ZSPACE_WASM_EXPORT int zspace_usd_initialize()
	{
		try {
			gBuffers.lastError.clear();
			// Host budget: main thread plus the two preallocated pthread workers.
			// Apply only after SDK/TBB static initialization has completed.
			zSpace::io_detail::setUSDConcurrencyLimit(3);
			const char* path = "/tmp/zspace_usd_warmup.usda";
			{ std::ofstream file(path); file << "#usda 1.0\ndef Mesh \"Warmup\" {\n point3f[] points = [(0,0,0),(1,0,0),(0,1,0)]\n int[] faceVertexCounts = [3]\n int[] faceVertexIndices = [0,1,2]\n}\n"; }
			zSpace::io_detail::USDSceneData scene;
			const auto result = zSpace::io_detail::readSceneUSD(path, scene);
			std::remove(path);
			if (!result) throw std::runtime_error(result.message());
			return 1;
		} catch (const std::exception& error) { gBuffers.lastError = error.what(); return 0; }
	}

	ZSPACE_WASM_EXPORT void zspace_usd_scene_clear() { gUSDScene = {}; clearMeshBuffers(gUSDMeshBuffers); }
	ZSPACE_WASM_EXPORT int zspace_usd_scene_open(const char* path)
	{
		zspace_usd_scene_clear();
		try {
			gBuffers.lastError.clear();
			if (!path || !*path) throw std::runtime_error("Missing USD scene path.");
			zSpace::io_detail::USDSceneData scene;
			const auto result = zSpace::io_detail::readSceneUSD(path, scene);
			if (!result) throw std::runtime_error(result.message());
			gUSDScene = std::move(scene);
			return 1;
		} catch (const std::exception& error) { gBuffers.lastError = error.what(); return 0; }
	}
	ZSPACE_WASM_EXPORT int zspace_usd_scene_mesh_count() { return static_cast<int>(gUSDScene.meshes.size()); }
	ZSPACE_WASM_EXPORT const char* zspace_usd_scene_name(int index) { return index >= 0 && index < static_cast<int>(gUSDScene.meshes.size()) ? gUSDScene.meshes[index].name.c_str() : ""; }
	ZSPACE_WASM_EXPORT const char* zspace_usd_scene_path(int index) { return index >= 0 && index < static_cast<int>(gUSDScene.meshes.size()) ? gUSDScene.meshes[index].path.c_str() : ""; }
	ZSPACE_WASM_EXPORT int zspace_usd_scene_visible(int index) { return index >= 0 && index < static_cast<int>(gUSDScene.meshes.size()) && gUSDScene.meshes[index].visible; }
	ZSPACE_WASM_EXPORT int zspace_usd_scene_warning_count() { return static_cast<int>(gUSDScene.warnings.size()); }
	ZSPACE_WASM_EXPORT const char* zspace_usd_scene_warning(int index) { return index >= 0 && index < static_cast<int>(gUSDScene.warnings.size()) ? gUSDScene.warnings[index].c_str() : ""; }
	ZSPACE_WASM_EXPORT int zspace_usd_scene_select(int index)
	{
		clearMeshBuffers(gUSDMeshBuffers);
		try {
			gBuffers.lastError.clear();
			if (index < 0 || index >= static_cast<int>(gUSDScene.meshes.size())) throw std::runtime_error("USD mesh index out of range.");
			zSpace::zObjectMesh object;
			auto data = gUSDScene.meshes[index].mesh;
			data.faceNormals.clear(); // Baked world geometry needs newly computed normals.
			const auto result = installMeshData(data, object);
			if (!result) throw std::runtime_error(result.message());
			zSpace::zFnMesh fn(object);
			copyMeshToRenderBuffers(fn, {}, 0, 1, gUSDMeshBuffers);
			return 1;
		} catch (const std::exception& error) { gBuffers.lastError = error.what(); return 0; }
	}
	// Slots 0..7: float positions, normals, vertex RGB, triangle RGB, face centers,
	// face normals, edge centers, edge weights. Slots 8..11: uint32 triangles,
	// edge endpoints, polygon counts, polygon connects. Counts are scalar counts.
	ZSPACE_WASM_EXPORT const void* zspace_usd_buffer_ptr(int slot)
	{
		switch (slot) {
		case 0: return gUSDMeshBuffers.positions.data(); case 1: return gUSDMeshBuffers.normals.data();
		case 2: return gUSDMeshBuffers.colors.data(); case 3: return gUSDMeshBuffers.faceColors.data();
		case 4: return gUSDMeshBuffers.faceCenters.data(); case 5: return gUSDMeshBuffers.faceNormals.data();
		case 6: return gUSDMeshBuffers.edgeCenters.data(); case 7: return gUSDMeshBuffers.edgeWeights.data();
		case 8: return gUSDMeshBuffers.indices.data(); case 9: return gUSDMeshBuffers.edgeIndices.data();
		case 10: return gUSDMeshBuffers.faceCounts.data(); case 11: return gUSDMeshBuffers.faceConnects.data();
		default: return nullptr;
		}
	}
	ZSPACE_WASM_EXPORT int zspace_usd_buffer_count(int slot)
	{
		switch (slot) {
		case 0: return gUSDMeshBuffers.positions.size(); case 1: return gUSDMeshBuffers.normals.size();
		case 2: return gUSDMeshBuffers.colors.size(); case 3: return gUSDMeshBuffers.faceColors.size();
		case 4: return gUSDMeshBuffers.faceCenters.size(); case 5: return gUSDMeshBuffers.faceNormals.size();
		case 6: return gUSDMeshBuffers.edgeCenters.size(); case 7: return gUSDMeshBuffers.edgeWeights.size();
		case 8: return gUSDMeshBuffers.indices.size(); case 9: return gUSDMeshBuffers.edgeIndices.size();
		case 10: return gUSDMeshBuffers.faceCounts.size(); case 11: return gUSDMeshBuffers.faceConnects.size();
		default: return 0;
		}
	}
	ZSPACE_WASM_EXPORT int zspace_usd_scene_add_mesh(const char* name, const char* path, int visible,
		const float* positions, int positionCount, const std::uint32_t* counts, int faceCount,
		const std::uint32_t* connects, int connectCount)
	{
		try {
			gBuffers.lastError.clear();
			if (!positions || !counts || !connects || positionCount <= 0 || positionCount % 3 || faceCount <= 0 || connectCount <= 0)
				throw std::runtime_error("Invalid USD mesh buffers.");
			zSpace::io_detail::USDMeshData object;
			object.name = name ? name : "Mesh"; object.path = path ? path : ""; object.visible = visible != 0;
			auto& data = object.mesh;
			for (int i = 0; i < positionCount; i += 3) {
				if (!std::isfinite(positions[i]) || !std::isfinite(positions[i+1]) || !std::isfinite(positions[i+2])) throw std::runtime_error("Non-finite USD mesh positions.");
				data.positions.emplace_back(positions[i], positions[i+1], positions[i+2]);
			}
			std::size_t cursor = 0;
			for (int i = 0; i < faceCount; ++i) {
				if (counts[i] < 3 || counts[i] > static_cast<std::uint32_t>(connectCount) || cursor + counts[i] > static_cast<std::size_t>(connectCount)) throw std::runtime_error("Invalid USD polygon counts.");
				data.polygonCounts.push_back(static_cast<int>(counts[i])); cursor += counts[i];
			}
			if (cursor != static_cast<std::size_t>(connectCount)) throw std::runtime_error("USD polygon connect count mismatch.");
			for (int i = 0; i < connectCount; ++i) {
				if (connects[i] >= static_cast<std::uint32_t>(positionCount / 3)) throw std::runtime_error("USD polygon vertex index out of range.");
				data.polygonConnects.push_back(static_cast<int>(connects[i]));
			}
			gUSDScene.meshes.push_back(std::move(object));
			return 1;
		} catch (const std::exception& error) { gBuffers.lastError = error.what(); return 0; }
	}
	// Upload attribute slots: 0 vertex RGBA, 1 polygon RGBA, 2 uint32 edge pairs,
	// 3 edge RGBA, 4 float edge weights. Data is copied before the call returns.
	ZSPACE_WASM_EXPORT int zspace_usd_scene_set_attribute(int index, int slot, const void* pointer, int count)
	{
		try {
			gBuffers.lastError.clear();
			if (index < 0 || index >= static_cast<int>(gUSDScene.meshes.size()) || count < 0 || (count && !pointer)) throw std::runtime_error("Invalid USD attribute buffer.");
			auto& data = gUSDScene.meshes[index].mesh;
			if (slot == 2) {
				if (count % 2) throw std::runtime_error("USD edges require endpoint pairs.");
				zSpace::zIntArray edges;
				const auto* values = static_cast<const std::uint32_t*>(pointer);
				for (int i = 0; i < count; ++i) { if (values[i] >= data.positions.size()) throw std::runtime_error("USD edge vertex out of range."); edges.push_back(static_cast<int>(values[i])); }
				data.edgeConnects = std::move(edges);
			} else if (slot == 4) {
				if (count != static_cast<int>(data.edgeConnects.size()/2)) throw std::runtime_error("USD edge weight count mismatch.");
				zSpace::zDoubleArray weights;
				const auto* values = static_cast<const float*>(pointer);
				for (int i = 0; i < count; ++i) { if (!std::isfinite(values[i])) throw std::runtime_error("Non-finite USD edge weight."); weights.push_back(values[i]); }
				data.edgeWeights = std::move(weights);
			} else if (slot == 0 || slot == 1 || slot == 3) {
				const auto expected = slot == 0 ? data.positions.size() : slot == 1 ? data.polygonCounts.size() : data.edgeConnects.size()/2;
				if (count % 4 || static_cast<std::size_t>(count/4) != expected) throw std::runtime_error("USD color count mismatch.");
				zSpace::zColorArray colors;
				const auto* values = static_cast<const float*>(pointer);
				for (int i = 0; i < count; i += 4) { for(int c=0;c<4;++c) if (!std::isfinite(values[i+c])) throw std::runtime_error("Non-finite USD color."); colors.emplace_back(values[i],values[i+1],values[i+2],values[i+3]); }
				(slot == 0 ? data.vertexColors : slot == 1 ? data.faceColors : data.edgeColors) = std::move(colors);
			} else throw std::runtime_error("Unknown USD attribute slot.");
			return 1;
		} catch (const std::exception& error) { gBuffers.lastError = error.what(); return 0; }
	}
	ZSPACE_WASM_EXPORT int zspace_usd_scene_save(const char* path)
	{
		try {
			gBuffers.lastError.clear();
			if (!path || !*path) throw std::runtime_error("Missing USD output path.");
			const auto result = zSpace::io_detail::writeSceneUSD(path, gUSDScene);
			if (!result) throw std::runtime_error(result.message());
			return 1;
		} catch (const std::exception& error) { gBuffers.lastError = error.what(); return 0; }
	}

	// Document IO uses a separate JSON/file boundary and does not mutate the
	// bridge's active mesh or solver. The viewer uses an isolated USD runtime.
	ZSPACE_WASM_EXPORT int zspace_usd_scene_read(const char* path, const char* jsonPath)
	{
		try
		{
			gBuffers.lastError.clear();
			if (!path || !jsonPath) throw std::runtime_error("Missing USD scene path.");
			std::string document;
			const auto result = zSpace::io_detail::readSceneUSD(path, document);
			if (!result) throw std::runtime_error(result.message());
			std::ofstream output(jsonPath);
			output << document;
			if (!output) throw std::runtime_error("Could not write USD scene JSON.");
			return 1;
		}
		catch (const std::exception& error) { gBuffers.lastError = error.what(); }
		catch (...) { gBuffers.lastError = "Unknown USD scene read error."; }
		return 0;
	}

	ZSPACE_WASM_EXPORT int zspace_usd_scene_write(const char* jsonPath, const char* path)
	{
		try
		{
			gBuffers.lastError.clear();
			if (!path || !jsonPath) throw std::runtime_error("Missing USD scene path.");
			std::ifstream input(jsonPath);
			if (!input) throw std::runtime_error("Could not open USD scene JSON.");
			const std::string document((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
			const auto result = zSpace::io_detail::writeSceneUSD(path, document);
			if (!result) throw std::runtime_error(result.message());
			return 1;
		}
		catch (const std::exception& error) { gBuffers.lastError = error.what(); }
		catch (...) { gBuffers.lastError = "Unknown USD scene write error."; }
		return 0;
	}

	ZSPACE_WASM_EXPORT int zspace_object_set_transform(
		float tx, float ty, float tz,
		float rx, float ry, float rz,
		float sx, float sy, float sz)
	{
		try
		{
			gBuffers.lastError.clear();
			zSpace::zFloat4 translation = { tx, ty, tz, 1.0f };
			zSpace::zFloat4 rotation = { rx, ry, rz, 0.0f };
			zSpace::zFloat4 scale = { sx, sy, sz, 1.0f };
			activeMeshObject().transformationMatrix.setTranslation(translation, false);
			activeMeshObject().transformationMatrix.setRotation(rotation, false);
			activeMeshObject().transformationMatrix.setScale(scale);
			return 1;
		}
		catch (const std::exception& error)
		{
			gBuffers.lastError = error.what();
		}
		catch (...)
		{
			gBuffers.lastError = "Unknown zSpace object transform error.";
		}
		return 0;
	}

	ZSPACE_WASM_EXPORT const float* zspace_positions_ptr() { return gBuffers.positions.data(); }
	ZSPACE_WASM_EXPORT int zspace_positions_count() { return static_cast<int>(gBuffers.positions.size()); }
	ZSPACE_WASM_EXPORT const float* zspace_normals_ptr() { return gBuffers.normals.data(); }
	ZSPACE_WASM_EXPORT int zspace_normals_count() { return static_cast<int>(gBuffers.normals.size()); }
	ZSPACE_WASM_EXPORT const float* zspace_colors_ptr() { return gBuffers.colors.data(); }
	ZSPACE_WASM_EXPORT int zspace_colors_count() { return static_cast<int>(gBuffers.colors.size()); }
	ZSPACE_WASM_EXPORT const float* zspace_face_colors_ptr() { return gBuffers.faceColors.data(); }
	ZSPACE_WASM_EXPORT int zspace_face_colors_count() { return static_cast<int>(gBuffers.faceColors.size()); }
	ZSPACE_WASM_EXPORT const float* zspace_face_centers_ptr() { return gBuffers.faceCenters.data(); }
	ZSPACE_WASM_EXPORT int zspace_face_centers_count() { return static_cast<int>(gBuffers.faceCenters.size()); }
	ZSPACE_WASM_EXPORT const float* zspace_face_normals_ptr() { return gBuffers.faceNormals.data(); }
	ZSPACE_WASM_EXPORT int zspace_face_normals_count() { return static_cast<int>(gBuffers.faceNormals.size()); }
	ZSPACE_WASM_EXPORT const float* zspace_edge_centers_ptr() { return gBuffers.edgeCenters.data(); }
	ZSPACE_WASM_EXPORT int zspace_edge_centers_count() { return static_cast<int>(gBuffers.edgeCenters.size()); }
	ZSPACE_WASM_EXPORT const float* zspace_edge_weights_ptr() { return gBuffers.edgeWeights.data(); }
	ZSPACE_WASM_EXPORT int zspace_edge_weights_count() { return static_cast<int>(gBuffers.edgeWeights.size()); }
	ZSPACE_WASM_EXPORT const float* zspace_point_positions_ptr() { return gBuffers.pointPositions.data(); }
	ZSPACE_WASM_EXPORT int zspace_point_positions_count() { return static_cast<int>(gBuffers.pointPositions.size()); }
	ZSPACE_WASM_EXPORT const float* zspace_point_colors_ptr() { return gBuffers.pointColors.data(); }
	ZSPACE_WASM_EXPORT int zspace_point_colors_count() { return static_cast<int>(gBuffers.pointColors.size()); }
	ZSPACE_WASM_EXPORT const float* zspace_point_sizes_ptr() { return gBuffers.pointSizes.data(); }
	ZSPACE_WASM_EXPORT int zspace_point_sizes_count() { return static_cast<int>(gBuffers.pointSizes.size()); }
	ZSPACE_WASM_EXPORT const float* zspace_line_positions_ptr() { return gBuffers.linePositions.data(); }
	ZSPACE_WASM_EXPORT int zspace_line_positions_count() { return static_cast<int>(gBuffers.linePositions.size()); }
	ZSPACE_WASM_EXPORT const float* zspace_line_colors_ptr() { return gBuffers.lineColors.data(); }
	ZSPACE_WASM_EXPORT int zspace_line_colors_count() { return static_cast<int>(gBuffers.lineColors.size()); }
	ZSPACE_WASM_EXPORT const float* zspace_line_weights_ptr() { return gBuffers.lineWeights.data(); }
	ZSPACE_WASM_EXPORT int zspace_line_weights_count() { return static_cast<int>(gBuffers.lineWeights.size()); }
	ZSPACE_WASM_EXPORT const float* zspace_vector_origins_ptr() { return gBuffers.vectorOrigins.data(); }
	ZSPACE_WASM_EXPORT int zspace_vector_origins_count() { return static_cast<int>(gBuffers.vectorOrigins.size()); }
	ZSPACE_WASM_EXPORT const float* zspace_vector_directions_ptr() { return gBuffers.vectorDirections.data(); }
	ZSPACE_WASM_EXPORT int zspace_vector_directions_count() { return static_cast<int>(gBuffers.vectorDirections.size()); }
	ZSPACE_WASM_EXPORT const float* zspace_vector_colors_ptr() { return gBuffers.vectorColors.data(); }
	ZSPACE_WASM_EXPORT int zspace_vector_colors_count() { return static_cast<int>(gBuffers.vectorColors.size()); }
	ZSPACE_WASM_EXPORT const float* zspace_vector_weights_ptr() { return gBuffers.vectorWeights.data(); }
	ZSPACE_WASM_EXPORT int zspace_vector_weights_count() { return static_cast<int>(gBuffers.vectorWeights.size()); }
	ZSPACE_WASM_EXPORT const float* zspace_polygon_positions_ptr() { return gBuffers.polygonPositions.data(); }
	ZSPACE_WASM_EXPORT int zspace_polygon_positions_count() { return static_cast<int>(gBuffers.polygonPositions.size()); }
	ZSPACE_WASM_EXPORT const float* zspace_polygon_colors_ptr() { return gBuffers.polygonColors.data(); }
	ZSPACE_WASM_EXPORT int zspace_polygon_colors_count() { return static_cast<int>(gBuffers.polygonColors.size()); }
	ZSPACE_WASM_EXPORT const std::uint32_t* zspace_polygon_indices_ptr() { return gBuffers.polygonIndices.data(); }
	ZSPACE_WASM_EXPORT int zspace_polygon_indices_count() { return static_cast<int>(gBuffers.polygonIndices.size()); }
	ZSPACE_WASM_EXPORT const std::uint32_t* zspace_indices_ptr() { return gBuffers.indices.data(); }
	ZSPACE_WASM_EXPORT int zspace_indices_count() { return static_cast<int>(gBuffers.indices.size()); }
	ZSPACE_WASM_EXPORT const std::uint32_t* zspace_edges_ptr() { return gBuffers.edgeIndices.data(); }
	ZSPACE_WASM_EXPORT int zspace_edges_count() { return static_cast<int>(gBuffers.edgeIndices.size()); }
	ZSPACE_WASM_EXPORT const std::uint32_t* zspace_face_counts_ptr() { return gBuffers.faceCounts.data(); }
	ZSPACE_WASM_EXPORT int zspace_face_counts_count() { return static_cast<int>(gBuffers.faceCounts.size()); }
	ZSPACE_WASM_EXPORT const std::uint32_t* zspace_face_connects_ptr() { return gBuffers.faceConnects.data(); }
	ZSPACE_WASM_EXPORT int zspace_face_connects_count() { return static_cast<int>(gBuffers.faceConnects.size()); }
	ZSPACE_WASM_EXPORT int zspace_faces_count() { return static_cast<int>(gBuffers.faceCount); }
	ZSPACE_WASM_EXPORT int zspace_analysis_count() { return static_cast<int>(gBuffers.analysisCount); }
	ZSPACE_WASM_EXPORT double zspace_analysis_min_value() { return gBuffers.analysisMinValue; }
	ZSPACE_WASM_EXPORT double zspace_analysis_max_value() { return gBuffers.analysisMaxValue; }
	ZSPACE_WASM_EXPORT double zspace_analysis_max_abs_value() { return gBuffers.analysisMaxAbsValue; }
	ZSPACE_WASM_EXPORT const char* zspace_last_error_ptr() { return gBuffers.lastError.c_str(); }
}

#include "zUSDapi.h"
