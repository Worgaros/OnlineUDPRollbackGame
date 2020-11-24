/*
 MIT License

 Copyright (c) 2020 SAE Institute Switzerland AG

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
 */

#include "asteroid/ball_manager.h"
#include "asteroid/game.h"
#include "asteroid/game_manager.h"

namespace neko::asteroid
{
BallManager::BallManager(EntityManager& entityManager, GameManager& gameManager, PhysicsManager& physicsManager, PlayerCharacterManager& playerCharacterManager) :
    ComponentManager(entityManager), gameManager_(gameManager), physicsManager_(physicsManager), playerCharacterManager_(playerCharacterManager)
{
}

BallManager& BallManager::operator=(const BallManager& ballManager)
{
    components_ = ballManager.components_;
    return *this;
}

void BallManager::RespawnPlayers()
{
	for (Entity playerEntity = 0; playerEntity < entityManager_.get().GetEntitiesSize(); playerEntity++)
	{
        if (!entityManager_.get().HasComponent(playerEntity, EntityMask(ComponentType::PLAYER_CHARACTER)))
            continue;
        auto playerCharacter = playerCharacterManager_.get().GetComponent(playerEntity);
		if (playerCharacter.playerNumber == 0)
		{
            auto player1Body = physicsManager_.get().GetBody(playerEntity);
            player1Body.position = spawnPositions[0];
            player1Body.velocity = Vec2f::zero;
            physicsManager_.get().SetBody(playerEntity, player1Body);
		}
        else
        {
            auto player2Body = physicsManager_.get().GetBody(playerEntity);
            player2Body.position = spawnPositions[1];
            player2Body.velocity = Vec2f::zero;
            physicsManager_.get().SetBody(playerEntity, player2Body);
        }
	}
}

void BallManager::FixedUpdate(seconds dt)
{
    for (Entity ballEntity = 0; ballEntity < entityManager_.get().GetEntitiesSize(); ballEntity++)
    {
        if (entityManager_.get().HasComponent(ballEntity, EntityMask(ComponentType::BALL)))
        {
            auto& ball = components_[ballEntity];
            auto ballBody = physicsManager_.get().GetBody(ballEntity);
            // Left and right ball max pos
            if ((ballBody.position.x > ball.ballMaxHeight && ballBody.velocity.x > 0) || (ballBody.position.x < ball.ballMinHeight && ballBody.velocity.x < 0))
            {
                ballBody.velocity = Vec2f(-ballBody.velocity.x, ballBody.velocity.y);
                physicsManager_.get().SetBody(ballEntity, ballBody);
            }
        	// goals
            if (ballBody.position.y < -7)
            {
                auto playerCharacter = playerCharacterManager_.get().GetComponent(gameManager_.get().GetEntityFromPlayerNumber(0));
                playerCharacter.health--;
                playerCharacterManager_.get().SetComponent(gameManager_.get().GetEntityFromPlayerNumber(0), playerCharacter);
                ballBody.position = Vec2f::zero;
                ballBody.velocity = Vec2f::zero;
                RespawnPlayers();
                physicsManager_.get().SetBody(ballEntity, ballBody);
            }
            if (ballBody.position.y > 7)
            {
                auto playerCharacter = playerCharacterManager_.get().GetComponent(gameManager_.get().GetEntityFromPlayerNumber(1));
                playerCharacter.health--;
                playerCharacterManager_.get().SetComponent(gameManager_.get().GetEntityFromPlayerNumber(1), playerCharacter);
                ballBody.position = Vec2f::zero;
                ballBody.velocity = Vec2f::zero;
            	RespawnPlayers();
                physicsManager_.get().SetBody(ballEntity, ballBody);
            }
        }
    }
}
}