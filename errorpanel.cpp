#include "errorpanel.h"
#include "QLayout"
#include "QVBoxLayout"
#include "QHBoxLayout"
#include "QSpacerItem"
#include <QSizePolicy>
#include "uidescription.h"

ErrorPanel::ErrorPanel(QWidget *parent)
    : QWidget{parent}
{
    this->setLayout(new QHBoxLayout());
    fBackground = new QFrame(this);
    fBackground->setLayout(new QVBoxLayout());
    fBackground->setObjectName("ErrorPanelBackground");
    this->layout()->addWidget(fBackground);

    static QLabel *name = new QLabel("Статуси\nпомилок", this);
    name->setObjectName("ErrPanName");
    name->setAlignment(Qt::AlignCenter);
    fBackground->layout()->addWidget(name);
    foreach (auto item, UiDescription::getErrorNameList()) {
        indicatoList.append(new ErrorIndicator(this, item, QSize(40,40)));
        fBackground->layout()->addWidget(indicatoList.last());
    }
    fBackground->layout()->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));
    fBackground->layout()->setContentsMargins(0,0,0,0);
    this->layout()->setContentsMargins(0,0,0,0);
}

void ErrorPanel::setErrorState(int item, bool isError) {
    if (item >= indicatoList.length()) {
        return;
    }
    indicatoList[item]->setState(isError);
}