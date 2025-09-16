#include "common/PlayerRegistration.h"
#include "AlgorithmRegistrar.h"
#include "common/Player.h"

PlayerRegistration::PlayerRegistration(PlayerFactory factory)
{
    auto &registrar = AlgorithmRegistrar::getAlgorithmRegistrar();
    registrar.addPlayerFactoryToLastEntry(std::move(factory));
}
