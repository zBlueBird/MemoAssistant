#include "ui/MemoPanel.h"

#include "data/MemoRepository.h"
#include "ui/DeleteConfirmDialog.h"
#include "ui/IconFactory.h"
#include "ui/MemoDetailsDialog.h"
#include "ui/MemoEditorDialog.h"
#include "ui/MemoItemWidget.h"
#include "ui/TaskCalendarWidget.h"
#include "ui/WeekOverviewWidget.h"

#include <QDate>
#include <QEvent>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLocale>
#include <QLabel>
#include <QMessageBox>
#include <QPainter>
#include <QPushButton>
#include <QScrollArea>
#include <QStackedWidget>
#include <QVBoxLayout>

MemoPanel::MemoPanel(MemoRepository *repository, QWidget *parent)
    : QWidget(parent, Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint),
      m_repository(repository),
      m_selectedDate(QDate::currentDate()),
      m_titleLabel(new QLabel(this)),
      m_dateLabel(new QLabel(this)),
      m_previousDayButton(new QPushButton(QStringLiteral("‹"), this)),
      m_nextDayButton(new QPushButton(QStringLiteral("›"), this)),
      m_todayButton(new QPushButton(tr("今天"), this)),
      m_dayViewButton(new QPushButton(tr("日"), this)),
      m_weekViewButton(new QPushButton(tr("周"), this)),
      m_monthViewButton(new QPushButton(tr("月"), this)),
      m_contentStack(new QStackedWidget(this)),
      m_weekOverview(new WeekOverviewWidget(this)),
      m_calendar(new TaskCalendarWidget(this)),
      m_scrollArea(new QScrollArea(this)),
      m_listContainer(new QWidget),
      m_listLayout(new QVBoxLayout(m_listContainer)),
      m_emptyLabel(new QLabel(m_listContainer))
{
    setObjectName(QStringLiteral("memoPanel"));
    setAttribute(Qt::WA_TranslucentBackground);
    setFocusPolicy(Qt::StrongFocus);
    setFixedWidth(420);
    setMinimumHeight(220);
    setMaximumHeight(560);

    m_titleLabel->setObjectName(QStringLiteral("panelTitle"));
    m_dateLabel->setObjectName(QStringLiteral("dateLabel"));

    auto *titleLayout = new QVBoxLayout;
    titleLayout->setContentsMargins(0, 0, 0, 0);
    titleLayout->setSpacing(2);
    titleLayout->addWidget(m_titleLabel);
    titleLayout->addWidget(m_dateLabel);

    auto *addButton = new QPushButton(IconFactory::addIcon(), QString(), this);
    addButton->setObjectName(QStringLiteral("addButton"));
    addButton->setToolTip(tr("新建备忘录"));
    addButton->setFixedSize(38, 38);
    addButton->setIconSize(QSize(22, 22));

    auto *closeButton = new QPushButton(QStringLiteral("×"), this);
    closeButton->setObjectName(QStringLiteral("closeButton"));
    closeButton->setToolTip(tr("收起"));
    closeButton->setFixedSize(34, 34);

    auto *header = new QHBoxLayout;
    header->addLayout(titleLayout, 1);
    header->addWidget(addButton);
    header->addWidget(closeButton);

    for (QPushButton *button : {m_previousDayButton, m_nextDayButton}) {
        button->setObjectName(QStringLiteral("dateNavigationButton"));
        button->setFixedSize(34, 30);
        button->setFocusPolicy(Qt::NoFocus);
    }
    m_previousDayButton->setToolTip(tr("查看前一天"));
    m_nextDayButton->setToolTip(tr("查看后一天"));
    m_previousDayButton->setAccessibleName(tr("前一天"));
    m_nextDayButton->setAccessibleName(tr("后一天"));

    m_todayButton->setObjectName(QStringLiteral("todayButton"));
    m_todayButton->setFixedHeight(30);
    m_todayButton->setMinimumWidth(56);
    m_todayButton->setFocusPolicy(Qt::NoFocus);
    m_todayButton->setToolTip(tr("回到今天"));

    auto *dateNavigation = new QHBoxLayout;
    dateNavigation->setContentsMargins(0, 0, 0, 0);
    dateNavigation->setSpacing(8);
    dateNavigation->addWidget(m_previousDayButton);
    dateNavigation->addStretch();
    dateNavigation->addWidget(m_todayButton);
    dateNavigation->addStretch();
    dateNavigation->addWidget(m_nextDayButton);

    for (QPushButton *button : {m_dayViewButton, m_weekViewButton, m_monthViewButton}) {
        button->setObjectName(QStringLiteral("viewModeButton"));
        button->setCheckable(true);
        button->setAutoExclusive(true);
        button->setFixedSize(46, 28);
        button->setFocusPolicy(Qt::NoFocus);
    }
    m_dayViewButton->setChecked(true);

    auto *viewModes = new QHBoxLayout;
    viewModes->setContentsMargins(0, 0, 0, 0);
    viewModes->setSpacing(0);
    viewModes->addStretch();
    viewModes->addWidget(m_dayViewButton);
    viewModes->addWidget(m_weekViewButton);
    viewModes->addWidget(m_monthViewButton);
    viewModes->addStretch();

    m_listLayout->setContentsMargins(0, 0, 0, 0);
    m_listLayout->setSpacing(8);
    m_listLayout->addStretch();

    m_emptyLabel->setObjectName(QStringLiteral("emptyLabel"));
    m_emptyLabel->setAlignment(Qt::AlignCenter);
    m_emptyLabel->setMinimumHeight(105);

    m_listContainer->setObjectName(QStringLiteral("memoListContainer"));
    m_scrollArea->setObjectName(QStringLiteral("memoScrollArea"));
    m_scrollArea->setWidget(m_listContainer);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_calendar->setMinimumDate(QDate(2000, 1, 1));
    m_calendar->setMaximumDate(QDate::currentDate().addYears(20));
    m_calendar->setSelectedDate(m_selectedDate);
    m_weekOverview->setToolTip(tr("单击选择日期，双击查看当天任务"));
    m_calendar->setToolTip(tr("单击选择日期，双击查看当天任务"));
    m_contentStack->addWidget(m_scrollArea);
    m_contentStack->addWidget(m_weekOverview);
    m_contentStack->addWidget(m_calendar);

    auto *surface = new QWidget(this);
    surface->setObjectName(QStringLiteral("panelSurface"));
    auto *surfaceLayout = new QVBoxLayout(surface);
    surfaceLayout->setContentsMargins(18, 16, 18, 18);
    surfaceLayout->setSpacing(14);
    surfaceLayout->addLayout(header);
    surfaceLayout->addLayout(viewModes);
    surfaceLayout->addLayout(dateNavigation);
    surfaceLayout->addWidget(m_contentStack, 1);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(10, 10, 10, 12);
    layout->addWidget(surface);

    auto *shadow = new QGraphicsDropShadowEffect(surface);
    shadow->setBlurRadius(26);
    shadow->setColor(QColor(20, 32, 55, 75));
    shadow->setOffset(0, 5);
    surface->setGraphicsEffect(shadow);

    connect(m_previousDayButton, &QPushButton::clicked, this, &MemoPanel::showPreviousPeriod);
    connect(m_nextDayButton, &QPushButton::clicked, this, &MemoPanel::showNextPeriod);
    connect(m_todayButton, &QPushButton::clicked, this, &MemoPanel::showToday);
    connect(m_dayViewButton, &QPushButton::clicked, this, &MemoPanel::setDayView);
    connect(m_weekViewButton, &QPushButton::clicked, this, &MemoPanel::setWeekView);
    connect(m_monthViewButton, &QPushButton::clicked, this, &MemoPanel::setMonthView);
    connect(m_weekOverview, &WeekOverviewWidget::dateSelected,
            this, &MemoPanel::selectOverviewDate);
    connect(m_weekOverview, &WeekOverviewWidget::dateActivated,
            this, &MemoPanel::openDateInDayView);
    connect(m_calendar, &QCalendarWidget::clicked,
            this, &MemoPanel::selectOverviewDate);
    connect(m_calendar, &QCalendarWidget::activated,
            this, &MemoPanel::openDateInDayView);
    connect(addButton, &QPushButton::clicked, this, &MemoPanel::addMemo);
    connect(closeButton, &QPushButton::clicked, this, &MemoPanel::closeRequested);

    updateDateControls();
}

void MemoPanel::refresh()
{
    updateDateControls();
    updateCalendar();
    clearList();
    const QList<Memo> memos = m_repository->scheduledOn(m_selectedDate);
    const bool isEmpty = memos.isEmpty();
    m_emptyLabel->setText(m_selectedDate == QDate::currentDate()
                              ? tr("今天还没有备忘录\n点击右上角的＋开始记录")
                              : tr("这一天没有备忘录"));
    m_scrollArea->setVerticalScrollBarPolicy(isEmpty ? Qt::ScrollBarAlwaysOff
                                                     : Qt::ScrollBarAsNeeded);
    m_emptyLabel->setVisible(isEmpty);
    if (isEmpty) {
        m_listLayout->insertWidget(0, m_emptyLabel);
    }

    for (const Memo &memo : memos) {
        auto *item = new MemoItemWidget(memo, m_listContainer);
        connect(item, &MemoItemWidget::completionChanged,
                this, &MemoPanel::setMemoCompleted);
        connect(item, &MemoItemWidget::detailsRequested,
                this, &MemoPanel::showMemoDetails);
        connect(item, &MemoItemWidget::editRequested,
                this, &MemoPanel::editMemo);
        connect(item, &MemoItemWidget::deleteRequested,
                this, &MemoPanel::deleteMemo);
        m_listLayout->insertWidget(m_listLayout->count() - 1, item);
    }

    const int preferredHeight = m_viewMode == ViewMode::Day
                                    ? (isEmpty ? 300 : qBound(314, 200 + memos.size() * 78, 540))
                                    : 500;
    resize(width(), preferredHeight);
}

void MemoPanel::showPreviousPeriod()
{
    if (m_viewMode == ViewMode::Month) {
        m_selectedDate = m_selectedDate.addMonths(-1);
    } else if (m_viewMode == ViewMode::Week) {
        m_selectedDate = m_selectedDate.addDays(-7);
    } else {
        m_selectedDate = m_selectedDate.addDays(-1);
    }
    refresh();
}

void MemoPanel::showNextPeriod()
{
    if (m_viewMode == ViewMode::Month) {
        m_selectedDate = m_selectedDate.addMonths(1);
    } else if (m_viewMode == ViewMode::Week) {
        m_selectedDate = m_selectedDate.addDays(7);
    } else {
        m_selectedDate = m_selectedDate.addDays(1);
    }
    refresh();
}

void MemoPanel::showToday()
{
    m_selectedDate = QDate::currentDate();
    refresh();
}

void MemoPanel::setDayView()
{
    setViewMode(ViewMode::Day);
}

void MemoPanel::setWeekView()
{
    setViewMode(ViewMode::Week);
}

void MemoPanel::setMonthView()
{
    setViewMode(ViewMode::Month);
}

void MemoPanel::selectOverviewDate(const QDate &date)
{
    m_selectedDate = date;
    refresh();
}

void MemoPanel::openDateInDayView(const QDate &date)
{
    m_selectedDate = date;
    setViewMode(ViewMode::Day);
}

void MemoPanel::leaveEvent(QEvent *event)
{
    QWidget::leaveEvent(event);
    emit pointerLeft();
}

bool MemoPanel::event(QEvent *event)
{
    if (event->type() == QEvent::WindowDeactivate) {
        emit pointerLeft();
    }
    return QWidget::event(event);
}

void MemoPanel::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        emit closeRequested();
        event->accept();
        return;
    }
    QWidget::keyPressEvent(event);
}

void MemoPanel::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
}

void MemoPanel::addMemo()
{
    MemoEditorDialog editor(this);
    editor.setScheduledDate(m_selectedDate);
    if (editor.exec() != QDialog::Accepted) {
        return;
    }
    const Memo memo = editor.memo();
    if (!m_repository->add(memo)) {
        showError(tr("创建备忘录"));
        return;
    }
    m_selectedDate = memo.scheduledDate;
    setViewMode(ViewMode::Day);
    emit memoChanged();
}

void MemoPanel::showMemoDetails(qint64 id)
{
    const std::optional<Memo> memo = m_repository->findById(id);
    if (!memo) {
        showError(tr("读取备忘录"));
        return;
    }
    MemoDetailsDialog details(*memo, this);
    details.exec();
}

void MemoPanel::editMemo(qint64 id)
{
    const std::optional<Memo> current = m_repository->findById(id);
    if (!current) {
        showError(tr("读取备忘录"));
        return;
    }

    MemoEditorDialog editor(this);
    editor.setMemo(*current);
    if (editor.exec() != QDialog::Accepted) {
        return;
    }
    const Memo updatedMemo = editor.memo();
    if (!m_repository->update(updatedMemo)) {
        showError(tr("更新备忘录"));
        return;
    }
    m_selectedDate = updatedMemo.scheduledDate;
    setViewMode(ViewMode::Day);
    emit memoChanged();
}

void MemoPanel::deleteMemo(qint64 id)
{
    DeleteConfirmDialog confirmation(this);
    if (confirmation.exec() != QDialog::Accepted) {
        return;
    }
    if (!m_repository->remove(id)) {
        showError(tr("删除备忘录"));
        return;
    }
    refresh();
    emit memoChanged();
}

void MemoPanel::setMemoCompleted(qint64 id, bool completed)
{
    if (!m_repository->setCompleted(id, completed)) {
        showError(tr("更新完成状态"));
        refresh();
        return;
    }
    refresh();
    emit memoChanged();
}

void MemoPanel::clearList()
{
    m_emptyLabel->setParent(m_listContainer);
    for (int index = m_listLayout->count() - 1; index >= 0; --index) {
        QLayoutItem *item = m_listLayout->takeAt(index);
        if (QWidget *widget = item->widget(); widget && widget != m_emptyLabel) {
            widget->deleteLater();
        }
        delete item;
    }
    m_listLayout->addStretch();
}

void MemoPanel::updateDateControls()
{
    const QDate today = QDate::currentDate();
    if (!m_selectedDate.isValid()) {
        m_selectedDate = today;
    }

    if (m_viewMode == ViewMode::Day) {
        if (m_selectedDate == today) {
            m_titleLabel->setText(tr("今天的备忘录"));
        } else if (m_selectedDate == today.addDays(-1)) {
            m_titleLabel->setText(tr("昨天的备忘录"));
        } else if (m_selectedDate > today) {
            m_titleLabel->setText(tr("未来备忘录"));
        } else {
            m_titleLabel->setText(tr("历史备忘录"));
        }
        m_dateLabel->setText(QLocale().toString(m_selectedDate, QStringLiteral("yyyy年M月d日 dddd")));
    } else {
        const auto [start, end] = visibleRange();
        m_titleLabel->setText(m_viewMode == ViewMode::Week ? tr("一周任务") : tr("一月任务"));
        m_dateLabel->setText(m_viewMode == ViewMode::Week
                                 ? tr("%1 至 %2").arg(start.toString(QStringLiteral("M月d日")),
                                                     end.toString(QStringLiteral("M月d日")))
                                 : m_selectedDate.toString(QStringLiteral("yyyy年M月")));
    }

    m_todayButton->setEnabled(m_selectedDate != today || m_viewMode != ViewMode::Day);
    m_previousDayButton->setToolTip(m_viewMode == ViewMode::Day ? tr("前一天")
                                                                 : m_viewMode == ViewMode::Week ? tr("前一周")
                                                                                                 : tr("前一月"));
    m_nextDayButton->setToolTip(m_viewMode == ViewMode::Day ? tr("后一天")
                                                             : m_viewMode == ViewMode::Week ? tr("后一周")
                                                                                             : tr("后一月"));
}

void MemoPanel::updateCalendar()
{
    if (m_viewMode == ViewMode::Day) {
        m_contentStack->setCurrentWidget(m_scrollArea);
        return;
    }

    const auto [start, end] = visibleRange();
    const QMap<QDate, DailyTaskSummary> counts = m_repository->taskCounts(start, end);
    if (m_viewMode == ViewMode::Week) {
        m_weekOverview->setWeek(start, m_selectedDate, counts);
        m_contentStack->setCurrentWidget(m_weekOverview);
        return;
    }

    m_calendar->setMinimumDate(start);
    m_calendar->setMaximumDate(end);
    m_calendar->setCurrentPage(m_selectedDate.year(), m_selectedDate.month());
    m_calendar->setSelectedDate(m_selectedDate);
    m_calendar->setTaskCounts(counts);
    m_contentStack->setCurrentWidget(m_calendar);
}

QPair<QDate, QDate> MemoPanel::visibleRange() const
{
    if (m_viewMode == ViewMode::Week) {
        const QDate start = m_selectedDate.addDays(1 - m_selectedDate.dayOfWeek());
        return {start, start.addDays(6)};
    }
    if (m_viewMode == ViewMode::Month) {
        const QDate start(m_selectedDate.year(), m_selectedDate.month(), 1);
        return {start, start.addMonths(1).addDays(-1)};
    }
    return {m_selectedDate, m_selectedDate};
}

void MemoPanel::setViewMode(ViewMode mode)
{
    m_viewMode = mode;
    m_dayViewButton->setChecked(mode == ViewMode::Day);
    m_weekViewButton->setChecked(mode == ViewMode::Week);
    m_monthViewButton->setChecked(mode == ViewMode::Month);
    refresh();
}

void MemoPanel::showError(const QString &operation)
{
    QMessageBox::critical(this,
                          tr("操作失败"),
                          tr("%1失败。\n%2").arg(operation, m_repository->lastError()));
}
