#include <QStyle>
#include <QDebug>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSpacerItem>
#include <QLabel>
#include <QSize>

#include "TeleControl.h"

const QSize TELECONTROL_PB_SIZE(60, 60);

TeleControlItem::TeleControlItem(QWidget *parent, const Descriptor &descriptorIn) {
    descriptor = descriptorIn;
    this->setLayout(new QVBoxLayout());
    this->layout()->setContentsMargins(0,0,0,0);

    contButton = new QPushButton(this);
    QFont font("Segoe UI", 11, QFont::Bold);
    contButton->setFont(font);
    contButton->setText(descriptor.name);
    contButton->setFixedSize(TELECONTROL_PB_SIZE);
    this->layout()->addWidget(contButton);
    contButton->setObjectName("contButton");
    connect(contButton, &QPushButton::clicked, this, &TeleControlItem::clickedSlot);
}







TeleControl::TeleControl(QWidget *parent, const Descriptor &descriptorIn)
    : QWidget(parent)
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    fBackground = new QFrame(this);
    auto *backgroundLayout = new QHBoxLayout(fBackground);
    mainLayout->addWidget(fBackground);

    // Left part: button and last action
    auto *leftLayout = new QVBoxLayout();
    backgroundLayout->addLayout(leftLayout);

    auto *controlaLayouts = new QHBoxLayout();
    leftLayout->addLayout(controlaLayouts);
    controlaLayouts->addStretch();
    for (const auto &item : descriptorIn.items) {
        auto *control = new TeleControlItem(this, item);
        connect(control, &TeleControlItem::clickedSignal, this, &TeleControl::clickedSlot);
        controlsList.insert(item.id, control);
        controlaLayouts->addWidget(control);
        controlaLayouts->addStretch();
    }

    auto *lastActionLayouts = new QHBoxLayout();
    leftLayout->addLayout(lastActionLayouts);
    lastActionLayouts->addStretch();
    lastActionLayouts->addWidget(new QLabel("Остання команда:", fBackground));

    lLastAction = new QLabel("-", fBackground);
    lastActionLayouts->addWidget(lLastAction);
    lastActionLayouts->addStretch();

    // Right part: name
    auto *lName = new QLabel(descriptorIn.name, fBackground);
    QFont font("Segoe UI", 12, QFont::Bold);
    lName->setFont(font);
    lName->setAlignment(Qt::AlignCenter);

    backgroundLayout->addWidget(lName);
}
