#pragma once
#include "common/ActionRequest.h"
#include "UserCommon/Position.h"
#include "common/TankAlgorithm.h"

namespace GameManager
{

    class TankState
    {
    private:
        UserCommon::Position pos;
        std::string direction;
        int player_idx, tank_idx;
        bool is_Alive, pendingBackward;
        int ammo, backwardWait, cooldown;
        ActionRequest lastAction;
        bool actionIgnored = false, was_KilledThisRound = false;

    public:
        TankState(int player_idx, int tank_idx, int ammo, UserCommon::Position pos);
        ~TankState();

        UserCommon::Position getPosition() { return pos; };
        std::string getDirection() { return direction; };
        int getPlayerIdx() { return player_idx; };
        int getTankIdx() { return tank_idx; };
        int getCooldown() { return cooldown; };
        bool isAlive() { return is_Alive; };
        int getAmmo() { return ammo; };
        int getBackwardWait() { return backwardWait; };
        bool isPendingBackward() { return pendingBackward; };
        ActionRequest getLastAction() { return lastAction; };
        bool isActionIgnored() { return actionIgnored; };
        bool getWasKilledThisRound() { return was_KilledThisRound; };

        void setPosition(UserCommon::Position p) { pos = p; }
        void setDirection(std::string s) { direction = s; }
        void setPlayerIdx(int idx) { player_idx = idx; }
        void setTankIdx(int idx) { tank_idx = idx; }
        void setCooldown(int c) { cooldown = c; }
        void setIsAlive(bool alive) { is_Alive = alive; }
        void setAmmo(int a) { ammo = a; }
        void setBackwardWait(int b) { backwardWait = b; }
        void setPendingBackward(bool pending) { pendingBackward = pending; }
        void setLastAction(const ActionRequest &action) { lastAction = action; }
        void setActionIgnored(bool ignored) { actionIgnored = ignored; }
        void setWasKilledThisRound(bool isKilled) { was_KilledThisRound = isKilled; }
    };
}