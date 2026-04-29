#include "App.h"

#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <sstream>
#include <unordered_map>
#include <vector>

#include <glaze/glaze.hpp>

#include "Log.h"

#if __has_include(<assimp/config.h>) && __has_include(<assimp/Importer.hpp>) && \
    __has_include(<assimp/postprocess.h>) && __has_include(<assimp/scene.h>)
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#define KKCRAFT_HAS_ASSIMP 1
#else
#define KKCRAFT_HAS_ASSIMP 0
#endif

namespace {
struct EntityTypeDef {
  std::string type;
  std::string model;
  std::string texture;
};

struct BlockDef {
  std::string type;
  std::array<float, 3> position{};
  float scale = 1.0f;
  float brightness = 1.0f;
  glz::json_t nbt{};
};

struct WorldDef {
  std::vector<BlockDef> blocks;
};

struct SaveDef {
  std::string texturepack;
  std::vector<EntityTypeDef> entitytypes;
  WorldDef world;
};

#if KKCRAFT_HAS_ASSIMP
void processNode(aiNode *node, const aiScene *scene, const aiMatrix4x4 &parentTransform,
                 std::vector<App::Vertex> &vertices, float &minX, float &minY, float &minZ,
                 float &maxX, float &maxY, float &maxZ) {
  aiMatrix4x4 transform = parentTransform * node->mTransformation;

  for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
    aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
    for (unsigned int f = 0; f < mesh->mNumFaces; ++f) {
      const aiFace &face = mesh->mFaces[f];
      for (unsigned int idx = 0; idx < face.mNumIndices; ++idx) {
        unsigned int vIdx = face.mIndices[idx];
        aiVector3D p = transform * mesh->mVertices[vIdx];

        minX = std::min(minX, p.x); minY = std::min(minY, p.y); minZ = std::min(minZ, p.z);
        maxX = std::max(maxX, p.x); maxY = std::max(maxY, p.y); maxZ = std::max(maxZ, p.z);

        float u = 0.0f, v = 0.0f;
        if (mesh->HasTextureCoords(0)) {
          u = mesh->mTextureCoords[0][vIdx].x;
          v = mesh->mTextureCoords[0][vIdx].y;
        }

        vertices.push_back(App::Vertex{
            {p.x, p.y, p.z},
            {1.0f, 1.0f, 1.0f},
            {u, v}
        });
      }
    }
  }

  for (unsigned int i = 0; i < node->mNumChildren; ++i) {
    processNode(node->mChildren[i], scene, transform, vertices, minX, minY, minZ, maxX, maxY, maxZ);
  }
}
#endif

} // namespace

template <>
struct glz::meta<EntityTypeDef> {
  using T = EntityTypeDef;
  static constexpr auto value = object("type", &T::type, "model", &T::model,
                                       "texture", &T::texture);
};

template <> struct glz::meta<BlockDef> {
  using T = BlockDef;
  static constexpr auto value = object("type", &T::type, "position", &T::position, "scale", &T::scale, "brightness", &T::brightness, "nbt", &T::nbt);
};

template <> struct glz::meta<WorldDef> {
  using T = WorldDef;
  static constexpr auto value = object("blocks", &T::blocks);
};

template <> struct glz::meta<SaveDef> {
  using T = SaveDef;
  static constexpr auto value = object("texturepack", &T::texturepack,
                                       "entitytypes", &T::entitytypes, "world",
                                       &T::world);
};

void App::loadWorldData() {
#if !KKCRAFT_HAS_ASSIMP
  throw std::runtime_error(
      "Assimp headers are unavailable. Cannot load model from world.json");
#else
  kk::log_info("loadWorldData: scanning assets/saves for world.json");
  std::filesystem::path worldPath;
  for (const auto &entry :
       std::filesystem::directory_iterator("assets/saves")) {
    if (!entry.is_directory()) {
      continue;
    }
    const std::filesystem::path candidate = entry.path() / "world" / "world.json";
    if (std::filesystem::exists(candidate)) {
      worldPath = candidate;
      break;
    }
  }
  if (worldPath.empty()) {
    throw std::runtime_error(
        "No world.json found under assets/saves/<save-name>/world/world.json");
  }
  kk::log_info(std::string("loadWorldData: using ") + worldPath.string());

  std::ifstream in(worldPath);
  if (!in.is_open()) {
    throw std::runtime_error("Failed to read world json: " + worldPath.string());
  }
  std::stringstream buffer;
  buffer << in.rdbuf();
  const std::string json = buffer.str();

  SaveDef save{};
  if (const auto parseResult = glz::read_json(save, json); parseResult) {
    throw std::runtime_error("Failed to parse world json");
  }
  kk::log_info(std::string("loadWorldData: texturepack=") + save.texturepack +
               " entitytypes=" + std::to_string(save.entitytypes.size()) +
               " blocks=" + std::to_string(save.world.blocks.size()));

  std::unordered_map<std::string, EntityTypeDef> entityTypes;
  for (const auto &entityType : save.entitytypes) {
    entityTypes[entityType.type] = entityType;
  }

  models.clear();
  blocks.clear();

  for (const auto &block : save.world.blocks) {
    const auto it = entityTypes.find(block.type);
    if (it == entityTypes.end()) {
      kk::log_error(std::string("Unknown block type: ") + block.type);
      continue;
    }

    if (models.find(block.type) == models.end()) {
      const std::filesystem::path modelPath =
          std::filesystem::path("assets/saves") / save.texturepack / "model" / it->second.model;
      if (!std::filesystem::exists(modelPath)) {
        throw std::runtime_error("Model file not found: " + modelPath.string());
      }

      const std::filesystem::path texturePath =
          std::filesystem::path("assets/saves") / save.texturepack / "texture" /
          save.texturepack / it->second.texture;
      if (!std::filesystem::exists(texturePath)) {
        throw std::runtime_error("Texture file not found: " + texturePath.string());
      }

      kk::log_info(std::string("loadWorldData: loading model ") +
                   modelPath.string() + " texture " + texturePath.string());
      
      ModelData modelData{};
      modelData.modelPath = modelPath.string();
      modelData.texturePath = texturePath.string();

      Assimp::Importer importer;
      const aiScene *scene = importer.ReadFile(
          modelPath.string(),
          aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_GenNormals | aiProcess_FlipUVs);
      if (!scene || !scene->HasMeshes()) {
        kk::log_error(std::string("Assimp failed: ") + importer.GetErrorString());
        throw std::runtime_error("Failed to load model: " + modelPath.string());
      }

      float minX = 1e9, minY = 1e9, minZ = 1e9;
      float maxX = -1e9, maxY = -1e9, maxZ = -1e9;

      std::vector<Vertex> rawVertices;
      aiMatrix4x4 identity;
      processNode(scene->mRootNode, scene, identity, rawVertices, minX, minY, minZ, maxX, maxY, maxZ);

      // Normalize model: center it and scale it to fit in a 1.0 unit box
      float sizeX = maxX - minX;
      float sizeY = maxY - minY;
      float sizeZ = maxZ - minZ;
      float maxSide = std::max({sizeX, sizeY, sizeZ});
      if (maxSide < 1e-6f) maxSide = 1.0f;
      float scale = 1.0f / maxSide;
      
      float centerX = (minX + maxX) * 0.5f;
      float centerY = (minY + maxY) * 0.5f;
      float centerZ = (minZ + maxZ) * 0.5f;

      for (auto& v : rawVertices) {
          v.position[0] = (v.position[0] - centerX) * scale;
          v.position[1] = (v.position[1] - centerY) * scale;
          v.position[2] = (v.position[2] - centerZ) * scale;
          modelData.vertices.push_back(v);
      }

      kk::log_info(std::string("loadWorldData: model ") + block.type + " normalized. " +
                   "Original bounds: [" + std::to_string(minX) + "," + std::to_string(minY) + "," + std::to_string(minZ) + "] to [" +
                   std::to_string(maxX) + "," + std::to_string(maxY) + "," + std::to_string(maxZ) + "], Scale applied: " + std::to_string(scale));
      models[block.type] = std::move(modelData);
    }

    kk::log_info(std::string("loadWorldData: adding instance of ") + block.type + " at [" + 
                 std::to_string(block.position[0]) + ", " + 
                 std::to_string(block.position[1]) + ", " + 
                 std::to_string(block.position[2]) + "]");

    blocks.push_back(BlockInstance{
        glm::vec3(block.position[0], block.position[1], block.position[2]), block.type, block.scale, block.brightness});
  }

  if (models.empty()) {
    throw std::runtime_error("No models loaded from world definition");
  }
  if (blocks.empty()) {
    throw std::runtime_error("No blocks found in world definition");
  }
#endif
}
