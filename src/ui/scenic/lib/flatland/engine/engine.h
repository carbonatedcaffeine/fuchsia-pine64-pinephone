// Copyright 2020 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SRC_UI_SCENIC_LIB_FLATLAND_ENGINE_ENGINE_H_
#define SRC_UI_SCENIC_LIB_FLATLAND_ENGINE_ENGINE_H_

#include <fuchsia/hardware/display/cpp/fidl.h>

#include "src/ui/scenic/lib/flatland/link_system.h"
#include "src/ui/scenic/lib/flatland/renderer/buffer_collection_importer.h"
#include "src/ui/scenic/lib/flatland/renderer/renderer.h"
#include "src/ui/scenic/lib/flatland/uber_struct_system.h"

namespace flatland {

class Engine final : public BufferCollectionImporter {
 public:
  Engine(std::unique_ptr<fuchsia::hardware::display::ControllerSyncPtr> display_controller,
         const std::shared_ptr<Renderer>& renderer, const std::shared_ptr<LinkSystem>& link_system,
         const std::shared_ptr<UberStructSystem>& uber_struct_system);

  // |BufferCollectionImporter|
  bool ImportBufferCollection(
      sysmem_util::GlobalBufferCollectionId collection_id,
      fuchsia::sysmem::Allocator_Sync* sysmem_allocator,
      fidl::InterfaceHandle<fuchsia::sysmem::BufferCollectionToken> token) override;

  // |BufferCollectionImporter|
  void ReleaseBufferCollection(sysmem_util::GlobalBufferCollectionId collection_id) override;

  // |BufferCollectionImporter|
  bool ImportImage(const ImageMetadata& meta_data) override;

  // |BufferCollectionImporter|
  void ReleaseImage(GlobalImageId image_id) override;

  // TODO(fxbug.dev/59646): Add in parameters for scheduling, etc. Right now we're just making sure
  // the data is processed correctly.
  void RenderFrame();

  // Register a new display to the engine. The display_id is a unique display to reference the
  // display object by, and can be retrieved by calling display_id() on a display object. The
  // TransformHandle must be the root transform of the root Flatland instance. The pixel scale is
  // the display's width/height.
  // TODO(fxbug.dev/59646): We need to figure out exactly how we want the display to anchor
  // to the Flatland hierarchy.
  void AddDisplay(uint64_t display_id, TransformHandle transform, glm::uvec2 pixel_scale);

  // Registers a sysmem buffer collection with the engine, causing it to register with both
  // the display controller and the renderer. A valid display must have already been added
  // to the Engine via |AddDisplay| before this is called with the same display_id.
  // The result is a GlobalBufferCollectionId which references the collection for both the
  // renderer and the display. If the collection failed to allocate, the id will be 0.
  sysmem_util::GlobalBufferCollectionId RegisterTargetCollection(
      fuchsia::sysmem::Allocator_Sync* sysmem_allocator, uint64_t display_id, uint32_t num_vmos);

 private:
  // The data that gets forwarded either to the display or the software renderer. The lengths
  // of |rectangles| and |images| must be the same, and each rectangle/image pair for a given
  // index represents a single renderable object.
  struct RenderData {
    std::vector<Rectangle2D> rectangles;
    std::vector<ImageMetadata> images;
    uint64_t display_id;
  };

  // Struct to represent the display's flatland info. The TransformHandle must be the root
  // transform of the root Flatland instance. The pixel scale is the display's width/height.
  // A new DisplayInfo struct is added to the display_map_ when a client calls AddDisplay().
  struct DisplayInfo {
    TransformHandle transform;
    glm::uvec2 pixel_scale;
  };

  // Gathers all of the flatland data and converts it all into a format that can be
  // directly converted into the data required by the display and the 2D renderer.
  // This is done per-display, so the result is a vector of per-display render data.
  std::vector<RenderData> ComputeRenderData();

  // Returns the image id used by the display controller.
  uint64_t InternalImageId(GlobalImageId image_id) const;

  // This mutex protects access to |display_controller_| and |image_id_map_|.
  //
  // TODO(fxbug.dev/44335): Convert this to a lock-free structure. This is a unique
  // case since we are talking to a FIDL interface (display_controller_) through a lock.
  // We either need lock-free threadsafe FIDL bindings, multiple channels to the display
  // controller, or something else.
  mutable std::mutex lock_;

  // Handle to the display controller interface.
  std::unique_ptr<fuchsia::hardware::display::ControllerSyncPtr> display_controller_;

  // Maps the flatland global image id to the image id used by the display controller.
  std::unordered_map<GlobalImageId, uint64_t> image_id_map_;

  // Software renderer used when render data cannot be directly composited to the display.
  std::shared_ptr<Renderer> renderer_;

  // The link system and uberstruct system are used to extract flatland render data.
  std::shared_ptr<LinkSystem> link_system_;
  std::shared_ptr<UberStructSystem> uber_struct_system_;

  // Maps display unique ids to the displays' flatland specific data.
  std::unordered_map<uint64_t, DisplayInfo> display_map_;
};

}  // namespace flatland

#endif  // SRC_UI_SCENIC_LIB_FLATLAND_ENGINE_ENGINE_H_
