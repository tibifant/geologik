#include "objReader.h"

#include "io.h"

//////////////////////////////////////////////////////////////////////////

inline const char * FindLineEnd(const char *pos)
{
  lsAssert(pos != nullptr);

  while (*pos != '\n' && *pos != '\r' && *pos != '\0')
    pos++;

  return pos;
}

//////////////////////////////////////////////////////////////////////////

lsResult objReader_load(objInfo *pInfo, const char *contents)
{
  lsResult result = lsR_Success;

  queue<vec2f> texCoords;
  queue<vec3f> normals;

  struct vertexData
  {
    uint64_t index[3];
  };

  queue<vertexData> indices;

  LS_ERROR_IF(pInfo == nullptr || contents == nullptr, lsR_ArgumentNull);

  bool hasData = true;

  while (hasData)
  {
    switch (*contents)
    {
      case 'v':
      case 'V':
      {
        contents++;

        switch (*contents)
        {
          case ' ':
          {
            contents++;

            objInfo_vertex vertex;
            lsZeroMemory(&vertex);

            size_t index = 0;

            while (index < 3)
            {
              while (*contents == ' ')
                contents++;

              vertex.position.asArray[index] = (float_t)lsParseFloat(contents, &contents);
              index++;
            }

            LS_ERROR_CHECK(queue_pushBack(&pInfo->vertices, &vertex));
            break;
          }

          case 'n':
          {
            contents++;

            vec3f normal;

            size_t index = 0;

            while (index < 3)
            {
              while (*contents == ' ')
                contents++;

              normal.asArray[index] = (float_t)lsParseFloat(contents, &contents);
              index++;
            }

            LS_ERROR_CHECK(queue_pushBack(&normals, &normal));
            break;
          }

          case 't':
          {
            contents++;

            vec2f tex;

            size_t index = 0;

            while (index < 2)
            {
              while (*contents == ' ')
                contents++;

              tex.asArray[index] = (float_t)lsParseFloat(contents, &contents);
              index++;
            }

            LS_ERROR_CHECK(queue_pushBack(&texCoords, &tex));
            break;
          }

          default:
          {
            contents = FindLineEnd(contents);
            break;
          }
        }

        contents = FindLineEnd(contents);
        break;
      }

      case 'f':
      case 'F':
      {
        contents++;

        if (*contents == ' ')
        {
          contents++;

          vertexData data;
          queue_clear(&indices);

          size_t index = 0;

          while (index < 4)
          {
            while (*contents == ' ')
              contents++;

            if (*contents == '\n' || *contents == '\r')
              break;

            for (size_t i = 0; i < 3; i++)
            {
              if (*contents == '/')
                data.index[i] = (uint64_t)-1;
              else
                data.index[i] = lsParseInt(contents, &contents);

              if (*contents == '/')
                contents++;
            }

            index++;

            LS_ERROR_CHECK(queue_pushBack(&indices, &data));
          }

          lsAssert(index >= 3);

          while (indices.count >= 3)
          {
            triangle<objInfo_triangleVertexInfo> tri;
            lsZeroMemory(&tri);

            for (size_t i = 0; i < 3; i++)
            {
              vertexData j;
              LS_ERROR_CHECK(queue_get(&indices, i, &j));
              LS_ERROR_CHECK(queue_get(&pInfo->vertices, j.index[0] - 1, static_cast<objInfo_vertex *>(&tri.v[i])));

              if (j.index[1] != (uint64_t)-1)
              {
                LS_ERROR_CHECK(queue_get(&texCoords, j.index[1] - 1, &tri.v[i].texCoord));
                tri.v[i].hasTexCoord = true;
              }

              if (j.index[2] != (uint64_t)-1)
              {
                LS_ERROR_CHECK(queue_get(&normals, j.index[2] - 1, &tri.v[i].normal));
                tri.v[i].hasNormal = true;
              }
            }

            LS_ERROR_CHECK(queue_pushBack(&pInfo->triangles, &tri));

            vertexData _unused;
            LS_ERROR_CHECK(queue_popFront(&indices, &_unused));
          }
        }

        contents = FindLineEnd(contents);
        break;
      }

      case '\n':
      case '\r':
      {
        contents++;
        break;
      }

      case '\0':
      {
        hasData = false;
        break;
      }

      default:
      {
        contents = FindLineEnd(contents);        
        break;
      }
    }
  }

epilogue:
  queue_destroy(&texCoords);
  queue_destroy(&normals);
  queue_destroy(&indices);

  return result;
}

lsResult objReader_loadFromFile(objInfo *pInfo, const char *filename)
{
  lsResult result = lsR_Success;

  size_t chars;
  char *fileStart = nullptr;

  LS_ERROR_IF(pInfo == nullptr || filename == nullptr, lsR_ArgumentNull);
  LS_ERROR_CHECK(lsReadFile(filename, &fileStart, &chars));

  const int64_t start = lsGetCurrentTimeNs();

  LS_ERROR_CHECK(objReader_load(pInfo, fileStart));

  const int64_t end = lsGetCurrentTimeNs();

  printf("objReader has parsed '%s' (%" PRIu64 " bytes) in %f s (%f MB/s).\n", filename, chars, (end - start) * 1e-9, (float_t)chars / ((end - start) * 1e-9 * 1024 * 1024));

epilogue:
  lsFreePtr(&fileStart);
  return result;
}
