#pragma once

#include <QObject>
#include <QTimer>
#include <memory>

class DatabaseManager;
class FloatingBall;
class MemoPanel;
class MemoRepository;
class ReminderDialog;
class ReminderService;
class SettingsService;
class SingleInstanceGuard;
class QMenu;
class QSystemTrayIcon;

class MemoApplication : public QObject
{
    Q_OBJECT

public:
    explicit MemoApplication(QObject *parent = nullptr);
    ~MemoApplication() override;

    bool initialize();

private slots:
    void togglePanel();
    void showPanel();
    void hidePanel();
    void schedulePanelCollapse();
    void showContextMenu(const QPoint &globalPosition);
    void quitApplication();

private:
    void positionPanel();
    void positionReminder();
    void createTrayMenu();
    void showReminder(const QString &message);
    void applyStyleSheet();
    bool cursorWithinAssistant() const;

    std::unique_ptr<SingleInstanceGuard> m_singleInstance;
    std::unique_ptr<DatabaseManager> m_databaseManager;
    std::unique_ptr<MemoRepository> m_repository;
    std::unique_ptr<SettingsService> m_settings;
    FloatingBall *m_floatingBall = nullptr;
    MemoPanel *m_panel = nullptr;
    QSystemTrayIcon *m_trayIcon = nullptr;
    QMenu *m_trayMenu = nullptr;
    ReminderDialog *m_reminderDialog = nullptr;
    std::unique_ptr<ReminderService> m_reminderService;
    QTimer m_collapseTimer;
};
