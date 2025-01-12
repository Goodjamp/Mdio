#ifndef TSPOLLSWITCHER_H
#define TSPOLLSWITCHER_H

#include <QWidget>
#include <QMouseEvent>

namespace Ui {
class TsPollSwitcher;
}

class TsPollSwitcher : public QWidget
{
    Q_OBJECT

public:
    explicit TsPollSwitcher(QWidget *parent = nullptr);
    ~TsPollSwitcher();

    typedef enum {
        POLLING_TYPE_BINARY,
        POLLING_TYPE_REGISTER,
    } PollingType;

    PollingType getPollingType(void);

private slots:
    void on_pbPollingSwitcher_toggled(bool checked);

private:
    Ui::TsPollSwitcher *ui;

    void mousePressEvent(QMouseEvent *event) override;

};

#endif // TSPOLLSWITCHER_H
