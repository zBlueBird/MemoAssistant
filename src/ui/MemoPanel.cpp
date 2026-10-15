#include "ui/MemoPanel.h"

#include "data/MemoRepository.h"
#include "ui/DeleteConfirmDialog.h"
#include "ui/IconFactory.h"
#include "ui/MemoEditorDialog.h"
#include "ui/MemoItemWidget.h"

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
#include <QVBoxLayout>

MemoPanel::MemoPanel(MemoRepository *repository, QWidget *parent)
    : QWidget(parent, Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint),
      m_repository(repository),
      m_dateLabel(new QLabel(this)),
      m_scrollArea(new QScrollArea(this)),
      m_listContainer(new QWidget),
      m_listLayout(new QVBoxLayout(m_listContainer)),
      m_emptyLabel(new QLabel(tr("今天还没有备忘录\n点击右上角的＋开始记录"), m_listContainer))
{
    setObjectName(QStringLiteral("memoPanel"));
    setAttribute(Qt::WA_TranslucentBackground);
    setFocusPolicy(Qt::StrongFocus);
    setFixedWidth(380);
    setMinimumHeight(220);
    setMaximumHeight(560);

    auto *title = new QLabel(tr("今天的备忘录"), this);
    title->setObjectName(QStringLiteral("panelTitle"));
    m_dateLabel->setObjectName(QStringLiteral("dateLabel"));

    auto *titleLayout = new QVBoxLayout;
    titleLayout->setContentsMargins(0, 0, 0, 0);
    titleLayout->setSpacing(2);
    titleLayout->addWidget(title);
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

    auto *surface = new QWidget(this);
    surface->setObjectName(QStringLiteral("panelSurface"));
    auto *surfaceLayout = new QVBoxLayout(surface);
    surfaceLayout->setContentsMargins(18, 16, 18, 18);
    surfaceLayout->setSpacing(14);
    surfaceLayout->addLayout(header);
    surfaceLayout->addWidget(m_scrollArea, 1);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(10, 10, 10, 12);
    layout->addWidget(surface);

    auto *shadow = new QGraphicsDropShadowEffect(surface);
    shadow->setBlurRadius(26);
    shadow->setColor(QColor(20, 32, 55, 75));
    shadow->setOffset(0, 5);
    surface->setGraphicsEffect(shadow);

    connect(addButton, &QPushButton::clicked, this, &MemoPanel::addMemo);
    connect(closeButton, &QPushButton::clicked, this, &MemoPanel::closeRequested);
}

void MemoPanel::refresh()
{
    m_dateLabel->setText(QLocale().toString(QDate::currentDate(), QStringLiteral("M月d日 dddd")));
    clearList();
    const QList<Memo> memos = m_repository->createdOn(QDate::currentDate());
    const bool isEmpty = memos.isEmpty();
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
        connect(item, &MemoItemWidget::editRequested,
                this, &MemoPanel::editMemo);
        connect(item, &MemoItemWidget::deleteRequested,
                this, &MemoPanel::deleteMemo);
        m_listLayout->insertWidget(m_listLayout->count() - 1, item);
    }

    const int preferredHeight = isEmpty ? 226 : qBound(240, 126 + memos.size() * 78, 540);
    resize(width(), preferredHeight);
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
    if (editor.exec() != QDialog::Accepted) {
        return;
    }
    if (!m_repository->add(editor.memo())) {
        showError(tr("创建备忘录"));
        return;
    }
    refresh();
    emit memoChanged();
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
    if (!m_repository->update(editor.memo())) {
        showError(tr("更新备忘录"));
        return;
    }
    refresh();
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

void MemoPanel::showError(const QString &operation)
{
    QMessageBox::critical(this,
                          tr("操作失败"),
                          tr("%1失败。\n%2").arg(operation, m_repository->lastError()));
}
