#include "back_end/graphics/graphics_controller.h"

namespace back_end {
namespace graphics {

using std::vector;
using memory::OAMSegment;
using memory::SpriteAttribute;
using memory::Tile;
using memory::TileData;
using memory::BackgroundMap;

class TileReflectedX : public Tile {
 public:
  TileReflectedX() : tile_(nullptr) {}

  TileReflectedX(Tile* tile) : tile_(tile) {}

  virtual unsigned char Get(unsigned int y, unsigned int x) {
    return tile_->Get(y, Tile::kTileSize - x);
  }

  virtual void Set(unsigned int y, unsigned int x, unsigned char value) {
    tile_->Set(y, Tile::kTileSize - x, value);
  }

  void set_tile(Tile* tile) { tile_ = tile; }

 private:
  Tile* tile_;
};

class TileReflectedY : public Tile {
 public:
  TileReflectedY() : tile_(nullptr) {}

  TileReflectedY(Tile* tile) : tile_(tile) {}

  virtual unsigned char Get(unsigned int y, unsigned int x) {
    return tile_->Get(Tile::kTileSize - y, x);
  }

  virtual void Set(unsigned int y, unsigned int x, unsigned char value) {
    tile_->Set(Tile::kTileSize - y, x, value);
  }

  void set_tile(Tile* tile) { tile_ = tile; }

 private:
  Tile* tile_;
};

void GraphicsController::Tick(unsigned int number_of_cycles) {
  LCDStatus* lcd_status = graphics_flags()->lcd_status();
  time_ += number_of_cycles;
  time_ %= kLargePeriod;
  // Mode 1.
  if (time_ > kVBlankLowerBound) {
    lcd_status->set_mode(LCDStatus::V_BLANK);
    EnableOAM();
    EnableVRAM();
    SetVBlankInterrupt();
    if (lcd_status->v_blank_interrupt()) {
      SetLCDSTATInterrupt();
    }
  // Mode 0.
  } else if (time_ % kSmallPeriod > kHBlankLowerBound) {
    lcd_status->set_mode(LCDStatus::H_BLANK);
    EnableOAM();
    EnableVRAM();
    if (lcd_status->h_blank_interrupt()) {
      SetLCDSTATInterrupt();
    }
  // Mode 3.
  } else if (time_ % kSmallPeriod > kVRAMOAMLockedLowerBound) {
    lcd_status->set_mode(LCDStatus::VRAM_OAM_LOCKED);
    DisableOAM();
    DisableVRAM();
    Draw();
  // Mode 2.
  } else if (time_ % kSmallPeriod > kOAMLockedLowerBound) {
    lcd_status->set_mode(LCDStatus::OAM_LOCKED);
    DisableOAM();
    EnableVRAM();
    if (lcd_status->oam_interrupt()) {
      SetLCDSTATInterrupt();
    }
  }
}

void GraphicsController::Draw() {
  RenderLowPrioritySprites();
  RenderBackground();
  RenderWindow();
  RenderHighPrioritySprites();
  WriteToScreen();
}

void GraphicsController::WriteToScreen() {
  ScreenRaster raster;
  const int y_offset = graphics_flags()->scroll_y()->flag();
  const int x_offset = graphics_flags()->scroll_x()->flag();
  for (int y = 0; y < ScreenRaster::kScreenHeight; y++) {
    for (int x = 0; x < ScreenRaster::kScreenWidth; x++) {
      raster.Set(y, x, screen_buffer_[(x + x_offset) + (y + y_offset) * kScreenBufferSize]);
    }
  }

  screen()->Draw(raster);
}

void GraphicsController::RenderLowPrioritySprites() {
  for (int i = 0; i < OAMSegment::kAttributeNumber; i++) {
    SpriteAttribute* sprite_attribute = oam_segment()->sprite_attribute(i);
    if (!sprite_attribute->over_background()) {
      RenderSprite(sprite_attribute);
    }
  }
}

void GraphicsController::RenderHighPrioritySprites() {
  for (int i = 0; i < OAMSegment::kAttributeNumber; i++) {
    SpriteAttribute* sprite_attribute = oam_segment()->sprite_attribute(i);
    if (sprite_attribute->over_background()) {
      RenderSprite(sprite_attribute);
    }
  }
}

void GraphicsController::RenderBackground() {
  LCDControl* lcd_control = graphics_flags()->lcd_control();
  if (!lcd_control->bg_display()) {
    // Nothing to do if BG display is unset.
    return;
  }

  BackgroundMap* background;
  if (lcd_control->bg_tile_map_display_select()) {
    background = vram_segment()->upper_background_map();
  } else {
    background = vram_segment()->lower_background_map();
  }

  TileData* tile_data;
  if (lcd_control->bg_window_tile_data_select()) {
    tile_data = vram_segment()->upper_tile_data();
  } else {
    tile_data = vram_segment()->lower_tile_data();
  }

  for (int y = 0; y < kScreenBufferSize / Tile::kTileSize; y++) {
    for (int x = 0; x < kScreenBufferSize / Tile::kTileSize; x++) {
      Tile* tile = tile_data->tile(background->Get(y, x));
      RenderTile(tile, y * Tile::kTileSize, x * Tile::kTileSize, graphics_flags()->background_palette());
    }
  }
}

void GraphicsController::RenderWindow() {
  LCDControl* lcd_control = graphics_flags()->lcd_control();
  int y_offset = graphics_flags()->window_y_position()->flag();
  y_offset += graphics_flags()->scroll_y()->flag();
  int x_offset = graphics_flags()->window_x_position()->flag() + 7;
  x_offset += graphics_flags()->scroll_x()->flag();

  if (lcd_control->window_display_enable()) {
    return; // Nothing to do if disabled.
  }

  BackgroundMap* background;
  if (lcd_control->window_tile_map_display_select()) {
    background = vram_segment()->upper_background_map();
  } else {
    background = vram_segment()->lower_background_map();
  }

  TileData* tile_data;
  if (lcd_control->bg_window_tile_data_select()) {
    tile_data = vram_segment()->upper_tile_data();
  } else {
    tile_data = vram_segment()->lower_tile_data();
  }

  for (int y = 0; y < (kScreenBufferSize - y_offset) / Tile::kTileSize; y++) {
    for (int x = 0; x < (kScreenBufferSize - x_offset) / Tile::kTileSize; x++) {
      Tile* tile = tile_data->tile(background->Get(y, x));
      RenderTile(tile,
                 y * Tile::kTileSize + y_offset,
                 x * Tile::kTileSize + x_offset,
                 graphics_flags()->background_palette());
    }
  }
}

void GraphicsController::RenderSprite(SpriteAttribute* sprite_attribute) {
  TileReflectedX tile_reflected_x;
  TileReflectedY tile_reflected_y;
  Tile* tile = vram_segment()->lower_tile_data()->tile(sprite_attribute->tile_index());

  // Reflect the tile as appropriate.
  if (sprite_attribute->x_flip()) {
    tile_reflected_x.set_tile(tile);
    tile = &tile_reflected_x;
  }
  if (sprite_attribute->y_flip()) {
    tile_reflected_y.set_tile(tile);
    tile = &tile_reflected_y;
  }

  // Select color palette.
  MonochromePalette* palette;
  if (sprite_attribute->palette()) {
    palette = graphics_flags()->object_palette_1();
  } else {
    palette = graphics_flags()->object_palette_0();
  }

  RenderTile(tile, sprite_attribute->y(), sprite_attribute->x(), palette);
}

void GraphicsController::RenderTile(Tile* tile, unsigned char y_offset, unsigned char x_offset, MonochromePalette* palette) {
  for (int y = 0; y < Tile::kTileSize; y++) {
    for (int x = 0; x < Tile::kTileSize; x++) {
      MonochromePalette::Color color = palette->lookup(tile->Get(y, x));
      if (color != MonochromePalette::NONE) {
        unsigned char realized_color = static_cast<unsigned char>(color) * (256 / 4);
        screen_buffer_[(x + x_offset) + (y + y_offset) * kScreenBufferSize] = realized_color;
      }
    }
  }
}

} // namespace graphics
} // namespace back_end