#include "MemoApplication.h"

#include "data/DatabaseManager.h"
#include "data/MemoRepository.h"
#include "services/ReminderService.h"
#include "services/SettingsService.h"
#include "services/SingleInstanceGuard.h"
#include "ui/FloatingBall.h"
#include "ui/IconFactory.h"
#include "ui/MemoPanel.h"

#include <QAction>
#include <QApplication>
#include <QCursor>
#include <QGuiApplication>
#include <QMenu>
#include <QMessageBox>
#include <QPalette>
#include <QScreen>
#include <QSignalBlocker>
#include <QSystemTrayIcon>

MemoApplication::MemoApplication(QObject *parent)
    : QObject(parent)
{
    m_collapseTimer.setSingleShot(true);
    connect(&m_collapseTimer, &QTimer::timeout, this, [this] {
        if (!m_panel || !m_panel->isVisible()) {
            return;
        }
        if (cursorWithinAssistant() || QApplication::activeModalWidget() != nullptr) {
            m_collapseTimer.start(m_settings ? m_settings->collapseDelayMs() : 700);
            return;
        }
        hidePanel();
    });
}

MemoApplication::~MemoApplication()
{
    if (m_trayIcon) {
        m_trayIcon->hide();
        m_trayIcon->setContextMenu(nullptr);
    }
    delete m_panel;
    delete m_floatingBall;
    delete m_trayMenu;
}

bool MemoApplication::initialize()
{
    m_singleInstance = std::make_unique<SingleInstanceGuard>(QStringLiteral("local.memoassistant.MemoAssistant.v1"));
    if (!m_singleInstance->acquire()) {
        return false;
    }

    m_databaseManager = std::make_unique<DatabaseManager>();
    if (!m_databaseManager->initialize()) {
        QMessageBox::critical(nullptr,
                              tr("备忘录助手无法启动"),
                              tr("无法初始化本地数据库：\n%1").arg(m_databaseManager->lastError()));
        return false;
    }

    m_repository = std::make_unique<MemoRepository>(m_databaseManager->database());
    m_settings = std::make_unique<SettingsService>();

    m_floatingBall = new FloatingBall;
    m_panel = new MemoPanel(m_repository.get());
    m_trayIcon = new QSystemTrayIcon(IconFactory::applicationIcon(), this);
    m_trayIcon->setToolTip(tr("备忘录助手"));

    applyStyleSheet();
    createTrayMenu();

    connect(m_floatingBall, &FloatingBall::clicked, this, &MemoApplication::togglePanel);
    connect(m_floatingBall, &FloatingBall::positionChanged, this, [this](const QPoint &position) {
        m_settings->setFloatingBallPosition(position);
        if (m_panel->isVisible()) {
            positionPanel();
        }
    });
    connect(m_floatingBall, &FloatingBall::contextMenuRequested,
            this, &MemoApplication::showContextMenu);
    connect(m_panel, &MemoPanel::closeRequested, this, &MemoApplication::hidePanel);
    connect(m_panel, &MemoPanel::pointerLeft, this, &MemoApplication::schedulePanelCollapse);
    connect(m_singleInstance.get(), &SingleInstanceGuard::activationRequested,
            this, &MemoApplication::showPanel);
    connect(m_trayIcon, &QSystemTrayIcon::activated, this, [this](QSystemTrayIcon::ActivationReason reason) {
        if (reason == QSystemTrayIcon::Trigger || reason == QSystemTrayIcon::DoubleClick) {
            togglePanel();
        }
    });
    connect(m_trayIcon, &QSystemTrayIcon::messageClicked, this, &MemoApplication::showPanel);

    m_reminderService = std::make_unique<ReminderService>(m_repository.get(), m_trayIcon, this);
    connect(m_reminderService.get(), &ReminderService::remindersTriggered,
            m_panel, &MemoPanel::refresh);
    connect(m_reminderService.get(), &ReminderService::reminderDue,
            this, [this](const QString &message) {
                m_panel->refresh();
                if (!QSystemTrayIcon::isSystemTrayAvailable() || !m_trayIcon->isVisible()) {
                    m_collapseTimer.stop();
                    positionPanel();
                    m_panel->show();
                    m_panel->raise();
                    m_floatingBall->raise();
                    QMessageBox::information(m_panel, tr("备忘录提醒"), message);
                }
            }, Qt::QueuedConnection);

    m_floatingBall->moveToVisiblePosition(m_settings->floatingBallPosition());
    m_floatingBall->show();
    m_trayIcon->show();
    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        QMessageBox::warning(nullptr,
                             tr("提醒功能不可用"),
                             tr("Windows 系统托盘当前不可用，提醒将改用应用内弹窗显示。"));
    }
    m_reminderService->start();
    return true;
}

void MemoApplication::togglePanel()
{
    if (m_panel->isVisible()) {
        hidePanel();
    } else {
        showPanel();
    }
}

void MemoApplication::showPanel()
{
    m_collapseTimer.stop();
    m_panel->refresh();
    positionPanel();
    m_panel->show();
    m_panel->raise();
    m_panel->activateWindow();
    m_panel->setFocus();
    m_collapseTimer.start(m_settings->collapseDelayMs());
}

void MemoApplication::hidePanel()
{
    m_collapseTimer.stop();
    m_panel->hide();
}

void MemoApplication::schedulePanelCollapse()
{
    if (m_panel->isVisible()) {
        m_collapseTimer.start(m_settings->collapseDelayMs());
    }
}

void MemoApplication::showContextMenu(const QPoint &globalPosition)
{
    m_trayMenu->popup(globalPosition);
}

void MemoApplication::quitApplication()
{
    m_settings->setFloatingBallPosition(m_floatingBall->pos());
    m_trayIcon->hide();
    QApplication::quit();
}

void MemoApplication::positionPanel()
{
    QScreen *screen = QGuiApplication::screenAt(m_floatingBall->frameGeometry().center());
    if (!screen) {
        screen = QGuiApplication::primaryScreen();
    }
    const QRect available = screen->availableGeometry();
    const int gap = 10;

    int x = m_floatingBall->x() + m_floatingBall->width() + gap;
    if (x + m_panel->width() > available.right()) {
        x = m_floatingBall->x() - m_panel->width() - gap;
    }
    x = qBound(available.left(), x, available.right() - m_panel->width() + 1);

    int y = m_floatingBall->y() + m_floatingBall->height() / 2 - m_panel->height() / 2;
    y = qBound(available.top(), y, available.bottom() - m_panel->height() + 1);
    m_panel->move(x, y);
}

void MemoApplication::createTrayMenu()
{
    m_trayMenu = new QMenu;

    QAction *showAction = m_trayMenu->addAction(tr("显示今天的备忘录"));
    QAction *newAction = m_trayMenu->addAction(tr("新建备忘录"));
    m_trayMenu->addSeparator();
    QAction *autoStartAction = m_trayMenu->addAction(tr("登录 Windows 后自动启动"));
    autoStartAction->setCheckable(true);
    autoStartAction->setChecked(m_settings->startAtLogin());
    m_trayMenu->addSeparator();
    QAction *quitAction = m_trayMenu->addAction(tr("退出"));

    connect(showAction, &QAction::triggered, this, &MemoApplication::showPanel);
    connect(newAction, &QAction::triggered, this, [this] {
        showPanel();
        QMetaObject::invokeMethod(m_panel, "addMemo", Qt::QueuedConnection);
    });
    connect(autoStartAction, &QAction::toggled, this, [this, autoStartAction](bool enabled) {
        if (!m_settings->setStartAtLogin(enabled)) {
            const QSignalBlocker blocker(autoStartAction);
            autoStartAction->setChecked(!enabled);
            QMessageBox::warning(nullptr, tr("设置失败"), tr("无法修改 Windows 开机启动设置。"));
        }
    });
    connect(quitAction, &QAction::triggered, this, &MemoApplication::quitApplication);

    m_trayIcon->setContextMenu(m_trayMenu);
}

void MemoApplication::applyStyleSheet()
{
    QPalette palette = qApp->palette();
    palette.setColor(QPalette::Window, QColor(QStringLiteral("#F8FAFE")));
    palette.setColor(QPalette::WindowText, QColor(QStringLiteral("#202938")));
    palette.setColor(QPalette::Base, Qt::white);
    palette.setColor(QPalette::AlternateBase, QColor(QStringLiteral("#F3F6FA")));
    palette.setColor(QPalette::Text, QColor(QStringLiteral("#202938")));
    palette.setColor(QPalette::Button, Qt::white);
    palette.setColor(QPalette::ButtonText, QColor(QStringLiteral("#263448")));
    palette.setColor(QPalette::Highlight, QColor(QStringLiteral("#4F7CFF")));
    palette.setColor(QPalette::HighlightedText, Qt::white);
    palette.setColor(QPalette::ToolTipBase, Qt::white);
    palette.setColor(QPalette::ToolTipText, QColor(QStringLiteral("#202938")));
    palette.setColor(QPalette::PlaceholderText, QColor(QStringLiteral("#8995A5")));
    palette.setColor(QPalette::Disabled, QPalette::WindowText, QColor(QStringLiteral("#9AA6B7")));
    palette.setColor(QPalette::Disabled, QPalette::Text, QColor(QStringLiteral("#9AA6B7")));
    palette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(QStringLiteral("#9AA6B7")));
    qApp->setPalette(palette);

    qApp->setStyleSheet(QStringLiteral(R"QSS(
        QWidget {
            font-family: "Microsoft YaHei UI", "Segoe UI";
            font-size: 13px;
            color: #202938;
        }
        QMenu {
            background-color: #FFFFFF;
            color: #202938;
            border: 1px solid #D7DFEA;
            border-radius: 8px;
            padding: 6px;
        }
        QMenu::item {
            background-color: transparent;
            color: #202938;
            border-radius: 6px;
            padding: 8px 28px 8px 12px;
            min-width: 190px;
        }
        QMenu::item:selected {
            background-color: #E9EFFF;
            color: #254FAF;
        }
        QMenu::item:disabled { color: #9AA6B7; }
        QMenu::separator {
            height: 1px;
            background: #E5EAF2;
            margin: 5px 8px;
        }
        QMenu::indicator {
            width: 16px;
            height: 16px;
            left: 8px;
        }
        QMenu::indicator:checked {
            background: #4F7CFF;
            border: 1px solid #4F7CFF;
            border-radius: 4px;
        }
        QMessageBox {
            background-color: #F8FAFE;
            color: #202938;
        }
        QMessageBox QLabel {
            background-color: transparent;
            color: #202938;
            min-width: 280px;
            padding: 6px 4px;
            font-size: 14px;
        }
        QMessageBox QLabel#qt_msgbox_label {
            min-width: 300px;
            line-height: 1.5;
        }
        QMessageBox QPushButton {
            min-width: 88px;
            min-height: 34px;
            padding: 5px 16px;
            border: 1px solid #C8D2E1;
            border-radius: 7px;
            background-color: #FFFFFF;
            color: #263448;
        }
        QMessageBox QPushButton:hover {
            background-color: #E9EFFF;
            border-color: #9CB2EE;
            color: #254FAF;
        }
        QMessageBox QPushButton:pressed { background-color: #DCE6FF; }
        QMessageBox QPushButton:default {
            background-color: #4F7CFF;
            border-color: #4F7CFF;
            color: #FFFFFF;
        }
        QMessageBox QPushButton:default:hover {
            background-color: #3F6EE8;
            border-color: #3F6EE8;
            color: #FFFFFF;
        }
        QMessageBox QPushButton#destructiveButton {
            background-color: #E5484D;
            border-color: #E5484D;
            color: #FFFFFF;
        }
        QMessageBox QPushButton#destructiveButton:hover {
            background-color: #D63E43;
            border-color: #D63E43;
            color: #FFFFFF;
        }
        QMessageBox QPushButton#secondaryButton:default {
            background-color: #FFFFFF;
            border-color: #9FB0C6;
            color: #263448;
        }
        QDialog#deleteConfirmDialog { background: transparent; }
        QFrame#deleteDialogSurface {
            background: #FFFFFF;
            border: 1px solid #D9E1EC;
            border-radius: 14px;
        }
        QLabel#deleteWarningIcon {
            background: #FFF0F0;
            color: #D93F45;
            border: 1px solid #FFD6D8;
            border-radius: 21px;
            font-size: 24px;
            font-weight: 700;
        }
        QLabel#deleteDialogTitle {
            color: #1F2B3D;
            font-size: 17px;
            font-weight: 700;
        }
        QLabel#deleteDialogMessage {
            color: #6F7C8F;
            font-size: 13px;
        }
        QPushButton#deleteCancelButton,
        QPushButton#deleteConfirmButton {
            border-radius: 8px;
            font-size: 13px;
            font-weight: 600;
        }
        QPushButton#deleteCancelButton {
            background: #FFFFFF;
            color: #344258;
            border: 1px solid #C8D2E1;
        }
        QPushButton#deleteCancelButton:hover {
            background: #F2F5F9;
            border-color: #AAB7C9;
        }
        QPushButton#deleteCancelButton:focus { border: 2px solid #7899ED; }
        QPushButton#deleteConfirmButton {
            background: #E5484D;
            color: #FFFFFF;
            border: 1px solid #E5484D;
        }
        QPushButton#deleteConfirmButton:hover {
            background: #D63E43;
            border-color: #D63E43;
        }
        QPushButton#deleteConfirmButton:pressed { background: #C9363B; }
        QWidget#panelSurface {
            background: #F8FAFE;
            border: 1px solid #DEE5F1;
            border-radius: 16px;
        }
        QLabel#panelTitle {
            font-size: 20px;
            font-weight: 700;
            color: #1E2A3B;
        }
        QLabel#dateLabel, QLabel#metaLabel {
            color: #7A8799;
            font-size: 11px;
        }
        QPushButton#dateNavigationButton,
        QPushButton#todayButton {
            border: 1px solid #D5DEEA;
            border-radius: 7px;
            background: #FFFFFF;
            color: #536177;
        }
        QPushButton#dateNavigationButton {
            font-size: 22px;
            font-weight: 500;
            padding-bottom: 3px;
        }
        QPushButton#todayButton {
            padding: 4px 12px;
            font-size: 12px;
            font-weight: 600;
        }
        QPushButton#dateNavigationButton:hover,
        QPushButton#todayButton:hover {
            background: #E9EFFF;
            border-color: #AFC2F5;
            color: #315FD6;
        }
        QPushButton#dateNavigationButton:pressed,
        QPushButton#todayButton:pressed { background: #DCE6FF; }
        QPushButton#dateNavigationButton:disabled,
        QPushButton#todayButton:disabled {
            background: #F2F5F9;
            border-color: #E4E9F0;
            color: #AAB4C2;
        }
        QLabel#emptyLabel {
            color: #8995A5;
            line-height: 1.6;
        }
        QScrollArea#memoScrollArea,
        QScrollArea#memoScrollArea > QWidget > QWidget,
        QWidget#memoListContainer {
            background: transparent;
        }
        QWidget#memoItem {
            background: white;
            border: 1px solid #E7ECF4;
            border-radius: 10px;
        }
        QWidget#memoItem:hover {
            border-color: #AFC2F5;
            background: #FCFDFF;
        }
        QLabel#memoTitleLabel {
            color: #1F2B3D;
            font-size: 13px;
        }
        QLabel#memoTitleLabel[completed="true"] { color: #8A96A6; }
        QCheckBox {
            spacing: 7px;
            color: #425067;
        }
        QCheckBox::indicator {
            width: 17px;
            height: 17px;
            border: 1px solid #B9C4D3;
            border-radius: 5px;
            background: #FFFFFF;
        }
        QCheckBox::indicator:hover {
            border-color: #6E91EA;
            background: #F2F6FF;
        }
        QCheckBox::indicator:checked {
            image: url(:/icons/icons/check.svg);
            background: #4F7CFF;
            border-color: #4F7CFF;
        }
        QPushButton#memoEditButton,
        QPushButton#memoDeleteButton {
            border: none;
            border-radius: 7px;
            background: transparent;
        }
        QPushButton#memoEditButton:hover { background: #E9EFFF; }
        QPushButton#memoDeleteButton:hover { background: #FFECEC; }
        QPushButton#addButton {
            border: none;
            border-radius: 10px;
            background: #E9EFFF;
        }
        QPushButton#addButton:hover { background: #DCE6FF; }
        QPushButton#closeButton {
            border: none;
            border-radius: 9px;
            color: #6F7B8D;
            font-size: 24px;
        }
        QPushButton#closeButton:hover { background: #E9EDF4; }
        QPushButton:flat:hover { background: #EDF2FC; border-radius: 7px; }
        QLineEdit, QTextEdit, QDateTimeEdit {
            background: white;
            border: 1px solid #CBD4E2;
            border-radius: 7px;
            padding: 7px;
            selection-background-color: #4F7CFF;
        }
        QLineEdit:focus, QTextEdit:focus, QDateTimeEdit:focus { border-color: #4F7CFF; }
        QScrollArea { background: transparent; }
        QScrollArea > QWidget > QWidget { background: transparent; }
        QScrollBar:vertical {
            background: transparent;
            width: 7px;
            margin: 2px 0;
        }
        QScrollBar::handle:vertical {
            background: #BCC6D5;
            border-radius: 3px;
            min-height: 28px;
        }
        QScrollBar::handle:vertical:hover { background: #91A0B5; }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
        QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: transparent; }

        QDialog#memoEditorDialog { background: transparent; }
        QFrame#editorSurface {
            background: #F8FAFE;
            border: 1px solid #CFD9E8;
            border-radius: 12px;
        }
        QLabel#editorTitleIcon {
            background: #4F7CFF;
            color: white;
            border-radius: 7px;
            font-size: 13px;
            font-weight: 700;
        }
        QLabel#editorTitle {
            color: #1E2A3B;
            font-size: 14px;
            font-weight: 700;
        }
        QLabel#editorFieldLabel {
            color: #657286;
            padding-top: 8px;
        }
        QPushButton#editorCloseButton {
            border: none;
            border-radius: 8px;
            background: transparent;
            color: #647287;
            font-size: 22px;
        }
        QPushButton#editorCloseButton:hover {
            background: #E7ECF4;
            color: #27364A;
        }
        QLineEdit#editorTitleInput,
        QTextEdit#editorContentInput,
        QDateEdit#editorReminderDate,
        QTimeEdit#editorReminderTime {
            color: #263448;
            background: white;
            border: 1px solid #C8D2E1;
            border-radius: 8px;
            padding: 7px 10px;
        }
        QLineEdit#editorTitleInput:focus,
        QTextEdit#editorContentInput:focus,
        QDateEdit#editorReminderDate:focus,
        QTimeEdit#editorReminderTime:focus {
            border: 2px solid #4F7CFF;
            padding: 6px 9px;
        }
        QDateEdit#editorReminderDate:disabled,
        QTimeEdit#editorReminderTime:disabled {
            color: #9AA6B7;
            background: #EEF2F7;
            border-color: #D8DFE9;
        }
        QLabel#editorInlineLabel {
            color: #657286;
            padding: 0 2px;
        }
        QDateEdit#editorReminderDate::drop-down {
            subcontrol-origin: padding;
            subcontrol-position: top right;
            width: 32px;
            border: none;
            border-left: 1px solid #E1E6EE;
            background: #F4F7FB;
            border-top-right-radius: 7px;
            border-bottom-right-radius: 7px;
        }
        QDateEdit#editorReminderDate::drop-down:hover { background: #E9EFFF; }
        QDateEdit#editorReminderDate::down-arrow {
            image: url(:/icons/icons/chevron-down.svg);
            width: 14px;
            height: 14px;
        }
        QTimeEdit#editorReminderTime::up-button,
        QTimeEdit#editorReminderTime::down-button {
            width: 18px;
            background: #F4F7FB;
            border: none;
            border-left: 1px solid #E1E6EE;
        }
        QTimeEdit#editorReminderTime::up-button:hover,
        QTimeEdit#editorReminderTime::down-button:hover { background: #E9EFFF; }
        QTimeEdit#editorReminderTime::up-arrow {
            image: url(:/icons/icons/chevron-up.svg);
            width: 12px;
            height: 12px;
        }
        QTimeEdit#editorReminderTime::down-arrow {
            image: url(:/icons/icons/chevron-down.svg);
            width: 12px;
            height: 12px;
        }
        QCalendarWidget QWidget#qt_calendar_navigationbar {
            background: #4F7CFF;
            color: white;
            min-height: 34px;
        }
        QCalendarWidget QToolButton {
            background: transparent;
            color: white;
            border: none;
            border-radius: 6px;
            padding: 5px 8px;
            font-weight: 600;
        }
        QCalendarWidget QToolButton:hover { background: rgba(255, 255, 255, 45); }
        QCalendarWidget QToolButton::menu-indicator { image: none; }
        QCalendarWidget QToolButton#qt_calendar_prevmonth {
            qproperty-icon: url(:/icons/icons/chevron-left.svg);
        }
        QCalendarWidget QToolButton#qt_calendar_nextmonth {
            qproperty-icon: url(:/icons/icons/chevron-right.svg);
        }
        QCalendarWidget QSpinBox {
            background: white;
            color: #202938;
            border: 1px solid #CBD4E2;
            border-radius: 5px;
            padding: 3px 6px;
            selection-background-color: #4F7CFF;
            selection-color: white;
        }
        QCalendarWidget QAbstractItemView:enabled {
            background-color: #FFFFFF;
            alternate-background-color: #FFFFFF;
            color: #263448;
            selection-background-color: #4F7CFF;
            selection-color: #FFFFFF;
            outline: 0;
        }
        QCalendarWidget QAbstractItemView:disabled { color: #A7B1C0; }
        QCalendarWidget QTableView {
            background: #FFFFFF;
            color: #263448;
            border: 1px solid #D8E0EB;
            gridline-color: transparent;
            selection-background-color: #4F7CFF;
            selection-color: white;
        }
        QCalendarWidget QTableView QHeaderView::section {
            background: #F0F4FA;
            color: #536177;
            border: none;
            padding: 4px;
            font-weight: 600;
        }
        QCalendarWidget QWidget {
            background-color: #FFFFFF;
            color: #263448;
        }
        QCheckBox#editorReminderCheck {
            color: #425067;
            spacing: 8px;
        }
        QCheckBox#editorReminderCheck::indicator {
            width: 17px;
            height: 17px;
        }
        QCheckBox#editorReminderCheck::indicator:checked {
            image: url(:/icons/icons/check.svg);
        }
        QPushButton#editorSaveButton,
        QPushButton#editorCancelButton {
            border: 1px solid #C8D2E1;
            border-radius: 8px;
            padding: 6px 16px;
            background: white;
            color: #344258;
        }
        QPushButton#editorSaveButton {
            background: #4F7CFF;
            border-color: #4F7CFF;
            color: white;
        }
        QPushButton#editorSaveButton:hover { background: #3F6EE8; }
        QPushButton#editorCancelButton:hover { background: #EEF2F8; }
        QPushButton#editorSaveButton:disabled {
            background: #CBD4E2;
            border-color: #CBD4E2;
            color: #F4F6F9;
        }
    )QSS"));
}

bool MemoApplication::cursorWithinAssistant() const
{
    const QPoint cursor = QCursor::pos();
    return m_floatingBall->frameGeometry().contains(cursor)
        || (m_panel->isVisible() && m_panel->frameGeometry().contains(cursor));
}
