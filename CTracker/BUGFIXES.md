# Critical Bug Fixes Applied

## Date: 2026-05-17

### 1. CRITICAL: Infinite Recursion Crash (FIXED)
**Location:** `src/core/DatabaseManager.cpp` line 122

**Problem:**
```cpp
void DatabaseManager::emitDataChanged() {
    if (m_batchUpdateMode) {
        m_pendingDataChanged = true;
    } else {
        emitDataChanged();  // ← INFINITE RECURSION!
    }
}
```

The function was calling itself recursively instead of emitting the signal, causing stack overflow and immediate crash after any add/remove/modify operation.

**Fix:**
```cpp
void DatabaseManager::emitDataChanged() {
    if (m_batchUpdateMode) {
        m_pendingDataChanged = true;
    } else {
        emit dataChanged();  // ← Correctly emit the signal
    }
}
```

**Impact:** This was causing the application to crash after every database modification (add unit, add session, delete, etc.)

---

### 2. UI Bug: Unit Expansion Not Working (FIXED)
**Location:** `src/courses/UnitExpandableWidget.cpp` line 40-50

**Problem:**
```cpp
void UnitExpandableWidget::setExpanded(bool expanded) {
    if (m_expanded == expanded) {
        m_content->setVisible(expanded);  // ← Wrong: updates UI then returns
        m_expandButton->setText(expanded ? QStringLiteral("\u25BC")
                                         : QStringLiteral("\u25B6"));
        return;  // ← Early return prevents signal emission
    }
    // ... rest of code never reached when state matches
}
```

The logic was backwards - it would update the UI when the state matched, but then return early, preventing the signal from being emitted and causing inconsistent state.

**Fix:**
```cpp
void UnitExpandableWidget::setExpanded(bool expanded) {
    if (m_expanded == expanded) {
        return;  // Already in the desired state, nothing to do
    }
    m_expanded = expanded;
    m_content->setVisible(expanded);
    m_expandButton->setText(expanded ? QStringLiteral("\u25BC")   // ▼
                                     : QStringLiteral("\u25B6")); // ▶
    emit expandStateChanged(m_expanded);
}
```

**Impact:** Units would not expand/collapse properly when clicked, sessions inside units would not display correctly.

---

### 3. Memory Safety: Signal Disconnection (IMPROVED)
**Location:** `src/courses/EntityDetailView.cpp` and `src/courses/UnitExpandableWidget.cpp`

**Problem:**
Widgets were being deleted without disconnecting their signals first, potentially causing crashes if signals were emitted during or after deletion.

**Fix in EntityDetailView::clearUnits():**
```cpp
void EntityDetailView::clearUnits() {
    // Disconnect all signals before deleting to prevent dangling connections
    for (UnitExpandableWidget* u : m_unitWidgets) {
        if (u) {
            disconnect(u, nullptr, this, nullptr);
            m_unitsLayout->removeWidget(u);
            u->deleteLater();
        }
    }
    m_unitWidgets.clear();
}
```

**Fix in UnitExpandableWidget::removeSessionTask():**
```cpp
void UnitExpandableWidget::removeSessionTask(int sessionId) {
    auto it = m_rows.find(sessionId);
    if (it == m_rows.end()) {
        return;
    }
    SessionTaskRow* row = it.value();
    if (row) {
        disconnect(row, nullptr, this, nullptr);  // Disconnect signals first
        m_contentLayout->removeWidget(row);
        row->deleteLater();
    }
    m_rows.erase(it);
    refreshOverall();
}
```

**Impact:** Prevents potential crashes from dangling signal connections when widgets are deleted.

---

## Testing Instructions

1. **Close the currently running CTracker.exe** (it's blocking the rebuild)

2. **Rebuild the application:**
   ```powershell
   cd C:\Users\01226\Desktop\MySelf\CTracker\build
   cmake --build . --config Release
   ```

3. **Test the fixes:**
   - ✅ Add a new course → should NOT crash
   - ✅ Add a unit to the course → should NOT crash
   - ✅ Add sessions to the unit → should NOT crash
   - ✅ Click to expand the unit → sessions should display properly
   - ✅ Click to collapse the unit → should hide sessions
   - ✅ Modify session progress → should NOT crash
   - ✅ Delete a session → should NOT crash
   - ✅ Delete a unit → should NOT crash
   - ✅ Delete the course → should NOT crash

4. **Expected behavior:**
   - All operations should complete without crashing
   - Units should expand/collapse smoothly
   - Sessions should be visible when unit is expanded
   - Progress updates should work correctly
   - Application should remain stable after multiple operations

---

## Next Steps

After confirming these fixes work:
1. Implement Phase 9 testing suite
2. Add regression tests for these specific bugs
3. Consider adding debug logging for signal emissions
4. Review other potential recursion or signal connection issues

