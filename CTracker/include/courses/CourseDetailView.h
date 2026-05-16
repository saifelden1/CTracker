#pragma once

#include "courses/EntityDetailView.h"

class CourseDetailView : public EntityDetailView {
    Q_OBJECT
public:
    explicit CourseDetailView(QWidget* parent = nullptr)
        : EntityDetailView(EntityCard::EntityType::Course, parent) {}

    // Convenience alias used by MainWindow wiring (mirrors design.md).
    void loadCourse(int courseId) { loadEntity(courseId); }
};
