#include "ui/MemoItemWidget.h"

#include "ui/IconFactory.h"

#include <QCheckBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QPushButton>
#include <QVBoxLayout>

MemoItemWidget::MemoItemWidget(const Memo &memo, QWidget *parent)
    : QWidget(parent),
      m_memo(memo),
      m_completedCheck(new QCheckBox(this)),
      m_titleLabel(new QLabel(memo.title, this))
{
    setObjectName(QStringLiteral("memoItem"));
    setAttribute(Qt::WA_StyledBackground, true);
    setToolTip(tr("双击查看详情"));

    m_completedCheck->setChecked(memo.completed);
    m_completedCheck->setToolTip(tr("标记为完成"));

    m_titleLabel->setObjectName(QStringLiteral("memoTitleLabel"));
    m_titleLabel->setWordWrap(true);
    m_titleLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_titleLabel->setProperty("completed", memo.completed);
    if (memo.completed) {
        QFont completedFont = m_titleLabel->font();
        completedFont.setStrikeOut(true);
        m_titleLabel->setFont(completedFont);
    }

    auto *metaLabel = new QLabel(this);
    QStringList metadata;
    metadata << tr("计划 %1").arg(memo.scheduledDate.toString(QStringLiteral("MM-dd")));
    if (memo.reminderAt.isValid()) {
        metadata << tr("提醒 %1").arg(memo.reminderAt.toLocalTime().toString(QStringLiteral("MM-dd HH:mm")));
    }
    metaLabel->setText(metadata.join(QStringLiteral(" · ")));
    metaLabel->setObjectName(QStringLiteral("metaLabel"));

    auto *textLayout = new QVBoxLayout;
    textLayout->setContentsMargins(0, 0, 0, 0);
    textLayout->setSpacing(4);
    textLayout->addWidget(m_titleLabel);
    textLayout->addWidget(metaLabel);

    auto *editButton = new QPushButton(IconFactory::editIcon(), QString(), this);
    editButton->setObjectName(QStringLiteral("memoEditButton"));
    auto *deleteButton = new QPushButton(IconFactory::deleteIcon(), QString(), this);
    deleteButton->setObjectName(QStringLiteral("memoDeleteButton"));
    for (QPushButton *button : {editButton, deleteButton}) {
        button->setFixedSize(30, 30);
        button->setIconSize(QSize(17, 17));
        button->setFlat(true);
    }
    editButton->setToolTip(tr("编辑"));
    deleteButton->setToolTip(tr("删除"));

    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(12, 10, 8, 10);
    layout->setSpacing(9);
    layout->addWidget(m_completedCheck, 0, Qt::AlignVCenter);
    layout->addLayout(textLayout, 1);
    layout->addWidget(editButton, 0, Qt::AlignTop);
    layout->addWidget(deleteButton, 0, Qt::AlignTop);

    connect(m_completedCheck, &QCheckBox::toggled, this, [this](bool completed) {
        emit completionChanged(m_memo.id, completed);
    });
    connect(editButton, &QPushButton::clicked, this, [this] { emit editRequested(m_memo.id); });
    connect(deleteButton, &QPushButton::clicked, this, [this] { emit deleteRequested(m_memo.id); });
}

void MemoItemWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        emit detailsRequested(m_memo.id);
        event->accept();
        return;
    }
    QWidget::mouseDoubleClickEvent(event);
}
