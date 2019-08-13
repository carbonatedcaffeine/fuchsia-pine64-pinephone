// Copyright 2018 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SRC_UI_LIB_ESCHER_PAPER_PAPER_SHAPE_CACHE_H_
#define SRC_UI_LIB_ESCHER_PAPER_PAPER_SHAPE_CACHE_H_

#include <functional>
#include <vector>

#include <lib/fit/function.h>

#include "src/ui/lib/escher/forward_declarations.h"
#include "src/ui/lib/escher/geometry/types.h"
#include "src/ui/lib/escher/paper/paper_renderer_config.h"
#include "src/ui/lib/escher/util/hash_map.h"

namespace escher {

class BoundingBox;
struct RoundedRectSpec;

// Stored internally by PaperShapeCache.  Exposed externally for convenience,
// as a way to get access to both |num_indices| and |num_shadow_volume_indices|.
// This allows us to use the same mesh for two different purposes ("regular"
// geometry and extruded shadow volume geometry).
// NOTE: messy-ish but OK for now because encapsulated in |PaperRenderer|.
struct PaperShapeCacheEntry {
  uint64_t last_touched_frame = 0;
  MeshPtr mesh;
  uint32_t num_indices = 0;
  uint32_t num_shadow_volume_indices = 0;

  explicit operator bool() const { return mesh.get() != nullptr; }
};

// Generates and caches clipped triangle meshes that match the requested shape
// specification.
class PaperShapeCache {
 public:
  static constexpr size_t kNumFramesBeforeEviction = 3;

  explicit PaperShapeCache(EscherWeakPtr escher, const PaperRendererConfig& config);
  ~PaperShapeCache();

  // Return a (possibly cached) mesh that matches the shape parameters.  To
  // look up the mesh, a hash is computed from the shape parameters along with
  // the list of clip planes.  If the mesh is not found, a new mesh is generated
  // from the shape parameters, clipped by the list of planes, and
  // post-processed in whatever way is required by the current |PaperRenderer|
  // configuration (e.g. perhaps adding a vertex attribute to allow
  // shadow-volume extrusion in the vertex shader).
  const PaperShapeCacheEntry& GetRoundedRectMesh(const RoundedRectSpec& spec,
                                                 const plane3* clip_planes, size_t num_clip_planes);
  const PaperShapeCacheEntry& GetCircleMesh(float radius, const plane3* clip_planes,
                                            size_t num_clip_planes);
  const PaperShapeCacheEntry& GetRectMesh(vec2 min, vec2 max, const plane3* clip_planes,
                                          size_t num_clip_planes);
  const PaperShapeCacheEntry& GetRectMesh(float width, float height, const plane3* clip_planes,
                                          size_t num_clip_planes) {
    vec2 half_extent(0.5f * width, 0.5f * height);
    return GetRectMesh(-half_extent, half_extent, clip_planes, num_clip_planes);
  }

  // Used for wireframe debugging.
  const PaperShapeCacheEntry& GetBoxMesh(const plane3* clip_planes, size_t num_clip_planes);

  void BeginFrame(BatchGpuUploader* uploader, uint64_t frame_number);
  void EndFrame();

  uint64_t frame_number() const { return frame_number_; }

  void SetConfig(const PaperRendererConfig& config);

  size_t size() const { return cache_.size(); }

  uint64_t cache_hit_count() const { return cache_hit_count_; }
  uint64_t cache_hit_after_plane_culling_count() const {
    return cache_hit_after_plane_culling_count_;
  }
  uint64_t cache_miss_count() const { return cache_miss_count_; }

 private:
  enum class ShapeType { kRect, kRoundedRect, kCircle, kBox };

  // Args: array of planes to clip the generated mesh, and size of the array.
  using CacheMissMeshGenerator =
      fit::function<PaperShapeCacheEntry(const plane3* planes, size_t num_planes)>;

  // Computes a lookup key by starting with |shape_hash| and then hashing the
  // list of |clip_planes|.  If no mesh is found with this key, a secondary key
  // is generated similarly, this time after culling the planes against
  // |bounding_box|.  If no mesh is found with the second key, a new mesh is
  // generated by invoking |mesh_generator|; this mesh is then cached using both
  // lookup keys, based on the following rationale:
  // - the mesh is cached with the first key to maximize performance in the
  //   case that it is looked up again with the exact same set of parameters.
  // - the mesh is cached with the second key because it will be common for the
  //   culled set of planes to be the same even though the original set isn't.
  //   For example, when an object is moving freely within a large clip region,
  //   the list of unculled planes will be empty; it would be a shame to
  //   continually regenerate the mesh in such a situation.
  const PaperShapeCacheEntry& GetShapeMesh(const Hash& shape_hash, const BoundingBox& bounding_box,
                                           const plane3* clip_planes, size_t num_clip_planes,
                                           CacheMissMeshGenerator mesh_generator);

  // Populates |unculled_planes_out| with the planes that clip at least one of
  // the bounding box corners; other planes are culled, because they cannot
  // possibly intersect anything within the box.  |num_planes_inout| must
  // initially contain the number of planes in |planes|; when the function
  // returns it will contain the number of planes in |unculled_planes_out|.
  //
  // Returns true if any of the planes clips the entire bounding box, otherwise
  // return false.  If such a plane is encountered, iteration halts immediately
  // since there would be nothing left within the bounding box for subsequent
  // planes to clip (this final plane does appear in |unculled_planes_out|).
  //
  // Preserves the order of any unculled planes.
  bool CullPlanesAgainstBoundingBox(const BoundingBox& bounding_box, const plane3* planes,
                                    plane3* unculled_planes_out, size_t* num_planes_inout);

  // Called from EndFrame(); evicts all entries that have not been touched for
  // kNumFramesBeforeEviction.
  void TrimCache();

  // Return the PaperShapeCacheEntry corresponding to the hash, or nullptr if
  // none such is present in the cache.
  PaperShapeCacheEntry* FindEntry(const Hash& hash);

  // Entry must not already exist.
  void AddEntry(const Hash& hash, PaperShapeCacheEntry entry);

  const EscherWeakPtr escher_;
  HashMap<Hash, PaperShapeCacheEntry> cache_;
  BatchGpuUploader* uploader_ = nullptr;
  uint64_t frame_number_ = 0;

  // Reset every frame.
  uint64_t cache_hit_count_ = 0;
  uint64_t cache_hit_after_plane_culling_count_ = 0;
  uint64_t cache_miss_count_ = 0;

  PaperRendererShadowType shadow_type_ = PaperRendererShadowType::kNone;
};

}  // namespace escher

#endif  // SRC_UI_LIB_ESCHER_PAPER_PAPER_SHAPE_CACHE_H_
