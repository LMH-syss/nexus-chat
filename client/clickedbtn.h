#ifndef CLICKEDBTN_H
#define CLICKEDBTN_H

#include <QPushButton>

class ClickedBtn : public QPushButton
{
    Q_OBJECT
public:
    explicit ClickedBtn(QWidget *parent = nullptr);

    void SetState(const QString& normal, const QString& hover, const QString& press);

protected:
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    QString _normal;
    QString _hover;
    QString _press;
};

#endif // CLICKEDBTN_H
