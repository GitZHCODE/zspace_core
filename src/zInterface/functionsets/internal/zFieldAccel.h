#ifndef ZSPACE_FIELD_ACCEL_H
#define ZSPACE_FIELD_ACCEL_H

#pragma once

#include <zspace/zCore/base/zVector.h>
#include <zspace/zCore/base/zColor.h>
#include <zspace/zCore/base/zDomain.h>
#include <zspace/zCore/base/zTypeDef.h>

#include <algorithm>
#include <atomic>
#include <cmath>
#include <condition_variable>
#include <cstddef>
#include <functional>
#include <limits>
#include <mutex>
#include <thread>
#include <vector>

namespace zSpace::detail
{
#if !defined(__EMSCRIPTEN__) || defined(__EMSCRIPTEN_PTHREADS__)
	class FieldThreadPool
	{
	private:
		std::vector<std::thread> workers;
		std::mutex mutex;
		std::condition_variable cvWork;
		std::condition_variable cvDone;
		std::function<void(int, int)> currentTask;
		int totalItems = 0;
		int chunkSize = 0;
		int activeWorkers = 0;
		std::atomic<int> remainingWorkers{ 0 };
		std::uint64_t generation = 0;
		bool stop = false;

		FieldThreadPool()
		{
			const unsigned int hw = std::thread::hardware_concurrency();
			const int numWorkers = (hw > 1u) ? static_cast<int>(std::min(hw, 16u)) - 1 : 0;
			workers.reserve(static_cast<std::size_t>(numWorkers));
			for (int t = 0; t < numWorkers; ++t)
			{
				workers.emplace_back([this, t]()
				{
					std::uint64_t localGen = 0;
					while (true)
					{
						std::function<void(int, int)> task;
						int begin = 0;
						int end = 0;
						{
							std::unique_lock<std::mutex> lock(mutex);
							cvWork.wait(lock, [this, localGen]()
							{
								return stop || generation != localGen;
							});
							if (stop) return;
							localGen = generation;
							if (t >= activeWorkers) continue;
							task = currentTask;
							begin = (t + 1) * chunkSize;
							end = std::min(totalItems, begin + chunkSize);
						}
						if (begin < end && task)
						{
							task(begin, end);
						}
						if (remainingWorkers.fetch_sub(1, std::memory_order_acq_rel) == 1)
						{
							std::lock_guard<std::mutex> lock(mutex);
							cvDone.notify_one();
						}
					}
				});
			}
		}

		~FieldThreadPool()
		{
			{
				std::lock_guard<std::mutex> lock(mutex);
				stop = true;
			}
			cvWork.notify_all();
			for (auto& w : workers)
			{
				if (w.joinable()) w.join();
			}
		}

	public:
		static FieldThreadPool& instance()
		{
			static FieldThreadPool pool;
			return pool;
		}

		template<typename Func>
		void parallelFor(int count, int minParallelCount, Func&& fn)
		{
			if (count < minParallelCount || workers.empty())
			{
				fn(0, count);
				return;
			}
			const int maxThreads = static_cast<int>(workers.size()) + 1;
			const int minChunk = std::max(1, minParallelCount / 4);
			const int numThreads = std::min(maxThreads, (count + minChunk - 1) / minChunk);
			if (numThreads <= 1)
			{
				fn(0, count);
				return;
			}

			const int chunk = (count + numThreads - 1) / numThreads;
			const int neededWorkers = (count - chunk + chunk - 1) / chunk;
			if (neededWorkers <= 0)
			{
				fn(0, count);
				return;
			}

			{
				std::lock_guard<std::mutex> lock(mutex);
				currentTask = [&](int b, int e) { fn(b, e); };
				totalItems = count;
				chunkSize = chunk;
				activeWorkers = neededWorkers;
				remainingWorkers.store(neededWorkers, std::memory_order_release);
				++generation;
			}
			cvWork.notify_all();

			fn(0, std::min(count, chunk));

			std::unique_lock<std::mutex> lock(mutex);
			cvDone.wait(lock, [this]()
			{
				return remainingWorkers.load(std::memory_order_acquire) == 0;
			});
			currentTask = nullptr;
		}
	};
#endif

	template<typename Func>
	inline void zFieldParallelFor(int count, int minParallelCount, Func&& fn)
	{
		if (count <= 0) return;
#if !defined(__EMSCRIPTEN__) || defined(__EMSCRIPTEN_PTHREADS__)
		FieldThreadPool::instance().parallelFor(count, minParallelCount, std::forward<Func>(fn));
#else
		(void)minParallelCount;
		fn(0, count);
#endif
	}

	// Flat AABB-bounded Bucket K-D Tree (leaf size 16) for fast exact 2D/3D nearest-point queries.
	class FlatPointKDTree
	{
	public:
		static constexpr int kLeafSize = 16;

		struct PointEntry
		{
			float x;
			float y;
			float z;
		};

		struct Node
		{
			float minX, minY, minZ;
			float maxX, maxY, maxZ;
			int leftOrStart;  // >= 0: left child node index; if count > 0, start index in points
			int rightOrCount; // if > 0 and isLeaf: point count; if < 0: -(right child node index)
		};

	private:
		std::vector<PointEntry> points;
		std::vector<Node> nodes;

		int buildRecursive(int start, int end)
		{
			const int nodeIdx = static_cast<int>(nodes.size());
			nodes.emplace_back();

			float minX = points[start].x, maxX = points[start].x;
			float minY = points[start].y, maxY = points[start].y;
			float minZ = points[start].z, maxZ = points[start].z;
			for (int i = start + 1; i < end; ++i)
			{
				if (points[i].x < minX) minX = points[i].x;
				if (points[i].x > maxX) maxX = points[i].x;
				if (points[i].y < minY) minY = points[i].y;
				if (points[i].y > maxY) maxY = points[i].y;
				if (points[i].z < minZ) minZ = points[i].z;
				if (points[i].z > maxZ) maxZ = points[i].z;
			}

			nodes[nodeIdx].minX = minX;
			nodes[nodeIdx].minY = minY;
			nodes[nodeIdx].minZ = minZ;
			nodes[nodeIdx].maxX = maxX;
			nodes[nodeIdx].maxY = maxY;
			nodes[nodeIdx].maxZ = maxZ;

			const int count = end - start;
			if (count <= kLeafSize)
			{
				nodes[nodeIdx].leftOrStart = start;
				nodes[nodeIdx].rightOrCount = count;
				return nodeIdx;
			}

			const float dx = maxX - minX;
			const float dy = maxY - minY;
			const float dz = maxZ - minZ;
			const int axis = (dx >= dy && dx >= dz) ? 0 : ((dy >= dz) ? 1 : 2);
			const int mid = start + count / 2;

			std::nth_element(
				points.begin() + start,
				points.begin() + mid,
				points.begin() + end,
				[axis](const PointEntry& a, const PointEntry& b)
				{
					if (axis == 0) return a.x < b.x;
					if (axis == 1) return a.y < b.y;
					return a.z < b.z;
				});

			const int leftChild = buildRecursive(start, mid);
			const int rightChild = buildRecursive(mid, end);
			nodes[nodeIdx].leftOrStart = leftChild;
			nodes[nodeIdx].rightOrCount = -rightChild;
			return nodeIdx;
		}

		static inline float aabbDistSq(const Node& n, float qx, float qy, float qz) noexcept
		{
			const float dx = (qx < n.minX) ? (n.minX - qx) : ((qx > n.maxX) ? (qx - n.maxX) : 0.0f);
			const float dy = (qy < n.minY) ? (n.minY - qy) : ((qy > n.maxY) ? (qy - n.maxY) : 0.0f);
			const float dz = (qz < n.minZ) ? (n.minZ - qz) : ((qz > n.maxZ) ? (qz - n.maxZ) : 0.0f);
			return dx * dx + dy * dy + dz * dz;
		}

	public:
		void build(const zPoint* srcPts, std::size_t count)
		{
			points.resize(count);
			nodes.clear();
			nodes.reserve((count / kLeafSize + 1) * 2 + 1);
			for (std::size_t i = 0; i < count; ++i)
			{
				points[i] = { srcPts[i].x, srcPts[i].y, srcPts[i].z };
			}
			if (!points.empty())
			{
				buildRecursive(0, static_cast<int>(points.size()));
			}
		}

		bool empty() const noexcept { return points.empty(); }
		std::size_t size() const noexcept { return points.size(); }

		float nearestDistSq(float qx, float qy, float qz, int& hintIdx) const noexcept
		{
			const int n = static_cast<int>(points.size());
			if (n == 0) return 0.0f;

			float bestDistSq = std::numeric_limits<float>::max();
			if (hintIdx >= 0 && hintIdx < n)
			{
				const PointEntry& hp = points[hintIdx];
				const float dx = qx - hp.x;
				const float dy = qy - hp.y;
				const float dz = qz - hp.z;
				bestDistSq = dx * dx + dy * dy + dz * dz;
			}

			int stack[64];
			int top = 0;
			stack[top++] = 0;

			while (top > 0)
			{
				const int nodeIdx = stack[--top];
				const Node& node = nodes[nodeIdx];

				if (node.rightOrCount > 0)
				{
					const int start = node.leftOrStart;
					const int end = start + node.rightOrCount;
					for (int i = start; i < end; ++i)
					{
						const float dx = qx - points[i].x;
						const float dy = qy - points[i].y;
						const float dz = qz - points[i].z;
						const float distSq = dx * dx + dy * dy + dz * dz;
						if (distSq < bestDistSq)
						{
							bestDistSq = distSq;
							hintIdx = i;
						}
					}
					continue;
				}

				const int leftChild = node.leftOrStart;
				const int rightChild = -node.rightOrCount;
				const float dLeft = aabbDistSq(nodes[leftChild], qx, qy, qz);
				const float dRight = aabbDistSq(nodes[rightChild], qx, qy, qz);

				if (dLeft < dRight)
				{
					if (dRight < bestDistSq) stack[top++] = rightChild;
					if (dLeft < bestDistSq) stack[top++] = leftChild;
				}
				else
				{
					if (dLeft < bestDistSq) stack[top++] = leftChild;
					if (dRight < bestDistSq) stack[top++] = rightChild;
				}
			}

			return bestDistSq;
		}
	};

	// Precomputed HSV endpoint interpolator that avoids calling colMin.toHSV() and colMax.toHSV()
	// on every grid cell and converts HSV -> RGB inline.
	inline zColor blendHSVPrecomputed(
		float val,
		float domMin,
		float invDomSpan,
		const zColor& hsvMin,
		const zColor& hsvMax) noexcept
	{
		const float factor = (val - domMin) * invDomSpan;
		float h = (1.0f - factor) * hsvMin.h + factor * hsvMax.h;
		const float s = (1.0f - factor) * hsvMin.s + factor * hsvMax.s;
		const float v = (1.0f - factor) * hsvMin.v + factor * hsvMax.v;

		float r = 0.0f, g = 0.0f, b = 0.0f;
		if (s == 0.0f)
		{
			r = v; g = v; b = v;
		}
		else
		{
			if (h >= 360.0f) h = 0.0f;
			else h /= 60.0f;

			const int i = static_cast<int>(std::trunc(h));
			const float f = h - static_cast<float>(i);
			const float p = v * (1.0f - s);
			const float q = v * (1.0f - (s * f));
			const float t = v * (1.0f - (s * (1.0f - f)));

			switch (i)
			{
			case 0: r = v; g = t; b = p; break;
			case 1: r = q; g = v; b = p; break;
			case 2: r = p; g = v; b = t; break;
			case 3: r = p; g = q; b = v; break;
			case 4: r = t; g = p; b = v; break;
			default: r = v; g = p; b = q; break;
			}
		}

		zColor out;
		out.r = r; out.g = g; out.b = b; out.a = 1.0f;
		out.h = h * 60.0f; out.s = s; out.v = v;
		return out;
	}
}

#endif
