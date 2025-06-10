#pragma once

#include <QJsonObject>
#include <QJsonArray>
#include <QString>

class JsonSerializable {
public:
    virtual ~JsonSerializable() = default;
    virtual QJsonObject toJson() const = 0;
    virtual bool fromJson(const QJsonObject& json) = 0;
};
