#pragma once

#include "core.h"

enum gpu_buffer_acces_type
{
  gbat_dynamic, // GL_DYNAMIC_STORAGE_BIT
  gbat_read_only, // GL_MAP_READ_BIT
  gbat_write_only, // GL_MAP_WRITE_BIT
  gbat_persistent_stream, // GL_MAP_PERSISTENT_BIT
  gbat_persistent_stream_immediate_update, // GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT
};

struct gpu_buffer
{
  uint32_t bufferId = 0; // this is actually a GLuint.
  
  size_t size; // in bytes
  gpu_buffer_acces_type accessType;
  bool uploaded = false;
};
