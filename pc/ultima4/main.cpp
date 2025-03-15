// Extracts the shapes, charset graphics, intro, and endgame graphic from the PC version of Ultima 4.
// Requires the shapes.ega and charset.ega files.

// Also extracts any of the RLE intro and engame files, such as start.ega and key7.ega.

#define ALLEGRO_NO_MAGIC_MAIN
#define ALLEGRO_STATICLINK 1

#include <fstream>

#include "../../allegro/include/allegro.h"
#include "../../allegro/include/winalleg.h"

#define TILE_WIDTH    16
#define TILE_HEIGHT   16
#define NUM_TILES     256
#define TILES_PER_COL 256
#define TILES_PER_ROW 1

#define TILE_BUFFER_WIDTH  ( TILE_WIDTH * TILES_PER_ROW )
#define TILE_BUFFER_HEIGHT ( TILE_HEIGHT * TILES_PER_COL )

#define CHAR_WIDTH    8
#define CHAR_HEIGHT   8
#define NUM_CHARS     128
#define CHARS_PER_COL 128
#define CHARS_PER_ROW 1

#define CHAR_BUFFER_WIDTH  ( CHAR_WIDTH * CHARS_PER_ROW )
#define CHAR_BUFFER_HEIGHT ( CHAR_HEIGHT * CHARS_PER_COL )

#define BORDER_WIDTH  320
#define BORDER_HEIGHT 200


int32_t main()
{
  if( allegro_init() != 0 )
  {
    allegro_message("Allegro initialization failed");
    return -1;
  }

  int32_t depth{ 0 };
  if( ( depth = desktop_color_depth() ) != 0 )
  {
    depth = 16;
  }

  set_color_depth( depth );
  if( set_gfx_mode( GFX_AUTODETECT_WINDOWED, 640, 480, 0, 0 ) != 0 )
  {
    allegro_message( "Failed to set graphics mode" );
    return -1;
  }

  int32_t egaColorPalette[16] =
  {
    makecol( 0x00, 0x00, 0x00 ), // Black
    makecol( 0x00, 0x00, 0xAA ), // Blue
    makecol( 0x00, 0xAA, 0x00 ), // Green
    makecol( 0x00, 0xAA, 0xAA ), // Cyan
    makecol( 0xAA, 0x00, 0x00 ), // Red
    makecol( 0xAA, 0x00, 0xAA ), // Magenta
    makecol( 0xAA, 0x55, 0x00 ), // Brown
    makecol( 0xAA, 0xAA, 0xAA ), // Light Gray
    makecol( 0x55, 0x55, 0x55 ), // Dark Gray
    makecol( 0x55, 0x55, 0xFF ), // Bright Blue
    makecol( 0x55, 0xFF, 0x55 ), // Bright Green
    makecol( 0x55, 0xFF, 0xFF ), // Bright Cyan
    makecol( 0xFF, 0x55, 0x55 ), // Bright Red
    makecol( 0xFF, 0x55, 0xFF ), // Bright Magenta
    makecol( 0xFF, 0xFF, 0x55 ), // Bright Yellow
    makecol( 0xFF, 0xFF, 0xFF ), // White
  };

  // ---------------------
  // Process tile graphics
  // ---------------------

  BITMAP* backBuffer{ create_bitmap( TILE_BUFFER_WIDTH, TILE_BUFFER_HEIGHT ) };

  std::ifstream infile;

  infile.open( "shapes.ega", std::ios::in | std::ios::binary | std::ios::ate );

  if( !infile.is_open() )
  {
    return -1;
  }

  // Get file size (note that the first file was opened with ios::ate so the size can be fetched)
  int32_t numBytes{ static_cast<int32_t>( infile.tellg() ) };

  // Reset to the beginning
  infile.seekg( 0, std::ios::beg );

  int32_t x{ 0 };
  int32_t y{ 0 };

  int32_t currentBytes{ 0 };
  int32_t lineNum{ 0 };

  while( currentBytes < numBytes )
  {
    x = 0;

    // Read and display the next 8 bytes
    for( int32_t i = 0; i < 8; ++i )
    {
      // Display 2 pixels per byte
      char tileData{ static_cast<char>( infile.get() ) };
      putpixel( backBuffer, x++, y, egaColorPalette[ (tileData >> 4) & 0xF ] );
      putpixel( backBuffer, x++, y, egaColorPalette[ tileData & 0xF ] );
    }

    y += 1;

    ++currentBytes;
  }

  infile.close();

  save_pcx( "shapes.pcx", backBuffer, nullptr );

  destroy_bitmap( backBuffer );

  // ---------------------
  // Process text graphics
  // ---------------------

  backBuffer = create_bitmap( CHAR_BUFFER_WIDTH, CHAR_BUFFER_HEIGHT );

  infile.open( "charset.ega", std::ios::in | std::ios::binary | std::ios::ate );

  if( !infile.is_open() )
  {
    return -1;
  }

  // Get file size (note that the first file was opened with ios::ate so the size can be fetched)
  numBytes = static_cast<int32_t>( infile.tellg() );

  // Reset to the beginning
  infile.seekg( 0, std::ios::beg );

  x = 0;
  y = 0;

  currentBytes = 0;
  lineNum = 0;

  while( currentBytes < numBytes )
  {
    x = 0;

    // Read and display the next 4 bytes
    for( int32_t i = 0; i < 4; ++i )
    {
      // Display 2 pixels per byte
      char tileData{ static_cast<char>( infile.get() ) };
      putpixel( backBuffer, x++, y, egaColorPalette[( tileData >> 4 ) & 0xF] );
      putpixel( backBuffer, x++, y, egaColorPalette[tileData & 0xF] );
    }

    y += 1;

    ++currentBytes;
  }

  infile.close();

  save_pcx( "charset.pcx", backBuffer, nullptr );

  destroy_bitmap( backBuffer );

  // -----------------------
  // Border / codex graphics
  // -----------------------

  backBuffer = create_bitmap( BORDER_WIDTH, BORDER_HEIGHT );

  // Just replace this file with any of the .ega files you'd like to extract.
  // Rename the output file below as well.
  infile.open( "start.ega", std::ios::in | std::ios::binary | std::ios::ate );

  if( !infile.is_open() )
  {
    return -1;
  }

  // Get file size (note that the first file was opened with ios::ate so the size can be fetched)
  numBytes = static_cast<int32_t>( infile.tellg() );

  // Reset to the beginning
  infile.seekg( 0, std::ios::beg );

  x = 0;
  y = 0;

  currentBytes = 0;
  lineNum = 0;

  while( currentBytes < numBytes )
  {
    const char tileData{ static_cast<char>( infile.get() ) };
    ++currentBytes;

    if( tileData == 0x2 )
    {
      // Handle the run of info
      const uint8_t numPixels{ static_cast<uint8_t>( infile.get() ) };
      ++currentBytes;

      const uint8_t color{ static_cast<uint8_t>( infile.get() ) };
      ++currentBytes;

      for( uint16_t i = 0; i < numPixels; ++i )
      {
        putpixel( backBuffer, x++, y, egaColorPalette[( color >> 4 ) & 0xF] );
        putpixel( backBuffer, x++, y, egaColorPalette[color & 0xF] );

        if( x >= BORDER_WIDTH )
        {
          x = 0;
          ++y;
        }
      }
    }
    else
    {
      // Simple pixel data
      putpixel( backBuffer, x++, y, egaColorPalette[( tileData >> 4 ) & 0xF] );
      putpixel( backBuffer, x++, y, egaColorPalette[tileData & 0xF] );
    }

    if( x >= BORDER_WIDTH )
    {
      x = 0;
      ++y;
    }
  }

  infile.close();

  save_pcx( "start.pcx", backBuffer, nullptr );

  destroy_bitmap( backBuffer );

  return 0;
}
