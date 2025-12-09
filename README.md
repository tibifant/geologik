# Geo-Logik

Erosion Simulation

```
// Specification for the shared file format:
// all data is written to the file raw -> this is a binary data format.

uint8_t version = 1; // for now this is 1, but may be incremented in the furure if we ever decide to change anything about the file format.
uint16_t width;
uint16_t height;

tile data[width * height];

// the data contain the following information for each 'tile':

struct tile
{
  uint16_t layers[8] { // all the heights are in decimeters (sry).
    snowHeight, 
    waterHeight, 
    grassHeight, 
    soilHeight, 
    sandHeight, 
    limeStoneHeight, 
    stoneHeight, 
    bedrockHeight // cannot be carried away by erosion 
  };
  
  // within the 'world' the terrain types can only ever be layered in the exact same order as in the array. So snow is always on top, then follows water, ..., with bedrock as the last layer.
```
