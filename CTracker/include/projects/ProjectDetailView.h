#pragma once

#include "courses/EntityDetailView.h"

class ProjectDetailView : public EntityDetailView {
    Q_OBJECT
public:
    explicit ProjectDetailView(QWidget* parent = nullptr)
        : EntityDetailView(EntityCard::EntityType::Project, parent) {}

    void loadProject(int projectId) { loadEntity(projectId); }
};
