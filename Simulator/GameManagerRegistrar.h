#pragma once
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <cassert>
#include "common/AbstractGameManager.h"

class GameManagerRegistrar
{
public:
    using GameManagerFactory = std::function<std::unique_ptr<AbstractGameManager>(bool verbose)>;

private:
    struct GameManagerFactoryEntry
    {
        std::string name;
        GameManagerFactory factory;

        GameManagerFactoryEntry(std::string name) : name(std::move(name)), factory(nullptr) {}

        void setFactory(GameManagerFactory f)
        {
            assert(!factory && "Factory already set for this entry.");
            factory = std::move(f);
        }
        GameManagerFactory getFactory()
        {
            return factory;
        }

        bool hasFactory() const { return factory != nullptr; }

        std::unique_ptr<AbstractGameManager> create(bool verbose) const
        {
            assert(factory && "Factory not set.");
            return factory(verbose);
        }
    };

    std::vector<GameManagerFactoryEntry> managers;
    static GameManagerRegistrar registrar;

public:
    static GameManagerRegistrar &getGameManagerRegistrar();

    void createFactoryEntry(const std::string &name)
    {
        managers.emplace_back(name);
    }

    void addFactoryToLastEntry(GameManagerFactory f)
    {
        assert(!managers.empty() && "No entry to add factory to.");
        managers.back().setFactory(std::move(f));
    }

    struct BadRegistrationException
    {
        std::string name;
        bool hasName;
        bool hasFactory;
    };

    void validateLastRegistration()
    {
        assert(!managers.empty() && "No entries to validate.");
        const auto &last = managers.back();
        bool hasName = !last.name.empty();
        bool hasFactory = last.hasFactory();

        if (!hasName || !hasFactory)
        {
            throw BadRegistrationException{last.name, hasName, hasFactory};
        }
    }

    void removeLast() { managers.pop_back(); }

    auto begin() const { return managers.begin(); }
    auto end() const { return managers.end(); }
    std::size_t count() const { return managers.size(); }
    void clear() { managers.clear(); }
    std::vector<GameManagerFactoryEntry> &getGM() { return managers; }

    void registerGameManager(const std::string &name, GameManagerFactory f)
    {
        createFactoryEntry(name);
        addFactoryToLastEntry(std::move(f));
        validateLastRegistration();
    }
};
