// File: Command.cpp
#include "Command.h"
#include <algorithm>
#include <cassert>

// Deklaracje funkcji globalnych do zarządzania sceną
// W rzeczywistej aplikacji byłyby to metody odpowiedniego menedżera sceny
namespace {
    // Dodaje obiekt do sceny, zwraca przypisane ID obiektu
    int addObjectToScene(std::shared_ptr<GeometricObject> object);

    // Usuwa obiekt ze sceny po ID
    bool removeObjectFromScene(int objectId);

    // Pobiera obiekt ze sceny po ID
    std::shared_ptr<GeometricObject> getObjectFromScene(int objectId);

    // Stosuje transformację do obiektu
    bool transformObject(int objectId, const Eigen::Matrix3d& transform);

    // Pobiera aktualny styl obiektu
    ObjectStyle getObjectStyle(int objectId);

    // Ustawia styl dla obiektu
    bool setObjectStyle(int objectId, const ObjectStyle& style);
}

// Implementacja AddObjectCommand
AddObjectCommand::AddObjectCommand(std::shared_ptr<GeometricObject> object)
    : m_object(std::move(object)), m_objectId(-1) {
}

bool AddObjectCommand::execute() {
    if (!m_object) {
        return false;
    }

    m_objectId = addObjectToScene(m_object);
    return m_objectId >= 0;
}

bool AddObjectCommand::undo() {
    if (m_objectId < 0) {
        return false;
    }

    return removeObjectFromScene(m_objectId);
}

bool AddObjectCommand::redo() {
    return execute();
}

// Implementacja RemoveObjectCommand
RemoveObjectCommand::RemoveObjectCommand(int objectId)
    : m_objectId(objectId), m_removedObject(nullptr) {
}

bool RemoveObjectCommand::execute() {
    // Zapisz obiekt przed usunięciem
    m_removedObject = getObjectFromScene(m_objectId);
    if (!m_removedObject) {
        return false;
    }

    return removeObjectFromScene(m_objectId);
}

bool RemoveObjectCommand::undo() {
    if (!m_removedObject) {
        return false;
    }

    // Dodaj obiekt z powrotem do sceny
    m_objectId = addObjectToScene(m_removedObject);
    return m_objectId >= 0;
}

bool RemoveObjectCommand::redo() {
    return execute();
}

// Implementacja TransformObjectCommand
TransformObjectCommand::TransformObjectCommand(int objectId, const Eigen::Matrix3d& transformation)
    : m_objectId(objectId), m_forwardTransform(transformation) {
    // Oblicz odwrotną transformację dla operacji undo
    m_inverseTransform = transformation.inverse();
}

bool TransformObjectCommand::execute() {
    return transformObject(m_objectId, m_forwardTransform);
}

bool TransformObjectCommand::undo() {
    return transformObject(m_objectId, m_inverseTransform);
}

bool TransformObjectCommand::redo() {
    return execute();
}

// Implementacja ModifyObjectStyleCommand
ModifyObjectStyleCommand::ModifyObjectStyleCommand(int objectId, const ObjectStyle& newStyle)
    : m_objectId(objectId), m_newStyle(newStyle) {
    // Zapisz aktualny styl dla operacji undo
    m_oldStyle = getObjectStyle(m_objectId);
}

bool ModifyObjectStyleCommand::execute() {
    return setObjectStyle(m_objectId, m_newStyle);
}

bool ModifyObjectStyleCommand::undo() {
    return setObjectStyle(m_objectId, m_oldStyle);
}

bool ModifyObjectStyleCommand::redo() {
    return execute();
}

// Implementacja CompoundCommand
CompoundCommand::CompoundCommand(const std::string& name)
    : m_name(name) {
}

void CompoundCommand::addCommand(std::unique_ptr<AbstractCommand> command) {
    if (command) {
        m_commands.push_back(std::move(command));
    }
}

bool CompoundCommand::execute() {
    bool success = true;
    size_t executedCommands = 0;

    // Wykonaj wszystkie komendy po kolei
    for (size_t i = 0; i < m_commands.size(); ++i) {
        if (!m_commands[i]->execute()) {
            success = false;
            executedCommands = i;
            break;
        }
        executedCommands = i + 1;
    }

    // Jeśli którakolwiek komenda nie powiodła się, cofnij wszystkie wykonane w odwrotnej kolejności
    if (!success) {
        for (size_t i = 0; i < executedCommands; ++i) {
            size_t index = executedCommands - i - 1;
            m_commands[index]->undo();
        }
    }

    return success;
}

bool CompoundCommand::undo() {
    bool success = true;

    // Cofnij wszystkie komendy w odwrotnej kolejności
    for (auto it = m_commands.rbegin(); it != m_commands.rend(); ++it) {
        if (!(*it)->undo()) {
            success = false;
            break;
        }
    }

    return success;
}

bool CompoundCommand::redo() {
    bool success = true;

    // Ponownie wykonaj wszystkie komendy po kolei
    for (auto& command : m_commands) {
        if (!command->redo()) {
            success = false;
            break;
        }
    }

    return success;
}

// Implementacja funkcji globalnych do zarządzania sceną
namespace {
    // Te implementacje byłyby zastąpione rzeczywistym kodem zarządzania sceną
    // w prawdziwej aplikacji

    int addObjectToScene(std::shared_ptr<GeometricObject> object) {
        // Tymczasowa implementacja - dodałaby obiekt do sceny i zwróciła ID
        static int nextId = 0;
        return ++nextId;
    }

    bool removeObjectFromScene(int objectId) {
        // Tymczasowa implementacja - usunęłaby obiekt ze sceny
        return objectId > 0;
    }

    std::shared_ptr<GeometricObject> getObjectFromScene(int objectId) {
        // Tymczasowa implementacja - pobrałaby obiekt ze sceny
        // Zwraca nullptr, ponieważ nie możemy utworzyć rzeczywistej instancji
        return nullptr;
    }

    bool transformObject(int objectId, const Eigen::Matrix3d& transform) {
        // Tymczasowa implementacja - zastosowałaby transformację
        return objectId > 0;
    }

    ObjectStyle getObjectStyle(int objectId) {
        // Tymczasowa implementacja - pobrałaby styl obiektu
        return ObjectStyle{QColor(0, 0, 0), 1.0, Qt::SolidLine};
    }

    bool setObjectStyle(int objectId, const ObjectStyle& style) {
        // Tymczasowa implementacja - ustawiłaby styl obiektu
        return objectId > 0;
    }
}
