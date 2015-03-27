#ifndef TURBO_SANTA_COMMON_BACK_END_GRAPHICS_GRAPHICS_CONTROLLER_H_
#define TURBO_SANTA_COMMON_BACK_END_GRAPHICS_GRAPHICS_CONTROLLER_H_

#include "backend/config.h"

#include "backend/memory/module.h"
#include "backend/memory/primary_flags.h"
#include "backend/graphics/graphics_flags.h"
#include "backend/graphics/screen.h"
#include "backend/memory/interrupt_flag.h"
#include "backend/memory/vram_segment.h"

namespace back_end {
namespace graphics {

static const int kSmallPeriod = 456;
static const int kLargePeriod = 70224;
static const int kVBlankLowerBound = kLargePeriod - kSmallPeriod * 10; // Mode 1.
static const int kOAMLockedLowerBound = 80; // Mode 2.
static const int kVRAMOAMLockedLowerBound = 172 + kOAMLockedLowerBound; // Mode 3.
static const int kHBlankLowerBound = 204 + kVRAMOAMLockedLowerBound; // Mode 0.

static const int kScreenBufferSize = 256; // Square.

class GraphicsController : public memory::Module {
 public:
  GraphicsController(Screen* screen, memory::PrimaryFlags* primary_flags) : 
      screen_(screen), primary_flags_(primary_flags) {}

  void Init();

  void Tick(unsigned int number_of_cycles);

 private:
  // TODO(Brendan): Finish implementing interrupt_flag.
  GraphicsFlags graphics_flags_;
  memory::VRAMSegment vram_segment_;
  memory::OAMSegment oam_segment_;
  Screen* screen_;
  memory::PrimaryFlags* primary_flags_;
  memory::InterruptFlag* interrupt_flag() { return primary_flags_->interrupt_flag(); }

  void SetLCDSTATInterrupt() { interrupt_flag()->set_lcd_stat(true); }
  void SetVBlankInterrupt() { interrupt_flag()->set_v_blank(true); }
  void EnableVRAM() { vram_segment_.Enable(); }
  void EnableOAM() { oam_segment_.Enable(); }
  void DisableVRAM() { vram_segment_.Disable(); }
  void DisableOAM() { oam_segment_.Disable(); }
  unsigned long time_ = 0;
};

} // namespace graphics
} // namespace back_end

#endif // TURBO_SANTA_COMMON_BACK_END_GRAPHICS_GRAPHICS_CONTROLLER_H_
