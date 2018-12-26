/*
 Copyright (C) 2010-2017 Kristian Duske

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 TrenchBroom is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#include <gtest/gtest.h>

#include "Logger.h"
#include "Assets/Texture.h"
#include "Assets/TextureCollection.h"
#include "Assets/TextureManager.h"
#include "IO/DiskIO.h"
#include "IO/Path.h"
#include "Model/BrushContentType.h"
#include "Model/Entity.h"
#include "Model/GameConfig.h"
#include "Model/GameImpl.h"

#include <algorithm>
#include <iterator>

namespace TrenchBroom {
    namespace Model {
        TEST(GameTest, loadQuake3Shaders) {
            auto config = GameConfig(
                "test", // name
                IO::Path(), // path
                IO::Path(), // icon
                GameConfig::MapFormatConfig::List(),
                GameConfig::FileSystemConfig(
                    { IO::Path("baseq3") },
                    GameConfig::PackageFormatConfig("pk3", "zip")),
                GameConfig::TextureConfig(
                    GameConfig::TexturePackageConfig(IO::Path("textures")),
                    GameConfig::PackageFormatConfig( { "", "tga", "png", "jpg", "jpeg" }, "q3shader" ),
                    IO::Path(), // palette path
                    "__tb_textures"), // worldspawn key
                GameConfig::EntityConfig(),
                GameConfig::FaceAttribsConfig(),
                BrushContentType::List());

            const auto gamePath = IO::Disk::getCurrentWorkingDir() + IO::Path("data/Model/Game/Quake3");
            auto logger = NullLogger();
            auto game = GameImpl(config, gamePath, &logger);

            const auto textureCollections = game.findTextureCollections();
            ASSERT_EQ(1u, textureCollections.size());
            ASSERT_EQ(IO::Path("textures/test"), textureCollections.front());

            auto worldspawn = Entity();
            worldspawn.addOrUpdateAttribute("__tb_textures", textureCollections.front().asString());

            auto textureManager = Assets::TextureManager(&logger, 0, 0);
            game.loadTextureCollections(&worldspawn, IO::Path(), textureManager, &logger);

            ASSERT_EQ(1u, textureManager.collections().size());

            /*
             * The shader script contains four entries:
             * textures/test/test overrides an existing texture and points it to an editor image
             * textures/test/not_existing does not override an existing texture and points to an editor image
             * textures/test/test2 overrides an existing texture, but the editor image is missing
             * textures/test/not_existing2 does not override an existing texture, but the editor image is missing
             *
             * The file system contains three textures:
             * textures/test/test.tga is overridden by the shader script
             * textures/test/test2.tga is overridden by the shader script
             * textures/test/editor_image.jpg is not overridden by a shader
             *
             * In total, we expect the following entries:
             * test/test
             * test/not_existing
             * test/editor_image
             */

            const auto* collection = textureManager.collections().front();
            ASSERT_EQ(3u, collection->textureCount());

            const auto& textures = collection->textures();
            ASSERT_EQ(1u, std::count_if(std::begin(textures), std::end(textures), [](const auto* t) { return t->name() == "test/test"; }));
            ASSERT_EQ(1u, std::count_if(std::begin(textures), std::end(textures), [](const auto* t) { return t->name() == "test/not_existing"; }));
            ASSERT_EQ(1u, std::count_if(std::begin(textures), std::end(textures), [](const auto* t) { return t->name() == "test/editor_image"; }));
        }
    }
}
