#include "common/GameManagerRegistration.h"
#include "GameManagerRegistrar.h"
#include "common/AbstractGameManager.h"

GameManagerRegistration::GameManagerRegistration(GameManagerFactory factory)
{
    auto &regsitrar = GameManagerRegistrar::getGameManagerRegistrar();
    regsitrar.addFactoryToLastEntry(std::move(factory));
}